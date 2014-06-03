package org.libreoffice;


import org.mozilla.gecko.gfx.LayerView;

public class LOKitShell {
    public static int getDpi() {
        return 96;
    }

    public static int getScreenDepth() {
        return 24;
    }

    public static LayerView getLayerView() {
        return null;
    }

    public static float computeRenderIntegrity() {
        return 0.0f;
    }
}
