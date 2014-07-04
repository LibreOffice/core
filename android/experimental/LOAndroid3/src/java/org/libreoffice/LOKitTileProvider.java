package org.libreoffice;

import android.graphics.Bitmap;
import android.util.Log;

import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;

import java.nio.ByteBuffer;
import java.util.Iterator;

import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.libreoffice.kit.Document;
import org.mozilla.gecko.gfx.SubTile;

public class LOKitTileProvider implements TileProvider {
    private final LayerController mLayerController;

    public static int TILE_SIZE = 256;

    public final Office mOffice;
    public final Document mDocument;

    private double mDPI;

    private double twipToPixel(double input, double dpi) {
        return input / 1440.0 * dpi;
    }

    private double pixelToTwip(double input, double dpi) {
        return (input / dpi) * 1440.0;
    }

    public LOKitTileProvider(LayerController layerController) {
        mLayerController = layerController;
        mDPI = (double) LOKitShell.getDpi();
        LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO-INFO.legacy.osl-INFO.i18nlangtag");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());
        String input = "/assets/test1.odt";
        mDocument = mOffice.documentLoad(input);
    }

    @Override
    public int getPageWidth() {
        return (int) twipToPixel(mDocument.getDocumentWidth(), mDPI);
    }

    @Override
    public int getPageHeight() {
        return (int) twipToPixel(mDocument.getDocumentHeight(), mDPI);
    }

    public TileIterator getTileIterator() {
        return new LoKitTileIterator();
    }

    public class LoKitTileIterator implements TileIterator, Iterator<SubTile> {
        private final double mTileWidth;
        private final double mTileHeight;

        private double mPositionWidth = 0;
        private double mPositionHeight = 0;
        private int mX = 0;
        private int mY = 0;

        private double mPageWidth;
        private double mPageHeight;

        public LoKitTileIterator() {
            mTileWidth  = pixelToTwip(TILE_SIZE, mDPI);
            mTileHeight = pixelToTwip(TILE_SIZE, mDPI);

            mPageWidth  = mDocument.getDocumentWidth();
            mPageHeight = mDocument.getDocumentHeight();
        }

        @Override
        public boolean hasNext() {
            return mPositionHeight <= mPageHeight;
        }

        @Override
        public SubTile next() {
            ByteBuffer buffer = ByteBuffer.allocateDirect(TILE_SIZE * TILE_SIZE * 4);
            Bitmap bitmap = Bitmap.createBitmap(TILE_SIZE, TILE_SIZE, Bitmap.Config.ARGB_8888);

            mDocument.paintTile(buffer, TILE_SIZE, TILE_SIZE, (int) Math.round(mPositionWidth), (int) Math.round(mPositionHeight), (int) Math.round(mTileWidth + pixelToTwip(1, mDPI)), (int) Math.round(mTileHeight+ pixelToTwip(1, mDPI)));


            bitmap.copyPixelsFromBuffer(buffer);

            CairoImage image = new BufferedCairoImage(bitmap);
            SubTile tile = new SubTile(image, mX, mY);
            tile.beginTransaction();

            mPositionWidth += mTileWidth;
            mX += TILE_SIZE;

            if (mPositionWidth > mPageWidth) {
                mPositionHeight += mTileHeight;
                mY += TILE_SIZE;

                mPositionWidth = 0;
                mX = 0;
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
