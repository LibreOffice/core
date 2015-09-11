/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.storage;

import java.io.File;
import java.io.FileFilter;
import java.net.URI;
import java.util.Date;
import java.util.List;

/**
 * An abstraction of the File class, intended to be implemented by different
 * Document Providers.
 *
 * It represents a file or a directory in the context of a certain Document
 * Provider. It wraps the file-related operations and provides access to the
 * final document as a local File, downloading it if necessary.
 */
public interface IFile {

    /**
     * Provides a URI that represents this IFile object.
     *
     * @return URI that represents this IFile object in the context of the
     *         Document Provider that created it. The URI can be transformed
     *         back into an IFile object with IDocumentProvider.createFromUri().
     */
    URI getUri();

    /**
     * Returns the name of the file or directory represented by this file.
     *
     * @return This file's name.
     */
    String getName();

    /**
     * Indicates if this file represents a directory in the context of the
     * Document Provider which originated it.
     *
     * @return true if this file is a directory, false otherwise.
     */
    boolean isDirectory();

    /**
     * Returns the file size in bytes.
     *
     * @return file size in bytes, 0 if the file does not exist.
     */
    long getSize();

    /**
     * Returns the time when this file was last modified, measured in
     * milliseconds since January 1st, 1970, midnight.
     *
     * @return time when this file was last modified, or 0 if the file does not
     *         exist.
     */
    Date getLastModified();

    /**
     * Returns a list containing the files in the directory represented by this
     * file.
     *
     * @return list of files contained by this directory, or an empty list if
     *         this is not a directory.
     * @throws RuntimeException in case of error.
     */
    List<IFile> listFiles();

    /**
     * Gets the list of files contained in the directory represented by this
     * file, and filters it through some FilenameFilter.
     *
     * @param filter
     *            the filter to match names against.
     * @return filtered list of files contained by this directory, or an empty
     *         list if this is not a directory.
     * @throws RuntimeException in case of error.
     */
    List<IFile> listFiles(FileFilter filter);

    /**
     * Returns the pparent of this file.
     *
     * @return this file's parent or null if it does not have it.
     */
    IFile getParent();

    /**
     * Returns the document wrapped by this IFile as a local file. The result
     * for a directory is not defined.
     *
     * @return local file containing the document wrapped by this object.
     * @throws RuntimeException in case of error.
     */
    File getDocument();

    /**
     * Replaces the wrapped document with a new version of it.
     *
     * @param file
     *            A local file pointing to the new version of the document.
     */
    void saveDocument(File file);
}
