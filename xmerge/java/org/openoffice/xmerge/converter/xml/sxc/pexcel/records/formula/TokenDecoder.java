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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import java.io.*;
import java.util.Vector;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
/**
 * The TokenDecoder decodes a byte[] to an equivalent <code>String</code>. The only
 * public method apart from the default constructor is the getTokenVector method.
 * This method takes an entire formula as a pexcel byte[] and decodes it into
 * a series of <code>Token</code>s. It adds these to a <code>Vector</code> which
 * is returned once all the tokens have been decoded. The Decoder supports
 * the following tokens.<br><br>
 *
 * Operands     Floating point's, Cell references (absolute and relative),
 *              cell ranges<br>
 * Operators    +,-,*,/,&lt;,&gt;.&lt;=,&gt;=,&lt;&gt;<br>
 * Functions    All pexcel fixed and varaible argument functions
 *
 */
public class TokenDecoder {

    private TokenFactory tf;
    private FunctionLookup fl;
    private OperatorLookup operatorLookup;
    private OperandLookup operandLookup;

    /**
     * Default Constructor initializes the <code>TokenFactory</code> for generating
     * <code>Token</code> and the <code>SymbolLookup</code> for generating
     * Strings from hex values.
     */
    public TokenDecoder() {
        tf = new TokenFactory();
        fl = new FunctionLookup();
        operatorLookup = new OperatorLookup();
        operandLookup = new OperandLookup();
    }

    /**
     * Returns a <code>Vector</code> of <code>Token</code> decoded from a
     * byte[]. The byte[] is first converted to a
     * <code>ByteArrayInputStream</code> as this is the easiest way of reading
     * bytes.
     *
     * @param formula A Pocket Excel Formula byte[]
     * @return A <code>Vector</code> of deoded <code>Token</code>
     */
    public Vector getTokenVector(byte[] formula) {

        Vector v = new Vector();

        ByteArrayInputStream bis = new ByteArrayInputStream(formula);
        int b = 0 ;
        Token t;

        while (bis.available()!=0)
        {
            b = bis.read();

            switch (b) {

                case TokenConstants.TREF :
                                v.add(readCellRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded Cell Reference: " + v.lastElement());
                                break;
                case TokenConstants.TAREA :
                                v.add(readCellAreaRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded Cell Area Reference: " + v.lastElement());
                                break;
                case TokenConstants.TNUM :
                                v.add(readNumToken(bis));
                                Debug.log(Debug.TRACE, "Decoded number : " + v.lastElement());
                                break;
                case TokenConstants.TFUNCVAR :
                                v.add(readFunctionVarToken(bis));
                                Debug.log(Debug.TRACE, "Decoded variable argument function: " + v.lastElement());
                                break;
                case TokenConstants.TFUNC :
                                v.add(readFunctionToken(bis));
                                Debug.log(Debug.TRACE, "Decoded function: " + v.lastElement());
                                break;
                case TokenConstants.TADD :
                case TokenConstants.TSUB :
                case TokenConstants.TMUL :
                case TokenConstants.TDIV :
                case TokenConstants.TLESS :
                case TokenConstants.TLESSEQUALS :
                case TokenConstants.TEQUALS :
                case TokenConstants.TGTEQUALS :
                case TokenConstants.TGREATER :
                case TokenConstants.TNEQUALS :
                                v.add(readOperatorToken(b));

                                Debug.log(Debug.TRACE, "Decoded binary operator : " + v.lastElement());
                                break;
                default :
                                Debug.log(Debug.TRACE, "Unrecognized byte : " + b);
            }
        }
        return v;
    }

    /**
     * Converts a zero based integer to a char (eg. a=0, b=1).
     * It assumes the integer is less than 26.
     *
     * @param i A 0 based index
     * @return The equivalent character
     */
    private char int2Char(int i) {
        return (char) ('A' + i);
    }

    /**
     * Reads a Cell Reference token from the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded Cell Reference <code>Token</code>
     */
    private Token readCellRefToken(ByteArrayInputStream bis) {
        byte buffer[] = new byte[2];
        String outputString = new String();

        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int formulaRow = EndianConverter.readShort(buffer);
        int relativeFlags = (formulaRow & 0xC000)>>14;
        formulaRow &= 0x3FFF;
        int formulaCol = (byte) bis.read();

        outputString = int2CellStr(formulaRow, formulaCol, relativeFlags);

        return (tf.getOperandToken(outputString,"CELL_REFERENCE"));
    }

    /**
     * Converts a row and col 0 based index to a spreadsheet cell reference.
     * It also has a relativeFlags which indicates whether or not the
     * Cell Reference is relative or absolute (Absolute is denoted with '$')
     *
     * 00 = absolute row, absolute col
     * 01 = absolute row, relative col
     * 10 = relative row, absolute col
     * 11 = relative row, relative col
     *
     * @param row The cell reference 0 based index to the row
     * @param col The cell reference 0 based index to the row
     * @param relativeFlags Flags indicating addressing of row and column
     * @return A <code>String</code> representing a cell reference
     */
    private String int2CellStr(int row, int col, int relativeFlags) {
       String outputString = "";
       int firstChar = (col + 1) / 26;

       if((relativeFlags & 1) == 0) {
           outputString += "$";
       }

        if(firstChar>0) {
                int secondChar = (col + 1) % 26;
                outputString += Character.toString(int2Char(firstChar - 1)) + Character.toString(int2Char(secondChar - 1));
        } else {
                outputString += Character.toString(int2Char(col));
        }
       if((relativeFlags & 2) == 0) {
           outputString += "$";
       }
       outputString += Integer.toString(row+1);
       return outputString;
    }

    /**
     * Reads a Cell Area Reference (cell range) <code>Token</code> from
     * the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The equivalent Cell Area Reference (cell range)
     * <code>Token</code>
     */
    private Token readCellAreaRefToken(ByteArrayInputStream bis) {
        byte buffer[] = new byte[2];
        int formulaRow1, formulaRow2;
        int formulaCol1, formulaCol2;

        String outputString = new String();

        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        formulaRow1 = EndianConverter.readShort(buffer);
        int relativeFlags1 = (formulaRow1 & 0xC000)>>14;
        formulaRow1 &= 0x3FFF;
        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        formulaRow2 = EndianConverter.readShort(buffer);
        int relativeFlags2 = (formulaRow2 & 0xC000)>>14;
        formulaRow2 &= 0x3FFF;

        formulaCol1 = (byte) bis.read();
        formulaCol2 = (byte) bis.read();

        outputString = int2CellStr(formulaRow1, formulaCol1, relativeFlags1);
        outputString += (":" + int2CellStr(formulaRow2, formulaCol2, relativeFlags2));

        return (tf.getOperandToken(outputString,"CELL_AREA_REFERENCE"));
    }


    /**
     * Reads a Number (floating point) token from the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded Integer <code>Token</code>
     */
    private Token readNumToken(ByteArrayInputStream bis) {

        byte numBuffer[] = new byte[8];

        for(int j=0;j<8;j++) {
                numBuffer[j]=(byte) bis.read();
        }

        return (tf.getOperandToken(Double.toString(EndianConverter.readDouble(numBuffer)),"NUMBER"));
    }

    /**
     * Read an Operator token from the <code>ByteArrayInputStream</code>
     *
     * @param b A Pocket Excel number representing an operator.
     * @return The decoded Operator <code>Token</code>
     */
    private Token readOperatorToken(int b) {

        return tf.getOperatorToken(operatorLookup.getStringFromID(b), 2);
     }

    /**
     * Read a Function token from the <code>ByteArrayInputStream</code>
     * This function can have any number of arguments and this number is read
     * in with the record
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded variable argument Function <code>Token</code>
     */
    private Token readFunctionVarToken(ByteArrayInputStream bis) {

        int numArgs = 0;
        numArgs = bis.read();
        byte buffer[] = new byte[2];
        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int functionID = EndianConverter.readShort(buffer);
        return (tf.getFunctionToken(fl.getStringFromID(functionID),numArgs));
    }

    /**
     * Read a Function token from the <code>ByteArrayInputStream</code>
     * This function has a fixed number of arguments which it will get
     * from <code>FunctionLookup</code>.
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded fixed argument Function <code>Token</code>
     */
    private Token readFunctionToken(ByteArrayInputStream bis) {

        byte buffer[] = new byte[2];
        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int functionID = EndianConverter.readShort(buffer);
        String functionName = fl.getStringFromID(functionID);
        return (tf.getFunctionToken(functionName,fl.getArgCountFromString(functionName)));
    }

}
