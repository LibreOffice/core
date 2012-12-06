// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

// This activity creates a file chooser by which the user selects a document on devices SDCard
// Once the file is selected, this activity finishes and the DocumentLoader is reactivated

package org.libreoffice.android.examples;

import java.io.File;

import android.app.ListActivity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ListView;
import android.widget.Toast;
import android.content.Intent;
import android.util.Log;

import org.libreoffice.android.SelectDocumentAdapter;

public class SelectDocumentActivity extends ListActivity {
        private File currentDirectory = null;
        private File parentDirectory = null;
        private final String ROOT_DIRECTORY_PATH =
                Environment.getExternalStorageDirectory().getPath(); // "/mnt/sdcard";

        /** Called when the activity is first created. */
        @Override
        public void onCreate(Bundle savedInstanceState) {
                super.onCreate(savedInstanceState);
                // Set current and parent directories to root (initially)
                // Parent diretory is unused currently; it may be used to navigate back
                // to the parent directory
                currentDirectory = new File(ROOT_DIRECTORY_PATH);
                parentDirectory = currentDirectory;
                // Process the root directory
                processDirectory();
        }

        @Override
        protected void onListItemClick(ListView l, View v,
                        int position, long id) {

                File file = (File) getListAdapter().getItem(position);
                if (file.isDirectory()) {
                        currentDirectory = file;
                        parentDirectory = file.getParentFile() == null ?
                                        new File(ROOT_DIRECTORY_PATH) :
                                                file.getParentFile();
                        processDirectory();
                } else {
                        Intent i = new Intent(this, DocumentLoader.class);
                        i.putExtra("input", file.getPath());
                        Log.i("LODocumentLoader", "FilePath: "+file.getPath());
                        startActivity(i);
                        finish();
                }
        }

        /**
         * List content of current directory
         */
        private void processDirectory() {
                File[] files = currentDirectory.listFiles();
                SelectDocumentAdapter adapter = new SelectDocumentAdapter(this, files);
                setListAdapter(adapter);
        }
}
