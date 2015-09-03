/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.storage.local;

import java.net.URI;

import org.libreoffice.storage.IDocumentProvider;
import org.libreoffice.storage.IFile;

import org.libreoffice.R;

import android.os.Environment;

/**
 * Implementation of IDocumentProvider for the local file system.
 */
public class LocalDocumentsProvider implements IDocumentProvider {

    private int id;

    public LocalDocumentsProvider(int id) {
        this.id = id;
    }

    @Override
    public IFile getRootDirectory() {
        return new LocalFile(Environment.getExternalStorageDirectory());
    }

    @Override
    public IFile createFromUri(URI uri) {
        return new LocalFile(uri);
    }

    @Override
    public int getNameResource() {
        return R.string.local_file_system;
    }

    @Override
    public int getId() {
        return id;
    }
}
