/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.overlay;

import android.app.Activity;
import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.LOKitShell;
import org.libreoffice.R;
import org.libreoffice.canvas.SelectionHandle;
import org.mozilla.gecko.gfx.Layer;
import org.mozilla.gecko.gfx.LayerView;
import org.mozilla.gecko.util.FloatUtils;

import java.util.List;

/**
 * The DocumentOverlay is an overlay over the document. This class is responsible
 * to setup the document overlay view, report visibility and position of its elements
 * when they change and report any changes to the viewport.
 */
public class DocumentOverlay {
    private static final String LOGTAG = DocumentOverlay.class.getSimpleName();

    private final DocumentOverlayView mDocumentOverlayView;
    private final DocumentOverlayLayer mDocumentOverlayLayer;

    /**
     * DocumentOverlayLayer responsibility is to get the changes to the viewport
     * and report them to DocumentOverlayView.
     */
    private class DocumentOverlayLayer extends Layer {
        private float mViewLeft;
        private float mViewTop;
        private float mViewZoom;

        /**
         * @see Layer#draw(org.mozilla.gecko.gfx.Layer.RenderContext)
         */
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
                    mDocumentOverlayView.repositionWithViewport(mViewLeft, mViewTop, mViewZoom);
                }
            });
        }
    }

    public DocumentOverlay(Activity context, LayerView layerView) {
        mDocumentOverlayView = (DocumentOverlayView) context.findViewById(R.id.text_cursor_view);
        mDocumentOverlayLayer = new DocumentOverlayLayer();
        if (mDocumentOverlayView == null) {
            Log.e(LOGTAG, "Failed to initialize TextCursorLayer - CursorView is null");
        }
        layerView.addLayer(mDocumentOverlayLayer);
        mDocumentOverlayView.initialize(layerView);
    }

    /**
     * Show the cursor at the defined cursor position on the overlay.
     */
    public void showCursor() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.showCursor();
            }
        });
    }

    /**
     * Hide the cursor at the defined cursor position on the overlay.
     */
    public void hideCursor() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.hideCursor();
            }
        });
    }

    /**
     * Position the cursor to the input position on the overlay.
     */
    public void positionCursor(final RectF position) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.changeCursorPosition(position);
            }
        });
    }

    /**
     * Show selections on the overlay.
     */
    public void showSelections() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.showSelections();
            }
        });
    }

    /**
     * Hide selections on the overlay.
     */
    public void hideSelections() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.hideSelections();
            }
        });
    }

    /**
     * Change the list of selections.
     */
    public void changeSelections(final List<RectF> selections) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.changeSelections(selections);
            }
        });
    }

    /**
     * Show the graphic selection on the overlay.
     */
    public void showGraphicSelection() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.showGraphicSelection();
            }
        });
    }

    /**
     * Hide the graphic selection.
     */
    public void hideGraphicSelection() {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.hideGraphicSelection();
            }
        });
    }

    /**
     * Change the graphic selection rectangle to the input rectangle.
     */
    public void changeGraphicSelection(final RectF rectangle) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.changeGraphicSelection(rectangle);
            }
        });
    }

    /**
     * Show the handle (of input type) on the overlay.
     */
    public void showHandle(final SelectionHandle.HandleType type) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.showHandle(type);
            }
        });
    }

    /**
     * Hide the handle (of input type).
     */
    public void hideHandle(final SelectionHandle.HandleType type) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.hideHandle(type);
            }
        });
    }

    /**
     * Position the handle (of input type) position to the input rectangle.
     */
    public void positionHandle(final SelectionHandle.HandleType type, final RectF rectangle) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                mDocumentOverlayView.positionHandle(type, rectangle);
            }
        });
    }

    public RectF getCurrentCursorPosition() {
        return mDocumentOverlayView.getCurrentCursorPosition();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
