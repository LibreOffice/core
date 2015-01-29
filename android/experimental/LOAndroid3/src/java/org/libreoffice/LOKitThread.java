package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;
import android.view.MotionEvent;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.SubTile;

import java.util.concurrent.PriorityBlockingQueue;

public class LOKitThread extends Thread implements TileProvider.TileInvalidationCallback {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private PriorityBlockingQueue<LOEvent> mEventQueue = new PriorityBlockingQueue<LOEvent>();
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private ImmutableViewportMetrics mViewportMetrics;
    private GeckoLayerClient mLayerClient;

    public LOKitThread() {
        TileProviderFactory.initialize();
    }

    private void tileRequest(ComposedTileLayer composedTileLayer, TileIdentifier tileId, boolean forceRedraw) {
        if (composedTileLayer.isStillValid(tileId)) {
            CairoImage image = mTileProvider.createTile(tileId.x, tileId.y, tileId.size, tileId.zoom);
            if (image != null) {
                mLayerClient.beginDrawing();
                SubTile tile = new SubTile(image, tileId);
                composedTileLayer.addTile(tile);
                mLayerClient.endDrawing(mViewportMetrics);
                mLayerClient.forceRender();
            }
        } else {
            composedTileLayer.cleanupInvalidTile(tileId);
        }
    }

    private void tileRerender(ComposedTileLayer composedTileLayer, SubTile tile) {
        if (composedTileLayer.isStillValid(tile.id) && !tile.markedForRemoval) {
            mLayerClient.beginDrawing();
            mTileProvider.rerenderTile(tile.getImage(), tile.id.x, tile.id.y, tile.id.size, tile.id.zoom);
            tile.invalidate();
            mLayerClient.endDrawing(mViewportMetrics);
            mLayerClient.forceRender();
        }
    }

    /** Handle the geometry change + draw. */
    private void redraw() {
        if (mLayerClient == null || mTileProvider == null) {
            // called too early...
            return;
        }

        mLayerClient.setPageRect(0, 0, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        mViewportMetrics = mLayerClient.getViewportMetrics();
        mLayerClient.setViewportMetrics(mViewportMetrics);

        zoomAndRepositionTheDocument();

        mLayerClient.forceRedraw();
    }

    private void zoomAndRepositionTheDocument() {
        if (mTileProvider.isSpreadsheet()) {
            // Don't do anything for spreadsheets - show at 100%
        } else if (mTileProvider.isTextDocument()) {
            // Always zoom text document to the beginning of the document and centered by width
            float centerY = mViewportMetrics.getCssViewport().centerY();
            mLayerClient.zoomTo(new RectF(0, centerY, mTileProvider.getPageWidth(), centerY));
        } else {
            // Other documents - always show the whole document on the screen,
            // regardless of document shape and orientation.
            if (mViewportMetrics.getViewport().width() < mViewportMetrics.getViewport().height()) {
                mLayerClient.zoomTo(mTileProvider.getPageWidth(), 0);
            } else {
                mLayerClient.zoomTo(0, mTileProvider.getPageHeight());
            }
        }
    }

    /** Invalidate everything + handle the geometry change */
    private void refresh() {
        mLayerClient.clearAndResetlayers();
        redraw();
    }

    private void changePart(int partIndex) {
        LOKitShell.showProgressSpinner();
        mTileProvider.changePart(partIndex);
        mViewportMetrics = mLayerClient.getViewportMetrics();
        mLayerClient.setViewportMetrics(mViewportMetrics.scaleTo(0.9f, new PointF()));
        refresh();
        LOKitShell.hideProgressSpinner();
    }

    private boolean loadDocument(String filename) {
        if (mApplication == null) {
            mApplication = LibreOfficeMainActivity.mAppContext;
        }

        mLayerClient = mApplication.getLayerClient();

        mTileProvider = TileProviderFactory.create(mLayerClient, filename);
        boolean isReady = mTileProvider.isReady();
        if (isReady) {
            LOKitShell.showProgressSpinner();
            mTileProvider.registerInvalidationCallback(this);
            refresh();
            LOKitShell.hideProgressSpinner();
        }
        return isReady;
    }

    public void closeDocument() {
        if (mTileProvider != null) {
            mTileProvider.close();
        }
    }

    public void run() {
        try {
            while (true) {
                processEvent(mEventQueue.take());
            }
        } catch (InterruptedException ex) {
        }
    }

    private void processEvent(LOEvent event) {
        switch (event.mType) {
            case LOEvent.LOAD:
                loadDocument(event.mFilename);
                break;
            case LOEvent.CLOSE:
                closeDocument();
                break;
            case LOEvent.SIZE_CHANGED:
                Log.i(LOGTAG, "Size change event!");
                redraw();
                break;
            case LOEvent.CHANGE_PART:
                changePart(event.mPartIndex);
                break;
            case LOEvent.TILE_REQUEST:
                tileRequest(event.mComposedTileLayer, event.mTileId, event.mForceRedraw);
                break;
            case LOEvent.TILE_RERENDER:
                tileRerender(event.mComposedTileLayer, event.mTile);
                break;
            case LOEvent.THUMBNAIL:
                createThumbnail(event.mTask);
                break;
            case LOEvent.TOUCH:
                if (!LOKitShell.isEditingEnabled()) {
                    return;
                }
                touch(event.mTouchType, event.mMotionEvent, event.mDocumentTouchCoordinate);
                break;
            case LOEvent.KEY_EVENT:
                if (!LOKitShell.isEditingEnabled()) {
                    return;
                }
                if (event.mKeyEventType == "KeyPress") {
                    mTileProvider.keyPress(event.mKeyEvent);
                } else if (event.mKeyEventType.equals("KeyRelease")) {
                    mTileProvider.keyRelease(event.mKeyEvent);
                }
                break;
        }
    }

    private void touch(String touchType, MotionEvent motionEvent, PointF mDocumentTouchCoordinate) {
        LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
        mTileProvider.mouseButtonDown(mDocumentTouchCoordinate);
    }

    private void createThumbnail(final ThumbnailCreator.ThumbnailCreationTask task) {
        final Bitmap bitmap = task.getThumbnail(mTileProvider);
        task.applyBitmap(bitmap);
    }

    public void queueEvent(LOEvent event) {
        mEventQueue.add(event);
    }

    public void clearQueue() {
        mEventQueue.clear();
    }

    @Override
    public void invalidate(RectF rect) {
        if (!LOKitShell.isEditingEnabled()) {
            return;
        }

        Log.i(LOGTAG, "Invalidate request: " + rect);

        mLayerClient = mApplication.getLayerClient();
        mLayerClient.invalidateTiles(rect);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
