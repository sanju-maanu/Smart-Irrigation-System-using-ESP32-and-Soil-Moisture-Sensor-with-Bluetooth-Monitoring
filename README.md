# 💧 Smart Irrigation System using ESP32 and Soil Moisture Sensor

An **IoT-based Smart Irrigation System** that automatically waters plants using an **ESP32**, **YL-69 soil moisture sensor**, and a **relay-controlled water pump**.  
The system transmits real-time soil moisture data to a **mobile device via Bluetooth**, enabling efficient water usage and smart automation.

---

## 🚀 Features
- 🌿 Automatic pump control based on soil moisture
- 📲 Real-time Bluetooth monitoring on mobile
- ⚙️ Simple ESP32 + YL-69 integration
- 🔋 Low power and low cost
- 🧠 Scalable for IoT and smart farming applications

---

## 🧩 Components Used
| Component | Description |
|------------|-------------|
| **ESP32** | Main microcontroller with built-in Bluetooth |
| **YL-69 Soil Moisture Sensor** | Measures soil moisture level |
| **Relay Module** | Controls the water pump |
| **Water Pump** | Provides water to plants |
| **Power Supply** | 5V/12V (depending on your setup) |
| **Jumper Wires & Breadboard** | For circuit connections |

---

## ⚡ Circuit Connections
| ESP32 Pin | Component | Function |
|------------|------------|-----------|
| GPIO 34 | YL-69 Analog Output | Reads soil moisture |
| GPIO 26 | Relay IN Pin | Controls water pump |
| 5V & GND | YL-69 + Relay | Power supply |
| Built-in Bluetooth | Smartphone | Data communication |

*(Refer to the circuit diagram in the `images/` folder)*

---

## 🧠 Working Principle
1. The YL-69 sensor measures soil moisture and sends an analog signal to ESP32.  
2. ESP32 reads the value and compares it with a set threshold.  
3. If soil is dry → Pump turns **ON**.  
4. If soil is moist → Pump turns **OFF**.  
5. The current moisture value and pump status are sent to the mobile via Bluetooth.

---

## 💻 Arduino Code
Upload the following code using **Arduino IDE** (select Board: ESP32 Dev Module):

```cpp
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

const int MOISTURE_PIN = 34;
const int RELAY_PIN = 26;
const bool RELAY_ACTIVE_LOW = true;

const int RAW_WET = 1000;
const int RAW_DRY = 3000;
int THRESHOLD_PERCENT = 35;

bool pumpOn = false;
unsigned long lastSend = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  if (RELAY_ACTIVE_LOW) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  SerialBT.begin("ESP32_Irrigation");
  Serial.println("Smart Irrigation System Started!");
}

void loop() {
  int raw = analogRead(MOISTURE_PIN);
  int moisturePercent = map(raw, RAW_DRY, RAW_WET, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  if (!pumpOn && moisturePercent < THRESHOLD_PERCENT) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    pumpOn = true;
  } else if (pumpOn && moisturePercent > (THRESHOLD_PERCENT + 5)) {
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    pumpOn = false;
  }

  if (millis() - lastSend > 2000) {
    lastSend = millis();
    Serial.printf("Moisture: %d%% | Pump: %s\n", moisturePercent, pumpOn ? "ON" : "OFF");
    SerialBT.printf("Moisture: %d%% | Pump: %s\n", moisturePercent, pumpOn ? "ON" : "OFF");
  }
}
