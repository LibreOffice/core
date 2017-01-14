package org.libreoffice;

import android.app.Application;
import android.os.Handler;

public class LibreOfficeApplication extends Application {

    private static Handler mainHandler;
    private static LOKitThread loKitThread;

    public LibreOfficeApplication() {
        loKitThread = new LOKitThread();
        loKitThread.start();

        mainHandler = new Handler();
    }

    public static LOKitThread getLoKitThread() {
        return loKitThread;
    }

    public static Handler getMainHandler() {
        return mainHandler;
    }
}
