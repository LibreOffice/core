/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Row.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:04:05 $
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
 * Represents s BIFF Record that describes the format of a column
 */
public class Row implements BIFFRecord {

    private byte[] rw       = new byte[2];
    private byte[] miyRw    = new byte[2];
    private byte[] grbit    = new byte[2];
    private byte[] ixfe     = new byte[2];
    private float  scale = (float) 1;

    /**
     * Constructs a pocket Excel Document from the
     * <code>InputStream</code> and assigns it the document name passed in
     *
     * @param   rw Zero based row number
     * @param   miyRw row height
     */
    public Row(int rw, int miyRw, boolean userDefined) {
        this.rw     = EndianConverter.writeShort((short) rw);
        miyRw *= scale;
        this.miyRw  = EndianConverter.writeShort((short) miyRw);
        if(userDefined) {
            grbit   = EndianConverter.writeShort((short) 2);
        } else {
            grbit   = EndianConverter.writeShort((short) 0);
        }
        ixfe    = EndianConverter.writeShort((short) 0);
    }

    /**
     * Constructs a Row fro man <code>InputStream</code>
     *
     * @param   is InputStream containing a Pane Record
     */
    public Row(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Row</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.ROW_DESCRIPTION;
    }

    /**
     * Get the height of this row
     *
     * @return the height of this row
     */
    public short getRowHeight() {
        return EndianConverter.readShort(miyRw);
    }

    /**
     * Get the rown number for this style
     *
     * @return the row this style applies to
     */
    public short getRowNumber() {
        return EndianConverter.readShort(rw);
    }

    /**
     * Reads a Row from an <code>InputStream</code>
     *
     * @param   is InputStream containing a Pane Record
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        numOfBytesRead      += input.read(miyRw);
        short scaledHeight = (short) (EndianConverter.readShort(miyRw) / scale);
        miyRw = EndianConverter.writeShort(scaledHeight);
        numOfBytesRead      += input.read(grbit);
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE,"\trw : "+ EndianConverter.readShort(rw) +
                            " miyRw : " + EndianConverter.readShort(miyRw) +
                            " grbit : " + EndianConverter.readShort(grbit) +
                            " ixfe : " + EndianConverter.readShort(ixfe));
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rw);
        output.write(miyRw);
        output.write(grbit);
        output.write(ixfe);

        Debug.log(Debug.TRACE,"Writing Row record");

    }

}
