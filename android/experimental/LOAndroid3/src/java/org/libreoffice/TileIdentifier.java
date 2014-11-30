package org.libreoffice;

public class TileIdentifier {
    public final int x;
    public final int y;
    public final float zoom;

    public TileIdentifier(int x, int y, float zoom) {
        this.x = x;
        this.y = y;
        this.zoom = zoom;
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
}