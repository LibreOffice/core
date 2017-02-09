/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.content.ContextCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.AppCompatSpinner;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
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
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.libreoffice.AboutDialogFragment;
import org.libreoffice.LibreOfficeMainActivity;
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

public class LibreOfficeUIActivity extends AppCompatActivity implements SettingsListenerModel.OnSettingsPreferenceChangedListener{
    private String LOGTAG = LibreOfficeUIActivity.class.getSimpleName();
    private SharedPreferences prefs;
    private int filterMode = FileUtilities.ALL;
    private int viewMode;
    private int sortMode;

    FileFilter fileFilter;
    FilenameFilter filenameFilter;
    private List<IFile> filePaths;
    private DocumentProviderFactory documentProviderFactory;
    private IDocumentProvider documentProvider;
    private IFile homeDirectory;
    private IFile currentDirectory;
    private int currentlySelectedFile;

    private static final String CURRENT_DIRECTORY_KEY = "CURRENT_DIRECTORY";
    private static final String DOC_PROVIDER_KEY = "CURRENT_DOCUMENT_PROVIDER";
    private static final String FILTER_MODE_KEY = "FILTER_MODE";
    public static final String EXPLORER_VIEW_TYPE_KEY = "EXPLORER_VIEW_TYPE";
    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";
    public static final String SORT_MODE_KEY = "SORT_MODE";
    private static final String RECENT_DOCUMENTS_KEY = "RECENT_DOCUMENTS";

    public static final int GRID_VIEW = 0;
    public static final int LIST_VIEW = 1;

    private DrawerLayout drawerLayout;
    private NavigationView navigationDrawer;
    private ActionBarDrawerToggle drawerToggle;
    private RecyclerView fileRecyclerView;
    private RecyclerView recentRecyclerView;

    private boolean canQuit = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // initialize document provider factory
        DocumentProviderFactory.initialize(this);
        documentProviderFactory = DocumentProviderFactory.getInstance();

        PreferenceManager.setDefaultValues(this, R.xml.documentprovider_preferences, false);
        readPreferences();
        SettingsListenerModel.getInstance().setListener(this);
        // init UI and populate with contents from the provider
        switchToDocumentProvider(documentProviderFactory.getDefaultProvider());
        createUI();
    }

    public void createUI() {

        setContentView(R.layout.activity_document_browser);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();

        if (actionBar != null) {
            actionBar.setDisplayShowTitleEnabled(false);
            actionBar.setDisplayHomeAsUpEnabled(true);

            //make the navigation spinner
            Context context = actionBar.getThemedContext();
            AppCompatSpinner toolbarSpinner = (AppCompatSpinner) findViewById(R.id.toolbar_spinner);
            ArrayAdapter<CharSequence> spinnerAdapter = ArrayAdapter.createFromResource(context, R.array.file_view_modes, android.R.layout.simple_spinner_item);
            spinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            toolbarSpinner.setAdapter(spinnerAdapter);
            toolbarSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> adapterView, View view, int pos, long id) {
                    filterMode = pos -1; //bit of a hack, I know. -1 is ALL 0 Docs etc
                    openDirectory(currentDirectory);// Uses filter mode
                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {
                    filterMode = FileUtilities.ALL;
                    openDirectory(currentDirectory);
                }
            });
        }

        recentRecyclerView = (RecyclerView) findViewById(R.id.list_recent);

        Set<String> recentFileStrings = prefs.getStringSet(RECENT_DOCUMENTS_KEY, new HashSet<String>());

        final ArrayList<IFile> recentFiles = new ArrayList<IFile>();
        for (String recentFileString : recentFileStrings) {
            try {
                recentFiles.add(documentProvider.createFromUri(new URI(recentFileString)));
            } catch (URISyntaxException e) {
                e.printStackTrace();
            }
        }

        recentRecyclerView.setLayoutManager(new GridLayoutManager(this, 2));
        recentRecyclerView.setAdapter(new RecentFilesAdapter(this, recentFiles));

        fileRecyclerView = (RecyclerView) findViewById(R.id.file_recycler_view);
        //This should be tested because it possibly disables view recycling
        fileRecyclerView.setNestedScrollingEnabled(false);
        openDirectory(currentDirectory);
        registerForContextMenu(fileRecyclerView);

        //Setting up navigation drawer
        drawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        navigationDrawer = (NavigationView) findViewById(R.id.navigation_drawer);
        /*
         * These are the currently-known document providers (for which icons are assigned).
         * This is to ensure that there is an icon available if the provider is recognized, while
         * the unrecognized ones still appear, but without an icon. If there is a document provider
         * not on this list, it should be added and an icon assigned to it, in the if-else ladder
         * bellow. This is a hacky implementation, maybe we could make something better in the
         * future, i.e. we could move this into the menu file and load it that way.
         */
        final String LOCAL_DOCUMENTS_NAME = "Local documents";
        final String LOCAL_FILE_SYSTEM_NAME = "Local file system";
        final String EXTERNAL_SD_NAME = "External SD";
        final String OTG_FILE_SYSTEM_NAME = "OTG device (experimental)";
        final String OWNCLOUD_NAME = "Remote server";

        //Provider names are wrapped as a ArrayList so indexOf(Object) method could be used
        final ArrayList<CharSequence> providerNames = new ArrayList<CharSequence>(
                Arrays.asList(documentProviderFactory.getNames())
        );
        for (CharSequence name : providerNames) {
            int iconRes = 0;
            if (name.equals(LOCAL_DOCUMENTS_NAME)) {
                iconRes = R.drawable.ic_insert_drive_file_black_24dp;
            } else if (name.equals(LOCAL_FILE_SYSTEM_NAME)) {
                iconRes = R.drawable.ic_storage_black_24dp;
            } else if (name.equals(EXTERNAL_SD_NAME)) {
                iconRes = R.drawable.ic_sd_card_black_24dp;
            } else if (name.equals(OTG_FILE_SYSTEM_NAME)) {
                iconRes = R.drawable.ic_usb_black_24dp;
            } else if (name.equals(OWNCLOUD_NAME)) {
                iconRes = R.drawable.ic_cloud_black_24dp;
            }
            MenuItem item = navigationDrawer.getMenu().add(R.id.group_providers, Menu.NONE, Menu.NONE, name)
                            .setCheckable(true);
            if (iconRes != 0) {
                item.setIcon(iconRes);
            }
        }

        final Context context = this; //needed for anonymous method below
        navigationDrawer.setNavigationItemSelectedListener(new NavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem item) {
                if (item.getItemId() == R.id.menu_storage_preferences) {
                    startActivity(new Intent(context, DocumentProviderSettingsActivity.class));
                    return true;
                }
                int position = providerNames.indexOf(item.getTitle());
                switchToDocumentProvider(documentProviderFactory.getProvider(position));
                return true;
            }
        });
        drawerToggle = new ActionBarDrawerToggle(this, drawerLayout,
                R.string.document_locations, R.string.close_document_locations) {

            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                supportInvalidateOptionsMenu();
                navigationDrawer.requestFocus(); // Make keypad navigation easier
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
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        drawerToggle.syncState();
    }

    private void refreshView() {
        // enable home icon as "up" if required
        if (!currentDirectory.equals(homeDirectory)) {
            drawerToggle.setDrawerIndicatorEnabled(false);
        } else {
            drawerToggle.setDrawerIndicatorEnabled(true);
        }

        FileUtilities.sortFiles(filePaths, sortMode);
        // refresh view
        if (viewMode == GRID_VIEW) {
            fileRecyclerView.setLayoutManager(new GridLayoutManager(this, 3));
            fileRecyclerView.setAdapter(new GridItemAdapter(this, filePaths));
        } else {
            fileRecyclerView.setLayoutManager(new LinearLayoutManager(this));
            fileRecyclerView.setAdapter(new ListItemAdapter(this, filePaths));
        }
        // close drawer if it was open
        drawerLayout.closeDrawer(navigationDrawer);
    }

    @Override
    public void onBackPressed() {
        if (drawerLayout.isDrawerOpen(navigationDrawer)) {
            drawerLayout.closeDrawer(navigationDrawer);
        } else if (!currentDirectory.equals(homeDirectory)) {
            // navigate upwards in directory hierarchy
            openParentDirectory();
        } else {
            // only exit if warning has been shown
            if (canQuit) {
                super.onBackPressed();
                return;
            }

            // show warning about leaving the app and set a timer
            Toast.makeText(this, R.string.back_again_to_quit,
                    Toast.LENGTH_SHORT).show();
            canQuit = true;
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    canQuit = false;
                }
            }, 3000);
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

    private void switchToDocumentProvider(IDocumentProvider provider) {

        new AsyncTask<IDocumentProvider, Void, Void>() {
            @Override
            protected Void doInBackground(IDocumentProvider... provider) {
                // switch document provider:
                // these operations may imply network access and must be run in
                // a different thread
                try {
                    documentProvider = provider[0];
                    homeDirectory = documentProvider.getRootDirectory();
                    currentDirectory = homeDirectory;
                    filePaths = currentDirectory.listFiles(FileUtilities
                            .getFileFilter(filterMode));
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
        }.execute(provider);
    }

    public void openDirectory(IFile dir) {
        if (dir == null)
            return;

        //show recent files if in home directory
        if (dir.equals(homeDirectory)) {
            recentRecyclerView.setVisibility(View.VISIBLE);
            findViewById(R.id.header_browser).setVisibility((View.VISIBLE));
            findViewById(R.id.header_recents).setVisibility((View.VISIBLE));

        } else {
            recentRecyclerView.setVisibility(View.GONE);
            findViewById(R.id.header_browser).setVisibility((View.GONE));
            findViewById(R.id.header_recents).setVisibility((View.GONE));
        }

        new AsyncTask<IFile, Void, Void>() {
            @Override
            protected Void doInBackground(IFile... dir) {
                // get list of files:
                // this operation may imply network access and must be run in
                // a different thread
                currentDirectory = dir[0];
                try {
                    filePaths = currentDirectory.listFiles(FileUtilities
                            .getFileFilter(filterMode));
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
                return currentDirectory.getParent();
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

        MenuItem item = menu.findItem(R.id.menu_sort_size);
        if (sortMode == FileUtilities.SORT_LARGEST) {
            item.setTitle(R.string.sort_smallest);
        } else {
            item.setTitle(R.string.sort_largest);
        }

        item = menu.findItem(R.id.menu_sort_az);
        if (sortMode == FileUtilities.SORT_AZ) {
            item.setTitle(R.string.sort_za);
        } else {
            item.setTitle(R.string.sort_az);
        }

        item = menu.findItem(R.id.menu_sort_modified);
        if (sortMode == FileUtilities.SORT_NEWEST) {
            item.setTitle(R.string.sort_oldest);
        } else {
            item.setTitle(R.string.sort_newest);
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
            case R.id.menu_sort_size:
            case R.id.menu_sort_az:
            case R.id.menu_sort_modified:
                sortFiles(item);
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

    public void sortFiles(MenuItem item){
        switch (item.getItemId()) {
            case R.id.menu_sort_az:
                if (sortMode == FileUtilities.SORT_AZ){
                    sortMode = FileUtilities.SORT_ZA;
                    item.setTitle(R.string.sort_az);
                } else {
                    sortMode = FileUtilities.SORT_AZ;
                    item.setTitle(R.string.sort_za);
                }
                break;
            case R.id.menu_sort_modified:
                if (sortMode == FileUtilities.SORT_NEWEST){
                    sortMode = FileUtilities.SORT_OLDEST;
                    item.setTitle(R.string.sort_newest);
                } else {
                    sortMode = FileUtilities.SORT_NEWEST;
                    item.setTitle(R.string.sort_oldest);
                }
                break;
            case R.id.menu_sort_size:
                if (sortMode == FileUtilities.SORT_LARGEST){
                    sortMode = FileUtilities.SORT_SMALLEST;
                    item.setTitle(R.string.sort_largest);
                } else {
                    sortMode = FileUtilities.SORT_LARGEST;
                    item.setTitle(R.string.sort_smallest);
                }
                break;
            default:
                break;
        }
        this.onResume();
    }

    public void readPreferences(){
        prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        sortMode = prefs.getInt(SORT_MODE_KEY, FileUtilities.SORT_AZ);
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        viewMode = Integer.valueOf(defaultPrefs.getString(EXPLORER_VIEW_TYPE_KEY, ""+ GRID_VIEW));
        filterMode = Integer.valueOf(defaultPrefs.getString(FILTER_MODE_KEY , "-1"));

        Intent i = this.getIntent();
        if (i.hasExtra(CURRENT_DIRECTORY_KEY)) {
            try {
                currentDirectory = documentProvider.createFromUri(new URI(
                        i.getStringExtra(CURRENT_DIRECTORY_KEY)));
            } catch (URISyntaxException e) {
                currentDirectory = documentProvider.getRootDirectory();
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
        outState.putString(CURRENT_DIRECTORY_KEY, currentDirectory.getUri().toString());
        outState.putInt(FILTER_MODE_KEY, filterMode);
        outState.putInt(EXPLORER_VIEW_TYPE_KEY , viewMode);
        outState.putInt(DOC_PROVIDER_KEY, documentProvider.getId());

        Log.d(LOGTAG, currentDirectory.toString() + Integer.toString(filterMode) + Integer.toString(viewMode));
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
            currentDirectory = documentProvider.createFromUri(new URI(
                    savedInstanceState.getString(CURRENT_DIRECTORY_KEY)));
        } catch (URISyntaxException e) {
            currentDirectory = documentProvider.getRootDirectory();
        }
        filterMode = savedInstanceState.getInt(FILTER_MODE_KEY, FileUtilities.ALL);
        viewMode = savedInstanceState.getInt(EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
        //openDirectory(currentDirectory);
        Log.d(LOGTAG, "onRestoreInstanceState");
        Log.d(LOGTAG, currentDirectory.toString() + Integer.toString(filterMode) + Integer.toString(viewMode));
    }

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


    class ListItemAdapter extends RecyclerView.Adapter<ListItemAdapter.ViewHolder> {

        private Activity mActivity;
        private List<IFile> filePaths;
        private final long KB = 1024;
        private final long MB = 1048576;

        ListItemAdapter(Activity activity, List<IFile> filePaths) {
            this.mActivity = activity;
            this.filePaths = filePaths;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View item = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.file_list_item, parent, false);
            return new ViewHolder(item);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, final int position) {
            final IFile file = filePaths.get(position);

            holder.itemView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    open(position);
                }
            });
            holder.itemView.setOnLongClickListener(new OnLongClickListener() {

                @Override
                public boolean onLongClick(View view) {
                    //to be picked out by floating context menu (workaround-ish)
                    currentlySelectedFile = position;
                    //must return false so the click is not consumed
                    return false;
                }
            });

            holder.filenameView.setText(file.getName());

            if (!file.isDirectory()) {
                String size;
                long length = filePaths.get(position).getSize();
                if (length < KB){
                    size = Long.toString(length) + "B";
                } else if (length < MB){
                    size = Long.toString(length/KB) + "KB";
                } else {
                    size = Long.toString(length/MB) + "MB";
                }
                holder.fileSizeView.setText(size);
            }
            SimpleDateFormat df = new SimpleDateFormat("dd MMM yyyy hh:ss");
            Date date = file.getLastModified();
            //TODO format date
            holder.fileDateView.setText(df.format(date));

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
                holder.iconView.setColorFilter(ContextCompat.getColor(mActivity,R.color.text_color_secondary));
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
                filenameView = (TextView) itemView.findViewById(R.id.file_list_item_name);
                fileSizeView = (TextView) itemView.findViewById(R.id.file_list_item_size);
                fileDateView = (TextView) itemView.findViewById(R.id.file_list_item_date);
                iconView = (ImageView) itemView.findViewById(R.id.file_list_item_icon);
            }
        }
    }

    class GridItemAdapter extends RecyclerView.Adapter<GridItemAdapter.ViewHolder> {

        private Activity mActivity;
        private List<IFile> filePaths;

        GridItemAdapter(Activity mActivity, List<IFile> filePaths) {
            this.mActivity = mActivity;
            this.filePaths = filePaths;
        }

        @Override
        public GridItemAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View item = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.file_explorer_grid_item, parent, false);
            return new ViewHolder(item);
        }

        @Override
        public void onBindViewHolder(final GridItemAdapter.ViewHolder holder, final int position) {
            final IFile file = filePaths.get(position);

            holder.itemView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    open(position);
                }
            });

            holder.itemView.setOnLongClickListener(new OnLongClickListener() {
                @Override
                public boolean onLongClick(View view) {
                    //to be picked out by floating context menu (workaround-ish)
                    currentlySelectedFile = position;
                    //must return false so the click is not consumed
                    return false;
                }
            });

            String filename = file.getName();

            holder.filenameView.setText(filename);

            switch (FileUtilities.getType(filename)) {
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
                holder.iconView.setColorFilter(ContextCompat.getColor(mActivity,R.color.text_color_secondary));
            }

        }

        @Override
        public int getItemCount() {
            return filePaths.size();
        }

        class ViewHolder extends RecyclerView.ViewHolder {

            View itemView;
            TextView filenameView;
            ImageView iconView;

            ViewHolder(View itemView) {
                super(itemView);
                this.itemView = itemView;
                this.filenameView = (TextView) itemView.findViewById(R.id.grid_item_label);
                this.iconView = (ImageView) itemView.findViewById(R.id.grid_item_image);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
