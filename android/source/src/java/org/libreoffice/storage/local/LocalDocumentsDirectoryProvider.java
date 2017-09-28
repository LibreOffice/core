/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.storage.local;

import java.io.File;

import org.libreoffice.storage.IFile;
import org.libreoffice.R;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.util.Log;

/**
 * A convenience IDocumentProvider to browse the /sdcard/Documents directory.
 *
 * Extends LocalDocumentsProvider to overwrite getRootDirectory and set it to
 * /sdcard/Documents. Most documents will probably be stored there so there is
 * no need for the user to browse the filesystem from the root every time.
 */
public class LocalDocumentsDirectoryProvider extends LocalDocumentsProvider {

    public LocalDocumentsDirectoryProvider(int id) {
        super(id);
    }

    private static File getDocumentsDir() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            // DIRECTORY_DOCUMENTS is 19 or later only
            return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
        } else {
            return new File(Environment.getExternalStorageDirectory() + "/Documents");
        }
    }

    @Override
    public IFile getRootDirectory(Context context) {
        File documentsDirectory = getDocumentsDir();
        if (!documentsDirectory.exists()) {
            // might be a little counter-intuitive: if we were granted READ permission already, we're also granted the write-permission
            // when we ask for it, since they are both in the same storage group (for 5.1 and lower it is granted at install-time already)
            // seehttps://developer.android.com/guide/topics/permissions/requesting.html#perm-groups
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
                if(!documentsDirectory.mkdirs()) {
                    // fallback to the toplevel dir - might be due to the dir not mounted/used as USB-Mass-Storage or similar
                    // TODO: handle unavailability of the storage/failure of the mkdir properly
                    Log.e("LocalDocumentsProvider", "not sure how we ended up here - if we have read permissions to use it in the first place, we also should have the write-permissions..");
                    documentsDirectory = Environment.getExternalStorageDirectory();
                }
            }
        }
        return new LocalFile(documentsDirectory);
    }

    @Override
    public int getNameResource() {
        return R.string.local_documents;
    }

    @Override
    public boolean checkProviderAvailability(Context context) {
        File documentsDirectory = getDocumentsDir();
        return documentsDirectory.exists() || ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }
}
