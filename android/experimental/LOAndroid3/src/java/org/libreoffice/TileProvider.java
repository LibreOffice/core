package org.libreoffice;


import android.graphics.Bitmap;
import android.graphics.RectF;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;

public interface TileProvider {
    int getPageWidth();

    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, IntSize tileSize, float zoom);

    void changePart(int partIndex);

    int getCurrentPartNumber();

    Bitmap thumbnail(int size);

    void close();

    boolean isTextDocument();

    void registerInvalidationCallback(TileProvider.TileInvalidationCallback tileInvalidationCallback);

    public interface TileInvalidationCallback {
        void invalidate(RectF rect);
    }
}
