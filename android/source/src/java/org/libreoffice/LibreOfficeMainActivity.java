package org.libreoffice;

import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.DocumentsContract;
import com.google.android.material.bottomsheet.BottomSheetBehavior;
import com.google.android.material.snackbar.Snackbar;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.preference.PreferenceManager;
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
import org.libreoffice.ui.FileUtilities;
import org.libreoffice.ui.LibreOfficeUIActivity;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.LayerView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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
public class LibreOfficeMainActivity extends AppCompatActivity implements SharedPreferences.OnSharedPreferenceChangeListener {

    private static final String LOGTAG = "LibreOfficeMainActivity";
    public static final String ENABLE_EXPERIMENTAL_PREFS_KEY = "ENABLE_EXPERIMENTAL";
    private static final String ASSETS_EXTRACTED_PREFS_KEY = "ASSETS_EXTRACTED";
    private static final String ENABLE_DEVELOPER_PREFS_KEY = "ENABLE_DEVELOPER";
    private static final int REQUEST_CODE_SAVEAS = 12345;
    private static final int REQUEST_CODE_EXPORT_TO_PDF = 12346;

    //TODO "public static" is a temporary workaround
    public static LOKitThread loKitThread;

    private GeckoLayerClient mLayerClient;

    private static boolean mIsExperimentalMode;
    private static boolean mIsDeveloperMode;
    private static boolean mbReadOnlyDoc;

    private DrawerLayout mDrawerLayout;
    Toolbar toolbarTop;

    private ListView mDrawerList;
    private final List<DocumentPartView> mDocumentPartView = new ArrayList<DocumentPartView>();
    private DocumentPartViewListAdapter mDocumentPartViewListAdapter;
    private DocumentOverlay mDocumentOverlay;
    /** URI to save the document to. */
    private Uri mDocumentUri;
    /** Temporary local copy of the document. */
    private File mTempFile = null;
    private File mTempSlideShowFile = null;

    BottomSheetBehavior bottomToolbarSheetBehavior;
    BottomSheetBehavior toolbarColorPickerBottomSheetBehavior;
    BottomSheetBehavior toolbarBackColorPickerBottomSheetBehavior;
    private FormattingController mFormattingController;
    private ToolbarController mToolbarController;
    private FontController mFontController;
    private SearchController mSearchController;
    private UNOCommandsController mUNOCommandsController;
    private CalcHeadersController mCalcHeadersController;
    private LOKitTileProvider mTileProvider;
    private String mPassword;
    private boolean mPasswordProtected;
    private boolean mbSkipNextRefresh;

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

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.w(LOGTAG, "onCreate..");
        super.onCreate(savedInstanceState);

        updatePreferences();
        PreferenceManager.getDefaultSharedPreferences(getApplicationContext())
            .registerOnSharedPreferenceChangeListener(this);

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

        mbReadOnlyDoc = false;

        final Uri docUri = getIntent().getData();
        if (docUri != null) {
            if (docUri.getScheme().equals(ContentResolver.SCHEME_CONTENT)
                    || docUri.getScheme().equals(ContentResolver.SCHEME_ANDROID_RESOURCE)) {
                mbReadOnlyDoc  = (getIntent().getFlags() & Intent.FLAG_GRANT_WRITE_URI_PERMISSION) == 0;
                Log.d(LOGTAG, "SCHEME_CONTENT: getPath(): " + docUri.getPath());

                String displayName = FileUtilities.retrieveDisplayNameForDocumentUri(getContentResolver(), docUri);
                toolbarTop.setTitle(displayName);

            } else if (docUri.getScheme().equals(ContentResolver.SCHEME_FILE)) {
                mbReadOnlyDoc = true;
                Log.d(LOGTAG, "SCHEME_FILE: getPath(): " + docUri.getPath());
                toolbarTop.setTitle(docUri.getLastPathSegment());
            }
            // create a temporary local copy to work with
            boolean copyOK = copyFileToTemp(docUri) && mTempFile != null;
            if (!copyOK) {
                // TODO: can't open the file
                Log.e(LOGTAG, "couldn't create temporary file from " + docUri);
                return;
            }

            // if input doc is a template, a new doc is created and a proper URI to save to
            // will only be available after a "Save As"
            if (isTemplate(docUri)) {
                toolbarTop.setTitle(R.string.default_document_name);
            } else {
                mDocumentUri = docUri;
            }

            LOKitShell.sendLoadEvent(mTempFile.getPath());
        } else if (getIntent().getStringExtra(LibreOfficeUIActivity.NEW_DOC_TYPE_KEY) != null) {
            // New document type string is not null, meaning we want to open a new document
            String newDocumentType = getIntent().getStringExtra(LibreOfficeUIActivity.NEW_DOC_TYPE_KEY);
            // create a temporary local file, will be copied to the actual URI when saving
            loadNewDocument(newDocumentType);
            toolbarTop.setTitle(getString(R.string.default_document_name));
        } else {
            Log.e(LOGTAG, "No document specified. This should never happen.");
            return;
        }
        // the loadDocument/loadNewDocument event already triggers a refresh as well,
        // so there's no need to do another refresh in 'onStart'
        mbSkipNextRefresh = true;

        mDrawerLayout = findViewById(R.id.drawer_layout);

        if (mDocumentPartViewListAdapter == null) {
            mDrawerList = findViewById(R.id.left_drawer);

            mDocumentPartViewListAdapter = new DocumentPartViewListAdapter(this, R.layout.document_part_list_layout, mDocumentPartView);
            mDrawerList.setAdapter(mDocumentPartViewListAdapter);
            mDrawerList.setOnItemClickListener(new DocumentPartClickListener());
        }

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
    }

    public RectF getCurrentCursorPosition() {
        return mDocumentOverlay.getCurrentCursorPosition();
    }

    private boolean copyFileToTemp(Uri documentUri) {
        // CSV files need a .csv suffix to be opened in Calc.
        String suffix = null;
        String intentType = getIntent().getType();
        // K-9 mail uses the first, GMail uses the second variant.
        if ("text/comma-separated-values".equals(intentType) || "text/csv".equals(intentType))
            suffix = ".csv";

        try {
            mTempFile = File.createTempFile("LibreOffice", suffix, this.getCacheDir());
            final FileOutputStream outputStream = new FileOutputStream(mTempFile);
            return copyUriToStream(documentUri, outputStream);
        } catch (FileNotFoundException e) {
            return false;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * Save the document.
     */
    public void saveDocument() {
        Toast.makeText(this, R.string.message_saving, Toast.LENGTH_SHORT).show();
        // local save
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND_NOTIFY, ".uno:Save", true));
    }

    /**
     * Open file chooser and save the document to the URI
     * selected there.
     */
    public void saveDocumentAs() {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        String mimeType = getODFMimeTypeForDocument();
        intent.setType(mimeType);
        if (Build.VERSION.SDK_INT >= 26) {
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, mDocumentUri);
        }

        startActivityForResult(intent, REQUEST_CODE_SAVEAS);
    }

    /**
     * Saves the document under the given URI using ODF format
     * and uses that URI from now on for all operations.
     * @param newUri URI to save the document and use from now on.
     */
    private void saveDocumentAs(Uri newUri) {
        mDocumentUri = newUri;
        // save in ODF format
        mTileProvider.saveDocumentAs(mTempFile.getPath(), true);
        saveFileToOriginalSource();

        String displayName = FileUtilities.retrieveDisplayNameForDocumentUri(getContentResolver(), mDocumentUri);
        toolbarTop.setTitle(displayName);
        mbReadOnlyDoc = false;
        getToolbarController().setupToolbars();
    }

    public void exportToPDF() {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(FileUtilities.MIMETYPE_PDF);
        // suggest directory and file name based on the doc
        if (Build.VERSION.SDK_INT >= 26) {
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, mDocumentUri);
        }
        final String displayName = toolbarTop.getTitle().toString();
        final String suggestedFileName = FileUtilities.stripExtensionFromFileName(displayName) + ".pdf";
        intent.putExtra(Intent.EXTRA_TITLE, suggestedFileName);

        startActivityForResult(intent, REQUEST_CODE_EXPORT_TO_PDF);
    }

    private void exportToPDF(final Uri uri) {
        boolean exportOK = false;
        File tempFile = null;
        try {
            tempFile = File.createTempFile("LibreOffice_", ".pdf");
            mTileProvider.saveDocumentAs(tempFile.getAbsolutePath(),"pdf", false);

            try {
                FileInputStream inputStream = new FileInputStream(tempFile);
                exportOK = copyStreamToUri(inputStream, uri);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (tempFile != null && tempFile.exists()) {
                tempFile.delete();
            }
        }

        final int msgId = exportOK ? R.string.pdf_export_finished : R.string.unable_to_export_pdf;
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                showCustomStatusMessage(getString(msgId));
            }
        });
    }

    /**
     * Returns the ODF MIME type that can be used for the current document,
     * regardless of whether the document is an ODF Document or not
     * (e.g. returns FileUtilities.MIMETYPE_OPENDOCUMENT_TEXT for a DOCX file).
     * @return MIME type, or empty string, if no appropriate MIME type could be found.
     */
    private String getODFMimeTypeForDocument() {
        if (mTileProvider.isTextDocument())
            return FileUtilities.MIMETYPE_OPENDOCUMENT_TEXT;
        else if (mTileProvider.isSpreadsheet())
            return FileUtilities.MIMETYPE_OPENDOCUMENT_SPREADSHEET;
        else if (mTileProvider.isPresentation())
            return FileUtilities.MIMETYPE_OPENDOCUMENT_PRESENTATION;
        else if (mTileProvider.isDrawing())
            return FileUtilities.MIMETYPE_OPENDOCUMENT_GRAPHICS;
        else {
            Log.w(LOGTAG, "Cannot determine MIME type to use.");
            return "";
        }
    }

    /**
     * Returns whether the MIME type for the URI is considered one for a document template.
     */
    private boolean isTemplate(final Uri documentUri) {
        final String mimeType = getContentResolver().getType(documentUri);
        return FileUtilities.isTemplateMimeType(mimeType);
    }

    public void saveFileToOriginalSource() {
        if (mTempFile == null || mDocumentUri == null || !mDocumentUri.getScheme().equals(ContentResolver.SCHEME_CONTENT))
            return;

        boolean copyOK = false;
        try {
            final FileInputStream inputStream = new FileInputStream(mTempFile);
            copyOK = copyStreamToUri(inputStream, mDocumentUri);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        if (copyOK) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(LibreOfficeMainActivity.this, R.string.message_saved,
                        Toast.LENGTH_SHORT).show();
                }
            });
            setDocumentChanged(false);
        } else {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(LibreOfficeMainActivity.this, R.string.message_saving_failed,
                        Toast.LENGTH_SHORT).show();
                }
            });
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
        if (!mbSkipNextRefresh) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.REFRESH));
        }
        mbSkipNextRefresh = false;
    }

    @Override
    protected void onStop() {
        Log.i(LOGTAG, "onStop..");
        hideSoftKeyboardDirect();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.i(LOGTAG, "onDestroy..");
        PreferenceManager.getDefaultSharedPreferences(getApplicationContext())
            .unregisterOnSharedPreferenceChangeListener(this);

        LOKitShell.sendCloseEvent();
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
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
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
        mTileProvider.setDocumentPassword("file://" + mTempFile.getPath(), mPassword);
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

    public static boolean isReadOnlyMode() {
        return !isExperimentalMode() || mbReadOnlyDoc;
    }

    public boolean hasLocationForSave() {
        return mDocumentUri != null;
    }

    public static void setDocumentChanged (boolean changed) {
        isDocumentChanged = changed;
    }

    private class DocumentPartClickListener implements android.widget.AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            DocumentPartView partView = mDocumentPartViewListAdapter.getItem(position);
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

    /**
     * Copies everything from the given input stream to the given output stream
     * and closes both streams in the end.
     * @return Whether copy operation was successful.
     */
    private boolean copyStream(InputStream inputStream, OutputStream outputStream) {
        try {
            byte[] buffer = new byte[4096];
            int readBytes = inputStream.read(buffer);
            while (readBytes != -1) {
                outputStream.write(buffer, 0, readBytes);
                readBytes = inputStream.read(buffer);
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                inputStream.close();
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Copies everything from the given Uri to the given OutputStream
     * and closes the OutputStream in the end.
     * The copy operation runs in a separate thread, but the method only returns
     * after the thread has finished its execution.
     * This can be used to copy in a blocking way when network access is involved,
     * which is not allowed from the main thread, but that may happen when an underlying
     * DocumentsProvider (like the NextCloud one) does network access.
     */
    private boolean copyUriToStream(final Uri inputUri, final OutputStream outputStream) {
        class CopyThread extends Thread {
            /** Whether copy operation was successful. */
            private boolean result = false;

            @Override
            public void run() {
                final ContentResolver contentResolver = getContentResolver();
                try {
                    InputStream inputStream = contentResolver.openInputStream(inputUri);
                    result = copyStream(inputStream, outputStream);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            }
        }
        CopyThread copyThread = new CopyThread();
        copyThread.start();
        try {
            // wait for copy operation to finish
            // NOTE: might be useful to add some indicator in UI for long copy operations involving network...
            copyThread.join();
        } catch(InterruptedException e) {
            e.printStackTrace();
        }
        return copyThread.result;
    }

    /**
     * Copies everything from the given InputStream to the given URI and closes the
     * InputStream in the end.
     * @see LibreOfficeMainActivity#copyUriToStream(Uri, OutputStream)
     *      which does the same thing the other way around.
     */
    private boolean copyStreamToUri(final InputStream inputStream, final Uri outputUri) {
        class CopyThread extends Thread {
            /** Whether copy operation was successful. */
            private boolean result = false;

            @Override
            public void run() {
                final ContentResolver contentResolver = getContentResolver();
                try {
                    OutputStream outputStream = contentResolver.openOutputStream(outputUri);
                    result = copyStream(inputStream, outputStream);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            }
        }
        CopyThread copyThread = new CopyThread();
        copyThread.start();
        try {
            // wait for copy operation to finish
            // NOTE: might be useful to add some indicator in UI for long copy operations involving network...
            copyThread.join();
        } catch(InterruptedException e) {
            e.printStackTrace();
        }
        return copyThread.result;
    }

    public void showCustomStatusMessage(String message){
        Snackbar.make(mDrawerLayout, message, Snackbar.LENGTH_LONG).show();
    }

    public void preparePresentation() {
        if (getExternalCacheDir() != null) {
            String tempPath = getExternalCacheDir().getPath() + "/" + mTempFile.getName() + ".svg";
            mTempSlideShowFile = new File(tempPath);
            if (mTempSlideShowFile.exists() && !isDocumentChanged) {
                startPresentation("file://" + tempPath);
            } else {
                LOKitShell.sendSaveCopyAsEvent(tempPath, "svg");
            }
        }
    }

    public void startPresentation(String tempPath) {
        Intent intent = new Intent(this, PresentationActivity.class);
        intent.setData(Uri.parse(tempPath));
        startActivity(intent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_SAVEAS && resultCode == RESULT_OK) {
            final Uri fileUri = data.getData();
            saveDocumentAs(fileUri);
        } else if (requestCode == REQUEST_CODE_EXPORT_TO_PDF && resultCode == RESULT_OK) {
            final Uri fileUri = data.getData();
            exportToPDF(fileUri);
        } else {
            mFormattingController.handleActivityResult(requestCode, resultCode, data);
            hideBottomToolbar();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
