package org.libreoffice.canvas;

import android.graphics.Canvas;

/**
 * Common implementation to canvas elements.
 */
public abstract class CommonCanvasElement implements CanvasElement {

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
}
