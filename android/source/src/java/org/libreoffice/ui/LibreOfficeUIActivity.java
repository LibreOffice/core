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
import android.database.DataSetObserver;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
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
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.libreoffice.LOAbout;
import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;
import org.libreoffice.SettingsActivity;
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
import java.util.Date;
import java.util.List;

public class LibreOfficeUIActivity extends AppCompatActivity implements ActionBar.OnNavigationListener {
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

    private static final String CURRENT_DIRECTORY_KEY = "CURRENT_DIRECTORY";
    private static final String DOC_PROIVDER_KEY = "CURRENT_DOCUMENT_PROVIDER";
    private static final String FILTER_MODE_KEY = "FILTER_MODE";
    public static final String EXPLORER_VIEW_TYPE_KEY = "EXPLORER_VIEW_TYPE";
    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";
    public static final String SORT_MODE_KEY = "SORT_MODE";

    public static final int GRID_VIEW = 0;
    public static final int LIST_VIEW = 1;

    private DrawerLayout drawerLayout;
    private ListView drawerList;
    private ActionBarDrawerToggle drawerToggle;
    GridView gv;
    ListView lv;

    private final LOAbout mAbout;
    private boolean canQuit = false;

    public LibreOfficeUIActivity() {
        mAbout = new LOAbout(this, true);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // initialize document provider factory
        DocumentProviderFactory.initialize(this);
        documentProviderFactory = DocumentProviderFactory.getInstance();

        PreferenceManager.setDefaultValues(this, R.xml.documentprovider_preferences, false);

        readPreferences();

        // init UI and populate with contents from the provider
        createUI();
        switchToDocumentProvider(documentProviderFactory.getDefaultProvider());
    }

    public void createUI() {

        setContentView(R.layout.activity_document_browser);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayShowTitleEnabled(false); //This should show current directory if anything
        actionBar.setDisplayHomeAsUpEnabled(true);

        //make the navigation spinner
        Context context = actionBar.getThemedContext();
        ArrayAdapter<CharSequence> list = ArrayAdapter.createFromResource(context, R.array.file_view_modes, android.R.layout.simple_spinner_item);
        list.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        actionBar.setListNavigationCallbacks(list, this);

        LinearLayout content = (LinearLayout) findViewById(R.id.browser_main_content);

        if (viewMode == GRID_VIEW) {
            // code to make a grid view
            getLayoutInflater().inflate(R.layout.file_grid, content);
            gv = (GridView)findViewById(R.id.file_explorer_grid_view);
            gv.setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                    open(position);
                }
            });
            actionBar.setSelectedNavigationItem(filterMode + 1);//This triggers the listener which modifies the view.
            registerForContextMenu(gv);
        } else {
            getLayoutInflater().inflate(R.layout.file_list, content);
            lv = (ListView)findViewById(R.id.file_explorer_list_view);
            lv.setClickable(true);
            actionBar.setSelectedNavigationItem(filterMode + 1);
            registerForContextMenu(lv);
        }

        // setup the drawer

        drawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawerList = (ListView) findViewById(R.id.left_drawer);
        drawerToggle = new ActionBarDrawerToggle(this, drawerLayout,
                R.string.document_locations, R.string.close_document_locations) {

            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                supportInvalidateOptionsMenu();
                drawerList.requestFocus(); // Make keypad navigation easier
            }

            @Override
            public void onDrawerClosed(View drawerView) {
                super.onDrawerClosed(drawerView);
                supportInvalidateOptionsMenu();
            }
        };
        drawerToggle.setDrawerIndicatorEnabled(true);
        drawerLayout.setDrawerListener(drawerToggle);
        drawerToggle.syncState();

        // Set the adapter for the list view
        drawerList.setAdapter(new ArrayAdapter<String>(this,
                R.layout.item_in_drawer, documentProviderFactory.getNames()));
        // Set the list's click listener
        drawerList.setOnItemClickListener(new ListView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View view,
                    int position, long id) {
                switchToDocumentProvider(documentProviderFactory
                        .getProvider(position));
            }
        });
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
            gv.setAdapter(new GridItemAdapter(getApplicationContext(),
                    currentDirectory, filePaths));
        } else {
            lv.setAdapter(new ListItemAdapter(getApplicationContext(),
                    filePaths));
        }
        // close drawer if it was open
        drawerLayout.closeDrawer(drawerList);
    }

    @Override
    public void onBackPressed() {
        if (drawerLayout.isDrawerOpen(drawerList)) {
            drawerLayout.closeDrawer(drawerList);
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
        AdapterContextMenuInfo info = (AdapterContextMenuInfo) item
                .getMenuInfo();
        switch (item.getItemId()) {
            case R.id.context_menu_open:
                open(info.position);
                return true;
            case R.id.context_menu_share:
                share(info.position);
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

        MenuItem item = menu.findItem(R.id.menu_view_toggle);
        if (viewMode == GRID_VIEW) {
            item.setTitle(R.string.list_view);
            item.setIcon(R.drawable.light_view_as_list);
        } else {
            item.setTitle(R.string.grid_view);
            item.setIcon(R.drawable.light_view_as_grid);
        }

        item = menu.findItem(R.id.menu_sort_size);
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
            case R.id.menu_view_toggle:
                if (viewMode == GRID_VIEW){
                    viewMode = LIST_VIEW;
                    item.setTitle(R.string.grid_view); // Button points to next view.
                    item.setIcon(R.drawable.light_view_as_grid);
                } else {
                    viewMode = GRID_VIEW;
                    item.setTitle(R.string.list_view); // Button points to next view.
                    item.setIcon(R.drawable.light_view_as_list);
                }
                createUI();
                break;
            case R.id.menu_sort_size:
            case R.id.menu_sort_az:
            case R.id.menu_sort_modified:
                sortFiles(item);
                break;
            case R.id.action_about:
                mAbout.showAbout();
                return true;
            case R.id.action_settings:
                startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
                return true;
            case R.id.menu_storage_preferences:
                startActivity(new Intent(this, DocumentProviderSettingsActivity.class));
                break;

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
        viewMode = prefs.getInt(EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
        sortMode = prefs.getInt(SORT_MODE_KEY, FileUtilities.SORT_AZ);
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
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
    protected void onSaveInstanceState(Bundle outState) {
        // TODO Auto-generated method stub
        super.onSaveInstanceState(outState);
        outState.putString(CURRENT_DIRECTORY_KEY, currentDirectory.getUri().toString());
        outState.putInt(FILTER_MODE_KEY, filterMode);
        outState.putInt(EXPLORER_VIEW_TYPE_KEY , viewMode);
        outState.putInt(DOC_PROIVDER_KEY, documentProvider.getId());

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
                    .getProvider(savedInstanceState.getInt(DOC_PROIVDER_KEY));
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
        //prefs.edit().putInt(EXPLORER_VIEW_TYPE, viewType).commit();
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

    public boolean onNavigationItemSelected(int itemPosition, long itemId) {
        filterMode = itemPosition -1; //bit of a hack, I know. -1 is ALL 0 Docs etc
        openDirectory(currentDirectory);// Uses filter mode
        return true;
    }

    private int dpToPx(int dp){
        final float scale = getApplicationContext().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    class ListItemAdapter implements ListAdapter{
        private Context mContext;
        private List<IFile> filePaths;
        private final long KB = 1024;
        private final long MB = 1048576;

        public ListItemAdapter(Context mContext, List<IFile> filePaths) {
            this.mContext = mContext;
            this.filePaths = filePaths;
        }

        public int getCount() {
            return filePaths != null ? filePaths.size() : 0;
        }

        public Object getItem(int arg0) {
            return null;
        }

        public long getItemId(int arg0) {
            return 0;
        }

        public int getItemViewType(int arg0) {
            return 0;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(
                    Context.LAYOUT_INFLATER_SERVICE);

            View listItem;

            if (convertView == null) {
                listItem = new View(mContext);
                listItem = inflater.inflate(R.layout.file_list_item, null);
            } else {
                listItem = convertView;
            }
            final int pos = position;
            listItem.setClickable(true);
            listItem.setOnClickListener(new OnClickListener() {

                public void onClick(View v) {
                    open(pos);
                }
            });
            listItem.setOnLongClickListener(new OnLongClickListener() {

                @Override
                public boolean onLongClick(View v) {
                    // workaround to show the context menu:
                    // prevent onClickListener from getting this event
                    return false;
                }

            });



            // set value into textview
            TextView filename = (TextView) listItem.findViewById(R.id.file_list_item_name);
            filename.setText(filePaths.get(position).getName());
            //filename.setClickable(true);

            TextView fileSize = (TextView) listItem.findViewById(R.id.file_list_item_size);
            //TODO Give size in KB , MB as appropriate.
            String size = "0B";
            long length = filePaths.get(position).getSize();
            if (length < KB){
                size = Long.toString(length) + "B";
            }
            if (length >= KB && length < MB){
                size = Long.toString(length/KB) + "KB";
            }
            if (length >= MB){
                size = Long.toString(length/MB) + "MB";
            }
            fileSize.setText(size);
            //fileSize.setClickable(true);

            TextView fileDate = (TextView) listItem.findViewById(R.id.file_list_item_date);
            SimpleDateFormat df = new SimpleDateFormat("dd MMM yyyy hh:ss");
            Date date = filePaths.get(position).getLastModified();
            //TODO format date
            fileDate.setText(df.format(date));

            // set image based on selected text
            ImageView imageView = (ImageView) listItem.findViewById(R.id.file_list_item_icon);
            switch (FileUtilities.getType(filePaths.get(position).getName()))
            {
                case FileUtilities.DOC:
                    imageView.setImageResource(R.drawable.writer);
                    break;
                case FileUtilities.CALC:
                    imageView.setImageResource(R.drawable.calc);
                    break;
                case FileUtilities.DRAWING:
                    imageView.setImageResource(R.drawable.draw);
                    break;
                case FileUtilities.IMPRESS:
                    imageView.setImageResource(R.drawable.impress);
                    break;
                default:
                    break;
            }
            if (filePaths.get(position).isDirectory()) {
                //Eventually have thumbnails of each sub file on a black circle
                //For now just a folder icon
                imageView.setImageResource(R.drawable.folder);
            }
            //imageView.setClickable(true);
            return listItem;
        }

        public int getViewTypeCount() {
            return 1;
        }

        public boolean hasStableIds() {
            return false;
        }

        public boolean isEmpty() {
            return false;
        }

        public void registerDataSetObserver(DataSetObserver arg0) {

        }

        public void unregisterDataSetObserver(DataSetObserver arg0) {

        }

        public boolean areAllItemsEnabled() {
            return false;
        }

        public boolean isEnabled(int position) {
            return false;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
