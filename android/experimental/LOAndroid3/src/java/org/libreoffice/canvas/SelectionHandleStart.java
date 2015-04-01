package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;

import org.libreoffice.R;

public class SelectionHandleStart extends SelectionHandle {
    public SelectionHandleStart(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_start));
    }

    @Override
    public void reposition(float x, float y) {
        super.reposition(x, y);
        float offset = mScreenPosition.width();
        mScreenPosition.offset(-offset, 0);
    }

    @Override
    public HandleType getHandleType() {
        return HandleType.START;
    }
}