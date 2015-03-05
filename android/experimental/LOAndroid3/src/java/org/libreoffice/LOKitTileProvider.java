package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;
import android.view.KeyEvent;

import org.libreoffice.kit.DirectBufferAllocator;
import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.libreoffice.R;

import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.TextSelectionHandle;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.LayerView;


import java.nio.ByteBuffer;

/**
 * LOKit implementation of TileProvider.
 */
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

    private float mDPI;
    private float mWidthTwip;
    private float mHeightTwip;

    private InvalidationHandler mInvalidationHandler;

    private long objectCreationTime = System.currentTimeMillis();

    public LOKitTileProvider(GeckoLayerClient layerClient, InvalidationHandler invalidationHandler, String input) {
        mLayerClient = layerClient;
        mInvalidationHandler = invalidationHandler;
        mDPI = (float) LOKitShell.getDpi();
        mTileWidth = pixelToTwip(TILE_SIZE, mDPI);
        mTileHeight = pixelToTwip(TILE_SIZE, mDPI);

        LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO");
        LibreOfficeKit.init(LibreOfficeMainActivity.mAppContext);

        mOffice = new Office(LibreOfficeKit.getLibreOfficeKitHandle());

        mInputFile = input;

        Log.i(LOGTAG, "====> Loading file '" + input + "'");
        mDocument = mOffice.documentLoad(input);

        if (mDocument == null) {
            Log.i(LOGTAG, "====> mOffice.documentLoad() returned null, trying to restart 'Office' and loading again");
            mOffice.destroy();
            Log.i(LOGTAG, "====> mOffice.destroy() done");
            ByteBuffer handle = LibreOfficeKit.getLibreOfficeKitHandle();
            Log.i(LOGTAG, "====> getLibreOfficeKitHandle() = " + handle);
            mOffice = new Office(handle);
            Log.i(LOGTAG, "====> new Office created");
            mDocument = mOffice.documentLoad(input);
        }

        Log.i(LOGTAG, "====> mDocument = " + mDocument);

        if (mDocument != null)
            mDocument.initializeForRendering();

        if (checkDocument()) {
            postLoad();
            mIsReady = true;
        } else {
            mIsReady = false;
        }
    }

    public void postLoad() {
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

    public static float twipToPixel(float input, float dpi) {
        return input / 1440.0f * dpi;
    }

    public static float pixelToTwip(float input, float dpi) {
        return (input / dpi) * 1440.0f;
    }

    @Override
    public int getPartsCount() {
        return mDocument.getParts();
    }

    @Override
    public void onSwipeLeft() {
        if (mDocument.getDocumentType() == Document.DOCTYPE_PRESENTATION &&
                getCurrentPartNumber() < getPartsCount()-1) {
            LOKitShell.sendChangePartEvent(getCurrentPartNumber()+1);
        }
    }

    @Override
    public void onSwipeRight() {
        if (mDocument.getDocumentType() == Document.DOCTYPE_PRESENTATION &&
                getCurrentPartNumber() > 0) {
            LOKitShell.sendChangePartEvent(getCurrentPartNumber()-1);
        }
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
                error = "Document returned an invalid size or the document is empty.";
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
        ByteBuffer buffer = DirectBufferAllocator.guardedAllocate(tileSize.width * tileSize.height * 4);
        if (buffer == null)
            return null;

        CairoImage image = new BufferedCairoImage(buffer, tileSize.width, tileSize.height, CairoImage.FORMAT_ARGB32);
        rerenderTile(image, x, y, tileSize, zoom);
        return image;
    }

    @Override
    public void rerenderTile(CairoImage image, float x, float y, IntSize tileSize, float zoom) {
        if (mDocument != null && image.getBuffer() != null) {
            float twipX = pixelToTwip(x, mDPI) / zoom;
            float twipY = pixelToTwip(y, mDPI) / zoom;
            float twipWidth = mTileWidth / zoom;
            float twipHeight = mTileHeight / zoom;
            long start = System.currentTimeMillis() - objectCreationTime;

            //Log.i(LOGTAG, "paintTile >> @" + start + " (" + tileSize.width + " " + tileSize.height + " " + (int) twipX + " " + (int) twipY + " " + (int) twipWidth + " " + (int) twipHeight + ")");
            mDocument.paintTile(image.getBuffer(), tileSize.width, tileSize.height, (int) twipX, (int) twipY, (int) twipWidth, (int) twipHeight);

            long stop = System.currentTimeMillis() - objectCreationTime;
            //Log.i(LOGTAG, "paintTile << @" + stop + " elapsed: " + (stop - start));
        } else {
            if (mDocument == null) {
                Log.e(LOGTAG, "Document is null!!");
            }
        }
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
        if (mDocument != null)
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
        return mDocument != null && mDocument.getDocumentType() == Document.DOCTYPE_TEXT;
    }

    @Override
    public boolean isSpreadsheet() {
        return mDocument != null && mDocument.getDocumentType() == Document.DOCTYPE_SPREADSHEET;
    }

    /**
     * Returns the Unicode character generated by this event or 0.
     */
    private int getCharCode(KeyEvent keyEvent) {
        switch (keyEvent.getKeyCode())
        {
            case KeyEvent.KEYCODE_DEL:
            case KeyEvent.KEYCODE_ENTER:
                return 0;
        }
        return keyEvent.getUnicodeChar();
    }

    /**
     * Returns the integer code representing the key of the event (non-zero for
     * control keys).
     */
    private int getKeyCode(KeyEvent keyEvent) {
        switch (keyEvent.getKeyCode()) {
            case KeyEvent.KEYCODE_DEL:
                return com.sun.star.awt.Key.BACKSPACE;
            case KeyEvent.KEYCODE_ENTER:
                return com.sun.star.awt.Key.RETURN;
        }
        return 0;
    }

    @Override
    public void sendKeyEvent(KeyEvent keyEvent) {
        if (keyEvent.getAction() == KeyEvent.ACTION_MULTIPLE) {
            String keyString = keyEvent.getCharacters();
            for (int i = 0; i < keyString.length(); i++) {
                int codePoint = keyString.codePointAt(i);
                mDocument.postKeyEvent(Office.KEY_PRESS, codePoint, getKeyCode(keyEvent));
            }
        } else if (keyEvent.getAction() == KeyEvent.ACTION_DOWN) {
            mDocument.postKeyEvent(Office.KEY_PRESS, getCharCode(keyEvent), getKeyCode(keyEvent));
        } else if (keyEvent.getAction() == KeyEvent.ACTION_UP) {
            mDocument.postKeyEvent(Office.KEY_RELEASE, getCharCode(keyEvent), getKeyCode(keyEvent));
        }
    }

    private void mouseButton(int type, PointF inDocument, int numberOfClicks) {
        int x = (int) pixelToTwip(inDocument.x, mDPI);
        int y = (int) pixelToTwip(inDocument.y, mDPI);

        mDocument.postMouseEvent(type, x, y, numberOfClicks);
    }

    @Override
    public void mouseButtonDown(PointF documentCoordinate, int numberOfClicks) {
        mouseButton(Document.MOUSE_BUTTON_DOWN, documentCoordinate, numberOfClicks);
    }

    @Override
    public void mouseButtonUp(PointF documentCoordinate, int numberOfClicks) {
        mouseButton(Document.MOUSE_BUTTON_UP, documentCoordinate, numberOfClicks);
    }

    @Override
    public void setTextSelectionStart(PointF documentCoordinate) {
        int x = (int) pixelToTwip(documentCoordinate.x, mDPI);
        int y = (int) pixelToTwip(documentCoordinate.y, mDPI);
        mDocument.setTextSelection(Document.TEXT_SELECTION_START, x, y);
    }

    @Override
    public void setTextSelectionEnd(PointF documentCoordinate) {
        int x = (int) pixelToTwip(documentCoordinate.x, mDPI);
        int y = (int) pixelToTwip(documentCoordinate.y, mDPI);
        mDocument.setTextSelection(Document.TEXT_SELECTION_END, x, y);
    }

    @Override
    public void setTextSelectionReset() {
        mDocument.setTextSelection(Document.TEXT_SELECTION_RESET, 0, 0);
    }

    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    @Override
    public void changePart(int partIndex) {
        if (mDocument == null)
            return;

        mDocument.setPart(partIndex);
        resetDocumentSize();
    }

    @Override
    public int getCurrentPartNumber() {
        if (mDocument == null)
            return 0;

        return mDocument.getPart();
    }

    /**
     * Process the retrieved messages from LOK
     */
    @Override
    public void messageRetrieved(int messageID, String payload) {
        if (!LOKitShell.isEditingEnabled()) {
            return;
        }

        mInvalidationHandler.processMessage(messageID, payload);
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
