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

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.awt.Point;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;
import org.openoffice.xmerge.converter.xml.sxc.SheetSettings;


/**
 * Represents a BIFF Record that describes worksheet window attributes
 */
public class Window2 implements BIFFRecord {

    private final static int FROZEN 	= 0x08;
    private final static int NOSPLIT	= 0x01;

    private byte[] rwTop	= new byte[2];
    private byte   colLeft;
    private byte[] grbit	= new byte[2];
    
    /**
     * Constructor  
     */
    public Window2() {
        this.rwTop		= EndianConverter.writeShort((short) 0);
        this.colLeft	= 0; 
        this.grbit		= EndianConverter.writeShort((short) 0);
    }
    
       /**
      * Constructs a Window2 Record from an <code>InputStream</code> 
      *
      * @param	is InputStream containing a Window2 Record 
      */
    public Window2(InputStream is) throws IOException {
        read(is);	
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>Window2</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.SHEET_WINDOW_INFO;
    }

    /**
     * Sets the split type for this pane, the split type is the same for both
     * x and y so we only test against one.
     *
     * @param splitType the split type based on types defined in
     * <code>sheetSettings</code>
     */
    public void setSplitType(Point splitType) {
        if(splitType.getX()==SheetSettings.SPLIT) {
            grbit[0] &= ~FROZEN;
            grbit[1] &= ~NOSPLIT;
        } else {
            grbit[0] |= FROZEN;
            grbit[1] |= NOSPLIT;
        }
    }

    /** 
     * This method tests if this object describes a freeze 
     *
     * @return true if freeze otherwise false
     */
     public boolean isFrozen() {
        if((grbit[0] & FROZEN) != FROZEN)
            return false;

        if((grbit[1] & NOSPLIT) != NOSPLIT)
            return false;

        return true;
    }

    /** 
     * This method tests if this object describes a split
     *
     * @return true if split otherwise false
     */
     public boolean isSplit() {
        if((grbit[0] & FROZEN) == FROZEN)
            return false;

        if((grbit[1] & NOSPLIT) == NOSPLIT)
            return false;

        return true;
    }
    
       /**
      * Reads a Window2 Record from an <code>InputStream</code> 
      *
      * @param	is InputStream containing a Window2 Record 
      */
    public int read(InputStream input) throws IOException {
        
        int numOfBytesRead	= input.read(rwTop);
        colLeft				= (byte) input.read();
        numOfBytesRead++;
        numOfBytesRead		+= input.read(grbit);
        
        Debug.log(Debug.TRACE,"\trwTop : "+ EndianConverter.readShort(rwTop) + 
                            " colLeft : " + colLeft +
                            " grbit : " + EndianConverter.readShort(grbit));
        return numOfBytesRead;        
    }
    
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(rwTop);
        output.write(colLeft);
        output.write(grbit);

        Debug.log(Debug.TRACE,"Writing Window2 record");
    }
}
