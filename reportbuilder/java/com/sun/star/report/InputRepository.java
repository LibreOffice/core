/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InputRepository.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report;

import java.io.InputStream;
import java.io.IOException;

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
 * @author Thomas Morgner
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
}
