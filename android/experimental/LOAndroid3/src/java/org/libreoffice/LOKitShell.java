package org.libreoffice;


import android.os.Handler;
import android.util.DisplayMetrics;


public class LOKitShell {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    public static float getDpi() {
        DisplayMetrics metrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        return metrics.density * 160;
    }

    public static void sendEvent(LOEvent event) {
        if (LibreOfficeMainActivity.mAppContext != null && LibreOfficeMainActivity.mAppContext.getLOKitThread() != null) {
            LibreOfficeMainActivity.mAppContext.getLOKitThread().queueEvent(event);
        }
    }

    // Get a Handler for the main java thread
    public static Handler getMainHandler() {
        return LibreOfficeMainActivity.mAppContext.mMainHandler;
    }

    public static void queueRedraw() {
        LOKitShell.sendEvent(LOEventFactory.redraw());
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

    public static void sendThumbnailEvent(ThumbnailCreator.ThumbnailCreationTask task) {
        LOKitShell.sendEvent(LOEventFactory.thumbnail(task));
    }
}
