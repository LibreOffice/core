/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

 public interface BIFFRecord {

    /**
     * Get the type of the record. In the BIFF file format each record has a type
     * designated with a byte value. See @link PocketExcelBiffConstants
     * for a list of the BIFF constants and what they mean.
     *
     * @return byte The BIFF record value. 
     */
    public short getBiffType();
    
    /**
     * Read from the input stream <b>NB</b>The input stream is assumed to be in 
     * Little Endian format. The Biff identifier is expected to be in the stream.
     *
     *  @param input The InputStream to read from.
     *  @return The number of bytes that were read in.
     */
    public int read(InputStream input) throws IOException;
    
    /**
     * Writes the record, including the BIFF record byte to the outputstream
     * @param output The output stream to write to in LittleEndian format.
     */
    public void write(OutputStream output) throws IOException;
    
}

