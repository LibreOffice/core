/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <comphelper/base64.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <algorithm>

using namespace com::sun::star;

namespace comphelper {

const
  sal_Char aBase64EncodeTable[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

const
  sal_uInt8 aBase64DecodeTable[]  =
    {                                            62,255,255,255, 63, // 43-47
//                                                +               /

     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255, // 48-63
//    0   1   2   3   4   5   6   7   8   9               =

    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 64-79
//        A   B   C   D   E   F   G   H   I   J   K   L   M   N   O

     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255, // 80-95
//    P   Q   R   S   T   U   V   W   X   Y   Z

      0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
//        a   b   c   d   e   f   g   h   i   j   k   l   m   n   o

     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }; // 112-123
//    p   q   r   s   t   u   v   w   x   y   z


void ThreeByteToFourByte(const sal_Int8* pBuffer, const sal_Int32 nStart, const sal_Int32 nFullLen, OUStringBuffer& sBuffer)
{
    sal_Int32 nLen(nFullLen - nStart);
    if (nLen > 3)
        nLen = 3;
    if (nLen == 0)
    {
        return;
    }

    sal_Int32 nBinaer;
    switch (nLen)
    {
        case 1:
        {
            nBinaer = static_cast<sal_uInt8>(pBuffer[nStart + 0]) << 16;
        }
        break;
        case 2:
        {
            nBinaer = (static_cast<sal_uInt8>(pBuffer[nStart + 0]) << 16) +
                    (static_cast<sal_uInt8>(pBuffer[nStart + 1]) <<  8);
        }
        break;
        default:
        {
            nBinaer = (static_cast<sal_uInt8>(pBuffer[nStart + 0]) << 16) +
                    (static_cast<sal_uInt8>(pBuffer[nStart + 1]) <<  8) +
                    static_cast<sal_uInt8>(pBuffer[nStart + 2]);
        }
        break;
    }

    sal_Unicode buf[] = { '=', '=', '=', '=' };

    sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinaer & 0xFC0000) >> 18));
    buf[0] = aBase64EncodeTable [nIndex];

    nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F000) >> 12);
    buf[1] = aBase64EncodeTable [nIndex];
    if (nLen > 1)
    {
        nIndex = static_cast<sal_uInt8>((nBinaer & 0xFC0) >> 6);
        buf[2] = aBase64EncodeTable [nIndex];
        if (nLen > 2)
        {
            nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F));
            buf[3] = aBase64EncodeTable [nIndex];
        }
    }
    sBuffer.append(buf, SAL_N_ELEMENTS(buf));
}

void Base64::encode(OUStringBuffer& aStrBuffer, const uno::Sequence<sal_Int8>& aPass)
{
    sal_Int32 i(0);
    sal_Int32 nBufferLength(aPass.getLength());
    const sal_Int8* pBuffer = aPass.getConstArray();
    while (i < nBufferLength)
    {
        ThreeByteToFourByte(pBuffer, i, nBufferLength, aStrBuffer);
        i += 3;
    }
}

void Base64::decode(uno::Sequence<sal_Int8>& aBuffer, const OUString& sBuffer)
{
    sal_Int32 nCharsDecoded = decodeSomeChars( aBuffer, sBuffer );
    OSL_ENSURE( nCharsDecoded == sBuffer.getLength(), "some bytes left in base64 decoding!" );
}

sal_Int32 Base64::decodeSomeChars(uno::Sequence<sal_Int8>& rOutBuffer, const OUString& rInBuffer)
{
    sal_Int32 nInBufferLen = rInBuffer.getLength();
    sal_Int32 nMinOutBufferLen = (nInBufferLen / 4) * 3;
    if( rOutBuffer.getLength() < nMinOutBufferLen )
        rOutBuffer.realloc( nMinOutBufferLen );

    const sal_Unicode *pInBuffer = rInBuffer.getStr();
    sal_Int8 *pOutBuffer = rOutBuffer.getArray();
    sal_Int8 *pOutBufferStart = pOutBuffer;
    sal_Int32 nCharsDecoded = 0;

    sal_uInt8 aDecodeBuffer[4];
    sal_Int32 nBytesToDecode = 0;
    sal_Int32 nBytesGotFromDecoding = 3;
    sal_Int32 nInBufferPos= 0;
    while( nInBufferPos < nInBufferLen )
    {
        sal_Unicode cChar = *pInBuffer;
        if( cChar >= '+' && cChar <= 'z' )
        {
            sal_uInt8 nByte = aBase64DecodeTable[cChar-'+'];
            if( nByte != 255 )
            {
                // We have found a valid character!
                aDecodeBuffer[nBytesToDecode++] = nByte;

                // One '=' character at the end means 2 out bytes
                // Two '=' characters at the end mean 1 out bytes
                if( '=' == cChar && nBytesToDecode > 2 )
                    nBytesGotFromDecoding--;
                if( 4 == nBytesToDecode )
                {
                    // Four characters found, so we may convert now!
                    sal_uInt32 nOut = (aDecodeBuffer[0] << 18) +
                                      (aDecodeBuffer[1] << 12) +
                                      (aDecodeBuffer[2] << 6) +
                                       aDecodeBuffer[3];

                    *pOutBuffer++  = static_cast<sal_Int8>((nOut & 0xff0000) >> 16);
                    if( nBytesGotFromDecoding > 1 )
                        *pOutBuffer++  = static_cast<sal_Int8>((nOut & 0xff00) >> 8);
                    if( nBytesGotFromDecoding > 2 )
                        *pOutBuffer++  = static_cast<sal_Int8>(nOut & 0xff);
                    nCharsDecoded = nInBufferPos + 1;
                    nBytesToDecode = 0;
                    nBytesGotFromDecoding = 3;
                }
            }
            else
            {
                nCharsDecoded++;
            }
        }
        else
        {
            nCharsDecoded++;
        }

        nInBufferPos++;
        pInBuffer++;
    }
    if( (pOutBuffer - pOutBufferStart) != rOutBuffer.getLength() )
        rOutBuffer.realloc( pOutBuffer - pOutBufferStart );

    return nCharsDecoded;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
