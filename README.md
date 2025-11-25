# Solar Tracker (Dual-Axis) - ESP32 + 4 LDRs + 2 Servos

**A simple dual-axis solar tracker** using an ESP32, four LDRs, and two SG90 servos. The tracker continuously adjusts pan and tilt to point a small solar panel or mock panel toward the brightest direction. Includes a lightweight web telemetry page.

## Features
- Dual-axis tracking (pan + tilt)
- 4-quadrant LDR sensing for directional light
- Moving-average smoothing and deadzone to prevent jitter
- Simple web telemetry (angles and raw LDR readings)
- Single-file Arduino sketch (ESP32)

## Hardware (BOM)
- ESP32 DevKit (1)
- Micro servo SG90 (2)
- LDR / Photoresistors (4)
- 10kΩ resistors (4)
- 5V power supply for servos (recommended separate)
- Breadboard, jumper wires, frame for mounting

## Wiring (example)
- LDR_TL -> ADC1_6 (GPIO34)
- LDR_TR -> ADC1_7 (GPIO35)
- LDR_BL -> ADC1_4 (GPIO32)
- LDR_BR -> ADC1_5 (GPIO33)
- PAN_SERVO -> GPIO18
- TILT_SERVO -> GPIO19
- Servos Vcc -> 5V, Servos GND -> common GND with ESP32

> Adjust pins in code if your board uses different ADC pins.

## Usage
1. Clone repo.
2. Open `solar_tracker.ino` (or `main.cpp`) in Arduino IDE or PlatformIO.
3. Connect ESP32, select board and correct port.
4. Upload code.
5. Power servos with a stable 5V supply. Connect grounds together.
6. Monitor serial output (115200) for telemetry or open `http://<esp32-ip>/` if using network features.

## Tuning tips
- Increase `SMOOTHING` to stabilize noisy readings.
- Increase `DEADZONE` to avoid micro-movements.
- Tune sensitivity divisor (`/400` in code) to get desired movement speed.
- Adjust `PAN_MIN`, `PAN_MAX`, `TILT_MIN`, `TILT_MAX` for mechanical limits.

## Improvements & next steps
- Replace servos with stepper motors + motor drivers for larger panels.
- Add sunrise/sunset scheduling (use RTC or NTP).
- Add solar power measurement and logging to cloud (MQTT / ThingsBoard).
- Add PID control for smoother tracking.
- Add failsafe stow position for high wind (wind sensor) or night detection.

## License
MIT
