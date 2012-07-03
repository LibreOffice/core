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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.DefinedName;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
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
 * Functions    All pexcel fixed and variable argument functions
 *
 */
public class TokenDecoder {

    private TokenFactory tf;
    private FunctionLookup fl;
    private OperatorLookup operatorLookup;
    private Workbook wb;

    /**
     * Default Constructor initializes the <code>TokenFactory</code> for generating
     * <code>Token</code> and the <code>SymbolLookup</code> for generating
     * Strings from hex values.
     */
    public TokenDecoder() {
        tf = new TokenFactory();
        fl = new FunctionLookup();
        operatorLookup = new OperatorLookup();
    }

      /**
     * Sets global workbook data needed for defined names
     */
       public void setWorkbook(Workbook wb) {

        Debug.log(Debug.TRACE, "TokenDecoder : setWorkbook");
        this.wb = wb;
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
    public ArrayList<Token> getTokenVector(byte[] formula) {

        ArrayList<Token> v = new ArrayList<Token>();

        ByteArrayInputStream bis = new ByteArrayInputStream(formula);
        int b = 0 ;

        while ((b = bis.read())!=-1)
        {


            switch (b) {

                case TokenConstants.TAREA3D:
                                Debug.log(Debug.TRACE, "Decoded 3D Area Cell Reference: ");
                                v.add(read3DCellAreaRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded 3D Area Cell Reference: " + last(v));
                                break;
                case TokenConstants.TREF3D:
                                Debug.log(Debug.TRACE, "Decoded 3D Cell Reference: ");
                                v.add(read3DCellRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded 3D Cell Reference: " + last(v));
                                break;
                case TokenConstants.TREF :
                                v.add(readCellRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded Cell Reference: " + last(v));
                                break;
                case TokenConstants.TAREA :
                                v.add(readCellAreaRefToken(bis));
                                Debug.log(Debug.TRACE, "Decoded Cell Area Reference: " + last(v));
                                break;
                case TokenConstants.TNUM :
                                v.add(readNumToken(bis));
                                Debug.log(Debug.TRACE, "Decoded number : " + last(v));
                                break;
                case TokenConstants.TFUNCVAR :
                                v.add(readFunctionVarToken(bis));
                                Debug.log(Debug.TRACE, "Decoded variable argument function: " + last(v));
                                break;
                case TokenConstants.TFUNC :
                                v.add(readFunctionToken(bis));
                                Debug.log(Debug.TRACE, "Decoded function: " + last(v));
                                break;
                case TokenConstants.TSTRING :
                                v.add(readStringToken(bis));
                                Debug.log(Debug.TRACE, "Decoded string: " + last(v));
                                break;
                case TokenConstants.TNAME :
                                v.add(readNameToken(bis));
                                Debug.log(Debug.TRACE, "Decoded defined name: " + last(v));
                                break;
                case TokenConstants.TUPLUS:
                case TokenConstants.TUMINUS:
                case TokenConstants.TPERCENT:
                                v.add(readOperatorToken(b, 1));
                                Debug.log(Debug.TRACE, "Decoded Unary operator : " + last(v));
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
                                v.add(readOperatorToken(b, 2));
                                Debug.log(Debug.TRACE, "Decoded Binary operator : " + last(v));
                                break;

                default :
                                Debug.log(Debug.TRACE, "Unrecognized byte : " + b);
            }
        }
        return v;
    }

    private static Object last(ArrayList<Token> list)
    {
        return list.get(list.size() - 1);
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
     * @return The decoded String <code>Token</code>
     */
    private Token readStringToken(ByteArrayInputStream bis) {

        int len = bis.read()*2;
        int options = bis.read();
        Debug.log(Debug.TRACE,"String length is " + len + " and Options Flag is " + options);
        byte [] stringBytes = new byte[len];
        int numRead =0;
        if ((numRead = bis.read(stringBytes, 0, len)) != len) {
            Debug.log(Debug.TRACE,"Expected " + len + " bytes. Could only read " + numRead + " bytes.");
            //throw new IOException("Expected " + len + " bytes. Could only read " + numRead + " bytes.");
        }
        StringBuffer outputString = new StringBuffer();
        outputString.append('"');
        try {
            Debug.log(Debug.TRACE,"Using LE encoding");
            outputString.append(new String(stringBytes, "UTF-16LE"));
        } catch (IOException eIO) {
            outputString.append(new String(stringBytes)); //fall back to default encoding
        }
        outputString.append('"');

        return (tf.getOperandToken(outputString.toString(), "STRING"));
    }

    /**
     * Reads a Defined Name  token from the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded Name <code>Token</code>
     */
    private Token readNameToken(ByteArrayInputStream bis) {
        byte buffer[] = new byte[2];
        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int nameIndex = EndianConverter.readShort(buffer);
        bis.skip(12);       // the next 12 bytes are unused
        Iterator<DefinedName> e = wb.getDefinedNames();
        int i = 1;
        while(i<nameIndex) {
            e.next();
            i++;
        }
        Debug.log(Debug.TRACE,"Name index is " + nameIndex);
        DefinedName dn = e.next();
        Debug.log(Debug.TRACE,"DefinedName is " + dn.getName());
        return (tf.getOperandToken(dn.getName(), "NAME"));
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
     * Reads a Cell Reference token from the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded Cell Reference <code>Token</code>
     */
    private Token read3DCellRefToken(ByteArrayInputStream bis) {

        byte buffer[] = new byte[2];
        String outputString = new String();

        bis.skip(10);

           buffer[0] = (byte) bis.read();
         buffer[1] = (byte) bis.read();
        int Sheet1 = EndianConverter.readShort(buffer);
           buffer[0] = (byte) bis.read();
         buffer[1] = (byte) bis.read();
        int Sheet2 = EndianConverter.readShort(buffer);

        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int formulaRow = EndianConverter.readShort(buffer);
        int relativeFlags = (formulaRow & 0xC000)>>14;
        formulaRow &= 0x3FFF;
        int formulaCol = (byte) bis.read();
        String cellRef = "." + int2CellStr(formulaRow, formulaCol, relativeFlags);
        if(Sheet1 == Sheet2) {
            outputString = "$" + wb.getSheetName(Sheet1) + cellRef;
        } else {
            outputString = "$" + wb.getSheetName(Sheet1) + cellRef + ":$" + wb.getSheetName(Sheet2) + cellRef;
        }

        return (tf.getOperandToken(outputString,"3D_CELL_REFERENCE"));
    }

    /**
     * Reads a Cell Reference token from the <code>ByteArrayInputStream</code>
     *
     * @param bis The <code>ByteArrayInputStream</code> from which we read the
     * bytes.
     * @return The decoded Cell Reference <code>Token</code>
     */
    private Token read3DCellAreaRefToken(ByteArrayInputStream bis) {

        byte buffer[] = new byte[2];
        String outputString = new String();

        bis.skip(10);

           buffer[0] = (byte) bis.read();
         buffer[1] = (byte) bis.read();
        int Sheet1 = EndianConverter.readShort(buffer);
           buffer[0] = (byte) bis.read();
         buffer[1] = (byte) bis.read();
        int Sheet2 = EndianConverter.readShort(buffer);

        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int formulaRow1 = EndianConverter.readShort(buffer);
        int relativeFlags1 = (formulaRow1 & 0xC000)>>14;
        formulaRow1 &= 0x3FFF;

        buffer[0] = (byte) bis.read();
        buffer[1] = (byte) bis.read();
        int formulaRow2 = EndianConverter.readShort(buffer);
        int relativeFlags2 = (formulaRow2 & 0xC000)>>14;
        formulaRow2 &= 0x3FFF;

        int formulaCol1 = (byte) bis.read();
        int formulaCol2 = (byte) bis.read();

        String cellRef1 = "." + int2CellStr(formulaRow1, formulaCol1, relativeFlags1);
        String cellRef2 = int2CellStr(formulaRow2, formulaCol2, relativeFlags2);

        if(Sheet1 == Sheet2) {
            outputString = "$" + wb.getSheetName(Sheet1) + cellRef1 + ":" + cellRef2;
        } else {
            outputString = "$" + wb.getSheetName(Sheet1) + cellRef1 + ":$" + wb.getSheetName(Sheet2) + "." + cellRef2;
        }

        return (tf.getOperandToken(outputString,"3D_CELL_AREA_REFERENCE"));
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
     * @param args The number of arguments this operator takes.
     * @return The decoded Operator <code>Token</code>
     */
    private Token readOperatorToken(int b, int args) {

        Token t;

        if(b==TokenConstants.TUPLUS) {
            t = tf.getOperatorToken("+", args);
        } else if(b==TokenConstants.TUMINUS) {
            t = tf.getOperatorToken("-", args);
        } else {
            t = tf.getOperatorToken(operatorLookup.getStringFromID(b), args);
        }
        return t;
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
