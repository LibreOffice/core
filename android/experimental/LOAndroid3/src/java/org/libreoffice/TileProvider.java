package org.libreoffice;


import android.graphics.Bitmap;
import android.graphics.RectF;
import android.view.KeyEvent;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;

public interface TileProvider {
    int getPageWidth();

    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, IntSize tileSize, float zoom);

    /**
     * Rerender and overwrite tile's image buffer directly
     */
    void rerenderTile(CairoImage image, float x, float y, IntSize tileSize, float zoom);

    void changePart(int partIndex);

    int getCurrentPartNumber();

    Bitmap thumbnail(int size);

    void close();

    boolean isTextDocument();

    void registerInvalidationCallback(TileProvider.TileInvalidationCallback tileInvalidationCallback);

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
