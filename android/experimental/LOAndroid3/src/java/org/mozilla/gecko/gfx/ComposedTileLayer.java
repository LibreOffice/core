package org.mozilla.gecko.gfx;

import android.graphics.RectF;
import android.graphics.Region;
import android.util.Log;

import org.libreoffice.LOEvent;
import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.libreoffice.TileIdentifier;
import org.mozilla.gecko.util.FloatUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public abstract class ComposedTileLayer extends Layer {
    private static final String LOGTAG = ComposedTileLayer.class.getSimpleName();

    protected final List<SubTile> tiles = new CopyOnWriteArrayList<SubTile>();

    protected final IntSize tileSize;
    protected RectF currentViewport = new RectF();
    protected float currentZoom;

    public ComposedTileLayer() {
        this.tileSize = new IntSize(256, 256);
    }

    public ComposedTileLayer(IntSize tileSize) {
        this.tileSize = tileSize;
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

        for (SubTile tile : tiles) {
            tile.beginTransaction();
            tile.refreshTileMetrics();
            tile.endTransaction();
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

    protected RectF roundToTileSize(RectF input, IntSize tileSize) {
        float minX = ((int) (input.left / tileSize.width)) * tileSize.width;
        float minY = ((int) (input.top / tileSize.height)) * tileSize.height;
        float maxX = ((int) (input.right / tileSize.width) + 1) * tileSize.width;
        float maxY = ((int) (input.bottom / tileSize.height) + 1) * tileSize.height;
        return new RectF(minX, minY, maxX, maxY);
    }

    protected RectF inflate(RectF rect, IntSize inflateSize) {
        RectF newRect = new RectF(rect);
        newRect.left -= inflateSize.width;
        newRect.left = newRect.left < 0.0f ? 0.0f : newRect.left;

        newRect.top -= inflateSize.height;
        newRect.top = newRect.top < 0.0f ? 0.0f : newRect.top;

        newRect.right += inflateSize.width;
        newRect.bottom += inflateSize.height;

        return newRect;
    }

    protected RectF normalizeRect(RectF rect, float sourceFactor, float targetFactor) {
        RectF normalizedRect = new RectF(
                (rect.left / sourceFactor) * targetFactor,
                (rect.top / sourceFactor) * targetFactor,
                (rect.right / sourceFactor) * targetFactor,
                (rect.bottom / sourceFactor) * targetFactor);

        return normalizedRect;
    }

    public void reevaluateTiles(ImmutableViewportMetrics viewportMetrics, DisplayPortMetrics mDisplayPort) {
        RectF newCurrentViewPort = getViewPort(viewportMetrics);
        float newZoom = viewportMetrics.zoomFactor;

        if (!currentViewport.equals(newCurrentViewPort) || currentZoom != newZoom) {
            if (newZoom == 1.0f) {
                Log.i(LOGTAG, "Suspisious zoom 1.0");
            }
            currentViewport = newCurrentViewPort;
            currentZoom = newZoom;

            clearMarkedTiles();
            addNewTiles(viewportMetrics);
            markTiles(viewportMetrics);
        }
    }

    protected abstract RectF getViewPort(ImmutableViewportMetrics viewportMetrics);

    protected abstract float getZoom(ImmutableViewportMetrics viewportMetrics);

    protected abstract int getTilePriority();

    private void addNewTiles(ImmutableViewportMetrics viewportMetrics) {
        float zoom = getZoom(viewportMetrics);

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
                    if (tile.id.x == x && tile.id.y == y && tile.id.zoom == zoom) {
                        contains = true;
                    }
                }
                if (!contains) {
                    LOEvent event = LOEventFactory.tileRequest(this, new TileIdentifier((int) x, (int) y, zoom, tileSize), false);
                    event.mPriority = getTilePriority();
                    LOKitShell.sendEvent(event);
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
        float zoom = getZoom(viewportMetrics);
        for (SubTile tile : tiles) {
            if (FloatUtils.fuzzyEquals(tile.id.zoom, zoom)) {
                RectF tileRect = tile.id.getRect();
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

    public void addTile(SubTile tile) {
        tile.beginTransaction();
        tiles.add(tile);
    }

    public boolean isStillValid(TileIdentifier tileId) {
        return RectF.intersects(currentViewport, tileId.getRect()) || currentViewport.contains(tileId.getRect());
    }

    public void invalidateTiles(RectF rect) {
        Log.i(LOGTAG, "invalidate: " + rect);
        for (SubTile tile : tiles) {
            if (RectF.intersects(rect, tile.id.getRect()) || rect.contains(tile.id.getRect())) {
                tile.markForRemoval();
                LOKitShell.sendEvent(LOEventFactory.tileRequest(this, tile.id, true));
                Log.i(LOGTAG, "invalidate tile: " + tile.id);
            }
        }
    }
}