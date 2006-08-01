/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FloatNumber.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:02:41 $
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

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF Record describing a floating point
 */
public class FloatNumber extends CellValue {

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

        setIxfe(ixfe);
        setRow(row);
           setCol(column);
        double cellLong = (double) Double.parseDouble(cellContents);
        num     = EndianConverter.writeDouble(cellLong);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>FloatNumber</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.NUMBER_CELL;
    }

    /**
     * Reads a<code>FloatNumber</code> from the specified <code>InputStream</code>
     *
     * @param input the <code>InputStram</code> to read from
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        numOfBytesRead += input.read(num);

        Debug.log(Debug.TRACE," num : " + getString());
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());

        super.write(output);

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
