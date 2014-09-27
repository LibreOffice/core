package org.libreoffice;

import android.graphics.Rect;

import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.IntSize;

public class LOEvent {

    public static final int SIZE_CHANGED = 1;
    public static final int TILE_SIZE = 2;
    public static final int VIEWPORT = 3;
    public static final int DRAW = 4;
    public static final int CHANGE_PART = 5;
    public static final int LOAD = 6;

    public int mType;
    private ImmutableViewportMetrics mViewportMetrics;
    private String mTypeString;
    private int mPartIndex;
    private String mFilename;

    public LOEvent(int type, int widthPixels, int heightPixels) {
        mType = type;
        mTypeString = "Size Changed: " + widthPixels + " " + heightPixels;
    }

    public LOEvent(int type, String filename) {
        mType = type;
        mFilename = filename;
    }

    public LOEvent(int type, IntSize tileSize) {
        mType = type;
        mTypeString = "Tile size";
    }

    public LOEvent(int type, ImmutableViewportMetrics viewportMetrics) {
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

    public static LOEvent sizeChanged(int widthPixels, int heightPixels) {
        return new LOEvent(SIZE_CHANGED, widthPixels, heightPixels);
    }

    public static LOEvent tileSize(IntSize tileSize) {
        return new LOEvent(TILE_SIZE, tileSize);
    }

    public static LOEvent viewport(ImmutableViewportMetrics viewportMetrics) {
        return new LOEvent(VIEWPORT, viewportMetrics);
    }

    public static LOEvent changePart(int part) {
        return new LOEvent(CHANGE_PART, part);
    }

    public static LOEvent load(String inputFile) {
        return new LOEvent(LOAD, inputFile);
    }

    public String getTypeString() {
        return mTypeString;
    }

    public ImmutableViewportMetrics getViewport() {
        return mViewportMetrics;
    }

    public int getPartIndex() {
        return mPartIndex;
    }

    public String getFilename() {
        return mFilename;
    }

}
