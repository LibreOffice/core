package org.libreoffice;

import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.widget.DrawerLayout;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.RelativeLayout;

import org.mozilla.gecko.TextSelection;
import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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
    private TextSelection mTextSelection;
    private TextCursorLayer mTextCursorLayer;
    private File mTempFile = null;

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

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.w(LOGTAG, "onCreate..");
        mAppContext = this;
        super.onCreate(savedInstanceState);

        mMainHandler = new Handler();

        if (getIntent().getData() != null) {
            if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
                if (copyFileToTemp() && mTempFile != null) {
                    mInputFile = mTempFile.getPath();
                } else {
                    // TODO: can't open the file
                }
            } else if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_FILE)) {
                mInputFile = getIntent().getData().getPath();
            }
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

        mTextSelection = new TextSelection(mAppContext);
        mTextCursorLayer = new TextCursorLayer(mAppContext);

        mLayerClient = new GeckoLayerClient(this);
        mLayerClient.setZoomConstraints(new ZoomConstraints(true));
        LayerView layerView = (LayerView) findViewById(R.id.layer_view);
        mLayerClient.setView(layerView);
        layerView.setInputConnectionHandler(new LOKitInputConnectionHandler());
        mLayerClient.notifyReady();
    }

    private boolean copyFileToTemp() {
        ContentResolver contentResolver = getContentResolver();
        InputStream inputStream = null;
        try {
            inputStream = contentResolver.openInputStream(getIntent().getData());
            mTempFile = File.createTempFile("LibreOffice", null, this.getCacheDir());

            OutputStream outputStream = new FileOutputStream(mTempFile);
            byte[] buffer = new byte[4096];
            int len = 0;
            while ((len = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, len);
            }
            inputStream.close();
            outputStream.close();
            return true;
        } catch (FileNotFoundException e) {
        } catch (IOException e) {
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                }
            }
        }
        return false;
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
        LOKitShell.sendLoadEvent(mInputFile);
    }

    @Override
    protected void onStop() {
        Log.i(LOGTAG, "onStop..");
        hideSoftKeyboardDirect();
        LOKitShell.sendCloseEvent();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.i(LOGTAG, "onDestroy..");
        mLayerClient.destroy();
        super.onDestroy();

        if (isFinishing()) { // Not an orientation change
            if (mTempFile != null) {
                mTempFile.delete();
            }
        }
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

    /**
     * Hides software keyboard on UI thread.
     */
    public void hideSoftKeyboard() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                hideSoftKeyboardDirect();
            }
        });
    }

    /**
     * Hides software keyboard.
     */
    private void hideSoftKeyboardDirect() {
        LayerView layerView = (LayerView) findViewById(R.id.layer_view);

        if (getCurrentFocus() != null) {
            InputMethodManager inputMethodManager = (InputMethodManager) getApplicationContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            inputMethodManager.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(), 0);
        }
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

    public TextSelection getTextSelection() {
        return mTextSelection;
    }

    public TextCursorLayer getTextCursorLayer() {
        return mTextCursorLayer;
    }

    private class DocumentPartClickListener implements android.widget.AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            DocumentPartView partView = mDocumentPartViewListAdapter.getItem(position);
            LOKitShell.sendChangePartEvent(partView.partIndex);
            mDrawerLayout.closeDrawer(mDrawerList);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
