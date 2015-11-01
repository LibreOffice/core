package org.libreoffice.canvas;

import android.content.Context;

import org.libreoffice.R;

/**
 * Selection handle that is used to manipulate the cursor.
 */
public class SelectionHandleMiddle extends SelectionHandle {
    public SelectionHandleMiddle(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_alias_middle));
    }

    /**
     * Change the position of the handle on the screen. Take into account the
     * handle alignment to the center.
     */
    @Override
    public void reposition(float x, float y) {
        super.reposition(x, y);
        // align to the center
        float offset = mScreenPosition.width() / 2.0f;
        mScreenPosition.offset(-offset, 0);
    }

    /**
     * Define the type of the handle.
     */
    @Override
    public HandleType getHandleType() {
        return HandleType.MIDDLE;
    }
}
