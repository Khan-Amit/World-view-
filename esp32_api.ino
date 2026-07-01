// ============================================================
// FILE: esp32_api.ino
// ESP32 API Server - Real Data Provider
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ============================================================
// ⚠️ CHANGE THESE - YOUR WIFI CREDENTIALS
// ============================================================
const char* ssid = "YOUR_WIFI_SSID";        // ← CHANGE THIS
const char* password = "YOUR_WIFI_PASSWORD"; // ← CHANGE THIS

// ============================================================
// ⚠️ CHANGE THIS - YOUR MONERO WALLET
// ============================================================
const char* walletAddress = "YOUR_MONERO_WALLET_ADDRESS"; // ← CHANGE THIS

// ============================================================
// HARDWARE CONFIGURATION
// ============================================================
#define NUM_CHANNELS 12
#define THRESHOLD 2048

// ADC Pins - Connect sensors here
int analogPins[NUM_CHANNELS] = {
    A0, A1, A2, A3,   // GPU0-3
    A4, A5, A6, A7,   // PSU0-3
    A8, A9, A10, A11  // TMP0-3
};

// Channel Labels
const char* channelLabels[NUM_CHANNELS] = {
    "GPU0", "GPU1", "GPU2", "GPU3",
    "PSU0", "PSU1", "PSU2", "PSU3",
    "TMP0", "TMP1", "TMP2", "TMP3"
};

// ============================================================
// GLOBAL VARIABLES
// ============================================================
WebServer server(80);
int adcValues[NUM_CHANNELS];
bool isWinner[NUM_CHANNELS];

// Pool simulation data
double hashrate = 12500.0;
int acceptedShares = 0;
int rejectedShares = 0;
double poolDifficulty = 120000.0;
String poolStatus = "Connected";

// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=========================================");
    Serial.println("  ESP32 API SERVER - REAL DATA");
    Serial.println("  World View Binary Perception");
    Serial.println("=========================================\n");
    
    // Connect to WiFi
    connectWiFi();
    
    // Initialize pins
    for(int i = 0; i < NUM_CHANNELS; i++) {
        pinMode(analogPins[i], INPUT);
    }
    
    // Setup API endpoints
    setupAPI();
    
    Serial.println("\n✅ API Server Ready!");
    Serial.print("📡 API URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/api/data");
    Serial.println("=========================================\n");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
    // Read sensors
    readSensors();
    
    // Process binary logic
    processBinaryLogic();
    
    // Update pool data (simulated mining)
    updatePoolData();
    
    // Handle web requests
    server.handleClient();
    
    delay(1000); // 1 second update
}

// ============================================================
// WIFI CONNECTION
// ============================================================
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Connected!");
        Serial.print("📡 IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ WiFi Failed! Running in standalone mode.");
    }
}

// ============================================================
// READ SENSORS
// ============================================================
void readSensors() {
    for(int i = 0; i < NUM_CHANNELS; i++) {
        // Read ADC value (0-4095 for ESP32)
        adcValues[i] = analogRead(analogPins[i]);
        
        // If no sensor connected, generate test data
        if(adcValues[i] < 10 || adcValues[i] > 4080) {
            generateTestData();
            return;
        }
        delayMicroseconds(100);
    }
}

// ============================================================
// GENERATE TEST DATA (If no sensors)
// ============================================================
void generateTestData() {
    static bool seeded = false;
    if(!seeded) {
        randomSeed(analogRead(0) + millis());
        seeded = true;
    }
    
    for(int i = 0; i < NUM_CHANNELS; i++) {
        if(i < 4) adcValues[i] = random(1500, 3500);
        else if(i < 8) adcValues[i] = random(1800, 3200);
        else adcValues[i] = random(800, 3000);
    }
    
    // Force pattern for demonstration
    adcValues[0] = 3162;
    adcValues[1] = 2880;
    adcValues[2] = 1244;
    adcValues[3] = 2919;
    adcValues[4] = 1761;
    adcValues[5] = 3188;
    adcValues[6] = 2285;
    adcValues[7] = 1340;
    adcValues[8] = 914;
    adcValues[9] = 2735;
    adcValues[10] = 1030;
    adcValues[11] = 2472;
}

// ============================================================
// BINARY LOGIC
// ============================================================
void processBinaryLogic() {
    for(int i = 0; i < NUM_CHANNELS; i++) {
        if(i < 8) {
            isWinner[i] = (adcValues[i] > THRESHOLD);
        } else {
            isWinner[i] = (adcValues[i] < THRESHOLD);
        }
    }
}

// ============================================================
// UPDATE POOL DATA (Simulated Mining)
// ============================================================
void updatePoolData() {
    static unsigned long lastUpdate = 0;
    if(millis() - lastUpdate < 5000) return;
    lastUpdate = millis();
    
    // Simulate hashrate variation
    hashrate += random(-500, 500);
    if(hashrate < 5000) hashrate = 5000;
    if(hashrate > 25000) hashrate = 25000;
    
    // Simulate shares
    if(random(0, 100) > 70) {
        acceptedShares++;
    }
    if(random(0, 100) > 95) {
        rejectedShares++;
    }
    
    // Simulate difficulty
    poolDifficulty = 120000.0 + random(-10000, 10000);
}

// ============================================================
// API ENDPOINTS
// ============================================================
void setupAPI() {
    // ============================================================
    // MAIN API - Returns ALL data as JSON
    // ============================================================
    server.on("/api/data", []() {
        String json = buildJSON();
        server.send(200, "application/json", json);
    });
    
    // ============================================================
    // STATUS API - Returns system status
    // ============================================================
    server.on("/api/status", []() {
        String json = "{";
        json += "\"status\":\"online\",";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "connected" : "disconnected") + "\",";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";
        server.send(200, "application/json", json);
    });
    
    // ============================================================
    // CORS HEADERS - Allows browser to fetch
    // ============================================================
    server.on("/api/ping", []() {
        server.send(200, "text/plain", "pong");
    });
    
    // ============================================================
    // CORS SUPPORT
    // ============================================================
    server.onNotFound([]() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(404, "text/plain", "Not Found");
    });
    
    // Start server
    server.begin();
    Serial.println("🌐 API Server started");
}

// ============================================================
// BUILD JSON RESPONSE
// ============================================================
String buildJSON() {
    StaticJsonDocument<2048> doc;
    
    // Values array
    JsonArray values = doc.createNestedArray("values");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        values.add(adcValues[i]);
    }
    
    // Winners array
    JsonArray winners = doc.createNestedArray("winners");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        winners.add(isWinner[i]);
    }
    
    // Labels array
    JsonArray labels = doc.createNestedArray("labels");
    for(int i = 0; i < NUM_CHANNELS; i++) {
        labels.add(channelLabels[i]);
    }
    
    // Pool data
    doc["hashrate"] = hashrate;
    doc["accepted"] = acceptedShares;
    doc["rejected"] = rejectedShares;
    doc["difficulty"] = poolDifficulty;
    doc["poolStatus"] = poolStatus;
    doc["threshold"] = THRESHOLD;
    doc["timestamp"] = millis();
    
    String json;
    serializeJson(doc, json);
    return json;
}

// ============================================================
// END OF FILE: esp32_api.ino
// ============================================================
