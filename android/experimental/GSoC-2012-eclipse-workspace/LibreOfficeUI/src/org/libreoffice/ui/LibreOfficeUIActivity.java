package org.libreoffice.ui;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;

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
import android.widget.TextView;
import android.widget.Toast;

public class LibreOfficeUIActivity extends Activity {
    private String tag = "file_manager";
    private File homeDirectory;//make final?
	private File currentDirectory;
	private String[] fileNames;
	private File[] filePaths;
	
	private String currentDirectoryKey = "CURRENT_DIRECTORY";
	GridView gv;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_explorer);
        gv = (GridView)findViewById(R.id.file_explorer_grid_view);
        
        //Need to sort out how best to maintain application state (directory, basically)
        Intent i = this.getIntent();
        if( i.hasExtra( currentDirectoryKey ) ){
        	//This isn't what I think it is. It's not a full path
        	currentDirectory = new File( i.getStringExtra( currentDirectoryKey ) );
        }
        
        homeDirectory  = new File(Environment.getExternalStorageDirectory(),"LibreOffice");
        currentDirectory = homeDirectory;
        //createDummyFileSystem();
    	filePaths = new File[ currentDirectory.listFiles().length ];
    	fileNames = new String[ filePaths.length ];
    	filePaths = currentDirectory.listFiles();// Need to write a class to order lists of Files
    	fileNames = currentDirectory.list();// should be based on filePaths -> keep same order

        // code to make a grid view 
        GridItemAdapter gridAdapter = new GridItemAdapter(getApplicationContext(), filePaths );
        gv.setAdapter(gridAdapter);
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
        
        
    }
    
    public void openDirectory(File dir){
    	currentDirectory = dir; 
    	fileNames = currentDirectory.list();
    	filePaths = currentDirectory.listFiles();
    	gv.setAdapter( new GridItemAdapter(getApplicationContext(), currentDirectory ) );
    }
    
    public void open(String file){
    	//Should add file to the intent as an extra
    	Intent i = new Intent( this , WriterViewerActivity.class );
    	i.putExtra( currentDirectoryKey, currentDirectory.getName() );
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
	            Intent intent = new Intent(this, LibreOfficeUIActivity.class);
	            //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
	            startActivity(intent);
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
    	outState.putString( currentDirectoryKey , currentDirectory.getName() );
    }
    
    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
    	// TODO Auto-generated method stub
    	super.onRestoreInstanceState(savedInstanceState);
    	currentDirectory = new File( savedInstanceState.getString( currentDirectoryKey ) );
    }
    
    @Override
    protected void onPause() {
    	// TODO need to save fileSystem state here.
    	
    	super.onPause();
    }

}


