/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BlankCell.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:00:17 $
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
 * Represents a BIFF Record that describes a blank cell
 */
public class BlankCell extends CellValue {

    /**
     * Constructs a BlankCell <code>InputStream</code>
     *
     * @param   is InputStream containing a BlankCell.
     */
    public BlankCell(InputStream is) throws IOException {
       read(is);
    }

    /**
      * Constructs a <code>BlankCell</code> using specified attributes
     *
     * @param row row number
     * @param col column number
     * @param cellContents contents of the cell
     * @param ixfe font index
      */
    public BlankCell(int row, int column, int ixfe) throws IOException {

        setRow(row);
           setCol(column);
        setIxfe(ixfe);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BlankCell</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BLANK_CELL;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rw);
        output.write(col);
        output.write(ixfe);

        Debug.log(Debug.TRACE, "Writing BlankCell record");

    }

    /**
     * Reads a BlankCell <code>InputStream</code>
     *
     * @param   is InputStream containing a BlankCell.
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        numOfBytesRead++;
        col                 += input.read();
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE, "\tRow : "+ EndianConverter.readShort(rw) +
                            " Column : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe));

        return numOfBytesRead;
    }

    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {

        return (new String(""));
    }
}
