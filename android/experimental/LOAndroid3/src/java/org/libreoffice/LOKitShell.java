package org.libreoffice;


import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;


public class LOKitShell {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    public static float getDpi() {
        DisplayMetrics metrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        return  metrics.density * 160;
    }

    public static void sendEvent(LOEvent event) {
        if (LibreOfficeMainActivity.mAppContext != null && LibreOfficeMainActivity.mAppContext.getLOKitThread() != null) {
            LibreOfficeMainActivity.mAppContext.getLOKitThread().queueEvent(event);
        }
    }

    public static void viewSizeChanged() {
        Log.i(LOGTAG, "viewSizeChanged");
    }

    // Get a Handler for the main java thread
    public static Handler getMainHandler() {
        return LibreOfficeMainActivity.mAppContext.mMainHandler;
    }

    public static void queueRedraw() {
        LOKitShell.sendEvent(LOEventFactory.redraw());
    }
}
