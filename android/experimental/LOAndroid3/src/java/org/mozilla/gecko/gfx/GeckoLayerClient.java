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
import android.graphics.RectF;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

import org.libreoffice.LOEvent;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.mozilla.gecko.util.FloatUtils;

import java.util.List;
import java.util.regex.Pattern;

public class GeckoLayerClient implements LayerView.Listener {
    private static final String LOGTAG = "GeckoLayerClient";

    private LayerController mLayerController;
    private LayerRenderer mLayerRenderer;
    private boolean mLayerRendererInitialized;

    private IntSize mScreenSize;
    private IntSize mWindowSize;
    private DisplayPortMetrics mDisplayPort;
    private boolean mRecordDrawTimes;
    private DrawTimingQueue mDrawTimingQueue;

    private MultiTileLayer mRootLayer;

    /* The viewport that Gecko is currently displaying. */
    private ViewportMetrics mGeckoViewport;

    /* The viewport that Gecko will display when drawing is finished */
    private ViewportMetrics mNewGeckoViewport;
    private Context mContext;
    private static final long MIN_VIEWPORT_CHANGE_DELAY = 25L;
    private long mLastViewportChangeTime;
    private boolean mPendingViewportAdjust;
    private boolean mViewportSizeChanged;
    private boolean mIgnorePaintsPendingViewportSizeChange;
    private boolean mFirstPaint = true;

    // mUpdateViewportOnEndDraw is used to indicate that we received a
    // viewport update notification while drawing. therefore, when the
    // draw finishes, we need to update the entire viewport rather than
    // just the page size. this boolean should always be accessed from
    // inside a transaction, so no synchronization is needed.
    private boolean mUpdateViewportOnEndDraw;

    private String mLastCheckerboardColor;

    private static Pattern sColorPattern;

    /* Used as a temporary ViewTransform by syncViewportInfo */
    private ViewTransform mCurrentViewTransform;

    public GeckoLayerClient(Context context) {
        mContext = context;
        mScreenSize = new IntSize(0, 0);
        mWindowSize = new IntSize(0, 0);
        mDisplayPort = new DisplayPortMetrics();
        mRecordDrawTimes = true;
        mDrawTimingQueue = new DrawTimingQueue();
        mCurrentViewTransform = new ViewTransform(0, 0, 1);
    }

    /** Attaches the root layer to the layer controller so that Gecko appears. */
    public void setLayerController(LayerController layerController) {
        LayerView view = layerController.getView();

        mLayerController = layerController;

        mRootLayer = new MultiTileLayer(new IntSize(256, 256));

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

    protected void updateLayerAfterDraw() {
        mRootLayer.invalidate();
    }

    public void beginDrawing(ViewportMetrics viewportMetrics) {
        mNewGeckoViewport = viewportMetrics;
        mRootLayer.beginTransaction();
    }

    public void endDrawing() {
        synchronized (mLayerController) {
            try {
                updateViewport(!mUpdateViewportOnEndDraw);
                mUpdateViewportOnEndDraw = false;
                updateLayerAfterDraw();
            } finally {
                mRootLayer.endTransaction();
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

        PointF displayportOrigin = mGeckoViewport.getOrigin();
        RectF position = mGeckoViewport.getViewport();
        mRootLayer.setPosition(RectUtils.round(position));
        mRootLayer.setResolution(mGeckoViewport.getZoomFactor());

        Log.e(LOGTAG, "### updateViewport onlyUpdatePageSize=" + onlyUpdatePageSize + " getTileViewport " + mGeckoViewport);

        if (onlyUpdatePageSize) {
            // Don't adjust page size when zooming unless zoom levels are
            // approximately equal.
            if (FloatUtils.fuzzyEquals(mLayerController.getZoomFactor(), mGeckoViewport.getZoomFactor())) {
                mLayerController.setPageSize(mGeckoViewport.getPageSize(), mGeckoViewport.getPageSize());
            }
        } else {
            mLayerController.setViewportMetrics(mGeckoViewport);
            mLayerController.abortPanZoomAnimation();
        }
    }

    /* Informs Gecko that the screen size has changed. */
    private void sendResizeEventIfNecessary(boolean force) {
        Log.e(LOGTAG, "### sendResizeEventIfNecessary " + force);

        DisplayMetrics metrics = new DisplayMetrics();
        LibreOfficeMainActivity.mAppContext.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        View view = mLayerController.getView();

        IntSize newScreenSize = new IntSize(metrics.widthPixels, metrics.heightPixels);
        IntSize newWindowSize = new IntSize(view.getWidth(), view.getHeight());

        // Return immediately if the screen size hasn't changed or the viewport
        // size is zero (which indicates that the rendering surface hasn't been
        // allocated yet).
        boolean screenSizeChanged = !mScreenSize.equals(newScreenSize);
        boolean windowSizeChanged = !mWindowSize.equals(newWindowSize);

        if (!force && !screenSizeChanged && !windowSizeChanged) {
            return;
        }

        mScreenSize = newScreenSize;
        mWindowSize = newWindowSize;

        if (screenSizeChanged) {
            Log.d(LOGTAG, "Screen-size changed to " + mScreenSize);
        }

        if (windowSizeChanged) {
            Log.d(LOGTAG, "Window-size changed to " + mWindowSize);
        }

        LOEvent event = LOEvent.sizeChanged(metrics.widthPixels, metrics.heightPixels);
        LOKitShell.sendEvent(event);
    }

    public void viewportSizeChanged() {
        sendResizeEventIfNecessary(true);
        LOKitShell.viewSizeChanged();
    }

    void adjustViewport(DisplayPortMetrics displayPort) {
        ImmutableViewportMetrics metrics = mLayerController.getViewportMetrics();

        ViewportMetrics clampedMetrics = new ViewportMetrics(metrics);
        clampedMetrics.setViewport(clampedMetrics.getClampedViewport());

        if (displayPort == null) {
            displayPort = DisplayPortCalculator.calculate(metrics,
                    mLayerController.getPanZoomController().getVelocityVector());
        }

        mDisplayPort = displayPort;
        mGeckoViewport = clampedMetrics;

        if (mRecordDrawTimes) {
            mDrawTimingQueue.add(displayPort);
        }

        LOKitShell.sendEvent(LOEvent.viewport(clampedMetrics));
        if (mViewportSizeChanged) {
            mViewportSizeChanged = false;
            LOKitShell.viewSizeChanged();
        }
    }

    public void setPageSize(float zoom, float pageWidth, float pageHeight, float cssPageWidth, float cssPageHeight) {
        synchronized (mLayerController) {
        // adjust the page dimensions to account for differences in zoom
        // between the rendered content (which is what the compositor tells us)
        // and our zoom level (which may have diverged).
        float ourZoom = mLayerController.getZoomFactor();
        pageWidth = pageWidth * ourZoom / zoom;
        pageHeight = pageHeight * ourZoom /zoom;
        mLayerController.setPageSize(new FloatSize(pageWidth, pageHeight), new FloatSize(pageWidth, pageHeight));
        // Here the page size of the document has changed, but the document being displayed
        // is still the same. Therefore, we don't need to send anything to browser.js; any
        // changes we need to make to the display port will get sent the next time we call
        // adjustViewport().
        }
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary(false);
        if (mLayerController.getRedrawHint())
            adjustViewport(null);
    }

    public ViewportMetrics getGeckoViewportMetrics() {
        return mGeckoViewport;
    }

    public List<SubTile> getTiles() {
        return mRootLayer.getTiles();
    }

    public void addTile(SubTile tile) {
        mRootLayer.addTile(tile);
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
        compositionResumeRequested(width, height);
        renderRequested();
    }

    private class AdjustRunnable implements Runnable {
        public void run() {
            mPendingViewportAdjust = false;
            adjustViewport(null);
        }
    }
}