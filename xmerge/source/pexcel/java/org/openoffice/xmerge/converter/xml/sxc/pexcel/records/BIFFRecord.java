/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BIFFRecord.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:59:49 $
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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.DataInputStream;
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

