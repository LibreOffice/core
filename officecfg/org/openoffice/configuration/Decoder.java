/*************************************************************************
 *
 *  $RCSfile: Decoder.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dg $ $Date: 2001-07-09 09:57:31 $
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
package org.openoffice.configuration;


/**
 * Title:        Decoder
 * Description:  decoding of set element names given encoded (base64)
 */
public class Decoder extends Object
{
    //===========================================================
    // encoding table
    //===========================================================
    static final int[] aEncodingTable =
              { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '.' };

    static class ModifiedUTF7Buffer extends Object
    {
        StringBuffer aBuffer;
        int nValue;
        int nFilled = 0;

        public ModifiedUTF7Buffer(StringBuffer _aBuffer)
        {
            aBuffer = _aBuffer;
        }

        public void write(char c)
        {
            switch (nFilled)
            {
                case 0:
                    nValue = ((int)c) << 8;
                    nFilled = 2;
                    break;
                case 1:
                    nValue |= ((int)c);
                    nFilled = 3;
                    flush();
                    break;
                case 2:
                    nValue |= ((int)c) >> 8;
                    nFilled = 3;
                    flush();
                    nValue = (((int)c) & 0xFF) << 16;
                    nFilled = 1;
                    break;
            }
        }

        void flush()
        {
            switch (nFilled)
            {
                case 1:
                    aBuffer.append((char)aEncodingTable[nValue >> 18]);
                    aBuffer.append((char)aEncodingTable[nValue >> 12 & 63]);
                    break;

                case 2:
                    aBuffer.append((char)aEncodingTable[nValue >> 18]);
                    aBuffer.append((char)aEncodingTable[nValue >> 12 & 63]);
                    aBuffer.append((char)aEncodingTable[nValue >> 6 & 63]);
                    break;

                case 3:
                    aBuffer.append((char)aEncodingTable[nValue >> 18]);
                    aBuffer.append((char)aEncodingTable[nValue >> 12 & 63]);
                    aBuffer.append((char)aEncodingTable[nValue >> 6 & 63]);
                    aBuffer.append((char)aEncodingTable[nValue & 63]);
                    break;
            }
            nFilled = 0;
            nValue = 0;
        }
    };


    //===========================================================
    // decoding table
    //===========================================================
    static final int[] aModifiedBase64
        =   {   65, 65, 65, 65, 65, 65, 65, 65,
                65, 65, 65, 65, 65, 65, 65, 65,
                65, 65, 65, 65, 65, 65, 65, 65,
                65, 65, 65, 65, 65, 65, 65, 65,
                65, 65, 65, 65, 65, 65, 65, 65, //  !"#$%&'
                65, 65, 65, 65, 65, 62, 63, 65, // ()*+,-./
                52, 53, 54, 55, 56, 57, 58, 59, // 01234567
                60, 61, 65, 65, 65, 65, 65, 65, // 89:;<=>?
                65,  0,  1,  2,  3,  4,  5,  6, // @ABCDEFG
                 7,  8,  9, 10, 11, 12, 13, 14, // HIJKLMNO
                15, 16, 17, 18, 19, 20, 21, 22, // PQRSTUVW
                23, 24, 25, 65, 65, 65, 65, 64, // XYZ[\]^_
                65, 26, 27, 28, 29, 30, 31, 32, // `abcdefg
                33, 34, 35, 36, 37, 38, 39, 40, // hijklmno
                41, 42, 43, 44, 45, 46, 47, 48, // pqrstuvw
                49, 50, 51, 65, 65, 65, 65, 65  // xyz{|}~
            };

    static boolean isUsAsciiAlphaDigit(char c)
    {
        return isUsAsciiAlphaDigit(c,true);
    }

    static boolean isUsAsciiAlphaDigit(char c, boolean bDigitAllowed)
    {
        return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'
               || bDigitAllowed && c >= '0' && c <= '9';
    }

    static boolean write(int nUTF16Char, StringBuffer pBuffer, boolean bInitial)
    {
        if (isUsAsciiAlphaDigit((char)nUTF16Char, !bInitial)
            || !bInitial&& (nUTF16Char == (int)'-' || nUTF16Char == (int)'.'))
            return false;
        pBuffer.append((char)nUTF16Char);
        return true;
    }

    //===========================================================
    // return the next position after the decoded part
    // or -1 if an invalid character was found
    //===========================================================
    static int decodeModifiedUTF7(String sSource, int nStartPos,
                           boolean bInitial, StringBuffer aBuffer)
    {
        final int nEnd = sSource.length();

        int nUTF16Char = 0;
        int nFilled = 0;
        for (int nPos = nStartPos; nPos < nEnd; ++nPos)
        {

            char c = sSource.charAt(nPos);

            int nc = (int)c;
            int nDigit = nc < 128 ? aModifiedBase64[nc] : 65;

            switch (nDigit)
            {
                default: // valid character
                    switch (nFilled)
                    {
                        case 2:
                            nUTF16Char |= nDigit >> 2;
                            if (!write(nUTF16Char, aBuffer, bInitial))
                                return -1;
                            break;

                        case 5:
                            nUTF16Char |= nDigit >> 4;
                            if (!write(nUTF16Char, aBuffer, bInitial))
                                return -1;
                            break;

                        case 7:
                            nUTF16Char |= nDigit;
                            if (!write(nUTF16Char, aBuffer, bInitial))
                                return -1;
                            break;
                    }
                    bInitial = false;
                    switch (nFilled)
                    {

                        case 0:
                            nUTF16Char = nDigit << 10;
                            ++nFilled;
                            break;

                        case 1:
                            nUTF16Char |= nDigit << 4;
                            ++nFilled;
                            break;

                        case 2:
                            nUTF16Char = (nDigit & 3) << 14;
                            ++nFilled;
                            break;

                        case 3:
                            nUTF16Char |= nDigit << 8;
                            ++nFilled;
                            break;

                        case 4:
                            nUTF16Char |= nDigit << 2;
                            ++nFilled;
                            break;

                        case 5:
                            nUTF16Char = (nDigit & 15) << 12;
                            ++nFilled;
                            break;

                        case 6:
                            nUTF16Char |= nDigit << 6;
                            ++nFilled;
                            break;

                        case 7:
                            nFilled = 0;
                            break;
                    }
                    break;

                case 64: // terminating '_'
                    switch (nFilled)
                    {
                        case 3:
                        case 6:
                            if (nUTF16Char != 0)
                                break;
                        case 0:
                            // success
                            return ++nPos;
                    }
                case 65: // invalid character
                    return -1;
            }
        }
        return -1;
    }

    // return null, if the string was invalid
    public static String decodeValid(String sSource)
    {
        StringBuffer aTarget = new StringBuffer();

        final int nEnd = sSource.length();

        int nPos = 0;
        int nCopyEnd = 0;

        while(nPos < nEnd)
        {
            char c = sSource.charAt(nPos);
            if (!isUsAsciiAlphaDigit(c, nPos != 0))
                switch (c)
                {
                    case '_':
                        aTarget.append(sSource.substring(nCopyEnd, nPos ));
                        ++nPos;

                        nPos = decodeModifiedUTF7(sSource, nPos, nPos == 1, aTarget);
                        if (nPos < 0)
                            return null;

                        nCopyEnd = nPos;
                        continue;

                    case '-':
                    case '.':
                        if (nPos != 0)
                            break;
                    default:
                        return null;
                }
            ++nPos;
        }

        // System.out.println("Encoded string:" + sSource);
        if (nCopyEnd == 0)
            return sSource;
        else
        {
            aTarget.append(sSource.substring(nCopyEnd));
            // System.out.println("Decoded string:" + aTarget.toString());
            return aTarget.toString();
        }
    }

    // return the original value, if the string was invalid
    public static String decode(String sSource)
    {
        String sResult = decodeValid(sSource);
        System.out.println("Encoded string:" + sSource);
        if (sResult == null)
            System.out.println("Decoded string:" + "null");
        else
            System.out.println("Decoded string:" + sResult);
        return sResult != null ? sResult : sSource;
    }

    public static String encode(String sSource)
    {
        StringBuffer aTarget = new StringBuffer();
        final int nEnd = sSource.length();
        int nCopyEnd = 0;
        int nPos = 0;
        while(nPos < nEnd)
        {
            char c = sSource.charAt(nPos);
            if (!isUsAsciiAlphaDigit(c, nPos != 0))
                switch (c)
                {
                    case '-':
                    case '.':
                        if (nPos != 0)
                            break;
                    default:
                        aTarget.append(sSource.substring(nCopyEnd, nPos ));
                        aTarget.append('_');

                        ModifiedUTF7Buffer aBuffer = new ModifiedUTF7Buffer(aTarget);
                        for (;;)
                        {
                            aBuffer.write(c);
                            nPos++;
                            if (nPos == nEnd)
                                break;
                            c = sSource.charAt(nPos);
                            if (isUsAsciiAlphaDigit(c) || c == '-' || c == '.')
                                break;
                        }
                        aBuffer.flush();
                        aTarget.append('_');
                        nCopyEnd = nPos;
                        continue;
                }
            nPos++;
        }

        if (nCopyEnd == 0)
            return sSource;
        else
        {
            aTarget.append(sSource.substring(nCopyEnd));
            return aTarget.toString();
        }
    }


    public static void main(String args[])
    {
        if (args.length != 1)
            System.out.println("Usage : Convert filname");

        String str = Decoder.decode(args[0]);
        if(str == null) {
            System.out.println("Our String is null");
        } else  {
            System.out.println("Our String " + str);
        }
    }

}


