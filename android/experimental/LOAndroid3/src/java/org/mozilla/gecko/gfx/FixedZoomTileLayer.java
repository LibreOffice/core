package org.mozilla.gecko.gfx;

public class FixedZoomTileLayer extends ComposedTileLayer {
    @Override
    protected float getZoom(ImmutableViewportMetrics viewportMetrics) {
        return 0.5f;
    }

    @Override
    public void addTile(SubTile tile) {

    }
}
