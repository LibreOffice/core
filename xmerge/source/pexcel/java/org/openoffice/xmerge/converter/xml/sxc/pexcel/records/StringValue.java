/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StringValue.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:04:28 $
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
import java.io.UnsupportedEncodingException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 * Represents a BIFF Record that describes the value of a formula that
 * evaluates to a string
 */
public class StringValue implements BIFFRecord {

    private byte[]  cch     = new byte[2];
    private byte[]  rgch;

    /**
      * Constructs a StringValue Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a StringValue Record
      */
    public StringValue(String str) throws IOException {
        cch = EndianConverter.writeShort((short) str.length());
        rgch = new byte[str.length()];
        rgch = str.getBytes("UTF-16LE");
    }

    /**
      * Constructs a StringValue Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a StringValue Record
      */
    public StringValue(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>StringValue</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.FORMULA_STRING;
    }

       /**
      * Reads a StringVlaue Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a StringValue Record
      */
    public int read(InputStream input) throws IOException {

        cch[0] = (byte) input.read();
        cch[1] = (byte) input.read();
        int numOfBytesRead = 1;

        int strlen = EndianConverter.readShort(cch)*2;
        rgch = new byte[strlen];
        numOfBytesRead  += input.read(rgch, 0, strlen);

        Debug.log(Debug.TRACE,"\tcch : "+ cch +
                            " rgch : " + rgch);

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(cch);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing StringValue record");
    }

    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {
        String name;

        try {
            name = new String(rgch, "UTF-16LE");
        } catch (UnsupportedEncodingException e){
            name = "unknown";
        }
        return name;
    }

}
