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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.Records;

import java.io.DataInputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Vector;
import java.util.Enumeration;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * Represents a BIFF Record describing a formula
 */
public class Formula extends CellValue {

    private byte[] ixfe     = new byte[2];
    private byte[] num      = new byte[8];
    private byte grbit;
    private byte[] cce      = new byte[2];
    private byte[] rgce;

    /**
      * Constructs a <code>Formula</code> using specified attributes
     *
     * @param row row number
     * @param col column number
     * @param cellContents contents of the cell
     * @param ixfe font index
     * @param value the value of the cell
      */
    public Formula(int row, int column, String cellContents, int ixfe, String value)
    throws IOException {

        this.ixfe   = EndianConverter.writeShort((short)ixfe);
        cce         = EndianConverter.writeShort((short)0);
           setFormula(cellContents);
           setRow(row);
           setCol(column);
        double cellLong = (double) Double.parseDouble(value);
        num     = EndianConverter.writeDouble(cellLong);
    }

    /**
      * TRanslates a <code>String</code> written in RPN which represents a
     * formula into a byte[] what can be written to pocket excel file.
      *
      * @param  formula string
      */
    public void setFormula(String inFormula) throws IOException {

        Debug.log(Debug.TRACE,"setFormula : " + inFormula);
        String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        Vector tempArray = new Vector();
        int col = 0, row = 0;

        try {
            FormulaParser fp = new FormulaParser(inFormula);

            Enumeration formula = fp.toReversePolishNotation();

            while(formula.hasMoreElements()) {

                String nextOp = (String) formula.nextElement();

                char firstChar = nextOp.charAt(0);
                if (Character.isLetter(firstChar))  {
                    tempArray.add(new Byte((byte)0x44));
                    char nextRef = nextOp.charAt(0);
                    if(Character.isLetter(firstChar)) {
                        for(int j = 0;j< alphabet.length();j++) {
                            if(nextRef==alphabet.charAt(j)) {
                                col=j;
                                break;
                            }
                        }
                    }
                    nextRef = nextOp.charAt(1);
                    if(Character.isLetter(nextRef)) {
                        for(int j = 0;j< alphabet.length();j++) {
                            if(nextRef==alphabet.charAt(j)) {
                                col = (col*26)+j;
                                break;
                            }
                        }
                        row = Integer.parseInt(nextOp.substring(2)) - 1;
                    } else {
                        row = Integer.parseInt(nextOp.substring(1)) - 1;
                    }
                    row |= 0xC000;
                    tempArray.add(new Byte((byte)row));
                    tempArray.add(new Byte((byte)(row>>8)));
                    tempArray.add(new Byte((byte)col));
                } else if(nextOp.equals("+")){
                    tempArray.add(new Byte((byte)0x03));
                } else if(nextOp.equals("-")){
                    tempArray.add(new Byte((byte)0x04));
                } else if(nextOp.equals("*")){
                    tempArray.add(new Byte((byte)0x05));
                } else if(nextOp.equals("/")){
                    tempArray.add(new Byte((byte)0x06));
                } else {                                        // We will assume it's a number

                    double cellLong = (double) Double.parseDouble(nextOp);
                    tempArray.add(new Byte((byte)0x1F));
                    byte[] tempByte = EndianConverter.writeDouble(cellLong);
                    for(int byteIter=0;byteIter<tempByte.length;byteIter++) {
                        tempArray.add(new Byte(tempByte[byteIter]));
                    }
                }
            }
            Enumeration e = tempArray.elements();
            int i = 0;
            rgce = new byte[tempArray.size()];
            cce = EndianConverter.writeShort((short) tempArray.size());
            while(e.hasMoreElements()) {
                Byte myByte = (Byte)e.nextElement();
                rgce[i] = myByte.byteValue();
                i++;
            }

        } catch (UnsupportedFormulaException e) {
            Debug.log(Debug.ERROR,"UnsupportFormulaException : " + e.getMessage());
            rgce = new byte[0];
        }
    }

    /**
      * Constructs a pocket Excel formula from the
      * <code>InputStream</code>
      *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public Formula(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>Formula</code>
     */
    public short getBiffType() {
        return PocketExcelBiffConstants.FORMULA_CELL;
    }

    /**
     * Reads the formula data members from the stream. Byte arrays for Strings
     * are doubled as they are stored as  unicode
     *
     * @return total number of bytes read
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        col                 = (byte) input.read();
        numOfBytesRead      ++;
        numOfBytesRead      += input.read(ixfe);
        numOfBytesRead      += input.read(num);
        grbit               = (byte) input.read();
        numOfBytesRead      ++;
        numOfBytesRead      += input.read(cce);

        int strLen = EndianConverter.readShort(cce);
        rgce = new byte[strLen];
        input.read(rgce, 0, strLen);

        Debug.log(Debug.TRACE,"\trw : "+ EndianConverter.readShort(rw) +
                            " col : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe) +
                            " num : " + num +
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
        output.write(rw);
        output.write(col);
        output.write(ixfe);
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

        Vector outputString = new Vector();
        byte[] buffer = new byte[2];
        byte[] numBuffer = new byte[8];
        String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        int i = 0;

        while(i<EndianConverter.readShort(cce)) {
            switch(rgce[i]) {
                case 0x44 :
                            buffer[0]=rgce[i+1];
                            buffer[1]=rgce[i+2];
                            int formulaRow = EndianConverter.readShort(buffer);
                            formulaRow &= 0x3FFF;
                            int formulaCol = rgce[i+3];
                            int firstChar = formulaCol / 26;
                            if(firstChar>0) {
                                int secondChar = formulaCol - (firstChar*25);
                                outputString.add(alphabet.charAt(firstChar) + alphabet.charAt(secondChar) + Integer.toString(formulaRow+1));
                            } else {
                                outputString.add(alphabet.charAt(formulaCol) + Integer.toString(formulaRow+1));
                            }
                            // outputString.append(".");
                            i += 4;
                            break;

                case 0x03 :
                            outputString.add("+");
                            i++;
                            break;
                case 0x04 :
                            outputString.add("-");
                            i++;
                            break;
                case 0x05 :
                            outputString.add("*");
                            i++;
                            break;
                case 0x06 :
                            outputString.add("/");
                            i++;
                            break;
                case 0x1F :
                            for(int j=0;j<8;j++) {
                                numBuffer[j]=rgce[i+1+j];
                            }
                            long longBits = Double.doubleToLongBits(EndianConverter.readDouble(numBuffer));
                            Long longHelper = new Long(longBits);

                            outputString.add(longHelper.toString());
                            i+=9;
                            break;
                default :
                            return "";  // we dont't support functions for now
                                        // we only support cell references

            }
        }
        FormulaParser fp = new FormulaParser(outputString);
        return fp.toInfixNotation();
        // return outputString.toString();
    }

}
