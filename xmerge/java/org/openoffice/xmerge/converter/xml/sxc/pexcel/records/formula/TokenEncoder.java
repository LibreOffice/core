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
import java.util.Enumeration;

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
 * Functions    All pexcel fixed and varaible argument functions
 *
 */
public class TokenEncoder {

    FunctionLookup fl;
    String parseString;
    int index;

    /**
     * Default Constructor
     */
    public TokenEncoder() {
        parseString = new String();
        fl = new FunctionLookup();
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

        Vector tmpByteArray = null;     // we use this cause we don't know till after
                                        // the encoding takes place how big the byte [] will be

        if(t.getTokenType()==ParseToken.TOKEN_OPERATOR) {
            tmpByteArray = OperatorEncoder(t);
        } else if (t.getTokenType()==ParseToken.TOKEN_FUNCTION_VARIABLE || t.getTokenType()==ParseToken.TOKEN_FUNCTION_FIXED){
            tmpByteArray = FunctionEncoder(t);
        } else {                                    // Operands and functions
            switch(t.getTokenID()) {
                case TokenConstants.TREF :
                    tmpByteArray = CellRefEncoder(t);
                    break;
                case TokenConstants.TAREA :
                    tmpByteArray = AreaRefEncoder(t);
                    break;
                case TokenConstants.TINT :
                    tmpByteArray = IntEncoder(t);
                    break;
                default :
                    Debug.log(Debug.ERROR, "Encoder found unrecognized Token");
            }
        }

        byte cellRefArray[] = new byte[tmpByteArray.size()];
        int i = 0;
        Debug.log(Debug.TRACE, "Token Encoder : ");
        for(Enumeration e = tmpByteArray.elements();e.hasMoreElements();) {
            Byte tmpByte = (Byte) e.nextElement();
            Debug.log(Debug.TRACE, tmpByte + " ");
            cellRefArray[i] = tmpByte.byteValue();
            i++;
        }
        Debug.log(Debug.TRACE, "");
        return cellRefArray;
    }

    /**
     * An Operator Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private Vector OperatorEncoder(Token t) {

        Vector tmpByteArray = new Vector();
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        return tmpByteArray;
    }


    /**
     * An Integer Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private Vector IntEncoder(Token t) {

        Vector tmpByteArray = new Vector();

        double cellLong = (double) Double.parseDouble(t.getValue());
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
            Debug.log(Debug.ERROR, "Invalid Column Reference");
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
     * A Cell Reference Encoder. It supports absolute and relative addressing
     * but not sheetnames.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private Vector CellRefEncoder(Token t) {
        Vector tmpByteArray = new Vector();
        int col = 0, row = 0;
        int addressing = 0xC000;
        boolean colAbs = false, rowAbs = false;

        parseString = t.getValue();
        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        if(parseString.charAt(index)=='$') {
            addressing &= 0x8000;
            index++;
        }
        col = column();
        if(parseString.charAt(index)=='$') {
            addressing &= 0x4000;
            index++;
        }
        row = row();    // Pexcel uses a 0 based index
        row |= addressing;
        tmpByteArray.add(new Byte((byte)row));
        tmpByteArray.add(new Byte((byte)(row>>8)));
        tmpByteArray.add(new Byte((byte)col));
        index = 0;
        return tmpByteArray;
    }

    /**
     * A Cell Range Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private Vector AreaRefEncoder(Token t) {
        Vector tmpByteArray = new Vector();
        int row = 0, col1 = 0, col2 = 0;
        boolean rowAbs = false, colAbs = false;

        tmpByteArray.add(new Byte((byte)t.getTokenID()));
        parseString = t.getValue();
        if(parseString.charAt(index)=='$') {
            colAbs = true;
            index++;
        }
        col1 =  column();
        row = row();
        row |= 0xC000;
        tmpByteArray.add(new Byte((byte)row));
        tmpByteArray.add(new Byte((byte)(row>>8)));
        if(parseString.charAt(index)==':') {
            index++;
        } else {
            Debug.log(Debug.ERROR, "Invalid Cell Range, could not find :");
        }
        col2 = column();
        row = row();
        row |= 0xC000;
        tmpByteArray.add(new Byte((byte)row));
        tmpByteArray.add(new Byte((byte)(row>>8)));

        tmpByteArray.add(new Byte((byte)col1));
        tmpByteArray.add(new Byte((byte)col2));
        index = 0;
        return tmpByteArray;
    }

    /**
     * A Function Encoder.
     *
     * @param t <code>Token</code> to be encoded
     * @return A <code>Vector</code> of pexcel <code>Byte</code>
     */
    private Vector FunctionEncoder(Token t) {
        Vector tmpByteArray = new Vector();

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
