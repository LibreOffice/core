package org.libreoffice;


import android.graphics.Bitmap;
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
     * @param partIndex - part index to change to
     */
    void changePart(int partIndex);

    /**
     * Get the current document part number.
     * @return
     */
    int getCurrentPartNumber();

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
     * Register a callback that is invoked when a tile invalidation is
     * required.
     *
     * @param tileInvalidationCallback - the tile invalidation callback
     */
    void registerInvalidationCallback(TileProvider.TileInvalidationCallback tileInvalidationCallback);

    /**
     * Trigger a key press.
     * @param keyEvent - contains the
     */
    void keyPress(KeyEvent keyEvent);

    /**
     * Callback to retrieve invalidation calls
     */
    public interface TileInvalidationCallback {
        /**
         * Invoked when a region is invalidated.
         * @param rect area in pixels which was invalidated and needs to be redrawn
         */
        void invalidate(RectF rect);
    }
}
