package org.libreoffice;

import android.graphics.Bitmap;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.LayerController;

import java.nio.ByteBuffer;

public class LOKitTileProvider implements TileProvider {
    private static final String LOGTAG = LOKitTileProvider.class.getSimpleName();
    public static int TILE_SIZE = 256;
    public final Office mOffice;
    public final Document mDocument;
    private final LayerController mLayerController;
    private final float mTileWidth;
    private final float mTileHeight;
    private final String mInputFile;

    private float mDPI;
    private float mWidthTwip;
    private float mHeightTwip;

    public LOKitTileProvider(LayerController layerController, String input) {
        mLayerController = layerController;
        mDPI = (float) LOKitShell.getDpi();
        mTileWidth = pixelToTwip(TILE_SIZE, mDPI);
        mTileHeight = pixelToTwip(TILE_SIZE, mDPI);

        LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO-INFO.legacy.osl-INFO.i18nlangtag");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());

        mInputFile = input;
        mDocument = mOffice.documentLoad(input);

        if (checkDocument()) {
            int parts = mDocument.getParts();
            Log.i(LOGTAG, "Document parts: " + parts);
            if (parts >= 1) {
                mDocument.setPart(0);
            }
            for (int i = 0; i < parts; i++) {
                String partName = mDocument.getPartName(i);
                if (partName.isEmpty()) {
                    switch (mDocument.getDocumentType()) {
                        case Document.DOCTYPE_DRAWING:
                        case Document.DOCTYPE_TEXT:
                            partName = "Page " + (i + 1);
                            break;
                        case Document.DOCTYPE_SPREADSHEET:
                            partName = "Sheet " + (i + 1);
                            break;
                        case Document.DOCTYPE_PRESENTATION:
                            partName = "Slide " + (i + 1);
                            break;
                        case Document.DOCTYPE_OTHER:
                        default:
                            partName = "Part " + (i + 1);
                            break;
                    }
                }
                Log.i(LOGTAG, "Document part " + i + " name:'" + partName + "'");
                final DocumentPartView partView = new DocumentPartView(i, partName);
                LibreOfficeMainActivity.mAppContext.getDocumentPartView().add(partView);
            }

            LibreOfficeMainActivity.mAppContext.mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdpater().notifyDataSetChanged();
                }
            });
        }
    }

    private float twipToPixel(float input, float dpi) {
        return input / 1440.0f * dpi;
    }

    private float pixelToTwip(float input, float dpi) {
        return (input / dpi) * 1440.0f;
    }

    private boolean checkDocument() {
        if (mDocument == null || !mOffice.getError().isEmpty()) {
            Log.e(LOGTAG, "Error at loading: " + mOffice.getError());
            return false;
        }

        mWidthTwip = mDocument.getDocumentWidth();
        mHeightTwip = mDocument.getDocumentHeight();

        if (mWidthTwip == 0 && mHeightTwip == 0) {
            Log.e(LOGTAG, "Document size zero - last error: " + mOffice.getError());
        } else {
            Log.i(LOGTAG, "Document size: " + mDocument.getDocumentWidth() + " x " + mDocument.getDocumentHeight());
        }

        return true;
    }

    @Override
    public int getPageWidth() {
        return (int) twipToPixel(mWidthTwip, mDPI);
    }

    @Override
    public int getPageHeight() {
        return (int) twipToPixel(mHeightTwip, mDPI);
    }

    @Override
    public boolean isReady() {
        return mDocument != null;
    }

    @Override
    public CairoImage createTile(float x, float y, float zoom) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(TILE_SIZE * TILE_SIZE * 4);
        Bitmap bitmap = Bitmap.createBitmap(TILE_SIZE, TILE_SIZE, Bitmap.Config.ARGB_8888);

        if (mDocument != null) {
            float twipX = pixelToTwip(x, mDPI) / zoom;
            float twipY = pixelToTwip(y, mDPI) / zoom;
            float twipWidth  = mTileWidth / zoom;
            float twipHeight = mTileHeight / zoom;
            mDocument.paintTile(buffer, TILE_SIZE, TILE_SIZE, (int) twipX, (int) twipY, (int)twipWidth, (int)twipHeight);
        } else {
            Log.e(LOGTAG, "Document is null!!");
        }

        bitmap.copyPixelsFromBuffer(buffer);

        CairoImage image = new BufferedCairoImage(bitmap);

        return image;
    }

    @Override
    public Bitmap thumbnail(int size) {
        int widthPixel = getPageWidth();
        int heightPixel = getPageHeight();

        if (widthPixel > heightPixel) {
            double ratio = heightPixel / (double) widthPixel;
            widthPixel = size;
            heightPixel = (int) (widthPixel * ratio);
        } else {
            double ratio = widthPixel / (double) heightPixel;
            heightPixel = size;
            widthPixel = (int) (heightPixel * ratio);
        }

        ByteBuffer buffer = ByteBuffer.allocateDirect(widthPixel * heightPixel * 4);
        mDocument.paintTile(buffer, widthPixel, heightPixel, 0, 0, (int) mWidthTwip, (int) mHeightTwip);

        Bitmap bitmap = Bitmap.createBitmap(widthPixel, heightPixel, Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(buffer);
        if (bitmap == null) {
            Log.w(LOGTAG, "Thumbnail not created!");
        }
        return bitmap;
    }

    @Override
    public void close() {
        Log.i(LOGTAG, "Document destroyed: " + mInputFile);
        if (mDocument != null) {
            mDocument.destroy();
        }
    }

    @Override
    public void changePart(int partIndex) {
        mDocument.setPart(partIndex);
    }
}
