package org.libreoffice;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;

import org.mozilla.gecko.gfx.LayerView;

public class ViewFactory implements LayoutInflater.Factory {
    private static final String LOGTAG = ViewFactory.class.getSimpleName();
    private static final String LAYER_VIEW_ID = "org.mozilla.gecko.gfx.LayerView";
    private static final ViewFactory INSTANCE = new ViewFactory();

    private ViewFactory() {
    }

    public static LayoutInflater.Factory getInstance() {
        return INSTANCE;
    }

    @Override
    public View onCreateView(String name, Context context, AttributeSet attrs) {
        if (name.equals(LAYER_VIEW_ID)) {
            Log.i(LOGTAG, "Creating custom Gecko view: " + name);
            return new LayerView(context, attrs);
        }

        return null;
    }
}