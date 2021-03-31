package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.RectF;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.support.design.widget.BottomSheetBehavior;
import android.support.design.widget.Snackbar;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.Toast;

import org.libreoffice.overlay.CalcHeadersController;
import org.libreoffice.overlay.DocumentOverlay;
import org.libreoffice.storage.DocumentProviderFactory;
import org.libreoffice.storage.IFile;
import org.libreoffice.ui.LibreOfficeUIActivity;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URI;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.ReadableByteChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Main activity of the LibreOffice App. It is started in the UI thread.
 */
public class LibreOfficeMainActivity extends AppCompatActivity implements SettingsListenerModel.OnSettingsPreferenceChangedListener {

    private static final String LOGTAG = "LibreOfficeMainActivity";
    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";
    private static final String ENABLE_EXPERIMENTAL_PREFS_KEY = "ENABLE_EXPERIMENTAL";
    private static final String ASSETS_EXTRACTED_PREFS_KEY = "ASSETS_EXTRACTED";
    private static final String ENABLE_DEVELOPER_PREFS_KEY = "ENABLE_DEVELOPER";

    //TODO "public static" is a temporary workaround
    public static LOKitThread loKitThread;

    private GeckoLayerClient mLayerClient;

    private static boolean mIsExperimentalMode;
    private static boolean mIsDeveloperMode;
    private static boolean mbISReadOnlyMode;

    private int providerId;
    private URI documentUri;

    private DrawerLayout mDrawerLayout;
    Toolbar toolbarTop;

    private ListView mDrawerList;
    private List<DocumentPartView> mDocumentPartView = new ArrayList<DocumentPartView>();
    private DocumentPartViewListAdapter mDocumentPartViewListAdapter;
    private int partIndex=-1;
    private File mInputFile;
    private DocumentOverlay mDocumentOverlay;
    private File mTempFile = null;
    private File mTempSlideShowFile = null;
    public boolean firstStart = true;

    BottomSheetBehavior bottomToolbarSheetBehavior;
    BottomSheetBehavior toolbarColorPickerBottomSheetBehavior;
    BottomSheetBehavior toolbarBackColorPickerBottomSheetBehavior;
    private FormattingController mFormattingController;
    private ToolbarController mToolbarController;
    private FontController mFontController;
    private SearchController mSearchController;
    private UNOCommandsController mUNOCommandsController;
    private CalcHeadersController mCalcHeadersController;
    private boolean mIsSpreadsheet;
    private LOKitTileProvider mTileProvider;
    private String mPassword;
    private boolean mPasswordProtected;
    public boolean pendingInsertGraphic; // boolean indicating a pending insert graphic action, used in LOKitTileProvider.postLoad()

    public GeckoLayerClient getLayerClient() {
        return mLayerClient;
    }

    public static boolean isExperimentalMode() {
        return mIsExperimentalMode;
    }

    public static boolean isDeveloperMode() {
        return mIsDeveloperMode;
    }

    private boolean isKeyboardOpen = false;
    private boolean isFormattingToolbarOpen = false;
    private boolean isSearchToolbarOpen = false;
    private static boolean isDocumentChanged = false;
    private boolean isUNOCommandsToolbarOpen = false;
    private boolean isNewDocument = false;
    private long lastModified = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.w(LOGTAG, "onCreate..");
        super.onCreate(savedInstanceState);

        SettingsListenerModel.getInstance().setListener(this);
        updatePreferences();

        setContentView(R.layout.activity_main);

        toolbarTop = findViewById(R.id.toolbar);
        hideBottomToolbar();

        mToolbarController = new ToolbarController(this, toolbarTop);
        mFormattingController = new FormattingController(this);
        toolbarTop.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LOKitShell.sendNavigationClickEvent();
            }
        });

        mFontController = new FontController(this);
        mSearchController = new SearchController(this);
        mUNOCommandsController = new UNOCommandsController(this);

        loKitThread = new LOKitThread(this);
        loKitThread.start();

        mLayerClient = new GeckoLayerClient(this);
        LayerView layerView = findViewById(R.id.layer_view);
        mLayerClient.setView(layerView);
        layerView.setInputConnectionHandler(new LOKitInputConnectionHandler());
        mLayerClient.notifyReady();

        layerView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View view, int i, KeyEvent keyEvent) {
                if(!isReadOnlyMode() && keyEvent.getKeyCode() != KeyEvent.KEYCODE_BACK){
                    setDocumentChanged(true);
                }
                return false;
            }
        });

        // create TextCursorLayer
        mDocumentOverlay = new DocumentOverlay(this, layerView);

        mbISReadOnlyMode = !isExperimentalMode();

        if (getIntent().getData() != null) {
            if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
                final boolean isReadOnlyDoc;
                if (getIntent().getStringExtra(LibreOfficeUIActivity.NEW_DOC_TYPE_KEY) != null) {
                    // New document type string is not null, meaning we want to open a new document
                    String newDocumentType = getIntent().getStringExtra(LibreOfficeUIActivity.NEW_DOC_TYPE_KEY);
                    // create a temporary local file, will be copied to the actual URI when saving
                    loadNewDocument(newDocumentType);
                    mInputFile = mTempFile;
                    isReadOnlyDoc = false;
                } else if (copyFileToTemp() && mTempFile != null) {
                    mInputFile = mTempFile;
                    isReadOnlyDoc = (getIntent().getFlags() & Intent.FLAG_GRANT_WRITE_URI_PERMISSION) == 0;
                } else {
                    // TODO: can't open the file
                    Log.e(LOGTAG, "couldn't create temporary file from " + getIntent().getData());
                    return;
                }

                mbISReadOnlyMode = !isExperimentalMode()  || isReadOnlyDoc;
                Log.d(LOGTAG, "SCHEME_CONTENT: getPath(): " + getIntent().getData().getPath());

                String displayName = extractDisplayNameFromIntent();
                if (displayName.isEmpty()) {
                    // fall back to using temp file name
                    displayName = mInputFile.getName();
                }
                toolbarTop.setTitle(displayName);

            } else if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_FILE)) {
                mInputFile = new File(getIntent().getData().getPath());
                Log.d(LOGTAG, "SCHEME_FILE: getPath(): " + getIntent().getData().getPath());
                toolbarTop.setTitle(mInputFile.getName());
                // Gather data to rebuild IFile object later
                providerId = getIntent().getIntExtra(
                        "org.libreoffice.document_provider_id", 0);
                documentUri = (URI) getIntent().getSerializableExtra(
                        "org.libreoffice.document_uri");
            }
        } else {
            mInputFile = new File(DEFAULT_DOC_PATH);
        }

        mDrawerLayout = findViewById(R.id.drawer_layout);

        if (mDocumentPartViewListAdapter == null) {
            mDrawerList = findViewById(R.id.left_drawer);

            mDocumentPartViewListAdapter = new DocumentPartViewListAdapter(this, R.layout.document_part_list_layout, mDocumentPartView);
            mDrawerList.setAdapter(mDocumentPartViewListAdapter);
            mDrawerList.setOnItemClickListener(new DocumentPartClickListener());
        }

        lastModified = mInputFile.lastModified();

        mToolbarController.setupToolbars();

        TabHost host = findViewById(R.id.toolbarTabHost);
        host.setup();

        TabHost.TabSpec spec = host.newTabSpec(getString(R.string.tabhost_character));
        spec.setContent(R.id.tab_character);
        spec.setIndicator(getString(R.string.tabhost_character));
        host.addTab(spec);

        spec = host.newTabSpec(getString(R.string.tabhost_paragraph));
        spec.setContent(R.id.tab_paragraph);
        spec.setIndicator(getString(R.string.tabhost_paragraph));
        host.addTab(spec);

        spec = host.newTabSpec(getString(R.string.tabhost_insert));
        spec.setContent(R.id.tab_insert);
        spec.setIndicator(getString(R.string.tabhost_insert));
        host.addTab(spec);

        spec = host.newTabSpec(getString(R.string.tabhost_style));
        spec.setContent(R.id.tab_style);
        spec.setIndicator(getString(R.string.tabhost_style));
        host.addTab(spec);

        LinearLayout bottomToolbarLayout = findViewById(R.id.toolbar_bottom);
        LinearLayout toolbarColorPickerLayout = findViewById(R.id.toolbar_color_picker);
        LinearLayout toolbarBackColorPickerLayout = findViewById(R.id.toolbar_back_color_picker);
        bottomToolbarSheetBehavior = BottomSheetBehavior.from(bottomToolbarLayout);
        toolbarColorPickerBottomSheetBehavior = BottomSheetBehavior.from(toolbarColorPickerLayout);
        toolbarBackColorPickerBottomSheetBehavior = BottomSheetBehavior.from(toolbarBackColorPickerLayout);
        bottomToolbarSheetBehavior.setHideable(true);
        toolbarColorPickerBottomSheetBehavior.setHideable(true);
        toolbarBackColorPickerBottomSheetBehavior.setHideable(true);
    }

    private void updatePreferences() {
        SharedPreferences sPrefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        mIsExperimentalMode = BuildConfig.ALLOW_EDITING
                && sPrefs.getBoolean(ENABLE_EXPERIMENTAL_PREFS_KEY, false);
        mIsDeveloperMode = mIsExperimentalMode
                && sPrefs.getBoolean(ENABLE_DEVELOPER_PREFS_KEY, false);
        if (sPrefs.getInt(ASSETS_EXTRACTED_PREFS_KEY, 0) != BuildConfig.VERSION_CODE) {
            if(copyFromAssets(getAssets(), "unpack", getApplicationInfo().dataDir)) {
                sPrefs.edit().putInt(ASSETS_EXTRACTED_PREFS_KEY, BuildConfig.VERSION_CODE).apply();
            }
        }
    }

    // Loads a new Document and saves it to a temporary file
    private void loadNewDocument(String newDocumentType) {
        String tempFileName = "LibreOffice_" + UUID.randomUUID().toString();
        mTempFile = new File(this.getCacheDir(), tempFileName);
        LOKitShell.sendNewDocumentLoadEvent(mTempFile.getPath(), newDocumentType);
        isNewDocument = true;
    }

    public RectF getCurrentCursorPosition() {
        return mDocumentOverlay.getCurrentCursorPosition();
    }

    private boolean copyFileToTemp() {
        final ContentResolver contentResolver = getContentResolver();
        // CSV files need a .csv suffix to be opened in Calc.
        String suffix = null;
        String intentType = getIntent().getType();
        // K-9 mail uses the first, GMail uses the second variant.
        if ("text/comma-separated-values".equals(intentType) || "text/csv".equals(intentType))
            suffix = ".csv";

        try {
            mTempFile = File.createTempFile("LibreOffice", suffix, this.getCacheDir());
            final FileChannel outputChannel = new FileOutputStream(mTempFile).getChannel();
            try {
                // need to run copy operation in a separate thread, since network access is not
                // allowed from main thread, but that may happen here when underlying
                // DocumentsProvider (like the NextCloud one) does that
                class CopyThread extends Thread {
                    /** Whether copy operation was successful. */
                    private boolean result = false;

                    @Override
                    public void run() {
                        result = false;
                        try {
                            final AssetFileDescriptor assetFD = contentResolver.openAssetFileDescriptor(getIntent().getData(), "r");
                            if (assetFD == null) {
                                Log.e(LOGTAG, "couldn't create assetfiledescriptor from " + getIntent().getDataString());
                                return;
                            }
                            FileChannel inputChannel = assetFD.createInputStream().getChannel();
                            long bytesTransferred = 0;
                            // might not  copy all at once, so make sure everything gets copied...
                            while (bytesTransferred < inputChannel.size()) {
                                bytesTransferred += outputChannel.transferFrom(inputChannel, bytesTransferred, inputChannel.size());
                            }
                            Log.e(LOGTAG, "Success copying " + bytesTransferred + " bytes");
                            inputChannel.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                            return;
                        }
                        result = true;
                    }
                };
                CopyThread copyThread = new CopyThread();
                copyThread.start();
                try {
                    // wait for copy operation to finish
                    // NOTE: might be useful to add some indicator in UI for long copy operations involving network...
                    copyThread.join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                return copyThread.result;
            } finally {
                outputChannel.close();
            }
        } catch (FileNotFoundException e) {
            return false;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * Save the document and invoke save on document provider to upload the file
     * to the cloud if necessary.
     */
    public void saveDocument() {
        if (!mInputFile.exists()) {
            // Needed for handling null in case new document is not created.
            mInputFile = new File(DEFAULT_DOC_PATH);
            lastModified = mInputFile.lastModified();
        }
        Toast.makeText(this, R.string.message_saving, Toast.LENGTH_SHORT).show();
        // local save
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND_NOTIFY, ".uno:Save", true));
    }

    public void saveFileToOriginalSource() {
        if (documentUri != null) {
            // case where file was opened using IDocumentProvider from within LO app
            saveFilesToCloud();
        } else {
            // case where file URI was passed via Intent
            if (isReadOnlyMode() || mInputFile == null || getIntent().getData() == null || !getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT))
                return;

            Uri uri = getIntent().getData();
            FileInputStream inputStream = null;
            OutputStream outputStream = null;

            try {
                inputStream = new FileInputStream(mInputFile);
                // OutputStream for the actual (original) location
                outputStream = getContentResolver().openOutputStream(uri);

                byte[] buffer = new byte[4096];
                int readBytes = inputStream.read(buffer);
                while (readBytes != -1) {
                    outputStream.write(buffer, 0, readBytes);
                    readBytes = inputStream.read(buffer);
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(LibreOfficeMainActivity.this, R.string.message_saved,
                            Toast.LENGTH_SHORT).show();
                    }
                });
                setDocumentChanged(false);
            } catch (Exception e) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(LibreOfficeMainActivity.this, R.string.message_saving_failed,
                            Toast.LENGTH_SHORT).show();
                    }
                });
                e.printStackTrace();
            } finally {
                try {
                    if (inputStream != null)
                        inputStream.close();
                    if (outputStream != null)
                        outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public void saveFilesToCloud(){
        final Activity activity = LibreOfficeMainActivity.this;
        final AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                try {
                    // rebuild the IFile object from the data passed in the Intent
                    IFile mStorageFile = DocumentProviderFactory.getInstance()
                            .getProvider(providerId).createFromUri(LibreOfficeMainActivity.this, documentUri);
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
                setDocumentChanged(false);
            }
        };

        if (lastModified < mInputFile.lastModified()) {
            task.execute();
            lastModified = mInputFile.lastModified();
        } else {
            Toast.makeText(activity, R.string.message_save_incomplete, Toast.LENGTH_LONG).show();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(LOGTAG, "onResume..");
        // check for config change
        updatePreferences();
        if (mToolbarController.getEditModeStatus() && isExperimentalMode()) {
            mToolbarController.switchToEditMode();
        } else {
            mToolbarController.switchToViewMode();
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
        if (!isNewDocument){
            if (partIndex == -1)
                LOKitShell.sendLoadEvent(mInputFile.getPath());
            else
                LOKitShell.sendResumeEvent(mInputFile.getPath(), partIndex);
        }
    }

    @Override
    protected void onStop() {
        Log.i(LOGTAG, "onStop..");
        //save document to cache
        if (mTileProvider != null)
            mTileProvider.cacheDocument();
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
                // noinspection ResultOfMethodCallIgnored
                mTempFile.delete();
            }
            if (mTempSlideShowFile != null && mTempSlideShowFile.exists()) {
                // noinspection ResultOfMethodCallIgnored
                mTempSlideShowFile.delete();
            }
        }
    }
    @Override
    public void onBackPressed() {
        if (!isDocumentChanged) {
            super.onBackPressed();
            return;
        }


        DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which){
                    case DialogInterface.BUTTON_POSITIVE:
                        mTileProvider.saveDocument();
                        isDocumentChanged=false;
                        onBackPressed();
                        break;
                    case DialogInterface.BUTTON_NEGATIVE:
                        //CANCEL
                        break;
                    case DialogInterface.BUTTON_NEUTRAL:
                        //NO
                        isDocumentChanged=false;
                        onBackPressed();
                        break;
                }
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(R.string.save_alert_dialog_title)
                .setPositiveButton(R.string.save_document, dialogClickListener)
                .setNegativeButton(R.string.action_cancel, dialogClickListener)
                .setNeutralButton(R.string.no_save_document, dialogClickListener)
                .show();

    }

    /**
     * Tries to retrieve display name for data in Intent,
     * which should be the file name.
     */
    private String extractDisplayNameFromIntent() {
        String displayName = "";
        // try to retrieve original file name
        Cursor cursor = null;
        try {
            String[] columns = {OpenableColumns.DISPLAY_NAME};
            cursor = getContentResolver().query(getIntent().getData(), columns, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                displayName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return displayName;
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
                if(!isKeyboardOpen) showSoftKeyboardDirect();
                else hideSoftKeyboardDirect();
            }
        });

    }

    private void showSoftKeyboardDirect() {
        LayerView layerView = findViewById(R.id.layer_view);

        if (layerView.requestFocus()) {
            InputMethodManager inputMethodManager = (InputMethodManager) getApplicationContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            inputMethodManager.showSoftInput(layerView, InputMethodManager.SHOW_FORCED);
        }
        isKeyboardOpen=true;
        isSearchToolbarOpen=false;
        isFormattingToolbarOpen=false;
        isUNOCommandsToolbarOpen=false;
        hideBottomToolbar();
    }

    public void showSoftKeyboardOrFormattingToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if (findViewById(R.id.toolbar_bottom).getVisibility() != View.VISIBLE
                        && findViewById(R.id.toolbar_color_picker).getVisibility() != View.VISIBLE) {
                    showSoftKeyboardDirect();
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
        if (getCurrentFocus() != null) {
            InputMethodManager inputMethodManager = (InputMethodManager) getApplicationContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            inputMethodManager.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(), 0);
            isKeyboardOpen=false;
        }
    }

    public void showBottomToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                bottomToolbarSheetBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
            }
        });
    }

    public void hideBottomToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                bottomToolbarSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                toolbarColorPickerBottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                toolbarBackColorPickerBottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                findViewById(R.id.search_toolbar).setVisibility(View.GONE);
                findViewById(R.id.UNO_commands_toolbar).setVisibility(View.GONE);
                isFormattingToolbarOpen=false;
                isSearchToolbarOpen=false;
                isUNOCommandsToolbarOpen=false;
            }
        });
    }

    public void showFormattingToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if (isFormattingToolbarOpen) {
                    hideFormattingToolbar();
                } else {
                    showBottomToolbar();
                    findViewById(R.id.search_toolbar).setVisibility(View.GONE);
                    findViewById(R.id.formatting_toolbar).setVisibility(View.VISIBLE);
                    findViewById(R.id.search_toolbar).setVisibility(View.GONE);
                    findViewById(R.id.UNO_commands_toolbar).setVisibility(View.GONE);
                    hideSoftKeyboardDirect();
                    isSearchToolbarOpen=false;
                    isFormattingToolbarOpen=true;
                    isUNOCommandsToolbarOpen=false;
                }

            }
        });
    }

    public void hideFormattingToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                hideBottomToolbar();
            }
        });
    }

    public void showSearchToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if (isSearchToolbarOpen) {
                    hideSearchToolbar();
                } else {
                    showBottomToolbar();
                    findViewById(R.id.formatting_toolbar).setVisibility(View.GONE);
                    toolbarColorPickerBottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                    toolbarBackColorPickerBottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                    findViewById(R.id.search_toolbar).setVisibility(View.VISIBLE);
                    findViewById(R.id.UNO_commands_toolbar).setVisibility(View.GONE);
                    hideSoftKeyboardDirect();
                    isFormattingToolbarOpen=false;
                    isSearchToolbarOpen=true;
                    isUNOCommandsToolbarOpen=false;
                }
            }
        });
    }

    public void hideSearchToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                hideBottomToolbar();
            }
        });
    }

    public void showUNOCommandsToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                if(isUNOCommandsToolbarOpen){
                    hideUNOCommandsToolbar();
                }else{
                    showBottomToolbar();
                    findViewById(R.id.formatting_toolbar).setVisibility(View.GONE);
                    findViewById(R.id.search_toolbar).setVisibility(View.GONE);
                    findViewById(R.id.UNO_commands_toolbar).setVisibility(View.VISIBLE);
                    hideSoftKeyboardDirect();
                    isFormattingToolbarOpen=false;
                    isSearchToolbarOpen=false;
                    isUNOCommandsToolbarOpen=true;
                }
            }
        });
    }

    public void hideUNOCommandsToolbar() {
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                hideBottomToolbar();
            }
        });
    }

    public void showProgressSpinner() {
        findViewById(R.id.loadingPanel).setVisibility(View.VISIBLE);
    }

    public void hideProgressSpinner() {
        findViewById(R.id.loadingPanel).setVisibility(View.GONE);
    }

    public void showAlertDialog(String message) {

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(LibreOfficeMainActivity.this);

        alertDialogBuilder.setTitle(R.string.error);
        alertDialogBuilder.setMessage(message);
        alertDialogBuilder.setNeutralButton(R.string.alert_ok, new DialogInterface.OnClickListener() {
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

    public CalcHeadersController getCalcHeadersController() {
        return mCalcHeadersController;
    }

    public ToolbarController getToolbarController() {
        return mToolbarController;
    }

    public FontController getFontController() {
        return mFontController;
    }

    public FormattingController getFormattingController() {
        return mFormattingController;
    }

    public void openDrawer() {
        mDrawerLayout.openDrawer(mDrawerList);
        hideBottomToolbar();
    }

    public void showAbout() {
        AboutDialogFragment aboutDialogFragment = new AboutDialogFragment();
        aboutDialogFragment.show(getSupportFragmentManager(), "AboutDialogFragment");
    }

    public void addPart(){
        mTileProvider.addPart();
        mDocumentPartViewListAdapter.notifyDataSetChanged();
        setDocumentChanged(true);
    }

    public void renamePart(){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.enter_part_name);
        final EditText input = new EditText(this);
        input.setInputType(InputType.TYPE_CLASS_TEXT);
        builder.setView(input);

        builder.setPositiveButton(R.string.alert_ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                mTileProvider.renamePart( input.getText().toString());
            }
        });
        builder.setNegativeButton(R.string.alert_cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });

        builder.show();
    }

    public void deletePart() {
        mTileProvider.removePart();
    }

    public void showSettings() {
        startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
    }

    public boolean isDrawerEnabled() {
        boolean isDrawerOpen = mDrawerLayout.isDrawerOpen(mDrawerList);
        boolean isDrawerLocked = mDrawerLayout.getDrawerLockMode(mDrawerList) != DrawerLayout.LOCK_MODE_UNLOCKED;
        return !isDrawerOpen && !isDrawerLocked;
    }

    @Override
    public void settingsPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if (key.matches(ENABLE_EXPERIMENTAL_PREFS_KEY)) {
            Log.d(LOGTAG, "Editing Preference Changed");
            mIsExperimentalMode = sharedPreferences.getBoolean(ENABLE_EXPERIMENTAL_PREFS_KEY, false);
        }
    }

    public void promptForPassword() {
        PasswordDialogFragment passwordDialogFragment = new PasswordDialogFragment();
        passwordDialogFragment.setLOMainActivity(this);
        passwordDialogFragment.show(getSupportFragmentManager(), "PasswordDialogFragment");
    }

    // this function can only be called in InvalidationHandler.java
    public void setPassword() {
        mTileProvider.setDocumentPassword("file://"+mInputFile.getPath(), mPassword);
    }

    // setTileProvider is meant to let main activity have a handle of LOKit when dealing with password
    public void setTileProvider(LOKitTileProvider loKitTileProvider) {
        mTileProvider = loKitTileProvider;
    }

    public LOKitTileProvider getTileProvider() {
        return mTileProvider;
    }

    public void savePassword(String pwd) {
        mPassword = pwd;
        synchronized (mTileProvider.getMessageCallback()) {
            mTileProvider.getMessageCallback().notifyAll();
        }
    }

    public void setPasswordProtected(boolean b) {
        mPasswordProtected = b;
    }

    public boolean isPasswordProtected() {
        return mPasswordProtected;
    }

    public void initializeCalcHeaders() {
        mCalcHeadersController = new CalcHeadersController(this, mLayerClient.getView());
        mCalcHeadersController.setupHeaderPopupView();
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                findViewById(R.id.calc_header_top_left).setVisibility(View.VISIBLE);
                findViewById(R.id.calc_header_row).setVisibility(View.VISIBLE);
                findViewById(R.id.calc_header_column).setVisibility(View.VISIBLE);
                findViewById(R.id.calc_address).setVisibility(View.VISIBLE);
                findViewById(R.id.calc_formula).setVisibility(View.VISIBLE);
            }
        });
    }

    public void setIsSpreadsheet(boolean b) {
        mIsSpreadsheet = b;
    }

    public boolean isSpreadsheet() {
        return mIsSpreadsheet;
    }

    public static boolean isReadOnlyMode() {
        return mbISReadOnlyMode;
    }

    public static void setDocumentChanged (boolean changed) {
        isDocumentChanged = changed;
    }

    private class DocumentPartClickListener implements android.widget.AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            DocumentPartView partView = mDocumentPartViewListAdapter.getItem(position);
            partIndex = partView.partIndex;
            LOKitShell.sendChangePartEvent(partView.partIndex);
            mDrawerLayout.closeDrawer(mDrawerList);
        }
    }

    private static boolean copyFromAssets(AssetManager assetManager,
                                          String fromAssetPath, String targetDir) {
        try {
            String[] files = assetManager.list(fromAssetPath);

            boolean res = true;
            for (String file : files) {
                String[] dirOrFile = assetManager.list(fromAssetPath + "/" + file);
                if ( dirOrFile.length == 0) {
                    // noinspection ResultOfMethodCallIgnored
                    new File(targetDir).mkdirs();
                    res &= copyAsset(assetManager,
                            fromAssetPath + "/" + file,
                            targetDir + "/" + file);
                } else
                    res &= copyFromAssets(assetManager,
                            fromAssetPath + "/" + file,
                            targetDir + "/" + file);
            }
            return res;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(LOGTAG, "copyFromAssets failed: " + e.getMessage());
            return false;
        }
    }

    private static boolean copyAsset(AssetManager assetManager, String fromAssetPath, String toPath) {
        ReadableByteChannel source = null;
        FileChannel dest = null;
        try {
            try {
                source = Channels.newChannel(assetManager.open(fromAssetPath));
                dest = new FileOutputStream(toPath).getChannel();
                long bytesTransferred = 0;
                // might not copy all at once, so make sure everything gets copied...
                ByteBuffer buffer = ByteBuffer.allocate(4096);
                while (source.read(buffer) > 0) {
                    buffer.flip();
                    bytesTransferred += dest.write(buffer);
                    buffer.clear();
                }
                Log.v(LOGTAG, "Success copying " + fromAssetPath + " to " + toPath + " bytes: " + bytesTransferred);
                return true;
            } finally {
                if (dest != null) dest.close();
                if (source != null) source.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(LOGTAG, "file " + fromAssetPath + " not found! " + e.getMessage());
            return false;
        } catch (IOException e) {
            Log.e(LOGTAG, "failed to copy file " + fromAssetPath + " from assets to " + toPath + " - " + e.getMessage());
            return false;
        }
    }

    public void showCustomStatusMessage(String message){
        Snackbar.make(mDrawerLayout, message, Snackbar.LENGTH_LONG).show();
    }

    public void preparePresentation() {
        if (getExternalCacheDir() != null) {
            String tempPath = getExternalCacheDir().getPath() + "/" + mInputFile.getName() + ".svg";
            mTempSlideShowFile = new File(tempPath);
            if (mTempSlideShowFile.exists() && !isDocumentChanged) {
                startPresentation("file://" + tempPath);
            } else {
                LOKitShell.sendSaveCopyAsEvent(tempPath, "svg");
            }
        }
    }

    public void startPresentation(String tempPath) {
        // pre-KitKat android doesn't have chrome-based WebView, which is needed to show svg slideshow
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            Intent intent = new Intent(this, PresentationActivity.class);
            intent.setData(Uri.parse(tempPath));
            startActivity(intent);
        } else {
            // copy the svg file path to clipboard for the user to paste in a browser
            ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData clip = ClipData.newPlainText("temp svg file path", tempPath);
            clipboard.setPrimaryClip(clip);

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage(R.string.alert_copy_svg_slide_show_to_clipboard)
                    .setPositiveButton(R.string.alert_copy_svg_slide_show_to_clipboard_dismiss, null).show();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        mFormattingController.handleActivityResult(requestCode, resultCode, data);
        hideBottomToolbar();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
