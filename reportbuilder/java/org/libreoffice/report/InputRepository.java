/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report;

import java.io.IOException;
import java.io.InputStream;

/**
 * This allows the job processor to load data from a repository. It is assumed,
 * that all resource names are given as strings and furthermore, that the names
 * identify a resource uniquely within the input repository.
 *
 * An input repository connects the report processing to the xml definitions.
 * Unless defined otherwise, it is assumed, that the input-name is 'content.xml';
 * possible other files are 'settings.xml' and 'styles.xml' (see the Oasis standard
 * for details on these files and their contents).
 *
 */
public interface InputRepository
{

    /**
     * Returns a unique identifier for this repository. Two repositories accessing
     * the same location should return the same id. The identifier must never
     * be null.
     *
     * @return the repository id
     */
    Object getId();

    InputStream createInputStream(final String name) throws IOException;

    /** allows to acces sub repositories inside this repository
     *
     * @param name describes the path to the sub repository
     * @return the sub repository
     * @throws java.io.IOException when the sub repository doesn't exist.
     */
    InputRepository openInputRepository(final String name) throws IOException;

    /**
     * This returns an version number for the given resource. Return zero, if
     * the resource is not versionable, else return a unique number for each version.
     * As rule of thumb: Increase the version number by at least one for each change
     * made to the resource.
     *
     * @param name the name of the resource
     * @return the version number
     */
    long getVersion(final String name);

    boolean exists(final String name);

    boolean isReadable(final String name);

    void closeInputRepository();

    /** returns the URL of the database document
     *
     * @return the URL of the database document
     */
    String getRootURL();
}
