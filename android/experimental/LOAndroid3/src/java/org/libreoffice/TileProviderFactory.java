package org.libreoffice;


import org.libreoffice.kit.LibreOfficeKit;
import org.mozilla.gecko.gfx.LayerController;

public class TileProviderFactory {
    private static TileProviderID currentTileProvider = TileProviderID.LOKIT;

    private TileProviderFactory() {

    }

    public static void initialize() {
        if (currentTileProvider == TileProviderID.LOKIT) {
            LibreOfficeKit.initializeLibrary();
        }
    }

    public static TileProvider create(LayerController layerController, String filename) {
        if (currentTileProvider == TileProviderID.LOKIT) {
            return new LOKitTileProvider(layerController, filename);
        } else {
            return new MockTileProvider(layerController, filename);
        }
    }

    private static enum TileProviderID {
        MOCK, LOKIT
    }
}