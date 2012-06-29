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
 *  This class describes the beginning of file. It is the
 *  the Biff record that marks the beginning of a a worbook
 *  or the beginning of worksheets in the workbook
 *
 */
public class BeginningOfFile implements BIFFRecord {

    private byte[] version = new byte[2];
    private byte[] subStream = new byte[2];

   /**
    *  Constructor that initializes the member values.
    *
    *  @param  global   True for a workbook, false for a worksheet
    */
    public BeginningOfFile(boolean global) {
        setVersion((short) 271);
        if(global)
            setSubStreamWBGlobal();
        else
            setSubStreamWorkSheet();
    }

    public BeginningOfFile(InputStream is) throws IOException {
        read(is);
    }

    private void setVersion(short version) {
        this.version = EndianConverter.writeShort(version);
    }

    int getVersion() {
        return EndianConverter.readShort(version);
    }

    private void setSubStreamWBGlobal() {
        subStream = EndianConverter.writeShort((short) 0x05);
    }

    private void setSubStreamWorkSheet() {
        subStream = EndianConverter.writeShort((short) 0x10);
    }

    int getSubStreamType() {
        return EndianConverter.readShort(subStream);
    }

    public int read(InputStream input) throws IOException {
        int numBytesRead = input.read(version);
        numBytesRead += input.read(subStream);
        Debug.log(Debug.TRACE,"\tVersion : "+ EndianConverter.readShort(version) +
                            " Stream : " + EndianConverter.readShort(subStream));

        return numBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(version);
        output.write(subStream);

        Debug.log(Debug.TRACE, "Writing BeginningOfFile record");
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BeginningOfFile</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BOF_RECORD;
   }
}
