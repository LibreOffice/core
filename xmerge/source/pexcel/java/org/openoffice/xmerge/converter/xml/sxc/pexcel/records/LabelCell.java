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

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Reperesent a BIFF Record descibing a cell containing a string
 */
public class LabelCell extends CellValue {

    private byte[] cch  = new byte[2];
    private byte[] rgch;

    /**
      * Constructs a <code>LabelCell</code> using specified attributes
     *
     * @param row          row number
     * @param column       column number
     * @param cellContents contents of the cell
     * @param ixfe         font index
      */
    public LabelCell(int row, int column, String cellContents, int ixfe)
    throws IOException {

           setLabel(cellContents);
           setRow(row);
           setCol(column);
        setIxfe(ixfe);
    }

    /**
     * Reads a LabelCell from the <code>InputStream</code>
     *
     * @param is the <code>Inputstream</code> to read from
     */
    public LabelCell(InputStream is) throws IOException {
       read(is);
    }

    /**
     * Writes a <code>LabelCell</code> to the specified <code>Outputstream</code>
     *
     * @param output The <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());

        super.write(output);

        output.write(cch);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing Label record");
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>LabelCell</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.LABEL_CELL;
    }

    /**
     * Reads a<code>LabelCell</code> from the specified <code>InputStream</code>
     *
     * @param input the <code>InputStram</code> to read from
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        numOfBytesRead += input.read(cch);

        int strLen = EndianConverter.readShort(cch)*2;
        rgch = new byte[strLen];
        input.read(rgch, 0, strLen);

        Debug.log(Debug.TRACE, " cch : " + EndianConverter.readShort(cch) +
                            " rgch : " + new String(rgch, "UTF-16LE"));

        return numOfBytesRead;
    }

    /**
     * Gets the <code>String</code> representing the cells contents
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {
        return (new String(rgch,"UTF-16LE"));
    }

    /**
     * Sets the <code>String</code> representing the cells contents
     */
    private void setLabel(String cellContents) throws IOException {
        rgch = cellContents.getBytes("UTF-16LE");
        cch = EndianConverter.writeShort((short)cellContents.length());
    }
}
