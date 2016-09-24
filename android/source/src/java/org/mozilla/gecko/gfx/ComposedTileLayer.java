package org.mozilla.gecko.gfx;

import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.RectF;
import android.graphics.Region;
import android.util.Log;

import org.libreoffice.LOKitShell;
import org.libreoffice.TileIdentifier;
import org.mozilla.gecko.util.FloatUtils;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public abstract class ComposedTileLayer extends Layer implements ComponentCallbacks2 {
    private static final String LOGTAG = ComposedTileLayer.class.getSimpleName();

    protected final List<SubTile> tiles = new ArrayList<SubTile>();

    protected final IntSize tileSize;
    private final ReadWriteLock tilesReadWriteLock = new ReentrantReadWriteLock();
    private final Lock tilesReadLock = tilesReadWriteLock.readLock();
    private final Lock tilesWriteLock = tilesReadWriteLock.writeLock();

    protected RectF currentViewport = new RectF();
    protected float currentZoom = 1.0f;
    protected RectF currentPageRect = new RectF();

    private long reevaluationNanoTime = 0;

    public ComposedTileLayer(Context context) {
        context.registerComponentCallbacks(this);
        this.tileSize = new IntSize(256, 256);
    }

    protected static RectF roundToTileSize(RectF input, IntSize tileSize) {
        float minX = ((int) (input.left / tileSize.width)) * tileSize.width;
        float minY = ((int) (input.top / tileSize.height)) * tileSize.height;
        float maxX = ((int) (input.right / tileSize.width) + 1) * tileSize.width;
        float maxY = ((int) (input.bottom / tileSize.height) + 1) * tileSize.height;
        return new RectF(minX, minY, maxX, maxY);
    }

    protected static RectF inflate(RectF rect, IntSize inflateSize) {
        RectF newRect = new RectF(rect);
        newRect.left -= inflateSize.width;
        newRect.left = newRect.left < 0.0f ? 0.0f : newRect.left;

        newRect.top -= inflateSize.height;
        newRect.top = newRect.top < 0.0f ? 0.0f : newRect.top;

        newRect.right += inflateSize.width;
        newRect.bottom += inflateSize.height;

        return newRect;
    }

    protected static RectF normalizeRect(RectF rect, float sourceFactor, float targetFactor) {
        RectF normalizedRect = new RectF(
                (rect.left / sourceFactor) * targetFactor,
                (rect.top / sourceFactor) * targetFactor,
                (rect.right / sourceFactor) * targetFactor,
                (rect.bottom / sourceFactor) * targetFactor);

        return normalizedRect;
    }

    public void invalidate() {
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            tile.invalidate();
        }
        tilesReadLock.unlock();
    }

    @Override
    public void beginTransaction() {
        super.beginTransaction();
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            tile.beginTransaction();
        }
        tilesReadLock.unlock();
    }

    @Override
    public void endTransaction() {
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            tile.endTransaction();
        }
        tilesReadLock.unlock();
        super.endTransaction();
    }

    @Override
    public void draw(RenderContext context) {
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            if (RectF.intersects(tile.getBounds(context), context.viewport)) {
                tile.draw(context);
            }
        }
        tilesReadLock.unlock();
    }

    @Override
    protected void performUpdates(RenderContext context) {
        super.performUpdates(context);

        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            tile.beginTransaction();
            tile.refreshTileMetrics();
            tile.endTransaction();
            tile.performUpdates(context);
        }
        tilesReadLock.unlock();
    }

    @Override
    public Region getValidRegion(RenderContext context) {
        Region validRegion = new Region();
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            validRegion.op(tile.getValidRegion(context), Region.Op.UNION);
        }
        tilesReadLock.unlock();
        return validRegion;
    }

    @Override
    public void setResolution(float newResolution) {
        super.setResolution(newResolution);
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            tile.setResolution(newResolution);
        }
        tilesReadLock.unlock();
    }

    public void reevaluateTiles(ImmutableViewportMetrics viewportMetrics, DisplayPortMetrics mDisplayPort) {
        RectF newViewPort = getViewPort(viewportMetrics);
        float newZoom = getZoom(viewportMetrics);

        // When
        if (newZoom <= 0.0 || Float.isNaN(newZoom)) {
            return;
        }

        if (currentViewport.equals(newViewPort) && FloatUtils.fuzzyEquals(currentZoom, newZoom)) {
            return;
        }

        long currentReevaluationNanoTime = System.nanoTime();
        if ((currentReevaluationNanoTime - reevaluationNanoTime) < 25 * 1000000) {
            return;
        }

        reevaluationNanoTime = currentReevaluationNanoTime;

        currentViewport = newViewPort;
        currentZoom = newZoom;
        currentPageRect = viewportMetrics.getPageRect();

        LOKitShell.sendTileReevaluationRequest(this);
    }

    protected abstract RectF getViewPort(ImmutableViewportMetrics viewportMetrics);

    protected abstract float getZoom(ImmutableViewportMetrics viewportMetrics);

    protected abstract int getTilePriority();

    private boolean containsTilesMatching(float x, float y, float currentZoom) {
        tilesReadLock.lock();
        try {
            for (SubTile tile : tiles) {
                if (tile.id.x == x && tile.id.y == y && tile.id.zoom == currentZoom) {
                    return true;
                }
            }
            return false;
        } finally {
            tilesReadLock.unlock();
        }
    }

    public void addNewTiles(List<SubTile> newTiles) {
        for (float y = currentViewport.top; y < currentViewport.bottom; y += tileSize.height) {
            if (y > currentPageRect.height()) {
                continue;
            }
            for (float x = currentViewport.left; x < currentViewport.right; x += tileSize.width) {
                if (x > currentPageRect.width()) {
                    continue;
                }
                if (!containsTilesMatching(x, y, currentZoom)) {
                    TileIdentifier tileId = new TileIdentifier((int) x, (int) y, currentZoom, tileSize);
                    SubTile tile = createNewTile(tileId);
                    newTiles.add(tile);
                }
            }
        }
    }

    public void clearMarkedTiles() {
        tilesWriteLock.lock();
        Iterator<SubTile> iterator = tiles.iterator();
        while (iterator.hasNext()) {
            SubTile tile = iterator.next();
            if (tile.markedForRemoval) {
                tile.destroy();
                iterator.remove();
            }
        }
        tilesWriteLock.unlock();
    }

    public void markTiles() {
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            if (FloatUtils.fuzzyEquals(tile.id.zoom, currentZoom)) {
                RectF tileRect = tile.id.getRectF();
                if (!RectF.intersects(currentViewport, tileRect)) {
                    tile.markForRemoval();
                }
            } else {
                tile.markForRemoval();
            }
        }
        tilesReadLock.unlock();
    }

    public void clearAndReset() {
        tilesWriteLock.lock();
        tiles.clear();
        tilesWriteLock.unlock();
        currentViewport = new RectF();
    }

    private SubTile createNewTile(TileIdentifier tileId) {
        SubTile tile = new SubTile(tileId);
        tile.beginTransaction();
        tilesWriteLock.lock();
        tiles.add(tile);
        tilesWriteLock.unlock();
        return tile;
    }

    public boolean isStillValid(TileIdentifier tileId) {
        return RectF.intersects(currentViewport, tileId.getRectF()) || currentViewport.contains(tileId.getRectF());
    }

    /**
     * Invalidate tiles which intersect the input rect
     */
    public void invalidateTiles(List<SubTile> tilesToInvalidate, RectF cssRect) {
        RectF zoomedRect = RectUtils.scale(cssRect, currentZoom);
        tilesReadLock.lock();
        for (SubTile tile : tiles) {
            if (!tile.markedForRemoval && RectF.intersects(zoomedRect, tile.id.getRectF())) {
                tilesToInvalidate.add(tile);
            }
        }
        tilesReadLock.unlock();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
    }

    @Override
    public void onLowMemory() {
        Log.i(LOGTAG, "onLowMemory");
    }

    @Override
    public void onTrimMemory(int level) {
        if (level >= 15 /*TRIM_MEMORY_RUNNING_CRITICAL*/) {
            Log.i(LOGTAG, "Trimming memory - TRIM_MEMORY_RUNNING_CRITICAL");
        } else if (level >= 10 /*TRIM_MEMORY_RUNNING_LOW*/) {
            Log.i(LOGTAG, "Trimming memory - TRIM_MEMORY_RUNNING_LOW");
        }
    }
}
