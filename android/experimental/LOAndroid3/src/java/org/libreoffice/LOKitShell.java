package org.libreoffice;


import android.os.Handler;
import android.util.DisplayMetrics;

public class LOKitShell {
    private static final String LOGTAG = LOKitShell.class.getSimpleName();

    public static int getDpi() {
        DisplayMetrics metrics = LibreOfficeMainActivity.mAppContext.getResources().getDisplayMetrics();
        return (int) metrics.density * 160;
    }

    public static void bindWidgetTexture() {
    }

    public static void sendEvent(LOEvent event) {
        if (LibreOfficeMainActivity.mAppContext != null && LibreOfficeMainActivity.mAppContext.getLOKitThread() != null) {
            LibreOfficeMainActivity.mAppContext.getLOKitThread().queueEvent(event);
        }
    }

    public static void viewSizeChanged() {
    }

    public static void scheduleComposite() {
    }

    public static void schedulePauseComposition() {
    }

    public static void scheduleResumeComposition() {

    }

    // Get a Handler for the main java thread
    public static Handler getMainHandler() {
        return LibreOfficeMainActivity.mAppContext.mMainHandler;
    }
}
