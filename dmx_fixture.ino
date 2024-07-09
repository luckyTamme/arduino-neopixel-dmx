#include <Conceptinetics.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define LED_COUNT 300

#define FIXTURE_BEAMS 100
#define FIXTURE_BEAMS_CHANNELS 4
#define FIXTURE_GLOBAL_CHANNELS 1
#define FIXTURE_LEDS_PER_BEAM LED_COUNT / FIXTURE_BEAMS
#define DEAD_PIX 4

#define DMX_ADDRESS 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

DMX_Slave dmx_receiver(FIXTURE_BEAMS *FIXTURE_BEAMS_CHANNELS + FIXTURE_GLOBAL_CHANNELS);

unsigned long lastUpdate = millis();
bool strobeState = true;

void setup()
{
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  dmx_receiver.onReceiveComplete(OnFrameReceiveComplete);
  dmx_receiver.enable();
  dmx_receiver.setStartAddress(DMX_ADDRESS);
}

void loop() {}

void OnFrameReceiveComplete(void)
{
  int strobeValue = dmx_receiver.getChannelValue(DMX_ADDRESS);

  for (int beam = 0; beam < FIXTURE_BEAMS; beam++)
  {
    int beamAddress = FIXTURE_GLOBAL_CHANNELS + DMX_ADDRESS + FIXTURE_BEAMS_CHANNELS * beam;
    int newBrightness = dmx_receiver.getChannelValue(beamAddress);
    int newRed = dmx_receiver.getChannelValue(beamAddress + 1);
    int newGreen = dmx_receiver.getChannelValue(beamAddress + 2);
    int newBlue = dmx_receiver.getChannelValue(beamAddress + 3);

    newRed = newRed * newBrightness / 255;
    newGreen = newGreen * newBrightness / 255;
    newBlue = newBlue * newBrightness / 255;

    uint32_t newColor;

    unsigned long time = millis();
    // sqrt function to have more fine-grained control in the upper end of strobe frequency
    // if lastUpdate has passed long enough, switch strobeState
    int waitStrobe = 500 - (500 / pow(255, 0.2) * pow(strobeValue, 0.2));
    if (time > lastUpdate + waitStrobe)
    {
      strobeState = !strobeState;
      lastUpdate = time;
    }

    // set color to black if strobe is set and strobeState says it should be off now
    if (strobeState && strobeValue > 0)
    {
      newColor = strip.Color(0, 0, 0);
    }
    else
    {
      newColor = strip.Color(newRed, newGreen, newBlue);
    }

    int pixelStart = FIXTURE_LEDS_PER_BEAM * beam + DEAD_PIX * FIXTURE_LEDS_PER_BEAM;
    strip.fill(newColor, pixelStart, pixelStart + FIXTURE_LEDS_PER_BEAM);
  }

  strip.show();
}