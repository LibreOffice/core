package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

import org.libreoffice.overlay.DocumentOverlay;
import org.libreoffice.storage.DocumentProviderFactory;
import org.libreoffice.storage.IFile;
import org.mozilla.gecko.ZoomConstraints;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

/**
 * Main activity of the LibreOffice App. It is started in the UI thread.
 */
public class LibreOfficeMainActivity extends AppCompatActivity {

    private static final String LOGTAG = "LibreOfficeMainActivity";
    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";
    private static final String ENABLE_EXPERIMENTAL_PREFS_KEY = "ENABLE_EXPERIMENTAL";

    public static LibreOfficeMainActivity mAppContext;

    private static GeckoLayerClient mLayerClient;
    private static LOKitThread sLOKitThread;

    private static boolean mEnableEditing;

    int providerId;
    URI documentUri;

    public Handler mMainHandler;

    private DrawerLayout mDrawerLayout;
    private ListView mDrawerList;
    private List<DocumentPartView> mDocumentPartView = new ArrayList<DocumentPartView>();
    private DocumentPartViewListAdapter mDocumentPartViewListAdapter;
    private File mInputFile;
    private DocumentOverlay mDocumentOverlay;
    private File mTempFile = null;
    private LOAbout mAbout;
    private ToolbarController mToolbarController;

    public LibreOfficeMainActivity() {
        mAbout = new LOAbout(this, false);
    }

    public static GeckoLayerClient getLayerClient() {
        return mLayerClient;
    }

    public static boolean isExperimentalMode() {
        return mEnableEditing;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        mToolbarController.setOptionMenu(menu);

        if (mTempFile != null) {
            mToolbarController.disableMenuItem(R.id.action_save, true);
            Toast.makeText(this, getString(R.string.temp_file_saving_disabled), Toast.LENGTH_LONG).show();
        }
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        switch (id) {
            case R.id.action_bold:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Bold"));
                return true;
            case R.id.action_italic:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Italic"));
                return true;
            case R.id.action_underline:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Underline"));
                return true;
            case R.id.action_strikeout:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:StrikeOut"));
                return true;
            case R.id.action_keyboard:
                showSoftKeyboard();
                break;
            case R.id.action_about:
                mAbout.showAbout();
                return true;
            case R.id.action_save:
                saveDocument();
                return true;
            case R.id.action_parts:
                mDrawerLayout.openDrawer(mDrawerList);
                return true;
            case R.id.action_settings:
                startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
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
        menu.setGroupVisible(R.id.group_edit_actions, mEnableEditing);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.w(LOGTAG, "onCreate..");
        mAppContext = this;
        super.onCreate(savedInstanceState);

        mEnableEditing = PreferenceManager.getDefaultSharedPreferences(getApplicationContext())
                                          .getBoolean(ENABLE_EXPERIMENTAL_PREFS_KEY, false);

        mMainHandler = new Handler();

        setContentView(R.layout.activity_main);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        mToolbarController = new ToolbarController(this, getSupportActionBar(), toolbar);

        if (getIntent().getData() != null) {
            if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
                if (copyFileToTemp() && mTempFile != null) {
                    mInputFile = mTempFile;
                    Log.d(LOGTAG, "SCHEME_CONTENT: getPath(): " + getIntent().getData().getPath());
                } else {
                    // TODO: can't open the file
                }
            } else if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_FILE)) {
                mInputFile = new File(getIntent().getData().getPath());
                Log.d(LOGTAG, "SCHEME_FILE: getPath(): " + getIntent().getData().getPath());

                // Gather data to rebuild IFile object later
                providerId = getIntent().getIntExtra(
                        "org.libreoffice.document_provider_id", 0);
                documentUri = (URI) getIntent().getSerializableExtra(
                        "org.libreoffice.document_uri");
            }
        } else {
            mInputFile = new File(DEFAULT_DOC_PATH);
        }

        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LOKitShell.sendNavigationClickEvent();
            }
        });

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
        layerView.setInputConnectionHandler(new LOKitInputConnectionHandler());
        mLayerClient.notifyReady();

        // create TextCursorLayer
        mDocumentOverlay = new DocumentOverlay(mAppContext, layerView);
    }

    private boolean copyFileToTemp() {
        ContentResolver contentResolver = getContentResolver();
        InputStream inputStream = null;
        try {
            inputStream = contentResolver.openInputStream(getIntent().getData());

            // CSV files need a .csv suffix to be opened in Calc.
            String suffix = null;
            String intentType = getIntent().getType();
            // K-9 mail uses the first, GMail uses the second variant.
            if ("text/comma-separated-values".equals(intentType) || "text/csv".equals(intentType))
                suffix = ".csv";
            mTempFile = File.createTempFile("LibreOffice", suffix, this.getCacheDir());

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

    /**
     * Save the document and invoke save on document provider to upload the file
     * to the cloud if necessary.
     */
    private void saveDocument() {
        final long lastModified = mInputFile.lastModified();
        final Activity activity = LibreOfficeMainActivity.this;
        Toast.makeText(this, R.string.message_saving, Toast.LENGTH_SHORT).show();
        // local save
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Save"));

        final AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                try {
                    // rebuild the IFile object from the data passed in the Intent
                    IFile mStorageFile = DocumentProviderFactory.getInstance()
                            .getProvider(providerId).createFromUri(documentUri);
                    // call document provider save operation
                    mStorageFile.saveDocument(mInputFile);
                }
                catch (final RuntimeException e) {
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(activity, e.getMessage(),
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    Log.e(LOGTAG, e.getMessage(), e.getCause());
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void param) {
                Toast.makeText(activity, R.string.message_saved,
                        Toast.LENGTH_SHORT).show();
            }
        };
        // Delay the call to document provider save operation and check the
        // modification time periodically to ensure the local file has been saved.
        // TODO: ideally the save operation should have a callback
        Runnable runTask = new Runnable() {
            private int timesRun = 0;

            @Override
            public void run() {
                if (lastModified < mInputFile.lastModified()) {
                    // we are sure local save is complete, push changes to cloud
                    task.execute();
                }
                else {
                    timesRun++;
                    if(timesRun < 4) {
                        new Handler().postDelayed(this, 5000);
                    }
                    else {
                        // 20 seconds later, the local file has not changed,
                        // maybe there were no changes at all
                        Toast.makeText(activity,
                                R.string.message_save_incomplete,
                                Toast.LENGTH_LONG).show();
                    }
                }
            }
        };
        new Handler().postDelayed(runTask, 5000);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(LOGTAG, "onResume..");
        // check for config change
        boolean bEnableExperimental = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(ENABLE_EXPERIMENTAL_PREFS_KEY, false);
        if (bEnableExperimental != mEnableEditing) {
            mEnableEditing = bEnableExperimental;
            invalidateOptionsMenu();
        }
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
        LOKitShell.sendLoadEvent(mInputFile.getPath());
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

    public DocumentOverlay getDocumentOverlay() {
        return mDocumentOverlay;
    }

    public ToolbarController getToolbarController() {
        return mToolbarController;
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
