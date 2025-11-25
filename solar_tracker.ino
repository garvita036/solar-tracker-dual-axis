
#include <Arduino.h>
#include <ESPAsyncWebServer.h> // optional - used only if web telemetry is enabled
#include <WebServer.h>
#include <Servo.h>

// ---------- CONFIG ----------
const int LDR_TL = 34;
const int LDR_TR = 35;
const int LDR_BL = 32;
const int LDR_BR = 33;

const int PAN_SERVO_PIN = 18;
const int TILT_SERVO_PIN = 19;

const int PAN_MIN = 0;     // degrees
const int PAN_MAX = 180;
const int TILT_MIN = 0;
const int TILT_MAX = 90;

const int UPDATE_DELAY_MS = 300;   // sensor read frequency
const int SMOOTHING = 4;           // moving average length for readings
const int DEADZONE = 30;           // LDR difference threshold to cause movement

// ---------- GLOBAL ----------
Servo panServo;
Servo tiltServo;

int panAngle = 90;   // start centered
int tiltAngle = 45;

unsigned long lastUpdate = 0;

// simple circular buffer smoothing
int bufTL[SMOOTHING];
int bufTR[SMOOTHING];
int bufBL[SMOOTHING];
int bufBR[SMOOTHING];
int bi = 0;

WebServer server(80); // simple server for telemetry (optional)

// ---------- UTIL -----------
int readSmooth(int pin, int *buf) {
  int raw = analogRead(pin);
  buf[bi] = raw;
  long sum = 0;
  for (int i = 0; i < SMOOTHING; ++i) sum += buf[i];
  return sum / SMOOTHING;
}

void clampAngles() {
  if (panAngle < PAN_MIN) panAngle = PAN_MIN;
  if (panAngle > PAN_MAX) panAngle = PAN_MAX;
  if (tiltAngle < TILT_MIN) tiltAngle = TILT_MIN;
  if (tiltAngle > TILT_MAX) tiltAngle = TILT_MAX;
}

String htmlTelemetry(int tl, int tr, int bl, int br) {
  String s = "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'/><title>Solar Tracker</title></head><body>";
  s += "<h3>Solar Tracker Telemetry</h3>";
  s += "<p>Pan angle: " + String(panAngle) + " &deg;</p>";
  s += "<p>Tilt angle: " + String(tiltAngle) + " &deg;</p>";
  s += "<table border='1' cellpadding='6'><tr><th>TL</th><th>TR</th><th>BL</th><th>BR</th></tr>";
  s += "<tr><td>" + String(tl) + "</td><td>" + String(tr) + "</td><td>" + String(bl) + "</td><td>" + String(br) + "</td></tr></table>";
  s += "<p>Last updated: " + String(millis()/1000) + " s</p>";
  s += "</body></html>";
  return s;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(100);

  // initialize smoothing buffers with initial readings
  for (int i = 0; i < SMOOTHING; ++i) {
    bufTL[i] = analogRead(LDR_TL);
    bufTR[i] = analogRead(LDR_TR);
    bufBL[i] = analogRead(LDR_BL);
    bufBR[i] = analogRead(LDR_BR);
  }

  panServo.attach(PAN_SERVO_PIN);
  tiltServo.attach(TILT_SERVO_PIN);
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);

  // Optional: lightweight web server endpoint
  server.on("/", []() {
    int tl = readSmooth(LDR_TL, bufTL);
    int tr = readSmooth(LDR_TR, bufTR);
    int bl = readSmooth(LDR_BL, bufBL);
    int br = readSmooth(LDR_BR, bufBR);
    server.send(200, "text/html", htmlTelemetry(tl, tr, bl, br));
  });
  server.begin();

  Serial.println("Solar Tracker started.");
}

// ---------- MAIN LOOP ----------
void loop() {
  // handle http requests
  server.handleClient();

  unsigned long now = millis();
  if (now - lastUpdate < UPDATE_DELAY_MS) return;
  lastUpdate = now;

  bi = (bi + 1) % SMOOTHING;
  int tl = readSmooth(LDR_TL, bufTL);
  int tr = readSmooth(LDR_TR, bufTR);
  int bl = readSmooth(LDR_BL, bufBL);
  int br = readSmooth(LDR_BR, bufBR);

  // compute horizontal (left-right) and vertical (top-bottom) biases
  long leftSum = (long)tl + bl;
  long rightSum = (long)tr + br;
  long topSum = (long)tl + tr;
  long bottomSum = (long)bl + br;

  long lrDiff = leftSum - rightSum;   // positive => more light on left
  long tbDiff = topSum - bottomSum;   // positive => more light on top

  // Debug prints
  Serial.printf("TL:%d TR:%d BL:%d BR:%d | L-R:%ld T-B:%ld | pan:%d tilt:%d\n",
                tl, tr, bl, br, lrDiff, tbDiff, panAngle, tiltAngle);

  // If difference is below deadzone threshold, do nothing (prevent jitter)
  if (abs(lrDiff) > DEADZONE) {
    // scale difference to a small angle change
    int delta = constrain((int)(lrDiff / 400), -3, 3); // adjust divisor for sensitivity
    panAngle += (delta); // positive lrDiff moves left (depends on servo orientation)
  }

  if (abs(tbDiff) > DEADZONE) {
    int delta = constrain((int)(tbDiff / 400), -3, 3);
    // topSum > bottomSum => tbDiff positive => more light on top => tilt up (decrease angle) or vice versa
    tiltAngle -= (delta);
  }

  clampAngles();

  // write to servos smoothly by small increments to avoid sudden jumps
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);
}
