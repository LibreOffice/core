/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Insets;
import android.hardware.input.InputManager;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintManager;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;
import android.provider.Settings;
import android.util.Log;
import android.view.InputDevice;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.webkit.JavascriptInterface;
import android.webkit.MimeTypeMap;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.RatingBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedWriter;
import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.ReadableByteChannel;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Base64;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicReference;

import androidx.activity.OnBackPressedCallback;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.android.tools.r8.keepanno.annotations.UsedByNative;

public class LOActivity extends AppCompatActivity {
    final static String TAG = "LOActivity";

    private static final String ASSETS_EXTRACTED_GIT_COMMIT = "ASSETS_EXTRACTED_GIT_COMMIT";
    private static final int PERMISSION_WRITE_EXTERNAL_STORAGE = 777;
    private static final long CLOSE_TIMEOUT_MS = 30 * 1000;
    private static final String KEY_ENABLE_SHOW_DEBUG_INFO = "ENABLE_SHOW_DEBUG_INFO";
    private static final String KEY_ALL_FILES_ACCESS_DECLINED = "ALL_FILES_ACCESS_DECLINED";

    private static final String KEY_PROVIDER_ID = "providerID";
    private static final String KEY_DOCUMENT_URI = "documentUri";
    private static final String KEY_IS_EDITABLE = "isEditable";
    private static final String KEY_INTENT_URI = "intentUri";
    public static final String RECENT_DOCUMENTS_KEY = "RECENT_DOCUMENTS_LIST";
    private static String USER_NAME_KEY = "USER_NAME";
    public static final String NIGHT_MODE_KEY = "NIGHT_MODE";

    private File mTempFile = null;

    /// The actual file path of the document, when the app can write it directly.
    private volatile File mResolvedFile = null;

    /// True once the user has been asked for access to all files for the document being opened.
    private boolean mAllFilesAccessAsked = false;

    private int providerId;
    private Activity mActivity;

    @Nullable
    private URI documentUri;

    private String urlToLoad;
    // Both are read from the thread that forwards the server's messages and cleared when the
    // activity is destroyed.
    private volatile COWebView mWebView = null;
    private volatile MobileSocket mMobileSocket = null;
    private COWebViewClient mWebViewClient = null;
    private SharedPreferences sPrefs;
    private Handler mMainHandler = null;
    private RateAppController rateAppController;

    private boolean isDocEditable = false;
    private boolean isDocDebuggable = BuildConfig.DEBUG;
    private volatile boolean documentLoaded = false;
    /** What to do once the engine has dropped the document that is closing. */
    private final AtomicReference<Runnable> mWhenDocumentClosed = new AtomicReference<>();
    /** Ends the wait for the engine's CLOSED report when that report does not come in time. */
    private final AtomicReference<Runnable> mCloseTimeout = new AtomicReference<>();
    /** The URI the closing document was opened from. */
    private volatile String mClosingUri = null;

    /// The number the native side knows this activity's document by, 0 before it is opened.
    private volatile int mDocumentId = 0;

    private Bundle savedInstanceState;

    private ProgressDialog mProgressDialog = null;

    /** In case the mobile-wizard is visible, we have to intercept the Android's Back button. */
    private boolean mMobileWizardVisible = false;
    private boolean mIsEditModeActive = false;
    /// True while a slide show is on screen in the WebView.
    private volatile boolean mSlideShowActive = false;

    /// True while a physical keyboard is attached and can be typed on. Written on the main thread
    /// and read on the thread that serves the JavaScript bridge, so it is volatile.
    private volatile boolean mHardwareKeyboardAttached = false;

    /// The theme the loaded document is rendered in: true is dark mode, false is light mode.
    private boolean mDarkModeApplied = false;

    private InputManager mInputManager = null;
    private InputManager.InputDeviceListener mInputDeviceListener = null;

    private ValueCallback<Uri[]> valueCallback;

    private View mCurrentFullScreenView = null;
    private WebChromeClient.CustomViewCallback mCurrentFullScreenCallback = null;

    /// Source file path remembered while a REQUEST_EXPORT_FILE intent is in flight.
    private String mPendingExportSourcePath = null;

    public static final int REQUEST_SELECT_IMAGE_FILE = 500;
    public static final int REQUEST_SAVEAS_PDF = 501;
    public static final int REQUEST_SAVEAS_RTF = 502;
    public static final int REQUEST_SAVEAS_ODT = 503;
    public static final int REQUEST_SAVEAS_ODP = 504;
    public static final int REQUEST_SAVEAS_ODS = 505;
    public static final int REQUEST_SAVEAS_DOCX = 506;
    public static final int REQUEST_SAVEAS_PPTX = 507;
    public static final int REQUEST_SAVEAS_XLSX = 508;
    public static final int REQUEST_SAVEAS_DOC = 509;
    public static final int REQUEST_SAVEAS_PPT = 510;
    public static final int REQUEST_SAVEAS_XLS = 511;
    public static final int REQUEST_SAVEAS_EPUB = 512;
    public static final int REQUEST_EXPORT_FILE = 513;
    public static final int REQUEST_SAVEAS_CSV = 514;
    public static final int REQUEST_SAVEAS_HTML = 515;
    public static final int REQUEST_SAVEAS_MD = 516;
    public static final int REQUEST_COPY = 600;
    public static final int REQUEST_ALL_FILES_ACCESS = 601;
    public static final int REQUEST_LINKED_FOLDER = 602;

    /// The files the document links to that wait for the user to allow the folder holding them.
    private JSONArray mPendingLinkedFiles = null;
    /** The copy the engine reads of each linked file, by its path relative to the document. */
    private final Map<String, File> mLinkedFileCopies = new HashMap<>();
    private static final ExecutorService linkedFilesExecutor = Executors.newSingleThreadExecutor();

    /** Broadcasting event for passing info back to the shell. */
    public static final String LO_ACTIVITY_BROADCAST = "LOActivityBroadcast";

    /** Event description for passing info back to the shell. */
    public static final String LO_ACTION_EVENT = "LOEvent";

    /** Data description for passing info back to the shell. */
    public static final String LO_ACTION_DATA = "LOData";

    /** shared pref key for recent files. */
    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";

    private static boolean copyFromAssets(AssetManager assetManager,
                                          String fromAssetPath, String targetDir) {
        try {
            String[] files = assetManager.list(fromAssetPath);
            boolean res = true;
            for (String file : files) {
                String[] dirOrFile = assetManager.list(fromAssetPath + "/" + file);
                if (dirOrFile.length == 0) {
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
            Log.e(TAG, "copyFromAssets failed: " + e.getMessage());
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
                // might not copy all at once, so make sure everything gets copied....
                ByteBuffer buffer = ByteBuffer.allocate(4096);
                while (source.read(buffer) > 0) {
                    buffer.flip();
                    bytesTransferred += dest.write(buffer);
                    buffer.clear();
                }
                Log.v(TAG, "Success copying " + fromAssetPath + " to " + toPath + " bytes: " + bytesTransferred);
                return true;
            } finally {
                if (dest != null) dest.close();
                if (source != null) source.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "file " + fromAssetPath + " not found! " + e.getMessage());
            return false;
        } catch (IOException e) {
            Log.e(TAG, "failed to copy file " + fromAssetPath + " from assets to " + toPath + " - " + e.getMessage());
            return false;
        }
    }

    private Handler getMainHandler() {
        if (mMainHandler == null) {
            mMainHandler = new Handler(getMainLooper());
        }
        return mMainHandler;
    }

    /** True if the App is running under ChromeOS. */
    public static boolean isChromeOS(Context context) {
        return context.getPackageManager().hasSystemFeature("org.chromium.arc.device_management");
    }

    /** True if a physical keyboard with letter keys is attached and can be typed on right now. */
    private static boolean detectHardwareKeyboard(Context context) {
        Configuration configuration = context.getResources().getConfiguration();
        if (configuration.keyboard != Configuration.KEYBOARD_NOKEYS
                && configuration.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_YES)
            return false;

        for (int id : InputDevice.getDeviceIds()) {
            InputDevice device = InputDevice.getDevice(id);
            if (device == null || device.isVirtual())
                continue;

            if ((device.getSources() & InputDevice.SOURCE_KEYBOARD) != 0
                    && device.getKeyboardType() == InputDevice.KEYBOARD_TYPE_ALPHABETIC)
                return true;
        }

        return false;
    }

    /** Watch for keyboards being paired, plugged in or removed. */
    private void startWatchingForHardwareKeyboard() {
        mHardwareKeyboardAttached = detectHardwareKeyboard(this);

        mInputManager = (InputManager) getSystemService(Context.INPUT_SERVICE);
        if (mInputManager == null)
            return;

        mInputDeviceListener = new InputManager.InputDeviceListener() {
            @Override
            public void onInputDeviceAdded(int deviceId) {
                updateHardwareKeyboardState();
            }

            @Override
            public void onInputDeviceRemoved(int deviceId) {
                updateHardwareKeyboardState();
            }

            @Override
            public void onInputDeviceChanged(int deviceId) {
                updateHardwareKeyboardState();
            }
        };

        mInputManager.registerInputDeviceListener(mInputDeviceListener, getMainHandler());
    }

    private void stopWatchingForHardwareKeyboard() {
        if (mInputManager != null && mInputDeviceListener != null)
            mInputManager.unregisterInputDeviceListener(mInputDeviceListener);

        mInputManager = null;
        mInputDeviceListener = null;
    }

    /** Re-read the keyboard state and pass it on when it differs from what JavaScript last saw. */
    private void updateHardwareKeyboardState() {
        boolean attached = detectHardwareKeyboard(this);
        if (attached == mHardwareKeyboardAttached)
            return;

        mHardwareKeyboardAttached = attached;
        Log.i(TAG, "Hardware keyboard " + (attached ? "attached" : "detached"));

        if (documentLoaded)
            callFakeWebsocketOnMessage(attached ? "mobile: hardwarekeyboardattached"
                                                : "mobile: hardwarekeyboarddetached");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.savedInstanceState = savedInstanceState;
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        sPrefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

        setContentView(R.layout.lolib_activity_main);
        mProgressDialog = new ProgressDialog(this);
        if (BuildConfig.GOOGLE_PLAY_ENABLED)
            this.rateAppController = new RateAppController(this);
        else
            this.rateAppController = null;
        this.mActivity = this;

        getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                if (!documentLoaded) {
                    finishAndRemoveTask();
                    return;
                }

                if (mCurrentFullScreenView != null) {
                    hideFullScreenView();
                    return;
                } else if (mSlideShowActive) {
                    callFakeWebsocketOnMessage("mobile: slideshowback");
                    return;
                } else if (mMobileWizardVisible) {
                    // just return one level up in the mobile-wizard (or close it)
                    callFakeWebsocketOnMessage("mobile: mobilewizardback");
                    return;
                } else if (mIsEditModeActive) {
                    callFakeWebsocketOnMessage("mobile: readonlymode");
                    return;
                }

                closeDocumentAndFinish();
            }
        });

        startWatchingForHardwareKeyboard();

        init();
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfiguration) {
        super.onConfigurationChanged(newConfiguration);
        updateHardwareKeyboardState();
        updateDarkMode();
    }

    /** Retheme the loaded document when the effective dark mode setting has changed. */
    private void updateDarkMode() {
        if (!documentLoaded)
            return;

        boolean darkMode = isDarkMode();
        if (darkMode == mDarkModeApplied)
            return;

        mDarkModeApplied = darkMode;
        callFakeWebsocketOnMessage(darkMode ? "mobile: darkthemeactivated"
                                            : "mobile: lightthemeactivated");
    }

    /** Initialize the app - copy the assets and create the UI. */
    private void init() {
        if (sPrefs.getString(ASSETS_EXTRACTED_GIT_COMMIT, "").equals(BuildConfig.GIT_COMMIT)) {
            // all is fine, we have already copied the assets
            initUIWhenDocumentFree();
            return;
        }

        mProgressDialog.indeterminate(R.string.preparing_for_the_first_start_after_an_update);

        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                // copy the new assets
                if (copyFromAssets(getAssets(), "unpack", getApplicationInfo().dataDir)) {
                    sPrefs.edit().putString(ASSETS_EXTRACTED_GIT_COMMIT, BuildConfig.GIT_COMMIT).apply();
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                initUIWhenDocumentFree();
            }
        }.execute();
    }

    private void initUIWhenDocumentFree() {
        final String uri = intentUri();
        if (uri == null || !DocumentCloseService.isPending(uri)) {
            initUI();
            return;
        }

        mProgressDialog.indeterminate(R.string.loading);
        DocumentCloseService.whenNotPending(uri, CLOSE_TIMEOUT_MS, () -> {
            if (isFinishing() || isDestroyed())
                return;

            initUI();
        });
    }

    /** Actual initialization of the UI. */
    private void initUI() {
        isDocDebuggable = sPrefs.getBoolean(KEY_ENABLE_SHOW_DEBUG_INFO, false) && BuildConfig.DEBUG;
        mResolvedFile = null;

        if (getIntent().getData() != null) {

            if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
                isDocEditable = true;

                // The launching app did not grant write access, so we were
                // handed a read-only copy and cannot save changes back to the
                // original. Ask for "All files access" permission to edit the
                // underlying file.
                if ((getIntent().getFlags() & Intent.FLAG_GRANT_WRITE_URI_PERMISSION) == 0) {
                    mResolvedFile = resolveWritableFile(getIntent().getData());
                    if (mResolvedFile == null) {
                        isDocEditable = false;
                        Log.d(TAG, "Disabled editing: Read-only");
                        if (!canAllFilesAccessHelp())
                            showReadOnlyToast();
                    } else {
                        Log.i(TAG, "Editing enabled, saving back to " + mResolvedFile);
                    }
                }

                // turns out that on ChromeOS, it is not possible to save back
                // to Google Drive; detect it already here to avoid disappointment
                // also the volumeprovider does not work for saving back,
                // which is much more serious :-(
                if (isDocEditable && (getIntent().getData().toString().startsWith("content://org.chromium.arc.chromecontentprovider/externalfile") ||
                                      getIntent().getData().toString().startsWith("content://org.chromium.arc.volumeprovider/"))) {
                    isDocEditable = false;
                    Log.d(TAG, "Disabled editing: Chrome OS unsupported content providers");
                    Toast.makeText(this, getResources().getString(R.string.file_chromeos_read_only), Toast.LENGTH_LONG).show();
                }

                if (copyFileToTemp() && mTempFile != null) {
                    documentUri = mTempFile.toURI();
                    urlToLoad = documentUri.toString();
                    Log.d(TAG, "SCHEME_CONTENT: getPath(): " + getIntent().getData().getPath());
                } else {
                    Log.e(TAG, "couldn't create temporary file from " + getIntent().getData());
                    Toast.makeText(this, R.string.cant_open_the_document, Toast.LENGTH_SHORT).show();
                    finish();
                }
            } else if (getIntent().getData().getScheme().equals(ContentResolver.SCHEME_FILE)) {
                isDocEditable = true;
                urlToLoad = getIntent().getData().toString();
                Log.d(TAG, "SCHEME_FILE: getPath(): " + getIntent().getData().getPath());
                // Gather data to rebuild IFile object later
                providerId = getIntent().getIntExtra(
                        "org.libreoffice.document_provider_id", 0);
                documentUri = (URI) getIntent().getSerializableExtra(
                        "org.libreoffice.document_uri");
            }
        } else if (savedInstanceState != null) {
            getIntent().setAction(Intent.ACTION_VIEW)
                    .setData(Uri.parse(savedInstanceState.getString(KEY_INTENT_URI)));
            urlToLoad = getIntent().getData().toString();
            providerId = savedInstanceState.getInt(KEY_PROVIDER_ID);
            if (savedInstanceState.getString(KEY_DOCUMENT_URI) != null) {
                try {
                    documentUri = new URI(savedInstanceState.getString(KEY_DOCUMENT_URI));
                    urlToLoad = documentUri.toString();
                } catch (URISyntaxException e) {
                    e.printStackTrace();
                }
            }
            isDocEditable = savedInstanceState.getBoolean(KEY_IS_EDITABLE);
        } else {
            //User can't reach here but if he/she does then
            Toast.makeText(this, getString(R.string.failed_to_load_file), Toast.LENGTH_SHORT).show();
            finish();
        }
        // some types don't have export filter so we cannot edit them
        // only set it to false if it returns false otherwise it can break previous controls
        if (!canDocumentBeExported())
            isDocEditable = false;
        if (urlToLoad != null)
        {
            mWebView = (COWebView) findViewById(R.id.browser);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                mWebView.setOnApplyWindowInsetsListener((v, windowInsets) -> {
                    Insets insets = windowInsets.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.ime() | (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE ? WindowInsets.Type.systemOverlays() : 0));

                    ViewGroup.MarginLayoutParams mlp = (ViewGroup.MarginLayoutParams) v.getLayoutParams();
                    mlp.leftMargin = insets.left;
                    mlp.topMargin = insets.top;
                    mlp.rightMargin = insets.right;
                    mlp.bottomMargin = insets.bottom;
                    v.setLayoutParams(mlp);

                    return WindowInsets.CONSUMED;
                });
                mWebView.requestApplyInsets();

                boolean lightMode = (getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_YES) == 0;
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).setAppearanceLightStatusBars(lightMode);
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).setAppearanceLightNavigationBars(lightMode);
            }

            mWebViewClient = mWebView.getWebViewClient();
            mMobileSocket = mWebViewClient.getMobileSocket();

            WebSettings webSettings = mWebView.getSettings();
            webSettings.setJavaScriptEnabled(true);
            mWebView.addJavascriptInterface(this, "COOLMessageHandler");

            webSettings.setDomStorageEnabled(true);

            // allow debugging (when building the debug version); see details in
            // https://developers.google.com/web/tools/chrome-devtools/remote-debugging/webviews
            boolean isChromeDebugEnabled = sPrefs.getBoolean("ENABLE_CHROME_DEBUGGING", false);
            if ((getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0 || isChromeDebugEnabled) {
                WebView.setWebContentsDebuggingEnabled(true);
            }

            getMainHandler();

            SystemClipboard.initialize(this);

            mWebView.setWebChromeClient(new WebChromeClient() {
                @Override
                public boolean onShowFileChooser(WebView mWebView, ValueCallback<Uri[]> filePathCallback, WebChromeClient.FileChooserParams fileChooserParams) {
                    if (valueCallback != null) {
                        valueCallback.onReceiveValue(null);
                        valueCallback = null;
                    }

                    valueCallback = filePathCallback;
                    Intent intent = fileChooserParams.createIntent();
                    restrictToAcceptedTypes(intent, fileChooserParams.getAcceptTypes());

                    try {
                        startActivityForResult(intent, REQUEST_SELECT_IMAGE_FILE);
                    } catch (ActivityNotFoundException e) {
                        valueCallback = null;
                        Toast.makeText(LOActivity.this, getString(R.string.cannot_open_file_chooser), Toast.LENGTH_LONG).show();
                        return false;
                    }
                    return true;
                }

                @Override
                public void onShowCustomView(View view, WebChromeClient.CustomViewCallback callback) {
                    showFullScreenView(view, callback);
                }

                @Override
                public void onHideCustomView() {
                    hideFullScreenView();
                }
            });

            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU && ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                Log.i(TAG, "asking for read storage permission");
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        PERMISSION_WRITE_EXTERNAL_STORAGE);
            } else {
                loadDocument();
            }
        }
    }

    /**
     * Limit the file chooser to the accept types of the file input. The intent type is the kind
     * most of the types share ("image/*"), so apps of that kind offer themselves in the chooser.
     * The full list goes along as EXTRA_MIME_TYPES for the documents picker, which filters on it.
     * Extensions the platform knows map to their type, unknown ones are left out. No known type
     * means any file.
     */
    private static void restrictToAcceptedTypes(Intent intent, String[] acceptTypes) {
        ArrayList<String> mimeTypes = new ArrayList<>();
        HashMap<String, Integer> kindCounts = new HashMap<>();
        if (acceptTypes != null) {
            for (String acceptType : acceptTypes) {
                if (acceptType == null)
                    continue;
                for (String part : acceptType.split(",")) {
                    String type = part.trim().toLowerCase(Locale.ROOT);
                    if (type.startsWith("."))
                        type = MimeTypeMap.getSingleton()
                                .getMimeTypeFromExtension(type.substring(1));
                    if (type == null || type.indexOf('/') < 1 || mimeTypes.contains(type))
                        continue;
                    mimeTypes.add(type);
                    String kind = type.substring(0, type.indexOf('/'));
                    Integer count = kindCounts.get(kind);
                    kindCounts.put(kind, count == null ? 1 : count + 1);
                }
            }
        }

        String dominantKind = null;
        int dominantCount = 0;
        for (Map.Entry<String, Integer> entry : kindCounts.entrySet()) {
            if (entry.getValue() > dominantCount) {
                dominantKind = entry.getKey();
                dominantCount = entry.getValue();
            }
        }

        intent.removeExtra(Intent.EXTRA_MIME_TYPES);
        if (dominantKind == null) {
            intent.setType("*/*");
            return;
        }
        intent.setType(dominantKind + "/*");
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes.toArray(new String[0]));
    }

    /** True when "All files access" permission can help edit the document. */
    private boolean canAllFilesAccessHelp() {
        if (mAllFilesAccessAsked || Build.VERSION.SDK_INT < Build.VERSION_CODES.R)
            return false;

        if (sPrefs.getBoolean(KEY_ALL_FILES_ACCESS_DECLINED, false))
            return false;

        if (isDocEditable || !canDocumentBeExported())
            return false;

        Uri uri = getIntent().getData();
        if (uri == null || !ContentResolver.SCHEME_CONTENT.equals(uri.getScheme()))
            return false;

        if ((getIntent().getFlags() & Intent.FLAG_GRANT_WRITE_URI_PERMISSION) != 0)
            return false;

        return !Environment.isExternalStorageManager() && resolveSaveTarget(uri) != null;
    }

    /** Show something in fullscreen mode. Required to show slideshows (but also, say, videos) in a fullscreen view */
    private void showFullScreenView(View view, WebChromeClient.CustomViewCallback callback) {
        if (mCurrentFullScreenView != null) {
            callback.onCustomViewHidden();
            return;
        }

        mCurrentFullScreenView = view;
        mCurrentFullScreenCallback = callback;

        ViewGroup decorView = (ViewGroup) getWindow().getDecorView();
        decorView.addView(view, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        WindowInsetsControllerCompat insetsController = WindowCompat.getInsetsController(getWindow(), decorView);
        insetsController.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        insetsController.hide(WindowInsetsCompat.Type.systemBars());
    }

    private void hideFullScreenView() {
        if (mCurrentFullScreenView == null)
            return;

        ViewGroup decorView = (ViewGroup) getWindow().getDecorView();
        decorView.removeView(mCurrentFullScreenView);
        WindowCompat.getInsetsController(getWindow(), decorView).show(WindowInsetsCompat.Type.systemBars());

        mCurrentFullScreenView = null;
        mCurrentFullScreenCallback.onCustomViewHidden();
        mCurrentFullScreenCallback = null;
    }

    /** Offer the user access to all files, so the document can be saved where it is stored. */
    private void askForAllFilesAccess() {
        mAllFilesAccessAsked = true;
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(getString(R.string.all_files_access_title));
        builder.setMessage(getString(R.string.all_files_access_message));
        builder.setPositiveButton(getString(R.string.all_files_access_grant),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
                            showAllFilesAccessSettings();
                    }
                });
        builder.setNegativeButton(getString(R.string.view_only),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        showReadOnlyToast();
                    }
                });
        builder.setNeutralButton(getString(R.string.do_not_ask_again),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        sPrefs.edit().putBoolean(KEY_ALL_FILES_ACCESS_DECLINED, true).apply();
                        showReadOnlyToast();
                    }
                });
        builder.setCancelable(true);
        builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialogInterface) {
                showReadOnlyToast();
            }
        });
        builder.show();
    }

    /** Tell the user that the document stays as it is, whatever they do to it here. */
    private void showReadOnlyToast() {
        Toast.makeText(this, getResources().getString(R.string.temp_file_saving_disabled),
                Toast.LENGTH_LONG).show();
    }

    /** Open the system screen where the user turns access to all files on for this app. */
    @RequiresApi(api = Build.VERSION_CODES.R)
    private void showAllFilesAccessSettings() {
        Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                Uri.parse("package:" + getPackageName()));
        try {
            startActivityForResult(intent, REQUEST_ALL_FILES_ACCESS);
        } catch (ActivityNotFoundException e) {
            Log.i(TAG, "no per-app screen for access to all files: " + e.getMessage());
            try {
                startActivityForResult(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION),
                        REQUEST_ALL_FILES_ACCESS);
            } catch (ActivityNotFoundException e2) {
                Log.i(TAG, "no screen for access to all files at all: " + e2.getMessage());
            }
        }
    }

    /** The place in the filesystem where saving could work, if the app may write it. */
    private File resolveSaveTarget(Uri uri) {
        File file = UriPathResolver.resolve(this, uri);
        if (file == null)
            return null;

        // Access to all files stops at the private directory of another app.
        String path = file.getAbsolutePath();
        if (path.contains("/Android/data/") || path.contains("/Android/obb/"))
            return null;

        return file;
    }

    /** True when the URI and the file describe one and the same document. */
    private boolean isSameFileAsUri(Uri uri, File file) {
        String[] projection = new String[]{OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE};
        try (Cursor cursor = getContentResolver().query(uri, projection, null, null, null)) {
            if (cursor == null || !cursor.moveToFirst())
                return false;

            int nameColumn = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
            if (nameColumn < 0 || cursor.isNull(nameColumn)
                    || !cursor.getString(nameColumn).equals(file.getName()))
                return false;

            // A provider that keeps the size of the document to itself leaves the name of the
            // file as the only thing the two can be compared by.
            int sizeColumn = cursor.getColumnIndex(OpenableColumns.SIZE);
            if (sizeColumn < 0 || cursor.isNull(sizeColumn))
                return true;

            return cursor.getLong(sizeColumn) == file.length();
        } catch (Exception e) {
            Log.i(TAG, "no name and size for " + uri + ": " + e.getMessage());
            return false;
        }
    }

    /** The document as a file this app can write, or null when the save has to go elsewhere. */
    private File resolveWritableFile(Uri uri) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R || !Environment.isExternalStorageManager())
            return null;

        File file = resolveSaveTarget(uri);
        if (file == null || !file.isFile() || !file.canWrite())
            return null;

        // The new content is written beside the document before it is put in place, so the
        // directory has to take a new file as well.
        File directory = file.getParentFile();
        if (directory == null || !directory.canWrite())
            return null;

        if (!isSameFileAsUri(uri, file)) {
            return null;
        }

        return file;
    }

    @Override
    protected void onNewIntent(Intent intent) {

        Log.i(TAG, "onNewIntent");

        super.onNewIntent(intent);

        // Every document has a task of its own, so this intent is for the document on screen.
        // While that document is closing, the file opens again in a fresh instance once the
        // engine has dropped it.
        final Intent reopenIntent = new Intent(intent);
        reopenIntent.setClass(this, LOActivity.class);
        final Runnable reopen = () -> {
            finish();
            startActivity(reopenIntent);
        };
        if (runAfterPendingClose(reopen, true))
            return;

        // After "edit a copy" this activity shows another file than the one its task is matched
        // by, so the requested file opens once the copy has closed.
        if (!Objects.equals(intent.getData(), getIntent().getData()))
            closeDocumentThen(reopen);
    }

    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString(KEY_INTENT_URI, getIntent().getData().toString());
        outState.putInt(KEY_PROVIDER_ID, providerId);
        if (documentUri != null) {
            outState.putString(KEY_DOCUMENT_URI, documentUri.toString());
        }
        //If this activity was opened via contentUri
        outState.putBoolean(KEY_IS_EDITABLE, isDocEditable);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_WRITE_EXTERNAL_STORAGE:
                if (permissions.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    loadDocument();
                } else {
                    Toast.makeText(this, getString(R.string.storage_permission_required), Toast.LENGTH_SHORT).show();
                    finishAndRemoveTask();
                    break;
                }
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    /** When we get the file via a content: URI, we need to put it to a temp file. */
    private boolean copyFileToTemp() {
        final ContentResolver contentResolver = getContentResolver();
        class CopyThread extends Thread {
            /** Whether copy operation was successful. */
            private boolean result = false;
            @Override
            public void run() {
                InputStream inputStream = null;
                OutputStream outputStream = null;
                // CSV files need a .csv suffix to be opened in Calc.
                String suffix = null;
                @Nullable String intentType = mActivity.getIntent().getType();
                if (mActivity.getIntent().getType() == null) {
                    intentType = getMimeType();
                }
                // K-9 mail uses the first, GMail uses the second variant.
                if ("text/comma-separated-values".equals(intentType) || "text/csv".equals(intentType))
                    suffix = ".csv";
                else if ("application/pdf".equals(intentType))
                    suffix = ".pdf";
                else if ("application/vnd.ms-excel".equals(intentType))
                    suffix = ".xls";
                else if ("application/vnd.ms-powerpoint".equals(intentType))
                    suffix = ".ppt";
                try {
                    try {
                        Uri uri = mActivity.getIntent().getData();
                        inputStream = contentResolver.openInputStream(uri);

                        mTempFile = File.createTempFile("CollaboraOffice", suffix, mActivity.getCacheDir());
                        outputStream = new FileOutputStream(mTempFile);

                        byte[] buffer = new byte[1024];
                        int length;
                        long bytes = 0;
                        while ((length = inputStream.read(buffer)) != -1) {
                            outputStream.write(buffer, 0, length);
                            bytes += length;
                        }

                        Log.i(TAG, "Success copying " + bytes + " bytes from " + uri + " to " + mTempFile);
                    } finally {
                        if (inputStream != null)
                            inputStream.close();
                        if (outputStream != null)
                            outputStream.close();
                        result = true;
                    }
                } catch (FileNotFoundException e) {
                    Log.e(TAG, "file not found: " + e.getMessage());
                    result = false;
                } catch (IOException e) {
                    Log.e(TAG, "exception: " + e.getMessage());
                    result = false;
                }
            }
        }
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
    }

    /** Check that we have created a temp file, and if yes, copy it back to the content: URI. */
    private void copyTempBackToIntent() {
        if (!isDocEditable || mTempFile == null || getIntent().getData() == null || !getIntent().getData().getScheme().equals(ContentResolver.SCHEME_CONTENT))
            return;

        if (mResolvedFile != null) {
            copyTempBackToFile();
            return;
        }

        final ContentResolver contentResolver = getContentResolver();
        try {
            Thread copyThread = new Thread(new Runnable() {
                @Override
                public void run() {
                    InputStream inputStream = null;
                    OutputStream outputStream = null;
                    try {
                        try {
                            inputStream = new FileInputStream(mTempFile);

                            int len = inputStream.available();
                            if (len <= 0)
                                // empty for some reason & do not write it back
                                return;

                            Uri uri = getIntent().getData();
                            try {
                                outputStream = contentResolver.openOutputStream(uri, "wt");
                            }
                            catch (FileNotFoundException e) {
                                Log.i(TAG, "failed with the 'wt' mode, trying without: " + e.getMessage());
                                outputStream = contentResolver.openOutputStream(uri);
                            }

                            byte[] buffer = new byte[1024];
                            int length;
                            long bytes = 0;
                            while ((length = inputStream.read(buffer)) != -1) {
                                outputStream.write(buffer, 0, length);
                                bytes += length;
                            }

                            Log.i(TAG, "Success copying " + bytes + " bytes from " + mTempFile + " to " + uri);
                        } finally {
                            if (inputStream != null)
                                inputStream.close();
                            if (outputStream != null)
                                outputStream.close();
                        }
                    } catch (FileNotFoundException e) {
                        Log.e(TAG, "file not found: " + e.getMessage());
                    } catch (Exception e) {
                        Log.e(TAG, "exception: " + e.getMessage());
                    }
                }
            });
            copyThread.start();
            copyThread.join();
        } catch (Exception e) {
            Log.i(TAG, "copyTempBackToIntent: " + e.getMessage());
        }
    }

    /** Copy the temp file back to the document. */
    private void copyTempBackToFile() {
        final File resolvedFile = mResolvedFile;
        if (resolvedFile == null)
            return;

        File newContent = new File(resolvedFile.getParentFile(), "." + resolvedFile.getName() + ".part");
        try {
            long bytes = 0;
            try (InputStream inputStream = new FileInputStream(mTempFile)) {
                int len = inputStream.available();
                if (len <= 0)
                    return;

                try (FileOutputStream outputStream = new FileOutputStream(newContent)) {
                    byte[] buffer = new byte[1024];
                    int length;
                    while ((length = inputStream.read(buffer)) != -1) {
                        outputStream.write(buffer, 0, length);
                        bytes += length;
                    }
                    outputStream.flush();
                    outputStream.getFD().sync();
                }
            }

            if (!newContent.renameTo(resolvedFile)) {
                Log.e(TAG, "failed to put the new content in place of " + resolvedFile);
                newContent.delete();
                reportSaveToFileFailed();
                return;
            }

            Log.i(TAG, "Success copying " + bytes + " bytes from " + mTempFile + " to " + resolvedFile);

            // Make the system read the file again to update meta data.
            MediaScannerConnection.scanFile(this, new String[]{resolvedFile.getAbsolutePath()}, null, null);
        } catch (Exception e) {
            Log.e(TAG, "copyTempBackToFile: " + e.getMessage());
            newContent.delete();
            reportSaveToFileFailed();
        }
    }

    /** Tell the user that the document still holds the content it had before this save. */
    private void reportSaveToFileFailed() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(LOActivity.this, getString(R.string.failed_to_save_file), Toast.LENGTH_LONG).show();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume..");
    }

    @Override
    protected void onPause() {
        // A Save similar to an autosave
        if (documentLoaded)
            postMobileMessageNative(mDocumentId, "save dontTerminateEdit=1 dontSaveIfUnmodified=1");

        super.onPause();
        Log.d(TAG, "onPause() - hinting to save, we might need to return to the doc");
    }

    @Override
    protected void onDestroy() {
        stopWatchingForHardwareKeyboard();

        if (mWebView != null) {
            // Remove the webview from the hierarchy & destroy
            final ViewGroup viewGroup = (ViewGroup) mWebView.getParent();
            if (viewGroup != null)
                viewGroup.removeView(mWebView);
            mWebView.destroy();
            mWebView = null;
            mMobileSocket = null;
        }

        // Destroyed with the document still open: it unloads in the background, and a content:
        // URI permission that came with the intent is gone, so only a file: URL is written back.
        if (documentLoaded)
            closeDocument();

        // The copies of the files the document links to go with it.
        linkedFilesExecutor.execute(this::removeLinkedFileCopies);

        mProgressDialog.dismiss();

        super.onDestroy();
        Log.i(TAG, "onDestroy() - we know we are leaving the document");
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        if (requestCode == REQUEST_LINKED_FOLDER) {
            JSONArray links = mPendingLinkedFiles;
            mPendingLinkedFiles = null;
            if (resultCode == RESULT_OK && intent != null && intent.getData() != null && links != null) {
                Uri tree = intent.getData();
                // Keep what the user allowed: reading the files, and writing them, so that a
                // chapter opened with Edit can be saved where it is.
                int granted = intent.getFlags()
                        & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                if (granted == 0)
                    granted = Intent.FLAG_GRANT_READ_URI_PERMISSION;
                getContentResolver().takePersistableUriPermission(tree, granted);
                final Uri document = getIntent().getData();
                linkedFilesExecutor.execute(() -> giveLinkedFiles(document, tree, links));
            }
            return;
        }

        if (requestCode == REQUEST_ALL_FILES_ACCESS) {
            mResolvedFile = resolveWritableFile(getIntent().getData());
            if (mResolvedFile != null) {
                isDocEditable = true;
                callFakeWebsocketOnMessage("mobile: editmode");
                Log.i(TAG, "Editing enabled, saving back to " + mResolvedFile);
            } else {
                showReadOnlyToast();
            }
            return;
        }

        if (resultCode != RESULT_OK) {
            if (requestCode == REQUEST_SELECT_IMAGE_FILE) {
                valueCallback.onReceiveValue(null);
                valueCallback = null;
            } else if (requestCode == REQUEST_EXPORT_FILE && mPendingExportSourcePath != null) {
                new File(mPendingExportSourcePath).delete();
                mPendingExportSourcePath = null;
            }
            return;
        }

        /*
            Copy is just save-as in general but with TakeOwnership.
            Meaning that we will switch to the copied (saved-as) document in the bg
            this way we don't need to reload the activity.
        */
        boolean requestCopy = false;
        if (requestCode == REQUEST_COPY) {
            requestCopy = true;
            if (Objects.equals(getMimeType(), "text/plain")) {
                requestCode = REQUEST_SAVEAS_ODT;
            } else if (Objects.equals(getMimeType(), "text/comma-separated-values")) {
                requestCode = REQUEST_SAVEAS_ODS;
            } else if (Objects.equals(getMimeType(), "application/vnd.ms-excel.sheet.binary.macroenabled.12")) {
                requestCode = REQUEST_SAVEAS_ODS;
            } else {
                String filename = getFileName(true);
                String extension = filename.substring(filename.lastIndexOf('.') + 1);
                requestCode = getRequestIDForFormat(extension);
                assert (requestCode != 0);
            }
        }
        switch (requestCode) {
            case REQUEST_SELECT_IMAGE_FILE:
                if (valueCallback == null)
                    return;
                valueCallback.onReceiveValue(WebChromeClient.FileChooserParams.parseResult(resultCode, intent));
                valueCallback = null;
                return;
            case REQUEST_EXPORT_FILE: {
                if (intent == null || mPendingExportSourcePath == null) {
                    mPendingExportSourcePath = null;
                    return;
                }
                File srcFile = new File(mPendingExportSourcePath);
                mPendingExportSourcePath = null;
                InputStream inputStream = null;
                OutputStream outputStream = null;
                try {
                    inputStream = new FileInputStream(srcFile);
                    try {
                        outputStream = getContentResolver().openOutputStream(intent.getData(), "wt");
                    } catch (FileNotFoundException e) {
                        Log.i(TAG, "failed with the 'wt' mode, trying without: " + e.getMessage());
                        outputStream = getContentResolver().openOutputStream(intent.getData());
                    }
                    byte[] buffer = new byte[4096];
                    int len;
                    while ((len = inputStream.read(buffer)) != -1) {
                        outputStream.write(buffer, 0, len);
                    }
                    outputStream.flush();
                } catch (Exception e) {
                    Toast.makeText(this, "Something went wrong while exporting: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                    e.printStackTrace();
                } finally {
                    try {
                        if (inputStream != null)
                            inputStream.close();
                        if (outputStream != null)
                            outputStream.close();
                    } catch (Exception e) {
                    }
                    srcFile.delete();
                }
                return;
            }
            case REQUEST_SAVEAS_PDF:
            case REQUEST_SAVEAS_RTF:
            case REQUEST_SAVEAS_ODT:
            case REQUEST_SAVEAS_ODP:
            case REQUEST_SAVEAS_ODS:
            case REQUEST_SAVEAS_DOCX:
            case REQUEST_SAVEAS_PPTX:
            case REQUEST_SAVEAS_XLSX:
            case REQUEST_SAVEAS_DOC:
            case REQUEST_SAVEAS_PPT:
            case REQUEST_SAVEAS_XLS:
            case REQUEST_SAVEAS_EPUB:
            case REQUEST_SAVEAS_CSV:
            case REQUEST_SAVEAS_HTML:
            case REQUEST_SAVEAS_MD:
                if (intent == null) {
                    return;
                }
                String format = getFormatForRequestCode(requestCode);
                File _tempFile = null;
                if (format != null) {
                    InputStream inputStream = null;
                    OutputStream outputStream = null;
                    try {
                        final File tempFile = File.createTempFile("CollaboraOffice", "." + format, this.getCacheDir());
                        String options = requestCopy ? "TakeOwnership" : null;
                        // The HTML filter writes pictures as separate files unless embedded.
                        if ("html".equals(format))
                            options = options == null ? "EmbedImages" : options + ",EmbedImages";
                        LOActivity.this.saveAs(tempFile.toURI().toString(), format, options);

                        inputStream = new FileInputStream(tempFile);
                        try {
                            outputStream = getContentResolver().openOutputStream(intent.getData(), "wt");
                        } catch (FileNotFoundException e) {
                            Log.i(TAG, "failed with the 'wt' mode, trying without: " + e.getMessage());
                            outputStream = getContentResolver().openOutputStream(intent.getData());
                        }

                        byte[] buffer = new byte[4096];
                        int len;
                        while ((len = inputStream.read(buffer)) != -1) {
                            outputStream.write(buffer, 0, len);
                        }
                        outputStream.flush();
                        _tempFile = tempFile;
                    } catch (Exception e) {
                        Toast.makeText(this, "Something went wrong while Saving as: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                        e.printStackTrace();
                    } finally {
                        try {
                            if (inputStream != null)
                                inputStream.close();
                            if (outputStream != null)
                                outputStream.close();
                        } catch (Exception e) {
                        }
                    }
                    if (requestCopy == true) {
                        assert (_tempFile != null);
                        mTempFile = _tempFile;
                        getIntent().setData(intent.getData());
                        mResolvedFile = null;
                        /** add the document to recents */
                        addIntentToRecents(intent);
                        callFakeWebsocketOnMessage("mobile: editmode");
                        isDocEditable = true;
                    }
                    return;
                }
                break;
        }
        Toast.makeText(this, "Unknown request", Toast.LENGTH_LONG).show();
    }

    private void addIntentToRecents(Intent intent) {
        Uri treeFileUri = intent.getData();
        SharedPreferences recentPrefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        String recentList =  recentPrefs.getString(RECENT_DOCUMENTS_KEY, "");
        recentList = treeFileUri.toString() + "\n" + recentList;
        recentPrefs.edit().putString(RECENT_DOCUMENTS_KEY, recentList).apply();
    }

    private String getFormatForRequestCode(int requestCode) {
        switch(requestCode) {
            case REQUEST_SAVEAS_PDF: return "pdf";
            case REQUEST_SAVEAS_RTF: return "rtf";
            case REQUEST_SAVEAS_ODT: return "odt";
            case REQUEST_SAVEAS_ODP: return "odp";
            case REQUEST_SAVEAS_ODS: return "ods";
            case REQUEST_SAVEAS_DOCX: return "docx";
            case REQUEST_SAVEAS_PPTX: return "pptx";
            case REQUEST_SAVEAS_XLSX: return "xlsx";
            case REQUEST_SAVEAS_DOC: return "doc";
            case REQUEST_SAVEAS_PPT: return "ppt";
            case REQUEST_SAVEAS_XLS: return "xls";
            case REQUEST_SAVEAS_EPUB: return "epub";
            case REQUEST_SAVEAS_CSV: return "csv";
            case REQUEST_SAVEAS_HTML: return "html";
            case REQUEST_SAVEAS_MD: return "md";
        }
        return null;
    }

    /**
     * The write-back to a content: URI needs this activity's permission, so the activity stays
     * until the native side reports CLOSED.
     */
    private void closeDocumentThen(Runnable whenClosed) {
        if (documentLoaded) {
            mProgressDialog.indeterminate(R.string.exiting);
            closeDocument();
        }

        if (!runAfterPendingClose(whenClosed, false))
            runOnUiThread(whenClosed);
    }

    /**
     * Queue the work behind the running close, or in place of what is queued there. Returns
     * false when no close is running.
     */
    private boolean runAfterPendingClose(Runnable work, boolean replace) {
        if (mCloseTimeout.get() == null)
            return false;

        Runnable pending;
        Runnable combined;
        do {
            pending = mWhenDocumentClosed.get();
            if (pending == null || replace) {
                combined = work;
            } else {
                final Runnable earlier = pending;
                combined = () -> {
                    earlier.run();
                    work.run();
                };
            }
        } while (!mWhenDocumentClosed.compareAndSet(pending, combined));

        // The close may have ended while the work was queued.
        if (mCloseTimeout.get() == null) {
            final Runnable left = mWhenDocumentClosed.getAndSet(null);
            if (left != null)
                runOnUiThread(left);
        }
        return true;
    }

    private String intentUri() {
        final Uri uri = getIntent().getData();
        return uri != null ? uri.toString() : null;
    }

    /** The server saves and unloads the document in the background. */
    private void closeDocument() {
        final Runnable timeout = this::documentClosed;
        if (!documentLoaded)
            return;

        documentLoaded = false;
        // The timeout marks a close in progress.
        mCloseTimeout.set(timeout);
        final String uri = intentUri();
        mClosingUri = uri;
        DocumentCloseService.pendingStarted(getApplicationContext(), uri);
        getMainHandler().postDelayed(timeout, CLOSE_TIMEOUT_MS);
        if (!closeDocumentNative(mDocumentId))
            documentClosed();
    }

    /**
     * The close is over: the engine dropped the document, the wait gave up, or nothing was loaded.
     */
    private void documentClosed() {
        final Runnable timeout = mCloseTimeout.getAndSet(null);
        if (timeout != null) {
            getMainHandler().removeCallbacks(timeout);
            DocumentCloseService.pendingFinished(getApplicationContext(), mClosingUri);
            mClosingUri = null;
            mProgressDialog.dismiss();
        }

        final Runnable whenClosed = mWhenDocumentClosed.getAndSet(null);
        if (whenClosed != null)
            runOnUiThread(whenClosed);
    }

    private void closeDocumentAndFinish() {
        closeDocumentThen(this::finishAndRemoveTask);
    }

    private void loadDocument() {
        mProgressDialog.determinate(R.string.loading);

        // setup the COOLWSD
        ApplicationInfo applicationInfo = getApplicationInfo();
        String dataDir = applicationInfo.dataDir;
        Log.i(TAG, String.format("Initializing COKit, dataDir=%s\n", dataDir));

        String cacheDir = getApplication().getCacheDir().getAbsolutePath();
        String apkFile = getApplication().getPackageResourcePath();
        AssetManager assetManager = getResources().getAssets();
        String uiMode = (isLargeScreen() && !isChromeOS()) ? "notebookbar" : "classic";
        String userName = getPrefs().getString(USER_NAME_KEY, "Guest User");
        initializeEngine(dataDir, cacheDir, apkFile, assetManager, uiMode, userName);

        mDocumentId = openDocument(urlToLoad);
        mWebViewClient.setDocumentId(mDocumentId);

        // trigger the load of the document
        String finalUrlToLoad = "file:///android_asset/dist/cool.html?file_path=" +
                urlToLoad + "&closebutton=1";

        // set the language
        String language = getResources().getConfiguration().locale.toLanguageTag();

        Log.i(TAG, "Loading with language:  " + language);

        finalUrlToLoad += "&lang=" + language;

        if (isDocEditable) {
            finalUrlToLoad += "&permission=edit";
        } else {
            finalUrlToLoad += "&permission=readonly";
        }

        if (isDocDebuggable) {
            finalUrlToLoad += "&debug=true";
        }

        if (isLargeScreen() && !isChromeOS())
            finalUrlToLoad += "&userinterfacemode=notebookbar";

        mDarkModeApplied = isDarkMode();
        if (mDarkModeApplied) {
            finalUrlToLoad += "&darkTheme=true";
        }

        // load the page
        mWebView.loadUrl(finalUrlToLoad);

        documentLoaded = true;
        DocumentCloseService.documentOpened(getApplicationContext());
    }

    private boolean isDarkMode() {
        SharedPreferences recentPrefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        int mode = recentPrefs.getInt(NIGHT_MODE_KEY, AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
        switch (mode) {
            case -1:
                int darkModeFlag = getBaseContext().getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK;
                return darkModeFlag == Configuration.UI_MODE_NIGHT_YES;
            case 1:
                return false;
            case 2:
                return true;
        }
        return false;
    }

    static {
        System.loadLibrary("androidapp");
    }

    /**
     * Used for determining tablets
     */
    public boolean isLargeScreen() {
        return getResources().getBoolean(R.bool.isLargeScreen);
    }

    public SharedPreferences getPrefs() {
        return sPrefs;
    }

    /**
     * Start the engine and the COOLWSD server. They serve every document of the process, so only
     * the first call does anything.
     */
    private native void initializeEngine(String dataDir, String cacheDir, String apkFile, AssetManager assetManager, String uiMode, String userName);

    /** Returns the number the native side knows the document by. */
    private native int openDocument(String loadFileURL);

    /**
     * Returns the number of the document the page is connected to, a new one when the page
     * started again without closing the old document first.
     */
    private native int connectDocument(int documentId);

    /**
     * Passing messages from JS (instead of the websocket communication).
     */
    @JavascriptInterface
    public void postMobileMessage(String message) {
        Log.d(TAG, "postMobileMessage: " + message);

        // The page outlives the document. Anything it sends after the close would reach the
        // next document.
        if (!documentLoaded)
            return;

        String[] messageAndParameterArray= message.split(" ", 2); // the command and the rest (that can potentially contain spaces too)

        if (beforeMessageFromWebView(messageAndParameterArray)) {
            postMobileMessageNative(mDocumentId, message);
        }
    }

    /** Messages from the native side, which keep arriving after the close, like SAVE and CLOSED. */
    @UsedByNative // androidapp.cpp postDirectMessage()
    void postNativeMessage(String message) {
        Log.d(TAG, "postNativeMessage: " + message);

        final String[] messageAndParameter = message.split(" ", 2);
        if (messageAndParameter[0].equals("CLOSED")) {
            // An earlier document of this activity, replaced when the page started again, reports
            // its close too.
            if (messageAndParameter.length > 1
                    && Integer.parseInt(messageAndParameter[1]) == mDocumentId)
                documentClosed();
            return;
        }

        beforeMessageFromWebView(messageAndParameter);
    }

    private native void postMobileMessageNative(int documentId, String message);

    /**
     * Close the document on the native side. Returns true when the engine still holds a loaded
     * document, in which case postNativeMessage gets "CLOSED <documentId>" once the engine has
     * dropped it.
     */
    private native boolean closeDocumentNative(int documentId);

    /**
     * Passing messages from JS (instead of the websocket communication).
     */
    @JavascriptInterface
    public void postMobileError(String message) {
        // TODO handle this
        Log.d(TAG, "postMobileError: " + message);
    }

    /**
     * Passing messages from JS (instead of the websocket communication).
     */
    @JavascriptInterface
    public void postMobileDebug(String message) {
        // TODO handle this
        Log.d(TAG, "postMobileDebug: " + message);
    }

    /**
     * Provide the info that this app is actually running under ChromeOS - so
     * has to mostly look like on desktop.
     */
    @JavascriptInterface
    public boolean isChromeOS() {
        return isChromeOS(this);
    }

    /**
     * Tell the JavaScript side whether a physical keyboard is attached.
     */
    @JavascriptInterface
    public boolean hasHardwareKeyboard() {
        return mHardwareKeyboardAttached;
    }

    /**
     * Passing message the other way around - from Java to the FakeWebSocket in JS.
     */
    void callFakeWebsocketOnMessage(final String message) {
        rawCallFakeWebsocketOnMessage(message.getBytes());
    }

    /**
     * Similar to callFakeWebsocketOnMessage but 'message' is instead any expression evaluable as
     * JavaScript. For example, you should use this to pass Base64ToArrayBuffer invocations to
     * the fake websocket
     */
    @UsedByNative // androidapp.cpp send2JS
    void rawCallFakeWebsocketOnMessage(final byte[] message) {
        // A late message from the server can arrive after the webview is gone.
        final MobileSocket mobileSocket = mMobileSocket;
        final COWebView webView = mWebView;
        if (mobileSocket == null || webView == null)
            return;

        try {
            mobileSocket.queueSend(message, () -> {
                webView.post(() -> {
                    webView.loadUrl("javascript:window.socket.doSend();");
                });
            });
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        // update progress bar when loading
        if (messageStartsWith(message, "progress")) {
            runOnUiThread(() -> {
                JSONObject messageJSON;
                String messageID;
                String messageString;
                try {
                    messageString = new String(message, "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    throw new RuntimeException(e);
                }

                int jsonStart = messageString.indexOf("{");
                if (jsonStart == -1) {
                    return;
                }

                try {
                    messageJSON = new JSONObject(messageString.substring(jsonStart));
                    messageID = messageJSON.getString("id");
                } catch (JSONException e) {
                    return;
                }

                if (messageID.equals("finish")) {
                    mProgressDialog.dismiss();
                    if (BuildConfig.GOOGLE_PLAY_ENABLED && rateAppController != null)
                        rateAppController.askUserForRating();
                    return;
                }

                try {
                    String text = messageJSON.getString("text");
                    mProgressDialog.message(text);
                } catch (JSONException ignored) {}

                try {
                    int progress = messageJSON.getInt("value");
                    mProgressDialog.determinateProgress(progress);
                } catch (JSONException ignored) {}
            });
        } else if (messageStartsWith(message, "error:")) {
            runOnUiThread(() -> mProgressDialog.dismiss());
        }
    }

    /**
     * The document links to files, the chapters of a master document for example, that the app may
     * not read. A document opened from a document provider is a copy without its folder, so the
     * files are copied from a folder tree the user allowed, where they are next to the document.
     * When no tree allowed before holds them, the user is asked for one.
     */
    private void grantLinkedFiles(String json) {
        final JSONArray links;
        try {
            links = new JSONObject(json).getJSONArray("links");
        } catch (JSONException e) {
            Log.e(TAG, "GRANTLINKEDFILES without a list of links");
            return;
        }
        final Uri document = getIntent().getData();
        if (document == null || !ContentResolver.SCHEME_CONTENT.equals(document.getScheme()))
            return;

        linkedFilesExecutor.execute(() -> {
            for (Uri tree : LinkedFiles.allowedTrees(getContentResolver())) {
                if (resolvesAll(tree, document, links)) {
                    giveLinkedFiles(document, tree, links);
                    return;
                }
            }
            runOnUiThread(() -> {
                mPendingLinkedFiles = links;
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, document);
                startActivityForResult(intent, REQUEST_LINKED_FOLDER);
            });
        });
    }

    private boolean resolvesAll(Uri tree, Uri document, JSONArray links) {
        for (int i = 0; i < links.length(); i++) {
            String id = links.optJSONObject(i) != null ? links.optJSONObject(i).optString("id") : "";
            if (id.isEmpty() || LinkedFiles.resolve(getContentResolver(), tree, document, id) == null)
                return false;
        }
        return true;
    }

    /** Copies each linked file found in the tree and gives the engine the copy, then lets the links update. */
    private void giveLinkedFiles(Uri document, Uri tree, JSONArray links) {
        final List<String> messages = copyLinkedFiles(document, Collections.singletonList(tree), links);
        messages.add("allowlinkupdate");
        postLinkedFileMessages(document, messages);
    }

    /**
     * Gives the engine the messages made for the linked files of the document, on the UI thread,
     * when this activity still shows that document: the copies are made on the linked files
     * executor, which outlives the activity, and the messages go to the document the app shows.
     */
    private void postLinkedFileMessages(Uri document, List<String> messages) {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed() || !document.equals(getIntent().getData()))
                return;
            for (String message : messages)
                postMobileMessageNative(mDocumentId, message);
        });
    }

    /**
     * The user asked to read the linked files anew: a copy is what the file was when it was made,
     * so the files are copied again from the folder trees the user allowed, and the engine reads
     * the fresh copies.
     */
    private void refreshLinkedFiles(String json) {
        final JSONArray links;
        try {
            links = new JSONObject(json).getJSONArray("links");
        } catch (JSONException e) {
            Log.e(TAG, "LINKEDFILESREFRESH without a list of links");
            return;
        }
        final Uri document = getIntent().getData();
        if (document == null || !ContentResolver.SCHEME_CONTENT.equals(document.getScheme()))
            return;

        linkedFilesExecutor.execute(() -> {
            final List<String> messages =
                copyLinkedFiles(document, LinkedFiles.allowedTrees(getContentResolver()), links);
            postLinkedFileMessages(document, messages);
        });
    }

    /**
     * Copies each linked file found in one of the trees, and returns the messages that give the
     * engine the copies. The copy a file had before is removed, as the engine reads the new one.
     * Runs on the linked files executor, which owns mLinkedFileCopies.
     */
    private List<String> copyLinkedFiles(Uri document, List<Uri> trees, JSONArray links) {
        final List<String> messages = new ArrayList<>();
        for (int i = 0; i < links.length(); i++) {
            JSONObject link = links.optJSONObject(i);
            String id = link != null ? link.optString("id") : "";
            if (id.isEmpty())
                continue;
            for (Uri tree : trees) {
                Uri found = LinkedFiles.resolve(getContentResolver(), tree, document, id);
                if (found == null)
                    continue;
                String name = id.substring(id.lastIndexOf('/') + 1);
                File copy = LinkedFiles.copyToCache(getContentResolver(), found, getCacheDir(), name);
                if (copy != null) {
                    messages.add("linkedfile source id=" + Uri.encode(id) + " file="
                                 + Uri.encode(Uri.fromFile(copy).toString()));
                    File previous = mLinkedFileCopies.put(id, copy);
                    if (previous != null)
                        LinkedFiles.removeCopy(previous);
                }
                break;
            }
        }
        return messages;
    }

    /** Removes the copies of the linked files. Runs on the linked files executor. */
    private void removeLinkedFileCopies() {
        for (File copy : mLinkedFileCopies.values())
            LinkedFiles.removeCopy(copy);
        mLinkedFileCopies.clear();
    }

    /** Edit of a file a master document links to: it opens as the document of this activity. */
    private void openLinkedDocument(String arguments) {
        String file = null;
        String id = null;
        for (String token : arguments.split(" ")) {
            if (token.startsWith("file="))
                file = Uri.decode(token.substring("file=".length()));
            else if (token.startsWith("id="))
                id = Uri.decode(token.substring("id=".length()));
        }
        final String fileUrl = file;
        final String relativePath = id;
        final Uri document = getIntent().getData();
        linkedFilesExecutor.execute(() -> {
            Uri target = null;
            // A file the app may write where it is, or a file in a tree the user allowed the app
            // to write in, opens for editing; else it opens read-only, as its edits could not be
            // saved.
            boolean writable = false;
            if (fileUrl != null && fileUrl.startsWith("file://") && new File(Uri.parse(fileUrl).getPath()).isFile()
                    && !ContentResolver.SCHEME_CONTENT.equals(document != null ? document.getScheme() : null)) {
                target = Uri.parse(fileUrl);
                writable = new File(target.getPath()).canWrite();
            } else if (relativePath != null && document != null) {
                for (Uri tree : LinkedFiles.allowedTrees(getContentResolver())) {
                    target = LinkedFiles.resolve(getContentResolver(), tree, document, relativePath);
                    if (target != null) {
                        writable = LinkedFiles.canWrite(getContentResolver(), tree);
                        break;
                    }
                }
            }
            if (target == null) {
                Log.w(TAG, "opendoc: cannot find " + fileUrl);
                return;
            }
            final Intent intent = new Intent(Intent.ACTION_EDIT, target);
            intent.addFlags(writable
                    ? Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                    : Intent.FLAG_GRANT_READ_URI_PERMISSION);
            intent.setClass(LOActivity.this, LOActivity.class);
            runOnUiThread(() -> startActivity(intent));
        });
    }

    /**
     * @param message The message to test for the prefix
     * @param prefix The prefix to test for
     * @return true if the decoded message starts with the prefix, else false
     */
    private static boolean messageStartsWith(byte[] message, String prefix) {
        byte[] prefixBytes = prefix.getBytes();
        if (message.length < prefixBytes.length)
            return false;
        for (int i = 0; i < prefixBytes.length; i++) {
            if (message[i] != prefixBytes[i]) {
                return false;
            }
        }

        return true;
    }

    /**
     * return true to pass the message to the native part or false to block the message
     */
    private boolean beforeMessageFromWebView(String[] messageAndParam) {
        switch (messageAndParam[0]) {
            case "HULLO":
                mDocumentId = connectDocument(mDocumentId);
                mWebViewClient.setDocumentId(mDocumentId);
                return false;
            case "BYE":
                closeDocumentAndFinish();
                return false;
            case "PRINT":
                getMainHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        LOActivity.this.initiatePrint();
                    }
                });
                return false;
            case "SAVE":
                copyTempBackToIntent();
                sendBroadcast(messageAndParam[0], messageAndParam[1]);
                return false;
            case "downloadas":
                initiateSaveAs(messageAndParam[1]);
                return false;
            case "exportfile":
                initiateExportFile(messageAndParam[1]);
                return false;
            case "DIM_SCREEN": {
                getMainHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    }
                });
                return false;
            }
            case "LIGHT_SCREEN": {
                getMainHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    }
                });
                return false;
            }
            case "MOBILEWIZARD": {
                switch (messageAndParam[1]) {
                    case "show":
                        mMobileWizardVisible = true;
                        break;
                    case "hide":
                        mMobileWizardVisible = false;
                        break;
                }
                return false;
            }
            case "GRANTLINKEDFILES": {
                grantLinkedFiles(messageAndParam.length > 1 ? messageAndParam[1] : "");
                return false;
            }
            case "LINKEDFILESREFRESH": {
                refreshLinkedFiles(messageAndParam.length > 1 ? messageAndParam[1] : "");
                return false;
            }
            case "opendoc": {
                openLinkedDocument(messageAndParam.length > 1 ? messageAndParam[1] : "");
                return false;
            }
            case "HYPERLINK": {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setData(Uri.parse(messageAndParam[1]));
                startActivity(intent);
                return false;
            }
            case "EDITMODE": {
                switch (messageAndParam[1]) {
                    case "on":
                        mIsEditModeActive = true;
                        // prompt for file conversion
                        requestForOdf();
                        break;
                    case "off":
                        mIsEditModeActive = false;
                        break;
                }
                return false;
            }
            case "SLIDESHOW": {
                switch (messageAndParam[1]) {
                    case "on":
                        mSlideShowActive = true;
                        break;
                    case "off":
                        mSlideShowActive = false;
                        break;
                }
                return false;
            }
            case "hideProgressbar": {
                if (mProgressDialog != null)
                    mProgressDialog.dismiss();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (canAllFilesAccessHelp())
                            askForAllFilesAccess();
                    }
                });
                return false;
            }
            case "loadwithpassword": {
                mProgressDialog.determinate(R.string.loading);
                return true;
            }
            case "REQUESTFILECOPY": {
                requestForCopy();
                return false;
            }
        }
        return true;
    }

    public static void createNewFileInputDialog(Activity activity, final String defaultFileName, final @Nullable String mimeType, final int requestCode) {
        Intent i = new Intent(Intent.ACTION_CREATE_DOCUMENT);

        // The mime type and category must be set
        i.setType(mimeType);
        i.addCategory(Intent.CATEGORY_OPENABLE);

        i.putExtra(Intent.EXTRA_TITLE, defaultFileName);

        // Try to default to the Documents folder
        Uri documentsUri = Uri.parse("content://com.android.externalstorage.documents/document/home%3A");
        i.putExtra(DocumentsContract.EXTRA_INITIAL_URI, documentsUri);

        activity.startActivityForResult(i, requestCode);
    }

    private AlertDialog.Builder buildPrompt(final String mTitle, final String mMessage, final String mPositiveBtnText, final String mNegativeBtnText, DialogInterface.OnClickListener callback) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(mTitle);
        if (mMessage.length() > 0)
            builder.setMessage(mMessage);
        builder.setPositiveButton(mPositiveBtnText, callback);
        builder.setNegativeButton(mNegativeBtnText, null);
        builder.setCancelable(false);
        return builder;
    }

    private @Nullable String getMimeType() {
        ContentResolver cR = getContentResolver();

        Uri data = getIntent().getData();
        if (data == null) return null;

        return cR.getType(data);
    }

    private String getFileName(boolean withExtension) {
        Cursor cursor = null;
        String filename = null;
        try {
            cursor = getContentResolver().query(getIntent().getData(), null, null, null, null);
            if (cursor != null && cursor.moveToFirst())
                filename = cursor.getString(cursor.getColumnIndexOrThrow(OpenableColumns.DISPLAY_NAME));
        } catch (Exception e) {
            return null;
        } finally {
            if (cursor != null)
                cursor.close();
        }
        if (filename != null && !withExtension) {
            int dot = filename.lastIndexOf('.');
            if (dot > 0)
                filename = filename.substring(0, dot);
        }
        return filename;
    }

    /**
     * Returns the name to offer in a save dialog for document export. The base name
     * is the display name the content provider reports for the document.
     */
    private String getExportDisplayName(String exportedName) {
        String displayName = getFileName(false);
        if (displayName == null || displayName.isEmpty())
            return exportedName;

        int dot = exportedName.lastIndexOf('.');
        String extension = dot >= 0 ? exportedName.substring(dot) : "";
        return displayName + extension;
    }

    private void requestForCopy() {
        final boolean canBeExported = canDocumentBeExported();
        buildPrompt(getString(R.string.ask_for_copy), "", canBeExported ? getString(R.string.edit_copy) : getString(R.string.use_odf), getString(R.string.view_only), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                if (canBeExported)
                    createNewFileInputDialog(mActivity, getFileName(true), getMimeType(), REQUEST_COPY);
                else {
                    String extension = getOdfExtensionForDocType(getMimeType());
                    createNewFileInputDialog(mActivity, getFileName(false) + "." + extension, getMimeForFormat(extension), REQUEST_COPY);
                }

            }
        }).show();
    }

    // readonly formats here
    private boolean canDocumentBeExported() {
        if (Objects.equals(getMimeType(), "application/vnd.ms-excel.sheet.binary.macroenabled.12")) {
            return false;
        }
        return true;
    }

    private String getOdfExtensionForDocType(@Nullable String mimeType)
    {
        String extTemp = null;
        if (Objects.equals(mimeType, "text/plain")) {
            extTemp = "odt";
        } else if (Objects.equals(mimeType, "text/comma-separated-values")) {
            extTemp = "ods";
        } else if (Objects.equals(mimeType, "application/vnd.ms-excel.sheet.binary.macroenabled.12")) {
            extTemp = "ods";
        }
        return extTemp;
    }

    private void requestForOdf() {
        String extTemp = getOdfExtensionForDocType(getMimeType());
        if (extTemp == null)
            // this means we don't need to request for odf type.
            return;
        final String ext = extTemp;
        buildPrompt(getString(R.string.ask_for_convert_odf), getString(R.string.convert_odf_message), getString(R.string.use_odf), getString(R.string.use_text), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                createNewFileInputDialog(mActivity, getFileName(false) + "." + ext, getMimeForFormat(ext), REQUEST_COPY);
            }
        }).show();
    }

    private void initiateExportFile(String optionsString) {
        // The kit's COKitCallbackType::EXPORT_FILE branch sends "exportfile url=file:///..."
        // for export flows like .uno:ExportToPDF. The file is already at a tmp
        // path; show ACTION_CREATE_DOCUMENT and copy on result.
        Map<String, String> optionsMap = new HashMap<>();
        for (String option : optionsString.split(" ")) {
            String[] keyValue = option.split("=", 2);
            if (keyValue.length == 2)
                optionsMap.put(keyValue[0], keyValue[1]);
        }
        String urlString = optionsMap.get("url");
        if (urlString == null) {
            Log.e(TAG, "exportfile: missing url=");
            return;
        }
        Uri srcUri = Uri.parse(urlString);
        String srcPath = srcUri.getPath();
        if (srcPath == null) {
            Log.e(TAG, "exportfile: bad url=" + urlString);
            return;
        }
        File srcFile = new File(srcPath);
        if (!srcFile.exists()) {
            Log.e(TAG, "exportfile: source file missing: " + srcPath);
            return;
        }

        String filename = getExportDisplayName(srcFile.getName());
        int dot = filename.lastIndexOf('.');
        String extension = dot > 0 ? filename.substring(dot + 1).toLowerCase() : "";
        String mime = getMimeForFormat(extension);
        if (mime == null)
            mime = "application/octet-stream";

        mPendingExportSourcePath = srcPath;

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType(mime);
        intent.putExtra(Intent.EXTRA_TITLE, filename);
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, false);
        File folder = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.fromFile(folder).toString());
        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        startActivityForResult(intent, REQUEST_EXPORT_FILE);
    }

    private void initiateSaveAs(String optionsString) {
        Map<String, String> optionsMap = new HashMap<>();
        String[] options = optionsString.split(" ");
        for (String option : options) {
            String[] keyValue = option.split("=", 2);
            if (keyValue.length == 2)
                optionsMap.put(keyValue[0], keyValue[1]);
        }
        String format = optionsMap.get("format");
        String mime = format != null ? getMimeForFormat(format) : null;
        if (mime == null) {
            Log.e(TAG, "downloadas: no MIME type for format '" + format + "', not exporting");
            return;
        }

        String filename = optionsMap.get("name");
        if (filename == null)
            filename = "document." + format;
        filename = getExportDisplayName(filename);
        int requestID = getRequestIDForFormat(format);

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType(mime);
        intent.putExtra(Intent.EXTRA_TITLE, filename);
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, false);
        File folder = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.fromFile(folder).toString());
        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        startActivityForResult(intent, requestID);
    }

    private int getRequestIDForFormat(String format) {
        switch (format) {
            case "pdf": return REQUEST_SAVEAS_PDF;
            case "rtf": return REQUEST_SAVEAS_RTF;
            case "odt": return REQUEST_SAVEAS_ODT;
            case "odp": return REQUEST_SAVEAS_ODP;
            case "ods": return REQUEST_SAVEAS_ODS;
            case "docx": return REQUEST_SAVEAS_DOCX;
            case "pptx": return REQUEST_SAVEAS_PPTX;
            case "xlsx": return REQUEST_SAVEAS_XLSX;
            case "doc": return REQUEST_SAVEAS_DOC;
            case "ppt": return REQUEST_SAVEAS_PPT;
            case "xls": return REQUEST_SAVEAS_XLS;
            case "epub": return REQUEST_SAVEAS_EPUB;
            case "csv": return REQUEST_SAVEAS_CSV;
            case "html": return REQUEST_SAVEAS_HTML;
            case "md": return REQUEST_SAVEAS_MD;
        }
        return 0;
    }

    private String getMimeForFormat(String format) {
        switch(format) {
            case "pdf": return "application/pdf";
            case "rtf": return "text/rtf";
            case "odt": return "application/vnd.oasis.opendocument.text";
            case "odp": return "application/vnd.oasis.opendocument.presentation";
            case "ods": return "application/vnd.oasis.opendocument.spreadsheet";
            case "docx": return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
            case "pptx": return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
            case "xlsx": return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
            case "doc": return "application/msword";
            case "ppt": return "application/vnd.ms-powerpoint";
            case "xls": return "application/vnd.ms-excel";
            case "epub": return "application/epub+zip";
            case "csv": return "text/csv";
            case "html": return "text/html";
            case "md": return "text/markdown";
        }
        return null;
    }

    private void initiatePrint() {
        PrintManager printManager = (PrintManager) getSystemService(PRINT_SERVICE);
        PrintDocumentAdapter printAdapter = new PrintAdapter(LOActivity.this);
        printManager.print("Document", printAdapter, new PrintAttributes.Builder().build());
    }

    /** Send message back to the shell (for example for the cloud save). */
    public void sendBroadcast(String event, String data) {
        Intent intent = new Intent(LO_ACTIVITY_BROADCAST);
        intent.putExtra(LO_ACTION_EVENT, event);
        intent.putExtra(LO_ACTION_DATA, data);
    }

    public void saveAs(String fileUri, String format, String options) {
        saveAsNative(mDocumentId, fileUri, format, options);
    }

    public void postUnoCommand(String command, String arguments, boolean bNotifyWhenFinished) {
        postUnoCommandNative(mDocumentId, command, arguments, bNotifyWhenFinished);
    }

    private native void saveAsNative(int documentId, String fileUri, String format, String options);

    private native void postUnoCommandNative(int documentId, String command, String arguments, boolean bNotifyWhenFinished);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
