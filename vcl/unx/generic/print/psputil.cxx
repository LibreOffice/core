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

#include <string.h>
#include <rtl/instance.hxx>
#include "psputil.hxx"

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
    bool bNegative = nValue < 0;
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

/*
 * copy strings to file
 */

bool
WritePS (osl::File* pFile, const sal_Char* pString)
{
    sal_uInt64 nInLength = rtl_str_getLength (pString);
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (pString, nInLength, nOutLength);

    return nInLength == nOutLength;
}

bool
WritePS (osl::File* pFile, const sal_Char* pString, sal_uInt64 nInLength)
{
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (pString, nInLength, nOutLength);

    return nInLength == nOutLength;
}

bool
WritePS (osl::File* pFile, const OString &rString)
{
    sal_uInt64 nInLength = rString.getLength();
    sal_uInt64 nOutLength = 0;

    if (nInLength > 0 && pFile)
        pFile->write (rString.getStr(), nInLength, nOutLength);

    return nInLength == nOutLength;
}

bool
WritePS (osl::File* pFile, const OUString &rString)
{
    return WritePS (pFile, OUStringToOString(rString, RTL_TEXTENCODING_ASCII_US));
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
    return nullptr;
}

// wrapper for rtl_convertUnicodeToText that handles the usual cases for
// textconversion in drawtext
sal_Size
ConverterFactory::Convert (const sal_Unicode *pText, int nTextLen,
                           unsigned char *pBuffer, sal_Size nBufferSize, rtl_TextEncoding nEncoding)
{
    const sal_uInt32 nCvtFlags =  RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
        | RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK ;
    sal_uInt32  nCvtInfo;
    sal_Size    nCvtChars;

    rtl_UnicodeToTextConverter aConverter = Get (nEncoding);
    rtl_UnicodeToTextContext   aContext   = rtl_createUnicodeToTextContext (aConverter);

    sal_Size nSize = rtl_convertUnicodeToText (aConverter, aContext,
                                               pText, nTextLen, reinterpret_cast<char*>(pBuffer), nBufferSize,
                                               nCvtFlags, &nCvtInfo, &nCvtChars);

    rtl_destroyUnicodeToTextContext (aConverter, aContext);

    return nSize;
}

namespace
{
    class theConverterFactory
        : public rtl::Static<ConverterFactory, theConverterFactory>
    {
    };
}

ConverterFactory& GetConverterFactory()
{
    return theConverterFactory::get();
}

} /* namespace psp */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
