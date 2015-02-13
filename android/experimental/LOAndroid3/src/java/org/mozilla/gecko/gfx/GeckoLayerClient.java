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
import org.libreoffice.LOKitThread;
import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.util.FloatUtils;

import java.util.List;

public class GeckoLayerClient implements PanZoomTarget, LayerView.Listener {
    private static final String LOGTAG = GeckoLayerClient.class.getSimpleName();

    private LayerRenderer mLayerRenderer;
    private boolean mLayerRendererInitialized;

    private Context mContext;
    private IntSize mScreenSize;
    private DisplayPortMetrics mDisplayPort;
    private DisplayPortMetrics mReturnDisplayPort;

    private ComposedTileLayer mLowResLayer;
    private ComposedTileLayer mRootLayer;

    /*
     * The viewport metrics being used to draw the current frame. This is only
     * accessed by the compositor thread, and so needs no synchronisation.
     */
    private ImmutableViewportMetrics mFrameMetrics;

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

    private boolean mIsReady ;

    /* The new color for the checkerboard. */
    private int mCheckerboardColor;

    private PanZoomController mPanZoomController;
    private LayerView mView;

    public GeckoLayerClient(Context context) {
        // we can fill these in with dummy values because they are always written
        // to before being read
        mContext = context;
        mScreenSize = new IntSize(0, 0);
        mDisplayPort = new DisplayPortMetrics();

        mForceRedraw = true;
        DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
        mViewportMetrics = new ImmutableViewportMetrics(displayMetrics);
        mZoomConstraints = new ZoomConstraints(false);
    }

    public void setView(LayerView view) {
        mView = view;
        mPanZoomController = PanZoomController.Factory.create(this, view);
        mView.connect(this);
    }

    public void notifyReady() {
        mIsReady = true;

        mRootLayer = new DynamicTileLayer(mContext);
        mLowResLayer = new FixedZoomTileLayer(mContext);

        mLayerRenderer = new LayerRenderer(mView);

        mView.setListener(this);
        mView.setLayerRenderer(mLayerRenderer);
        sendResizeEventIfNecessary();
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
        return mIsReady ? mRootLayer : null;
    }

    Layer getLowResLayer() {
        return mIsReady ? mLowResLayer : null;
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
        sendResizeEventIfNecessary();
    }

    public PanZoomController getPanZoomController() {
        return mPanZoomController;
    }

    /* Informs Gecko that the screen size has changed. */
    private void sendResizeEventIfNecessary() {
        DisplayMetrics metrics = LOKitShell.getDisplayMetrics();
        IntSize newScreenSize = new IntSize(metrics.widthPixels, metrics.heightPixels);

        if (mScreenSize.equals(newScreenSize)) {
            return;
        }

        mScreenSize = newScreenSize;

        LOEvent event = LOEventFactory.sizeChanged(mScreenSize.width, mScreenSize.height);
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
            mLowResLayer.endTransaction();
            mRootLayer.endTransaction();
        }
    }

    public void geometryChanged() {
        sendResizeEventIfNecessary();
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
        if (mIsReady) {
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
        if (mIsReady) {
            geometryChanged();
        }
    }

    /** Implementation of PanZoomTarget */
    public void forceRedraw() {
        mForceRedraw = true;
        if (mIsReady) {
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

    public PointF convertViewPointToLayerPoint(PointF viewPoint) {
        ImmutableViewportMetrics viewportMetrics = mViewportMetrics;
        PointF origin = viewportMetrics.getOrigin();
        float zoom = viewportMetrics.zoomFactor;

        PointF layerPoint = new PointF(
                ((viewPoint.x + origin.x) / zoom),
                ((viewPoint.y + origin.y) / zoom));

        return layerPoint;
    }

    @Override
    public boolean isFullScreen() {
        return false;
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

    /* Root Layer Access */
    public void reevaluateTiles() {
        mLowResLayer.reevaluateTiles(mViewportMetrics, mDisplayPort);
        mRootLayer.reevaluateTiles(mViewportMetrics, mDisplayPort);
    }

    public void clearAndResetlayers() {
        mLowResLayer.clearAndReset();
        mRootLayer.clearAndReset();
    }

    public void invalidateTiles(List<SubTile> tilesToInvalidate, RectF rect) {
        mLowResLayer.invalidateTiles(tilesToInvalidate, rect);
        mRootLayer.invalidateTiles(tilesToInvalidate, rect);
    }
}