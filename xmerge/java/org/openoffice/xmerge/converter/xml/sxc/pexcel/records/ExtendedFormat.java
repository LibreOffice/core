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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.Records;

import java.io.DataInputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * Represents a BIFF Record descibing extended formatting information
 *
 */
public class ExtendedFormat implements BIFFRecord {

    private byte[] ixfnt        = new byte[2];  // Index to Font Record
    private byte[] ixnf         = new byte[2];
    private byte[] fattributes  = new byte[4];
    private byte[] fBaseAttr    = new byte[2];  // base attribute flags
    private byte[] fTextAttr    = new byte[2];  // text attribute flags
    private byte[] icvFore      = new byte[2];  // Background colour of the cell
    private byte[] icvFill      = new byte[2];
    private byte bRight;                        // Right border Style
    private byte bTop;                          // Top border style
    private byte bLeft;                         // Left border style
    private byte bBottom;                       // Bottom border style
    private byte backstyle;
    private byte borderstyle;

    /**
      * Constructs an <code>ExtendedFormat</code> from the
      * <code>InputStream</code>
      *
      * @param  is <code>InputStream</code> to read from
      */
    public ExtendedFormat(InputStream is) throws IOException {
        read(is);
    }

    /**
      * Constructs a pocket Excel Document using defualt values and sets the
     * font index using the specified attribute
      *
      * @param  ixfnt   index of the font this format should use
      */
    public ExtendedFormat(int ixfnt) {

        this.ixfnt          = EndianConverter.writeShort((short)ixfnt);
        this.ixnf           = EndianConverter.writeShort((short) 0);
        this.fattributes    = new byte[] {(byte)0xFF,(byte)0xFF,(byte)0xFF,(byte)0xFF};
        this.fBaseAttr      = new byte[] {(byte)0x02,(byte)0x00};
        this.fTextAttr      = new byte[] {(byte)0x30,(byte)0x00};
        this.icvFore        = new byte[] {(byte)0xFF,(byte)0x00};
        this.icvFill        = new byte[] {(byte)0xFF,(byte)0x00};
        this.bRight         = (byte) 0xFF;
        this.bTop           = (byte) 0xFF;
        this.bLeft          = (byte) 0xFF;
        this.bBottom        = (byte) 0xFF;
        this.backstyle      = (byte) 0x00;
        this.borderstyle    = (byte) 0x00;

    }

    /**
     * Get the font index this format uses
     *
     * @return the font index
     */
    public int getIndex() {
        return EndianConverter.readShort(ixfnt);
    }

    /**
     * Compare two ExtendedFormat to see if the font index is the same
     *
     * @param the ExtendedFormat to be used in the comaprison
     * @return boolean if the two are the same otherwise false
     */
    public boolean compareTo(ExtendedFormat rhs) {

        if (this.getIndex() == rhs.getIndex())
            return true;
        else
            return false;
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>ExtendedFormat</code>
     */
    public short getBiffType() {
        return PocketExcelBiffConstants.EXTENDED_FORMAT;
    }

    /**
      * Reads the extended format from the <code>Inputstream</code>
      *
      * @param  input the <code>Inputstream</code>to read
      * @return toal number of bytes read
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(ixfnt);
        numOfBytesRead  += input.read(ixnf);
        numOfBytesRead  += input.read(fattributes);
        numOfBytesRead  += input.read(fBaseAttr);
        numOfBytesRead  += input.read(fTextAttr);
        numOfBytesRead  += input.read(icvFore);
        numOfBytesRead  += input.read(icvFill);
        bRight      = (byte) input.read();
        bTop        = (byte) input.read();
        bLeft       = (byte) input.read();
        bBottom     = (byte) input.read();
        backstyle   = (byte) input.read();
        borderstyle = (byte) input.read();
        numOfBytesRead += 6;

        Debug.log(Debug.TRACE,"\tixfnt : "+ EndianConverter.readShort(ixfnt) +
                            " ixnf : " + EndianConverter.readShort(ixnf) +
                            " fattributes : " + EndianConverter.readInt(fattributes) +
                            " fBaseAttr : " + EndianConverter.readShort(fBaseAttr) +
                            "\n\tfTextAttr : " + EndianConverter.readShort(fTextAttr) +
                            " icvFore : " + EndianConverter.readShort(icvFore) +
                            " icvFill : " + EndianConverter.readShort(icvFill) +
                            " bRight : " + bRight +
                            "\n\tbTop : " + bTop +
                            " bLeft : " + bLeft +
                            " bBottom : " + bBottom +
                            " backstyle : " + backstyle +
                            " borderstyle : " + borderstyle);
        return numOfBytesRead;
    }

    /**
      * Writes the ExtendedFormat to the <code>Outputstream<code>
      *
      * @param  output the <code>Outputstream</code>to write to
      */
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(ixfnt);
        output.write(ixnf);
        output.write(fattributes);
        output.write(fBaseAttr);
        output.write(fTextAttr);
        output.write(icvFore);
        output.write(icvFill);
        output.write(bRight);
        output.write(bTop);
        output.write(bLeft);
        output.write(bBottom);
        output.write(backstyle);
        output.write(borderstyle);

        Debug.log(Debug.TRACE,"Writing ExtendedFormat record");

    }

}
