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
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.libreoffice.LOEvent;
import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.TileProvider;
import org.mozilla.gecko.util.FloatUtils;

public class GeckoLayerClient implements LayerView.Listener {
    private static final String LOGTAG = "GeckoLayerClient";

    private LayerController mLayerController;

    private IntSize mScreenSize;
    private IntSize mWindowSize;
    private DisplayPortMetrics mDisplayPort;
    private boolean mRecordDrawTimes;
    private DrawTimingQueue mDrawTimingQueue;

    private DynamicTileLayer mRootLayer;

    /* The viewport that Gecko is currently displaying. */
    private ImmutableViewportMetrics mGeckoViewport;

    /* The viewport that Gecko will display when drawing is finished */
    private ImmutableViewportMetrics mNewGeckoViewport;
    private Context mContext;
    private boolean mPendingViewportAdjust;

    public GeckoLayerClient(Context context) {
        mContext = context;
        mScreenSize = new IntSize(0, 0);
        mWindowSize = new IntSize(0, 0);
        mDisplayPort = new DisplayPortMetrics();
        mRecordDrawTimes = true;
        mDrawTimingQueue = new DrawTimingQueue();
    }

    /** Attaches the root layer to the layer controller so that Gecko appears. */
    public void setLayerController(LayerController layerController) {
        LayerView view = layerController.getView();

        mLayerController = layerController;

        mRootLayer = new DynamicTileLayer();

        view.setListener(this);
        layerController.setRoot(mRootLayer);

        if (mGeckoViewport != null) {
            layerController.setViewportMetrics(mGeckoViewport);
        }

        sendResizeEventIfNecessary(true);
    }

    DisplayPortMetrics getDisplayPort() {
        return mDisplayPort;
    }

    public void beginDrawing() {
        mRootLayer.beginTransaction();

    }

    public void endDrawing(ImmutableViewportMetrics viewportMetrics) {
        synchronized (mLayerController) {
            try {
                mNewGeckoViewport = viewportMetrics;
                updateViewport(true);
                mRootLayer.invalidate();
            } finally {
                mRootLayer.endTransaction();
            }
        }
    }

    protected void updateViewport(boolean onlyUpdatePageSize) {
        // save and restore the viewport size stored in java; never let the
        // JS-side viewport dimensions override the java-side ones because
        // java is the One True Source of this information, and allowing JS
        // to override can lead to race conditions where this data gets clobbered.
        FloatSize viewportSize = mLayerController.getViewportSize();
        mGeckoViewport = mNewGeckoViewport.setViewportSize(viewportSize.width, viewportSize.height);

        RectF position = mGeckoViewport.getViewport();
        mRootLayer.setPosition(RectUtils.round(position));
        mRootLayer.setResolution(mGeckoViewport.zoomFactor);

        if (onlyUpdatePageSize) {
            // Don't adjust page size when zooming unless zoom levels are
            // approximately equal.
            if (FloatUtils.fuzzyEquals(mLayerController.getViewportMetrics().zoomFactor, mGeckoViewport.zoomFactor)) {
                mLayerController.setPageRect(mGeckoViewport.getPageRect(), mGeckoViewport.getCssPageRect());
            }
        } else {
            mLayerController.setViewportMetrics(mGeckoViewport);
            mLayerController.abortPanZoomAnimation();
        }
    }

    /* Informs Gecko that the screen size has changed. */
    private void sendResizeEventIfNecessary(boolean force) {
        DisplayMetrics metrics = LOKitShell.getDisplayMetrics();

        IntSize newScreenSize = new IntSize(metrics.widthPixels, metrics.heightPixels);

        // Return immediately if the screen size hasn't changed or the viewport
        // size is zero (which indicates that the rendering surface hasn't been
        // allocated yet).
        boolean screenSizeChanged = !mScreenSize.equals(newScreenSize);

        if (!force && !screenSizeChanged) {
            return;
        }

        Log.d(LOGTAG, "Screen-size changed to " + mScreenSize + " - > " + newScreenSize);

        mScreenSize = newScreenSize;

        LOEvent event = LOEventFactory.sizeChanged(metrics.widthPixels, metrics.heightPixels);
        LOKitShell.sendEvent(event);
    }

    public void viewportSizeChanged() {
        sendResizeEventIfNecessary(true);
    }

    void adjustViewport(DisplayPortMetrics displayPort) {
        ImmutableViewportMetrics metrics = mLayerController.getViewportMetrics();

        ImmutableViewportMetrics clampedMetrics = metrics.clamp();

        if (displayPort == null) {
            displayPort = DisplayPortCalculator.calculate(metrics,
                    mLayerController.getPanZoomController().getVelocityVector());
        }

        mDisplayPort = displayPort;
        mGeckoViewport = clampedMetrics;

        if (mRecordDrawTimes) {
            mDrawTimingQueue.add(displayPort);
        }

        LOKitShell.sendEvent(LOEventFactory.viewport(clampedMetrics));
    }

    /** This function is invoked by Gecko via JNI; be careful when modifying signature.
     * The compositor invokes this function whenever it determines that the page size
     * has changed (based on the information it gets from layout). If setFirstPaintViewport
     * is invoked on a frame, then this function will not be. For any given frame, this
     * function will be invoked before syncViewportInfo.
     */
    public void setPageSize(float zoom, float pageWidth, float pageHeight, float cssPageWidth, float cssPageHeight) {
        synchronized (mLayerController) {
            // adjust the page dimensions to account for differences in zoom
            // between the rendered content (which is what the compositor tells us)
            // and our zoom level (which may have diverged).
            RectF pageRect = new RectF(0.0f, 0.0f, pageWidth, pageHeight);
            RectF cssPageRect = new RectF(0.0f, 0.0f, cssPageWidth, cssPageHeight);
            float ourZoom = mLayerController.getViewportMetrics().zoomFactor;
            mLayerController.setPageRect(RectUtils.scale(pageRect, ourZoom / zoom), cssPageRect);
            // Here the page size of the document has changed, but the document being displayed
            // is still the same. Therefore, we don't need to send anything to browser.js; any
            // changes we need to make to the display port will get sent the next time we call
            // adjustViewport().
        }
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary(false);
        if (mLayerController.getRedrawHint()) {
            adjustViewport(null);
        }
    }

    public ImmutableViewportMetrics getGeckoViewportMetrics() {
        return mGeckoViewport;
    }

    @Override
    public void compositorCreated() {
    }

    @Override
    public void renderRequested() {
    }

    @Override
    public void compositionPauseRequested() {
    }

    @Override
    public void compositionResumeRequested(int width, int height) {
    }

    @Override
    public void surfaceChanged(int width, int height) {
        renderRequested();
    }

    public void setTileProvider(TileProvider tileProvider) {
        mRootLayer.setTileProvider(tileProvider);
    }

    public void reevaluateTiles() {
        mRootLayer.reevaluateTiles(mLayerController.getViewportMetrics());
    }

    public void clearAndResetlayers() {
        mRootLayer.clearAndReset();
    }

    private class AdjustRunnable implements Runnable {
        public void run() {
            mPendingViewportAdjust = false;
            adjustViewport(null);
        }
    }
}