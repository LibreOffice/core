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

#include <sal/config.h>

#include <rtl/textcvt.h>

#include "handleundefinedunicodetotextchar.hxx"
#include "tcvtbyte.hxx"
#include "tenchelp.hxx"

sal_Size ImplSymbolToUnicode( SAL_UNUSED_PARAMETER const void*,
                              SAL_UNUSED_PARAMETER void*,
                              const char* pSrcBuf, sal_Size nSrcBytes,
                              sal_Unicode* pDestBuf, sal_Size nDestChars,
                              SAL_UNUSED_PARAMETER sal_uInt32,
                              sal_uInt32* pInfo, sal_Size* pSrcCvtBytes )
{
    sal_Unicode*            pEndDestBuf;
    const char*         pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
            break;
        }

        /* 0-31 (all Control-Character get the same Unicode value) */
        unsigned char c = static_cast<unsigned char>(*pSrcBuf);
        if ( c <= 0x1F )
            *pDestBuf = static_cast<sal_Unicode>(c);
        else
            *pDestBuf = static_cast<sal_Unicode>(c)+0xF000;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

sal_Size ImplUnicodeToSymbol( SAL_UNUSED_PARAMETER const void*,
                              SAL_UNUSED_PARAMETER void*,
                              const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                              char* pDestBuf, sal_Size nDestBytes,
                              sal_uInt32 nFlags, sal_uInt32* pInfo,
                              sal_Size* pSrcCvtChars )
{
    sal_Unicode             c;
    char*               pEndDestBuf;
    const sal_Unicode*      pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
            break;
        }

        c = *pSrcBuf;
        if ( (c >= 0xF000) && (c <= 0xF0FF) )
        {
            *pDestBuf = static_cast< char >(static_cast< unsigned char >(c-0xF000));
            pDestBuf++;
            pSrcBuf++;
        }
        // Normally 0x001F, but in many cases also symbol characters
        // are stored in the first 256 bytes, so that we don't change
        // these values
        else if ( c <= 0x00FF )
        {
            *pDestBuf = static_cast< char >(static_cast< unsigned char >(c));
            pDestBuf++;
            pSrcBuf++;
        }
        else
        {
            if ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE )
            {
                /* !!! */
                /* Only ascii  characters < 0x1F */
            }

            /* Handle undefined and surrogates characters */
            /* (all surrogates characters are undefined) */
            if (!sal::detail::textenc::handleUndefinedUnicodeToTextChar(
                    &pSrcBuf, pEndSrcBuf, &pDestBuf, pEndDestBuf, nFlags,
                    pInfo))
                break;
        }
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

sal_Size ImplUpperCharToUnicode( const void* pData,
                            SAL_UNUSED_PARAMETER void*,
                            const char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            SAL_UNUSED_PARAMETER sal_uInt32, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    sal_Unicode                 cConv;
    const ImplByteConvertData*  pConvertData = static_cast<const ImplByteConvertData*>(pData);
    sal_Unicode*                pEndDestBuf;
    const char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    if ( pDestBuf == pEndDestBuf )
    {
        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
        *pSrcCvtBytes = 0;
        return 0;
    }
    while ( pSrcBuf < pEndSrcBuf )
    {
        unsigned char c = static_cast<unsigned char>(*pSrcBuf);
        if (c < 0x80)
            cConv = c;
        else
            // c <= 0xFF is implied.
            cConv = pConvertData->mpToUniTab1[c - 0x80];

        *pDestBuf = cConv;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
