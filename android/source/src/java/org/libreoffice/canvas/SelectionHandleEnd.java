package org.libreoffice.canvas;

import android.content.Context;

import org.libreoffice.R;

/**
 * Selection handle for showing and manipulating the end of a selection.
 */
public class SelectionHandleEnd extends SelectionHandle {
    public SelectionHandleEnd(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_alias_end));
    }

    /**
     * Define the type of the handle.
     */
    @Override
    public HandleType getHandleType() {
        return HandleType.END;
    }
}
