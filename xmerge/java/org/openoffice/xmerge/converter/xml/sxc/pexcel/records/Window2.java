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
import java.awt.Point;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;
import org.openoffice.xmerge.converter.xml.sxc.SheetSettings;


/**
 * Represents a BIFF Record that describes worksheet window attributes
 */
public class Window2 implements BIFFRecord {

    private final static int FROZEN     = 0x08;
    private final static int NOSPLIT    = 0x01;

    private byte[] rwTop    = new byte[2];
    private byte   colLeft;
    private byte[] grbit    = new byte[2];

    /**
     * Constructor
     */
    public Window2() {
        this.rwTop      = EndianConverter.writeShort((short) 0);
        this.colLeft    = 0;
        this.grbit      = EndianConverter.writeShort((short) 0);
    }

       /**
      * Constructs a Window2 Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a Window2 Record
      */
    public Window2(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Window2</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.SHEET_WINDOW_INFO;
    }

    /**
     * Sets the split type for this pane, the split type is the same for both
     * x and y so we only test against one.
     *
     * @param splitType the split type based on types defined in
     * <code>sheetSettings</code>
     */
    public void setSplitType(Point splitType) {
        if(splitType.getX()==SheetSettings.SPLIT) {
            grbit[0] &= ~FROZEN;
            grbit[1] &= ~NOSPLIT;
        } else {
            grbit[0] |= FROZEN;
            grbit[1] |= NOSPLIT;
        }
    }

    /**
     * This method tests if this object describes a freeze
     *
     * @return true if freeze otherwise false
     */
     public boolean isFrozen() {
        if((grbit[0] & FROZEN) != FROZEN)
            return false;

        if((grbit[1] & NOSPLIT) != NOSPLIT)
            return false;

        return true;
    }

    /**
     * This method tests if this object describes a split
     *
     * @return true if split otherwise false
     */
     public boolean isSplit() {
        if((grbit[0] & FROZEN) == FROZEN)
            return false;

        if((grbit[1] & NOSPLIT) == NOSPLIT)
            return false;

        return true;
    }

       /**
      * Reads a Window2 Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a Window2 Record
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rwTop);
        colLeft             = (byte) input.read();
        numOfBytesRead++;
        numOfBytesRead      += input.read(grbit);

        Debug.log(Debug.TRACE,"\trwTop : "+ EndianConverter.readShort(rwTop) +
                            " colLeft : " + colLeft +
                            " grbit : " + EndianConverter.readShort(grbit));
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rwTop);
        output.write(colLeft);
        output.write(grbit);

        Debug.log(Debug.TRACE,"Writing Window2 record");
    }
}
