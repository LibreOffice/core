package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;

import org.mozilla.gecko.gfx.FloatSize;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.ImmutableViewportMetrics;
import org.mozilla.gecko.gfx.SubTile;
import org.mozilla.gecko.gfx.ViewportMetrics;

import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private static final int TILE_SIZE = 256;
    public LinkedBlockingQueue<LOEvent> mEventQueue = new LinkedBlockingQueue<LOEvent>();
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;
    private ViewportMetrics mViewportMetrics;
    private Rect mOldRect;
    private boolean mCheckboardImageSet = false;

    public LOKitThread() {
        TileProviderFactory.initialize();
    }

    private RectF normlizeRect(ImmutableViewportMetrics metrics) {
        RectF rect = metrics.getViewport();
        float zoomFactor = metrics.zoomFactor;
        return new RectF(rect.left / zoomFactor, rect.top / zoomFactor, rect.right / zoomFactor, rect.bottom / zoomFactor);
    }

    private Rect roundToTileSize(RectF input, int tileSize) {
        int minX = (Math.round(input.left) / tileSize) * tileSize;
        int minY = (Math.round(input.top) / tileSize) * tileSize;
        int maxX = ((Math.round(input.right) / tileSize) + 1) * tileSize;
        int maxY = ((Math.round(input.bottom) / tileSize) + 1) * tileSize;
        return new Rect(minX, minY, maxX, maxY);
    }

    private Rect inflate(Rect rect, int inflateSize) {
        Rect newRect = new Rect(rect);
        newRect.left -= inflateSize;
        newRect.left = newRect.left < 0 ? 0 : newRect.left;

        newRect.top -= inflateSize;
        newRect.top = newRect.top < 0 ? 0 : newRect.top;

        newRect.right += inflateSize;
        newRect.bottom += inflateSize;

        return newRect;
    }

    private boolean draw() throws InterruptedException {
        Log.i(LOGTAG, "tilerender draw");
        int pageWidth = mTileProvider.getPageWidth();
        int pageHeight = mTileProvider.getPageHeight();

        mViewportMetrics = new ViewportMetrics();
        FloatSize size = new FloatSize(pageWidth, pageHeight);
        mViewportMetrics.setPageSize(size, size);

        GeckoLayerClient layerClient = mApplication.getLayerClient();
        layerClient.beginDrawing(mViewportMetrics);

        ImmutableViewportMetrics metrics = mApplication.getLayerController().getViewportMetrics();
        RectF viewport = normlizeRect(metrics);
        Rect rect = inflate(roundToTileSize(viewport, TILE_SIZE), TILE_SIZE);

        mOldRect = rect;

        Log.i(LOGTAG, "tilerender rect: " + rect);

        long start = System.currentTimeMillis();

        ArrayList<SubTile> removeTiles = new ArrayList<SubTile>();
        for (SubTile tile : layerClient.getTiles()) {
            Rect tileRect = new Rect(tile.x, tile.y, tile.x + TILE_SIZE, tile.y + TILE_SIZE);
            if (!Rect.intersects(rect, tileRect)) {
                Log.i(LOGTAG, "tilerender delete " + tileRect);
                tile.destroy();
                removeTiles.add(tile);
            }
        }
        Log.i(LOGTAG, "TileRendering Remove: " + (System.currentTimeMillis() - start));

        layerClient.getTiles().removeAll(removeTiles);

        Log.i(LOGTAG, "TileRendering Clear: " + (System.currentTimeMillis() - start));

        for (int y = rect.top; y < rect.bottom; y += TILE_SIZE) {
            for (int x = rect.left; x < rect.right; x += TILE_SIZE) {
                if (x > pageWidth) {
                    continue;
                }
                if (y > pageHeight) {
                    continue;
                }
                boolean contains = false;
                for (SubTile tile : layerClient.getTiles()) {
                    if (tile.x == x && tile.y == y) {
                        contains = true;
                    }
                }
                if (!contains) {
                    SubTile tile = mTileProvider.createTile(x, y);
                    layerClient.addTile(tile);
                }
            }
        }

        Log.i(LOGTAG, "TileRendering Add: " + (System.currentTimeMillis() - start));

        layerClient.endDrawing();
        Log.i(LOGTAG, "tilerender end draw");

        return true;
    }

    private void changePart(int partIndex) throws InterruptedException {
        mTileProvider.changePart(partIndex);
        GeckoLayerClient layerClient = mApplication.getLayerClient();
        layerClient.getTiles().clear();
        LOKitShell.sendEvent(LOEvent.draw(new Rect()));
    }

    private boolean load(String filename) {
        if (mApplication == null) {
            mApplication = LibreOfficeMainActivity.mAppContext;
        }
        if (mTileProvider != null) {
            mTileProvider.close();
        }
        mTileProvider = TileProviderFactory.create(mApplication.getLayerController(), filename);
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
