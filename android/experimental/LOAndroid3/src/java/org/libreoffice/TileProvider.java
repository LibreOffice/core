package org.libreoffice;


import android.graphics.Bitmap;

import java.util.List;

public interface TileProvider  {
    int getPageWidth();

    int getPageHeight();

    TileIterator getTileIterator();
}
