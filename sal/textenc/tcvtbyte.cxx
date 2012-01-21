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

#include "sal/config.h"

#include "rtl/textcvt.h"

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
    sal_uChar               c;
    sal_Unicode*            pEndDestBuf;
    const char*         pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        /* 0-31 (all Control-Character get the same Unicode value) */
        c = (sal_uChar)*pSrcBuf;
        if ( c <= 0x1F )
            *pDestBuf = (sal_Unicode)c;
        else
            *pDestBuf = ((sal_Unicode)c)+0xF000;
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
    sal_uChar                   c;
    sal_Unicode                 cConv;
    const ImplByteConvertData*  pConvertData = (const ImplByteConvertData*)pData;
    sal_Unicode*                pEndDestBuf;
    const char*             pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    if ( pDestBuf == pEndDestBuf )
    {
        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        *pSrcCvtBytes = 0;
        return 0;
    }
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = (sal_uChar)*pSrcBuf;
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
