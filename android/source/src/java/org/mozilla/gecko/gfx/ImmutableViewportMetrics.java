/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.PointF;
import android.graphics.RectF;
import android.util.DisplayMetrics;

import org.mozilla.gecko.util.FloatUtils;

/**
 * ImmutableViewportMetrics are used to store the viewport metrics
 * in way that we can access a version of them from multiple threads
 * without having to take a lock
 */
public class ImmutableViewportMetrics {

    // We need to flatten the RectF and FloatSize structures
    // because Java doesn't have the concept of const classes
    public final float pageRectLeft;
    public final float pageRectTop;
    public final float pageRectRight;
    public final float pageRectBottom;
    public final float cssPageRectLeft;
    public final float cssPageRectTop;
    public final float cssPageRectRight;
    public final float cssPageRectBottom;
    public final float viewportRectLeft;
    public final float viewportRectTop;
    public final float viewportRectRight;
    public final float viewportRectBottom;
    public final float zoomFactor;

    public ImmutableViewportMetrics(DisplayMetrics metrics) {
        viewportRectLeft   = pageRectLeft   = cssPageRectLeft   = 0;
        viewportRectTop    = pageRectTop    = cssPageRectTop    = 0;
        viewportRectRight  = pageRectRight  = cssPageRectRight  = metrics.widthPixels;
        viewportRectBottom = pageRectBottom = cssPageRectBottom = metrics.heightPixels;
        zoomFactor = 1.0f;
    }

    private ImmutableViewportMetrics(float aPageRectLeft, float aPageRectTop,
        float aPageRectRight, float aPageRectBottom, float aCssPageRectLeft,
        float aCssPageRectTop, float aCssPageRectRight, float aCssPageRectBottom,
        float aViewportRectLeft, float aViewportRectTop, float aViewportRectRight,
        float aViewportRectBottom, float aZoomFactor)
    {
        pageRectLeft = aPageRectLeft;
        pageRectTop = aPageRectTop;
        pageRectRight = aPageRectRight;
        pageRectBottom = aPageRectBottom;
        cssPageRectLeft = aCssPageRectLeft;
        cssPageRectTop = aCssPageRectTop;
        cssPageRectRight = aCssPageRectRight;
        cssPageRectBottom = aCssPageRectBottom;
        viewportRectLeft = aViewportRectLeft;
        viewportRectTop = aViewportRectTop;
        viewportRectRight = aViewportRectRight;
        viewportRectBottom = aViewportRectBottom;
        zoomFactor = aZoomFactor;
    }

    public float getWidth() {
        return viewportRectRight - viewportRectLeft;
    }

    public float getHeight() {
        return viewportRectBottom - viewportRectTop;
    }

    public PointF getOrigin() {
        return new PointF(viewportRectLeft, viewportRectTop);
    }

    public FloatSize getSize() {
        return new FloatSize(viewportRectRight - viewportRectLeft, viewportRectBottom - viewportRectTop);
    }

    public RectF getViewport() {
        return new RectF(viewportRectLeft,
                         viewportRectTop,
                         viewportRectRight,
                         viewportRectBottom);
    }

    public RectF getCssViewport() {
        return RectUtils.scale(getViewport(), 1/zoomFactor);
    }

    public RectF getPageRect() {
        return new RectF(pageRectLeft, pageRectTop, pageRectRight, pageRectBottom);
    }

    public float getPageWidth() {
        return pageRectRight - pageRectLeft;
    }

    public float getPageHeight() {
        return pageRectBottom - pageRectTop;
    }

    public RectF getCssPageRect() {
        return new RectF(cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom);
    }

    public float getZoomFactor() {
        return zoomFactor;
    }

    /*
     * Returns the viewport metrics that represent a linear transition between "this" and "to" at
     * time "t", which is on the scale [0, 1). This function interpolates all values stored in
     * the viewport metrics.
     */
    public ImmutableViewportMetrics interpolate(ImmutableViewportMetrics to, float t) {
        return new ImmutableViewportMetrics(
            FloatUtils.interpolate(pageRectLeft, to.pageRectLeft, t),
            FloatUtils.interpolate(pageRectTop, to.pageRectTop, t),
            FloatUtils.interpolate(pageRectRight, to.pageRectRight, t),
            FloatUtils.interpolate(pageRectBottom, to.pageRectBottom, t),
            FloatUtils.interpolate(cssPageRectLeft, to.cssPageRectLeft, t),
            FloatUtils.interpolate(cssPageRectTop, to.cssPageRectTop, t),
            FloatUtils.interpolate(cssPageRectRight, to.cssPageRectRight, t),
            FloatUtils.interpolate(cssPageRectBottom, to.cssPageRectBottom, t),
            FloatUtils.interpolate(viewportRectLeft, to.viewportRectLeft, t),
            FloatUtils.interpolate(viewportRectTop, to.viewportRectTop, t),
            FloatUtils.interpolate(viewportRectRight, to.viewportRectRight, t),
            FloatUtils.interpolate(viewportRectBottom, to.viewportRectBottom, t),
            FloatUtils.interpolate(zoomFactor, to.zoomFactor, t));
    }

    public ImmutableViewportMetrics setViewportSize(float width, float height) {
        return new ImmutableViewportMetrics(
            pageRectLeft, pageRectTop, pageRectRight, pageRectBottom,
            cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom,
            viewportRectLeft, viewportRectTop, viewportRectLeft + width, viewportRectTop + height,
            zoomFactor);
    }

    public ImmutableViewportMetrics setViewportOrigin(float newOriginX, float newOriginY) {
        return new ImmutableViewportMetrics(
            pageRectLeft, pageRectTop, pageRectRight, pageRectBottom,
            cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom,
            newOriginX, newOriginY, newOriginX + getWidth(), newOriginY + getHeight(),
            zoomFactor);
    }

    public ImmutableViewportMetrics setZoomFactor(float newZoomFactor) {
        return new ImmutableViewportMetrics(
            pageRectLeft, pageRectTop, pageRectRight, pageRectBottom,
            cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom,
            viewportRectLeft, viewportRectTop, viewportRectRight, viewportRectBottom,
            newZoomFactor);
    }

    public ImmutableViewportMetrics offsetViewportBy(float dx, float dy) {
        return setViewportOrigin(viewportRectLeft + dx, viewportRectTop + dy);
    }

    public ImmutableViewportMetrics setPageRect(RectF pageRect, RectF cssPageRect) {
        return new ImmutableViewportMetrics(
            pageRect.left, pageRect.top, pageRect.right, pageRect.bottom,
            cssPageRect.left, cssPageRect.top, cssPageRect.right, cssPageRect.bottom,
            viewportRectLeft, viewportRectTop, viewportRectRight, viewportRectBottom,
            zoomFactor);
    }

    /* This will set the zoom factor and re-scale page-size and viewport offset
     * accordingly. The given focus will remain at the same point on the screen
     * after scaling.
     */
    public ImmutableViewportMetrics scaleTo(float newZoomFactor, PointF focus) {
        // cssPageRect* is invariant, since we're setting the scale factor
        // here. The page rect is based on the CSS page rect.
        float newPageRectLeft = cssPageRectLeft * newZoomFactor;
        float newPageRectTop = cssPageRectTop * newZoomFactor;
        float newPageRectRight = cssPageRectLeft + ((cssPageRectRight - cssPageRectLeft) * newZoomFactor);
        float newPageRectBottom = cssPageRectTop + ((cssPageRectBottom - cssPageRectTop) * newZoomFactor);

        PointF origin = getOrigin();
        origin.offset(focus.x, focus.y);
        origin = PointUtils.scale(origin, newZoomFactor / zoomFactor);
        origin.offset(-focus.x, -focus.y);

        return new ImmutableViewportMetrics(
            newPageRectLeft, newPageRectTop, newPageRectRight, newPageRectBottom,
            cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom,
            origin.x, origin.y, origin.x + getWidth(), origin.y + getHeight(),
            newZoomFactor);
    }

    /** Clamps the viewport to remain within the page rect. */
    public ImmutableViewportMetrics clamp() {
        RectF newViewport = getViewport();

        // The viewport bounds ought to never exceed the page bounds.
        if (newViewport.right > pageRectRight)
            newViewport.offset(pageRectRight - newViewport.right, 0);
        if (newViewport.left < pageRectLeft)
            newViewport.offset(pageRectLeft - newViewport.left, 0);

        if (newViewport.bottom > pageRectBottom)
            newViewport.offset(0, pageRectBottom - newViewport.bottom);
        if (newViewport.top < pageRectTop)
            newViewport.offset(0, pageRectTop - newViewport.top);

        return new ImmutableViewportMetrics(
            pageRectLeft, pageRectTop, pageRectRight, pageRectBottom,
            cssPageRectLeft, cssPageRectTop, cssPageRectRight, cssPageRectBottom,
            newViewport.left, newViewport.top, newViewport.right, newViewport.bottom,
            zoomFactor);
    }

    public boolean fuzzyEquals(ImmutableViewportMetrics other) {
        return FloatUtils.fuzzyEquals(pageRectLeft, other.pageRectLeft)
            && FloatUtils.fuzzyEquals(pageRectTop, other.pageRectTop)
            && FloatUtils.fuzzyEquals(pageRectRight, other.pageRectRight)
            && FloatUtils.fuzzyEquals(pageRectBottom, other.pageRectBottom)
            && FloatUtils.fuzzyEquals(cssPageRectLeft, other.cssPageRectLeft)
            && FloatUtils.fuzzyEquals(cssPageRectTop, other.cssPageRectTop)
            && FloatUtils.fuzzyEquals(cssPageRectRight, other.cssPageRectRight)
            && FloatUtils.fuzzyEquals(cssPageRectBottom, other.cssPageRectBottom)
            && FloatUtils.fuzzyEquals(viewportRectLeft, other.viewportRectLeft)
            && FloatUtils.fuzzyEquals(viewportRectTop, other.viewportRectTop)
            && FloatUtils.fuzzyEquals(viewportRectRight, other.viewportRectRight)
            && FloatUtils.fuzzyEquals(viewportRectBottom, other.viewportRectBottom)
            && FloatUtils.fuzzyEquals(zoomFactor, other.zoomFactor);
    }

    @Override
    public String toString() {
        return "ImmutableViewportMetrics v=(" + viewportRectLeft + "," + viewportRectTop + ","
                + viewportRectRight + "," + viewportRectBottom + ") p=(" + pageRectLeft + ","
                + pageRectTop + "," + pageRectRight + "," + pageRectBottom + ") c=("
                + cssPageRectLeft + "," + cssPageRectTop + "," + cssPageRectRight + ","
                + cssPageRectBottom + ") z=" + zoomFactor;
    }
}
