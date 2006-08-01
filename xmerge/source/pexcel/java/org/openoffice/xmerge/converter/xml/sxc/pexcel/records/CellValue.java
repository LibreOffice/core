/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CellValue.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:00:56 $
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

public abstract class CellValue implements BIFFRecord {

    protected byte[] rw = new byte[2];
    protected byte   col;
    protected byte[] ixfe = new byte[2];

       /**
     * Get the row number of this cell
     *
     * @return the row number of this cell
     */
    public int getRow() {
        return EndianConverter.readShort(rw) + 1;
    }

    /**
     * Set the row number of this cell
     *
     * @param row sets the row number for this cell
     */
    public void setRow(int row) {
        this.rw = EndianConverter.writeShort((short) (row - 1));
    }
       /**
     * Get the Index to the <code>ExtendedFormat</code>
     *
     * @return the index number of this cell's <code>ExtendedFormat</code>
     */
    public int getIxfe() {
        return EndianConverter.readShort(ixfe);
    }

    /**
     * Sets the Index to the <code>ExtendedFormat</code>
     *
     * @param ixfe sets the index number for this cell's <code>ExtendedFormat</code>
     */
    public void setIxfe(int ixfe) {
        this.ixfe = EndianConverter.writeShort((short) (ixfe));
    }

    /**
     * Get the column number of this cell
     *
     * @return the column number of this cell
     */
    public int getCol() {
        return col + 1;         // The cols start at 1
    }

    /**
     * Set the row number of this cell
     *
     * @param col sets the row number for this cell
     */
    public void setCol(int col) {
        this.col = (byte) (col - 1);        // The cols start at 1
    }

    /**
     * Writes basic cell value attributes to the specified <code>Outputstream</code>
     *
     * @param os the <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(rw);
        output.write(col);
        output.write(ixfe);
    }

    /**
     * Writes a<code>LabelCell</code> to the specified <code>Outputstream</code>
     *
     * @param os the <code>OutputStream</code> to write to
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        col                 += input.read();
        numOfBytesRead++;
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE, "\tRow : "+ EndianConverter.readShort(rw) +
                            " Column : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe));

        return numOfBytesRead;
    }


    /**
     * Returns the contents of the cell as a String
     *
     * @return the contents of the cell
     */
    abstract public String getString() throws IOException;

}

