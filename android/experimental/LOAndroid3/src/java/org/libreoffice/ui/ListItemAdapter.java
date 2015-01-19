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

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.database.DataSetObserver;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

/*Currently this is class is not used but instead is implemented as an inner class in LibreOfficeUI.
 * This is because I can't get the onItemClickListener to fire on the listview so I need to set an
 * onClick listener in the adapter. ( I've tried turning off the focusability etc of the listitem
 * contents but no dice...) */
public class ListItemAdapter implements ListAdapter{
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

        listItem.setClickable(true);
        listItem.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                Log.d("LIST", "click!");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
