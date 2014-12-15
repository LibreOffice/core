package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.ComposedTileLayer;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.SubTile;

import java.util.concurrent.LinkedBlockingQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private LinkedBlockingQueue<LOEvent> mEventQueue = new LinkedBlockingQueue<LOEvent>();
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private ImmutableViewportMetrics mViewportMetrics;
    private GeckoLayerClient mLayerClient;

    public LOKitThread() {
        TileProviderFactory.initialize();
    }

    private void tileRequest(ComposedTileLayer composedTileLayer, TileIdentifier tileId) {
        if (composedTileLayer.isStillValid(tileId)) {
            mLayerClient.beginDrawing();
            CairoImage image = mTileProvider.createTile(tileId.x, tileId.y, tileId.size, tileId.zoom);
            SubTile tile = new SubTile(image, tileId);
            composedTileLayer.addTile(tile);
            mLayerClient.endDrawing(mViewportMetrics);
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
        mLayerClient.setViewportMetrics(mViewportMetrics.setZoomFactor(mViewportMetrics.getWidth() / mViewportMetrics.getPageWidth()));
        mLayerClient.forceRedraw();
    }

    /** Invalidate everything + handle the geometry change */
    private void refresh() {
        mLayerClient.clearAndResetlayers();
        redraw();
    }

    private void changePart(int partIndex) {
        LOKitShell.showProgressSpinner();
        mTileProvider.changePart(partIndex);
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
                redraw();
                break;
            case LOEvent.CHANGE_PART:
                changePart(event.mPartIndex);
                break;
            case LOEvent.TILE_REQUEST:
                tileRequest(event.mComposedTileLayer, event.mTileId);
                break;
        }
    }

    public void queueEvent(LOEvent event) {
        mEventQueue.add(event);
    }

    public void clearQueue() {
        mEventQueue.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
