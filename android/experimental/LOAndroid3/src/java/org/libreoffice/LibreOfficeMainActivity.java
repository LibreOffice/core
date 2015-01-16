package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.widget.DrawerLayout;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerView;

import java.util.ArrayList;
import java.util.List;

public class LibreOfficeMainActivity extends LOAbout {

    private static final String LOGTAG = "LibreOfficeMainActivity";
    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";

    public static LibreOfficeMainActivity mAppContext;

    private static GeckoLayerClient mLayerClient;
    private static LOKitThread sLOKitThread;

    public Handler mMainHandler;

    private DrawerLayout mDrawerLayout;
    private RelativeLayout mGeckoLayout;
    private ListView mDrawerList;
    private List<DocumentPartView> mDocumentPartView = new ArrayList<DocumentPartView>();
    private DocumentPartViewListAdapter mDocumentPartViewListAdapter;
    private String mInputFile;

    public LibreOfficeMainActivity() {
        super(/*newActivity=*/false);
    }

    public static GeckoLayerClient getLayerClient() {
        return mLayerClient;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        switch (id) {
            case R.id.action_about:
                showAbout();
                return true;
            case R.id.action_parts:
                mDrawerLayout.openDrawer(mDrawerList);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // If the nav drawer is open, hide action items related to the content view
        boolean isDrawerOpen = mDrawerLayout.isDrawerOpen(mDrawerList);
        // Do the same in case the drawer is locked.
        boolean isDrawerLocked = mDrawerLayout.getDrawerLockMode(mDrawerList) != DrawerLayout.LOCK_MODE_UNLOCKED;
        menu.findItem(R.id.action_parts).setVisible(!isDrawerOpen && !isDrawerLocked);
        return super.onPrepareOptionsMenu(menu);
    }

    public DisplayMetrics getDisplayMetrics() {
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        return metrics;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.w(LOGTAG, "onCreate..");
        mAppContext = this;
        super.onCreate(savedInstanceState);

        mMainHandler = new Handler();

        LayoutInflater.from(this).setFactory(ViewFactory.getInstance());

        if (getIntent().getData() != null) {
            mInputFile = getIntent().getData().getEncodedPath();
        } else {
            mInputFile = DEFAULT_DOC_PATH;
        }

        setContentView(R.layout.activity_main);

        getActionBar().setDisplayHomeAsUpEnabled(false);
        getActionBar().setHomeButtonEnabled(false);

        mGeckoLayout = (RelativeLayout) findViewById(R.id.gecko_layout);
        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);

        if (mDocumentPartViewListAdapter == null) {
            mDrawerList = (ListView) findViewById(R.id.left_drawer);

            mDocumentPartViewListAdapter = new DocumentPartViewListAdapter(this, R.layout.document_part_list_layout, mDocumentPartView);
            mDrawerList.setAdapter(mDocumentPartViewListAdapter);
            mDrawerList.setOnItemClickListener(new DocumentPartClickListener());
        }

        if (sLOKitThread == null) {
            sLOKitThread = new LOKitThread();
            sLOKitThread.start();
        } else {
            sLOKitThread.clearQueue();
        }

        mLayerClient = new GeckoLayerClient(this);
        mLayerClient.setZoomConstraints(new ZoomConstraints(true));
        LayerView layerView = (LayerView) findViewById(R.id.layer_view);
        mLayerClient.setView(layerView);
        mLayerClient.notifyReady();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(LOGTAG, "onResume..");
    }

    @Override
    protected void onPause() {
        Log.i(LOGTAG, "onPause..");
        super.onPause();
    }

    @Override
    protected void onStart() {
        Log.i(LOGTAG, "onStart..");
        super.onStart();
        LOKitShell.sendEvent(LOEventFactory.load(mInputFile));
    }

    @Override
    protected void onStop() {
        Log.i(LOGTAG, "onStop..");
        LOKitShell.sendEvent(LOEventFactory.close());
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.i(LOGTAG, "onDestroy..");
        mLayerClient.destroy();
        super.onDestroy();
    }

    public LOKitThread getLOKitThread() {
        return sLOKitThread;
    }

    public List<DocumentPartView> getDocumentPartView() {
        return mDocumentPartView;
    }

    public void disableNavigationDrawer() {
        // Only the original thread that created mDrawerLayout should touch its views.
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED, mDrawerList);
            }
        });
    }

    public DocumentPartViewListAdapter getDocumentPartViewListAdapter() {
        return mDocumentPartViewListAdapter;
    }

    /**
     * Show software keyboard.
     * Force the request on main thread.
     */
    public void showSoftKeyboard() {
        Log.i(LOGTAG, "SoftKeyboard show request..");

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                LayerView layerView = (LayerView) findViewById(R.id.layer_view);

                if (layerView.requestFocus()) {
                    InputMethodManager inputMethodManager = (InputMethodManager) getApplicationContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                    inputMethodManager.showSoftInput(layerView, InputMethodManager.SHOW_FORCED);
                }
            }
        });
    }

    public void showProgressSpinner() {
        findViewById(R.id.loadingPanel).setVisibility(View.VISIBLE);
    }

    public void hideProgressSpinner() {
        findViewById(R.id.loadingPanel).setVisibility(View.GONE);
    }

    public void showAlertDialog(String s) {

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(LibreOfficeMainActivity.this);

        alertDialogBuilder.setTitle("Error");
        alertDialogBuilder.setMessage(s);
        alertDialogBuilder.setNeutralButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                finish();
            }
        });

        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private class DocumentPartClickListener implements android.widget.AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            DocumentPartView partView = mDocumentPartViewListAdapter.getItem(position);
            LOKitShell.sendEvent(LOEventFactory.changePart(partView.partIndex));
            mDrawerLayout.closeDrawer(mDrawerList);
        }
    }


    /**
     * Listen to key presses and send event to LOK.
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        LOKitShell.sendKeyPressEvent(event);
        return super.onKeyDown(keyCode, event);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
