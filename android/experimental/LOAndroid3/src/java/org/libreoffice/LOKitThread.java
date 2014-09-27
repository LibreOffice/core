package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.DisplayMetrics;
import android.util.Log;

import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ViewportMetrics;

import java.util.concurrent.LinkedBlockingQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    public LinkedBlockingQueue<LOEvent> mEventQueue = new LinkedBlockingQueue<LOEvent>();
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private ViewportMetrics mViewportMetrics;
    private boolean mCheckboardImageSet = false;

    public LOKitThread() {
        TileProviderFactory.initialize();
    }

    private boolean draw() throws InterruptedException {
        int pageWidth = mTileProvider.getPageWidth();
        int pageHeight = mTileProvider.getPageHeight();

        RectF rect = new RectF(0, 0, pageWidth, pageHeight);
        DisplayMetrics displayMetrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        mViewportMetrics = new ViewportMetrics(displayMetrics);
        mViewportMetrics.setPageRect(rect, rect);

        GeckoLayerClient layerClient = mApplication.getLayerClient();

        layerClient.beginDrawing();

        layerClient.reevaluateTiles();

        layerClient.endDrawing(mViewportMetrics);

        return true;
    }

    private void changePart(int partIndex) throws InterruptedException {
        mTileProvider.changePart(partIndex);
        GeckoLayerClient layerClient = mApplication.getLayerClient();
        updateCheckbardImage();
        LOKitShell.sendEvent(LOEvent.draw(new Rect()));
    }

    private boolean load(String filename) {
        if (mApplication == null) {
            mApplication = LibreOfficeMainActivity.mAppContext;
        }
        if (mTileProvider != null) {
            mTileProvider.close();
        }
        GeckoLayerClient layerClient = mApplication.getLayerClient();
        mTileProvider = TileProviderFactory.create(mApplication.getLayerController(), filename);
        layerClient.setTileProvider(mTileProvider);

        boolean isReady = mTileProvider.isReady();
        if (isReady) {
            updateCheckbardImage();
        }
        return isReady;
    }

    private void updateCheckbardImage() {
        if (!mCheckboardImageSet) {
            Log.i(LOGTAG, "Generate thumbnail!");
            Bitmap bitmap = mTileProvider.thumbnail();
            Log.i(LOGTAG, "Done generate thumbnail!");
            if (bitmap != null) {
                Log.i(LOGTAG, "Setting checkboard image!");
                mApplication.getLayerController().getView().changeCheckerboardBitmap(bitmap, mTileProvider.getPageWidth(), mTileProvider.getPageHeight());
                Log.i(LOGTAG, "Done setting checkboard image!!");
                mCheckboardImageSet = true;
            }
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

    private void processEvent(LOEvent event) throws InterruptedException {
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
