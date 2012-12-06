// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

// This class is used as an adapter by the document chooser interface
// The document chooser UI is defined in select_document.xml
// It renders a simple listing of files and directories; files and directories are
// visually distinguished by an icon in the listing.
// The UI may be improved in future; it is simple and experimental for now

package org.libreoffice.android;

import org.libreoffice.android.examples.R;

import java.io.File;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class SelectDocumentAdapter extends ArrayAdapter<File> {
        private final Context context;
               private final File[] files;
        public SelectDocumentAdapter(Context context, File[] files) {
                super(context, R.layout.select_document, files);
                this.context = context;
                this.files = files;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
                LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                View rowView = inflater.inflate(R.layout.select_document, parent, false);
                TextView textView = (TextView) rowView.findViewById(R.id.label);
                ImageView imageView = (ImageView) rowView.findViewById(R.id.icon_file);
                textView.setText(files[position].getName());
                // Get the current file
                File f = files[position];

                // Select the right icon based on whether f is a directory or a regular
                if (f.isDirectory()) {
                        imageView.setImageResource(R.drawable.folder);
                } else {
                        imageView.setImageResource(R.drawable.file);
                }

                return rowView;
        }
}
