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
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF Record that describes a blank cell
 */
public class BlankCell extends CellValue {

    /**
     * Constructs a BlankCell <code>InputStream</code>
     *
     * @param   is InputStream containing a BlankCell.
     */
    public BlankCell(InputStream is) throws IOException {
       read(is);
    }

    /**
      * Constructs a <code>BlankCell</code> using specified attributes
     *
     * @param row     row number
     * @param column  column number
     * @param ixfe    font index
      */
    public BlankCell(int row, int column, int ixfe) throws IOException {

        setRow(row);
           setCol(column);
        setIxfe(ixfe);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BlankCell</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BLANK_CELL;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rw);
        output.write(col);
        output.write(ixfe);

        Debug.log(Debug.TRACE, "Writing BlankCell record");

    }

    /**
     * Reads a BlankCell <code>InputStream</code>
     *
     * @param   input InputStream containing a BlankCell.
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        numOfBytesRead++;
        col                 += input.read();
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE, "\tRow : "+ EndianConverter.readShort(rw) +
                            " Column : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe));

        return numOfBytesRead;
    }

    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {

        return (new String(""));
    }
}
