package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.JsonWriter;

import org.mozilla.gecko.gfx.ViewportMetrics;

import java.io.IOException;
import java.io.StringWriter;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.ConcurrentLinkedQueue;

public class LOKitThread extends Thread {
    private static final String LOGTAG = "GeckoThread";

    public ConcurrentLinkedQueue<LOEvent> gEvents = new ConcurrentLinkedQueue<LOEvent>();
    private ViewportMetrics mViewportMetrics;
    private Random rand = new Random();

    LOKitThread() {
    }

    private void draw() throws InterruptedException {
        final LibreOfficeMainActivity application = LibreOfficeMainActivity.mAppContext;

        Bitmap bitmap = application.getSoftwareLayerClient().getLayerController().getDrawable16("dummy_page");
        bitmap  = convert(bitmap, Bitmap.Config.RGB_565);

        application.getSoftwareLayerClient().beginDrawing(bitmap.getWidth(), bitmap.getHeight());
        //application.getSoftwareLayerClient().beginDrawing(500,500);

        ByteBuffer buffer = application.getSoftwareLayerClient().getBuffer();
        bitmap.copyPixelsToBuffer(buffer.asIntBuffer());

        /*short mainColor16 = convertTo16Bit(rand.nextInt());

        short[] mainPattern = new short[500];
        Arrays.fill(mainPattern, mainColor16);

        buffer.rewind();
        ShortBuffer shortBuffer = buffer.asShortBuffer();
        for (int i = 0; i < 500; i++) {
            shortBuffer.put(mainPattern);
        }*/

        StringWriter stringWriter = new StringWriter();

        try {
            JsonWriter writer = new JsonWriter(stringWriter);
            writer.beginObject();
            if (mViewportMetrics == null) {
                writer.name("x").value(0);
                writer.name("y").value(0);
                writer.name("width").value(bitmap.getWidth());
                writer.name("height").value(bitmap.getHeight());
                writer.name("pageWidth").value(1000);
                writer.name("pageHeight").value(5000);
                writer.name("offsetX").value(0);
                writer.name("offsetY").value(0);
                writer.name("zoom").value(1.0);
                writer.name("allowZoom").value(true);
            } else {
                writer.name("x").value(mViewportMetrics.getOrigin().x);
                writer.name("y").value(mViewportMetrics.getOrigin().y);
                writer.name("width").value(mViewportMetrics.getSize().width);
                writer.name("height").value(mViewportMetrics.getSize().height);
                writer.name("pageWidth").value(mViewportMetrics.getPageSize().width);
                writer.name("pageHeight").value(mViewportMetrics.getPageSize().height);
                writer.name("offsetX").value(mViewportMetrics.getViewportOffset().x);
                writer.name("offsetY").value(mViewportMetrics.getViewportOffset().y);
                writer.name("zoom").value(mViewportMetrics.getZoomFactor());
                writer.name("allowZoom").value(mViewportMetrics.getAllowZoom());
            }
            writer.name("backgroundColor").value("rgb(255,255,255)");
            writer.endObject();
            writer.close();
        } catch (IOException ex) {
        }

        application.getSoftwareLayerClient().endDrawing(0, 0, bitmap.getWidth(), bitmap.getHeight(), stringWriter.toString(), false);
        //application.getSoftwareLayerClient().endDrawing(0, 0, 500, 500, stringWriter.toString(), false);
        application.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                application.getSoftwareLayerClient().handleMessage("Viewport:UpdateLater", null);
            }
        });

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
                    if(!drawn) {
                        draw();
                        drawn = true;
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
