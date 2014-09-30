package org.libreoffice;

import android.graphics.Bitmap;

import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;

public class MockTileProvider implements TileProvider {
    private static final int TILE_SIZE = 256;
    private final LayerController layerController;
    private final String inputFile;

    public MockTileProvider(LayerController layerController, String inputFile) {
        this.layerController = layerController;
        this.inputFile = inputFile;

        for (int i = 0; i < 5; i++) {
            String partName = "Part " + i;
            DocumentPartView partView = new DocumentPartView(i, partName);
            LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdpater().add(partView);
        }
        LibreOfficeMainActivity.mAppContext.mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdpater().notifyDataSetChanged();
            }
        });
    }

    @Override
    public int getPageWidth() {
        return 549;
    }

    @Override
    public int getPageHeight() {
        return 630*5;
    }

    @Override
    public boolean isReady() {
        return true;
    }

    @Override
    public CairoImage createTile(float x, float y, float zoom) {
        int tiles = (int) (getPageWidth() / TILE_SIZE) + 1;
        int tileNumber = (int) ((y / TILE_SIZE) * tiles + (x / TILE_SIZE));
        tileNumber %= 9;
        tileNumber += 1; // 0 to 1 based numbering

        String imageName = "d" + tileNumber;
        Bitmap bitmap = layerController.getDrawable(imageName);

        CairoImage image = new BufferedCairoImage(bitmap);

        return image;
    }

    @Override
    public Bitmap thumbnail(int size) {
        return layerController.getDrawable("dummy_page");
    }

    @Override
    public void close() {
    }

    @Override
    public void changePart(int partIndex) {

    }
}
