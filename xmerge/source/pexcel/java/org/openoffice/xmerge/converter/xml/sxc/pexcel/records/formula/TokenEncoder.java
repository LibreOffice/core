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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.DefinedName;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

/**
 * The TokenEncoder encodes a Token to an equivalent pexcel byte[]. The only
 * public method apart from the default constructor is the getByte method.
 * This method picks an encoder based onthe Token's type or id field and uses
 * that encoder to return a byte[] which it returns. This Encoder supports
 * Operands     Floating point's, Cell references (absolute and relative),
 *              cell ranges
 * Operators    +,-,*,/,&lt;,&gt;.&lt;=,&gt;=,&lt;&gt;
 * Functions    All pexcel fixed and variable argument functions
 *
 */
public class TokenEncoder {

    private FunctionLookup fl;
    private String parseString;
    private int index;
    private Workbook wb;

    /**
     * Default Constructor
     */
    public TokenEncoder() {

        parseString = new String();
        fl = new FunctionLookup();
    }

      /**
     * Sets global workbook data needed for defined names
     */
       public void setWorkbook(Workbook wb) {

        this.wb = wb;
    }


    /**
     * Return the byte[] equivalent of a <code>Token</code>. The various
     * encoders return <code>Vector</code> of <code>Byte</code> instead
     * of byte[] because the number of bytes returned varies with each
     * <code>Token</code> encoded. After the encoding is finished the Vector
     * in converted to a byte[].
     *
     * @param t The <code>Token</code> to be encoded
     * @return An equivalent Pocket Excel byte[]
     */
    public byte[] getByte(Token t) throws IOException {

        ArrayList<Byte> tmpByteArray = null;     // we use this cause we don't know till after
                                        // the encoding takes place how big the byte [] will be
           //index=0;                       // This class is declared static in
                                        // FormulaHelper so better make sure our index is 0
        if(t.getTokenType()==ParseToken.TOKEN_OPERATOR) {
            tmpByteArray = operatorEncoder(t);
        } else if (t.getTokenType()==ParseToken.TOKEN_FUNCTION_VARIABLE || t.getTokenType()==ParseToken.TOKEN_FUNCTION_FIXED){
            tmpByteArray = functionEncoder(t);
        } else {                                    // Operands and functions
            switch(t.getTokenID()) {
                case TokenConstants.TNAME :
                    tmpByteArray = nameDefinitionEncoder(t);
                    break;
                case TokenConstants.TREF3D :
                    tmpByteArray = threeDCellRefEncoder(t);
                    break;
                case TokenConstants.TAREA3D:
                    tmpByteArray = threeDAreaRefEncoder(t);
                    break;
                case TokenConstants.TREF :
                    tmpByteArray = cellRefEncoder(t);
                    break;
                case TokenConstants.TAREA :
                    tmpByteArray = areaRefEncoder(t);
                    break;
                case TokenConstants.TNUM :
                    tmpByteArray = numEncoder(t);
                    break;
                case TokenConstants.TSTRING :
                    tmpByteArray = stringEncoder(t);
                    break;
                default :
                    Debug.log(Debug.ERROR, "Encoder found unrecognized Token");
            }
        }

        byte cellRefArray[] = new byte[tmpByteArray.size()];
        int i = 0;
        String s = new String();
        for(Iterator<Byte> e = tmpByteArray.iterator();e.hasNext();) {
            Byte tmpByte = e.next();
            s = s + tmpByte + " ";
            cellRefArray[i] = tmpByte.byteValue();
            i++;
        }
        Debug.log(Debug.TRACE, "Encoding Token " + t.getValue() + " as [" + s + "]");
        return cellRefArray;
    }

    /**
     * An Operator Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> operatorEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        return tmpByteArray;
    }


    /**
     * A String Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> stringEncoder(Token t) throws IOException{

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        tmpByteArray.add(new Byte((byte)(t.getValue().length())));
        tmpByteArray.add(new Byte((byte)0x01));
        byte [] stringBytes = t.getValue().getBytes("UTF-16LE");
        for (int i=0; i<stringBytes.length; i++) {
            tmpByteArray.add(new Byte(stringBytes[i]));
        }
        return tmpByteArray;
    }


    /**
     * An Integer Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> numEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();

        double cellLong = Double.parseDouble(t.getValue());
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        byte[] tempByte = EndianConverter.writeDouble(cellLong);
        for(int byteIter=0;byteIter<tempByte.length;byteIter++) {
            tmpByteArray.add(new Byte(tempByte[byteIter]));
        }
        return tmpByteArray;
    }

    /**
     * Converts a char to an int. It is zero based
     * so a=0, b=1 etc.
     *
     * @param ch the character to be converted
     * @return -1 if not a character otherwise a 0 based index
     */
    private int char2int(char ch) {
        if(!Character.isLetter(ch))
            return -1;

        ch = Character.toUpperCase(ch);
        return ch-'A';
    }

     /**
      * Identify letters
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
    private boolean isAlpha(char c) {
            return(Character.isLetter(c));
    }

     /**
      * Identify numbers
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
    private boolean isDigit(char c) {
            return(Character.isDigit(c));
    }

     /**
      * Identify letters or numbers
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
    private boolean isAlphaNum(char c) {
        return(isAlpha(c) || isDigit(c));
    }

    /**
     * Parses a column reference and returns it's integer equivalent. (eg.
     * A=0, D=3, BA=27)
     *
     * @return an 0 based index to a column
     */
    private int column() {
        char ch = parseString.charAt(index);
        String columnStr = new String();
        int col = 0;

        while(isAlpha(ch)) {
            columnStr += ch;
            index++;
            ch = parseString.charAt(index);
        }

        if(columnStr.length()==1) {
            col = char2int(columnStr.charAt(0));
        } else if (columnStr.length()==2) {
            col = char2int(columnStr.charAt(0)) + 1;
            col = (col*26) + char2int(columnStr.charAt(1));
        } else {
            Debug.log(Debug.ERROR, "Invalid Column Reference " + columnStr );
        }


        return col;
    }

    /**
     * Parses a column reference and returns it's integer equivalent. (eg.
     * A=0, D=3, BA=27)
     *
     * @return an 0 based index to a column
     */
    private int row() {
        char ch = parseString.charAt(index);
        String rowStr = new String();
        int row = 0;
        boolean status = true;

        do {
            rowStr += ch;
            index++;
            if(index>=parseString.length()) {
                status = false;
            } else  {
                ch = parseString.charAt(index);
            }
        } while(isDigit(ch) && status);
        return Integer.parseInt(rowStr)-1;  // Pexcel uses a 0 based index
    }

    /**
     * A Cell Reference Encoder (It supports absolute and relative addressing)
     *
     * @param cellCoordinates <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private byte[] encodeCellCoordinates(String cellCoordinates) {
        int col = 0, row = 0;
        int addressing = 0xC000;

        index = 0;
        parseString = cellCoordinates;
        Debug.log(Debug.TRACE,"Encoding cell coordinates " + cellCoordinates);
        if(cellCoordinates.charAt(index)=='$') {
            addressing &= 0x8000;
            index++;
        }
        col = column();
        if(cellCoordinates.charAt(index)=='$') {
            addressing &= 0x4000;
            index++;
        }
        row = row();    // Pexcel uses a 0 based index
        row |= addressing;
        byte tokenBytes[] = new byte[3];
        tokenBytes[0] = (byte)row;
        tokenBytes[1] = (byte)(row>>8);
        tokenBytes[2] = (byte)col;
        return tokenBytes;
    }

    /**
     * A name definition Encoder
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> nameDefinitionEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();

        String nameString = t.getValue();
        Debug.log(Debug.TRACE,"NameDefinitionEncoder : " + nameString);
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        Iterator<DefinedName> e = wb.getDefinedNames();
        DefinedName dn;
        String name;
        int definedNameIndex = 0;
        do {
            dn = e.next();
            name = dn.getName();
            Debug.log(Debug.TRACE,"Name pulled from DefinedName : " + name);
            definedNameIndex++;
        } while(!nameString.equalsIgnoreCase(name) && e.hasNext());

        tmpByteArray.add(new Byte((byte)definedNameIndex));
        tmpByteArray.add(new Byte((byte)0x00));

        for(int i = 0;i < 12;i++) {
            tmpByteArray.add(new Byte((byte)0x00));
        }

        return tmpByteArray;
    }
    /**
     * A Cell Reference Encoder. It supports absolute and relative addressing
     * but not sheetnames.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> cellRefEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();

        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        byte cellRefBytes[] = encodeCellCoordinates(t.getValue());
        for(int i = 0;i < cellRefBytes.length;i++) {
            tmpByteArray.add(new Byte(cellRefBytes[i]));
        }
        return tmpByteArray;
    }

    /**
     * This function will find the sheetname index for a given String
     *
     * @param s <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private short findSheetIndex(String s) {

        short sheetIndex = 0;
        String savedName;
        String sheetName;
        if (s.startsWith("$")) {
            sheetName = s.substring(1,s.length());  // Remove $
        } else {
            sheetName = s.substring(0,s.length());
        }
        Debug.log(Debug.TRACE,"Searching for Worksheet : " + sheetName);
        ArrayList<Object> names = wb.getWorksheetNames();
        Iterator<Object> e = names.iterator();
        do {
            savedName = (String) e.next();
            sheetIndex++;
        } while(!savedName.equalsIgnoreCase(sheetName) && e.hasNext());

        Debug.log(Debug.TRACE,"Setting sheetindex to " + sheetIndex);
        return (short)(sheetIndex-1);
    }

    /**
     * A 3D Cell reference encoder
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> threeDCellRefEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();
        parseString = t.getValue();
        Debug.log(Debug.TRACE,"Encoding 3D Cell reference " + t);
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        tmpByteArray.add(new Byte((byte)0xFF));
        tmpByteArray.add(new Byte((byte)0xFF));
        for(int i = 0;i < 8;i++) {
            tmpByteArray.add(new Byte((byte)0x00));
        }

        String sheetRef = parseString.substring(0, parseString.indexOf('.') + 1);
        if (sheetRef.indexOf(':')!=-1) {
            sheetRef = parseString.substring(0, parseString.indexOf(':'));
            short sheetNum1 = findSheetIndex(sheetRef);
            sheetRef = parseString.substring(parseString.indexOf(':') + 1, parseString.length());
            short sheetNum2 = findSheetIndex(sheetRef);
            tmpByteArray.add(new Byte((byte)sheetNum1));
            tmpByteArray.add(new Byte((byte)0x00));
            tmpByteArray.add(new Byte((byte)sheetNum2));
            tmpByteArray.add(new Byte((byte)0x00));
        } else {
            sheetRef = parseString.substring(0, parseString.indexOf('.'));
            short sheetNum = findSheetIndex(sheetRef);
            tmpByteArray.add(new Byte((byte)sheetNum));
            tmpByteArray.add(new Byte((byte)0x00));
            tmpByteArray.add(new Byte((byte)sheetNum));
            tmpByteArray.add(new Byte((byte)0x00));
        }
        String s = parseString.substring(parseString.indexOf('.') + 1, parseString.length());
        Debug.log(Debug.TRACE,"Parsing : " + s);
        byte cellRefBytes[] = encodeCellCoordinates(s);
        for(int i = 0;i < cellRefBytes.length;i++) {
            tmpByteArray.add(new Byte(cellRefBytes[i]));
        }
        return tmpByteArray;
    }
    /**
     * A 3D Area Reference Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> threeDAreaRefEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();
        parseString = t.getValue();
        Debug.log(Debug.TRACE,"Encoding 3D Area reference " + t);
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        tmpByteArray.add(new Byte((byte)0xFF));
        tmpByteArray.add(new Byte((byte)0xFF));
        for(int i = 0;i < 8;i++) {
            tmpByteArray.add(new Byte((byte)0x00));
        }

        String param1= parseString.substring(0, parseString.indexOf(':'));
        String cellRef1 = param1.substring(parseString.indexOf('.') + 1, param1.length());
        String sheetRef1 = param1.substring(0, param1.indexOf('.'));
        short sheetNum1 = findSheetIndex(sheetRef1);

        String param2 = parseString.substring(parseString.indexOf(':') + 1, parseString.length());
        Debug.log(Debug.TRACE,"param2: " + param2);
        String cellRef2 = param2.substring(param2.indexOf('.') + 1, param2.length());
        Debug.log(Debug.TRACE,"cellRef2: " + cellRef2);

        if(param2.indexOf('.')==-1) {
            tmpByteArray.add(new Byte((byte)sheetNum1));
            tmpByteArray.add(new Byte((byte)0x00));
            tmpByteArray.add(new Byte((byte)sheetNum1));
            tmpByteArray.add(new Byte((byte)0x00));
        } else {
            String sheetRef2 = param2.substring(0, param2.indexOf('.'));
            short sheetNum2 = findSheetIndex(sheetRef2);
            tmpByteArray.add(new Byte((byte)sheetNum1));
            tmpByteArray.add(new Byte((byte)0x00));
            tmpByteArray.add(new Byte((byte)sheetNum2));
            tmpByteArray.add(new Byte((byte)0x00));
        }

        byte cellRefBytes1[] = encodeCellCoordinates(cellRef1);
        byte cellRefBytes2[] = encodeCellCoordinates(cellRef2);

        tmpByteArray.add(new Byte(cellRefBytes1[0]));
        tmpByteArray.add(new Byte(cellRefBytes1[1]));

        tmpByteArray.add(new Byte(cellRefBytes2[0]));
        tmpByteArray.add(new Byte(cellRefBytes2[1]));

        tmpByteArray.add(new Byte(cellRefBytes1[2]));
        tmpByteArray.add(new Byte(cellRefBytes2[2]));

        return tmpByteArray;
    }

    /**
     * A Cell Range Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> areaRefEncoder(Token t) {

        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();

        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        String param = t.getValue();
        String cellRef1 = new String();
        String cellRef2 = new String();

        if(param.indexOf(':')==-1) {
            Debug.log(Debug.ERROR, "Invalid Cell Range, could not find :");
        } else {
            cellRef1 = param.substring(0, param.indexOf(':'));
            cellRef2 = param.substring(param.indexOf(':') + 1, param.length());
        }
        byte cellRefBytes1[] = encodeCellCoordinates(cellRef1);
        byte cellRefBytes2[] = encodeCellCoordinates(cellRef2);

        tmpByteArray.add(new Byte(cellRefBytes1[0]));
        tmpByteArray.add(new Byte(cellRefBytes1[1]));

        tmpByteArray.add(new Byte(cellRefBytes2[0]));
        tmpByteArray.add(new Byte(cellRefBytes2[1]));

        tmpByteArray.add(new Byte(cellRefBytes1[2]));
        tmpByteArray.add(new Byte(cellRefBytes2[2]));

        return tmpByteArray;
    }

    /**
     * A Function Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private ArrayList<Byte> functionEncoder(Token t) {
        ArrayList<Byte> tmpByteArray = new ArrayList<Byte>();

        int id = t.getTokenID();
        if(t.getTokenType()==ParseToken.TOKEN_FUNCTION_VARIABLE) {
            tmpByteArray.add(new Byte((byte)TokenConstants.TFUNCVAR));
            tmpByteArray.add(new Byte((byte)t.getNumArgs()));
        } else {
            tmpByteArray.add(new Byte((byte)TokenConstants.TFUNC));
        }

        tmpByteArray.add(new Byte((byte)id));
        tmpByteArray.add(new Byte((byte)(id>>8)));
        return tmpByteArray;
    }


}
