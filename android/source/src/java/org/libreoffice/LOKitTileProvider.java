/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.util.Log;
import android.view.KeyEvent;

import org.libreoffice.kit.DirectBufferAllocator;
import org.libreoffice.kit.Document;
import org.libreoffice.kit.LibreOfficeKit;
import org.libreoffice.kit.Office;
import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.IntSize;

import java.nio.ByteBuffer;

/**
 * LOKit implementation of TileProvider.
 */
public class LOKitTileProvider implements TileProvider {
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

    private Document.MessageCallback mMessageCallback;

    private long objectCreationTime = System.currentTimeMillis();

    /**
     * Initialize LOKit and load the document.
     * @param layerClient - layerclient implementation
     * @param messageCallback - callback for messages retrieved from LOKit
     * @param input - input path of the document
     */
    public LOKitTileProvider(GeckoLayerClient layerClient, Document.MessageCallback messageCallback, String input) {
        mLayerClient = layerClient;
        mMessageCallback = messageCallback;
        mDPI = LOKitShell.getDpi();
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

    /**
     * Triggered after the document is loaded.
     */
    private void postLoad() {
        mDocument.setMessageCallback(mMessageCallback);

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

        setupDocumentFonts();

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdapter().notifyDataSetChanged();
            }
        });
    }

    private void setupDocumentFonts() {
        String values = mDocument.getCommandValues(".uno:CharFontName");
        if (values == null || values.isEmpty())
            return;

        LOKitShell.getFontController().parseJson(values);
        LOKitShell.getFontController().setupFontViews();
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


    /**
     * @see TileProvider#getPartsCount()
     */
    @Override
    public int getPartsCount() {
        return mDocument.getParts();
    }

    /**
     * @see TileProvider#onSwipeLeft()
     */
    @Override
    public void onSwipeLeft() {
        if (mDocument.getDocumentType() == Document.DOCTYPE_PRESENTATION &&
                getCurrentPartNumber() < getPartsCount()-1) {
            LOKitShell.sendChangePartEvent(getCurrentPartNumber()+1);
        }
    }

    /**
     * @see TileProvider#onSwipeRight()
     */
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

    /**
     * @see TileProvider#getPageWidth()
     */
    @Override
    public int getPageWidth() {
        return (int) twipToPixel(mWidthTwip, mDPI);
    }

    /**
     * @see TileProvider#getPageHeight()
     */
    @Override
    public int getPageHeight() {
        return (int) twipToPixel(mHeightTwip, mDPI);
    }

    /**
     * @see TileProvider#isReady()
     */
    @Override
    public boolean isReady() {
        return mIsReady;
    }

    /**
     * @see TileProvider#createTile(float, float, org.mozilla.gecko.gfx.IntSize, float)
     */
    @Override
    public CairoImage createTile(float x, float y, IntSize tileSize, float zoom) {
        ByteBuffer buffer = DirectBufferAllocator.guardedAllocate(tileSize.width * tileSize.height * 4);
        if (buffer == null)
            return null;

        CairoImage image = new BufferedCairoImage(buffer, tileSize.width, tileSize.height, CairoImage.FORMAT_ARGB32);
        rerenderTile(image, x, y, tileSize, zoom);
        return image;
    }

    /**
     * @see TileProvider#rerenderTile(org.mozilla.gecko.gfx.CairoImage, float, float, org.mozilla.gecko.gfx.IntSize, float)
     */
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

    /**
     * @see TileProvider#thumbnail(int)
     */
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

    /**
     * @see TileProvider#close()
     */
    @Override
    public void close() {
        Log.i(LOGTAG, "Document destroyed: " + mInputFile);
        if (mDocument != null) {
            mDocument.destroy();
            mDocument = null;
        }
    }

    /**
     * @see TileProvider#isTextDocument()
     */
    @Override
    public boolean isTextDocument() {
        return mDocument != null && mDocument.getDocumentType() == Document.DOCTYPE_TEXT;
    }

    /**
     * @see TileProvider#isSpreadsheet()
     */
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

    /**
     * @see TileProvider#sendKeyEvent(android.view.KeyEvent)
     */
    @Override
    public void sendKeyEvent(KeyEvent keyEvent) {
        if (keyEvent.getAction() == KeyEvent.ACTION_MULTIPLE) {
            String keyString = keyEvent.getCharacters();
            for (int i = 0; i < keyString.length(); i++) {
                int codePoint = keyString.codePointAt(i);
                mDocument.postKeyEvent(Document.KEY_EVENT_PRESS, codePoint, getKeyCode(keyEvent));
            }
        } else if (keyEvent.getAction() == KeyEvent.ACTION_DOWN) {
            mDocument.postKeyEvent(Document.KEY_EVENT_PRESS, getCharCode(keyEvent), getKeyCode(keyEvent));
        } else if (keyEvent.getAction() == KeyEvent.ACTION_UP) {
            mDocument.postKeyEvent(Document.KEY_EVENT_RELEASE, getCharCode(keyEvent), getKeyCode(keyEvent));
        }
    }

    private void mouseButton(int type, PointF inDocument, int numberOfClicks, float zoomFactor) {
        int x = (int) pixelToTwip(inDocument.x, mDPI);
        int y = (int) pixelToTwip(inDocument.y, mDPI);

        mDocument.setClientZoom(TILE_SIZE, TILE_SIZE, (int) (mTileWidth / zoomFactor), (int) (mTileHeight / zoomFactor));
        mDocument.postMouseEvent(type, x, y, numberOfClicks, Document.MOUSE_BUTTON_LEFT, Document.KEYBOARD_MODIFIER_NONE);
    }

    /**
     * @see TileProvider#mouseButtonDown(android.graphics.PointF, int)
     */
    @Override
    public void mouseButtonDown(PointF documentCoordinate, int numberOfClicks, float zoomFactor) {
        mouseButton(Document.MOUSE_EVENT_BUTTON_DOWN, documentCoordinate, numberOfClicks, zoomFactor);
    }

    /**
     * @see TileProvider#mouseButtonUp(android.graphics.PointF, int)
     */
    @Override
    public void mouseButtonUp(PointF documentCoordinate, int numberOfClicks, float zoomFactor) {
        mouseButton(Document.MOUSE_EVENT_BUTTON_UP, documentCoordinate, numberOfClicks, zoomFactor);
    }

    /**
     * @param command   UNO command string
     * @param arguments Arguments to UNO command
     */
    @Override
    public void postUnoCommand(String command, String arguments) {
        mDocument.postUnoCommand(command, arguments);
    }

    private void setTextSelection(int type, PointF documentCoordinate) {
        int x = (int) pixelToTwip(documentCoordinate.x, mDPI);
        int y = (int) pixelToTwip(documentCoordinate.y, mDPI);
        mDocument.setTextSelection(type, x, y);
    }

    /**
     * @see TileProvider#setTextSelectionStart(android.graphics.PointF)
     */
    @Override
    public void setTextSelectionStart(PointF documentCoordinate) {
        setTextSelection(Document.SET_TEXT_SELECTION_START, documentCoordinate);
    }

    /**
     * @see TileProvider#setTextSelectionEnd(android.graphics.PointF)
     */
    @Override
    public void setTextSelectionEnd(PointF documentCoordinate) {
        setTextSelection(Document.SET_TEXT_SELECTION_END, documentCoordinate);
    }

    /**
     * @see TileProvider#setTextSelectionReset(android.graphics.PointF)
     */
    @Override
    public void setTextSelectionReset(PointF documentCoordinate) {
        setTextSelection(Document.SET_TEXT_SELECTION_RESET, documentCoordinate);
    }

    /**
     * @see org.libreoffice.TileProvider#setGraphicSelectionStart(android.graphics.PointF)
     */
    @Override
    public void setGraphicSelectionStart(PointF documentCoordinate) {
        setGraphicSelection(Document.SET_GRAPHIC_SELECTION_START, documentCoordinate);
    }

    /**
     * @see org.libreoffice.TileProvider#setGraphicSelectionEnd(android.graphics.PointF)
     */
    @Override
    public void setGraphicSelectionEnd(PointF documentCoordinate) {
        setGraphicSelection(Document.SET_GRAPHIC_SELECTION_END, documentCoordinate);
    }

    private void setGraphicSelection(int type, PointF documentCoordinate) {
        int x = (int) pixelToTwip(documentCoordinate.x, mDPI);
        int y = (int) pixelToTwip(documentCoordinate.y, mDPI);
        mDocument.setGraphicSelection(type, x, y);
    }

    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    /**
     * @see TileProvider#changePart(int)
     */
    @Override
    public void changePart(int partIndex) {
        if (mDocument == null)
            return;

        mDocument.setPart(partIndex);
        resetDocumentSize();
    }

    /**
     * @see TileProvider#getCurrentPartNumber()
     */
    @Override
    public int getCurrentPartNumber() {
        if (mDocument == null)
            return 0;

        return mDocument.getPart();
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
