package org.libreoffice;


import android.os.Handler;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.MotionEvent;


public class LOKitShell {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    public static float getDpi() {
        DisplayMetrics metrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        return metrics.density * 160;
    }

    // Get a Handler for the main java thread
    public static Handler getMainHandler() {
        return LibreOfficeMainActivity.mAppContext.mMainHandler;
    }

    public static void showProgressSpinner() {
        getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.showProgressSpinner();
            }
        });
    }

    public static void hideProgressSpinner() {
        getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.hideProgressSpinner();
            }
        });
    }

    public static DisplayMetrics getDisplayMetrics() {
        if (LibreOfficeMainActivity.mAppContext == null) {
            return null;
        }
        DisplayMetrics metrics = new DisplayMetrics();
        LibreOfficeMainActivity.mAppContext.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        return metrics;
    }

    // EVENTS

    /**
     * Make sure LOKitThread is running and send event to it.
     */
    public static void sendEvent(LOEvent event) {
        if (LibreOfficeMainActivity.mAppContext != null && LibreOfficeMainActivity.mAppContext.getLOKitThread() != null) {
            LibreOfficeMainActivity.mAppContext.getLOKitThread().queueEvent(event);
        }
    }

    public static void sendThumbnailEvent(ThumbnailCreator.ThumbnailCreationTask task) {
        LOKitShell.sendEvent(new LOEvent(LOEvent.THUMBNAIL, task));
    }

    /**
     * Send touch event to LOKitThread.
     */
    public static void sentTouchEvent(String touchType, MotionEvent motionEvent) {
        LOKitShell.sendEvent(new LOEvent(LOEvent.TOUCH, "SingleTap", motionEvent));
    }

    /**
     * Send key press event to LOKitThread.
     */
    public static void sendKeyPressEvent(KeyEvent event) {
        LOKitShell.sendEvent(new LOEvent(LOEvent.KEY_PRESS, event));
    }
}
