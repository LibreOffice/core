/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import org.libreoffice.R;
import org.libreoffice.storage.IFile;

import java.util.List;

public class GridItemAdapter extends BaseAdapter {
    Context mContext;
    List<IFile> filePaths;
    IFile currentDirectory;
    String LOGTAG = "GridItemAdapter";

    public GridItemAdapter(Context mContext, IFile currentDirectory,
            List<IFile> filteredFiles) {
        this.mContext = mContext;
        this.currentDirectory = currentDirectory;
        filePaths = filteredFiles;
        Log.d(LOGTAG, "currentDirectory.getName(): " + currentDirectory.getName());
    }

    public int getCount() {
        return filePaths != null ? filePaths.size() : 0;
    }

    public Object getItem(int position) {
        return null; //filePaths[ position ];
    }

    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return 0;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);

        View gridView;

        if (convertView == null) {
            gridView = new View(mContext);
        } else {
            gridView = convertView;
        }

        // get layout from mobile.xml
        gridView = inflater.inflate(R.layout.file_explorer_grid_item, null);

        // set value into textview
        TextView textView = (TextView) gridView
            .findViewById(R.id.grid_item_label);
        textView.setText(filePaths.get(position).getName());
        // set image based on selected text
        ImageView imageView = (ImageView) gridView
            .findViewById(R.id.grid_item_image);
        if (filePaths.get(position).isDirectory()) { // Is a folder
            // Default view is a generic folder icon.
            imageView.setImageResource(R.drawable.folder);
        } else {
            /*
            File thumbnailFile = new File( filePaths[position].getParent() , "."
                    + filePaths[position].getName().split("[.]")[0] + ".png");
            BitmapFactory factory = new BitmapFactory();
            Bitmap thumb = factory.decodeFile( thumbnailFile.getAbsolutePath() );
            if(thumb != null){
                Log.i( "GRID" , "true" );
            }else{
                Log.i( "GRID" , thumbnailFile.getAbsolutePath() );
            }
            */
            switch (FileUtilities.getType(filePaths.get(position).getName()))
            {
                case FileUtilities.DOC:
                    /*
                    if( thumb != null){
                        imageView.setImageBitmap( thumb );
                        break;
                    }
                    */
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
                case FileUtilities.UNKNOWN:
                default:
                    break; // FIXME something prettier ?
            }
        }
        return gridView;
    }

    public void update() {
        this.notifyDataSetChanged();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
