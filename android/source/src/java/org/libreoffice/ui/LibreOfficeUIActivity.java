/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.view.ViewCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.text.InputType;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.OvershootInterpolator;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.libreoffice.AboutDialogFragment;
import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.LocaleHelper;
import org.libreoffice.R;
import org.libreoffice.SettingsActivity;
import org.libreoffice.SettingsListenerModel;
import org.libreoffice.storage.DocumentProviderFactory;
import org.libreoffice.storage.DocumentProviderSettingsActivity;
import org.libreoffice.storage.IDocumentProvider;
import org.libreoffice.storage.IFile;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.net.URI;
import java.net.URISyntaxException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class LibreOfficeUIActivity extends AppCompatActivity implements SettingsListenerModel.OnSettingsPreferenceChangedListener, View.OnClickListener{
    public enum DocumentType {
        WRITER,
        CALC,
        IMPRESS,
        DRAW,
        INVALID
    }

    private String LOGTAG = LibreOfficeUIActivity.class.getSimpleName();
    private SharedPreferences prefs;
    private int filterMode = FileUtilities.ALL;
    private int viewMode;
    private int sortMode;
    private boolean showHiddenFiles;
    private String displayLanguage;

    // dynamic permissions IDs
    private static final int PERMISSION_WRITE_EXTERNAL_STORAGE = 0;

    FileFilter fileFilter;
    FilenameFilter filenameFilter;
    private List<IFile> filePaths = new ArrayList<IFile>();
    private DocumentProviderFactory documentProviderFactory;
    private IDocumentProvider documentProvider;
    private IFile homeDirectory;
    private IFile currentDirectory;
    private int currentlySelectedFile;

    private DocumentType newDocType = DocumentType.INVALID;

    private static final String CURRENT_DIRECTORY_KEY = "CURRENT_DIRECTORY";
    private static final String DOC_PROVIDER_KEY = "CURRENT_DOCUMENT_PROVIDER";
    private static final String FILTER_MODE_KEY = "FILTER_MODE";
    public static final String EXPLORER_VIEW_TYPE_KEY = "EXPLORER_VIEW_TYPE";
    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";
    public static final String SORT_MODE_KEY = "SORT_MODE";
    private static final String RECENT_DOCUMENTS_KEY = "RECENT_DOCUMENTS";
    private static final String ENABLE_SHOW_HIDDEN_FILES_KEY = "ENABLE_SHOW_HIDDEN_FILES";
    private static final String DISPLAY_LANGUAGE = "DISPLAY_LANGUAGE";

    public static final String NEW_DOC_TYPE_KEY = "NEW_DOC_TYPE_KEY";
    public static final String NEW_WRITER_STRING_KEY = "private:factory/swriter";
    public static final String NEW_IMPRESS_STRING_KEY = "private:factory/simpress";
    public static final String NEW_CALC_STRING_KEY = "private:factory/scalc";
    public static final String NEW_DRAW_STRING_KEY = "private:factory/sdraw";

    // keep this in sync with 'AndroidManifext.xml'
    private static final String[] SUPPORTED_MIME_TYPES = {
            "application/vnd.oasis.opendocument.text",
            "application/vnd.oasis.opendocument.graphics",
            "application/vnd.oasis.opendocument.presentation",
            "application/vnd.oasis.opendocument.spreadsheet",
            "application/vnd.oasis.opendocument.text-flat-xml",
            "application/vnd.oasis.opendocument.graphics-flat-xml",
            "application/vnd.oasis.opendocument.presentation-flat-xml",
            "application/vnd.oasis.opendocument.spreadsheet-flat-xml",
            "application/vnd.oasis.opendocument.text-template",
            "application/vnd.oasis.opendocument.spreadsheet-template",
            "application/vnd.oasis.opendocument.graphics-template",
            "application/vnd.oasis.opendocument.presentation-template",
            "application/rtf",
            "text/rtf",
            "application/msword",
            "application/vnd.ms-powerpoint",
            "application/vnd.ms-excel",
            "application/vnd.visio",
            "application/vnd.visio.xml",
            "application/x-mspublisher",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            "application/vnd.openxmlformats-officedocument.presentationml.presentation",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.template",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.template",
            "application/vnd.openxmlformats-officedocument.presentationml.template",
            "text/csv",
            "text/comma-separated-values",
            "application/vnd.ms-works",
            "application/vnd.apple.keynote",
            "application/x-abiword",
            "application/x-pagemaker",
            "image/x-emf",
            "image/x-svm",
            "image/x-wmf",
            "image/svg+xml",
    };

    public static final int GRID_VIEW = 0;
    public static final int LIST_VIEW = 1;

    private static final int REQUEST_CODE_OPEN_FILECHOOSER = 12345;
    private static final int REQUEST_CODE_CREATE_NEW_DOCUMENT = 12346;

    private DrawerLayout drawerLayout;
    private NavigationView navigationDrawer;
    private ActionBar actionBar;
    private ActionBarDrawerToggle drawerToggle;
    private RecyclerView fileRecyclerView;
    private RecyclerView recentRecyclerView;

    private Animation fabOpenAnimation;
    private Animation fabCloseAnimation;
    private boolean isFabMenuOpen = false;
    private FloatingActionButton editFAB;
    private FloatingActionButton writerFAB;
    private FloatingActionButton drawFAB;
    private FloatingActionButton impressFAB;
    private FloatingActionButton calcFAB;
    private LinearLayout drawLayout;
    private LinearLayout writerLayout;
    private LinearLayout impressLayout;
    private LinearLayout calcLayout;
    private LinearLayout systemFilePickerLayout;
    private TextView openFileView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // initialize document provider factory
        DocumentProviderFactory.initialize(this);
        documentProviderFactory = DocumentProviderFactory.getInstance();

        PreferenceManager.setDefaultValues(this, R.xml.documentprovider_preferences, false);
        readPreferences();
        SettingsListenerModel.getInstance().setListener(this);
        // Registering the USB detect broadcast receiver
        IntentFilter filter = new IntentFilter();
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(mUSBReceiver, filter);
        // init UI and populate with contents from the provider


        createUI();
        fabOpenAnimation = AnimationUtils.loadAnimation(this, R.anim.fab_open);
        fabCloseAnimation = AnimationUtils.loadAnimation(this, R.anim.fab_close);
    }

    @Override
    protected void attachBaseContext(Context newBase) {
        super.attachBaseContext(LocaleHelper.onAttach(newBase));
    }

    public void createUI() {

        setContentView(R.layout.activity_document_browser);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        actionBar = getSupportActionBar();

        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        editFAB = findViewById(R.id.editFAB);
        editFAB.setOnClickListener(this);
        impressFAB = findViewById(R.id.newImpressFAB);
        impressFAB.setOnClickListener(this);
        writerFAB = findViewById(R.id.newWriterFAB);
        writerFAB.setOnClickListener(this);
        calcFAB = findViewById(R.id.newCalcFAB);
        calcFAB.setOnClickListener(this);
        drawFAB = findViewById(R.id.newDrawFAB);
        drawFAB.setOnClickListener(this);
        writerLayout = findViewById(R.id.writerLayout);
        impressLayout = findViewById(R.id.impressLayout);
        calcLayout = findViewById(R.id.calcLayout);
        drawLayout = findViewById(R.id.drawLayout);
        openFileView = findViewById(R.id.open_file_view);
        openFileView.setOnClickListener(this);

        recentRecyclerView = findViewById(R.id.list_recent);

        Set<String> recentFileStrings = prefs.getStringSet(RECENT_DOCUMENTS_KEY, new HashSet<String>());

        final ArrayList<IFile> recentFiles = new ArrayList<IFile>();
        for (String recentFileString : recentFileStrings) {
            try {
                if(documentProvider != null)
                    recentFiles.add(documentProvider.createFromUri(this, new URI(recentFileString)));
            } catch (URISyntaxException e) {
                e.printStackTrace();
            } catch (RuntimeException e){
                e.printStackTrace();
            }
        }

        recentRecyclerView.setLayoutManager(new GridLayoutManager(this, 2));
        recentRecyclerView.setAdapter(new RecentFilesAdapter(this, recentFiles));

        fileRecyclerView = findViewById(R.id.file_recycler_view);
        systemFilePickerLayout = findViewById(R.id.system_file_picker_layout);
        //This should be tested because it possibly disables view recycling
        fileRecyclerView.setNestedScrollingEnabled(false);
        openDirectory(currentDirectory);
        registerForContextMenu(fileRecyclerView);

        //Setting up navigation drawer
        drawerLayout = findViewById(R.id.drawer_layout);
        navigationDrawer = findViewById(R.id.navigation_drawer);

        final ArrayList<CharSequence> providerNames = new ArrayList<CharSequence>(
                Arrays.asList(documentProviderFactory.getNames())
        );

        // Loop through the document providers menu items and check if they are available or not
        for (int index = 0; index < providerNames.size(); index++) {
            MenuItem item = navigationDrawer.getMenu().getItem(index);
            item.setEnabled(documentProviderFactory.getProvider(index).checkProviderAvailability(this));
        }

        navigationDrawer.setNavigationItemSelectedListener(new NavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem item) {

                switch (item.getItemId()) {
                    case R.id.menu_storage_preferences: {
                        startActivity(new Intent(LibreOfficeUIActivity.this, DocumentProviderSettingsActivity.class));
                        return true;
                    }

                    case R.id.menu_provider_documents: {
                        switchToDocumentProvider(documentProviderFactory.getProvider(0));
                        return true;
                    }

                    case R.id.menu_provider_filesystem: {
                        switchToDocumentProvider(documentProviderFactory.getProvider(1));
                        return true;
                    }

                    case R.id.menu_provider_extsd: {
                        switchToDocumentProvider(documentProviderFactory.getProvider(2));
                        return true;
                    }

                    case R.id.menu_provider_otg: {
                        switchToDocumentProvider(documentProviderFactory.getProvider(3));
                        return true;
                    }

                    case R.id.menu_system_file_dialog: {
                        switchToSystemFileDialogLayout();
                        return true;
                    }

                    default:
                        return false;
                }


            }
        });
        drawerToggle = new ActionBarDrawerToggle(this, drawerLayout,
                R.string.document_locations, R.string.close_document_locations) {

            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                supportInvalidateOptionsMenu();
                navigationDrawer.requestFocus(); // Make keypad navigation easier
                if (isFabMenuOpen) {
                    collapseFabMenu(); //Collapse FAB Menu when drawer is opened
                }
            }

            @Override
            public void onDrawerClosed(View drawerView) {
                super.onDrawerClosed(drawerView);
                supportInvalidateOptionsMenu();
            }
        };
        drawerToggle.setDrawerIndicatorEnabled(true);
        drawerLayout.addDrawerListener(drawerToggle);
        drawerToggle.syncState();

        // initially show layout with item to open system file picker
        switchToSystemFileDialogLayout();
    }

    private void expandFabMenu() {
        ViewCompat.animate(editFAB).rotation(45.0F).withLayer().setDuration(300).setInterpolator(new OvershootInterpolator(10.0F)).start();
        drawLayout.startAnimation(fabOpenAnimation);
        impressLayout.startAnimation(fabOpenAnimation);
        writerLayout.startAnimation(fabOpenAnimation);
        calcLayout.startAnimation(fabOpenAnimation);
        writerFAB.setClickable(true);
        impressFAB.setClickable(true);
        drawFAB.setClickable(true);
        calcFAB.setClickable(true);
        isFabMenuOpen = true;
    }

    private void collapseFabMenu() {
        ViewCompat.animate(editFAB).rotation(0.0F).withLayer().setDuration(300).setInterpolator(new OvershootInterpolator(10.0F)).start();
        writerLayout.startAnimation(fabCloseAnimation);
        impressLayout.startAnimation(fabCloseAnimation);
        drawLayout.startAnimation(fabCloseAnimation);
        calcLayout.startAnimation(fabCloseAnimation);
        writerFAB.setClickable(false);
        impressFAB.setClickable(false);
        drawFAB.setClickable(false);
        calcFAB.setClickable(false);
        isFabMenuOpen = false;
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        drawerToggle.syncState();
    }

    private void refreshView() {
        // enable home icon as "up" if required
        if (currentDirectory != null && homeDirectory != null && !currentDirectory.equals(homeDirectory)) {
            drawerToggle.setDrawerIndicatorEnabled(false);
        } else {
            drawerToggle.setDrawerIndicatorEnabled(true);
        }

        FileUtilities.sortFiles(filePaths, sortMode);
        // refresh view
        fileRecyclerView.setLayoutManager(isViewModeList() ? new LinearLayoutManager(this) : new GridLayoutManager(this, 3));
        fileRecyclerView.setAdapter(new ExplorerItemAdapter(this, filePaths));
        // close drawer if it was open
        drawerLayout.closeDrawer(navigationDrawer);
        if (isFabMenuOpen) {
            collapseFabMenu();
        }
    }

    @Override
    public void onBackPressed() {
        if (drawerLayout.isDrawerOpen(navigationDrawer)) {
            drawerLayout.closeDrawer(navigationDrawer);
            if (isFabMenuOpen) {
                collapseFabMenu();
            }
        } else if (currentDirectory != null && homeDirectory != null && !currentDirectory.equals(homeDirectory)) {
            // navigate upwards in directory hierarchy
            openParentDirectory();
        } else if (isFabMenuOpen) {
            collapseFabMenu();
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.context_menu, menu);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.context_menu_open:
                open(currentlySelectedFile);
                return true;
            case R.id.context_menu_share:
                share(currentlySelectedFile);
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

    private boolean isViewModeList(){
        return viewMode == LIST_VIEW;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE_OPEN_FILECHOOSER && resultCode == RESULT_OK) {
            final Uri fileUri = data.getData();

            // "forward" to LibreOfficeMainActivity to open the file
            Intent intent = new Intent(Intent.ACTION_VIEW, fileUri);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            String packageName = getApplicationContext().getPackageName();
            ComponentName componentName = new ComponentName(packageName,
                    LibreOfficeMainActivity.class.getName());
            intent.setComponent(componentName);
            startActivity(intent);
        } else if (requestCode == REQUEST_CODE_CREATE_NEW_DOCUMENT) {
            // "forward" to LibreOfficeMainActivity to create + open the file
            final Uri fileUri = data.getData();
            loadNewDocument(newDocType, fileUri);
        }
    }

    private void showSystemFilePickerAndOpenFile() {
        Intent intent = new Intent();
        try {
            intent.setAction(Intent.ACTION_OPEN_DOCUMENT);
        } catch (ActivityNotFoundException exception) {
            // Intent.ACTION_OPEN_DOCUMENT added in API level 19, but minSdkVersion is currently 16
            intent.setAction(Intent.ACTION_GET_CONTENT);
        }

        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_MIME_TYPES, SUPPORTED_MIME_TYPES);;

        if (intent.resolveActivity(getPackageManager()) != null) {
            startActivityForResult(intent, REQUEST_CODE_OPEN_FILECHOOSER);
        }
    }


    private void switchToDocumentProvider(IDocumentProvider provider) {
        fileRecyclerView.setVisibility(View.VISIBLE);
        systemFilePickerLayout.setVisibility(View.GONE);

        new AsyncTask<IDocumentProvider, Void, Void>() {
            @Override
            protected Void doInBackground(IDocumentProvider... provider) {
                // switch document provider:
                // these operations may imply network access and must be run in
                // a different thread
                try {
                    homeDirectory = provider[0].getRootDirectory(LibreOfficeUIActivity.this);
                    List<IFile> paths = homeDirectory.listFiles(FileUtilities
                            .getFileFilter(filterMode));
                    filePaths = new ArrayList<IFile>();
                    for(IFile file: paths) {
                        if(showHiddenFiles){
                            filePaths.add(file);
                        } else {
                            if(!file.getName().startsWith(".")){
                                filePaths.add(file);
                            }
                        }
                    }
                }
                catch (final RuntimeException e) {
                    final Activity activity = LibreOfficeUIActivity.this;
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(activity, e.getMessage(),
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    startActivity(new Intent(activity, DocumentProviderSettingsActivity.class));
                    Log.e(LOGTAG, "failed to switch document provider "+ e.getMessage(), e.getCause());
                    return null;
                }
                //no exception
                documentProvider = provider[0];
                currentDirectory = homeDirectory;
                return null;
            }

            @Override
            protected void onPostExecute(Void result) {
                refreshView();
            }
        }.execute(provider);
    }

    private void switchToSystemFileDialogLayout() {
        fileRecyclerView.setVisibility(View.GONE);
        findViewById(R.id.text_directory_path).setVisibility(View.GONE);
        systemFilePickerLayout.setVisibility(View.VISIBLE);
        refreshView();
    }

    public void openDirectory(IFile dir) {
        if (dir == null)
            return;

        //show recent files if in home directory
        if (dir.equals(homeDirectory)) {
            recentRecyclerView.setVisibility(View.VISIBLE);
            findViewById(R.id.header_browser).setVisibility((View.VISIBLE));
            findViewById(R.id.header_recents).setVisibility((View.VISIBLE));
            actionBar.setTitle(R.string.app_name);
            findViewById(R.id.text_directory_path).setVisibility(View.GONE);
        } else {
            recentRecyclerView.setVisibility(View.GONE);
            findViewById(R.id.header_browser).setVisibility((View.GONE));
            findViewById(R.id.header_recents).setVisibility((View.GONE));
            actionBar.setTitle(dir.getName());
            findViewById(R.id.text_directory_path).setVisibility(View.VISIBLE);
            ((TextView)findViewById(R.id.text_directory_path)).setText(getString(R.string.current_dir,
                    dir.getUri().getPath()));
        }

        new AsyncTask<IFile, Void, Void>() {
            @Override
            protected Void doInBackground(IFile... dir) {
                // get list of files:
                // this operation may imply network access and must be run in
                // a different thread
                currentDirectory = dir[0];
                try {
                    List<IFile> paths = currentDirectory.listFiles(FileUtilities
                            .getFileFilter(filterMode));
                    filePaths = new ArrayList<IFile>();
                    for(IFile file: paths) {
                        if(showHiddenFiles){
                            filePaths.add(file);
                        } else {
                            if(!file.getName().startsWith(".")){
                                filePaths.add(file);
                            }
                        }
                    }
                }
                catch (final RuntimeException e) {
                    final Activity activity = LibreOfficeUIActivity.this;
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
            protected void onPostExecute(Void result) {
                refreshView();
            }
        }.execute(dir);
    }

    public void open(final IFile document) {
        addDocumentToRecents(document);
        new AsyncTask<IFile, Void, File>() {
            @Override
            protected File doInBackground(IFile... document) {
                // this operation may imply network access and must be run in
                // a different thread
                try {
                    return document[0].getDocument();
                }
                catch (final RuntimeException e) {
                    final Activity activity = LibreOfficeUIActivity.this;
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(activity, e.getMessage(),
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    Log.e(LOGTAG, e.getMessage(), e.getCause());
                    return null;
                }
            }

            @Override
            protected void onPostExecute(File file) {
                if (file != null) {
                    Intent i = new Intent(Intent.ACTION_VIEW, Uri.fromFile(file));
                    String packageName = getApplicationContext().getPackageName();
                    ComponentName componentName = new ComponentName(packageName,
                            LibreOfficeMainActivity.class.getName());
                    i.setComponent(componentName);

                    // these extras allow to rebuild the IFile object in LOMainActivity
                    i.putExtra("org.libreoffice.document_provider_id",
                            documentProvider.getId());
                    i.putExtra("org.libreoffice.document_uri",
                            document.getUri());

                    startActivity(i);
                }
            }
        }.execute(document);
    }

    private void createNewFileDialog() {
        final String extension;
        if (newDocType == DocumentType.WRITER) {
            extension = FileUtilities.DEFAULT_WRITER_EXTENSION;
        } else if (newDocType == DocumentType.CALC) {
            extension = FileUtilities.DEFAULT_SPREADSHEET_EXTENSION;
        } else if (newDocType == DocumentType.IMPRESS) {
            extension = FileUtilities.DEFAULT_IMPRESS_EXTENSION;
        } else if (newDocType == DocumentType.DRAW) {
            extension = FileUtilities.DEFAULT_DRAWING_EXTENSION;
        } else {
            Log.e(LOGTAG, "Invalid document type passed.");
            return;
        }

        String defaultFileName = getString(R.string.default_document_name) + extension;
        String mimeType = FileUtilities.getMimeType(defaultFileName);

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(mimeType);
        intent.putExtra(Intent.EXTRA_TITLE, defaultFileName);

        startActivityForResult(intent, REQUEST_CODE_CREATE_NEW_DOCUMENT);
    }

    private void loadNewDocument(DocumentType docType, Uri newFileUri) {
        final String newDocumentType;
        if (docType == DocumentType.WRITER) {
            newDocumentType = NEW_WRITER_STRING_KEY;
        } else if (docType == DocumentType.CALC) {
            newDocumentType = NEW_CALC_STRING_KEY;
        } else if (docType == DocumentType.IMPRESS) {
            newDocumentType = NEW_IMPRESS_STRING_KEY;
        } else if (docType == DocumentType.DRAW) {
            newDocumentType = NEW_DRAW_STRING_KEY;
        } else {
            Log.w(LOGTAG, "invalid document type passed to loadNewDocument method. Ignoring request");
            return;
        }

        Intent intent = new Intent(LibreOfficeUIActivity.this, LibreOfficeMainActivity.class);
        intent.putExtra(NEW_DOC_TYPE_KEY, newDocumentType);
        intent.setData(newFileUri);
        startActivity(intent);
    }

    private void open(int position) {
        IFile file = filePaths.get(position);
        if (!file.isDirectory()) {
            open(file);
        } else {
            openDirectory(file);
        }
    }

    private void openParentDirectory() {
        new AsyncTask<Void, Void, IFile>() {
            @Override
            protected IFile doInBackground(Void... dir) {
                // this operation may imply network access and must be run in
                // a different thread
                return currentDirectory.getParent(LibreOfficeUIActivity.this);
            }

            @Override
            protected void onPostExecute(IFile result) {
                openDirectory(result);
            }
        }.execute();
    }

    private void share(int position) {

        new AsyncTask<IFile, Void, File>() {
            @Override
            protected File doInBackground(IFile... document) {
                // this operation may imply network access and must be run in
                // a different thread
                try {
                    return document[0].getDocument();
                } catch (final RuntimeException e) {
                    final Activity activity = LibreOfficeUIActivity.this;
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(activity, e.getMessage(),
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    Log.e(LOGTAG, e.getMessage(), e.getCause());
                    return null;
                }
            }

            @Override
            protected void onPostExecute(File file) {
                if (file != null) {
                    Intent sharingIntent = new Intent(android.content.Intent.ACTION_SEND);
                    Uri uri = Uri.fromFile(file);
                    sharingIntent.setType(FileUtilities.getMimeType(file.getName()));
                    sharingIntent.putExtra(android.content.Intent.EXTRA_STREAM, uri);
                    sharingIntent.putExtra(android.content.Intent.EXTRA_SUBJECT,
                            file.getName());
                    startActivity(Intent.createChooser(sharingIntent,
                            getString(R.string.share_via)));
                }
            }
        }.execute(filePaths.get(position));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.view_menu, menu);

        switch (sortMode) {
            case FileUtilities.SORT_SMALLEST: {
                menu.findItem(R.id.menu_sort_size_asc).setChecked(true);
            }
            break;

            case FileUtilities.SORT_LARGEST: {
                menu.findItem(R.id.menu_sort_size_desc).setChecked(true);
            }
            break;

            case FileUtilities.SORT_AZ: {
                menu.findItem(R.id.menu_sort_az).setChecked(true);
            }
            break;

            case FileUtilities.SORT_ZA: {
                menu.findItem(R.id.menu_sort_za).setChecked(true);
            }
            break;

            case FileUtilities.SORT_NEWEST: {
                menu.findItem(R.id.menu_sort_modified_newest).setChecked(true);
            }
            break;

            case FileUtilities.SORT_OLDEST: {
                menu.findItem(R.id.menu_sort_modified_oldest).setChecked(true);
            }
            break;
        }

        switch (filterMode) {
            case FileUtilities.ALL:
                menu.findItem(R.id.menu_filter_everything).setChecked(true);
                break;

            case FileUtilities.DOC:
                menu.findItem(R.id.menu_filter_documents).setChecked(true);
                break;

            case FileUtilities.CALC:
                menu.findItem(R.id.menu_filter_presentations).setChecked(true);
                break;

            case FileUtilities.IMPRESS:
                menu.findItem(R.id.menu_filter_presentations).setChecked(true);
                break;

            case FileUtilities.DRAWING:
                menu.findItem(R.id.menu_filter_drawings).setChecked(true);
                break;
        }

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Will close the drawer if the home button is pressed
        if (drawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        switch (item.getItemId()) {
            case android.R.id.home:
                if (!currentDirectory.equals(homeDirectory)){
                    openParentDirectory();
                }
                break;

            case R.id.menu_filter_everything:
                item.setChecked(true);
                filterMode = FileUtilities.ALL;
                openDirectory(currentDirectory);
                break;

            case R.id.menu_filter_documents:
                item.setChecked(true);
                filterMode = FileUtilities.DOC;
                openDirectory(currentDirectory);
                break;

            case R.id.menu_filter_spreadsheets:
                item.setChecked(true);
                filterMode = FileUtilities.CALC;
                openDirectory(currentDirectory);
                break;

            case R.id.menu_filter_presentations:
                item.setChecked(true);
                filterMode = FileUtilities.IMPRESS;
                openDirectory(currentDirectory);
                break;

            case R.id.menu_filter_drawings:
                item.setChecked(true);
                filterMode = FileUtilities.DRAWING;
                openDirectory(currentDirectory);
                break;

            case R.id.menu_sort_size_asc: {
                sortMode = FileUtilities.SORT_SMALLEST;
                this.onResume();
            }
            break;

            case R.id.menu_sort_size_desc: {
                sortMode = FileUtilities.SORT_LARGEST;
                this.onResume();
            }
            break;

            case R.id.menu_sort_az: {
                sortMode = FileUtilities.SORT_AZ;
                this.onResume();
            }
            break;

            case R.id.menu_sort_za: {
                sortMode = FileUtilities.SORT_ZA;
                this.onResume();
            }
            break;

            case R.id.menu_sort_modified_newest: {
                sortMode = FileUtilities.SORT_NEWEST;
                this.onResume();
            }
            break;

            case R.id.menu_sort_modified_oldest: {
                sortMode = FileUtilities.SORT_OLDEST;
                this.onResume();
            }
            break;

            case R.id.action_about: {
                AboutDialogFragment aboutDialogFragment = new AboutDialogFragment();
                aboutDialogFragment.show(getSupportFragmentManager(), "AboutDialogFragment");
            }
                return true;
            case R.id.action_settings:
                startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    public void readPreferences(){
        prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        sortMode = prefs.getInt(SORT_MODE_KEY, FileUtilities.SORT_AZ);
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        viewMode = Integer.valueOf(defaultPrefs.getString(EXPLORER_VIEW_TYPE_KEY, ""+ GRID_VIEW));
        filterMode = Integer.valueOf(defaultPrefs.getString(FILTER_MODE_KEY , "-1"));
        showHiddenFiles = defaultPrefs.getBoolean(ENABLE_SHOW_HIDDEN_FILES_KEY, false);
        displayLanguage = defaultPrefs.getString(DISPLAY_LANGUAGE, LocaleHelper.SYSTEM_DEFAULT_LANGUAGE);

        Intent i = this.getIntent();
        if (i.hasExtra(CURRENT_DIRECTORY_KEY)) {
            try {
                currentDirectory = documentProvider.createFromUri(this, new URI(
                        i.getStringExtra(CURRENT_DIRECTORY_KEY)));
            } catch (URISyntaxException e) {
                currentDirectory = documentProvider.getRootDirectory(this);
            }
            Log.d(LOGTAG, CURRENT_DIRECTORY_KEY);
        }

        if (i.hasExtra(FILTER_MODE_KEY)) {
            filterMode = i.getIntExtra( FILTER_MODE_KEY, FileUtilities.ALL);
            Log.d(LOGTAG, FILTER_MODE_KEY);
        }

        if (i.hasExtra(EXPLORER_VIEW_TYPE_KEY)) {
            viewMode = i.getIntExtra( EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
            Log.d(LOGTAG, EXPLORER_VIEW_TYPE_KEY);
        }

        LocaleHelper.setLocale(this, displayLanguage);
    }

    @Override
    public void settingsPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        readPreferences();
        refreshView();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        // TODO Auto-generated method stub
        super.onSaveInstanceState(outState);

        if(currentDirectory != null) {
            outState.putString(CURRENT_DIRECTORY_KEY, currentDirectory.getUri().toString());
            Log.d(LOGTAG, currentDirectory.toString() + Integer.toString(filterMode) + Integer.toString(viewMode));
        }
        outState.putInt(FILTER_MODE_KEY, filterMode);
        outState.putInt(EXPLORER_VIEW_TYPE_KEY , viewMode);
        if(documentProvider != null)
            outState.putInt(DOC_PROVIDER_KEY, documentProvider.getId());

        outState.putBoolean(ENABLE_SHOW_HIDDEN_FILES_KEY , showHiddenFiles);

        //prefs.edit().putInt(EXPLORER_VIEW_TYPE, viewType).commit();
        Log.d(LOGTAG, "savedInstanceState");
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onRestoreInstanceState(savedInstanceState);
        if (savedInstanceState.isEmpty()){
            return;
        }
        if (documentProvider == null) {
            Log.d(LOGTAG, "onRestoreInstanceState - documentProvider is null");
            documentProvider = DocumentProviderFactory.getInstance()
                    .getProvider(savedInstanceState.getInt(DOC_PROVIDER_KEY));
        }
        try {
            currentDirectory = documentProvider.createFromUri(this, new URI(
                    savedInstanceState.getString(CURRENT_DIRECTORY_KEY)));
        } catch (URISyntaxException e) {
            currentDirectory = documentProvider.getRootDirectory(this);
        }
        filterMode = savedInstanceState.getInt(FILTER_MODE_KEY, FileUtilities.ALL);
        viewMode = savedInstanceState.getInt(EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
        showHiddenFiles = savedInstanceState.getBoolean(ENABLE_SHOW_HIDDEN_FILES_KEY, false);
        //openDirectory(currentDirectory);
        Log.d(LOGTAG, "onRestoreInstanceState");
        Log.d(LOGTAG, currentDirectory.toString() + Integer.toString(filterMode) + Integer.toString(viewMode));
    }

    private final BroadcastReceiver mUSBReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                Toast.makeText(context, R.string.usb_connected_configure, Toast.LENGTH_SHORT).show();
                startActivity(new Intent(context, DocumentProviderSettingsActivity.class));
                Log.d(LOGTAG, "USB device attached");
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                Log.d(LOGTAG, "USB device detached");
            }
        }
    };
    @Override
    protected void onPause() {
        super.onPause();
        Log.d(LOGTAG, "onPause");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(LOGTAG, "onResume");
        Log.d(LOGTAG, "sortMode="+ sortMode + " filterMode=" + filterMode);
        createUI();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.i(LOGTAG, "no permission to read external storage - asking for permission");
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    PERMISSION_WRITE_EXTERNAL_STORAGE);
        } else {
            switchToDocumentProvider(documentProviderFactory.getDefaultProvider());
            setEditFABVisibility(View.VISIBLE);
        }
        Log.d(LOGTAG, "onStart");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(LOGTAG, "onStop");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mUSBReceiver);
        Log.d(LOGTAG, "onDestroy");
    }

    private int dpToPx(int dp){
        final float scale = getApplicationContext().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    private void addDocumentToRecents(IFile iFile) {
        String newRecent = iFile.getUri().toString();
        Set<String> recentsSet = prefs.getStringSet(RECENT_DOCUMENTS_KEY, new HashSet<String>());

        //create array to work with
        ArrayList<String> recentsArrayList = new ArrayList<String>(recentsSet);

        //remove string if present, so that it doesn't appear multiple times
        recentsSet.remove(newRecent);

        //put the new value in the first place
        recentsArrayList.add(0, newRecent);


        /*
         * 4 because the number of recommended items in App Shortcuts is 4, and also
         * because it's a good number of recent items in general
         */
        final int RECENTS_SIZE = 4;

        while (recentsArrayList.size() > RECENTS_SIZE) {
            recentsArrayList.remove(RECENTS_SIZE);
        }

        //switch to Set, so that it could be inserted into prefs
        recentsSet = new HashSet<String>(recentsArrayList);

        prefs.edit().putStringSet(RECENT_DOCUMENTS_KEY, recentsSet).apply();


        //update app shortcuts (7.0 and above)
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N_MR1) {
            ShortcutManager shortcutManager = getSystemService(ShortcutManager.class);

            //Remove all shortcuts, and apply new ones.
            shortcutManager.removeAllDynamicShortcuts();

            ArrayList<ShortcutInfo> shortcuts = new ArrayList<ShortcutInfo>();
            for (String pathString : recentsArrayList) {

                //find the appropriate drawable
                int drawable = 0;
                switch (FileUtilities.getType(pathString)) {
                    case FileUtilities.DOC:
                        drawable = R.drawable.writer;
                        break;
                    case FileUtilities.CALC:
                        drawable = R.drawable.calc;
                        break;
                    case FileUtilities.DRAWING:
                        drawable = R.drawable.draw;
                        break;
                    case FileUtilities.IMPRESS:
                        drawable = R.drawable.impress;
                        break;
                }

                File file = new File(pathString);

                //for some reason, getName uses %20 instead of space
                String filename = file.getName().replace("%20", " ");

                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.fromFile(file));
                String packageName = this.getApplicationContext().getPackageName();
                ComponentName componentName = new ComponentName(packageName, LibreOfficeMainActivity.class.getName());
                intent.setComponent(componentName);

                ShortcutInfo shortcut = new ShortcutInfo.Builder(this, filename)
                        .setShortLabel(filename)
                        .setLongLabel(filename)
                        .setIcon(Icon.createWithResource(this, drawable))
                        .setIntent(intent)
                        .build();

                shortcuts.add(shortcut);
            }
            shortcutManager.setDynamicShortcuts(shortcuts);
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id){
            case R.id.editFAB:
                // Intent.ACTION_CREATE_DOCUMENT, used in 'createNewFileDialog' requires SDK version 19
                if (Build.VERSION.SDK_INT < 19) {
                    Toast.makeText(this,
                        getString(R.string.creating_new_files_not_supported), Toast.LENGTH_SHORT).show();
                    return;
                }
                if (isFabMenuOpen) {
                    collapseFabMenu();
                } else {
                    expandFabMenu();
                }
                break;
            case R.id.open_file_view:
                showSystemFilePickerAndOpenFile();
                break;
            case R.id.newWriterFAB:
                newDocType = DocumentType.WRITER;
                createNewFileDialog();
                break;
            case R.id.newImpressFAB:
                newDocType = DocumentType.IMPRESS;
                createNewFileDialog();
                break;
            case R.id.newCalcFAB:
                newDocType = DocumentType.CALC;
                createNewFileDialog();
                break;
            case R.id.newDrawFAB:
                newDocType = DocumentType.DRAW;
                createNewFileDialog();
                break;
        }
    }


    class ExplorerItemAdapter extends RecyclerView.Adapter<ExplorerItemAdapter.ViewHolder> {

        private Activity mActivity;
        private List<IFile> filePaths;
        private final long KB = 1024;
        private final long MB = 1048576;

        ExplorerItemAdapter(Activity activity, List<IFile> filePaths) {
            this.mActivity = activity;
            this.filePaths = filePaths;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View item = LayoutInflater.from(parent.getContext())
                    .inflate(isViewModeList() ? R.layout.file_list_item : R.layout.file_explorer_grid_item, parent, false);
            return new ViewHolder(item);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, final int position) {
            final IFile file = filePaths.get(position);

            holder.itemView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    open(holder.getAdapterPosition());
                }
            });
            holder.itemView.setOnLongClickListener(new OnLongClickListener() {

                @Override
                public boolean onLongClick(View view) {
                    //to be picked out by floating context menu (workaround-ish)
                    currentlySelectedFile = holder.getAdapterPosition();
                    //must return false so the click is not consumed
                    return false;
                }
            });

            holder.filenameView.setText(file.getName());
            switch (FileUtilities.getType(file.getName())) {
                case FileUtilities.DOC:
                    holder.iconView.setImageResource(R.drawable.writer);
                    break;
                case FileUtilities.CALC:
                    holder.iconView.setImageResource(R.drawable.calc);
                    break;
                case FileUtilities.DRAWING:
                    holder.iconView.setImageResource(R.drawable.draw);
                    break;
                case FileUtilities.IMPRESS:
                    holder.iconView.setImageResource(R.drawable.impress);
                    break;
            }

            if (file.isDirectory()) {
                //Eventually have thumbnails of each sub file on a black circle
                //For now just a folder icon
                holder.iconView.setImageResource(R.drawable.ic_folder_black_24dp);
                holder.iconView.setColorFilter(ContextCompat.getColor(mActivity, R.color.text_color_secondary));
            }

            // Date and Size field only exist when we are displaying items in a list.
            if(isViewModeList()) {
                if (!file.isDirectory()) {
                    String size;
                    long length = filePaths.get(position).getSize();
                    if (length < KB) {
                        size = Long.toString(length) + "B";
                    } else if (length < MB) {
                        size = Long.toString(length / KB) + "KB";
                    } else {
                        size = Long.toString(length / MB) + "MB";
                    }
                    holder.fileSizeView.setText(size);
                }
                SimpleDateFormat df = new SimpleDateFormat("dd MMM yyyy hh:ss");
                Date date = file.getLastModified();
                //TODO format date
                holder.fileDateView.setText(df.format(date));
            }
        }

        @Override
        public int getItemCount() {
            return filePaths.size();
        }

        class ViewHolder extends RecyclerView.ViewHolder {

            View itemView;
            TextView filenameView, fileSizeView, fileDateView;
            ImageView iconView;

            ViewHolder(View itemView) {
                super(itemView);
                this.itemView = itemView;
                filenameView = itemView.findViewById(R.id.file_item_name);
                iconView = itemView.findViewById(R.id.file_item_icon);
                // Check if view mode is List, only then initialise Size and Date field
                if (isViewModeList()) {
                    fileSizeView = itemView.findViewById(R.id.file_item_size);
                    fileDateView = itemView.findViewById(R.id.file_item_date);
                }
            }
        }
    }

    private void setEditFABVisibility(final int visibility){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                editFAB.setVisibility(visibility);
            }
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch(requestCode){
            case PERMISSION_WRITE_EXTERNAL_STORAGE:
                if(permissions.length>0 && grantResults[0] == PackageManager.PERMISSION_GRANTED){
                    switchToDocumentProvider(documentProviderFactory.getDefaultProvider());
                    setEditFABVisibility(View.VISIBLE);
                } else {
                    setEditFABVisibility(View.INVISIBLE);
                }
                break;
                default:
                    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
