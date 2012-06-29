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
 * Represents a BIFF record defiuning the default row height
 */
public class DefRowHeight implements BIFFRecord {

    private byte[] unknown1 = new byte[2];
    private byte[] unknown2 = new byte[2];

    /**
      * Default constructor. Sets something unknown to 255.
      */
    public DefRowHeight() {
        unknown1 = new byte[] {(byte)0x00, (byte)0x00};
        unknown2 = new byte[] {(byte)0xFF, (byte)0x00};
    }

    /**
      * Constructs a DefRowHeight from the <code>InputStream</code>
      *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public DefRowHeight(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>DefRowHeight</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.DEFAULT_ROW_HEIGHT;
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(unknown1);
        numOfBytesRead      += input.read(unknown2);

        Debug.log(Debug.TRACE,"\tunknown1 : "+ EndianConverter.readShort(unknown1) +
                            " unknown2 : " + EndianConverter.readShort(unknown2));
        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(unknown1);
        output.write(unknown2);

        Debug.log(Debug.TRACE,"Writing DefRowHeight record");


    }

}
