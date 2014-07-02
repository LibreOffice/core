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
 * Portions created by the Initial Developer are Copyright (C) 2009-2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Patrick Walton <pcwalton@mozilla.com>
 *   Chris Lord <chrislord.net@gmail.com>
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

import org.libreoffice.LOKitShell;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.LayerClient;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.LayerRenderer;
import org.mozilla.gecko.gfx.MultiTileLayer;
import org.mozilla.gecko.gfx.PointUtils;
import org.mozilla.gecko.gfx.WidgetTileLayer;
//import org.mozilla.gecko.GeckoAppShell;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;
import java.nio.ByteBuffer;

/**
 * Transfers a software-rendered Gecko to an ImageLayer so that it can be rendered by our
 * compositor.
 *
 * TODO: Throttle down Gecko's priority when we pan and zoom.
 */
public class GeckoSoftwareLayerClient extends GeckoLayerClient {
    private static final String LOGTAG = "GeckoSoftwareLayerClient";

    private int mFormat;
    private IntSize mViewportSize;
    private IntSize mBufferSize;
    private static final IntSize TILE_SIZE = new IntSize(256, 256);

    public GeckoSoftwareLayerClient(Context context) {
        super(context);
        mBufferSize = new IntSize(0,0);
        mFormat = CairoImage.FORMAT_ARGB32;
    }

    /*protected void finalize() throws Throwable {
        try {
            if (mBuffer != null)
                LOKitShell.freeDirectBuffer(mBuffer);
            mBuffer = null;
        } finally {
            super.finalize();
        }
    }*/

    public void setLayerController(LayerController layerController) {
        super.setLayerController(layerController);

        layerController.setRoot(mTileLayer);
        if (mGeckoViewport != null) {
            layerController.setViewportMetrics(mGeckoViewport);
        }

        sendResizeEventIfNecessary(false);
    }

    @Override
    protected boolean setupLayer() {
        Log.i(LOGTAG, "Creating MultiTileLayer");
        if(mTileLayer == null)
            mTileLayer = new MultiTileLayer(TILE_SIZE);

        getLayerController().setRoot(mTileLayer);

        // Force a resize event to be sent because the results of this
        // are different depending on what tile system we're using
        sendResizeEventIfNecessary(true);

        return false;
    }

    @Override
    protected boolean shouldDrawProceed(int tileWidth, int tileHeight) {
        // Make sure the tile-size matches. If it doesn't, we could crash trying
        // to access invalid memory.
        if (tileWidth != TILE_SIZE.width || tileHeight != TILE_SIZE.height) {
            Log.e(LOGTAG, "Aborting draw, incorrect tile size of " + tileWidth + "x" + tileHeight);
            return false;
        }
        return true;
    }

    @Override
    public Rect beginDrawing(int width, int height, int tileWidth, int tileHeight, String metadata) {
        Rect bufferRect = super.beginDrawing(width, height, tileWidth, tileHeight, metadata);

        if (bufferRect == null) {
            return bufferRect;
        }

        // If the window size has changed, reallocate the buffer to match.
        if (mBufferSize.width != width || mBufferSize.height != height) {
            mBufferSize = new IntSize(width, height);
        }

        return bufferRect;
    }

    @Override
    protected void updateLayerAfterDraw(Rect updatedRect) {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer)mTileLayer).invalidate(updatedRect);
        }
    }

    /*private void copyPixelsFromMultiTileLayer(Bitmap target) {
        Canvas c = new Canvas(target);
        ByteBuffer tileBuffer = mBuffer.slice();
        int bpp = CairoUtils.bitsPerPixelForCairoFormat(mFormat) / 8;

        for (int y = 0; y < mBufferSize.height; y += TILE_SIZE.height) {
            for (int x = 0; x < mBufferSize.width; x += TILE_SIZE.width) {
                // Calculate tile size
                IntSize tileSize = new IntSize(Math.min(mBufferSize.width - x, TILE_SIZE.width),
                        Math.min(mBufferSize.height - y, TILE_SIZE.height));

                // Create a Bitmap from this tile
                Bitmap tile = Bitmap.createBitmap(tileSize.width, tileSize.height,
                        CairoUtils.cairoFormatTobitmapConfig(mFormat));
                tile.copyPixelsFromBuffer(tileBuffer.asIntBuffer());

                // Copy the tile to the master Bitmap and recycle it
                c.drawBitmap(tile, x, y, null);
                tile.recycle();

                // Progress the buffer to the next tile
                tileBuffer.position(tileSize.getArea() * bpp);
                tileBuffer = tileBuffer.slice();
            }
        }
    }*/

    @Override
    protected void tileLayerUpdated() {
        /* No-op. */
    }

    @Override
    public Bitmap getBitmap() {
        if (mTileLayer == null)
            return null;

        // Begin a tile transaction, otherwise the buffer can be destroyed while
        // we're reading from it.
        /*beginTransaction(mTileLayer);
        try {
            if (mBuffer == null || mBufferSize.width <= 0 || mBufferSize.height <= 0)
                return null;
            try {
                Bitmap b = null;

                if (mTileLayer instanceof MultiTileLayer) {
                    b = Bitmap.createBitmap(mBufferSize.width, mBufferSize.height,CairoUtils.cairoFormatTobitmapConfig(mFormat));
                    copyPixelsFromMultiTileLayer(b);
                } else {
                    Log.w(LOGTAG, "getBitmap() called on a layer (" + mTileLayer + ") we don't know how to get a bitmap from");
                }

                return b;
            } catch (OutOfMemoryError oom) {
                Log.w(LOGTAG, "Unable to create bitmap", oom);
                return null;
            }
        } finally {
            endTransaction(mTileLayer);
        }*/

        return null;
    }

    @Override
    public int getType() {
        return LAYER_CLIENT_TYPE_SOFTWARE;
    }

    @Override
    protected IntSize getBufferSize() {
        return new IntSize(
            ((mScreenSize.width + LayerController.MIN_BUFFER.width - 1) / TILE_SIZE.width + 1) * TILE_SIZE.width,
            ((mScreenSize.height + LayerController.MIN_BUFFER.height - 1) / TILE_SIZE.height + 1) * TILE_SIZE.height);
    }

    @Override
    protected IntSize getTileSize() {
        return TILE_SIZE;
    }

    public void addTile(Bitmap bitmap, int x, int y) {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer)mTileLayer).addTile(bitmap, x, y);
        }
    }
}
