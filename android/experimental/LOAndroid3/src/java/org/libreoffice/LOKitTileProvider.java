package org.libreoffice;

import android.graphics.Bitmap;
import android.util.Log;

import org.mozilla.gecko.gfx.LayerController;

import java.nio.ByteBuffer;
import java.util.Iterator;

import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.libreoffice.kit.Document;

public class LOKitTileProvider implements TileProvider {
    private final LayerController mLayerController;

    public static int TILE_SIZE = 256;

    public final Office mOffice;
    public final Document mDocument;

    public LOKitTileProvider(LayerController layerController) {
        this.mLayerController = layerController;
        LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO-INFO.legacy.osl-INFO.i18nlangtag");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());
        String input = "/assets/test1.odt";
        mDocument = mOffice.documentLoad(input);
    }

    @Override
    public int getPageWidth() {
        return (int) (mDocument.getDocumentWidth() / 1440.0 * LOKitShell.getDpi());
    }

    @Override
    public int getPageHeight() {
        return (int) (mDocument.getDocumentHeight() / 1440.0 * LOKitShell.getDpi());
    }

    public TileIterator getTileIterator() {
        return new LoKitTileIterator();
    }

    public class LoKitTileIterator implements TileIterator, Iterator<Bitmap> {
        private final double mTileWidth;
        private final double mTileHeight;

        private boolean mShouldContinue = true;

        private double mPositionWidth = 0;
        private double mPositionHeight = 0;

        private double mPageWidth;
        private double mPageHeight;

        public LoKitTileIterator() {
            mTileWidth  = (TILE_SIZE / (double) LOKitShell.getDpi()) * 1440.0;
            mTileHeight = (TILE_SIZE / (double) LOKitShell.getDpi()) * 1440.0;
            mPageWidth  = mDocument.getDocumentWidth();
            mPageHeight = mDocument.getDocumentHeight();
        }

        @Override
        public boolean hasNext() {
            return mShouldContinue;
        }

        @Override
        public Bitmap next() {
            ByteBuffer buffer = ByteBuffer.allocateDirect(TILE_SIZE * TILE_SIZE * 4);
            Bitmap bitmap = Bitmap.createBitmap(TILE_SIZE, TILE_SIZE, Bitmap.Config.ARGB_8888);

            mDocument.paintTile(buffer, TILE_SIZE, TILE_SIZE, (int) mPositionWidth, (int) mPositionHeight, (int) mTileWidth, (int) mTileHeight);

            mPositionWidth += mTileWidth;

            if (mPositionWidth > mPageWidth) {
                mPositionHeight += mTileHeight;
                mPositionWidth = 0;
            }

            if (mPositionHeight > mPageHeight || mPositionHeight > 20000) {
                mShouldContinue = false;
            }

            bitmap.copyPixelsFromBuffer(buffer);
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
