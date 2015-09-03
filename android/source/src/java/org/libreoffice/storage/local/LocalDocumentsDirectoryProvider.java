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

import android.os.Environment;

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

    @Override
    public IFile getRootDirectory() {
        File documentsDirectory = new File(
                Environment.getExternalStorageDirectory(), "Documents");
        documentsDirectory.mkdirs();
        return new LocalFile(documentsDirectory);
    }

    @Override
    public int getNameResource() {
        return R.string.local_documents;
    }
}
