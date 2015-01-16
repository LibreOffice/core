/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.content.Context;
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.libreoffice.LOEvent;
import org.libreoffice.LOEventFactory;
import org.libreoffice.LOKitShell;
import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.util.FloatUtils;

public class GeckoLayerClient implements PanZoomTarget, LayerView.Listener {
    private static final String LOGTAG = GeckoLayerClient.class.getSimpleName();

    private LayerRenderer mLayerRenderer;
    private boolean mLayerRendererInitialized;

    private Context mContext;
    private IntSize mScreenSize;
    private IntSize mWindowSize;
    private DisplayPortMetrics mDisplayPort;
    private DisplayPortMetrics mReturnDisplayPort;

    private boolean mRecordDrawTimes;
    private final DrawTimingQueue mDrawTimingQueue;

    private ComposedTileLayer mLowResLayer;
    private ComposedTileLayer mRootLayer;

    /* The Gecko viewport as per the UI thread. Must be touched only on the UI thread.
     * If any events being sent to Gecko that are relative to the Gecko viewport position,
     * they must (a) be relative to this viewport, and (b) be sent on the UI thread to
     * avoid races. As long as these two conditions are satisfied, and the events being
     * sent to Gecko are processed in FIFO order, the events will properly be relative
     * to the Gecko viewport position. Note that if Gecko updates its viewport independently,
     * we get notified synchronously and also update this on the UI thread.
     */
    private ImmutableViewportMetrics mGeckoViewport;

    /*
     * The viewport metrics being used to draw the current frame. This is only
     * accessed by the compositor thread, and so needs no synchronisation.
     */
    private ImmutableViewportMetrics mFrameMetrics;

    private ImmutableViewportMetrics mNewGeckoViewport;

    private boolean mPendingViewportAdjust;

    private boolean mForceRedraw;

    /* The current viewport metrics.
     * This is volatile so that we can read and write to it from different threads.
     * We avoid synchronization to make getting the viewport metrics from
     * the compositor as cheap as possible. The viewport is immutable so
     * we don't need to worry about anyone mutating it while we're reading from it.
     * Specifically:
     * 1) reading mViewportMetrics from any thread is fine without synchronization
     * 2) writing to mViewportMetrics requires synchronizing on the layer controller object
     * 3) whenver reading multiple fields from mViewportMetrics without synchronization (i.e. in
     *    case 1 above) you should always frist grab a local copy of the reference, and then use
     *    that because mViewportMetrics might get reassigned in between reading the different
     *    fields. */
    private volatile ImmutableViewportMetrics mViewportMetrics;

    private ZoomConstraints mZoomConstraints;

    private boolean mGeckoIsReady;

    /* The new color for the checkerboard. */
    private int mCheckerboardColor;

    private final PanZoomController mPanZoomController;
    private LayerView mView;

    public GeckoLayerClient(Context context) {
        // we can fill these in with dummy values because they are always written
        // to before being read
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

        mPanZoomController = PanZoomController.Factory.create(this);
    }

    public void setView(LayerView v) {
        mView = v;
        mView.connect(this);
    }

    public void notifyReady() {
        mGeckoIsReady = true;

        mRootLayer = new DynamicTileLayer(mContext);
        mLowResLayer = new FixedZoomTileLayer(mContext);

        mLayerRenderer = new LayerRenderer(mView);

        mView.setListener(this);
        mView.setLayerRenderer(mLayerRenderer);
        sendResizeEventIfNecessary(true);
    }

    /**
     * Returns true if this controller is fine with performing a redraw operation or false if it
     * would prefer that the action didn't take place.
     */
    private boolean getRedrawHint() {
        if (mForceRedraw) {
            mForceRedraw = false;
            return true;
        }

        if (!mPanZoomController.getRedrawHint()) {
            return false;
        }

        return DisplayPortCalculator.aboutToCheckerboard(mViewportMetrics, mPanZoomController.getVelocityVector(), getDisplayPort());
    }

    Layer getRoot() {
        return mGeckoIsReady ? mRootLayer : null;
    }

    Layer getLowResLayer() {
        return mGeckoIsReady ? mLowResLayer : null;
    }

    public LayerView getView() {
        return mView;
    }

    /**
     * The view calls this function to indicate that the viewport changed size. It must hold the
     * monitor while calling it.
     *
     * TODO: Refactor this to use an interface. Expose that interface only to the view and not
     * to the layer client. That way, the layer client won't be tempted to call this, which might
     * result in an infinite loop.
     */
    void setViewportSize(FloatSize size) {
        mViewportMetrics = mViewportMetrics.setViewportSize(size.width, size.height);
        sendResizeEventIfNecessary(true);
    }

    public PanZoomController getPanZoomController() {
        return mPanZoomController;
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

    /**
     * Sets the current page rect. You must hold the monitor while calling this.
     */
    private void setPageRect(RectF rect, RectF cssRect) {
        // Since the "rect" is always just a multiple of "cssRect" we don't need to
        // check both; this function assumes that both "rect" and "cssRect" are relative
        // the zoom factor in mViewportMetrics.
        if (mViewportMetrics.getCssPageRect().equals(cssRect))
            return;

        mViewportMetrics = mViewportMetrics.setPageRect(rect, cssRect);

        // Page size is owned by the layer client, so no need to notify it of
        // this change.

        post(new Runnable() {
            public void run() {
                mPanZoomController.pageRectUpdated();
                mView.requestRender();
            }
        });
    }

    void adjustViewport(DisplayPortMetrics displayPort) {
        ImmutableViewportMetrics metrics = getViewportMetrics();

        ImmutableViewportMetrics clampedMetrics = metrics.clamp();

        if (displayPort == null) {
            displayPort = DisplayPortCalculator.calculate(metrics, mPanZoomController.getVelocityVector());
        }

        mDisplayPort = displayPort;
        mGeckoViewport = clampedMetrics;

        if (mRecordDrawTimes) {
            mDrawTimingQueue.add(displayPort);
        }

        reevaluateTiles();
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

    public void setZoomConstraints(ZoomConstraints constraints) {
        mZoomConstraints = constraints;
    }

    /** The compositor invokes this function whenever it determines that the page rect
     * has changed (based on the information it gets from layout). If setFirstPaintViewport
     * is invoked on a frame, then this function will not be. For any given frame, this
     * function will be invoked before syncViewportInfo.
     */
    public void setPageRect(float cssPageLeft, float cssPageTop, float cssPageRight, float cssPageBottom) {
        synchronized (getLock()) {
            RectF cssPageRect = new RectF(cssPageLeft, cssPageTop, cssPageRight, cssPageBottom);
            float ourZoom = getViewportMetrics().zoomFactor;
            setPageRect(RectUtils.scale(cssPageRect, ourZoom), cssPageRect);
            // Here the page size of the document has changed, but the document being displayed
            // is still the same. Therefore, we don't need to send anything to browser.js; any
            // changes we need to make to the display port will get sent the next time we call
            // adjustViewport().
        }
    }

    DisplayPortMetrics getDisplayPort() {
        return mDisplayPort;
    }

    public void beginDrawing() {
        mLowResLayer.beginTransaction();
        mRootLayer.beginTransaction();
    }

    public void endDrawing(ImmutableViewportMetrics viewportMetrics) {
        synchronized (this) {
            try {
                mNewGeckoViewport = viewportMetrics;
                updateViewport();
                mLowResLayer.invalidate();
                mRootLayer.invalidate();
            } finally {
                mLowResLayer.endTransaction();
                mRootLayer.endTransaction();
            }
        }
    }

    protected void updateViewport() {
        // save and restore the viewport size stored in java; never let the
        // JS-side viewport dimensions override the java-side ones because
        // java is the One True Source of this information, and allowing JS
        // to override can lead to race conditions where this data gets clobbered.
        FloatSize viewportSize = mViewportMetrics.getSize();
        mGeckoViewport = mNewGeckoViewport.setViewportSize(viewportSize.width, viewportSize.height);

        RectF position = mGeckoViewport.getViewport();
        mRootLayer.setPosition(RectUtils.round(position));
        mRootLayer.setResolution(mGeckoViewport.zoomFactor);

        mLowResLayer.setPosition(RectUtils.round(position));
        mLowResLayer.setResolution(mGeckoViewport.zoomFactor);

        // Don't adjust page size when zooming unless zoom levels are
        // approximately equal.
        if (FloatUtils.fuzzyEquals(getViewportMetrics().zoomFactor, mGeckoViewport.zoomFactor)) {
            setPageRect(mGeckoViewport.getPageRect(), mGeckoViewport.getCssPageRect());
        }
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary(false);
        if (getRedrawHint()) {
            adjustViewport(null);
        }
    }

    /** Implementation of LayerView.Listener */
    @Override
    public void renderRequested() {
    }

    /** Implementation of LayerView.Listener */
    @Override
    public void compositionPauseRequested() {
    }

    /** Implementation of LayerView.Listener */
    @Override
    public void compositionResumeRequested(int width, int height) {
    }

    /** Implementation of LayerView.Listener */
    @Override
    public void surfaceChanged(int width, int height) {
        setViewportSize(new FloatSize(width, height));

        // We need to make this call even when the compositor isn't currently
        // paused (e.g. during an orientation change), to make the compositor
        // aware of the changed surface.
        compositionResumeRequested(width, height);
        renderRequested();
    }

    /** Implementation of LayerView.Listener */
    @Override
    public void compositorCreated() {
    }

    /** Implementation of PanZoomTarget */
    public ImmutableViewportMetrics getViewportMetrics() {
        return mViewportMetrics;
    }

    /** Implementation of PanZoomTarget */
    public ZoomConstraints getZoomConstraints() {
        return mZoomConstraints;
    }

    /** Implementation of PanZoomTarget */
    public void setAnimationTarget(ImmutableViewportMetrics viewport) {
        if (mGeckoIsReady) {
            // We know what the final viewport of the animation is going to be, so
            // immediately request a draw of that area by setting the display port
            // accordingly. This way we should have the content pre-rendered by the
            // time the animation is done.
            DisplayPortMetrics displayPort = DisplayPortCalculator.calculate(viewport, null);
            adjustViewport(displayPort);
        }
    }

    /** Implementation of PanZoomTarget
     * You must hold the monitor while calling this.
     */
    public void setViewportMetrics(ImmutableViewportMetrics viewport) {
        mViewportMetrics = viewport;
        mView.requestRender();
        if (mGeckoIsReady) {
            geometryChanged();
        }
    }

    /** Implementation of PanZoomTarget */
    public void forceRedraw() {
        mForceRedraw = true;
        if (mGeckoIsReady) {
            geometryChanged();
        }
    }

    /** Implementation of PanZoomTarget */
    public boolean post(Runnable action) {
        return mView.post(action);
    }

    /** Implementation of PanZoomTarget */
    public Object getLock() {
        return this;
    }

    /** Implementation of PanZoomTarget
     * Converts a point from layer view coordinates to layer coordinates. In other words, given a
     * point measured in pixels from the top left corner of the layer view, returns the point in
     * pixels measured from the last scroll position we sent to Gecko, in CSS pixels. Assuming the
     * events being sent to Gecko are processed in FIFO order, this calculation should always be
     * correct.
     */
    public PointF convertViewPointToLayerPoint(PointF viewPoint) {
        if (!mGeckoIsReady) {
            return null;
        }

        ImmutableViewportMetrics viewportMetrics = mViewportMetrics;
        PointF origin = viewportMetrics.getOrigin();
        float zoom = viewportMetrics.zoomFactor;
        ImmutableViewportMetrics geckoViewport = mGeckoViewport;
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

    public void destroy() {
        mPanZoomController.destroy();
    }

    public Context getContext() {
        return mContext;
    }

    public void zoomTo(RectF rect) {
        if (mPanZoomController instanceof JavaPanZoomController) {
            ((JavaPanZoomController) mPanZoomController).animatedZoomTo(rect);
        }
    }

    public void zoomTo(float pageWidth, float pageHeight) {
        zoomTo(new RectF(0, 0, pageWidth, pageHeight));
    }

    public void forceRender() {
        post(new Runnable() {
            public void run() {
                mView.requestRender();
            }
        });
    }

    private class AdjustRunnable implements Runnable {
        public void run() {
            mPendingViewportAdjust = false;
            adjustViewport(null);
        }
    }

    /* Root Layer Access */
    public void reevaluateTiles() {
        mLowResLayer.reevaluateTiles(mViewportMetrics, mDisplayPort);
        mRootLayer.reevaluateTiles(mViewportMetrics, mDisplayPort);
    }

    public void clearAndResetlayers() {
        mLowResLayer.clearAndReset();
        mRootLayer.clearAndReset();
    }

    public void invalidateTiles(RectF rect) {
        mLowResLayer.invalidateTiles(rect);
        mRootLayer.invalidateTiles(rect);
    }
}