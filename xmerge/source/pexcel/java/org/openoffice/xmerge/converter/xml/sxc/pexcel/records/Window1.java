/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Window1.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:04:57 $
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
 * Represents a BIFF REcord that describes workbook window attributes
 */
public class Window1 implements BIFFRecord {

    private byte[] grbit    = new byte[2];
    private byte[] itabCur  = new byte[2];      // index of selected worksheet

    /**
      * Constructor
      */
    public Window1() {
        grbit   = EndianConverter.writeShort((short) 0);
        itabCur = EndianConverter.writeShort((short) 0);
    }

       /**
      * Constructs a Window1 Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a Window1 Record
      */
    public Window1(InputStream is) throws IOException{
        read(is);
    }

    /**
     * Set the number of the active sheet
     *
     * @param activeSheet number of the active sheet
     */
    public void setActiveSheet(int activeSheet) {
        itabCur = EndianConverter.writeShort((short) activeSheet);
    }

    /**
     * Get the number of the active sheet
     *
     * @return   number of the active sheet
     */
    public int getActiveSheet() {
        return EndianConverter.readShort(itabCur);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Window1</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.WINDOW_INFO;
    }

       /**
      * Reads a Window1 Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a Window1 Record
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(grbit);
        numOfBytesRead      += input.read(itabCur);

        Debug.log(Debug.TRACE,"\tgrbit : "+ EndianConverter.readShort(grbit) +
                            " itabCur : " + EndianConverter.readShort(itabCur));

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(grbit);
        output.write(itabCur);

        Debug.log(Debug.TRACE,"Writing Window1 record");
    }
}
