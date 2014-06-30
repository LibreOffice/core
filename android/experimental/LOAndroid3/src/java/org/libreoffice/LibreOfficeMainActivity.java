package org.libreoffice;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import org.mozilla.gecko.gfx.GeckoSoftwareLayerClient;
import org.mozilla.gecko.gfx.LayerController;
import org.mozilla.gecko.gfx.LayerView;

import org.libreoffice.android.LibreOfficeKit;

import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;

public class LibreOfficeMainActivity extends Activity {

    private static final String LOGTAG = "LibreOfficeMainActivity";

    private LinearLayout mMainLayout;
    private RelativeLayout mGeckoLayout;
    private static LayerController mLayerController;
    private static GeckoSoftwareLayerClient mLayerClient;
    private static LOKitThread sLOKitThread;

    private XComponentContext context;
    private XMultiComponentFactory mcf;
    private XComponentLoader componentLoader;

    public static LibreOfficeMainActivity mAppContext;

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public DisplayMetrics getDisplayMetrics() {
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        return metrics;
    }

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        mAppContext = this;

        super.onCreate(savedInstanceState);

        try {
            // enable debugging messages as the first thing
            LibreOfficeKit.putenv("SAL_LOG=+WARN+INFO-INFO.legacy.osl");

            LibreOfficeKit.init(this);

            setContentView(R.layout.activity_main);

            Log.w(LOGTAG, "zerdatime " + SystemClock.uptimeMillis() + " - onCreate");

            /*
            String input = "/assets/test1.odt";

            String[] argv = { "lo-document-loader", input };

            LibreOfficeKit.setCommandArgs(argv);

            Bootstrap.initVCL();

            context = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(LOGTAG, "context is" + (context!=null ? " not" : "") + " null");

            mcf = context.getServiceManager();

            Log.i(LOGTAG, "mcf is" + (mcf!=null ? " not" : "") + " null");

            Object desktop = mcf.createInstanceWithContext("com.sun.star.frame.Desktop", context);
            Log.i(LOGTAG, "desktop is" + (desktop!=null ? " not" : "") + " null");

            componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);
            Log.i(LOGTAG, "componentLoader is" + (componentLoader!=null ? " not" : "") + " null");
            */
        } catch (Exception e) {
            e.printStackTrace(System.err);
            //finish();
        }

        setContentView(R.layout.activity_main);

        // setup gecko layout
        mGeckoLayout = (RelativeLayout) findViewById(R.id.gecko_layout);
        mMainLayout = (LinearLayout) findViewById(R.id.main_layout);


        if (mLayerController == null) {
            mLayerController = new LayerController(this);

            Log.e(LOGTAG, "### Creating GeckoSoftwareLayerClient");
            mLayerClient = new GeckoSoftwareLayerClient(this);
            Log.e(LOGTAG, "### Done creating GeckoSoftwareLayerClient");

            mLayerController.setLayerClient(mLayerClient);
            mGeckoLayout.addView(mLayerController.getView(), 0);
        }

        mLayerController.notifyLayerClientOfGeometryChange();

        sLOKitThread = new LOKitThread();
        sLOKitThread.start();


        Log.w(LOGTAG, "zerdatime " + SystemClock.uptimeMillis() + " - UI almost up");
    }

    public static GeckoSoftwareLayerClient getLayerClient() {
        return mLayerClient;
    }

    public static LayerController getLayerController() {
        return mLayerController;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
