package org.libreoffice;


public interface TileProvider {
    int getPageWidth();
    int getPageHeight();

    TileIterator getTileIterator();
}
