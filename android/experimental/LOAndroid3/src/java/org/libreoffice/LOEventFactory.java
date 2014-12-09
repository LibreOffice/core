package org.libreoffice;

import android.graphics.Rect;

import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.IntSize;


public class LOEventFactory {
    public static LOEvent draw(Rect rect) {
        return new LOEvent(LOEvent.DRAW, rect);
    }

    public static LOEvent sizeChanged(int widthPixels, int heightPixels) {
        return new LOEvent(LOEvent.SIZE_CHANGED, widthPixels, heightPixels);
    }

    public static LOEvent tileSize(IntSize tileSize) {
        return new LOEvent(LOEvent.TILE_SIZE, tileSize);
    }

    public static LOEvent viewport(ImmutableViewportMetrics viewportMetrics) {
        return new LOEvent(LOEvent.VIEWPORT, viewportMetrics);
    }

    public static LOEvent changePart(int part) {
        return new LOEvent(LOEvent.CHANGE_PART, part);
    }

    public static LOEvent load(String inputFile) {
        return new LOEvent(LOEvent.LOAD, inputFile);
    }

    public static LOEvent close() {
        return new LOEvent(LOEvent.CLOSE);
    }

    public static LOEvent redraw() {
        return new LOEvent(LOEvent.REDRAW);
    }

    public static LOEvent tileRequest(ComposedTileLayer composedTileLayer, TileIdentifier tileRequest) {
        return new LOEvent(LOEvent.TILE_REQUEST, composedTileLayer, tileRequest);
    }
}
