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

import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * Represents a BIFF Record descibing a font used
 */
public class FontDescription implements BIFFRecord {

    private byte[] dwHeight     = new byte[2];
    private byte[] grbit        = new byte[2];
    private byte[] Reserved1    = new byte[2];
    private byte[] bls          = new byte[2];
    private byte[] Reserved2    = new byte[2];
    private byte uls;
    private byte bFamily;
    private byte bCharSet;
    private byte Reserved3;
    private byte cch;
    private byte[] rgch;

    /**
      * Constructs a FontDescription from the bold italic and undelrine attributes
      *
      * @param  italic      Italic attribute
      * @param  bold        Bold attribute
      * @param  underline   Underline attribute
      */
    public FontDescription(Format fmt) throws IOException {

        Debug.log(Debug.TRACE,"Building FontDescriptor based on Format : " + fmt);

        this.dwHeight   = EndianConverter.writeShort((short) 200);
        if (fmt.getAttribute(Format.ITALIC))
            this.grbit      = EndianConverter.writeShort((short) 2);
        else
            this.grbit      = EndianConverter.writeShort((short) 0);

        if (fmt.getAttribute(Format.BOLD))
            this.bls        = EndianConverter.writeShort((short) 700);
        else
            this.bls        = EndianConverter.writeShort((short) 400);

        if (fmt.getAttribute(Format.UNDERLINE))
            this.uls        = 1;
        else
            this.uls        = 0;


        this.bFamily        = 0;
        this.bCharSet       = 0;
        this.cch            = 6;
        this.rgch           = (new String("Tahoma")).getBytes("UTF-16LE");


        Reserved1       = EndianConverter.writeShort((short) 0);
        Reserved2       = EndianConverter.writeShort((short) 0);
        Reserved3       = 0;

    }

    public boolean isItalic() {

        return (EndianConverter.readShort(grbit) == 2);
    }

    public boolean isUnderline() {

        return (uls == 1);
    }

    public boolean isBold() {

        return (EndianConverter.readShort(bls) == 700);
    }

    public boolean compareTo(FontDescription rhs) {

        if (this.isBold() == rhs.isBold() &&
            this.isItalic() == rhs.isItalic() &&
            this.isUnderline() == rhs.isUnderline())
            return true;
        else
            return false;
    }


    /**
      * Constructs a Font Description from the <code>InputStream</code>
      *
      * @param  is InputStream containing a <code>FontDescription</code>
      */
    public FontDescription(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>FontDescription</code>
     */
    public short getBiffType() {
        return PocketExcelBiffConstants.FONT_DESCRIPTION;
    }

    /**
      * Constructs a Font Description from the <code>InputStream</code>
      *
      * @param  is InputStream containing a <code>FontDescription</code>
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(dwHeight);
        numOfBytesRead      += input.read(grbit);
        numOfBytesRead      += input.read(Reserved1);
        numOfBytesRead      += input.read(bls);
        numOfBytesRead      += input.read(Reserved2);
        uls                 = (byte) input.read();
        bFamily             = (byte) input.read();
        bCharSet            = (byte) input.read();
        Reserved3           = (byte) input.read();
        cch                 = (byte) input.read();
        numOfBytesRead += 5;

        rgch = new byte[cch*2];
        input.read(rgch, 0, cch*2);

        Debug.log(Debug.TRACE,"\tdwHeight : "+ EndianConverter.readShort(dwHeight) +
                            " grbit : " + EndianConverter.readShort(grbit) +
                            " bls : " + EndianConverter.readShort(bls) +
                            " uls : " + uls +
                            "\n\tFamily : " + bFamily +
                            " bCharSet : " + bCharSet +
                            " cch : " + cch +
                            " rgch : " + new String(rgch,"UTF-16LE"));

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(dwHeight);
        output.write(grbit);
        output.write(Reserved1);
        output.write(bls);
        output.write(Reserved2);
        output.write(uls);
        output.write(bFamily);
        output.write(bCharSet);
        output.write(Reserved3);
        output.write(cch);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing FontDescription record");
    }

}
