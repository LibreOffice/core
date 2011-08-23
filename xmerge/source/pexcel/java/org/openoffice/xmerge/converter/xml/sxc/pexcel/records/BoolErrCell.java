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
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF Record that describes a Boolean or Error value 
 */
public class BoolErrCell extends CellValue {

    private byte   bBoolErr;
    private byte   fError;
    
     /**
     * Constructs a BoolErrCell from arguments
      *
     * @param row row number
     * @param col column number 
     * @param ixfe font index
     * @param bBoolErr Boolean value or error value 
     * @param fError Boolean error flag 
      */
    public BoolErrCell(int row, int column, int ixfe, int bBoolErr, int fError) throws IOException {

        setIxfe(ixfe);
        this.bBoolErr = (byte)bBoolErr;
        this.fError = (byte)fError;
        setRow(row);
        setCol(column);
    }   
    
    /**
     * Constructs a BoolErrCell from the <code>InputStream</code> 
      *
      * @param	is InputStream containing a BoolErrCell  
      */
    public BoolErrCell(InputStream is) throws IOException {
        read(is);	
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>BoolErrCEll</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BOOLERR_CELL;
    } 

    /**
     * Writes a <code>BoolErrCell</code> to the specified <code>Outputstream</code>
     *
     * @param os the <code>OutputStream</code> to write to  
     */    
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        
        super.write(output);

        output.write(bBoolErr);
        output.write(fError);

        Debug.log(Debug.TRACE,"Writing BoolErrCell record");
    }
       
    /**
     * Reads a BoolErrCell from the <code>InputStream</code> 
      *
      * @param	is InputStream containing a BoolErrCell 
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        bBoolErr			= (byte) input.read();
        fError				= (byte) input.read();
           numOfBytesRead += 2;     

        Debug.log(Debug.TRACE, " bBoolErr : " + bBoolErr +
                            " fError : " + fError);        
        return numOfBytesRead;
    }

    /**
     * Gets the <code>String</code> representing the cells contents 
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {
        return ("Error Cell");
    }
}
