package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.JsonWriter;
import android.util.Log;

import org.mozilla.gecko.gfx.FloatSize;
import org.mozilla.gecko.gfx.SubTile;
import org.mozilla.gecko.gfx.ViewportMetrics;

import java.io.IOException;
import java.io.StringWriter;
import java.nio.ByteBuffer;
import java.util.Random;
import java.util.concurrent.ConcurrentLinkedQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = LOKitThread.class.getSimpleName();

    private static final int TILE_SIZE = 256;
    private LibreOfficeMainActivity mApplication;
    private TileProvider mTileProvider;

    public ConcurrentLinkedQueue<LOEvent> gEvents = new ConcurrentLinkedQueue<LOEvent>();
    private ViewportMetrics mViewportMetrics;

    LOKitThread() {
    }

    private boolean draw() throws InterruptedException {
        int pageWidth = mTileProvider.getPageWidth();
        int pageHeight = mTileProvider.getPageHeight();

        mViewportMetrics = new ViewportMetrics();
        mViewportMetrics.setPageSize(new FloatSize(pageWidth, pageHeight));

        boolean shouldContinue = mApplication.getLayerClient().beginDrawing(mViewportMetrics);

        if (!shouldContinue) {
            return false;
        }

        for (SubTile tile : mTileProvider.getTileIterator()) {
            mApplication.getLayerClient().addTile(tile);
        }

        mApplication.getLayerClient().endDrawing();

        return true;
    }

    private void initialize() {
        mApplication = LibreOfficeMainActivity.mAppContext;
        mTileProvider = new LOKitTileProvider(mApplication.getLayerController());
    }

    public void run() {
        initialize();
        try {
            boolean drawn = false;
            while (true) {
                if (!gEvents.isEmpty()) {
                    processEvent(gEvents.poll());
                } else {
                    if (!drawn) {
                        drawn = draw();
                    }
                    Thread.sleep(100L);
                }
            }
        } catch (InterruptedException ex) {
        }
    }

    private void processEvent(LOEvent event) throws InterruptedException {
        switch (event.mType) {
            case LOEvent.VIEWPORT:
                mViewportMetrics = event.getViewport();
                break;
            case LOEvent.DRAW:
                draw();
                break;
            case LOEvent.SIZE_CHANGED:
                break;
        }
    }


}
