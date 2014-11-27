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
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.GestureDetector;

import org.libreoffice.LOEvent;
import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.libreoffice.TileProvider;
import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.ui.PanZoomController;
import org.mozilla.gecko.ui.PanZoomTarget;
import org.mozilla.gecko.ui.SimpleScaleGestureDetector;
import org.mozilla.gecko.util.FloatUtils;

public class GeckoLayerClient implements PanZoomTarget, LayerView.Listener {
    private static final String LOGTAG = GeckoLayerClient.class.getSimpleName();

    private LayerRenderer mLayerRenderer;
    private boolean mLayerRendererInitialized;

    private Context mContext;
    private IntSize mScreenSize;
    private IntSize mWindowSize;
    private DisplayPortMetrics mDisplayPort;

    private boolean mRecordDrawTimes;
    private DrawTimingQueue mDrawTimingQueue;

    private DynamicTileLayer mRootLayer;

    /* The Gecko viewport as per the UI thread. Must be touched only on the UI thread. */
    private ImmutableViewportMetrics mGeckoViewport;

    /*
     * The viewport metrics being used to draw the current frame. This is only
     * accessed by the compositor thread, and so needs no synchronisation.
     */
    private ImmutableViewportMetrics mFrameMetrics;

    private ImmutableViewportMetrics mNewGeckoViewport;

    private boolean mPendingViewportAdjust;

    /* The current viewport metrics.
    * This is volatile so that we can read and write to it from different threads.
    * We avoid synchronization to make getting the viewport metrics from
    * the compositor as cheap as possible. The viewport is immutable so
    * we don't need to worry about anyone mutating it while we're reading from it.
    * Specifically:
    * 1) reading mViewportMetrics from any thread is fine without synchronization
    * 2) writing to mViewportMetrics requires synchronizing on the layer controller object
    * 3) whenver reading multiple fields from mViewportMetrics without synchronization (i.e. in
    * case 1 above) you should always frist grab a local copy of the reference, and then use
    * that because mViewportMetrics might get reassigned in between reading the different
    * fields. */
    private volatile ImmutableViewportMetrics mViewportMetrics;

    private ZoomConstraints mZoomConstraints;

    private boolean mGeckoIsReady;

    /* The new color for the checkerboard. */
    private int mCheckerboardColor = Color.WHITE;
    private boolean mCheckerboardShouldShowChecks;

    private final PanZoomController mPanZoomController;

    private LayerView mView;

    private boolean mForceRedraw;

    public GeckoLayerClient(Context context) {
        mContext = context;
        mScreenSize = new IntSize(0, 0);
        mWindowSize = new IntSize(0, 0);
        mDisplayPort = new DisplayPortMetrics();
        mRecordDrawTimes = false;
        mDrawTimingQueue = new DrawTimingQueue();

        mForceRedraw = true;
        DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
        mViewportMetrics = new ImmutableViewportMetrics(displayMetrics);
        mZoomConstraints = new ZoomConstraints(false);
        mCheckerboardShouldShowChecks = true;

        mPanZoomController = new PanZoomController(this);
    }

    public void setView(LayerView v) {
        mView = v;
        mView.connect(this);
    }

    public void notifyReady() {
        mGeckoIsReady = true;
        mRootLayer = new DynamicTileLayer();

        mLayerRenderer = new LayerRenderer(mView);

        mView.setListener(this);
        mView.setLayerRenderer(mLayerRenderer);
        sendResizeEventIfNecessary(true);
    }

    DisplayPortMetrics getDisplayPort() {
        return mDisplayPort;
    }

    public void beginDrawing() {
        mRootLayer.beginTransaction();

    }

    public void endDrawing(ImmutableViewportMetrics viewportMetrics) {
        synchronized (this) {
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
        FloatSize viewportSize = getViewportSize();
        mGeckoViewport = mNewGeckoViewport.setViewportSize(viewportSize.width, viewportSize.height);

        RectF position = mGeckoViewport.getViewport();
        mRootLayer.setPosition(RectUtils.round(position));
        mRootLayer.setResolution(mGeckoViewport.zoomFactor);

        if (onlyUpdatePageSize) {
            // Don't adjust page size when zooming unless zoom levels are
            // approximately equal.
            if (FloatUtils.fuzzyEquals(getViewportMetrics().zoomFactor, mGeckoViewport.zoomFactor)) {
                setPageRect(mGeckoViewport.getPageRect(), mGeckoViewport.getCssPageRect());
            }
        } else {
            setViewportMetrics(mGeckoViewport);
            abortPanZoomAnimation();
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
        ImmutableViewportMetrics metrics = getViewportMetrics();

        ImmutableViewportMetrics clampedMetrics = metrics.clamp();

        if (displayPort == null) {
            displayPort = DisplayPortCalculator.calculate(metrics, getPanZoomController().getVelocityVector());
        }

        mDisplayPort = displayPort;
        mGeckoViewport = clampedMetrics;

        if (mRecordDrawTimes) {
            mDrawTimingQueue.add(displayPort);
        }

        LOKitShell.sendEvent(LOEventFactory.viewport(clampedMetrics));
    }

    /**
     * This function is invoked by Gecko via JNI; be careful when modifying signature.
     * The compositor invokes this function whenever it determines that the page size
     * has changed (based on the information it gets from layout). If setFirstPaintViewport
     * is invoked on a frame, then this function will not be. For any given frame, this
     * function will be invoked before syncViewportInfo.
     */
    public void setPageSize(float zoom, float pageWidth, float pageHeight, float cssPageWidth, float cssPageHeight) {
        synchronized (this) {
            // adjust the page dimensions to account for differences in zoom
            // between the rendered content (which is what the compositor tells us)
            // and our zoom level (which may have diverged).
            RectF pageRect = new RectF(0.0f, 0.0f, pageWidth, pageHeight);
            RectF cssPageRect = new RectF(0.0f, 0.0f, cssPageWidth, cssPageHeight);
            float ourZoom = getViewportMetrics().zoomFactor;
            setPageRect(RectUtils.scale(pageRect, ourZoom / zoom), cssPageRect);
            // Here the page size of the document has changed, but the document being displayed
            // is still the same. Therefore, we don't need to send anything to browser.js; any
            // changes we need to make to the display port will get sent the next time we call
            // adjustViewport().
        }
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary(false);
        if (getRedrawHint()) {
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
        mRootLayer.reevaluateTiles(getViewportMetrics());
    }

    public void clearAndResetlayers() {
        mRootLayer.clearAndReset();
    }

    public void destroy() {
        mPanZoomController.destroy();
    }

    public void setForceRedraw() {
        mForceRedraw = true;
        notifyLayerClientOfGeometryChange();
    }

    public LayerView getView() {
        return mView;
    }

    public Context getContext() {
        return mContext;
    }

    public ImmutableViewportMetrics getViewportMetrics() {
        return mViewportMetrics;
    }

    /**
     * Sets the entire viewport metrics at once.
     * You must hold the monitor while calling this.
     */
    public void setViewportMetrics(ImmutableViewportMetrics viewport) {
        mViewportMetrics = viewport;
        mView.requestRender();
        notifyLayerClientOfGeometryChange();
    }

    public Object getLock() {
        return this;
    }

    public FloatSize getViewportSize() {
        return mViewportMetrics.getSize();
    }

    /**
     * The view calls this function to indicate that the viewport changed size. It must hold the
     * monitor while calling it.
     * <p/>
     * TODO: Refactor this to use an interface. Expose that interface only to the view and not
     * to the layer client. That way, the layer client won't be tempted to call this, which might
     * result in an infinite loop.
     */
    public void setViewportSize(FloatSize size) {
        mViewportMetrics = mViewportMetrics.setViewportSize(size.width, size.height);
        viewportSizeChanged();
    }

    public PanZoomController getPanZoomController() {
        return mPanZoomController;
    }

    public GestureDetector.OnGestureListener getGestureListener() {
        return mPanZoomController;
    }

    public SimpleScaleGestureDetector.SimpleScaleGestureListener getScaleGestureListener() {
        return mPanZoomController;
    }

    public GestureDetector.OnDoubleTapListener getDoubleTapListener() {
        return mPanZoomController;
    }

    /**
     * Sets the current page rect. You must hold the monitor while calling this.
     */
    public void setPageRect(RectF rect, RectF cssRect) {
        // Since the "rect" is always just a multiple of "cssRect" we don't need to
        // check both; this function assumes that both "rect" and "cssRect" are relative
        // the zoom factor in mViewportMetrics.
        if (mViewportMetrics.getCssPageRect().equals(cssRect))
            return;

        mViewportMetrics = mViewportMetrics.setPageRect(rect, cssRect);

        // Page size is owned by the layer client, so no need to notify it of
        // this change.

        mView.post(new Runnable() {
            public void run() {
                mPanZoomController.pageRectUpdated();
                mView.requestRender();
            }
        });
    }

    public void setAnimationTarget(ImmutableViewportMetrics viewport) {
        // We know what the final viewport of the animation is going to be, so
        // immediately request a draw of that area by setting the display port
        // accordingly. This way we should have the content pre-rendered by the
        // time the animation is done.
        DisplayPortMetrics displayPort = DisplayPortCalculator.calculate(viewport, null);
        adjustViewport(displayPort);
    }

    public boolean post(Runnable action) {
        return mView.post(action);
    }

    private void notifyLayerClientOfGeometryChange() {
        geometryChanged();
    }

    /**
     * Aborts any pan/zoom animation that is currently in progress.
     */
    public void abortPanZoomAnimation() {
        if (mPanZoomController != null) {
            mView.post(new Runnable() {
                public void run() {
                    mPanZoomController.abortAnimation();
                }
            });
        }
    }

    /**
     * Returns true if this controller is fine with performing a redraw operation or false if it
     * would prefer that the action didn't take place.
     */
    public boolean getRedrawHint() {
        if (mForceRedraw) {
            mForceRedraw = false;
            return true;
        }

        if (!mPanZoomController.getRedrawHint()) {
            return false;
        }

        return DisplayPortCalculator.aboutToCheckerboard(mViewportMetrics, mPanZoomController.getVelocityVector(), getDisplayPort());
    }

    /**
     * Converts a point from layer view coordinates to layer coordinates. In other words, given a
     * point measured in pixels from the top left corner of the layer view, returns the point in
     * pixels measured from the last scroll position we sent to Gecko, in CSS pixels. Assuming the
     * events being sent to Gecko are processed in FIFO order, this calculation should always be
     * correct.
     */
    public PointF convertViewPointToLayerPoint(PointF viewPoint) {
        ImmutableViewportMetrics viewportMetrics = mViewportMetrics;
        PointF origin = viewportMetrics.getOrigin();
        float zoom = viewportMetrics.zoomFactor;
        ImmutableViewportMetrics geckoViewport = getGeckoViewportMetrics();
        PointF geckoOrigin = geckoViewport.getOrigin();
        float geckoZoom = geckoViewport.zoomFactor;

        // viewPoint + origin gives the coordinate in device pixels from the top-left corner of the page.
        // Divided by zoom, this gives us the coordinate in CSS pixels from the top-left corner of the page.
        // geckoOrigin / geckoZoom is where Gecko thinks it is (scrollTo position) in CSS pixels from
        // the top-left corner of the page. Subtracting the two gives us the offset of the viewPoint from
        // the current Gecko coordinate in CSS pixels.
        PointF layerPoint = new PointF(
                ((viewPoint.x + origin.x) / zoom) - (geckoOrigin.x / geckoZoom),
                ((viewPoint.y + origin.y) / zoom) - (geckoOrigin.y / geckoZoom));

        return layerPoint;
    }

    /**
     * Retrieves whether we should show checkerboard checks or not.
     */
    public boolean checkerboardShouldShowChecks() {
        return mCheckerboardShouldShowChecks;
    }

    /**
     * Retrieves the color that the checkerboard should be.
     */
    public int getCheckerboardColor() {
        return mCheckerboardColor;
    }

    /**
     * Sets a new color for the checkerboard.
     */
    public void setCheckerboardColor(int newColor) {
        mCheckerboardColor = newColor;
        mView.requestRender();
    }

    /**
     * Sets whether or not the checkerboard should show checkmarks.
     */
    public void setCheckerboardShowChecks(boolean showChecks) {
        mCheckerboardShouldShowChecks = showChecks;
        mView.requestRender();
    }

    public ZoomConstraints getZoomConstraints() {
        return mZoomConstraints;
    }

    public void setZoomConstraints(ZoomConstraints constraints) {
        mZoomConstraints = constraints;
    }

    private class AdjustRunnable implements Runnable {
        public void run() {
            mPendingViewportAdjust = false;
            adjustViewport(null);
        }
    }

    public Layer getRoot() {
        return mRootLayer;
    }
}