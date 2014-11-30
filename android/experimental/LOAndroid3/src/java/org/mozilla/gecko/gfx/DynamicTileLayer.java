package org.mozilla.gecko.gfx;

import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.util.Log;

import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.libreoffice.TileIdentifier;
import org.libreoffice.TileProvider;
import org.mozilla.gecko.util.FloatUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class DynamicTileLayer extends Layer {
    private static final String LOGTAG = DynamicTileLayer.class.getSimpleName();

    private final List<SubTile> tiles = new CopyOnWriteArrayList<SubTile>();

    private TileProvider tileProvider;
    private final IntSize tileSize;
    private RectF currentViewport = new RectF();

    public DynamicTileLayer() {
        this.tileSize = new IntSize(256, 256);
    }

    public DynamicTileLayer(IntSize tileSize) {
        this.tileSize = tileSize;
    }

    public void setTileProvider(TileProvider tileProvider) {
        this.tileProvider = tileProvider;
    }

    public void invalidate() {
        for (SubTile tile : tiles) {
            tile.invalidate();
        }
    }

    @Override
    public void beginTransaction() {
        super.beginTransaction();
        for (SubTile tile : tiles) {
            tile.beginTransaction();
        }
    }

    @Override
    public void endTransaction() {
        for (SubTile tile : tiles) {
            tile.endTransaction();
        }
        super.endTransaction();
    }

    @Override
    public void draw(RenderContext context) {
        for (SubTile tile : tiles) {
            if (RectF.intersects(tile.getBounds(context), context.viewport)) {
                tile.draw(context);
            }
        }
    }

    @Override
    protected void performUpdates(RenderContext context) {
        super.performUpdates(context);

        refreshTileMetrics();

        for (SubTile tile : tiles) {
            tile.performUpdates(context);
        }
    }

    @Override
    public Region getValidRegion(RenderContext context) {
        Region validRegion = new Region();
        for (SubTile tile : tiles) {
            validRegion.op(tile.getValidRegion(context), Region.Op.UNION);
        }

        return validRegion;
    }

    @Override
    public void setResolution(float newResolution) {
        super.setResolution(newResolution);
        for (SubTile tile : tiles) {
            tile.setResolution(newResolution);
        }
    }

    private void refreshTileMetrics() {
        for (SubTile tile : tiles) {
            tile.beginTransaction();

            Rect position = tile.getPosition();
            float positionX = tile.id.x / tile.id.zoom;
            float positionY = tile.id.y / tile.id.zoom;
            float tileSizeWidth = tileSize.width / tile.id.zoom;
            float tileSizeHeight = tileSize.height / tile.id.zoom;
            position.set((int) positionX, (int) positionY, (int) (positionX + tileSizeWidth + 1), (int) (positionY + tileSizeHeight + 1));
            tile.setPosition(position);

            tile.endTransaction();
        }
    }

    private RectF roundToTileSize(RectF input, IntSize tileSize) {
        float minX = ((int)(input.left / tileSize.width)) * tileSize.width;
        float minY = ((int)(input.top / tileSize.height)) * tileSize.height;
        float maxX = ((int)(input.right / tileSize.width) + 1) * tileSize.width;
        float maxY = ((int)(input.bottom / tileSize.height) + 1) * tileSize.height;
        return new RectF(minX, minY, maxX, maxY);
    }

    private RectF inflate(RectF rect, IntSize inflateSize) {
        RectF newRect = new RectF(rect);
        newRect.left -= inflateSize.width;
        newRect.left = newRect.left < 0.0f ? 0.0f : newRect.left;

        newRect.top -= inflateSize.height;
        newRect.top = newRect.top < 0.0f ? 0.0f : newRect.top;

        newRect.right += inflateSize.width;
        newRect.bottom += inflateSize.height;

        return newRect;
    }

    public void reevaluateTiles(ImmutableViewportMetrics viewportMetrics) {
        if (tileProvider == null) {
            return;
        }

        RectF newCurrentViewPort = inflate(roundToTileSize(viewportMetrics.getViewport(), tileSize), tileSize);

        if (!currentViewport.equals(newCurrentViewPort)) {
            Log.i(LOGTAG, "reevaluateTiles " + currentViewport + " " + newCurrentViewPort);
            currentViewport = newCurrentViewPort;
            clearMarkedTiles();
            addNewTiles(viewportMetrics);
            markTiles(viewportMetrics);
        }
    }

    private void addNewTiles(ImmutableViewportMetrics viewportMetrics) {
        for (float y = currentViewport.top; y < currentViewport.bottom; y += tileSize.height) {
            if (y > viewportMetrics.getPageHeight()) {
                continue;
            }
            for (float x = currentViewport.left; x < currentViewport.right; x += tileSize.width) {
                if (x > viewportMetrics.getPageWidth()) {
                    continue;
                }
                boolean contains = false;
                for (SubTile tile : tiles) {
                    if (tile.id.x == x && tile.id.y == y && tile.id.zoom == viewportMetrics.zoomFactor) {
                        contains = true;
                    }
                }
                if (!contains) {
                    LOKitShell.sendEvent(LOEventFactory.tileRequest(new TileIdentifier((int)x, (int)y, viewportMetrics.zoomFactor)));
                }
            }
        }
    }

    private void clearMarkedTiles() {
        List<SubTile> tilesToRemove = new ArrayList<SubTile>();
        for (SubTile tile : tiles) {
            if (tile.markedForRemoval) {
                tile.destroy();
                tilesToRemove.add(tile);
            }
        }
        tiles.removeAll(tilesToRemove);
    }

    private void markTiles(ImmutableViewportMetrics viewportMetrics) {
        for (SubTile tile : tiles) {
            if (FloatUtils.fuzzyEquals(tile.id.zoom, viewportMetrics.zoomFactor)) {
                RectF tileRect = new RectF(tile.id.x, tile.id.y, tile.id.x + tileSize.width, tile.id.y + tileSize.height);
                if (!RectF.intersects(currentViewport, tileRect)) {
                    tile.markForRemoval();
                }
            } else {
                tile.markForRemoval();
            }
        }
    }

    public void clearAndReset() {
        tiles.clear();
        currentViewport = new RectF();
    }

    public void addTile(TileIdentifier tileId) {
        CairoImage image = tileProvider.createTile(tileId.x, tileId.y, tileSize, tileId.zoom);
        SubTile tile = new SubTile(image, tileId);
        tile.beginTransaction();
        tiles.add(tile);
    }
}
