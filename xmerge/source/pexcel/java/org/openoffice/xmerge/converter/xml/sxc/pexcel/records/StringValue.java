/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

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
      * Constructs a StringValue Record from a string
      *
      * @param  str String containing value
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
      * @param  input InputStream containing a StringValue Record
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
