package org.mozilla.gecko.gfx;

public class DynamicTileLayer extends ComposedTileLayer {
    @Override
    protected float getZoom(ImmutableViewportMetrics viewportMetrics) {
        return viewportMetrics.zoomFactor;
    }
}
