/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataWriter.java,v $
 * $Revision: 1.3 $
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
/*
 * DataWriter.java
 *
 * parent of all XXXWriter classes used by
 * Converter
 */

package com.sun.star.tooling.converter;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

/**
 * The abstract parent class of all converter writer classes
 *
 * @author Christian Schmidt
 */
abstract public class DataWriter extends OutputStreamWriter {
    protected final String seperator = new String("|");

    /**
     * the char sequence used as line seperator
     */
    protected final String lineEnd   = java.lang.System.getProperty(
                                             "line.seperator", "\n");

    /** Creates a new instance of DataWriter */
    /**
     * @param bos the buffered output stream holding the data
     * @param encoding the encoding to use for read from bos
     * @throws java.io.UnsupportedEncodingException
     */
    public DataWriter(BufferedOutputStream bos, String encoding)
            throws java.io.UnsupportedEncodingException {
        super(bos, encoding);
    }

//    abstract protected void writeData(Map[] data) throws java.io.IOException;

    /**
     * get the data that should be written from the DataHandler
     *
     * @param handler the DataHandler having the data
     * @throws java.io.IOException
     */
    abstract protected void getDataFrom(DataHandler handler)
            throws java.io.IOException;

    /**
     * write the Data
     *
     * @throws java.io.IOException
     */
    abstract protected void writeData() throws java.io.IOException;

    /**
     * @param handler
     * @throws IOException
     */
    abstract protected void getDatafrom(DataHandler handler) throws IOException;
}