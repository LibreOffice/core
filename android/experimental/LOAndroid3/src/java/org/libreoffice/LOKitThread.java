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

    private boolean draw() throws InterruptedException {
        int pageWidth = mTileProvider.getPageWidth();
        int pageHeight = mTileProvider.getPageHeight();

        mViewportMetrics = new ViewportMetrics();
        mViewportMetrics.setPageSize(new FloatSize(pageWidth, pageHeight));

        GeckoLayerClient layerClient = mApplication.getLayerClient();
        ViewportMetrics metrics = mApplication.getLayerController().getViewportMetrics();

        RectF viewport = metrics.getClampedViewport();
        float zoomFactor = metrics.getZoomFactor();

        boolean shouldContinue = layerClient.beginDrawing(mViewportMetrics);

        if (!shouldContinue) {
            return false;
        }

        int minX = ((int) viewport.left / TILE_SIZE) * TILE_SIZE;
        int minY = ((int) viewport.top / TILE_SIZE) * TILE_SIZE;
        int maxX = (((int) viewport.right / TILE_SIZE) + 1) * TILE_SIZE;
        int maxY = (((int) viewport.bottom / TILE_SIZE) + 1) * TILE_SIZE;

        Rect rect = new Rect(
                Math.round(minX / zoomFactor),
                Math.round(minY / zoomFactor),
                Math.round(maxX / zoomFactor),
                Math.round(maxY / zoomFactor));

        ArrayList<SubTile> removeTiles = new ArrayList<SubTile>();
        for (SubTile tile : layerClient.getTiles()) {
            if (!rect.intersects(tile.x, tile.y, tile.x + TILE_SIZE, tile.y + TILE_SIZE)) {
                removeTiles.add(tile);
            }
        }
        layerClient.getTiles().removeAll(removeTiles);

        for (int y = minY; y <= maxY; y+=TILE_SIZE) {
            for (int x = minX; x <= maxX; x+=TILE_SIZE) {
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
