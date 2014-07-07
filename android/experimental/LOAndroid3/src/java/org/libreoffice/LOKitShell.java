package org.libreoffice;


import android.util.DisplayMetrics;
import android.util.Log;

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
    }

    // Called on the UI thread after Gecko loads.
    private static void geckoLoaded() {
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
