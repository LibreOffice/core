package org.libreoffice;

import android.graphics.Rect;

import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.ViewportMetrics;

public class LOEvent {

    public static final int SIZE_CHANGED = 1;
    public static final int TILE_SIZE = 2;
    public static final int VIEWPORT = 3;
    public static final int DRAW = 4;
    public static final int CHANGE_PART = 5;
    public int mType;
    ViewportMetrics viewportMetrics;
    private ViewportMetrics mViewportMetrics;
    private String mTypeString;
    private int mPartIndex;

    public LOEvent(int type, int widthPixels, int heightPixels, int tileWidth, int tileHeight) {
        mType = type;
        mTypeString = "Size Changed: " + widthPixels + " " + heightPixels;
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

    public LOEvent(int type, int partIndex) {
        mType = type;
        mPartIndex = partIndex;
        mTypeString = "Change part";
    }

    public static LOEvent draw(Rect rect) {
        return new LOEvent(DRAW, rect);
    }

    public static LOEvent sizeChanged(int widthPixels, int heightPixels, int tileWidth, int tileHeight) {
        return new LOEvent(SIZE_CHANGED, widthPixels, heightPixels, tileWidth, tileHeight);
    }

    public static LOEvent tileSize(IntSize tileSize) {
        return new LOEvent(TILE_SIZE, tileSize);
    }

    public static LOEvent viewport(ViewportMetrics viewportMetrics) {
        return new LOEvent(VIEWPORT, viewportMetrics);
    }

    public static LOEvent changePart(int part) {
        return new LOEvent(CHANGE_PART, part);
    }

    public String getTypeString() {
        return mTypeString;
    }

    public ViewportMetrics getViewport() {
        return mViewportMetrics;
    }

    public int getPartIndex() {
        return mPartIndex;
    }
}
