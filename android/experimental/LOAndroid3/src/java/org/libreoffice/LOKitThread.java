package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import org.mozilla.gecko.TextSelectionHandle;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.SubTile;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private LinkedBlockingQueue<LOEvent> mEventQueue = new LinkedBlockingQueue<LOEvent>();

    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private InvalidationHandler mInvalidationHandler;
    private ImmutableViewportMetrics mViewportMetrics;
    private GeckoLayerClient mLayerClient;

    public LOKitThread() {
        mInvalidationHandler = null;
        TileProviderFactory.initialize();
    }

    @Override
    public void run() {
        while (true) {
            LOEvent event;
            try {
                event = mEventQueue.take();
                processEvent(event);
            } catch (InterruptedException exception) {
                throw new RuntimeException(exception);
            }
        }
    }

    /* Viewport changed, recheck if tiles need to be added / removed */
    private void tileReevaluationRequest(ComposedTileLayer composedTileLayer) {
        if (mTileProvider == null) {
            return;
        }
        List<SubTile> tiles = new ArrayList<SubTile>();

        mLayerClient.beginDrawing();
        composedTileLayer.addNewTiles(tiles);
        mLayerClient.endDrawing();

        for (SubTile tile : tiles) {
            TileIdentifier tileId = tile.id;
            CairoImage image = mTileProvider.createTile(tileId.x, tileId.y, tileId.size, tileId.zoom);
            mLayerClient.beginDrawing();
            if (image != null) {
                tile.setImage(image);
            }
            mLayerClient.endDrawing();
            mLayerClient.forceRender();
        }

        mLayerClient.beginDrawing();
        composedTileLayer.markTiles();
        composedTileLayer.clearMarkedTiles();
        mLayerClient.endDrawing();
        mLayerClient.forceRender();
    }

    /* Invalidate tiles that intersect the input rect */
    private void tileInvalidation(RectF rect) {
        if (mLayerClient == null || mTileProvider == null) {
            return;
        }

        mLayerClient.beginDrawing();

        List<SubTile> tiles = new ArrayList<SubTile>();
        mLayerClient.invalidateTiles(tiles, rect);

        for (SubTile tile : tiles) {
            CairoImage image = mTileProvider.createTile(tile.id.x, tile.id.y, tile.id.size, tile.id.zoom);
            tile.setImage(image);
            tile.invalidate();
        }
        mLayerClient.endDrawing();
        mLayerClient.forceRender();
    }

    /**
     * Handle the geometry change + draw.
     */
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
        mLayerClient.forceRender();
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

    /**
     * Invalidate everything + handle the geometry change
     */
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

    private void loadDocument(String filename) {
        if (mApplication == null) {
            mApplication = LibreOfficeMainActivity.mAppContext;
        }

        mLayerClient = mApplication.getLayerClient();

        mInvalidationHandler = new InvalidationHandler(LibreOfficeMainActivity.mAppContext);
        mTileProvider = TileProviderFactory.create(mLayerClient, mInvalidationHandler, filename);

        if (mTileProvider.isReady()) {
            LOKitShell.showProgressSpinner();
            refresh();
            LOKitShell.hideProgressSpinner();
        } else {
            closeDocument();
        }
    }

    public void closeDocument() {
        if (mTileProvider != null) {
            mTileProvider.close();
            mTileProvider = null;
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
                redraw();
                break;
            case LOEvent.CHANGE_PART:
                changePart(event.mPartIndex);
                break;
            case LOEvent.TILE_INVALIDATION:
                tileInvalidation(event.mInvalidationRect);
                break;
            case LOEvent.THUMBNAIL:
                createThumbnail(event.mTask);
                break;
            case LOEvent.TOUCH:
                touch(event.mTouchType, event.mDocumentCoordinate);
                break;
            case LOEvent.KEY_EVENT:
                keyEvent(event.mKeyEvent);
                break;
            case LOEvent.TILE_REEVALUATION_REQUEST:
                tileReevaluationRequest(event.mComposedTileLayer);
                break;
            case LOEvent.CHANGE_HANDLE_POSITION:
                changeHandlePosition(event.mHandleType, event.mDocumentCoordinate);
                break;
            case LOEvent.SWIPE_LEFT:
                onSwipeLeft();
                break;
            case LOEvent.SWIPE_RIGHT:
                onSwipeRight();
                break;
        }
    }

    private void changeHandlePosition(TextSelectionHandle.HandleType handleType, PointF documentCoordinate) {
        if (handleType == TextSelectionHandle.HandleType.MIDDLE) {
            LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
            mInvalidationHandler.setOverlayState(InvalidationHandler.OverlayState.CURSOR);
            mTileProvider.mouseButtonDown(documentCoordinate, 1);
            mTileProvider.mouseButtonUp(documentCoordinate, 1);
        } else if (handleType == TextSelectionHandle.HandleType.START) {
            mTileProvider.setTextSelectionStart(documentCoordinate);
        } else if (handleType == TextSelectionHandle.HandleType.END) {
            mTileProvider.setTextSelectionEnd(documentCoordinate);
        }
    }

    /**
     * Processes key events.
     */
    private void keyEvent(KeyEvent keyEvent) {
        if (!LOKitShell.isEditingEnabled()) {
            return;
        }
        if (mTileProvider == null) {
            return;
        }
        mTileProvider.sendKeyEvent(keyEvent);
    }

    private void onSwipeLeft() {
        mTileProvider.onSwipeLeft();
    }

    private void onSwipeRight() {
        mTileProvider.onSwipeRight();
    }

    /**
     * Processes touch events.
     */
    private void touch(String touchType, PointF documentCoordinate) {
        if (!LOKitShell.isEditingEnabled()) {
            return;
        }
        if (mTileProvider == null) {
            return;
        }
        if (touchType.equals("LongPress")) {
            mInvalidationHandler.setOverlayState(InvalidationHandler.OverlayState.SELECTION);
            mTileProvider.mouseButtonDown(documentCoordinate, 1);
            mTileProvider.mouseButtonUp(documentCoordinate, 1);
            mTileProvider.mouseButtonDown(documentCoordinate, 2);
            mTileProvider.mouseButtonUp(documentCoordinate, 2);
        } else { // "SingleTap"
            LibreOfficeMainActivity.mAppContext.showSoftKeyboard();
            mInvalidationHandler.setOverlayState(InvalidationHandler.OverlayState.CURSOR);
            mTileProvider.mouseButtonDown(documentCoordinate, 1);
            mTileProvider.mouseButtonUp(documentCoordinate, 1);
            if (mInvalidationHandler.getOverlayState() == InvalidationHandler.OverlayState.SELECTION) {
                mTileProvider.setTextSelectionReset();
            }
        }
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
