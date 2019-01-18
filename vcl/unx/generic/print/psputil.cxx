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
getHexValueOf (sal_Int32 nValue, OStringBuffer& pBuffer)
{
    const static sal_Char pHex [0x10] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    pBuffer.append(pHex [(nValue & 0xF0) >> 4]);
    pBuffer.append(pHex [(nValue & 0x0F)     ]);

    return 2;
}

sal_Int32
getAlignedHexValueOf (sal_Int32 nValue, OStringBuffer& pBuffer)
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
    auto const start = pBuffer.getLength();
    while (nPrecision)
    {
        OStringBuffer scratch;
        nPrecision -= getHexValueOf (nValue % 256, scratch );
        pBuffer.insert(start, scratch.makeStringAndClear());
        nValue /= 256;
    }

    // set sign bit
    if (bNegative)
    {
        switch (pBuffer[start])
        {
            case '0' : pBuffer[start] = '8'; break;
            case '1' : pBuffer[start] = '9'; break;
            case '2' : pBuffer[start] = 'A'; break;
            case '3' : pBuffer[start] = 'B'; break;
            case '4' : pBuffer[start] = 'C'; break;
            case '5' : pBuffer[start] = 'D'; break;
            case '6' : pBuffer[start] = 'E'; break;
            case '7' : pBuffer[start] = 'F'; break;
            default: OSL_FAIL("Already a signed value");
        }
    }

    // report precision
    return nRet;
}

sal_Int32
getValueOf (sal_Int32 nValue, OStringBuffer& pBuffer)
{
    sal_Int32 nChar = 0;
    if (nValue < 0)
    {
        pBuffer.append('-');
        ++nChar;
        nValue *= -1;
    }
    else
        if (nValue == 0)
        {
            pBuffer.append('0');
            ++nChar;
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
        pBuffer.append(pInvBuffer [--nInvChar]);
        ++nChar;
    }

    return nChar;
}

sal_Int32
appendStr (const sal_Char* pSrc, OStringBuffer& pDst)
{
    sal_Int32 nBytes = strlen (pSrc);
    pDst.append(pSrc, nBytes);

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

} /* namespace psp */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
