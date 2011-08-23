/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    *  @param  ver	Version Number
    *				Substream type (workbook = 0x05, worksheet = 0x10)
    */
    public BeginningOfFile(boolean global) {
        setVersion((short) 271);
        if(global)
            setSubStreamWBGlobal();
        else
            setSubStreamWorkSheet();
        // this.subStream = EndianConverter.writeShort(dt);
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
        // subStream = new byte[] {0x05};
        subStream = EndianConverter.writeShort((short) 0x05);
    }
    
    private void setSubStreamWorkSheet() {
        // subStream = new byte[] {0x10};
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
