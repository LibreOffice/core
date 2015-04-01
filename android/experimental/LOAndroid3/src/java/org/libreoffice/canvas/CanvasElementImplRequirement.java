package org.libreoffice.canvas;

import android.graphics.Canvas;

interface CanvasElementImplRequirement {

    /**
     * Implement hit test here
     * @param x - x coordinate of the
     * @param y - y coordinate of the
     */
    boolean onHitTest(float x, float y);

    /**
     * Called inside draw if the element is visible. Override this method to
     * draw the element on the canvas.
     *
     * @param canvas - the canvas
     */
    void onDraw(Canvas canvas);
}
