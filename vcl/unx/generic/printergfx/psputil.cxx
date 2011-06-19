/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>

#include "psputil.hxx"

#include "tools/debug.hxx"

namespace psp {

/*
 * string convenience routines
 */

sal_Int32
getHexValueOf (sal_Int32 nValue, sal_Char* pBuffer)
{
    const static sal_Char pHex [0x10] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    pBuffer[0] = pHex [(nValue & 0xF0) >> 4];
    pBuffer[1] = pHex [(nValue & 0x0F)     ];

    return 2;
}

sal_Int32
getAlignedHexValueOf (sal_Int32 nValue, sal_Char* pBuffer)
{
    // get sign
    sal_Bool bNegative = nValue < 0;
    nValue = bNegative ? -nValue : nValue;

    // get required buffer size, must be a multiple of two
    sal_Int32 nPrecision;
    if (nValue < 0x80)
        nPrecision = 2;
    else
        if (nValue < 0x8000)
            nPrecision = 4;
        else
            if (nValue < 0x800000)
                nPrecision = 6;
            else
                nPrecision = 8;

    // convert the int into its hex representation, write it into the buffer
    sal_Int32 nRet = nPrecision;
    while (nPrecision)
    {
        nPrecision -= getHexValueOf (nValue % 256, pBuffer + nPrecision - 2 );
        nValue /= 256;
    }

    // set sign bit
    if (bNegative)
    {
        switch (pBuffer[0])
        {
            case '0' : pBuffer[0] = '8'; break;
            case '1' : pBuffer[0] = '9'; break;
            case '2' : pBuffer[0] = 'A'; break;
            case '3' : pBuffer[0] = 'B'; break;
            case '4' : pBuffer[0] = 'C'; break;
            case '5' : pBuffer[0] = 'D'; break;
            case '6' : pBuffer[0] = 'E'; break;
            case '7' : pBuffer[0] = 'F'; break;
            default: OSL_FAIL("Already a signed value");
        }
    }

    // report precision
    return nRet;
}


sal_Int32
getValueOf (sal_Int32 nValue, sal_Char* pBuffer)
{
    sal_Int32 nChar = 0;
    if (nValue < 0)
    {
        pBuffer [nChar++] = '-';
        nValue *= -1;
    }
    else
        if (nValue == 0)
        {
            pBuffer [nChar++] = '0';
            return nChar;
        }

    sal_Char  pInvBuffer [32];
    sal_Int32 nInvChar = 0;
    while (nValue > 0)
    {
        pInvBuffer [nInvChar++] = '0' + nValue % 10;
        nValue /= 10;
    }
    while (nInvChar > 0)
    {
        pBuffer [nChar++] = pInvBuffer [--nInvChar];
    }

    return nChar;
}

sal_Int32
appendStr (const sal_Char* pSrc, sal_Char* pDst)
{
    sal_Int32 nBytes = strlen (pSrc);
    strncpy (pDst, pSrc, nBytes + 1);

    return nBytes;
}

sal_Int32
appendStr (const sal_Char* pSrc, sal_Char* pDst, sal_Int32 nBytes)
{
    strncpy (pDst, pSrc, nBytes);
    pDst [nBytes] = '\0';
    return nBytes;
}

/*
 * copy strings to file
 */

sal_Bool
WritePS (osl::File* pFile, const sal_Char* pString)
{
    sal_uInt64 nInLength = rtl_str_getLength (pString);
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (pString, nInLength, nOutLength);

    return nInLength == nOutLength;
}

sal_Bool
WritePS (osl::File* pFile, const sal_Char* pString, sal_uInt64 nInLength)
{
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (pString, nInLength, nOutLength);

    return nInLength == nOutLength;
}

sal_Bool
WritePS (osl::File* pFile, const rtl::OString &rString)
{
    sal_uInt64 nInLength = rString.getLength();
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (rString, nInLength, nOutLength);

    return nInLength == nOutLength;
}

sal_Bool
WritePS (osl::File* pFile, const rtl::OUString &rString)
{
    return WritePS (pFile, rtl::OUStringToOString(rString, RTL_TEXTENCODING_ASCII_US));
}

/*
 * cache converter for use in postscript drawing routines
 */

ConverterFactory::ConverterFactory()
{
}

ConverterFactory::~ConverterFactory ()
{
    for( std::map< rtl_TextEncoding, rtl_UnicodeToTextConverter >::const_iterator it = m_aConverters.begin(); it != m_aConverters.end(); ++it )
            rtl_destroyUnicodeToTextConverter (it->second);
}

rtl_UnicodeToTextConverter
ConverterFactory::Get (rtl_TextEncoding nEncoding)
{
    if (rtl_isOctetTextEncoding( nEncoding ))
    {
        std::map< rtl_TextEncoding, rtl_UnicodeToTextConverter >::const_iterator it =
            m_aConverters.find( nEncoding );
        rtl_UnicodeToTextConverter aConverter;
        if (it == m_aConverters.end())
        {
            aConverter = rtl_createUnicodeToTextConverter (nEncoding);
            m_aConverters[nEncoding] = aConverter;
        }
        else
            aConverter = it->second;
        return aConverter;
    }
    return NULL;
}

// wrapper for rtl_convertUnicodeToText that handles the usual cases for
// textconversion in drawtext
sal_Size
ConverterFactory::Convert (const sal_Unicode *pText, int nTextLen,
                           sal_uChar *pBuffer, sal_Size nBufferSize, rtl_TextEncoding nEncoding)
{
    const sal_uInt32 nCvtFlags =  RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
        | RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK ;
    sal_uInt32  nCvtInfo;
    sal_Size    nCvtChars;

    rtl_UnicodeToTextConverter aConverter = Get (nEncoding);
    rtl_UnicodeToTextContext   aContext   = rtl_createUnicodeToTextContext (aConverter);

    sal_Size nSize = rtl_convertUnicodeToText (aConverter, aContext,
                                               pText, nTextLen, (sal_Char*)pBuffer, nBufferSize,
                                               nCvtFlags, &nCvtInfo, &nCvtChars);

    rtl_destroyUnicodeToTextContext (aConverter, aContext);

    return nSize;
}

ConverterFactory*
GetConverterFactory ()
{
    static ConverterFactory* pCvt = NULL;

    if (pCvt == NULL)
        pCvt = new ConverterFactory;

    return pCvt;
}


} /* namespace psp */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
