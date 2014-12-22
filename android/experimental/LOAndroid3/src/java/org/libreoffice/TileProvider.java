package org.libreoffice;


import android.graphics.Bitmap;

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
}
