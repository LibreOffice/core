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

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.OutputStream;
import java.io.InputStream;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BoundSheet Record which describes the name of a worksheet
 */
public class BoundSheet implements BIFFRecord {

    private byte    reserved;
    private byte    cch;
    private byte[]  sheetName;

    /**
     * Constructs a pocket Excel Document assigns it the document name passed in
      *
     * @param   name name of the worksheet represented
     */
    public BoundSheet(String name) throws IOException {
        setSheetName(name);
        reserved = 0;
    }

    /**
      * Constructs a pocket Excel Document from the
      * <code>InputStream</code> and assigns it the document name passed in
      *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public BoundSheet(InputStream is) throws IOException {
        read(is);
    }

    /**
      * Sets the worksheet name. The sheetname length must be doubled as the
     * String is stored in unicode format.
      *
      * @param  sheetName   worksheet name
      */
    void setSheetName(String sheetName) throws IOException {
        this.cch        = (byte) sheetName.length();
        this.sheetName  = new byte[cch*2];
        this.sheetName  = sheetName.getBytes("UTF-16LE");
    }

    public String getSheetName() {
        String name;

        try {
            name = new String(sheetName, "UTF-16LE");
        } catch (UnsupportedEncodingException e){
            name = "unknown";
        }
        return name;
    }

     /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BoundSheet</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BOUND_SHEET;
    }

     /**
     * Write this particular <code>BIFFRecord</code> to the <code>OutputStream</code>
     *
     * @param output the <code>OutputStream</code>
     */
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(reserved);
        output.write(cch);
        output.write(sheetName);

        Debug.log(Debug.TRACE,"Writing BoundSheet record");
    }

    /**
      * Reads a BoundSheet from the <code>InputStream</code> The byte array
     * must be twice the size of the String as it uses unicode.
      *
      * @param  input InputStream containing the record data
      */
    public int read(InputStream input) throws IOException {

        reserved            = (byte) input.read();
        cch                 = (byte) input.read();
        int numOfBytesRead = 2;
        int strLen = cch*2;
        sheetName   = new byte[strLen];
        numOfBytesRead      += input.read(sheetName, 0, strLen);

           Debug.log(Debug.TRACE,"\tReserved : "+ reserved +
                            " cch : " + cch +
                            " sheetName : " + new String(sheetName,"UTF-16LE"));

        return numOfBytesRead;
    }

}
