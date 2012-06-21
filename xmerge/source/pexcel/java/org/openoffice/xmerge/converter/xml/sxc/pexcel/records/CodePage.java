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
 * Represents the codepage for the document. There is a number of unknown
 * fields which are hardcoded at construction
 */
public class CodePage implements BIFFRecord {

    private byte[] codepage = new byte[2];
    private byte[] unknown1 = new byte[2];
    private byte[] unknown2 = new byte[2];
    private byte unknown3;

    /**
     * Constructs a pocket Excel Codepage
     */
    public CodePage() {
        codepage    = new byte[] {(byte)0xE4, (byte)0x04};
        unknown1    = new byte[] {(byte)0x8C, (byte)0x01};
        unknown2    = new byte[] {(byte)0x00, (byte)0x01};
        unknown3    = 0x00;
    }

    /**
     * Constructs a pocket Excel Codepage from the<code>InputStream</code>
     *
     * @param   is InputStream containing a Pocket Excel Data file.
     */
    public CodePage(InputStream is) throws IOException {
        read(is);
    }

     /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BoundSheet</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.CODEPAGE;
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(codepage);
        numOfBytesRead      += input.read(unknown1);
        numOfBytesRead      += input.read(unknown2);
        unknown3            = (byte) input.read();
        numOfBytesRead++;

        Debug.log(Debug.TRACE,"\tcodepage : "+ EndianConverter.readShort(codepage) +
                            " unknown1 : " + EndianConverter.readShort(unknown1) +
                            " unknown2 : " + EndianConverter.readShort(unknown2) +
                            " unknown3 : " + unknown3);

        return numOfBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(codepage);
        output.write(unknown1);
        output.write(unknown2);
        output.write(unknown3);

        Debug.log(Debug.TRACE,"Writing CodePage record");


    }

}
