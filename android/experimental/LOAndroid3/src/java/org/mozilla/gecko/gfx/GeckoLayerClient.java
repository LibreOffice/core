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

import android.content.Context;
import android.graphics.PointF;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;

import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.mozilla.gecko.util.FloatUtils;

import java.util.List;

public class GeckoLayerClient {
    private static final String LOGTAG = "GeckoLayerClient";
    private static final long MIN_VIEWPORT_CHANGE_DELAY = 25L;

    private static final IntSize TILE_SIZE = new IntSize(256, 256);

    protected IntSize mScreenSize;
    protected Layer mTileLayer;
    /* The viewport that Gecko is currently displaying. */
    protected ViewportMetrics mGeckoViewport;
    /* The viewport that Gecko will display when drawing is finished */
    protected ViewportMetrics mNewGeckoViewport;
    protected LayerController mLayerController;
    private Context mContext;
    private long mLastViewportChangeTime;
    private boolean mPendingViewportAdjust;
    private boolean mViewportSizeChanged;
    // mUpdateViewportOnEndDraw is used to indicate that we received a
    // viewport update notification while drawing. therefore, when the
    // draw finishes, we need to update the entire viewport rather than
    // just the page size. this boolean should always be accessed from
    // inside a transaction, so no synchronization is needed.
    private boolean mUpdateViewportOnEndDraw;
    private String mLastCheckerboardColor;

    public GeckoLayerClient(Context context) {
        mContext = context;
        mScreenSize = new IntSize(0, 0);
    }

    protected boolean setupLayer() {
        Log.i(LOGTAG, "Creating MultiTileLayer");
        if (mTileLayer == null) {
            mTileLayer = new MultiTileLayer(TILE_SIZE);
            mLayerController.setRoot(mTileLayer);
        }

        // Force a resize event to be sent because the results of this
        // are different depending on what tile system we're using
        //sendResizeEventIfNecessary(true);

        return false;
    }

    protected void updateLayerAfterDraw() {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer) mTileLayer).invalidate();
        }
    }

    protected IntSize getTileSize() {
        return TILE_SIZE;
    }

    /**
     * Attaches the root layer to the layer controller so that Gecko appears.
     */
    public void setLayerController(LayerController layerController) {
        mLayerController = layerController;

        layerController.setRoot(mTileLayer);
        if (mGeckoViewport != null) {
            layerController.setViewportMetrics(mGeckoViewport);
        }

        sendResizeEventIfNecessary(false);
    }

    public boolean beginDrawing(ViewportMetrics viewportMetrics) {
        if (setupLayer()) {
            Log.e(LOGTAG, "### Cancelling due to layer setup");
            return false;
        }
        mNewGeckoViewport = viewportMetrics;
        mTileLayer.beginTransaction();

        return true;
    }

    public void endDrawing() {
        synchronized (mLayerController) {
            try {
                updateViewport(!mUpdateViewportOnEndDraw);
                mUpdateViewportOnEndDraw = false;
                updateLayerAfterDraw();
            } finally {
                mTileLayer.endTransaction();
            }
        }
        Log.i(LOGTAG, "zerdatime " + SystemClock.uptimeMillis() + " - endDrawing");
    }

    protected void updateViewport(boolean onlyUpdatePageSize) {
        // save and restore the viewport size stored in java; never let the
        // JS-side viewport dimensions override the java-side ones because
        // java is the One True Source of this information, and allowing JS
        // to override can lead to race conditions where this data gets clobbered.
        FloatSize viewportSize = mLayerController.getViewportSize();
        mGeckoViewport = mNewGeckoViewport;
        mGeckoViewport.setSize(viewportSize);

        PointF displayportOrigin = mGeckoViewport.getDisplayportOrigin();
        mTileLayer.setOrigin(PointUtils.round(displayportOrigin));
        mTileLayer.setResolution(mGeckoViewport.getZoomFactor());

        Log.e(LOGTAG, "### updateViewport onlyUpdatePageSize=" + onlyUpdatePageSize + " getTileViewport " + mGeckoViewport);

        if (onlyUpdatePageSize) {
            // Don't adjust page size when zooming unless zoom levels are
            // approximately equal.
            if (FloatUtils.fuzzyEquals(mLayerController.getZoomFactor(), mGeckoViewport.getZoomFactor())) {
                mLayerController.setPageSize(mGeckoViewport.getPageSize());
            }
        } else {
            mLayerController.setViewportMetrics(mGeckoViewport);
            mLayerController.abortPanZoomAnimation();
        }
    }

    /* Informs Gecko that the screen size has changed. */
    protected void sendResizeEventIfNecessary(boolean force) {
        Log.e(LOGTAG, "### sendResizeEventIfNecessary " + force);

        DisplayMetrics metrics = new DisplayMetrics();
        LibreOfficeMainActivity.mAppContext.getWindowManager().getDefaultDisplay().getMetrics(metrics);

        IntSize newScreenSize = new IntSize(metrics.widthPixels, metrics.heightPixels);

        // Return immediately if the screen size hasn't changed or the viewport
        // size is zero (which indicates that the rendering surface hasn't been
        // allocated yet).
        boolean screenSizeChanged = !mScreenSize.equals(newScreenSize);
        boolean viewportSizeValid = (mLayerController != null && mLayerController.getViewportSize().isPositive());

        if (!(force || (screenSizeChanged && viewportSizeValid))) {
            return;
        }

        mScreenSize = newScreenSize;

        if (screenSizeChanged) {
            Log.d(LOGTAG, "Screen-size changed to " + mScreenSize);
        }

        IntSize tileSize = getTileSize();
        LOEvent event = LOEvent.sizeChanged(metrics.widthPixels, metrics.heightPixels, tileSize.width, tileSize.height);
        LOKitShell.sendEvent(event);
    }

    public void render() {
        adjustViewportWithThrottling();
    }

    private void adjustViewportWithThrottling() {
        if (!mLayerController.getRedrawHint())
            return;

        if (mPendingViewportAdjust)
            return;

        long timeDelta = System.currentTimeMillis() - mLastViewportChangeTime;
        if (timeDelta < MIN_VIEWPORT_CHANGE_DELAY) {
            mLayerController.getView().postDelayed(
                    new Runnable() {
                        public void run() {
                            mPendingViewportAdjust = false;
                            adjustViewport();
                        }
                    }, MIN_VIEWPORT_CHANGE_DELAY - timeDelta
            );
            mPendingViewportAdjust = true;
            return;
        }

        adjustViewport();
    }

    public void viewportSizeChanged() {
        mViewportSizeChanged = true;
    }

    private void adjustViewport() {
        ViewportMetrics viewportMetrics = new ViewportMetrics(mLayerController.getViewportMetrics());

        viewportMetrics.setViewport(viewportMetrics.getClampedViewport());

        LOKitShell.sendEvent(LOEvent.viewport(viewportMetrics));
        if (mViewportSizeChanged) {
            mViewportSizeChanged = false;
            LOKitShell.viewSizeChanged();
        }

        mLastViewportChangeTime = System.currentTimeMillis();
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary();
        render();
    }

    public ViewportMetrics getGeckoViewportMetrics() {
        if (mGeckoViewport != null)
            return new ViewportMetrics(mGeckoViewport);
        return null;
    }

    private void sendResizeEventIfNecessary() {
        sendResizeEventIfNecessary(false);
    }

    public void addTile(SubTile tile) {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer) mTileLayer).addTile(tile);
        }
    }

    public List<SubTile> getTiles() {
        if (mTileLayer instanceof MultiTileLayer) {
            return ((MultiTileLayer) mTileLayer).getTiles();
        }
        return null;
    }
}