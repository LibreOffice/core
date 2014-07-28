package org.libreoffice;

import android.graphics.Bitmap;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.SubTile;

import java.nio.ByteBuffer;

public class LOKitTileProvider implements TileProvider {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    private final LayerController mLayerController;

    public static int TILE_SIZE = 256;
    private final double mTileWidth;
    private final double mTileHeight;


    public final Office mOffice;
    public final Document mDocument;

    private double mDPI;

    private double twipToPixel(double input, double dpi) {
        return input / 1440.0 * dpi;
    }

    private double pixelToTwip(double input, double dpi) {
        return (input / dpi) * 1440.0;
    }

    public LOKitTileProvider(LayerController layerController, String input) {
        mLayerController = layerController;
        mDPI = (double) LOKitShell.getDpi();
        LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO-INFO.legacy.osl-INFO.i18nlangtag");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());

        mDocument = mOffice.documentLoad(input);

        if(mDocument == null) {
            Log.e(LOGTAG, "Error: " + mOffice.getError());
        } else {
            Log.i(LOGTAG, "Document parts: " + mDocument.getParts());
            if (mDocument.getParts() >= 1) {
                mDocument.setPart(1);
            }
            mTileWidth = pixelToTwip(TILE_SIZE, mDPI);
            mTileHeight = pixelToTwip(TILE_SIZE, mDPI);
        }
    }

    @Override
    public int getPageWidth() {
        return (int) twipToPixel(mDocument.getDocumentWidth(), mDPI);
    }

    @Override
    public int getPageHeight() {
        return (int) twipToPixel(mDocument.getDocumentHeight(), mDPI);
    }

    @Override
    public boolean isReady() {
        return mDocument != null;
    }

    public SubTile createTile(int x, int y) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(TILE_SIZE * TILE_SIZE * 4);
        Bitmap bitmap = Bitmap.createBitmap(TILE_SIZE, TILE_SIZE, Bitmap.Config.ARGB_8888);

        mDocument.paintTile(buffer, TILE_SIZE, TILE_SIZE, (int) pixelToTwip(x, mDPI), (int) pixelToTwip(y, mDPI), (int)mTileWidth, (int)mTileHeight);

        bitmap.copyPixelsFromBuffer(buffer);

        CairoImage image = new BufferedCairoImage(bitmap);
        SubTile tile = new SubTile(image, x, y);
        tile.beginTransaction();
        return tile;
    }
}
