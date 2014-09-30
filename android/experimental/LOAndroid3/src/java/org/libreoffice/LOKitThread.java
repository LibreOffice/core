package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.LayerController;

import java.util.concurrent.LinkedBlockingQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private LinkedBlockingQueue<LOEvent> mEventQueue = new LinkedBlockingQueue<LOEvent>();
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private ImmutableViewportMetrics mViewportMetrics;
    private GeckoLayerClient mLayerClient;
    private LayerController mController;

    public LOKitThread() {
        TileProviderFactory.initialize();
    }

    private boolean draw() {
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

    private void refresh() {
        Bitmap bitmap = mTileProvider.thumbnail(1000);
        if (bitmap != null) {
            mApplication.getLayerController().getView().changeCheckerboardBitmap(bitmap, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        }

        mLayerClient.clearAllTiles();

        RectF rect = new RectF(0, 0, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
        mController.setPageRect(rect, rect);
        mController.setForceRedraw();
    }

    private void changePart(int partIndex) {
        mTileProvider.changePart(partIndex);
        refresh();
    }

    private boolean load(String filename) {
        if (mApplication == null) {
            mApplication = LibreOfficeMainActivity.mAppContext;
        }

        mController = mApplication.getLayerController();
        mLayerClient = mApplication.getLayerClient();

        if (mTileProvider != null) {
            mTileProvider.close();
        }

        mTileProvider = TileProviderFactory.create(mController, filename);
        mLayerClient.setTileProvider(mTileProvider);

        boolean isReady = mTileProvider.isReady();
        if (isReady) {
            refresh();
            LOKitShell.getMainHandler().post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.hideProgressBar();
                }
            });

        }
        return isReady;
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
                load(event.getFilename());
                break;
            case LOEvent.VIEWPORT:
                mViewportMetrics = event.getViewport();
                draw();
                break;
            case LOEvent.DRAW:
                draw();
                break;
            case LOEvent.SIZE_CHANGED:
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
