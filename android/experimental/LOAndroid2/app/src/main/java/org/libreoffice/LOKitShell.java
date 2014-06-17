package org.libreoffice;


import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import org.mozilla.gecko.gfx.GeckoSoftwareLayerClient;
import org.mozilla.gecko.gfx.IntSize;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.LayerView;

import java.nio.ByteBuffer;

public class LOKitShell {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    public static int getDpi() {
        return 96;
    }

    public static int getScreenDepth() {
        return 24;
    }

    public static float computeRenderIntegrity() {
        return 0.0f;
    }

    public static ByteBuffer allocateDirectBuffer(int size) {
        if (size <= 0) {
            throw new IllegalArgumentException("Invalid size " + size);
        }

        ByteBuffer directBuffer = ByteBuffer.allocateDirect(size);
        //ByteBuffer directBuffer = nativeAllocateDirectBuffer(size);
        if (directBuffer == null) {
            throw new OutOfMemoryError("allocateDirectBuffer() returned null");
        } else if (!directBuffer.isDirect()) {
            throw new AssertionError("allocateDirectBuffer() did not return a direct buffer");
        }

        return directBuffer;
    }


    public static void freeDirectBuffer(ByteBuffer buffer) {
        if (buffer == null) {
            return;
        }

        if (!buffer.isDirect()) {
            throw new IllegalArgumentException("buffer must be direct");
        }
        //nativeFreeDirectBuffer(buffer);
        return ;
    }

    public static void bindWidgetTexture() {
    }

    public static void sendEvent(LOEvent event) {
        Log.i(LOGTAG, "Event: " + event.getTypeString());
    }

    public static void runGecko(String apkPath, String args, String url, boolean restoreSession) {
        // run gecko -- it will spawn its own thread
        // GeckoAppShell.nativeInit();

        Log.i(LOGTAG, "post native init");

        // Tell Gecko where the target byte buffer is for rendering
        //GeckoAppShell.setSoftwareLayerClient(GeckoApp.mAppContext.getSoftwareLayerClient());

        Log.i(LOGTAG, "setSoftwareLayerClient called");

        // First argument is the .apk path
        String combinedArgs = apkPath + " -greomni " + apkPath;
        if (args != null)
            combinedArgs += " " + args;
        if (url != null)
            combinedArgs += " -remote " + url;
        if (restoreSession)
            combinedArgs += " -restoresession";

        DisplayMetrics metrics = new DisplayMetrics();
        LibreOfficeMainActivity.mAppContext.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        combinedArgs += " -width " + metrics.widthPixels + " -height " + metrics.heightPixels;

        LibreOfficeMainActivity.mAppContext.runOnUiThread(new Runnable() {
            public void run() {
                geckoLoaded();
            }
        });

        //LOKitShell.nativeRun(combinedArgs);
    }

    // Called on the UI thread after Gecko loads.
    private static void geckoLoaded() {
        /*final LayerController layerController = LibreOfficeMainActivity.mAppContext.getLayerController();
        LayerView v = layerController.getView();
        mInputConnection = GeckoInputConnection.create(v);
        v.setInputConnectionHandler(mInputConnection);

        layerController.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View view, MotionEvent event) {
                if (event == null)
                    return true;
                GeckoAppShell.sendEventToGecko(new GeckoEvent(event));
                return true;
            }
        });

        layerController.notifyLayerClientOfGeometryChange();*/
    }

    public static void viewSizeChanged() {
    }

    public static void scheduleComposite() {
    }

    public static void schedulePauseComposition() {
    }

    public static void scheduleResumeComposition() {

    }
}
