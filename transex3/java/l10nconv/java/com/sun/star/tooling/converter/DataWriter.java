/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataWriter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:08:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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