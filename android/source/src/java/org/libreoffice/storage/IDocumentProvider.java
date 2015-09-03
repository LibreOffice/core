/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.storage;

import java.net.URI;

/**
 * Represents a Document Provider, an object able to provide documents from a
 * certain source (e.g. local documents, DropBox, Google Docs).
 */
public interface IDocumentProvider {

    /**
     * Provides the content root element for the Document Provider.
     *
     * @return Content root element.
     * @throws RuntimeException in case of error.
     */
    IFile getRootDirectory();

    /**
     * Transforms some URI into the IFile object that represents that content.
     *
     * @param uri
     *            URI pointing to some content object that has been previously
     *            retrieved with IFile.getUri().
     * @return IFile object pointing to the content represented by uri.
     * @throws RuntimeException in case of error.
     */
    IFile createFromUri(URI uri);

    /**
     * Get internationalized name for this provider. This name is intended to be
     * shown in the UI.
     *
     * @return string resource pointing to the provider name.
     */
    int getNameResource();

    /**
     * Provides the unique ID for a document provider instance in a program.
     *
     * This ID should be set when the instance is built. It could be used to
     * tell two instances of the same document provider apart, e. g. two
     * instances of OwnCloudProvider pointing to different servers.
     *
     * @return Unique ID for a document provider instance.
     */
    int getId();
}
