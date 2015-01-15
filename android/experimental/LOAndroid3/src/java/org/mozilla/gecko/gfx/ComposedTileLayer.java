package org.mozilla.gecko.gfx;

import android.app.ActivityManager;
import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.RectF;
import android.graphics.Region;
import android.util.Log;

import org.libreoffice.LOEvent;
import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.libreoffice.TileIdentifier;
import org.mozilla.gecko.util.FloatUtils;

import java.util.Collections;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public abstract class ComposedTileLayer extends Layer implements ComponentCallbacks2 {
    private static final String LOGTAG = ComposedTileLayer.class.getSimpleName();

    protected final ConcurrentMap<TileIdentifier, SubTile> tiles = new ConcurrentHashMap<TileIdentifier, SubTile>();
    protected final Set<TileIdentifier> newTileIds = Collections.newSetFromMap(new ConcurrentHashMap<TileIdentifier, Boolean>());

    protected final IntSize tileSize;
    protected RectF currentViewport = new RectF();
    protected float currentZoom;

    private static int getMemoryClass(Context context) {
        ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        return activityManager.getMemoryClass() * 1024 * 1024;
    }

    public ComposedTileLayer(Context context) {
        context.registerComponentCallbacks(this);
        this.tileSize = new IntSize(256, 256);
    }

    public void invalidate() {
        for (SubTile tile : tiles.values()) {
            tile.invalidate();
        }
    }

    @Override
    public void beginTransaction() {
        super.beginTransaction();
        for (SubTile tile : tiles.values()) {
            tile.beginTransaction();
        }
    }

    @Override
    public void endTransaction() {
        for (SubTile tile : tiles.values()) {
            tile.endTransaction();
        }
        super.endTransaction();
    }

    @Override
    public void draw(RenderContext context) {
        for (SubTile tile : tiles.values()) {
            if (RectF.intersects(tile.getBounds(context), context.viewport)) {
                tile.draw(context);
            }
        }
    }

    @Override
    protected void performUpdates(RenderContext context) {
        super.performUpdates(context);

        for (SubTile tile : tiles.values()) {
            tile.beginTransaction();
            tile.refreshTileMetrics();
            tile.endTransaction();
            tile.performUpdates(context);
        }
    }

    @Override
    public Region getValidRegion(RenderContext context) {
        Region validRegion = new Region();
        for (SubTile tile : tiles.values()) {
            validRegion.op(tile.getValidRegion(context), Region.Op.UNION);
        }

        return validRegion;
    }

    @Override
    public void setResolution(float newResolution) {
        super.setResolution(newResolution);
        for (SubTile tile : tiles.values()) {
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
        float newZoom = getZoom(viewportMetrics);

        if (!currentViewport.equals(newCurrentViewPort) || currentZoom != newZoom) {
            if (newZoom == 1.0f) {
                Log.i(LOGTAG, "Suspisious zoom 1.0");
            }
            currentViewport = newCurrentViewPort;
            currentZoom = newZoom;
            RectF pageRect = viewportMetrics.getPageRect();

            clearMarkedTiles();
            addNewTiles(pageRect);
            markTiles();
        }
    }

    protected abstract RectF getViewPort(ImmutableViewportMetrics viewportMetrics);

    protected abstract float getZoom(ImmutableViewportMetrics viewportMetrics);

    protected abstract int getTilePriority();

    private void addNewTiles(RectF pageRect) {
        for (float y = currentViewport.top; y < currentViewport.bottom; y += tileSize.height) {
            if (y > pageRect.height()) {
                continue;
            }
            for (float x = currentViewport.left; x < currentViewport.right; x += tileSize.width) {
                if (x > pageRect.width()) {
                    continue;
                }
                TileIdentifier tileId = new TileIdentifier((int) x, (int) y, currentZoom, tileSize);
                if (!tiles.containsKey(tileId) && !newTileIds.contains(tileId)) {
                    newTileIds.add(tileId);
                    LOEvent event = LOEventFactory.tileRequest(this, tileId, true);
                    event.mPriority = getTilePriority();
                    LOKitShell.sendEvent(event);
                }
            }
        }
    }

    private void clearMarkedTiles() {
        Iterator<Map.Entry<TileIdentifier, SubTile>> iterator;
        for (iterator = tiles.entrySet().iterator(); iterator.hasNext();) {
            SubTile tile = iterator.next().getValue();
            if (tile.markedForRemoval) {
                tile.destroy();
                iterator.remove();
            }
        }
    }

    private void markTiles() {
        for (SubTile tile : tiles.values()) {
            if (FloatUtils.fuzzyEquals(tile.id.zoom, currentZoom)) {
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
        tiles.put(tile.id, tile);
        newTileIds.remove(tile.id);
    }

    public boolean isStillValid(TileIdentifier tileId) {
        return RectF.intersects(currentViewport, tileId.getRect()) || currentViewport.contains(tileId.getRect());
    }

    /**
     * Invalidate tiles which intersect the input rect
     */
    public void invalidateTiles(RectF rect) {
        RectF zoomedRect = RectUtils.inverseScale(rect, currentZoom);

        for (SubTile tile : tiles.values()) {
            if (RectF.intersects(rect, tile.id.getRect())) {
                LOKitShell.sendEvent(LOEventFactory.tileRerender(this, tile));
            }
        }
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
        Log.i(LOGTAG, "Trimming memory " + level);
        if (level >= 10 /*TRIM_MEMORY_RUNNING_LOW*/) {
            Log.i(LOGTAG, "Trimming memory - TRIM_MEMORY_RUNNING_LOW");
        } else if (level >= 15 /*TRIM_MEMORY_RUNNING_CRITICAL*/) {
            Log.i(LOGTAG, "Trimming memory - TRIM_MEMORY_RUNNING_CRITICAL");
            //clearAndReset();
        }
    }
}