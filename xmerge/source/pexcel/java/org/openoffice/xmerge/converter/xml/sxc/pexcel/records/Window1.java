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

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 * Represents a BIFF REcord that describes workbook window attributes 
 */
public class Window1 implements BIFFRecord {

    private byte[] grbit 	= new byte[2];
    private byte[] itabCur	= new byte[2];		// index of selected worksheet
    
    /**
      * Constructor
      */
    public Window1() {
        grbit	= EndianConverter.writeShort((short) 0);
        itabCur	= EndianConverter.writeShort((short) 0);
    }

       /**
      * Constructs a Window1 Record from an <code>InputStream</code> 
      *
      * @param	is InputStream containing a Window1 Record 
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
     * @return 	 number of the active sheet
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
      * @param	is InputStream containing a Window1 Record 
      */
    public int read(InputStream input) throws IOException {
        
        int numOfBytesRead	= input.read(grbit);
        numOfBytesRead		+= input.read(itabCur);
                
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
