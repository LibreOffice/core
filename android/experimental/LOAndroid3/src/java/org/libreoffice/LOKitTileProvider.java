package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.IntSize;

import java.nio.ByteBuffer;

public class LOKitTileProvider implements TileProvider, Document.MessageCallback {
    private static final String LOGTAG = LOKitTileProvider.class.getSimpleName();
    private static int TILE_SIZE = 256;
    private final GeckoLayerClient mLayerClient;
    private final float mTileWidth;
    private final float mTileHeight;
    private final String mInputFile;
    private Office mOffice;
    private Document mDocument;
    private boolean mIsReady = false;
    private TileInvalidationCallback tileInvalidationCallback = null;

    private float mDPI;
    private float mWidthTwip;
    private float mHeightTwip;

    private long objectCreationTime = System.currentTimeMillis();

    public LOKitTileProvider(GeckoLayerClient layerClient, String input) {
        mLayerClient = layerClient;
        mDPI = (float) LOKitShell.getDpi();
        mTileWidth = pixelToTwip(TILE_SIZE, mDPI);
        mTileHeight = pixelToTwip(TILE_SIZE, mDPI);

        LibreOfficeKit.putenv("SAL_LOG=-WARN+INFO.lok");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());

        mInputFile = input;
        mDocument = mOffice.documentLoad(input);

        if (mDocument == null) {
            LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO");
            Log.i(LOGTAG, "====> mOffice.documentLoad() returned null, trying to restart 'Office' and loading again");
            mOffice.destroy();
            Log.i(LOGTAG, "====> mOffice.destroy() done");
            ByteBuffer handle = LibreOfficeKit.getLibreOfficeKitHandle();
            Log.i(LOGTAG, "====> getLibreOfficeKitHandle() = " + handle);
            mOffice = new Office(handle);
            Log.i(LOGTAG, "====> new Office created");
            mDocument = mOffice.documentLoad(input);
            LibreOfficeKit.putenv("SAL_LOG=-WARN+INFO.lok");
        }

        Log.i(LOGTAG, "====> mDocument = " + mDocument);

        if (checkDocument()) {
            postLoad();
            mIsReady = true;
        }
    }

    public void postLoad() {
        mDocument.initializeForRendering();
        mDocument.setMessageCallback(this);

        int parts = mDocument.getParts();
        Log.i(LOGTAG, "Document parts: " + parts);

        LibreOfficeMainActivity.mAppContext.getDocumentPartView().clear();

        // Writer documents always have one part, so hide the navigation drawer.
        if (mDocument.getDocumentType() != Document.DOCTYPE_TEXT) {
            for (int i = 0; i < parts; i++) {
                String partName = mDocument.getPartName(i);
                if (partName.isEmpty()) {
                    partName = getGenericPartName(i);
                }
                Log.i(LOGTAG, "Document part " + i + " name:'" + partName + "'");

                mDocument.setPart(i);
                resetDocumentSize();
                final DocumentPartView partView = new DocumentPartView(i, partName);
                LibreOfficeMainActivity.mAppContext.getDocumentPartView().add(partView);
            }
        } else {
            LibreOfficeMainActivity.mAppContext.disableNavigationDrawer();
        }

        mDocument.setPart(0);

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdapter().notifyDataSetChanged();
            }
        });
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
        String error = null;
        boolean ret;

        if (mDocument == null || !mOffice.getError().isEmpty()) {
            error = "Cannot open " + mInputFile + ": " + mOffice.getError();
            ret = false;
        } else {
            ret = resetDocumentSize();
            if (!ret) {
                error = "Document returned an invalid size or the document is empty!";
            }
        }

        if (!ret) {
            final String message = error;
            LOKitShell.getMainHandler().post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.showAlertDialog(message);
                }
            });
        }

        return ret;
    }

    private boolean resetDocumentSize() {
        mWidthTwip = mDocument.getDocumentWidth();
        mHeightTwip = mDocument.getDocumentHeight();

        if (mWidthTwip == 0 || mHeightTwip == 0) {
            Log.e(LOGTAG, "Document size zero - last error: " + mOffice.getError());
            return false;
        } else {
            Log.i(LOGTAG, "Reset document size: " + mDocument.getDocumentWidth() + " x " + mDocument.getDocumentHeight());
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
            long start = System.currentTimeMillis() - objectCreationTime;

            //Log.i(LOGTAG, "paintTile >> @" + start + " (" + tileSize.width + " " + tileSize.height + " " + (int) twipX + " " + (int) twipY + " " + (int) twipWidth + " " + (int) twipHeight + ")");
            mDocument.paintTile(buffer, tileSize.width, tileSize.height, (int) twipX, (int) twipY, (int) twipWidth, (int) twipHeight);

            long stop = System.currentTimeMillis() - objectCreationTime;
            //Log.i(LOGTAG, "paintTile << @" + stop + " elapsed: " + (stop - start));
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
    public boolean isTextDocument() {
        return mDocument.getDocumentType() == Document.DOCTYPE_TEXT;
    }

    @Override
    public void registerInvalidationCallback(TileInvalidationCallback tileInvalidationCallback) {
        this.tileInvalidationCallback = tileInvalidationCallback;
    }


    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    @Override
    public void changePart(int partIndex) {
        mDocument.setPart(partIndex);
        resetDocumentSize();
    }

    @Override
    public int getCurrentPartNumber() {
        return mDocument.getPart();
    }

    @Override
    public void messageRetrieved(int signalNumber, String payload) {
        switch (signalNumber) {
            case 0:
                if (!payload.equals("EMPTY")) {
                    String[] coordinates = payload.split(",");

                    if (coordinates.length == 4) {
                        int width = Integer.decode(coordinates[0].trim());
                        int height = Integer.decode(coordinates[1].trim());
                        int x = Integer.decode(coordinates[2].trim());
                        int y = Integer.decode(coordinates[3].trim());
                        RectF rect = new RectF(
                                twipToPixel(x, mDPI),
                                twipToPixel(y, mDPI),
                                twipToPixel(x + width, mDPI),
                                twipToPixel(y + height, mDPI)
                        );
                        Log.i(LOGTAG, "Invalidate R: " + rect +" - " + getPageWidth() + " " + getPageHeight());
                        tileInvalidationCallback.invalidate(rect);
                    }
                }
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
