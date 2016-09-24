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
import java.io.FileFilter;
import java.net.URI;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.libreoffice.storage.IFile;

/**
 * Implementation of IFile for the local file system.
 */
public class LocalFile implements IFile {

    private File file;

    public LocalFile(File file) {
        this.file = file;
    }

    public LocalFile(URI uri) {
        this.file = new File(uri);
    }

    public URI getUri() {
        return file.toURI();
    }

    public String getName() {
        return file.getName();
    }

    @Override
    public boolean isDirectory() {
        return file.isDirectory();
    }

    @Override
    public long getSize() {
        return file.length();
    }

    @Override
    public Date getLastModified() {
        return new Date(file.lastModified());
    }

    @Override
    public List<IFile> listFiles() {
        List<IFile> children = new ArrayList<IFile>();
        for (File child : file.listFiles()) {
            children.add(new LocalFile(child));
        }
        return children;
    }

    @Override
    public List<IFile> listFiles(FileFilter filter) {
        List<IFile> children = new ArrayList<IFile>();
        for (File child : file.listFiles(filter)) {
            children.add(new LocalFile(child));
        }
        return children;
    }

    @Override
    public IFile getParent() {
        return new LocalFile(file.getParentFile());
    }

    @Override
    public File getDocument() {
        return file;
    }

    @Override
    public boolean equals(Object object) {
        if (this == object)
            return true;
        if (!(object instanceof LocalFile))
            return false;
        LocalFile file = (LocalFile) object;
        return file.getUri().equals(getUri());
    }

    @Override
    public void saveDocument(File file) {
        // do nothing; file is local
    }
}
