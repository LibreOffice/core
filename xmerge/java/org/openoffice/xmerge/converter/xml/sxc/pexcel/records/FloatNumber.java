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

import java.io.DataInputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * Represents a BIFF Record describing a floating point
 */
public class FloatNumber extends CellValue {

    protected byte[] ixfe   = new byte[2];
    protected byte[] num  = new byte[8];

    /**
      * Constructs a pocket Excel Document from the
      * <code>InputStream</code> and assigns it the document name passed in
     *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public FloatNumber(InputStream is) throws IOException {
        read(is);
    }

    /**
      * Constructs a <code>FloatNumber</code> using specified attributes
     *
     * @param row row number
     * @param col column number
     * @param cellContents contents of the cell
     * @param ixfe font index
      */
    public FloatNumber(int row, int column, String cellContents, int ixfe) throws IOException {

        setRow(row);
           setCol(column);
        this.ixfe   = EndianConverter.writeShort((short) (ixfe));
        double cellLong = (double) Double.parseDouble(cellContents);
        num     = EndianConverter.writeDouble(cellLong);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>FloatNumber</code>
     */
    public short getBiffType() {
        return PocketExcelBiffConstants.NUMBER_CELL;
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        col                 = (byte) input.read();
        numOfBytesRead++;
        numOfBytesRead  = input.read(ixfe);
        numOfBytesRead  = input.read(num);

        Debug.log(Debug.TRACE,"\trw : "+ EndianConverter.readShort(rw) +
                            " col : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe) +
                            " num : " + getString());
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rw);
        output.write(col);
        output.write(ixfe);
        output.write(num);

        Debug.log(Debug.TRACE,"Writing FloatNumber record");
    }


    /**
     * Gets the numerical value the cell represents
     *
     * @return the <code>String</code> representing a double value
     */
    public String getString() throws IOException {

        double value = EndianConverter.readDouble(num);
        Double myDo = new Double(value);
        return myDo.toString();
    }

}
