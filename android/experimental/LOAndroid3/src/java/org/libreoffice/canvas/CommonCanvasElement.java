package org.libreoffice.canvas;

import android.graphics.Canvas;

/**
 * Common implementation to canvas elements.
 */
public abstract class CommonCanvasElement implements CanvasElement, CanvasElementImplRequirement {

    private boolean mVisible = false;

    @Override
    public boolean isVisible() {
        return mVisible;
    }

    @Override
    public void setVisible(boolean visible) {
        mVisible = visible;
    }

    @Override
    public void draw(Canvas canvas) {
        if (isVisible()) {
            onDraw(canvas);
        }
    }

    @Override
    public boolean contains(float x, float y) {
        if (!isVisible()) {
            return false;
        }
        return onHitTest(x, y);
    }
}
