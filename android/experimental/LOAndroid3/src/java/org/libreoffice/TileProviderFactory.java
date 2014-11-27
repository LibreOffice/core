package org.libreoffice;


import org.libreoffice.kit.LibreOfficeKit;
import org.mozilla.gecko.gfx.GeckoLayerClient;

public class TileProviderFactory {
    private static TileProviderID currentTileProvider = TileProviderID.LOKIT;

    private TileProviderFactory() {

    }

    public static void initialize() {
        if (currentTileProvider == TileProviderID.LOKIT) {
            LibreOfficeKit.initializeLibrary();
        }
    }

    public static TileProvider create(GeckoLayerClient layerClient, String filename) {
        if (currentTileProvider == TileProviderID.LOKIT) {
            return new LOKitTileProvider(layerClient, filename);
        } else {
            return new MockTileProvider(layerClient, filename);
        }
    }

    private static enum TileProviderID {
        MOCK, LOKIT
    }
}