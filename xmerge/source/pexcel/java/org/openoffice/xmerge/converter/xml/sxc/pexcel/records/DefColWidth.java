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
 * Represents a BIFF record defining the default column width
 */
public class DefColWidth implements BIFFRecord {

    private byte[] grbit = new byte[2];
    private byte[] coldx = new byte[2];
    private byte[] ixfe  = new byte[2];

    /**
     * Default constructor. Sets width to 9.
     */
    public DefColWidth() {
        grbit   = new byte[] {0x00, 0x00};
        coldx   = new byte[] {0x00, 0x09};
        ixfe    = new byte[] {0x00, 0x00};
    }

    /**
     * Constructs a pocket Excel Document from the
     * <code>InputStream</code> and assigns it the document name passed in
     *
     * @param   is InputStream containing a Pocket Excel Data file.
     */
    public DefColWidth(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>DefColWidth</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.DEF_COL_WIDTH;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(grbit);
        output.write(coldx);
        output.write(ixfe);

        Debug.log(Debug.TRACE,  "Writing DefColWidth record");
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(grbit);
        numOfBytesRead      += input.read(coldx);
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE,"\tgrbit : "+ EndianConverter.readShort(grbit) +
                            " coldx : " + EndianConverter.readShort(coldx) +
                            " ixfe : " + EndianConverter.readShort(ixfe));
        return 0;
    }

}
