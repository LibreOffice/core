package org.mozilla.gecko.gfx;

import android.content.Context;
import android.graphics.RectF;

public class FixedZoomTileLayer extends ComposedTileLayer {
    public FixedZoomTileLayer(Context context) {
        super(context);
    }

    @Override
    protected RectF getViewPort(ImmutableViewportMetrics viewportMetrics) {
        float zoom = getZoom(viewportMetrics);
        RectF rect = normalizeRect(viewportMetrics.getViewport(), viewportMetrics.zoomFactor, zoom);
        return inflate(roundToTileSize(rect, tileSize), getInflateFactor());
    }

    @Override
    protected float getZoom(ImmutableViewportMetrics viewportMetrics) {
        return 1.0f / 16.0f;
    }

    @Override
    protected int getTilePriority() {
        return -1;
    }

    private IntSize getInflateFactor() {
        return new IntSize(tileSize.width, tileSize.height*6);
    }
}
