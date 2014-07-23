package org.libreoffice;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerController;

public class LibreOfficeMainActivity extends Activity {

    private static final String LOGTAG = "LibreOfficeMainActivity";
    private static final String DEFAULT_DOC_PATH = "/assets/test1.odt";

    private LinearLayout mMainLayout;
    private RelativeLayout mGeckoLayout;
    private static LayerController mLayerController;
    private static GeckoLayerClient mLayerClient;
    private static LOKitThread sLOKitThread;

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

        Log.w(LOGTAG, "zerdatime " + SystemClock.uptimeMillis() + " - onCreate");

        String inputFile = new String();
        if (getIntent().getData() != null) {
            inputFile = getIntent().getData().getEncodedPath();
        }
        else {
            inputFile = DEFAULT_DOC_PATH;
        }

        setContentView(R.layout.activity_main);

        // setup gecko layout
        mGeckoLayout = (RelativeLayout) findViewById(R.id.gecko_layout);
        mMainLayout = (LinearLayout) findViewById(R.id.main_layout);

        if (mLayerController == null) {
            mLayerController = new LayerController(this);

            Log.e(LOGTAG, "### Creating GeckoSoftwareLayerClient");
            mLayerClient = new GeckoLayerClient(this);
            Log.e(LOGTAG, "### Done creating GeckoSoftwareLayerClient");

            mLayerController.setLayerClient(mLayerClient);
            mGeckoLayout.addView(mLayerController.getView(), 0);
        }

        sLOKitThread = new LOKitThread(inputFile);
        sLOKitThread.start();

        Log.w(LOGTAG, "zerdatime " + SystemClock.uptimeMillis() + " - UI almost up");
    }

    public LOKitThread getLOKitThread() {
        return sLOKitThread;
    }

    public static GeckoLayerClient getLayerClient() {
        return mLayerClient;
    }

    public static LayerController getLayerController() {
        return mLayerController;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
