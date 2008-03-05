/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputRepository.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:22:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    public Object getId();

    public InputStream createInputStream(String name) throws IOException;

    /** allows to acces sub repositories inside this repository
     *
     * @param name describes the path to the sub repository
     * @return the sub repository
     * @throws java.io.IOException when the sub repository doesn't exist.
     */
    public InputRepository openInputRepository(final String name) throws IOException;

    /**
     * This returns an version number for the given resource. Return zero, if
     * the resource is not versionable, else return a unique number for each version.
     * As rule of thumb: Increase the version number by at least one for each change
     * made to the resource.
     *
     * @param name the name of the resource
     * @return the version number
     */
    public long getVersion(String name);

    public boolean exists(String name);

    public boolean isReadable(String name);

    public void closeInputRepository();
}
