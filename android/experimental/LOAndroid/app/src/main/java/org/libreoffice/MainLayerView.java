package org.libreoffice;

import android.content.Context;
import android.os.Handler;
import android.util.AttributeSet;

import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.util.ThreadUtils;

public class MainLayerView extends LayerView {

    public MainLayerView(Context context) {
        super(context);
        init(context);
    }

    public MainLayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    private void init(Context context) {
        ThreadUtils.setUiThread(Thread.currentThread(), new Handler());
    }

}
