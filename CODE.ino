// Smart Irrigation System - ESP32 + YL-69 (soil moisture) + Relay + Bluetooth
// Upload with Arduino IDE (board: ESP32 Dev Module)
// Adjust pins and calibration constants below as needed.

#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

// --------- User-configurable pins & settings ----------
const int MOISTURE_PIN = 34;        // analog pin (ADC1) connected to YL-69 analog output
const int RELAY_PIN    = 26;        // digital pin driving relay module (or transistor)
const bool RELAY_ACTIVE_LOW = true; // true if relay turns ON when pin is LOW (common relay modules)

const int SAMPLES = 10;             // number of samples to average
const unsigned long SEND_INTERVAL_MS = 2000UL; // how often to send data over BT and Serial

// Calibration: raw ADC values (0 - 4095 for ESP32 ADC)
const int RAW_WET  = 1000; // raw ADC when sensor is in water / very wet soil (adjust after calibration)
const int RAW_DRY  = 3000; // raw ADC when sensor is in dry soil (adjust after calibration)

// Pump control thresholds (percent)
int THRESHOLD_PERCENT = 35; // below this percent -> start watering
const int HYSTERESIS_PERCENT = 5; // hysteresis to avoid rapid switching

// -----------------------------------------------------

// Internal state
unsigned long lastSend = 0;
bool pumpOn = false;

void setPump(bool on) {
  if (RELAY_ACTIVE_LOW) digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  else                digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  pumpOn = on;
}

int readSoilRawAvg() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; ++i) {
    int v = analogRead(MOISTURE_PIN);
    sum += v;
    delay(10);
  }
  return (int)(sum / SAMPLES);
}

int rawToPercent(int raw) {
  // Map raw ADC to 0-100% (0 = completely dry, 100 = completely wet)
  // Note: RAW_WET should be < RAW_DRY for typical YL-69 behavior; clamp afterwards.
  int pct = map(raw, RAW_DRY, RAW_WET, 0, 100);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  if (RELAY_ACTIVE_LOW) digitalWrite(RELAY_PIN, HIGH); // turn pump off initially
  else                  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  delay(100);
  SerialBT.begin("ESP32_Irrigation"); // Bluetooth device name

  // Optional: print header
  Serial.printf("Smart Irrigation started. BT name: %s\n", "ESP32_Irrigation");
  Serial.printf("Moisture pin: %d, Relay pin: %d\n", MOISTURE_PIN, RELAY_PIN);
}

void loop() {
  unsigned long now = millis();

  int raw = readSoilRawAvg();
  int moisturePercent = rawToPercent(raw);

  // Pump control with hysteresis:
  if (!pumpOn && moisturePercent <= THRESHOLD_PERCENT) {
    setPump(true);
  } else if (pumpOn && moisturePercent >= (THRESHOLD_PERCENT + HYSTERESIS_PERCENT)) {
    setPump(false);
  }

  // Send telemetry periodically
  if (now - lastSend >= SEND_INTERVAL_MS) {
    lastSend = now;

    // JSON-like telemetry
    String payload = "{";
    payload += "\"moisture_pct\":" + String(moisturePercent) + ",";
    payload += "\"raw\":" + String(raw) + ",";
    payload += "\"pump\":\"" + String(pumpOn ? "ON" : "OFF") + "\"";
    payload += "}";

    // Send to Serial and Bluetooth
    Serial.println(payload);
    if (SerialBT.hasClient()) SerialBT.println(payload);
  }

  delay(100);
}
