package org.libreoffice;

import android.graphics.Bitmap;

import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.SubTile;

public class MockTileProvider implements TileProvider {
    private static final int TILE_SIZE = 256;
    private final LayerController layerController;
    private final String inputFile;

    public MockTileProvider(LayerController layerController, String inputFile) {
        this.layerController = layerController;
        this.inputFile = inputFile;

        for (int i = 0; i < 5; i++) {
            String partName = "Part " + i;
            DocumentPartView partView = new DocumentPartView(partName);
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
    public SubTile createTile(int x, int y) {
        int tiles = (getPageWidth() / TILE_SIZE) + 1;
        int tileNumber = (y / TILE_SIZE) * tiles + (x / TILE_SIZE);
        tileNumber %= 9;
        tileNumber += 1; // 0 to 1 based numbering

        String imageName = "d" + tileNumber;
        Bitmap bitmap = layerController.getDrawable(imageName);

        CairoImage image = new BufferedCairoImage(bitmap);
        SubTile tile = new SubTile(image, x, y);
        tile.beginTransaction();

        return tile;
    }
}
