
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "InfinitePCA9685.h"

// 📶 Configura el WiFi com a Punt d'Accés
const char* ap_ssid = "MA_ROBOTICA";
const char* ap_password = "12345678";

// I2C: Adreces i servos
std::vector<uint8_t> pcaAddresses = {0x40};
MultiPCA9685 manyPCA9685s(Wire, pcaAddresses, 50);
// MaRobotica ma(manyPCA9685s); // 👉 Passar la instància global correctament

AsyncWebServer server(80);


const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Mà Robòtica</title>
</head>
<body>
  <h2>Mou un dit</h2>
  <form action="/mou" method="GET">
    <label for="servo">Selecciona un dit (0-4):</label>
    <select id="servo" name="servo">
      <option value="0">Dit Polze</option>
      <option value="1">Dit Índex</option>
      <option value="2">Dit Cor</option>
      <option value="3">Dit Anular</option>
      <option value="4">Dit Petit</option>
    </select><br><br>

    <label for="angle">Angle (0-180):</label>
    <input type="number" id="angle" name="angle" min="0" max="180"><br><br>

    <input type="submit" value="Moure">
  </form>
</body>
</html>
)rawliteral";

// ⚙️ Conversió d’angle a PWM
uint16_t angleAServoPWM(uint8_t angle) {
    const uint16_t SERVOMIN = 50;
    const uint16_t SERVOMAX = 400;
    return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setup() {
    Serial.begin(115200);

    // Inicialitza el bus I2C (ajusta pins si cal)
    Wire.begin(21, 20); // SDA, SCL per ESP32-S3 (ajusta segons la teva placa)
    
    manyPCA9685s.begin();
    //manyPCA9685s.toggleDebug(); // Activa si vols veure logs
    manyPCA9685s.getSetup();

    // ma.inicialitzar();

    // 🔌 Punt d'accés
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress IP = WiFi.softAPIP();
    Serial.println("📡 Punt d'accés creat!");
    Serial.print("🌍 IP del servidor: ");
    Serial.println(IP);

    // Serveix la pàgina web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", htmlPage);
    });

    // Mou el servo
    server.on("/mou", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("servo") && request->hasParam("angle")) {
            int dit = request->getParam("servo")->value().toInt();
            int angle = request->getParam("angle")->value().toInt();

            uint16_t pwm = angleAServoPWM(angle);
            Serial.printf("📥 Dit %d → Angle %d°\n", dit, angle);
            manyPCA9685s.setPWM(dit, pwm);

            request->send(200, "text/html", "<p>Servo mogut! <a href='/'>Tornar</a></p>");
        } else {
            request->send(400, "text/plain", "Falten paràmetres.");
        }
    });

    server.begin();
    Serial.println("✅ Servidor web actiu al port 80");
}

void loop() {
    // Tot gestionat pel AsyncWebServer
}

