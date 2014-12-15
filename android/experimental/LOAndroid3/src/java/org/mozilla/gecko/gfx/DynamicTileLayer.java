package org.mozilla.gecko.gfx;

import android.graphics.RectF;

public class DynamicTileLayer extends ComposedTileLayer {
    @Override
    protected RectF getViewPort(ImmutableViewportMetrics viewportMetrics) {
        RectF rect = viewportMetrics.getViewport();
        return inflate(roundToTileSize(rect, tileSize), getInflateFactor());
    }

    @Override
    protected float getZoom(ImmutableViewportMetrics viewportMetrics) {
        return viewportMetrics.zoomFactor;
    }

    protected IntSize getInflateFactor() {
        return tileSize;
    }
}
