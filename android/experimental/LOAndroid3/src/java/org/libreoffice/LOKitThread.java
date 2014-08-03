package org.libreoffice;

import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;

import org.mozilla.gecko.gfx.FloatSize;
import org.mozilla.gecko.gfx.GeckoLayerClient;
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
    private String mInputFile;

    LOKitThread(String inputFile) {
        mInputFile = inputFile;
    }

    RectF normlizeRect(ViewportMetrics metrics) {
        RectF rect = metrics.getViewport();
        float zoomFactor = metrics.getZoomFactor();
        return new RectF(rect.left / zoomFactor, rect.top / zoomFactor, rect.right / zoomFactor, rect.bottom / zoomFactor);
    }

    Rect roundToTileSize(RectF input, int tileSize) {
        int minX = (Math.round(input.left)    / tileSize) * tileSize;
        int minY = (Math.round(input.top)     / tileSize) * tileSize;
        int maxX = ((Math.round(input.right)  / tileSize) + 1) * tileSize;
        int maxY = ((Math.round(input.bottom) / tileSize) + 1) * tileSize;
        return new Rect(minX, minY, maxX, maxY);
    }

    private boolean draw() throws InterruptedException {
        int pageWidth = mTileProvider.getPageWidth();
        int pageHeight = mTileProvider.getPageHeight();

        mViewportMetrics = new ViewportMetrics();
        mViewportMetrics.setPageSize(new FloatSize(pageWidth, pageHeight));

        GeckoLayerClient layerClient = mApplication.getLayerClient();
        boolean shouldContinue = layerClient.beginDrawing(mViewportMetrics);

        if (!shouldContinue) {
            return false;
        }

        ViewportMetrics metrics = mApplication.getLayerController().getViewportMetrics();
        RectF viewport = normlizeRect(metrics);
        Rect rect = roundToTileSize(viewport, TILE_SIZE);

        ArrayList<SubTile> removeTiles = new ArrayList<SubTile>();
        for (SubTile tile : layerClient.getTiles()) {
            if (!rect.intersects(tile.x, tile.y, tile.x + TILE_SIZE, tile.y + TILE_SIZE)) {
                removeTiles.add(tile);
            }
        }
        layerClient.getTiles().removeAll(removeTiles);

        for (int y = rect.top; y <= rect.bottom; y += TILE_SIZE) {
            for (int x = rect.left; x <= rect.right; x += TILE_SIZE) {
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

        layerClient.endDrawing();

        return true;
    }

    private boolean initialize() {
        mApplication = LibreOfficeMainActivity.mAppContext;
        mTileProvider = new LOKitTileProvider(mApplication.getLayerController(), mInputFile);
        return mTileProvider.isReady();
    }

    public void run() {
        if (initialize()) {
            try {
                boolean drawn = false;
                while (true) {
                    processEvent(mEventQueue.take());
                }
            } catch (InterruptedException ex) {
            }
        }
    }

    private void processEvent(LOEvent event) throws InterruptedException {
        switch (event.mType) {
            case LOEvent.VIEWPORT:
                mViewportMetrics = event.getViewport();
                draw();
                break;
            case LOEvent.DRAW:
                draw();
                break;
            case LOEvent.SIZE_CHANGED:
                break;
        }
    }

    public void queueEvent(LOEvent event) {
        Log.i(LOGTAG, "Event: " + event.getTypeString());
        mEventQueue.add(event);
    }
}
