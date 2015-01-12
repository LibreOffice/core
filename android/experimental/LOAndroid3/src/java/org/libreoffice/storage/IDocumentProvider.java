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
     */
    IFile getRootDirectory();

    /**
     * Transforms some URI into the IFile object that represents that content.
     *
     * @param uri
     *            URI pointing to some content object that has been previously
     *            retrieved with IFile.getUri().
     * @return IFile object pointing to the content represented by uri.
     */
    IFile createFromUri(URI uri);
}
