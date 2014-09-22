package org.libreoffice;


import android.graphics.Bitmap;

import org.mozilla.gecko.gfx.SubTile;

public interface TileProvider {
    int getPageWidth();

    int getPageHeight();

    boolean isReady();

    SubTile createTile(int x, int y);

    void changePart(int partIndex);

    Bitmap thumbnail();

    void close();
}
