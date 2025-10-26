#include <EEPROM.h> //thu vien luu tru trang thai

const int PIR_PIN = 7;                //PIR
const int LS_PIN = A0;                //LS
const int BUTTON_PIN = 2;             //Bt
const int LED_PIN = 8;                //relay bat led
const int INDICATOR_LED = 9;          //mode led

#define ADDR_MODE 0                   //bien cho mode
#define ADDR_OPTION 1                 //thoi gian

int buttonState = HIGH;
int lastButtonState = HIGH;

bool lightOn = false;
bool alwaysOn = false;
bool alwaysOff = false;

unsigned long lightStartTime = 0;
unsigned long lightDuration = 10000;

unsigned long timeOptions[] = {10000, 20000, 100000};             //ms
int currentOption = 0;

void saveSettings() {                                           //Luu settings
    byte mode = 0;
    if (alwaysOn) mode = 1;
    else if (alwaysOff) mode = 2;
    EEPROM.update(ADDR_MODE, mode);
    EEPROM.update(ADDR_OPTION, currentOption);
}

void loadSettings() {                                           //Tai lai settings
    byte savedMode = EEPROM.read(ADDR_MODE);
    currentOption = EEPROM.read(ADDR_OPTION);
    if (currentOption > 2) currentOption = 0;
    lightDuration = timeOptions[currentOption];

    switch (savedMode) {
        case 1: alwaysOn = true;  alwaysOff = false; break;
        case 2: alwaysOn = false; alwaysOff = true;  break;
        default: alwaysOn = false; alwaysOff = false; break;
    }
}

void setup() {
    pinMode(LS_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(INDICATOR_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.begin(9600);
    loadSettings();

    Serial.println("=== HE THONG BAT DEN TU DONG ===");
    Serial.print("Thoi gian sang: "); Serial.print(lightDuration / 1000); Serial.println("s");
    if (alwaysOn) Serial.println("Che do: Luon sang");
    else if (alwaysOff) Serial.println("Che do: Luon tat");
    else Serial.println("Che do: Binh thuong");
    Serial.println("--------------------------------");
}

void blinkIndicatorLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(INDICATOR_LED, HIGH);
        delay(200);
        digitalWrite(INDICATOR_LED, LOW);
        delay(200);
    }
}

void checkButton() {
    static unsigned long buttonPressStart = 0;
    unsigned long currentTime = millis();
    buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW) {
        if (buttonPressStart == 0)
            buttonPressStart = currentTime;

        if ((currentTime - buttonPressStart) > 2000) {
            if (!alwaysOn && !alwaysOff) {
                alwaysOn = true;
                alwaysOff = false;
                Serial.println("Che do: Luon sang");
                digitalWrite(INDICATOR_LED, HIGH);
            } else if (alwaysOn) {
                alwaysOn = false;
                alwaysOff = true;
                Serial.println("Che do: Luon tat");
                digitalWrite(INDICATOR_LED, LOW);
            }
            saveSettings();
            buttonPressStart = 0;
            return;
        }
    } else {
        if (buttonPressStart > 0 && (currentTime - buttonPressStart) <= 2000) {
            buttonPressStart = 0;

            if (alwaysOn || alwaysOff) {
                alwaysOn = false;
                alwaysOff = false;
                Serial.println("Che do: Binh thuong");
                blinkIndicatorLED(currentOption + 1);
            } else {
                currentOption = (currentOption + 1) % 3;
                lightDuration = timeOptions[currentOption];
                Serial.print("Thoi gian sang moi: ");
                Serial.print(lightDuration / 1000);
                Serial.println("s");
                blinkIndicatorLED(currentOption + 1);
            }
            saveSettings();
        }
    }
}

void loop() {
    int PIR = digitalRead(PIR_PIN);
    int lightValue = analogRead(LS_PIN);
    int LS = (lightValue < 500) ? HIGH : LOW;

    checkButton();

    unsigned long currentMillis = millis();

    if (alwaysOff) {
        digitalWrite(LED_PIN, LOW);
        lightOn = false;
        return;
    }

    if (alwaysOn) {
        digitalWrite(LED_PIN, HIGH);
        return;
    }

    if (PIR == HIGH && LS == HIGH && !lightOn) {
        Serial.println("Den bat");
        digitalWrite(LED_PIN, HIGH);
        lightOn = true;
        lightStartTime = currentMillis;
    }

    if (lightOn && (currentMillis - lightStartTime >= lightDuration)) {
        Serial.println("Den tat");
        digitalWrite(LED_PIN, LOW);
        lightOn = false;
    }
}
