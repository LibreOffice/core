/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import org.libreoffice.R;


import java.io.File;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;

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

	public GridItemAdapter(Context mContext, File currentDirectory, File[] filteredFiles)
	{
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

	public View getView(int position, View convertView, ViewGroup parent)
	{
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
            if( filePaths[position].isDirectory() ) // Is a folder
            {
                // Default view is a generic folder icon.
                imageView.setImageResource(R.drawable.folder);
                // How should we handle empty folders / folders with no thumbnails? -> new files
                gridView =  inflater.inflate(R.layout.file_explorer_folder_icon, null);
                org.libreoffice.ui.FolderIconView icon =
                    (org.libreoffice.ui.FolderIconView)gridView.findViewById(R.id.folder_icon);
                icon.setDir( filePaths[position]);
                textView = (TextView) gridView.findViewById(R.id.grid_item_label);
                textView.setText(filePaths[position].getName());
                return gridView;
            }
            else
            {
                File thumbnailFile = new File( filePaths[position].getParent() , "."
            + filePaths[position].getName().split("[.]")[0] + ".png");
                BitmapFactory factory = new BitmapFactory();
                Bitmap thumb = factory.decodeFile( thumbnailFile.getAbsolutePath() );
                if(thumb != null){
                    Log.i( "GRID" , "true" );
                }else{
                    Log.i( "GRID" , thumbnailFile.getAbsolutePath() );
                }
                switch (FileUtilities.getType(filePaths[position].getName()))
                {
                case FileUtilities.DOC:
                    if( thumb != null){
                        imageView.setImageBitmap( thumb );
                        break;
                    }
                    imageView.setImageResource(R.drawable.writer);
                    break;
			    case FileUtilities.CALC:
                    imageView.setImageResource(R.drawable.calc);
                    break;
			    case FileUtilities.DRAWING: // FIXME: only for now ...
			    case FileUtilities.IMPRESS:
                    imageView.setImageResource(R.drawable.impress);
                    break;
			    case FileUtilities.UNKNOWN:
			    default:
				    break; // FIXME something prettier ?
			    }
			}
			return gridView;
	}

}
