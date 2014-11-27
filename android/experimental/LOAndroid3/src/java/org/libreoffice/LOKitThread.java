package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;;

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

    private boolean draw() {
        if (mTileProvider == null || mApplication == null) {
            // called too early...
            return false;
        }

        RectF rect = new RectF(0, 0, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        DisplayMetrics displayMetrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        mViewportMetrics = new ImmutableViewportMetrics(displayMetrics);
        mViewportMetrics = mViewportMetrics.setPageRect(rect, rect);

        GeckoLayerClient layerClient = mApplication.getLayerClient();

        layerClient.beginDrawing();
        layerClient.reevaluateTiles();
        layerClient.endDrawing(mViewportMetrics);

        return true;
    }

    /** Handle the geometry change + draw. */
    private void redraw() {
        if (mLayerClient == null || mTileProvider == null) {
            // called too early...
            return;
        }

        draw();

        RectF rect = new RectF(0, 0, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        mLayerClient.setPageRect(rect, rect);
        mLayerClient.setViewportMetrics(mLayerClient.getViewportMetrics());
        mLayerClient.setForceRedraw();
    }

    /** Invalidate everything + handle the geometry change + draw. */
    private void refresh() {
        Bitmap bitmap = mTileProvider.thumbnail(1000);
        if (bitmap != null) {
            mApplication.getLayerClient().getView().changeCheckerboardBitmap(bitmap, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        }

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
            mLayerClient.setTileProvider(mTileProvider);

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
        Log.i(LOGTAG, "processEvent: " + event.getTypeString());
        switch (event.mType) {
            case LOEvent.LOAD:
                loadDocument(event.getFilename());
                break;
            case LOEvent.CLOSE:
                closeDocument();
                break;
            case LOEvent.VIEWPORT:
                mViewportMetrics = event.getViewport();
                draw();
                break;
            case LOEvent.DRAW:
                draw();
                break;
            case LOEvent.SIZE_CHANGED:
                redraw();
                break;
            case LOEvent.CHANGE_PART:
                changePart(event.getPartIndex());
                break;
        }
    }

    public void queueEvent(LOEvent event) {
        Log.i(LOGTAG, "Event: " + event.getTypeString());
        mEventQueue.add(event);
    }

    public void clearQueue() {
        mEventQueue.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
