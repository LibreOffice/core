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
 * Represents a BIFF Record describing a number format
 */
public class NumberFormat implements BIFFRecord {

    private byte cce;
    private byte[] rgch;

    /**
     * Constructs a NumberFormat Record from the <code>InputStream</code>
     *
     * @param   is InputStream containing the record data
     */
    public NumberFormat(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>NumberFormat</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.NUMBER_FORMAT;
    }

    /**
     * Reads the NumberFormat from the <code>InputStream</code> Byte array
     * containg strings are doubled in length becuse they use unicode
     *
     * @return the total number of bytes read
     */
    public int read(InputStream input) throws IOException {

        cce = (byte) input.read();
        int numOfBytesRead = 1;

        rgch = new byte[cce*2];
        numOfBytesRead  += input.read(rgch, 0, cce*2);

        Debug.log(Debug.TRACE, "\tcce : "+ cce +
                            " rgch : " + new String(rgch,"UTF-16LE"));

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(cce);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing NumberFormat record");
    }

}
