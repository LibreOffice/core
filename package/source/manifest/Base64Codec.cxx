/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Base64Codec.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:10:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASE64_CODEC_HXX
#include "Base64Codec.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
using namespace rtl;
using namespace osl;
using namespace com::sun::star;

const
  sal_Char aBase64EncodeTable[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

const
  sal_uInt8 aBase64DecodeTable[]  =
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0-15

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 16-31

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63, // 32-47
//                                                +               /

     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0, // 48-63
//    0   1   2   3   4   5   6   7   8   9               =

      0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 64-79
//        A   B   C   D   E   F   G   H   I   J   K   L   M   N   O

     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0, // 80-95
//    P   Q   R   S   T   U   V   W   X   Y   Z

      0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
//        a   b   c   d   e   f   g   h   i   j   k   l   m   n   o

     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0, // 112-127
//    p   q   r   s   t   u   v   w   x   y   z

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};


void ThreeByteToFourByte (const sal_uInt8* pBuffer, const sal_Int32 nStart, const sal_Int32 nFullLen, rtl::OUStringBuffer& sBuffer)
{
    sal_Int32 nLen(nFullLen - nStart);
    if (nLen > 3)
        nLen = 3;
    if (nLen == 0)
    {
        sBuffer.setLength(0);
        return;
    }

    sal_Int32 nBinaer;
    switch (nLen)
    {
        case 1:
        {
            nBinaer = ((sal_uInt8)pBuffer[nStart + 0]) << 16;
        }
        break;
        case 2:
        {
            nBinaer = (((sal_uInt8)pBuffer[nStart + 0]) << 16) +
                    (((sal_uInt8)pBuffer[nStart + 1]) <<  8);
        }
        break;
        default:
        {
            nBinaer = (((sal_uInt8)pBuffer[nStart + 0]) << 16) +
                    (((sal_uInt8)pBuffer[nStart + 1]) <<  8) +
                    ((sal_uInt8)pBuffer[nStart + 2]);
        }
        break;
    }

    sBuffer.appendAscii("====");

    sal_uInt8 nIndex = static_cast< sal_uInt8 >((nBinaer & 0xFC0000) >> 18);
    sBuffer.setCharAt(0, aBase64EncodeTable [nIndex]);

    nIndex = static_cast< sal_uInt8 >((nBinaer & 0x3F000) >> 12);
    sBuffer.setCharAt(1, aBase64EncodeTable [nIndex]);
    if (nLen == 1)
        return;

    nIndex = static_cast< sal_uInt8 >((nBinaer & 0xFC0) >> 6);
    sBuffer.setCharAt(2, aBase64EncodeTable [nIndex]);
    if (nLen == 2)
        return;

    nIndex = static_cast< sal_uInt8 >(nBinaer & 0x3F);
    sBuffer.setCharAt(3, aBase64EncodeTable [nIndex]);
}

void Base64Codec::encodeBase64(rtl::OUStringBuffer& aStrBuffer, const uno::Sequence < sal_uInt8 >& aPass)
{
    sal_Int32 i(0);
    sal_Int32 nBufferLength(aPass.getLength());
    const sal_uInt8* pBuffer = aPass.getConstArray();
    while (i < nBufferLength)
    {
        rtl::OUStringBuffer sBuffer;
        ThreeByteToFourByte (pBuffer, i, nBufferLength, sBuffer);
        aStrBuffer.append(sBuffer);
        i += 3;
    }
}

const rtl::OUString s2equal(RTL_CONSTASCII_USTRINGPARAM("=="));
const rtl::OUString s1equal(RTL_CONSTASCII_USTRINGPARAM("="));

void FourByteToThreeByte (sal_uInt8* pBuffer, sal_Int32& nLength, const sal_Int32 nStart, const rtl::OUString& sString)
{
    nLength = 0;
    sal_Int32 nLen (sString.getLength());

    OSL_ASSERT( nLen == 4 );
    if (nLen != 4)
        return;

    if (sString.indexOf(s2equal) == 2)
        nLength = 1;
    else if (sString.indexOf(s1equal) == 3)
        nLength = 2;
    else
        nLength = 3;

    sal_Int32 nBinaer ((aBase64DecodeTable [sString [0]] << 18) +
            (aBase64DecodeTable [sString [1]] << 12) +
            (aBase64DecodeTable [sString [2]] <<  6) +
            (aBase64DecodeTable [sString [3]]));

    sal_uInt8 OneByte = static_cast< sal_uInt8 >((nBinaer & 0xFF0000) >> 16);
    pBuffer[nStart + 0] = (sal_uInt8)OneByte;

    if (nLength == 1)
        return;

    OneByte = static_cast< sal_uInt8 >((nBinaer & 0xFF00) >> 8);
    pBuffer[nStart + 1] = OneByte;

    if (nLength == 2)
        return;

    OneByte = static_cast< sal_uInt8 >(nBinaer & 0xFF);
    pBuffer[nStart + 2] = OneByte;
}

void Base64Codec::decodeBase64(uno::Sequence< sal_uInt8 >& aBuffer, const rtl::OUString& sBuffer)
{
    sal_Int32 nFirstLength((sBuffer.getLength() / 4) * 3);
    sal_uInt8* pBuffer = new sal_uInt8[nFirstLength];
    sal_Int32 nSecondLength(0);
    sal_Int32 nLength(0);
    sal_Int32 i = 0;
    sal_Int32 k = 0;
    while (i < sBuffer.getLength())
    {
        FourByteToThreeByte (pBuffer, nLength, k, sBuffer.copy(i, 4));
        nSecondLength += nLength;
        nLength = 0;
        i += 4;
        k += 3;
    }
    aBuffer = uno::Sequence<sal_uInt8>(pBuffer, nSecondLength);
    delete[] pBuffer;
}
