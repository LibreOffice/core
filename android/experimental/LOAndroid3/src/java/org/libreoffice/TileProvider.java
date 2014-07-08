package org.libreoffice;


import org.mozilla.gecko.gfx.SubTile;

public interface TileProvider {
    int getPageWidth();
    int getPageHeight();

    SubTile createTile(int x, int y);
}
