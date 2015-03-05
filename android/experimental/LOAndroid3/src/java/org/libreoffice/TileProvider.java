package org.libreoffice;


import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;
import android.view.KeyEvent;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;

public interface TileProvider {
    /**
     * Returns the page width in pixels.
     */
    int getPageWidth();

    /**
     * Returns the page height in pixels.
     */
    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, IntSize tileSize, float zoom);

    /**
     * Rerender and overwrite tile's image buffer directly
     */
    void rerenderTile(CairoImage image, float x, float y, IntSize tileSize, float zoom);

    /**
     * Change the document part to the one specified by the partIndex input parameter.
     *
     * @param partIndex - part index to change to
     */
    void changePart(int partIndex);

    /**
     * Get the current document part number.
     *
     * @return
     */
    int getCurrentPartNumber();

    /**
     * Get the total number of parts.
     */
    int getPartsCount();

    Bitmap thumbnail(int size);

    /**
     * Closes the document.
     */
    void close();

    /**
     * Returns true if the current open document is a text document.
     */
    boolean isTextDocument();

    /**
     * Returns true if the current open document is a spreadsheet.
     */
    boolean isSpreadsheet();

    /**
     * Trigger a key event.
     *
     * @param keyEvent - contains information about key event
     */
    void sendKeyEvent(KeyEvent keyEvent);

    /**
     * Trigger a mouse button down event.
     *
     * @param documentCoordinate - coordinate relative to the document where the mouse button should be triggered
     * @param numberOfClicks     - number of clicks (1 - single click, 2 - double click)
     */
    void mouseButtonDown(PointF documentCoordinate, int numberOfClicks);


    /**
     * Trigger a swipe left event.
     */
    void onSwipeLeft();

    /**
     * Trigger a swipe left event.
     */
    void onSwipeRight();

    /**
     * Trigger a mouse button up event.
     *
     * @param documentCoordinate - coordinate relative to the document where the mouse button should be triggered
     * @param numberOfClicks     - number of clicks (1 - single click, 2 - double click)
     */
    void mouseButtonUp(PointF documentCoordinate, int numberOfClicks);

    void setTextSelectionStart(PointF documentCoordinate);

    void setTextSelectionEnd(PointF documentCoordinate);

    void setTextSelectionReset();
}
