/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
* ***** BEGIN LICENSE BLOCK *****
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License Version
* 1.1 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is Mozilla Android code.
*
* The Initial Developer of the Original Code is Mozilla Foundation.
* Portions created by the Initial Developer are Copyright (C) 2011-2012
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*   Chris Lord <chrislord.net@gmail.com>
*   Arkady Blyakher <rkadyb@mit.edu>
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the MPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the MPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

package org.mozilla.gecko.gfx;

import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.util.Log;

import org.libreoffice.TileProvider;
import org.mozilla.gecko.util.FloatUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Encapsulates the logic needed to draw a layer made of multiple tiles.
 */
public class MultiTileLayer extends Layer {
    private static final String LOGTAG = "MultiTileLayer";

    private static int TILE_SIZE = 256;
    private final List<SubTile> mTiles;
    private TileProvider tileProvider;
    private float currentZoomFactor;
    private RectF tileViewPort = new RectF();
    private FloatSize currentPageSize = new FloatSize(0, 0);
    private boolean shouldRefreshZoom = true;

    public MultiTileLayer() {
        super();
        mTiles = new CopyOnWriteArrayList<SubTile>();
    }

    public void invalidate() {
        for (SubTile layer : mTiles) {
            layer.invalidate();
        }
    }

    private void validateTiles() {
        // Set tile origins and resolution
        Point origin = new Point();
        refreshTileMetrics(origin, getResolution(), false);
    }

    @Override
    protected void performUpdates(RenderContext context) {
        super.performUpdates(context);

        validateTiles();

        // Iterate over the tiles and decide which ones we'll be drawing
        int dirtyTiles = 0;
        boolean screenUpdateDone = false;
        SubTile firstDirtyTile = null;
        for (SubTile layer : mTiles) {
            // First do a non-texture update to make sure coordinates are
            // up-to-date.
            layer.performUpdates(context);

            RectF layerBounds = layer.getBounds(context);

            if (!RectF.intersects(layerBounds, context.viewport)) {
                if (firstDirtyTile == null) {
                    firstDirtyTile = layer;
                }
                dirtyTiles++;
            } else {
                // This tile intersects with the screen and is dirty,
                // update it immediately.
                screenUpdateDone = true;
                layer.performUpdates(context);
            }
        }

        // Now if no tiles that intersect with the screen were updated, update
        // a single tile that doesn't (if there are any). This has the effect
        // of spreading out non-critical texture upload over time, and smoothing
        // upload-related hitches.
        if (!screenUpdateDone && firstDirtyTile != null) {
            firstDirtyTile.performUpdates(context);
            dirtyTiles--;
        }

    }

    private void refreshTileMetrics(Point origin, float resolution, boolean inTransaction) {
        for (SubTile layer : mTiles) {
            if (!inTransaction) {
                layer.beginTransaction();
            }

            if (origin != null) {
                Rect position = layer.getPosition();
                int positionX = origin.x + Math.round(layer.x / layer.zoom);
                int positionY = origin.y + Math.round(layer.y / layer.zoom);
                int tileSize = Math.round(256.0f / layer.zoom);
                position.set(positionX, positionY, positionX + tileSize, positionY + tileSize);
                layer.setPosition(position);
            }
            if (resolution >= 0.0f) {
                layer.setResolution(resolution);
            }

            if (!inTransaction) {
                layer.endTransaction();
            }
        }
    }

    @Override
    public void setResolution(float newResolution) {
        super.setResolution(newResolution);
        refreshTileMetrics(null, newResolution, true);
    }

    @Override
    public void beginTransaction() {
        super.beginTransaction();

        for (SubTile layer : mTiles) {
            layer.beginTransaction();
        }
    }

    @Override
    public void endTransaction() {
        for (SubTile layer : mTiles) {
            layer.endTransaction();
        }
        super.endTransaction();
    }

    private RectF normlizeRect(RectF rect, FloatSize pageSize) {
        return new RectF(rect.left / pageSize.width, rect.top / pageSize.height, rect.right / pageSize.width, rect.bottom / pageSize.height);
    }

    private RectF roundToTileSize(RectF input, int tileSize) {
        float minX = (Math.round(input.left) / tileSize) * tileSize;
        float minY = (Math.round(input.top) / tileSize) * tileSize;
        float maxX = ((Math.round(input.right) / tileSize) + 1) * tileSize;
        float maxY = ((Math.round(input.bottom) / tileSize) + 1) * tileSize;
        return new RectF(minX, minY, maxX, maxY);
    }

    private RectF inflate(RectF rect, float inflateSize) {
        RectF newRect = new RectF(rect);
        newRect.left -= inflateSize;
        newRect.left = newRect.left < 0.0f ? 0.0f : newRect.left;

        newRect.top -= inflateSize;
        newRect.top = newRect.top < 0.0f ? 0.0f : newRect.top;

        newRect.right += inflateSize;
        newRect.bottom += inflateSize;

        return newRect;
    }

    @Override
    public void draw(RenderContext context) {
        if (tileProvider == null) {
            return;
        }

        currentPageSize = context.pageSize;

        for (SubTile layer : mTiles) {
            // Avoid work, only draw tiles that intersect with the viewport
            RectF layerBounds = layer.getBounds(context);

            if (RectF.intersects(layerBounds, context.viewport)) {
                layer.draw(context);
            }
        }
    }

    @Override
    public Region getValidRegion(RenderContext context) {
        Region validRegion = new Region();
        for (SubTile tile : mTiles) {
            validRegion.op(tile.getValidRegion(context), Region.Op.UNION);
        }

        return validRegion;
    }

    public void setTileProvider(TileProvider tileProvider) {
        this.tileProvider = tileProvider;
    }

    public void reevaluateTiles(ImmutableViewportMetrics viewportMetrics) {
        if (currentZoomFactor != viewportMetrics.zoomFactor) {
            currentZoomFactor = viewportMetrics.zoomFactor;
        }

        RectF newTileViewPort = inflate(roundToTileSize(viewportMetrics.getViewport(), TILE_SIZE), TILE_SIZE);

        Log.i(LOGTAG, "reevaluateTiles ( " + viewportMetrics + " )");

        if (tileViewPort != newTileViewPort) {
            tileViewPort = newTileViewPort;
            cleanTiles();
            addNewTiles();
            markTiles();
        }
    }

    private void cleanTiles() {
        List<SubTile> tilesToRemove = new ArrayList<SubTile>();
        for(SubTile tile : mTiles) {
            if (tile.markedForRemoval) {
                tile.destroy();
                tilesToRemove.add(tile);
            }
        }
        mTiles.removeAll(tilesToRemove);
    }

    private void addNewTiles() {
        for (float y = tileViewPort.top; y < tileViewPort.bottom; y += TILE_SIZE) {
            if (y > currentPageSize.height) {
                continue;
            }
            for (float x = tileViewPort.left; x < tileViewPort.right; x += TILE_SIZE) {
                if (x > currentPageSize.width) {
                    continue;
                }
                boolean contains = false;
                for (SubTile tile : mTiles) {
                    if (tile.x == x && tile.y == y) {
                        contains = true;
                    }
                }
                if (!contains) {
                    CairoImage image = tileProvider.createTile(x, y, currentZoomFactor);
                    SubTile tile = new SubTile(image, (int)x, (int)y, currentZoomFactor);
                    tile.beginTransaction();
                    mTiles.add(tile);
                }
            }
        }
    }

    private void markTiles() {
        for (SubTile tile : mTiles) {
            if (FloatUtils.fuzzyEquals(tile.zoom, currentZoomFactor)) {
                RectF tileRect = new RectF(tile.x, tile.y, tile.x + TILE_SIZE, tile.y + TILE_SIZE);
                if (!RectF.intersects(tileViewPort, tileRect)) {
                    tile.markForRemoval();
                }
            } else {
                tile.markForRemoval();
            }
        }
    }
}

