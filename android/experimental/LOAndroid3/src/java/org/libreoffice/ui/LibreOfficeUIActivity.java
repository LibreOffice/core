/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import org.libreoffice.R;
import org.libreoffice.LOAbout;
import org.libreoffice.android.Bootstrap;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.prefs.Preferences;

import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.graphics.Shader.TileMode;

import android.app.ActionBar;
import android.app.ActionBar.OnNavigationListener;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.DataSetObserver;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.webkit.MimeTypeMap;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

public class LibreOfficeUIActivity extends LOAbout implements ActionBar.OnNavigationListener {
    private String tag = "file_manager";
    private SharedPreferences prefs;
    private File homeDirectory;
    private File currentDirectory;
    private int filterMode = FileUtilities.ALL;
    private int viewMode;
    private int sortMode;

    FileFilter fileFilter;
    private File[] filePaths;

    private static final String CURRENT_DIRECTORY_KEY = "CURRENT_DIRECTORY";
    private static final String FILTER_MODE_KEY = "FILTER_MODE";
    public static final String EXPLORER_VIEW_TYPE_KEY = "EXPLORER_VIEW_TYPE";
    public static final String EXPLORER_PREFS_KEY = "EXPLORER_PREFS";
    public static final String SORT_MODE_KEY = "SORT_MODE";

    public static final int GRID_VIEW = 0;
    public static final int LIST_VIEW = 1;

    GridView gv;
    ListView lv;

    public LibreOfficeUIActivity() {
        super(/*newActivity=*/true);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        Log.d(tag, "onCreate - tweaked - meeks !");
        //Set the "home" - top level - directory.
        homeDirectory  = Environment.getExternalStorageDirectory();
        currentDirectory = homeDirectory;
        //Load default settings

        Bootstrap.setup(this);

        Bootstrap.putenv("SAL_LOG=yes");

    }

    public void createUI(){
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowTitleEnabled(false);//This should show current directory if anything
        /*actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
          SpinnerAdapter mSpinnerAdapter = ArrayAdapter.createFromResource(this, R.array.file_view_modes,
          android.R.layout.simple_spinner_dropdown_item);
          actionBar.setListNavigationCallbacks(mSpinnerAdapter, this);
          */
        //make the navigation spinner
        Context context = actionBar.getThemedContext();
        ArrayAdapter<CharSequence> list = ArrayAdapter.createFromResource(context, R.array.file_view_modes, android.R.layout.simple_spinner_item);
        list.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        actionBar.setListNavigationCallbacks(list, this);

        //make striped actionbar
        BitmapDrawable bg = (BitmapDrawable)getResources().getDrawable(R.drawable.bg_striped);
        bg.setTileModeXY(TileMode.REPEAT, TileMode.REPEAT);
        actionBar.setBackgroundDrawable(bg);

        BitmapDrawable bgSplit = (BitmapDrawable)getResources().getDrawable(R.drawable.bg_striped_split_img);
        bgSplit.setTileModeXY(TileMode.REPEAT, TileMode.REPEAT);
        actionBar.setSplitBackgroundDrawable(bgSplit);

        if( !currentDirectory.equals( homeDirectory )){
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        if( viewMode == GRID_VIEW){
            // code to make a grid view
            setContentView(R.layout.file_grid);
            gv = (GridView)findViewById(R.id.file_explorer_grid_view);
            filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
            gv.setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                    open(position);
                }
            });
            gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory, filePaths ) );
            actionBar.setSelectedNavigationItem( filterMode + 1 );//This triggers the listener which modifies the view.
            registerForContextMenu(gv);
        }else{
            setContentView(R.layout.file_list);
            lv = (ListView)findViewById( R.id.file_explorer_list_view);
            lv.setClickable(true);
            filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
            lv.setAdapter( new ListItemAdapter(getApplicationContext(), filePaths) );
            actionBar.setSelectedNavigationItem( filterMode + 1 );
            registerForContextMenu(lv);
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

    public void openDirectory(File dir ){
        currentDirectory = dir;
        if( !currentDirectory.equals( homeDirectory )){
            ActionBar actionBar = getActionBar();
            actionBar.setDisplayHomeAsUpEnabled(true);
        }else{
            ActionBar actionBar = getActionBar();
            actionBar.setDisplayHomeAsUpEnabled( false );
        }
        filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
        FileUtilities.sortFiles( filePaths, sortMode );
        /*
           for( int i = 0; i < fileNames.length; i++){
           fileNames[ i ] = filePaths[ i ].getName();
           if( !FileUtilities.hasThumbnail( filePaths[ i ] ) )
           {
           new ThumbnailGenerator( filePaths[ i ] );
           }
           }
           */
        if( viewMode == GRID_VIEW){
            gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory, filePaths ) );
        }else{
            lv.setAdapter( new ListItemAdapter(getApplicationContext(), filePaths) );
        }
    }

    public void open(File file) {
        Intent i = new Intent(Intent.ACTION_VIEW, Uri.fromFile(file));
        i.setComponent(new ComponentName(
                    "org.libreoffice",
                    "org.libreoffice.LibreOfficeMainActivity"));
        startActivity(i);
    }

    private void open(int position) {
        File file = filePaths[position];
        if (!file.isDirectory()) {
            open(file);
        } else {
            openDirectory(file);
        }
    }

    private void share(int position) {
        File file = filePaths[position];
        Intent sharingIntent = new Intent(android.content.Intent.ACTION_SEND);
        Uri uri = Uri.fromFile(file);
        String extension = MimeTypeMap.getFileExtensionFromUrl(uri.toString());
        sharingIntent.setType(MimeTypeMap.getSingleton()
                .getMimeTypeFromExtension(extension));
        sharingIntent.putExtra(android.content.Intent.EXTRA_STREAM, uri);
        sharingIntent.putExtra(android.content.Intent.EXTRA_SUBJECT,
                file.getName());
        startActivity(Intent.createChooser(sharingIntent,
                getString(R.string.share_via)));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.view_menu, menu);

        MenuItem item = (MenuItem)menu.findItem(R.id.menu_view_toggle);
        if( viewMode == GRID_VIEW){
            item.setTitle(R.string.list_view);
            item.setIcon( R.drawable.light_view_as_list );
        }else{
            item.setTitle(R.string.grid_view);
            item.setIcon( R.drawable.light_view_as_grid );
        }
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                if( !currentDirectory.equals( homeDirectory ) ){
                    openDirectory( currentDirectory.getParentFile() );
                }
                break;
            case R.id.menu_view_toggle:
                if( viewMode == GRID_VIEW){
                    viewMode = LIST_VIEW;
                    item.setTitle(R.string.grid_view);//Button points to next view.
                    item.setIcon( R.drawable.light_view_as_grid );

                }else{
                    viewMode = GRID_VIEW;
                    item.setTitle(R.string.list_view);//Button points to next view.
                    item.setIcon( R.drawable.light_view_as_list );
                }
                createUI();
                break;
            case R.id.menu_sort_size:
            case R.id.menu_sort_az:
            case R.id.menu_sort_modified:
                sortFiles(item);
                break;
            case R.id.action_about:
                showAbout();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    @SuppressWarnings("unused")//see android:onClick properties in view_menu.xml
    public void sortFiles(MenuItem item){
        switch ( item.getItemId() ) {
            case R.id.menu_sort_az:
                if( sortMode == FileUtilities.SORT_AZ ){
                    sortMode = FileUtilities.SORT_ZA;
                }else{
                    sortMode = FileUtilities.SORT_AZ;
                }
                break;
            case R.id.menu_sort_modified:
                if( sortMode == FileUtilities.SORT_NEWEST ){
                    sortMode = FileUtilities.SORT_OLDEST;
                }else{
                    sortMode = FileUtilities.SORT_NEWEST;
                }
                break;
            case R.id.menu_sort_size:
                if( sortMode == FileUtilities.SORT_LARGEST ){
                    sortMode = FileUtilities.SORT_SMALLEST;
                }else{
                    sortMode = FileUtilities.SORT_LARGEST;
                }
                break;
            default:
                break;
        }
        this.onResume();
    }

    public void readPreferences(){
        prefs = getSharedPreferences(EXPLORER_PREFS_KEY, MODE_PRIVATE);
        viewMode = prefs.getInt( EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
        sortMode = prefs.getInt( SORT_MODE_KEY, FileUtilities.SORT_AZ );
        SharedPreferences defaultPrefs = PreferenceManager.getDefaultSharedPreferences( getBaseContext() );
        filterMode = Integer.valueOf( defaultPrefs.getString( FILTER_MODE_KEY , "-1") );
        sortMode = Integer.valueOf( defaultPrefs.getString( SORT_MODE_KEY , "-1") );
    }

    public void editPreferences(MenuItem item){
        startActivity( new Intent( this , PreferenceEditor.class) );
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        // TODO Auto-generated method stub
        super.onSaveInstanceState(outState);
        outState.putString( CURRENT_DIRECTORY_KEY , currentDirectory.getAbsolutePath() );
        outState.putInt( FILTER_MODE_KEY , filterMode );
        outState.putInt( EXPLORER_VIEW_TYPE_KEY , viewMode );

        Log.d(tag, currentDirectory.toString() + Integer.toString(filterMode ) + Integer.toString(viewMode) );
        //prefs.edit().putInt(EXPLORER_VIEW_TYPE, viewType).commit();
        Log.d(tag, "savedInstanceSate");
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onRestoreInstanceState(savedInstanceState);
        if( savedInstanceState.isEmpty() ){
            return;
        }
        currentDirectory = new File( savedInstanceState.getString( CURRENT_DIRECTORY_KEY ) );
        filterMode = savedInstanceState.getInt( FILTER_MODE_KEY , FileUtilities.ALL ) ;
        viewMode = savedInstanceState.getInt( EXPLORER_VIEW_TYPE_KEY , GRID_VIEW );
        //openDirectory( currentDirectory );
        Log.d(tag, "onRestoreInstanceState");
        Log.d(tag, currentDirectory.toString() + Integer.toString(filterMode ) + Integer.toString(viewMode) );
    }

    @Override
    protected void onPause() {
        //prefs.edit().putInt(EXPLORER_VIEW_TYPE, viewType).commit();
        super.onPause();
        Log.d(tag, "onPause");
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        Log.d(tag, "onResume");
        readPreferences();// intent values take precedence over prefs?
        Intent i = this.getIntent();
        if( i.hasExtra( CURRENT_DIRECTORY_KEY ) ){
            currentDirectory = new File( i.getStringExtra( CURRENT_DIRECTORY_KEY ) );
            Log.d(tag, CURRENT_DIRECTORY_KEY);
        }
        if( i.hasExtra( FILTER_MODE_KEY ) ){
            filterMode = i.getIntExtra( FILTER_MODE_KEY, FileUtilities.ALL);
            Log.d(tag, FILTER_MODE_KEY);
        }
        if( i.hasExtra( EXPLORER_VIEW_TYPE_KEY ) ){
            viewMode = i.getIntExtra( EXPLORER_VIEW_TYPE_KEY, GRID_VIEW);
            Log.d(tag, EXPLORER_VIEW_TYPE_KEY);
        }
        createUI();
        openDirectory( currentDirectory );
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        Log.d(tag, "onStart");
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        Log.d(tag, "onStop");
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();

        Log.d(tag, "onDestroy");
    }

    public boolean onNavigationItemSelected(int itemPosition, long itemId) {
        filterMode = itemPosition -1; //bit of a hack, I know. -1 is ALL 0 Docs etc
        openDirectory( currentDirectory );// Uses filter mode
        return true;
    }

    private int dpToPx( int dp ){
        final float scale = getApplicationContext().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    class ListItemAdapter implements ListAdapter{
        private Context mContext;
        private File[] filePaths;
        private final long KB = 1024;
        private final long MB = 1048576;

        public ListItemAdapter(Context mContext, File[] filePaths) {
            this.mContext = mContext;
            this.filePaths = filePaths;
        }

        public int getCount() {
            // TODO Auto-generated method stub
            return filePaths.length;
        }

        public Object getItem(int arg0) {
            // TODO Auto-generated method stub
            return null;
        }

        public long getItemId(int arg0) {
            // TODO Auto-generated method stub
            return 0;
        }

        public int getItemViewType(int arg0) {
            // TODO Auto-generated method stub
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
                listItem = (View) convertView;
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
            filename.setText( filePaths[ position ].getName() );
            //filename.setClickable(true);

            TextView fileSize = (TextView) listItem.findViewById(R.id.file_list_item_size);
            //TODO Give size in KB , MB as appropriate.
            String size = "0B";
            long length = filePaths[ position ].length();
            if( length < KB ){
                size = Long.toString( length ) + "B";
            }
            if( length >= KB && length < MB){
                size = Long.toString( length/KB ) + "KB";
            }
            if( length >= MB){
                size = Long.toString( length/MB ) + "MB";
            }
            fileSize.setText( size );
            //fileSize.setClickable(true);

            TextView fileDate = (TextView) listItem.findViewById(R.id.file_list_item_date);
            SimpleDateFormat df = new SimpleDateFormat("dd MMM yyyy hh:ss");
            Date date = new Date( filePaths[ position ].lastModified() );
            //TODO format date
            fileDate.setText( df.format( date ) );

            // set image based on selected text
            ImageView imageView = (ImageView) listItem.findViewById(R.id.file_list_item_icon);
            switch (FileUtilities.getType(filePaths[position].getName()))
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
            if( filePaths[position].isDirectory() ){
                //Eventually have thumbnails of each sub file on a black circle
                //For now just a folder icon
                imageView.setImageResource(R.drawable.folder);
            }
            //imageView.setClickable(true);
            return listItem;
        }

        public int getViewTypeCount() {
            // TODO Auto-generated method stub
            return 1;
        }

        public boolean hasStableIds() {
            // TODO Auto-generated method stub
            return false;
        }

        public boolean isEmpty() {
            // TODO Auto-generated method stub
            return false;
        }

        public void registerDataSetObserver(DataSetObserver arg0) {
            // TODO Auto-generated method stub

        }

        public void unregisterDataSetObserver(DataSetObserver arg0) {
            // TODO Auto-generated method stub

        }

        public boolean areAllItemsEnabled() {
            // TODO Auto-generated method stub
            return false;
        }

        public boolean isEnabled(int position) {
            // TODO Auto-generated method stub
            return false;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
