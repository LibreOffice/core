/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.DataInputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Vector;
import java.util.Enumeration;
import java.util.Calendar;
import java.util.Date;
import java.text.DateFormat;

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

    private byte[] num      = new byte[8];
    private byte grbit;
    private byte[] cce      = new byte[2];
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
      * @param  formula string
      */
    public void setFormula(String inFormula) throws Exception {

        rgce = fh.convertCalcToPXL(inFormula);
        cce = EndianConverter.writeShort((short) rgce.length);
    }

    /**
      * Constructs a pocket Excel formula from the
      * <code>InputStream</code>
      *
      * @param  is InputStream containing a Pocket Excel Data file.
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
        grbit               = (byte) input.read();
        numOfBytesRead      ++;
        numOfBytesRead      += input.read(cce);

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

        long serialDate =   (1461 * (year + 4800 + (month - 14) / 12)) / 4 +
                            (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 -
                            (3 * ((year + 4900 + (month - 14) / 12)) / 100) / 4 +
                            day - 2415019 - 32075;

        return serialDate;
    }

}
