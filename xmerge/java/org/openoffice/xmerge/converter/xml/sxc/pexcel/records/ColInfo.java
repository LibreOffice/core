/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.IOException;
import java.io.DataInputStream;
import java.io.InputStream;
import java.io.OutputStream;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * ColInfo describes the formatting for a column
 *
 */
public class ColInfo implements BIFFRecord {

    private byte[] colFirst = new byte[2];  // first column this formatting applies to
    private byte[] colLast  = new byte[2];  // last column this formatting applies to
    private byte[] colDX    = new byte[2];  // column width
    private byte[] ixfe     = new byte[2];  // index for formatting
    private byte   grbit;                   // options flags
    private float  scale = (float) 1.798;

    /**
      * Constructs a pocket Excel Document from the
      * <code>InputStream</code> and assigns it the document name passed in
      *
      * @param  colFirst    the first column this formatting applies to
      * @param  colLast     last column this formatting applies to
      * @param  coldx       column width
      * @param  grbit       options flags
      */
    public ColInfo(int colFirst, int colLast, int colDX, byte grbit) {
        this.colFirst   = EndianConverter.writeShort((short)colFirst);
        this.colLast    = EndianConverter.writeShort((short)colLast);
        colDX *= scale;
        this.colDX      = EndianConverter.writeShort((short)colDX);
        this.ixfe       = EndianConverter.writeShort((short)0);
        this.grbit  = grbit;
    }

    /**
     * Construct a ColInfo from the InputStream
     *
     * @param is the <code>Inputstream</code> to read from
     */
    public ColInfo(InputStream is) throws IOException {
        read(is);
        short scaledDX = (short) (EndianConverter.readShort(colDX) / scale);
        colDX = EndianConverter.writeShort((short)scaledDX);;
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(colFirst);
        numOfBytesRead      += input.read(colLast);
        numOfBytesRead      += input.read(colDX);
        numOfBytesRead      += input.read(ixfe);
        grbit               = (byte) input.read();
        numOfBytesRead      ++;

        Debug.log(Debug.TRACE,"\tcolFirst : "+ EndianConverter.readShort(colFirst) +
                            " colLast : " + EndianConverter.readShort(colLast) +
                            " colDX : " + EndianConverter.readShort(colDX) +
                            " ixfe : " + EndianConverter.readShort(ixfe) +
                            " grbit : " + grbit);

        return numOfBytesRead;
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>ColInfo</code>
     */
    public short getBiffType() {
        return PocketExcelBiffConstants.COLINFO;
    }
    /**
     * Get the width of this column
     *
     * @return the width of this column
     */
    public short getColWidth() {
        return EndianConverter.readShort(colDX);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>ColInfo</code>
     */
    public short getFirst() {
        return EndianConverter.readShort(colFirst);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>ColInfo</code>
     */
    public short getLast() {
        return EndianConverter.readShort(colLast);
    }

    /**
     * Writes a ColInfo to the specified <code>Outputstream</code>
     *
     * @param os the <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(PocketExcelBiffConstants.COLINFO);
        output.write(colFirst);
        output.write(colLast);
        output.write(colDX);
        output.write(ixfe);
        output.write(grbit);

        Debug.log(Debug.TRACE,"Writing ColInfo record");

    }

}
