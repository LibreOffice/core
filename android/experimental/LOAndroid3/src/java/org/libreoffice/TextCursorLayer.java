package org.libreoffice;

import android.app.Activity;
import android.graphics.RectF;
import android.util.Log;
import android.view.View;

import org.mozilla.gecko.gfx.Layer;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.util.FloatUtils;

/**
 * The TextCursorLayer is a layer which is responsible for showing the cursor and
 * controls its position, height and visibility.
 */
public class TextCursorLayer extends Layer {
    private static final String LOGTAG = TextCursorLayer.class.getSimpleName();

    private final TextCursorView mCursorView;
    private float mViewLeft;
    private float mViewTop;
    private float mViewZoom;

    public TextCursorLayer(Activity context) {
        mCursorView = (TextCursorView) context.findViewById(R.id.text_cursor_view);
        if (mCursorView == null) {
            Log.e(LOGTAG, "Failed to initialize TextCursorLayer - CursorView is null");
        }
    }

    @Override
    public void draw(final RenderContext context) {
        if (FloatUtils.fuzzyEquals(mViewLeft, context.viewport.left)
                && FloatUtils.fuzzyEquals(mViewTop, context.viewport.top)
                && FloatUtils.fuzzyEquals(mViewZoom, context.zoomFactor)) {
            return;
        }

        mViewLeft = context.viewport.left;
        mViewTop = context.viewport.top;
        mViewZoom = context.zoomFactor;

        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mCursorView.repositionWithViewport(context.viewport.left, context.viewport.top, context.zoomFactor);
            }
        });
    }

    public void showCursor() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mCursorView.setVisibility(View.VISIBLE);

                mViewLeft = 0.0f;
                mViewTop = 0.0f;
                mViewZoom = 0.0f;
                LayerView layerView = LOKitShell.getLayerView();
                if (layerView != null) {
                    layerView.addLayer(TextCursorLayer.this);
                }
            }
        });
    }

    public void hideCursor() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mCursorView.setVisibility(View.GONE);
            }
        });
    }

    public void positionCursor(final RectF position) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mCursorView.changePosition(position);
            }
        });
    }
}
