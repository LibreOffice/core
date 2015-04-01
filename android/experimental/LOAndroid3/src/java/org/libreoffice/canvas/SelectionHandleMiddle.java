package org.libreoffice.canvas;

import android.content.Context;
import android.graphics.Bitmap;

import org.libreoffice.R;

public class SelectionHandleMiddle extends SelectionHandle {
    public SelectionHandleMiddle(Context context) {
        super(getBitmapForDrawable(context, R.drawable.handle_middle));
    }

    @Override
    public void reposition(float x, float y) {
        super.reposition(x, y);
        float offset = mScreenPosition.width() / 2.0f;
        mScreenPosition.offset(-offset, 0);
    }

    @Override
    public HandleType getHandleType() {
        return HandleType.MIDDLE;
    }
}
