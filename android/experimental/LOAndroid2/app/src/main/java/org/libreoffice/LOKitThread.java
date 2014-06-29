package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.JsonWriter;

import org.mozilla.gecko.gfx.ViewportMetrics;

import java.io.IOException;
import java.io.StringWriter;
import java.nio.ByteBuffer;
import java.util.Random;
import java.util.concurrent.ConcurrentLinkedQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = "GeckoThread";
    private static final int TILE_SIZE = 256;


    public ConcurrentLinkedQueue<LOEvent> gEvents = new ConcurrentLinkedQueue<LOEvent>();
    private ViewportMetrics mViewportMetrics;
    private Random rand = new Random();

    LOKitThread() {
    }

    private boolean draw() throws InterruptedException {
        final LibreOfficeMainActivity application = LibreOfficeMainActivity.mAppContext;

        Bitmap originalBitmap = application.getLayerClient().getLayerController().getDrawable("dummy_page");

        String metadata;
        if (mViewportMetrics == null) {
            metadata = createJson(0, 0, originalBitmap.getWidth(), originalBitmap.getHeight(), originalBitmap.getWidth(), originalBitmap.getHeight(), 0, 0, 1.0);
        } else {
            metadata = createJson(mViewportMetrics);
        }

        Rect bufferRect = application.getLayerClient().beginDrawing(originalBitmap.getWidth(), originalBitmap.getHeight(), 256, 256, metadata, false);
        if (bufferRect == null)
            return false;

        ByteBuffer buffer = application.getLayerClient().lockBuffer();
        for (Integer i = 1; i <= 9; i++) {
            String imageName = "d" + i;
            Bitmap bitmap = application.getLayerClient().getLayerController().getDrawable(imageName);
            bitmap.copyPixelsToBuffer(buffer.asIntBuffer());
            buffer.position(buffer.position() + bitmap.getByteCount());
        }
        buffer.position(0);

        application.getLayerClient().unlockBuffer();
        application.getLayerClient().endDrawing(0, 0, originalBitmap.getWidth(), originalBitmap.getHeight());

        application.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                application.getLayerClient().handleMessage("Viewport:UpdateLater", null);
            }
        });

        return true;
    }

    private String createJson(ViewportMetrics viewportMetrics) {
        return createJson(
                (int) viewportMetrics.getOrigin().x,
                (int) viewportMetrics.getOrigin().y,
                (int) viewportMetrics.getSize().width,
                (int) viewportMetrics.getSize().height,
                (int) viewportMetrics.getPageSize().width,
                (int) viewportMetrics.getPageSize().height,
                (int) viewportMetrics.getViewportOffset().x,
                (int) viewportMetrics.getViewportOffset().y,
                viewportMetrics.getZoomFactor());
    }

    private String createJson(int x, int y, int width, int height, int pageWidth, int pageHeight, int offsetX, int offsetY, double zoom) {
        try {
            StringWriter stringWriter = new StringWriter();
            JsonWriter writer = new JsonWriter(stringWriter);
            writer.beginObject();
            writer.name("x").value(x);
            writer.name("y").value(y);
            writer.name("width").value(width);
            writer.name("height").value(height);
            writer.name("pageWidth").value(pageWidth);
            writer.name("pageHeight").value(pageHeight);
            writer.name("offsetX").value(offsetX);
            writer.name("offsetY").value(offsetY);
            writer.name("zoom").value(zoom);
            writer.name("backgroundColor").value("rgb(255,255,255)");
            writer.endObject();
            writer.close();
            return stringWriter.toString();
        } catch (IOException ex) {
        }
        return null;
    }

    private short convertTo16Bit(int color) {
        int r = Color.red(color) >> 3, g = Color.green(color) >> 2, b = Color.blue(color) >> 3;
        int c = ((r << 11) | (g << 5) | b);
        // Swap endianness.
        return (short) ((c >> 8) | ((c & 0xff) << 8));
    }

    private Bitmap convert(Bitmap bitmap, Bitmap.Config config) {
        Bitmap convertedBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), config);
        Canvas canvas = new Canvas(convertedBitmap);
        Paint paint = new Paint();
        paint.setColor(Color.BLACK);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return convertedBitmap;
    }


    public void run() {
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
