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

#include "rtl/textcvt.h"
#include "gettextencodingdata.h"
#include "tenchelp.h"

/* ======================================================================= */

static sal_Size ImplDummyToUnicode( const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                                    sal_Unicode* pDestBuf, sal_Size nDestChars,
                                    sal_uInt32 nFlags, sal_uInt32* pInfo,
                                    sal_Size* pSrcCvtBytes )
{
    sal_Unicode*        pEndDestBuf;
    const sal_Char*     pEndSrcBuf;

    if ( ((nFlags & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR) ||
         ((nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR) )
    {
        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR |
                  RTL_TEXTTOUNICODE_INFO_UNDEFINED |
                  RTL_TEXTTOUNICODE_INFO_MBUNDEFINED;
        return 0;
    }

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

        *pDestBuf = (sal_Unicode)(sal_uChar)*pSrcBuf;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

static sal_Size ImplUnicodeToDummy( const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                    sal_Char* pDestBuf, sal_Size nDestBytes,
                                    sal_uInt32 nFlags, sal_uInt32* pInfo,
                                    sal_Size* pSrcCvtChars )
{
    sal_Char*               pEndDestBuf;
    const sal_Unicode*      pEndSrcBuf;

    if ( ((nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR) )
    {
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR |
                  RTL_UNICODETOTEXT_INFO_UNDEFINED;
        return 0;
    }

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

        *pDestBuf = (sal_Char)(sal_uChar)(*pSrcBuf & 0x00FF);
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}

/* ======================================================================= */

rtl_TextToUnicodeConverter SAL_CALL rtl_createTextToUnicodeConverter( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return (rtl_TextToUnicodeConverter) &pData->maConverter;
    else
        return 0;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyTextToUnicodeConverter( rtl_TextToUnicodeConverter hContext )
{
    (void) hContext; /* unused */
}

/* ----------------------------------------------------------------------- */

rtl_TextToUnicodeContext SAL_CALL rtl_createTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( !pConverter )
        return 0;
    else if ( pConverter->mpCreateTextToUnicodeContext )
        return (rtl_TextToUnicodeContext)pConverter->mpCreateTextToUnicodeContext();
    else
        return (rtl_TextToUnicodeContext)1;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter,
                                               rtl_TextToUnicodeContext hContext )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( pConverter && hContext && pConverter->mpDestroyTextToUnicodeContext )
        pConverter->mpDestroyTextToUnicodeContext( (void*)hContext );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_resetTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter,
                                             rtl_TextToUnicodeContext hContext )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( pConverter && hContext && pConverter->mpResetTextToUnicodeContext )
        pConverter->mpResetTextToUnicodeContext( (void*)hContext );
}

/* ----------------------------------------------------------------------- */

sal_Size SAL_CALL rtl_convertTextToUnicode( rtl_TextToUnicodeConverter hConverter,
                                            rtl_TextToUnicodeContext hContext,
                                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtBytes )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;

    /* Only temporaer, because we don't want die, if we don't have a
       converter, because not all converters are implemented yet */
    if ( !pConverter )
    {
        return ImplDummyToUnicode( pSrcBuf, nSrcBytes,
                                   pDestBuf, nDestChars,
                                   nFlags, pInfo, pSrcCvtBytes );
    }

    return pConverter->mpConvertTextToUnicodeProc( pConverter->mpConvertData,
                                                   (void*)hContext,
                                                   pSrcBuf, nSrcBytes,
                                                   pDestBuf, nDestChars,
                                                   nFlags, pInfo,
                                                   pSrcCvtBytes );
}

/* ======================================================================= */

rtl_UnicodeToTextConverter SAL_CALL rtl_createUnicodeToTextConverter( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return (rtl_TextToUnicodeConverter) &pData->maConverter;
    else
        return 0;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyUnicodeToTextConverter( rtl_UnicodeToTextConverter hConverter )
{
    (void) hConverter; /* unused */
}

/* ----------------------------------------------------------------------- */

rtl_UnicodeToTextContext SAL_CALL rtl_createUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( !pConverter )
        return 0;
    else if ( pConverter->mpCreateUnicodeToTextContext )
        return (rtl_UnicodeToTextContext)pConverter->mpCreateUnicodeToTextContext();
    else
        return (rtl_UnicodeToTextContext)1;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter,
                                               rtl_UnicodeToTextContext hContext )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( pConverter && hContext && pConverter->mpDestroyUnicodeToTextContext )
        pConverter->mpDestroyUnicodeToTextContext( (void*)hContext );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_resetUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter,
                                             rtl_UnicodeToTextContext hContext )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;
    if ( pConverter && hContext && pConverter->mpResetUnicodeToTextContext )
        pConverter->mpResetUnicodeToTextContext( (void*)hContext );
}

/* ----------------------------------------------------------------------- */

sal_Size SAL_CALL rtl_convertUnicodeToText( rtl_UnicodeToTextConverter hConverter,
                                            rtl_UnicodeToTextContext hContext,
                                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                            sal_Char* pDestBuf, sal_Size nDestBytes,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtChars )
{
    const ImplTextConverter* pConverter = (const ImplTextConverter*)hConverter;

    /* Only temporaer, because we don't want die, if we don't have a
       converter, because not all converters are implemented yet */
    if ( !pConverter )
    {
        return ImplUnicodeToDummy( pSrcBuf, nSrcChars,
                                   pDestBuf, nDestBytes,
                                   nFlags, pInfo, pSrcCvtChars );
    }

    return pConverter->mpConvertUnicodeToTextProc( pConverter->mpConvertData,
                                                   (void*)hContext,
                                                   pSrcBuf, nSrcChars,
                                                   pDestBuf, nDestBytes,
                                                   nFlags, pInfo,
                                                   pSrcCvtChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
