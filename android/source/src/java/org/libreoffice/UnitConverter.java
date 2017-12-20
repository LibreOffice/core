package org.libreoffice;


public class UnitConverter {
    public static float twipToPixel(float input, float dpi) {
        return input / 1440.0f * dpi;
    }

    public static float pixelToTwip(float input, float dpi) {
        return (input / dpi) * 1440.0f;
    }

    public static float twipsToHMM(float twips) {
        return (twips * 127 + 36) / 72;
    }
}
