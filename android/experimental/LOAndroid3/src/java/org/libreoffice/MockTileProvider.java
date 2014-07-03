package org.libreoffice;

import android.graphics.Bitmap;

import org.apache.http.MethodNotSupportedException;
import org.mozilla.gecko.gfx.LayerController;

import java.util.Iterator;
import java.util.List;

public class MockTileProvider implements TileProvider {
    private final LayerController layerController;

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

    public class MockTileIterator implements TileIterator, Iterator<Bitmap> {
        private final LayerController layerController;

        private int tileNumber = 1;

        public MockTileIterator(LayerController layerController) {
            this.layerController = layerController;
        }

        @Override
        public boolean hasNext() {
            return tileNumber <= 9;
        }

        @Override
        public Bitmap next() {
            String imageName = "d" + tileNumber;
            tileNumber++;
            Bitmap bitmap = layerController.getDrawable(imageName);
            return bitmap;
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }

        @Override
        public Iterator<Bitmap> iterator() {
            return this;
        }
    }
}
