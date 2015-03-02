/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko;

import android.app.Activity;
import android.graphics.RectF;
import android.util.Log;
import android.view.View;

import org.libreoffice.LOKitShell;
import org.libreoffice.R;
import org.mozilla.gecko.gfx.Layer;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.util.FloatUtils;

import static org.mozilla.gecko.TextSelectionHandle.HandleType.MIDDLE;
import static org.mozilla.gecko.TextSelectionHandle.HandleType.START;

public class TextSelection extends Layer {
    private static final String LOGTAG = "GeckoTextSelection";

    private final TextSelectionHandle mStartHandle;
    private final TextSelectionHandle mMiddleHandle;
    private final TextSelectionHandle mEndHandle;

    private float mViewLeft;
    private float mViewTop;
    private float mViewZoom;

    public TextSelection(Activity context) {
        mStartHandle = (TextSelectionHandle) context.findViewById(R.id.start_handle);
        mMiddleHandle = (TextSelectionHandle) context.findViewById(R.id.middle_handle);
        mEndHandle = (TextSelectionHandle) context.findViewById(R.id.end_handle);

        // Only register listeners if we have valid start/middle/end handles
        if (mStartHandle == null || mMiddleHandle == null || mEndHandle == null) {
            Log.e(LOGTAG, "Failed to initialize text selection because at least one handle is null");
        }
    }

    void destroy() {
    }

    @Override
    public void draw(final RenderContext context) {
        // cache the relevant values from the context and bail out if they are the same. we do this
        // because this draw function gets called a lot (once per compositor frame) and we want to
        // avoid doing a lot of extra work in cases where it's not needed.
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
                mStartHandle.repositionWithViewport(context.viewport.left, context.viewport.top, context.zoomFactor);
                mMiddleHandle.repositionWithViewport(context.viewport.left, context.viewport.top, context.zoomFactor);
                mEndHandle.repositionWithViewport(context.viewport.left, context.viewport.top, context.zoomFactor);
            }
        });
    }

    public void showHandle(final TextSelectionHandle.HandleType handleType) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                TextSelectionHandle handle = getHandle(handleType);

                handle.setVisibility(View.VISIBLE);

                mViewLeft = 0.0f;
                mViewTop = 0.0f;
                mViewZoom = 0.0f;
                LayerView layerView = LOKitShell.getLayerView();
                if (layerView != null) {
                    layerView.addLayer(TextSelection.this);
                }
            }
        });
    }

    private TextSelectionHandle getHandle(TextSelectionHandle.HandleType handleType) {
        if (handleType == START) {
            return mStartHandle;
        } else if (handleType == MIDDLE) {
            return mMiddleHandle;
        } else {
            return mEndHandle;
        }
    }

    public void hideHandle(final TextSelectionHandle.HandleType handleType) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                TextSelectionHandle handle = getHandle(handleType);
                handle.setVisibility(View.GONE);
            }
        });
    }


    public void positionHandle(final TextSelectionHandle.HandleType handleType, final RectF position) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                TextSelectionHandle handle = getHandle(handleType);
                handle.positionFromGecko(position, false);
            }
        });
    }
}
