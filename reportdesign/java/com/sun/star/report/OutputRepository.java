/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutputRepository.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:23:46 $
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

import java.io.OutputStream;
import java.io.IOException;

/**
 * A repository for writing. Providing a repository always assumes,
 * that more than one stream can be written.
 *
 * @author Thomas Morgner
 */
public interface OutputRepository
{

    /**
     * Creates an output stream for writing the data. If there is an entry with
     * that name already contained in the repository, try to overwrite it.
     *
     * @param name
     *    the name of the output stream
     * @param mimeType
     *    the mime type of the to-be-created output stream. Repository implementations which do not support
     *    associating a mime time with a stream might ignore this parameter.
     * @return the outputstream
     * @throws IOException if opening the stream fails
     */
    public OutputStream createOutputStream(String name, String mimeType) throws IOException;

    /** allows to acces sub repositories inside this repository
     *
     * @param name describes the path to the sub repository
     * @return the sub repository
     * @throws java.io.IOException when the sub repository doesn't exist.
     */
    public OutputRepository openOutputRepository(final String name) throws IOException;

    public boolean exists(String name);

    public boolean existsStorage(String name);

    public boolean isWritable(String name);

    public void closeOutputRepository();
}
