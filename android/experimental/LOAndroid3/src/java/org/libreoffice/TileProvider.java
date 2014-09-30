package org.libreoffice;


import android.graphics.Bitmap;

import org.mozilla.gecko.gfx.CairoImage;

public interface TileProvider {
    int getPageWidth();

    int getPageHeight();

    boolean isReady();

    CairoImage createTile(float x, float y, float zoom);

    void changePart(int partIndex);

    Bitmap thumbnail(int size);

    void close();
}
