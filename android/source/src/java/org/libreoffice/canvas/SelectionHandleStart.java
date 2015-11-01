package org.libreoffice.canvas;

import android.content.Context;

import org.libreoffice.R;

/**
 * Selection handle for showing and manipulating the start of a selection.
 */
public class SelectionHandleStart extends SelectionHandle {
    public SelectionHandleStart(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_alias_start));
    }

    /**
     * Change the position of the handle on the screen. Take into account the
     * handle alignment to the right.
     */
    @Override
    public void reposition(float x, float y) {
        super.reposition(x, y);
        // align to the right
        float offset = mScreenPosition.width();
        mScreenPosition.offset(-offset, 0);
    }

    /**
     * Define the type of the handle.
     */
    @Override
    public HandleType getHandleType() {
        return HandleType.START;
    }
}