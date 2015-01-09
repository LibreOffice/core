package org.libreoffice;

import android.graphics.RectF;

import org.mozilla.gecko.gfx.IntSize;

public class TileIdentifier {
    public final int x;
    public final int y;
    public final float zoom;
    public final IntSize size;

    public TileIdentifier(int x, int y, float zoom, IntSize size) {
        this.x = x;
        this.y = y;
        this.zoom = zoom;
        this.size = size;
    }

    public RectF getRect() {
        return new RectF(x, y, x + size.width, y + size.height);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        TileIdentifier that = (TileIdentifier) o;

        if (x != that.x) return false;
        if (y != that.y) return false;
        if (Float.compare(that.zoom, zoom) != 0) return false;

        return true;
    }

    @Override
    public int hashCode() {
        int result = x;
        result = 31 * result + y;
        result = 31 * result + (zoom != +0.0f ? Float.floatToIntBits(zoom) : 0);
        return result;
    }

    @Override
    public String toString() {
        return String.format("TileIdentifier (%d, %d) z=%f s=(%d, %d)", x, y, zoom, size.width, size.height);
    }
}