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
 * Represents a BIFF Record describing a floating point
 */
public class FloatNumber extends CellValue {

    protected byte[] num  = new byte[8];
    
    /**
      * Constructs a pocket Excel Document from the
      * <code>InputStream</code> and assigns it the document name passed in
     *
      * @param	is InputStream containing a Pocket Excel Data file.
      */
    public FloatNumber(InputStream is) throws IOException {
        read(is);
    }

    /**
      * Constructs a <code>FloatNumber</code> using specified attributes 
     *
     * @param row row number
     * @param col column number 
     * @param cellContents contents of the cell 
     * @param ixfe font index
      */
    public FloatNumber(int row, int column, String cellContents, int ixfe) throws IOException {

        setIxfe(ixfe);
        setRow(row);
           setCol(column);
        double cellLong = (double) Double.parseDouble(cellContents);
        num 	= EndianConverter.writeDouble(cellLong);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>FloatNumber</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.NUMBER_CELL;
    }

    /**
     * Reads a<code>FloatNumber</code> from the specified <code>InputStream</code>
     *
     * @param input the <code>InputStram</code> to read from  
     */    
    public int read(InputStream input) throws IOException {
    
        int numOfBytesRead = super.read(input);

        numOfBytesRead += input.read(num);    
        
        Debug.log(Debug.TRACE," num : " + getString());
        return numOfBytesRead;
    }
    
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        
        super.write(output);

        output.write(num);

        Debug.log(Debug.TRACE,"Writing FloatNumber record");
    }
   

    /**
     * Gets the numerical value the cell represents 
     *
     * @return the <code>String</code> representing a double value 
     */
    public String getString() throws IOException {

        double value = EndianConverter.readDouble(num);
        Double myDo = new Double(value);
        return myDo.toString();	
    }

}
