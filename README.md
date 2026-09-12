# RFID Attendance System

An ESP32-based RFID attendance system developed to automate roll calls for my Gym Club.

## Overview

As a Gym Club leader, I needed a more efficient way to record attendance because members often arrived at different times. I designed and built an RFID-based system using an ESP32 and an MFRC522 reader.

## Features

- RFID UID-based student identification
- Attendance tracking for five registered students
- Unknown-card rejection
- Duplicate check-in detection
- NTP-based timestamps
- Persistent attendance storage using ESP32 NVS
- Attendance recovery after ESP32 restart
- Real-time attendance dashboard
- Daily attendance reset
- LED and buzzer feedback

## Hardware

- ESP32 development board
- MFRC522 RFID reader
- MIFARE RFID cards
- LED
- 220Ω resistor
- Buzzer
- Breadboard
- Jumper wires

## System Architecture

```text
RFID Card
    ↓
MFRC522
    ↓ SPI
ESP32
    ├── UID Authentication
    ├── Attendance Logic
    ├── NVS Storage
    └── Wi-Fi / NTP
            ↓
        Timestamp

Attendance Logic

A registered card is identified using its RFID UID.

First scan → PRESENT
Repeated scan → ALREADY RECORDED
Unregistered card → ACCESS DENIED

Attendance status and timestamps are stored in the ESP32's NVS flash memory, allowing records to persist after restart.

Time Synchronization
The ESP32 connects to Wi-Fi and synchronizes its clock using NTP. Each first check-in receives a timestamp.

Reset
To start a new attendance session, enter: reset

author
Wenji Cheng
