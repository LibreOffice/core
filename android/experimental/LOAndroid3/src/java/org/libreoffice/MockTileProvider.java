package org.libreoffice;

import android.graphics.Bitmap;

import org.apache.http.MethodNotSupportedException;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.SubTile;

import java.util.Iterator;
import java.util.List;

public class MockTileProvider implements TileProvider {
    private final LayerController layerController;
    private static final int TILE_SIZE = 256;

    public MockTileProvider(LayerController layerController) {
        this.layerController = layerController;
    }

    @Override
    public int getPageWidth() {
        return 549;
    }

    @Override
    public int getPageHeight() {
        return 630;
    }

    public TileIterator getTileIterator() {
        return new MockTileIterator(layerController);
    }

    public class MockTileIterator implements TileIterator, Iterator<SubTile> {
        private final LayerController layerController;

        private int tileNumber = 1;

        private int x = 0;
        private int y = 0;

        public MockTileIterator(LayerController layerController) {
            this.layerController = layerController;
        }

        @Override
        public boolean hasNext() {
            return tileNumber <= 9;
        }

        @Override
        public SubTile next() {
            String imageName = "d" + tileNumber;
            tileNumber++;
            Bitmap bitmap = layerController.getDrawable(imageName);

            CairoImage image = new BufferedCairoImage(bitmap);
            SubTile tile = new SubTile(image, x, y);
            tile.beginTransaction();

            x += TILE_SIZE;

            if (x > getPageWidth()) {
                x = 0;
                y += TILE_SIZE;
            }

            return tile;
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }

        @Override
        public Iterator<SubTile> iterator() {
            return this;
        }
    }
}
