package org.libreoffice;

import android.graphics.Bitmap;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.LayerController;

import java.nio.ByteBuffer;

public class LOKitTileProvider implements TileProvider {
    private static final String LOGTAG = LOKitTileProvider.class.getSimpleName();
    private static int TILE_SIZE = 256;
    private Office mOffice;
    private Document mDocument;
    private final LayerController mLayerController;
    private final float mTileWidth;
    private final float mTileHeight;
    private final String mInputFile;
    private boolean mIsReady = false;

    private float mDPI;
    private float mWidthTwip;
    private float mHeightTwip;

    public LOKitTileProvider(LayerController layerController, String input) {
        mLayerController = layerController;
        mDPI = (float) LOKitShell.getDpi();
        mTileWidth = pixelToTwip(TILE_SIZE, mDPI);
        mTileHeight = pixelToTwip(TILE_SIZE, mDPI);

        LibreOfficeKit.putenv("SAL_LOG=-WARN+INFO.lok");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());

        mInputFile = input;
        mDocument = mOffice.documentLoad(input);

        if (mDocument == null) {
            Log.i(LOGTAG, "====> mOffice.documentLoad() returned null, trying to restart 'Office' and loading again");
            mOffice.destroy();
            Log.i(LOGTAG, "====> mOffice.destroy() done");
            long handle = LibreOfficeKit.getLibreOfficeKitHandle();
            Log.i(LOGTAG, "====> getLibreOfficeKitHandle() = " + handle);
            mOffice = new Office(handle);
            Log.i(LOGTAG, "====> new Office created");
            mDocument = mOffice.documentLoad(input);
        }

        Log.i(LOGTAG, "====> mDocument = " + mDocument);

        if (checkDocument()) {
            int parts = mDocument.getParts();
            Log.i(LOGTAG, "Document parts: " + parts);

            LibreOfficeMainActivity.mAppContext.getDocumentPartView().clear();

            if (parts > 1) {
                for (int i = 0; i < parts; i++) {
                    String partName = mDocument.getPartName(i);
                    if (partName.isEmpty()) {
                        partName = getGenericPartName(i);
                    }
                    Log.i(LOGTAG, "Document part " + i + " name:'" + partName + "'");

                    mDocument.setPart(i);
                    final DocumentPartView partView = new DocumentPartView(i, partName, thumbnail(128));
                    LibreOfficeMainActivity.mAppContext.getDocumentPartView().add(partView);
                }
            }

            mDocument.setPart(0);

            LOKitShell.getMainHandler().post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdpater().notifyDataSetChanged();
                }
            });

            mIsReady = true;
        }
    }

    private String getGenericPartName(int i) {
        if (mDocument == null) {
            return "";
        }
        switch (mDocument.getDocumentType()) {
            case Document.DOCTYPE_DRAWING:
            case Document.DOCTYPE_TEXT:
                return "Page " + (i + 1);
            case Document.DOCTYPE_SPREADSHEET:
                return "Sheet " + (i + 1);
            case Document.DOCTYPE_PRESENTATION:
                return "Slide " + (i + 1);
            case Document.DOCTYPE_OTHER:
            default:
                return "Part " + (i + 1);
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

        if (mWidthTwip == 0 || mHeightTwip == 0) {
            Log.e(LOGTAG, "Document size zero - last error: " + mOffice.getError());
            LOKitShell.getMainHandler().post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.showAlertDialog("Document has no size!");
                }
            });
            return false;
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
        return mIsReady;
    }

    @Override
    public CairoImage createTile(float x, float y, IntSize tileSize, float zoom) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(tileSize.width * tileSize.height * 4);
        Bitmap bitmap = Bitmap.createBitmap(tileSize.width, tileSize.height, Bitmap.Config.ARGB_8888);

        if (mDocument != null) {
            float twipX = pixelToTwip(x, mDPI) / zoom;
            float twipY = pixelToTwip(y, mDPI) / zoom;
            float twipWidth = mTileWidth / zoom;
            float twipHeight = mTileHeight / zoom;
            long start = System.currentTimeMillis();
            Log.i(LOGTAG, "paintTile TOP @ " + start + "(" + tileSize.width + " " + tileSize.height + " " + (int) twipX + " " + (int) twipY + " " + (int) twipWidth + " " + (int) twipHeight + ")");
            mDocument.paintTile(buffer, tileSize.width, tileSize.height, (int) twipX, (int) twipY, (int) twipWidth, (int) twipHeight);
            long stop = System.currentTimeMillis();
            Log.i(LOGTAG, "paintTile TAIL @ " + stop + " - elapsed: " + (stop - start) + " ");
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

        Log.w(LOGTAG, "Thumbnail size: " + getPageWidth() + " " + getPageHeight() + " " + widthPixel + " " + heightPixel);

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
            mDocument = null;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    @Override
    public void changePart(int partIndex) {
        mDocument.setPart(partIndex);
    }
}
