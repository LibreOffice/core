package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;

import org.libreoffice.R;

public class SelectionHandleEnd extends SelectionHandle {
    public SelectionHandleEnd(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_end));
    }

    @Override
    public HandleType getHandleType() {
        return HandleType.END;
    }
}
