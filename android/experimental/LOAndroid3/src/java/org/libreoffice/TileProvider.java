package org.libreoffice;


import android.graphics.Bitmap;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.FloatSize;

public interface TileProvider {
    int getPageWidth();

    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, FloatSize tileSize, float zoom);

    void changePart(int partIndex);

    Bitmap thumbnail(int size);

    void close();
}
