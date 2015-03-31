package org.libreoffice.canvas;

/**
 * Common implementation to canvas elements.
 */
public abstract class CommonCanvasElement implements CanvasElement {

    private boolean mVisible = true;

    @Override
    public boolean isVisible() {
        return mVisible;
    }

    @Override
    public void setVisible(boolean visible) {
        mVisible = visible;
    }
}
