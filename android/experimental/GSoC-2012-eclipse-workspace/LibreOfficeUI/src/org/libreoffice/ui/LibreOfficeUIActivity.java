package org.libreoffice.ui;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.IOException;

import android.app.ActionBar;
import android.app.ActionBar.OnNavigationListener;
import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.*;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ListView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class LibreOfficeUIActivity extends Activity implements OnNavigationListener {
    private String tag = "file_manager";
    private File homeDirectory;//make final?
	private File currentDirectory;
	private String filter = "";
	private String[] filters = {"all",".odt",".ods",".odp"};
	private int filterMode = FileUtilities.ALL;
	FileFilter fileFilter;
	FilenameFilter filenameFilter;
	private String[] fileNames;
	private File[] filePaths;
	
	private String currentDirectoryKey = "CURRENT_DIRECTORY";
	private String filterModeKey = "FILTER_MODE";
	
	GridView gv;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_grid);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowTitleEnabled(false);//This should show current directory if anything
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        SpinnerAdapter mSpinnerAdapter = ArrayAdapter.createFromResource(this, R.array.file_view_modes,
                android.R.layout.simple_spinner_dropdown_item);
        actionBar.setListNavigationCallbacks(mSpinnerAdapter, this);
			
        
        homeDirectory  = new File(Environment.getExternalStorageDirectory(),"LibreOffice");
        Intent i = this.getIntent();
        if( i.hasExtra( currentDirectoryKey ) ){
        	//This isn't what I think it is. It's not a full path
        	currentDirectory = new File( i.getStringExtra( currentDirectoryKey ) );
        }else{
	        currentDirectory = homeDirectory;
        }
        
        if( i.hasExtra( filterModeKey ) ){
        	Log.d(filterModeKey+"_GRID_OC" , Integer.toString( i.getIntExtra( filterModeKey, FileUtilities.ALL ) ));
            filterMode = i.getIntExtra( filterModeKey, FileUtilities.ALL);
        }
        if( !currentDirectory.equals( homeDirectory )){
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

		//fileFilter = FileUtilities.getFileFilter( filterMode );
		//filenameFilter = FileUtilities.getFilenameFilter( filterMode );	
		
        //createDummyFileSystem();
    	//filePaths = currentDirectory.listFiles( fileFilter );
    	//order/filter filePaths here
    	//fileNames = currentDirectory.list( filenameFilter );

        // code to make a grid view 
    	gv = (GridView)findViewById(R.id.file_explorer_grid_view);
        //GridItemAdapter gridAdapter = new GridItemAdapter(getApplicationContext(), filePaths );
        //gv.setAdapter(gridAdapter);
        gv.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view,
                int position, long id) {
            	File file = filePaths[position];
            	if(!file.isDirectory()){
            		open(fileNames[position]);
            	}else{
            		/*fileNames = file.list();
                    filePaths = file.listFiles();
                    GridItemAdapter gridAdapter = new GridItemAdapter(getApplicationContext(), filePaths );
                    gv.setAdapter(gridAdapter);*/
            		file = new File( currentDirectory, file.getName() );
            		openDirectory( file );
            	}
            		
            }
          });
        actionBar.setSelectedNavigationItem( filterMode + 1 );//This triggers the listener which modifies the view.
        //openDirectory( currentDirectory );
    }
    
    public void openDirectory(File dir ){
    	currentDirectory = dir; 
    	Log.d(tag, dir.toString() + " " + homeDirectory.toString());
        if( !currentDirectory.equals( homeDirectory )){
            ActionBar actionBar = getActionBar();
            actionBar.setDisplayHomeAsUpEnabled(true);
        }else{
            ActionBar actionBar = getActionBar();
            actionBar.setDisplayHomeAsUpEnabled( false );
        }
    	fileNames = currentDirectory.list( FileUtilities.getFilenameFilter( filterMode ) );
    	filePaths = currentDirectory.listFiles( FileUtilities.getFileFilter( filterMode ) );
    	gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory, filePaths ) );
    }
    
    public void open(String file){
    	//Should add file to the intent as an extra
    	Intent i = new Intent( this , WriterViewerActivity.class );
    	i.putExtra( currentDirectoryKey , currentDirectory.getAbsolutePath() );
    	i.putExtra( filterModeKey  , filterMode );
    	startActivity( i );
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.view_menu, menu);
        return true;
    }
    
    public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	        case android.R.id.home:
	            // app icon in action bar clicked; go home
	            //Intent intent = new Intent(this, LibreOfficeUIActivity.class);
	            //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
	            //startActivity(intent);
	            if( !currentDirectory.equals( homeDirectory ) ){
	            	openDirectory( currentDirectory.getParentFile() );
	            }
	            
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
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
    
    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	// TODO Auto-generated method stub
    	super.onSaveInstanceState(outState);
    	outState.putString( currentDirectoryKey , currentDirectory.getAbsolutePath() );
    }
    
    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
    	// TODO Auto-generated method stub
    	super.onRestoreInstanceState(savedInstanceState);
    	//currentDirectory = new File( savedInstanceState.getString( currentDirectoryKey ) );
    }
    
    @Override
    protected void onPause() {
    	// TODO need to save fileSystem state here.
    	
    	super.onPause();
    }
     
	public boolean onNavigationItemSelected(int itemPosition, long itemId) {
		filterMode = itemPosition -1; //bit of a hack, I know. -1 is ALL 0 Docs etc
		openDirectory( currentDirectory );// Uses filter mode 
		return true;
	}

}


