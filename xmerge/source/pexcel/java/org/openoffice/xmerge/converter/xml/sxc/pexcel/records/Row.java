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
 * Represents s BIFF Record that describes the format of a column
 */
public class Row implements BIFFRecord {

    private byte[] rw       = new byte[2];
    private byte[] miyRw    = new byte[2];
    private byte[] grbit    = new byte[2];
    private byte[] ixfe     = new byte[2];
    private float  scale = 1;

    /**
     * Constructs a pocket Excel Document from the
     * <code>InputStream</code> and assigns it the document name passed in
     *
     * @param   rw Zero based row number
     * @param   miyRw row height
     */
    public Row(int rw, int miyRw, boolean userDefined) {
        this.rw     = EndianConverter.writeShort((short) rw);
        miyRw *= scale;
        this.miyRw  = EndianConverter.writeShort((short) miyRw);
        if(userDefined) {
            grbit   = EndianConverter.writeShort((short) 2);
        } else {
            grbit   = EndianConverter.writeShort((short) 0);
        }
        ixfe    = EndianConverter.writeShort((short) 0);
    }

    /**
     * Constructs a Row fro man <code>InputStream</code>
     *
     * @param   is InputStream containing a Pane Record
     */
    public Row(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Row</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.ROW_DESCRIPTION;
    }

    /**
     * Get the height of this row
     *
     * @return the height of this row
     */
    public short getRowHeight() {
        return EndianConverter.readShort(miyRw);
    }

    /**
     * Get the row number
     *
     * @return the row this style applies to
     */
    public short getRowNumber() {
        return EndianConverter.readShort(rw);
    }

    /**
     * Reads a Row from an <code>InputStream</code>
     *
     * @param   input InputStream containing a Row Record
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        numOfBytesRead      += input.read(miyRw);
        short scaledHeight = (short) (EndianConverter.readShort(miyRw) / scale);
        miyRw = EndianConverter.writeShort(scaledHeight);
        numOfBytesRead      += input.read(grbit);
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE,"\trw : "+ EndianConverter.readShort(rw) +
                            " miyRw : " + EndianConverter.readShort(miyRw) +
                            " grbit : " + EndianConverter.readShort(grbit) +
                            " ixfe : " + EndianConverter.readShort(ixfe));
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rw);
        output.write(miyRw);
        output.write(grbit);
        output.write(ixfe);

        Debug.log(Debug.TRACE,"Writing Row record");

    }

}
