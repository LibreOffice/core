package org.libreoffice;

import android.app.Application;
import android.os.Handler;

public class LibreOfficeApplication extends Application {

    private static Handler mainHandler;

    public LibreOfficeApplication() {
        mainHandler = new Handler();
    }

    public static Handler getMainHandler() {
        return mainHandler;
    }
}
