#include <SonosUPnP.h>
#include <WiFi.h>
#include <M5EPD.h>
#include <M5GFX.h>

M5GFX display;
const int Abstand = 150;

struct BtnProvider
{
  LGFX_Button Button;
  int Spalte;
  int Zeile;
};

BtnProvider Provider [12];

void ConnectError();

WiFiClient client;
SonosUPnP sonos = SonosUPnP(client, ConnectError);

#define SERIAL_DATA_THRESHOLD_MS 500
#define SERIAL_ERROR_TIMEOUT "E: Serial"
#define ETHERNET_ERROR_DHCP "E: DHCP"
#define ETHERNET_ERROR_CONNECT "E: Connect"
#define HTTPPORT 88


const char* ssid     = "FRITZ!Box 6490 Cable 2,4";
const char* passwort = "ak42sh42";

IPAddress KuecheIP(192, 168, 178, 51);
const char KuecheID[] = "5CAAFD2C18A8";


int Spalte1 = 15, Spalte2 = 195, Spalte3 = 375, Zeile1 = 30, 
Zeile2 = 210, Zeile3 = 390, Zeile4 = 570, Zeile5 = 750, Zeile6 = 930;

int bat;
float temp,  feucht;

void setup()
{
  InitializeHardwareModul();
  InitializeWiFiAndDisplay();
  InitializeButtonProvider();
}

void loop()
{
  delay(200);
  ButtonStatus(Provider);
  updateSensorAndTp();
  displaySensor();
  pressStatus(Provider);
}

void ConnectError()
{
  Serial.println(ETHERNET_ERROR_CONNECT);
  Serial.println("Verbindung abgebrochen.");
}

void pressStatus(BtnProvider Provider[])
{
  int currentVolume;

  if (Provider[0].Button.isPressed())
    sonos.play(KuecheIP);

  if (Provider[1].Button.isPressed())
    sonos.pause(KuecheIP);

  if (Provider[2].Button.isPressed())
    sonos.stop(KuecheIP);

  if (Provider[3].Button.isPressed())
    sonos.skip(KuecheIP, SONOS_DIRECTION_FORWARD);

  if (Provider[4].Button.isPressed())
    sonos.skip(KuecheIP, SONOS_DIRECTION_BACKWARD);

  if (Provider[5].Button.isPressed())
    sonos.setPlayMode(KuecheIP, SONOS_PLAY_MODE_SHUFFLE);

  if (Provider[6].Button.isPressed())
    sonos.setPlayMode(KuecheIP, SONOS_PLAY_MODE_REPEAT);

  if (Provider[7].Button.isPressed())
    sonos.setPlayMode(KuecheIP, SONOS_PLAY_MODE_NORMAL);

  if (Provider[8].Button.isPressed())
    sonos.setMute(KuecheIP, true);

  if (Provider[9].Button.isPressed() && currentVolume <= 90)
  {
    currentVolume = sonos.getVolume(KuecheIP);
    sonos.setVolume(KuecheIP, sonos.getVolume(KuecheIP) + 10);
  }
  if (Provider[10].Button.isPressed() && currentVolume >= 10)
  {
    currentVolume = sonos.getVolume(KuecheIP);
    sonos.setVolume(KuecheIP, sonos.getVolume(KuecheIP) - 10);
  }

  if (currentVolume > 100)
    currentVolume = 100;
  if (currentVolume < 0)
    currentVolume = 0;
  if (Provider[11].Button.isPressed())
    sonos.setMute(KuecheIP, false);

}
void displaySensor()
  {
  int currentVolume=sonos.getVolume(KuecheIP);
   display.setCursor(5, 800);
  display.printf("Volume: %u \n Batteriespannung: %u\n Temperatur: %2.2f*C \n Feuchtigkeit: %0.2f%%", 
  currentVolume, bat, temp, feucht);
  
  display.display();
  }

void ButtonStatus(BtnProvider Provider[])
{
  for (int i = 0; i < sizeof(Provider); i++)
  {
    Provider[i].Button.press(false);
  }
}

void InitializeHardwareModul()
{
  M5.begin();
  M5.BatteryADCBegin();
}

void updateSensorAndTp()
{
  lgfx::touch_point_t tp[1];
  M5.SHT30.UpdateData();
  bat = M5.getBatteryVoltage();
  temp = M5.SHT30.GetTemperature();
  feucht = M5.SHT30.GetRelHumidity();
  int touchOn = display.getTouchRaw(tp, 4); // touchOn = 1 wenn Touch-Eingabe, sonst 0, tp wird hier mit den Touch-Koordinaten gefüllt
  PressIfInRange(tp[0].x, tp[0].y,Provider);
}

void InitializeWiFiAndDisplay()
{
  display.init();
  display.setFont(&fonts::Font4);
  display.setEpdMode(epd_mode_t::epd_fast);

  WiFi.begin(ssid, passwort);

  while (WiFi.status() != WL_CONNECTED)
  {
    int i = 0;
    display.setCursor(10, 10);
    display.printf("Verbindungsaufbau");
    display.setCursor(0 + i, 30);
    display.printf(".");
    display.display();
    i += 10;
    delay(500);
    WiFi.begin(ssid, passwort);
  }

  display.clear();
  display.setCursor(10, 10);
  display.printf("Verbunden mit dem WLAN-Netz");
  delay(1000);
  display.clear();
  display.setCursor(200, 800);
}

BtnProvider * InitializeButtonProvider()
{

  const char* namen [] = {"Play", "Pause", "Skip <", "Skip >", "Stopp", "Repeat", 
  "Shuffle", "Normal", "Mute", "Volume+", "Volume-", "Unmute"};

  BtnProvider playProvider;
  playProvider.Spalte = Spalte1;
  playProvider.Zeile = Zeile1;
  Provider[0] = playProvider;

  BtnProvider pauseProvider;
  pauseProvider.Spalte = Spalte2;
  pauseProvider.Zeile = Zeile1;
  Provider[1] = pauseProvider;

  BtnProvider stoppProvider;
  stoppProvider.Spalte = Spalte3;
  stoppProvider.Zeile = Zeile1;
  Provider[2] = stoppProvider;

  BtnProvider skipVProvider;
  skipVProvider.Spalte = Spalte1;
  skipVProvider.Zeile = Zeile2;
  Provider[3] = skipVProvider;

  BtnProvider skipRProvider;
  skipRProvider.Spalte = Spalte2;
  skipRProvider.Zeile = Zeile2;
  Provider[4] = skipRProvider;

  BtnProvider shuffleProvider;
  shuffleProvider.Spalte = Spalte3;
  shuffleProvider.Zeile = Zeile2;
  Provider[5] = shuffleProvider;

  BtnProvider repeatProvider;
  repeatProvider.Spalte = Spalte1;
  repeatProvider.Zeile = Zeile3;
  Provider[6] = repeatProvider;

  BtnProvider normalProvider;
  normalProvider.Spalte = Spalte2;
  normalProvider.Zeile = Zeile3;
  Provider[7] = normalProvider;

  BtnProvider muteProvider;
  muteProvider.Spalte = Spalte3;
  muteProvider.Zeile = Zeile3;
  Provider[8] = muteProvider;

  BtnProvider VolumePProvider;
  VolumePProvider.Spalte = Spalte1;
  VolumePProvider.Zeile = Zeile4;
  Provider[9] = VolumePProvider;

  BtnProvider VolumeMProvider;
  VolumeMProvider.Spalte = Spalte2;
  VolumeMProvider.Zeile = Zeile4;
  Provider[10] = VolumeMProvider;

  BtnProvider unmuteProvider;
  unmuteProvider.Spalte = Spalte3;
  unmuteProvider.Zeile = Zeile4;
  Provider[11] = unmuteProvider;

  for (int i = 0; i < sizeof(Provider); i++)
  {
    Provider[i].Button.initButtonUL
    (&display, Provider[i].Spalte, Provider[i].Zeile, Abstand, Abstand, 
    BLACK, WHITE, BLACK, namen[i]);
    Provider[i].Button.drawButton();

  }

  return Provider;
}

void PressIfInRange(int x, int y, BtnProvider Provider[])
{
  for (int i = 0; i < sizeof(Provider); i++)
  {
    if (Provider[i].Spalte > x)
    {
      continue;
    }
    if ((Provider[i].Spalte + Abstand) < x)
    {
      continue;
    }
    if (Provider[i].Zeile > y)
    {
      continue;
    }
    if ((Provider[i].Zeile + Abstand) < y)
    {
      continue;
    }
    Provider[i].Button.press(true);
  }
}
