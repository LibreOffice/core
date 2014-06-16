package org.libreoffice;

import android.graphics.Rect;
import android.util.Log;

import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.ViewportMetrics;

public class LOEvent {

    public static final int SIZE_CHANGED = 1;
    public static final int TILE_SIZE = 2;
    public static final int VIEWPORT = 3;
    public static final int DRAW = 4;

    private ViewportMetrics mViewportMetrics;

    public int mType;
    private String mTypeString;

    ViewportMetrics viewportMetrics;

    public LOEvent(int type, int width, int height, int widthPixels, int heightPixels) {
        mType = type;
        mTypeString = "Size Changed";
    }

    public LOEvent(int type, IntSize tileSize) {
        mType = type;
        mTypeString = "Tile size";
    }

    public LOEvent(int type, ViewportMetrics viewportMetrics) {
        mType = type;
        mTypeString = "Viewport";
        mViewportMetrics = viewportMetrics;
    }

    public LOEvent(int type, Rect rect) {
        mType = type;
        mTypeString = "Draw";
    }

    public static LOEvent draw(Rect rect) {
        return new LOEvent(DRAW, rect);
    }

    public static LOEvent sizeChanged(int width, int height, int widthPixels, int heightPixels) {
        return new LOEvent(SIZE_CHANGED, width, height, widthPixels, heightPixels);
    }

    public static LOEvent tileSize(IntSize tileSize) {
        return new LOEvent(TILE_SIZE, tileSize);
    }

    public static LOEvent viewport(ViewportMetrics viewportMetrics) {
        return new LOEvent(VIEWPORT, viewportMetrics);
    }

    public String getTypeString() {
        return mTypeString;
    }

    public ViewportMetrics getViewport() {
        return mViewportMetrics;
    }
}
