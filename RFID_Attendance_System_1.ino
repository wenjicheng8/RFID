#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

// ======================================================
// Wi-Fi Configuration
// ======================================================

const char* ssid = "Your Wifi";
const char* password = "Your Wifi Password";

// ======================================================
// Time Configuration
// ======================================================

// UTC+8
// If you want Japan Standard Time, use 9 * 3600

const long GMT_OFFSET_SEC = 8 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

// ======================================================
// Pin Definitions
// ======================================================

#define SS_PIN      5
#define RST_PIN     22
#define LED_PIN     25
#define BUZZER_PIN  26

// ======================================================
// Objects
// ======================================================

MFRC522 rfid(SS_PIN, RST_PIN);
Preferences preferences;

// ======================================================
// Student Structure
// ======================================================

struct Student {

  byte uid[4];

  const char* name;

  bool present;

  String time;
};

// ======================================================
// Student Database
// ======================================================

Student students[] = {

  // Student A
  {
    {0x87, 0xE3, 0x9E, 0xDD},
    "Student A",
    false,
    ""
  },

  // Student B
  {
    {0x37, 0xFD, 0xB5, 0xDD},
    "Student B",
    false,
    ""
  },

  // Student C
  {
    {0x47, 0xEB, 0xB3, 0xDD},
    "Student C",
    false,
    ""
  },

  // Student D
  {
    {0xA7, 0x74, 0x9F, 0xDD},
    "Student D",
    false,
    ""
  },

  // Student E
  {
    {0x17, 0x82, 0xBF, 0xDD},
    "Student E",
    false,
    ""
  }
};

const int NUM_STUDENTS =
  sizeof(students) / sizeof(students[0]);

// ======================================================
// Compare UID
// ======================================================

bool compareUID(byte* scannedUID, byte* storedUID) {

  for (byte i = 0; i < 4; i++) {

    if (scannedUID[i] != storedUID[i]) {
      return false;
    }
  }

  return true;
}

// ======================================================
// Find Student
// ======================================================

int findStudent(byte* scannedUID) {

  for (int i = 0; i < NUM_STUDENTS; i++) {

    if (compareUID(scannedUID, students[i].uid)) {
      return i;
    }
  }

  return -1;
}

// ======================================================
// Connect Wi-Fi
// ======================================================

void connectWiFi() {

  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {

    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi connected.");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("Wi-Fi connection failed.");
  }
}

// ======================================================
// Synchronize Time
// ======================================================

void setupTime() {

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    "pool.ntp.org",
    "time.nist.gov"
  );

  Serial.println("Synchronizing time...");

  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {

    Serial.println("Time synchronized.");

  } else {

    Serial.println("Time synchronization failed.");
  }
}

// ======================================================
// Get Current Time
// ======================================================

String getCurrentTime() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {

    return "TIME ERROR";
  }

  char buffer[25];

  strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d %H:%M:%S",
    &timeinfo
  );

  return String(buffer);
}

// ======================================================
// Load Attendance from NVS
// ======================================================

void loadAttendance() {

  preferences.begin("attendance", true);

  for (int i = 0; i < NUM_STUDENTS; i++) {

    String presentKey =
      "student" + String(i);

    String timeKey =
      "time" + String(i);

    students[i].present =
      preferences.getBool(
        presentKey.c_str(),
        false
      );

    students[i].time =
      preferences.getString(
        timeKey.c_str(),
        ""
      );
  }

  preferences.end();
}

// ======================================================
// Save Attendance to NVS
// ======================================================

void saveAttendance(int studentIndex) {

  preferences.begin("attendance", false);

  String presentKey =
    "student" + String(studentIndex);

  String timeKey =
    "time" + String(studentIndex);

  preferences.putBool(
    presentKey.c_str(),
    true
  );

  preferences.putString(
    timeKey.c_str(),
    students[studentIndex].time
  );

  preferences.end();
}

// ======================================================
// Reset Attendance
// ======================================================

void resetAttendance() {

  Serial.println();
  Serial.println("Resetting attendance...");

  preferences.begin("attendance", false);

  for (int i = 0; i < NUM_STUDENTS; i++) {

    String presentKey =
      "student" + String(i);

    String timeKey =
      "time" + String(i);

    // Clear PRESENT / ABSENT status
    preferences.putBool(
      presentKey.c_str(),
      false
    );

    // Delete timestamp
    preferences.remove(
      timeKey.c_str()
    );

    // Clear RAM
    students[i].present = false;
    students[i].time = "";
  }

  preferences.end();

  Serial.println();
  Serial.println("========================================");
  Serial.println("       ATTENDANCE RESET COMPLETE");
  Serial.println("========================================");

  printAttendance();

  Serial.println("System Ready.");
  Serial.println();
}

// ======================================================
// Attendance Dashboard
// ======================================================

void printAttendance() {

  int presentCount = 0;

  Serial.println();
  Serial.println("========================================");
  Serial.println("        RFID ATTENDANCE SYSTEM");
  Serial.println("========================================");

  Serial.println(
    "Student       Status          Check-in"
  );

  Serial.println(
    "----------------------------------------"
  );

  for (int i = 0; i < NUM_STUDENTS; i++) {

    Serial.print(students[i].name);

    if (String(students[i].name).length() < 12) {
      Serial.print("\t");
    }

    Serial.print("\t");

    if (students[i].present) {

      Serial.print("PRESENT");

      Serial.print("\t\t");

      if (students[i].time.length() > 0) {

        Serial.println(
          students[i].time
        );

      } else {

        Serial.println("--");
      }

      presentCount++;

    } else {

      Serial.println(
        "ABSENT\t\t--"
      );
    }
  }

  Serial.println(
    "----------------------------------------"
  );

  Serial.print("Present: ");
  Serial.print(presentCount);
  Serial.print(" / ");
  Serial.println(NUM_STUDENTS);

  Serial.print("Absent : ");
  Serial.print(NUM_STUDENTS - presentCount);
  Serial.print(" / ");
  Serial.println(NUM_STUDENTS);

  Serial.println(
    "========================================"
  );

  Serial.println();
}

// ======================================================
// Print Card UID
// ======================================================

void printCardUID() {

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(
      rfid.uid.uidByte[i],
      HEX
    );

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
}

// ======================================================
// Successful Card Feedback
// ======================================================

void successFeedback() {

  digitalWrite(
    LED_PIN,
    HIGH
  );

  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(200);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  delay(1000);

  digitalWrite(
    LED_PIN,
    LOW
  );
}

// ======================================================
// Warning Card Feedback
// ======================================================

void warningFeedback() {

  digitalWrite(
    LED_PIN,
    LOW
  );

  // First beep
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(150);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  delay(150);

  // Second beep
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(150);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );
}

// ======================================================
// Setup
// ======================================================

void setup() {

  Serial.begin(115200);

  // Start SPI
  SPI.begin();

  // Start RFID
  rfid.PCD_Init();

  // Configure outputs
  pinMode(
    LED_PIN,
    OUTPUT
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    LED_PIN,
    LOW
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // Connect Wi-Fi
  connectWiFi();

  // Synchronize time
  if (WiFi.status() == WL_CONNECTED) {

    setupTime();
  }

  // Load saved attendance
  loadAttendance();

  // Startup message
  Serial.println();

  Serial.println(
    "========================================"
  );

  Serial.println(
    "        RFID ATTENDANCE SYSTEM"
  );

  Serial.println(
    "========================================"
  );

  Serial.println(
    "Saved attendance loaded."
  );

  printAttendance();

  Serial.println(
    "System Ready."
  );

  Serial.println(
    "Please scan an RFID card."
  );

  Serial.println(
    "Type RESET to clear today's attendance."
  );

  Serial.println();
}

// ======================================================
// Main Loop
// ======================================================

void loop() {

  // ====================================================
  // Serial Command
  // ====================================================

  if (Serial.available()) {

    String command =
      Serial.readStringUntil('\n');

    command.trim();

    command.toUpperCase();

    if (command == "RESET") {

      resetAttendance();
    }
  }

  // ====================================================
  // Check for New RFID Card
  // ====================================================

  if (!rfid.PICC_IsNewCardPresent()) {

    return;
  }

  // ====================================================
  // Read RFID Card
  // ====================================================

  if (!rfid.PICC_ReadCardSerial()) {

    return;
  }

  Serial.println();

  Serial.println(
    "----------------------------------------"
  );

  // Print UID
  printCardUID();

  // ====================================================
  // Identify Student
  // ====================================================

  int studentIndex =
    findStudent(
      rfid.uid.uidByte
    );

  // ====================================================
  // UNKNOWN CARD
  // ====================================================

  if (studentIndex == -1) {

    Serial.println(
      "Student: UNKNOWN"
    );

    Serial.println(
      "Status : ACCESS DENIED"
    );

    warningFeedback();
  }

  // ====================================================
  // REGISTERED STUDENT
  // ====================================================

  else {

    Serial.print(
      "Student: "
    );

    Serial.println(
      students[studentIndex].name
    );

    // ==================================================
    // First Attendance
    // ==================================================

    if (!students[studentIndex].present) {

      students[studentIndex].present =
        true;

      // Get timestamp
      students[studentIndex].time =
        getCurrentTime();

      // Save permanently
      saveAttendance(
        studentIndex
      );

      Serial.println(
        "Status : PRESENT"
      );

      Serial.print(
        "Time   : "
      );

      Serial.println(
        students[studentIndex].time
      );

      Serial.println(
        "Attendance saved to ESP32."
      );

      successFeedback();
    }

    // ==================================================
    // Already Recorded
    // ==================================================

    else {

      Serial.println(
        "Status : ALREADY RECORDED"
      );

      Serial.print(
        "Original time: "
      );

      Serial.println(
        students[studentIndex].time
      );

      warningFeedback();
    }
  }

  // ====================================================
  // Updated Dashboard
  // ====================================================

  printAttendance();

  // ====================================================
  // Stop RFID Communication
  // ====================================================

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();

  delay(500);
}