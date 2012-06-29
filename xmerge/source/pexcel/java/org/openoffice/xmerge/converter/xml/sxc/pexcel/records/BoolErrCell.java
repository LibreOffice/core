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

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF Record that describes a Boolean or Error value
 */
public class BoolErrCell extends CellValue {

    private byte   bBoolErr;
    private byte   fError;

     /**
     * Constructs a BoolErrCell from arguments
      *
     * @param row      row number
     * @param column   column number
     * @param ixfe     font index
     * @param bBoolErr Boolean value or error value
     * @param fError   Boolean error flag
      */
    public BoolErrCell(int row, int column, int ixfe, int bBoolErr, int fError) throws IOException {

        setIxfe(ixfe);
        this.bBoolErr = (byte)bBoolErr;
        this.fError = (byte)fError;
        setRow(row);
        setCol(column);
    }

    /**
     * Constructs a BoolErrCell from the <code>InputStream</code>
      *
      * @param  is InputStream containing a BoolErrCell
      */
    public BoolErrCell(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BoolErrCEll</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BOOLERR_CELL;
    }

    /**
     * Writes a <code>BoolErrCell</code> to the specified <code>Outputstream</code>
     *
     * @param output the <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());

        super.write(output);

        output.write(bBoolErr);
        output.write(fError);

        Debug.log(Debug.TRACE,"Writing BoolErrCell record");
    }

    /**
     * Reads a BoolErrCell from the <code>InputStream</code>
      *
      * @param  input InputStream containing a BoolErrCell
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        bBoolErr            = (byte) input.read();
        fError              = (byte) input.read();
           numOfBytesRead += 2;

        Debug.log(Debug.TRACE, " bBoolErr : " + bBoolErr +
                            " fError : " + fError);
        return numOfBytesRead;
    }

    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {
        return ("Error Cell");
    }
}
