/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CodePage.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:01:10 $
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
 * Represents the codepage for the document. There is a number of unknown
 * fields which are hardcoded at construction
 */
public class CodePage implements BIFFRecord {

    private byte[] codepage = new byte[2];
    private byte[] unknown1 = new byte[2];
    private byte[] unknown2 = new byte[2];
    private byte unknown3;

    /**
     * Constructs a pocket Excel Codepage
     */
    public CodePage() {
        codepage    = new byte[] {(byte)0xE4, (byte)0x04};
        unknown1    = new byte[] {(byte)0x8C, (byte)0x01};
        unknown2    = new byte[] {(byte)0x00, (byte)0x01};
        unknown3    = 0x00;
    }

    /**
     * Constructs a pocket Excel Codepage from the<code>InputStream</code>
     *
     * @param   is InputStream containing a Pocket Excel Data file.
     */
    public CodePage(InputStream is) throws IOException {
        read(is);
    }

     /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BoundSheet</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.CODEPAGE;
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(codepage);
        numOfBytesRead      += input.read(unknown1);
        numOfBytesRead      += input.read(unknown2);
        // numOfBytesRead       += input.read(unknown3);
        unknown3            = (byte) input.read();
        numOfBytesRead++;

        Debug.log(Debug.TRACE,"\tcodepage : "+ EndianConverter.readShort(codepage) +
                            " unknown1 : " + EndianConverter.readShort(unknown1) +
                            " unknown2 : " + EndianConverter.readShort(unknown2) +
                            " unknown3 : " + unknown3);

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(codepage);
        output.write(unknown1);
        output.write(unknown2);
        output.write(unknown3);

        Debug.log(Debug.TRACE,"Writing CodePage record");


    }

}
