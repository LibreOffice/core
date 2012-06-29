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
 * Represents a BIFF REcord that describes workbook window attributes
 */
public class Window1 implements BIFFRecord {

    private byte[] grbit    = new byte[2];
    private byte[] itabCur  = new byte[2];      // index of selected worksheet

    /**
      * Constructor
      */
    public Window1() {
        grbit   = EndianConverter.writeShort((short) 0);
        itabCur = EndianConverter.writeShort((short) 0);
    }

       /**
      * Constructs a Window1 Record from an <code>InputStream</code>
      *
      * @param  is InputStream containing a Window1 Record
      */
    public Window1(InputStream is) throws IOException{
        read(is);
    }

    /**
     * Set the number of the active sheet
     *
     * @param activeSheet number of the active sheet
     */
    public void setActiveSheet(int activeSheet) {
        itabCur = EndianConverter.writeShort((short) activeSheet);
    }

    /**
     * Get the number of the active sheet
     *
     * @return   number of the active sheet
     */
    public int getActiveSheet() {
        return EndianConverter.readShort(itabCur);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Window1</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.WINDOW_INFO;
    }

       /**
      * Reads a Window1 Record from an <code>InputStream</code>
      *
      * @param  input InputStream containing a Window1 Record
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(grbit);
        numOfBytesRead      += input.read(itabCur);

        Debug.log(Debug.TRACE,"\tgrbit : "+ EndianConverter.readShort(grbit) +
                            " itabCur : " + EndianConverter.readShort(itabCur));

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(grbit);
        output.write(itabCur);

        Debug.log(Debug.TRACE,"Writing Window1 record");
    }
}
