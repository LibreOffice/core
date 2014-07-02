package org.mozilla.gecko.gfx;

/**
* Created by quikee on 29.6.2014.
*/
public class SubTile extends SingleTileLayer {
    public int x;
    public int y;

    public SubTile(CairoImage mImage, int mX, int mY) {
        super(mImage);
        x = mX;
        y = mY;
    }
}
