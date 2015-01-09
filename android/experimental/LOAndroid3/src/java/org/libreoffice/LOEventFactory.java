package org.libreoffice;

import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.SubTile;


public class LOEventFactory {
    public static LOEvent sizeChanged(int widthPixels, int heightPixels) {
        return new LOEvent(LOEvent.SIZE_CHANGED, widthPixels, heightPixels);
    }

    public static LOEvent tileSize(IntSize tileSize) {
        return new LOEvent(LOEvent.TILE_SIZE, tileSize);
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

    public static LOEvent tileRequest(ComposedTileLayer composedTileLayer, TileIdentifier tileID, boolean forceRedraw) {
        return new LOEvent(LOEvent.TILE_REQUEST, composedTileLayer, tileID, forceRedraw);
    }

    public static LOEvent thumbnail(ThumbnailCreator.ThumbnailCreationTask task) {
        return new LOEvent(LOEvent.THUMBNAIL, task);
    }

    public static LOEvent tileRerender(ComposedTileLayer composedTileLayer, SubTile tile) {
        return new LOEvent(LOEvent.TILE_RERENDER, composedTileLayer, tile);
    }
}
