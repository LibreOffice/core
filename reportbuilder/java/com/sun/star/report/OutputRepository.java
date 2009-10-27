/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OutputRepository.java,v $
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
    OutputStream createOutputStream(final String name, final String mimeType) throws IOException;

    /** allows to acces sub repositories inside this repository
     *
     * @param name describes the path to the sub repository
     * @return the sub repository
     * @throws java.io.IOException when the sub repository doesn't exist.
     */
    OutputRepository openOutputRepository(final String name, final String mimeType) throws IOException;

    boolean exists(final String name);

    boolean existsStorage(final String name);

    boolean isWritable(final String name);

    void closeOutputRepository();
}
