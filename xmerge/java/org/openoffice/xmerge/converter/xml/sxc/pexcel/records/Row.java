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
 * Represents s BIFF Record that describes the format of a column
 */
public class Row implements BIFFRecord {

    private byte[] rw       = new byte[2];
    private byte[] miyRw    = new byte[2];
    private byte[] grbit    = new byte[2];
    private byte[] ixfe     = new byte[2];

    /**
     * Constructs a pocket Excel Document from the
     * <code>InputStream</code> and assigns it the document name passed in
     *
     * @param   is InputStream containing a Pocket Excel Data file.
     */
    public Row(int rw, int miyRw) {
        this.rw     = EndianConverter.writeShort((short) rw);
        this.miyRw  = EndianConverter.writeShort((short) miyRw);
        grbit   = EndianConverter.writeShort((short) 0);
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
        return PocketExcelBiffConstants.ROW_DESCRIPTION;
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
        numOfBytesRead      += input.read(grbit);
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE,"\trw : "+ EndianConverter.readShort(rw) +
                            " miyRw : " + EndianConverter.readShort(miyRw) +
                            " grbit : " + EndianConverter.readShort(grbit) +
                            " ixfe : " + EndianConverter.readShort(ixfe));
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(PocketExcelBiffConstants.ROW_DESCRIPTION);
        output.write(rw);
        output.write(miyRw);
        output.write(grbit);
        output.write(ixfe);

        Debug.log(Debug.TRACE,"Writing Row record");

    }

}
