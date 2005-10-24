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
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 * Reperesent a BIFF Record descibing a cell containing a string
 */
public class LabelCell extends CellValue {

    private byte[] cch  = new byte[2];
    private byte[] rgch;

    /**
      * Constructs a <code>LabelCell</code> using specified attributes
     *
     * @param row row number
     * @param col column number
     * @param cellContents contents of the cell
     * @param ixfe font index
      */
    public LabelCell(int row, int column, String cellContents, int ixfe)
    throws IOException {

           setLabel(cellContents);
           setRow(row);
           setCol(column);
        setIxfe(ixfe);
    }

    /**
     * Reads a LabelCell from the <code>InputStream</code>
     *
     * @param is the <code>Inputstream</code> to read from
     */
    public LabelCell(InputStream is) throws IOException {
       read(is);
    }

    /**
     * Writes a <code>LabelCell</code> to the specified <code>Outputstream</code>
     *
     * @param os the <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());

        super.write(output);

        output.write(cch);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing Label record");
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>LabelCell</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.LABEL_CELL;
    }

    /**
     * Reads a<code>LabelCell</code> from the specified <code>InputStream</code>
     *
     * @param input the <code>InputStram</code> to read from
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        numOfBytesRead += input.read(cch);

        int strLen = EndianConverter.readShort(cch)*2;
        rgch = new byte[strLen];
        input.read(rgch, 0, strLen);

        Debug.log(Debug.TRACE, " cch : " + EndianConverter.readShort(cch) +
                            " rgch : " + new String(rgch, "UTF-16LE"));

        return numOfBytesRead;
    }


    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {
        return (new String(rgch,"UTF-16LE"));
    }

    /**
     * Sets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    private void setLabel(String cellContents) throws IOException {
        rgch = cellContents.getBytes("UTF-16LE");
        cch = EndianConverter.writeShort((short)cellContents.length());
    }
}
