package org.libreoffice.ui;

import java.io.File;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.MenuItem;

public class WriterViewerActivity extends Activity{
	private String currentDirectoryKey = "CURRENT_DIRECTORY";
	private String filterModeKey = "FILTER_MODE";
	File dir;
	private int filterMode;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		Intent i = this.getIntent();
		if( i.hasExtra( currentDirectoryKey ) ){
			Log.d(currentDirectoryKey, i.getStringExtra( currentDirectoryKey ) );
			dir = new File( i.getStringExtra( currentDirectoryKey ) );
		}
		if( i.hasExtra( filterModeKey ) ){
			//Log.d("FILTER_OC_WRITER", Integer.toString(i.getIntExtra( filterModeKey , FileUtilities.ALL )));
        	filterMode = i.getIntExtra( filterModeKey , FileUtilities.ALL );
		}
		setContentView(R.layout.main);
	    ActionBar actionBar = getActionBar();
	    actionBar.setDisplayHomeAsUpEnabled(true);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	        case android.R.id.home:
	            // app icon in action bar clicked; go home
	            Intent intent = new Intent(this, LibreOfficeUIActivity.class);
	            intent.putExtra(currentDirectoryKey, dir.getAbsolutePath() );
	    		intent.putExtra( filterModeKey , filterMode );
	            //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
	            startActivity(intent);
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}

}
