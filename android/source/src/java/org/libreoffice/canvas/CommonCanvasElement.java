package org.libreoffice.canvas;

import android.graphics.Canvas;

/**
 * Common implementation to canvas elements.
 */
public abstract class CommonCanvasElement implements CanvasElement, CanvasElementImplRequirement {

    private boolean mVisible = false;

    /**
     * Is element visible?
     */
    @Override
    public boolean isVisible() {
        return mVisible;
    }

    /**
     * Set element visibility.
     */
    @Override
    public void setVisible(boolean visible) {
        mVisible = visible;
    }

    /**
     * Trigger drawing the element on the canvas.
     */
    @Override
    public void draw(Canvas canvas) {
        if (isVisible()) {
            onDraw(canvas);
        }
    }

    /**
     * Hit test. Return true if the element was hit. Directly return false if
     * the element is invisible.
     */
    @Override
    public boolean contains(float x, float y) {
        if (!isVisible()) {
            return false;
        }
        return onHitTest(x, y);
    }
}
