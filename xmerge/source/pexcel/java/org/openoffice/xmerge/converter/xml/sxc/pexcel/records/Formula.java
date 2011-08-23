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
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula.FormulaHelper;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 * Represents a BIFF Record describing a formula  
 */
public class Formula extends CellValue implements OfficeConstants {

    private byte[] num		= new byte[8];
    private byte grbit;
    private byte[] cce		= new byte[2];
    private byte[] rgce;
    private FormulaHelper fh = new FormulaHelper();
   
    /**
      * Constructs a <code>Formula</code> using specified attributes
     * 
     * @param row row number
     * @param col column number 
     * @param cellContents contents of the cell 
     * @param ixfe font index
     * @param value the value of the cell
      */
    public Formula(int row, int column, String cellContents, int ixfe, Format fmt, Workbook wb)
    throws Exception {

        fh.setWorkbook(wb);

        setRow(row);
        setCol(column);
        setIxfe(ixfe);
        setFormula(cellContents);
        
        String category = fmt.getCategory();
        String value = fmt.getValue();
        
        if(category.equalsIgnoreCase(CELLTYPE_BOOLEAN)) {
            num[0]=(byte)0x01;
            num[1]=(byte)0x00;
            if(value.equalsIgnoreCase("true")) {
                num[2]=(byte)0x01;
            } else {
                num[2]=(byte)0x00;
            }
            num[3]=(byte)0x00;num[4]=(byte)0x00;num[5]=(byte)0x00;
            num[6]=(byte)0xFF;num[7]=(byte)0xFF;
        } else if(category.equalsIgnoreCase(CELLTYPE_DATE)) {
            Debug.log(Debug.TRACE,"Date Formula");
            num = EndianConverter.writeDouble(toExcelSerialDate(fmt.getValue()));
        } else if(category.equalsIgnoreCase(CELLTYPE_TIME)) {
            Debug.log(Debug.TRACE,"Time Formula");
            num = EndianConverter.writeDouble(toExcelSerialTime(fmt.getValue()));
        } else if(category.equalsIgnoreCase(CELLTYPE_PERCENT)) {
            Debug.log(Debug.TRACE,"Percent Formula");
            double percent = (double) Double.parseDouble(fmt.getValue());
            num = EndianConverter.writeDouble(percent);
        } else if(category.equalsIgnoreCase(CELLTYPE_CURRENCY)) {
            Debug.log(Debug.TRACE,"Currency Formula");
        } else if(category.equalsIgnoreCase(CELLTYPE_STRING)) {
            Debug.log(Debug.TRACE,"String Formula");
            num[0]=(byte)0x00;
            num[1]=(byte)0x00;
            num[2]=(byte)0x00;
            num[3]=(byte)0x00;
            num[4]=(byte)0x00;
            num[5]=(byte)0x00;
            num[6]=(byte)0xFF;
            num[7]=(byte)0xFF;
        } else {
            Debug.log(Debug.TRACE,"Float Formula");
            double cellLong = (double) Double.parseDouble(fmt.getValue());
            num = EndianConverter.writeDouble(cellLong);
        }
    }
  
    /**
      * Translates a <code>String</code> written in infix which represents a
     * formula into a byte[] what can be written to pocket excel file.
      *
      * @param	formula string
      */
    public void setFormula(String inFormula) throws Exception {

        rgce = fh.convertCalcToPXL(inFormula);
        cce = EndianConverter.writeShort((short) rgce.length);
    }
  
    /**
      * Constructs a pocket Excel formula from the
      * <code>InputStream</code> 
      *
      * @param	is InputStream containing a Pocket Excel Data file.
      */
    public Formula(InputStream is, Workbook wb) throws IOException {
        read(is);
        fh.setWorkbook(wb);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>Formula</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.FORMULA_CELL;
    }
    
    /**
     * Reads the formula data members from the stream. Byte arrays for Strings
     * are doubled as they are stored as  unicode
     *
     * @return total number of bytes read 
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead = super.read(input);

        numOfBytesRead += input.read(num);
        grbit				= (byte) input.read();        
        numOfBytesRead		++;
        numOfBytesRead		+= input.read(cce);
       
        int strLen = EndianConverter.readShort(cce);
        rgce = new byte[strLen];
        input.read(rgce, 0, strLen);        
        
        Debug.log(Debug.TRACE, " num : " + num +
                            "\n\tgrbit : " + grbit +        
                            " cce : " + EndianConverter.readShort(cce) +
                            " rgce : " + new String(rgce,"UTF-16LE") + 
                            "\n" + numOfBytesRead + " Bytes Read");
        
        return numOfBytesRead;
    }

     /**
     * Writes the Formula record to the <code>OutputStream</code> 
     *
     * @param the <code>OutputStream</code> being written to 
     */   
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        
        super.write(output);

        output.write(num);
        output.write(grbit);
        output.write(cce);
        output.write(rgce);

        Debug.log(Debug.TRACE,"Writing Formula record");
    }

   /**
     * Gets the <code>String</code> representing the cell value
     *
     * @return the <code>String</code> representing the cell value
     */
    public String getValue() throws IOException {

        double value = EndianConverter.readDouble(num);
        Double myDo = new Double(value);
        return myDo.toString();
    }

    /**
     * Gets the <code>String</code> representing the cells contents 
     *
     * @return the <code>String</code> representing the cells contents
     */
    public String getString() throws IOException {

        return fh.convertPXLToCalc(rgce);
    }
    
    /**
     * Excel dates are the number of days since 1/1/1900. This method converts
     * to this date.
     *
     * @param s String representing a date in the form YYYY-MM-DD 
     * @return The excel serial date  
     */
    public long toExcelSerialDate(String s) throws IOException {

        int year = Integer.parseInt(s.substring(0,4));
        int month = Integer.parseInt(s.substring(5,7));
        int day = Integer.parseInt(s.substring(8,10));

        long serialDate =	(1461 * (year + 4800 + (month - 14) / 12)) / 4 +
                            (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 -
                            (3 * ((year + 4900 + (month - 14) / 12)) / 100) / 4 +
                            day - 2415019 - 32075;	

        return serialDate;
    }
    
    /**
     * Excel times are a fraction of a 24 hour day expressed in seconds. This method converts
     * to this time.
     *
     * @param s String representing a time in the form ??HH?MM?SS? 
     * @return The excel serial time  
     */
    public double toExcelSerialTime(String s) throws IOException {

        int hours = Integer.parseInt(s.substring(2,4));
        int mins = Integer.parseInt(s.substring(5,7));
        int secs = Integer.parseInt(s.substring(8,10));

        int timeSecs = (hours*3600) + (mins*60) + (secs);

        double d = (double) timeSecs / (24 * 3600);	

        return d;
    }

}
