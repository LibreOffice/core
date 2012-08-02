/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import org.libreoffice.R;
import org.libreoffice.android.DocumentLoader;
import org.libreoffice.android.Bootstrap;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.prefs.Preferences;

//import android.app.ActionBar;
//import android.view.Menu;
//import android.view.MenuInflater;
//import android.view.MenuItem;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import com.actionbarsherlock.view.MenuInflater;
import com.actionbarsherlock.app.SherlockActivity;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
import android.graphics.Shader.TileMode;

import android.app.ActionBar.OnNavigationListener;
import android.app.Activity;
import android.os.AsyncTask;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.DataSetObserver;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

import com.sun.star.awt.Size;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XToolkit2;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.view.XRenderable;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class LibreOfficeUIActivity extends SherlockActivity implements ActionBar.OnNavigationListener {
    private String tag = "file_manager";
	private SharedPreferences prefs;
    private File homeDirectory;
	private File currentDirectory;
	private int filterMode = FileUtilities.ALL;
	private int viewMode;
	private int sortMode;
	
	FileFilter fileFilter;
	FilenameFilter filenameFilter;
	private String[] fileNames;
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
	
    private static final String TAG = "ThumbnailGenerator";
    private static final int SMALLSIZE = 128;

    long timingOverhead;
    XComponentContext context;
    XMultiComponentFactory mcf;
    XComponentLoader componentLoader;
    XToolkit2 toolkit;
    XDevice dummySmallDevice;
    Object doc;
    int pageCount;
    int currentPage;
    XRenderable renderable;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        Log.d(tag, "onCreate - tweaked - meeks !");
        //Set the "home" - top level - directory.
        homeDirectory  = new File(Environment.getExternalStorageDirectory(),"LibreOffice");
        homeDirectory.mkdirs();
        currentDirectory = homeDirectory;        
        //Load default settings
        

    }
    
    public void createUI(){
        ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayShowTitleEnabled(false);//This should show current directory if anything
        /*actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        SpinnerAdapter mSpinnerAdapter = ArrayAdapter.createFromResource(this, R.array.file_view_modes,
                android.R.layout.simple_spinner_dropdown_item);
        actionBar.setListNavigationCallbacks(mSpinnerAdapter, this);
        */
        //make the navigation spinner
        Context context = getSupportActionBar().getThemedContext();
        ArrayAdapter<CharSequence> list = ArrayAdapter.createFromResource(context, R.array.file_view_modes, R.layout.sherlock_spinner_item);
        list.setDropDownViewResource(R.layout.sherlock_spinner_dropdown_item);

        getSupportActionBar().setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        getSupportActionBar().setListNavigationCallbacks(list, this);

        //make striped actionbar
        BitmapDrawable bg = (BitmapDrawable)getResources().getDrawable(R.drawable.bg_striped);
        bg.setTileModeXY(TileMode.REPEAT, TileMode.REPEAT);
        getSupportActionBar().setBackgroundDrawable(bg);

        BitmapDrawable bgSplit = (BitmapDrawable)getResources().getDrawable(R.drawable.bg_striped_split_img);
        bgSplit.setTileModeXY(TileMode.REPEAT, TileMode.REPEAT);
        getSupportActionBar().setSplitBackgroundDrawable(bgSplit);

        if( !currentDirectory.equals( homeDirectory )){
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

    	if( viewMode == GRID_VIEW){
	        // code to make a grid view
        	setContentView(R.layout.file_grid);
	    	gv = (GridView)findViewById(R.id.file_explorer_grid_view);
        	fileNames = currentDirectory.list( FileUtilities.getFilenameFilter( filterMode ) );
        	filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
	        gv.setOnItemClickListener(new OnItemClickListener() {
	            public void onItemClick(AdapterView<?> parent, View view,
	                int position, long id) {
	            	File file = filePaths[position];
	            	if(!file.isDirectory()){
	            		open(fileNames[position]);
	            	}else{
	            		file = new File( currentDirectory, file.getName() );
	            		openDirectory( file );
	            	}
	            		
	            }
	          });
	        gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory, filePaths ) );
	        actionBar.setSelectedNavigationItem( filterMode + 1 );//This triggers the listener which modifies the view.
        }else{
        	setContentView(R.layout.file_list);
        	lv = (ListView)findViewById( R.id.file_explorer_list_view);
        	lv.setClickable(true);
        	fileNames = currentDirectory.list( FileUtilities.getFilenameFilter( filterMode ) );
        	filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
        	lv.setAdapter( new ListItemAdapter(getApplicationContext(), filePaths) );
        	actionBar.setSelectedNavigationItem( filterMode + 1 );
        }
    	
    }
    
    public void openDirectory(File dir ){
    	currentDirectory = dir; 
        if( !currentDirectory.equals( homeDirectory )){
            ActionBar actionBar = getSupportActionBar();
            actionBar.setDisplayHomeAsUpEnabled(true);
        }else{
            ActionBar actionBar = getSupportActionBar();
            actionBar.setDisplayHomeAsUpEnabled( false );
        }
    	filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
    	fileNames = new String[ filePaths.length ];
    	FileUtilities.sortFiles( filePaths, sortMode );
    	for( int i = 0; i < fileNames.length; i++){
            fileNames[ i ] = filePaths[ i ].getName();
            if( !FileUtilities.hasThumbnail( filePaths[ i ] ) )
            {
                new ThumbnailGenerator( filePaths[ i ] );
            }
    	}
    	if( viewMode == GRID_VIEW){
    		gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory, filePaths ) );
    	}else{
    		lv.setAdapter( new ListItemAdapter(getApplicationContext(), filePaths) );
    	}
    }
    
    public void open(String file){
    	/*
    	Intent i = new Intent( this , WriterViewerActivity.class );
    	i.putExtra( CURRENT_DIRECTORY_KEY , currentDirectory.getAbsolutePath() );
    	i.putExtra( FILTER_MODE_KEY  , filterMode );
    	i.putExtra( EXPLORER_VIEW_TYPE_KEY  , viewMode );
    	startActivity( i );
    	*/
    	Intent i = new Intent( this , DocumentLoader.class );
    	i.putExtra("input",new File( currentDirectory , file).getAbsolutePath() );
    	i.putExtra( CURRENT_DIRECTORY_KEY , currentDirectory.getAbsolutePath() );
    	i.putExtra( FILTER_MODE_KEY  , filterMode );
    	i.putExtra( EXPLORER_VIEW_TYPE_KEY  , viewMode );
    	startActivity( i );
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getSupportMenuInflater();
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
            case R.id.menu_preferences:
                editPreferences(item);
                break;
            default:
                return super.onOptionsItemSelected(item);
        }
	    return true;
	}
    
    public void createDummyFileSystem(){
    	boolean mExternalStorageAvailable = false;
        boolean mExternalStorageWriteable = false;
        String state = Environment.getExternalStorageState();

        if (Environment.MEDIA_MOUNTED.equals(state)) {
            mExternalStorageAvailable = mExternalStorageWriteable = true;
        } else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            mExternalStorageAvailable = true;
            mExternalStorageWriteable = false;
        } else {
            mExternalStorageAvailable = mExternalStorageWriteable = false;
        }
        if( mExternalStorageAvailable && mExternalStorageWriteable ){//can also check if its writeable 
        	Log.d(tag, Boolean.toString( currentDirectory.mkdir() ) );
        	try {
        		File[] removeList = currentDirectory.listFiles();
        		for(File item : removeList){
        			if(item.isDirectory())
        				continue;//Log.d(tag, item.getPath());
        			item.delete();
        		}
        		new File( currentDirectory , "d0.odp" ).createNewFile();
				new File( currentDirectory , "d1.odt" ).createNewFile();
				new File( currentDirectory , "d2.odt" ).createNewFile();
				new File( currentDirectory , "d3.odp" ).createNewFile();
				new File( currentDirectory , "d4.ods" ).createNewFile();
				new File( currentDirectory , "d5.odt" ).createNewFile();
				new File( currentDirectory , "d6.odp" ).createNewFile();
				new File( currentDirectory , "d7.odt" ).createNewFile();
				new File( currentDirectory , "d8.odt" ).createNewFile();
				new File( currentDirectory , "d9.odp" ).createNewFile();
				new File( currentDirectory , "d10.odp" ).createNewFile();
				new File( currentDirectory , "d11.odt" ).createNewFile();
				new File( currentDirectory , "d12.odt" ).createNewFile();
				new File( currentDirectory , "d13.odp" ).createNewFile();
				new File( currentDirectory , "d14.ods" ).createNewFile();
				new File( currentDirectory , "d15.odt" ).createNewFile();
				File templatesDirectory = new File( currentDirectory , "Templates" );
				templatesDirectory.mkdir();
				new File( templatesDirectory , "template1.odt" ).createNewFile();
				new File( templatesDirectory , "template2.odt" ).createNewFile();
				new File( templatesDirectory , "template3.ods" ).createNewFile();
				new File( templatesDirectory , "template4.odp" ).createNewFile();
				File regularDirectory = new File( currentDirectory , "Folder" );
				regularDirectory.mkdir();
				new File( regularDirectory , "yetAnotherDoc.odt" ).createNewFile();
				new File( regularDirectory , "some really long file name.ods" ).createNewFile();
				File anotherRegularDirectory = new File( regularDirectory , "AnotherFolder" );
				anotherRegularDirectory.mkdir();
				new File( anotherRegularDirectory , "yetAnotherDoc2.odt" ).createNewFile();
				//Should put a folder in at some stage.
				
			} catch (IOException e) {
				Log.d(tag, "file io failure");
				e.printStackTrace();
			}
        	//Log.d(tag, fileStore.toString());
        }
        else{
        	Log.d(tag, "No External Storage");
        }
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
		return;
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
					Log.d("LIST", "click!");
					if(filePaths[ pos ].isDirectory() ){
						openDirectory( filePaths[ pos ] );
					}else{
						open( filePaths[ pos ].getName() );
					}
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
			if( filePaths[position].getName().endsWith(".odt") ){
				imageView.setImageResource(R.drawable.writer);
			}
			if( filePaths[position].getName().endsWith(".ods") ){
				imageView.setImageResource(R.drawable.calc);
			}
			if( filePaths[position].getName().endsWith(".odp") ){
				imageView.setImageResource(R.drawable.impress);
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

    class ThumbnailGenerator{
        private File file;

        ThumbnailGenerator( File file ){
            this.file = file;
                String input = file.getAbsolutePath();
                if (input == null)
                    input = "/assets/test1.odt";
                // Load the wanted document
                new DocumentLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, "file://" + input);
        }

        class DocumentLoadTask
            extends AsyncTask<String, Void, Integer>
        {

            ByteBuffer renderPage(int number, int width , int height)
            {
                try {
                    // Use dummySmallDevice with no scale of offset just to find out
                    // the paper size of this page.

                    PropertyValue renderProps[] = new PropertyValue[3];
                    renderProps[0] = new PropertyValue();
                    renderProps[0].Name = "IsPrinter";
                    renderProps[0].Value = new Boolean(true);
                    renderProps[1] = new PropertyValue();
                    renderProps[1].Name = "RenderDevice";
                    renderProps[1].Value = dummySmallDevice;
                    renderProps[2] = new PropertyValue();
                    renderProps[2].Name = "View";
                    renderProps[2].Value = new MyXController();

                    // getRenderer returns a set of properties that include the PageSize
                    long t0 = System.currentTimeMillis();
                    PropertyValue rendererProps[] = renderable.getRenderer(number, doc, renderProps);
                    long t1 = System.currentTimeMillis();
                    Log.i(TAG, "w,h getRenderer took " + ((t1-t0)-timingOverhead) + " ms");

                    int pageWidth = 0, pageHeight = 0;
                    for (int i = 0; i < rendererProps.length; i++) {
                        if (rendererProps[i].Name.equals("PageSize")) {
                            pageWidth = ((Size) rendererProps[i].Value).Width;
                            pageHeight = ((Size) rendererProps[i].Value).Height;
                            Log.i(TAG, " w,h PageSize: " + pageWidth + "x" + pageHeight);
                        }
                    }

                    // Create a new device with the correct scale and offset
                    ByteBuffer bb = ByteBuffer.allocateDirect(width*height*4);
                    long wrapped_bb = Bootstrap.new_byte_buffer_wrapper(bb);

                    XDevice device;
                    if (pageWidth == 0) {
                        // Huh?
                        device = toolkit.createScreenCompatibleDeviceUsingBuffer(width, height, 1, 1, 0, 0, wrapped_bb);
                    } else {

                        // Scale so that it fits our device which has a resolution of 96/in (see
                        // SvpSalGraphics::GetResolution()). The page size returned from getRenderer() is in 1/mm * 100.

                        int scaleNumerator, scaleDenominator;

                        // If the view has a wider aspect ratio than the page, fit
                        // height; otherwise, fit width
                        if ((double) width / height > (double) pageWidth / pageHeight) {
                            scaleNumerator = height;
                            scaleDenominator = pageHeight / 2540 * 96;
                        } else {
                            scaleNumerator = width;
                            scaleDenominator = pageWidth / 2540 * 96;
                        }
                        Log.i(TAG, "w,h Scaling with " + scaleNumerator + "/" + scaleDenominator);

                        device = toolkit.createScreenCompatibleDeviceUsingBuffer(width, height,
                                                                                 scaleNumerator, scaleDenominator,
                                                                                 0, 0,
                                                                                 wrapped_bb);
                    }

                    // Update the property that points to the device
                    renderProps[1].Value = device;

                    t0 = System.currentTimeMillis();
                    renderable.render(number, doc, renderProps);
                    t1 = System.currentTimeMillis();
                    Log.i(TAG, "w,h Rendering page " + number + " took " + ((t1-t0)-timingOverhead) + " ms");

                    Bootstrap.force_full_alpha_bb(bb, 0, width * height * 4);

                    return bb;
                }
                catch (Exception e) {
                    e.printStackTrace(System.err);
                    finish();
                }

                return null;
            }

            protected void onPreExecute ()
            {
                try{

                }
                catch (Exception e) {
                    e.printStackTrace(System.err);
                }
            }

            protected Integer doInBackground(String... params)
            {
                try {
                    long t0 = System.currentTimeMillis();
                    long t1 = System.currentTimeMillis();
                    timingOverhead = t1 - t0;

                    Bootstrap.setup(LibreOfficeUIActivity.this);

                    Bootstrap.putenv("SAL_LOG=yes");

                    // Load a lot of shlibs here explicitly in advance because that
                    // makes debugging work better, sigh
                    Bootstrap.dlopen("libvcllo.so");
                    Bootstrap.dlopen("libmergedlo.so");
                    Bootstrap.dlopen("libswdlo.so");
                    Bootstrap.dlopen("libswlo.so");

                    // Log.i(TAG, "Sleeping NOW");
                    // Thread.sleep(20000);

                    context = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

                    Log.i(TAG, "context is" + (context!=null ? " not" : "") + " null");

                    mcf = context.getServiceManager();

                    Log.i(TAG, "mcf is" + (mcf!=null ? " not" : "") + " null");

                    // We need to fake up an argv, and the argv[0] even needs to
                    // point to some file name that we can pretend is the "program".
                    // setCommandArgs() will prefix argv[0] with the app's data
                    // directory.

                    String[] argv = { "lo-document-loader", file.getAbsolutePath() };

                    Bootstrap.setCommandArgs(argv);

                    Bootstrap.initVCL();

                    Object desktop = mcf.createInstanceWithContext
                        ("com.sun.star.frame.Desktop", context);

                    Log.i(TAG, "desktop is" + (desktop!=null ? " not" : "") + " null");

                    Bootstrap.initUCBHelper();

                    componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);

                    Log.i(TAG, "componentLoader is" + (componentLoader!=null ? " not" : "") + " null");

                    String url = params[0];
                    Log.i(TAG, "Attempting to load " + url);

                    PropertyValue loadProps[] = new PropertyValue[3];
                    loadProps[0] = new PropertyValue();
                    loadProps[0].Name = "Hidden";
                    loadProps[0].Value = new Boolean(true);
                    loadProps[1] = new PropertyValue();
                    loadProps[1].Name = "ReadOnly";
                    loadProps[1].Value = new Boolean(true);
                    loadProps[2] = new PropertyValue();
                    loadProps[2].Name = "Preview";
                    loadProps[2].Value = new Boolean(true);

                    t0 = System.currentTimeMillis();
                    doc = componentLoader.loadComponentFromURL(url, "_blank", 0, loadProps);
                    t1 = System.currentTimeMillis();
                    Log.i(TAG, "Loading took " + ((t1-t0)-timingOverhead) + " ms");

                    Object toolkitService = mcf.createInstanceWithContext
                        ("com.sun.star.awt.Toolkit", context);
                    toolkit = (XToolkit2) UnoRuntime.queryInterface(XToolkit2.class, toolkitService);

                    renderable = (XRenderable) UnoRuntime.queryInterface(XRenderable.class, doc);

                    // Set up dummySmallDevice and use it to find out the number
                    // of pages ("renderers").
                    ByteBuffer smallbb = ByteBuffer.allocateDirect(SMALLSIZE*SMALLSIZE*4);
                    long wrapped_smallbb = Bootstrap.new_byte_buffer_wrapper(smallbb);
                    dummySmallDevice = toolkit.createScreenCompatibleDeviceUsingBuffer(SMALLSIZE, SMALLSIZE, 1, 1, 0, 0, wrapped_smallbb);

                    PropertyValue renderProps[] = new PropertyValue[3];
                    renderProps[0] = new PropertyValue();
                    renderProps[0].Name = "IsPrinter";
                    renderProps[0].Value = new Boolean(true);
                    renderProps[1] = new PropertyValue();
                    renderProps[1].Name = "RenderDevice";
                    renderProps[1].Value = dummySmallDevice;
                    renderProps[2] = new PropertyValue();
                    renderProps[2].Name = "View";
                    renderProps[2].Value = new MyXController();

                    Log.i(TAG, "Document is" + (doc!=null ? " not" : "") + " null");
                    t0 = System.currentTimeMillis();
                    pageCount = renderable.getRendererCount(doc, renderProps);
                    t1 = System.currentTimeMillis();
                    Log.i(TAG, "getRendererCount: " + pageCount + ", took " + ((t1-t0)-timingOverhead) + " ms");
                }
                catch (Exception e) {
                    e.printStackTrace(System.err);
                    finish();
                }
                return new Integer( 0 );
        }

            protected void onPostExecute(Integer result){
                int widthInPx = dpToPx( 100 );
                int heightInPx = dpToPx( (int)( 100*Math.sqrt(2) ) );
                ByteBuffer bb = renderPage( 0 , widthInPx , heightInPx);
                Bitmap bm = Bitmap.createBitmap( widthInPx , heightInPx , Bitmap.Config.ARGB_8888);
                bm.copyPixelsFromBuffer(bb);

                Matrix m = new Matrix();
                m.preScale( 1.0f , -1.0f );
                Bitmap bmp = Bitmap.createBitmap( bm, 0, 0, bm.getWidth(), bm.getHeight(), m, true);
                File dir = file.getParentFile();
                File thumbnailFile = new File( dir , "." + file.getName().split("[.]")[0] + ".png");
                try {
                    ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                    bmp.compress(Bitmap.CompressFormat.PNG, 40, bytes);
                    thumbnailFile.createNewFile();
                    FileOutputStream fo = new FileOutputStream( thumbnailFile );
                    fo.write(bytes.toByteArray());
                } catch (IOException e) {
                    // TODO: handle exception
                }
                ( (GridItemAdapter)gv.getAdapter() ).update();
        }
        }

        class MyXController
            implements XController
        {

        XFrame frame;
        XModel model;

        public void attachFrame(XFrame frame)
        {
            Log.i(TAG, "attachFrame");
            this.frame = frame;
        }

        public boolean attachModel(XModel model)
        {
            Log.i(TAG, "attachModel");
            this.model = model;
            return true;
        }

        public boolean suspend(boolean doSuspend)
        {
            Log.i(TAG, "suspend");
            return false;
        }

        public Object getViewData()
        {
            Log.i(TAG, "getViewData");
            return null;
        }

        public void restoreViewData(Object data)
        {
            Log.i(TAG, "restoreViewData");
        }

        public XModel getModel()
        {
            Log.i(TAG, "getModel");
            return model;
        }

        public XFrame getFrame()
        {
            Log.i(TAG, "getFrame");
            return frame;
        }

        public void dispose()
        {
            Log.i(TAG, "dispose");
        }

        public void addEventListener(XEventListener listener)
        {
            Log.i(TAG, "addEventListener");
        }

        public void removeEventListener(XEventListener listener)
        {
            Log.i(TAG, "removeEventListener");
        }
    }

    }
}


