package org.libreoffice.ui;

import java.io.File;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class GridItemAdapter extends BaseAdapter{
	Context mContext;
	File[] filePaths;
	File currentDirectory;
	String tag = "GridItemAdapter";
	
	public GridItemAdapter(Context mContext, File[] filePaths) {
		this.mContext = mContext;
		this.filePaths = filePaths;
		for(File fn : filePaths){
			Log.d(tag, fn.getName());
		}
	}
	
	public GridItemAdapter(Context mContext, File currentDirectory) {
		this.mContext = mContext;
		this.currentDirectory = currentDirectory;
		filePaths = currentDirectory.listFiles();
	}
	
	public GridItemAdapter(Context mContext, File currentDirectory, File[] filteredFiles) {
		this.mContext = mContext;
		this.currentDirectory = currentDirectory;
		filePaths = filteredFiles;
	}

	public int getCount() {
		return filePaths.length;
	}

	public Object getItem(int position) {
		return null;//filePaths[ position ];
	}

	public long getItemId(int arg0) {
		// TODO Auto-generated method stub
		return 0;
	}

	public View getView(int position, View convertView, ViewGroup parent) {
		LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(
				Context.LAYOUT_INFLATER_SERVICE);
	 
			View gridView;
	 
			if (convertView == null) {
	 
				
	 
			} else {
				gridView = (View) convertView;
			}
			gridView = new View(mContext);
			 
			// get layout from mobile.xml
			gridView = inflater.inflate(R.layout.file_explorer_grid_item, null);
 
			// set value into textview
			TextView textView = (TextView) gridView
					.findViewById(R.id.grid_item_label);
			textView.setText(filePaths[position].getName());
			// set image based on selected text
			ImageView imageView = (ImageView) gridView
					.findViewById(R.id.grid_item_image);
			if( filePaths[position].getName().endsWith(".odt") ){
				imageView.setImageResource(R.drawable.writer);
			}
			if( filePaths[position].getName().endsWith(".ods") ){
				imageView.setImageResource(R.drawable.calc);
			}
			if( filePaths[position].getName().endsWith(".odp") ){
				imageView.setImageResource(R.drawable.impress);
			}
			if( filePaths[position].isDirectory() ){//Is a folder
				//Eventually have thumbnails of each sub file on a black circle
				//For now just a folder icon
				imageView.setImageResource(R.drawable.folder);
			}
			return gridView;
	}

}
