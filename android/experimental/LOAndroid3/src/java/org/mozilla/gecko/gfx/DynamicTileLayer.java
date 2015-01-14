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

    @Override
    protected int getTilePriority() {
        return 0;
    }

    private IntSize getInflateFactor() {
        return new IntSize(tileSize.width*2, tileSize.height*4);
    }
}
