package org.mozilla.gecko.gfx;

import android.graphics.RectF;

public class FixedZoomTileLayer extends ComposedTileLayer {
    @Override
    protected RectF getViewPort(ImmutableViewportMetrics viewportMetrics) {
        float zoom = getZoom(viewportMetrics);
        RectF rect = normalizeRect(viewportMetrics.getViewport(), viewportMetrics.zoomFactor, zoom);
        return inflate(roundToTileSize(rect, tileSize), getInflateFactor());
    }

    @Override
    protected float getZoom(ImmutableViewportMetrics viewportMetrics) {
        return 1.0f / 32.0f;
    }

    @Override
    protected int getTilePriority() {
        return -1;
    }

    private IntSize getInflateFactor() {
        return tileSize.scale(3);
    }
}
