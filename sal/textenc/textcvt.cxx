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

#include "sal/config.h"

#include "rtl/textcvt.h"

#include "gettextencodingdata.hxx"
#include "tenchelp.hxx"

/* ======================================================================= */

static sal_Size ImplDummyToUnicode( const char* pSrcBuf, sal_Size nSrcBytes,
                                    sal_Unicode* pDestBuf, sal_Size nDestChars,
                                    sal_uInt32 nFlags, sal_uInt32* pInfo,
                                    sal_Size* pSrcCvtBytes )
{
    sal_Unicode*        pEndDestBuf;
    const char*     pEndSrcBuf;

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

        *pDestBuf = (sal_Unicode)(unsigned char)*pSrcBuf;
        pDestBuf++;
        pSrcBuf++;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

static sal_Size ImplUnicodeToDummy( const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                    char* pDestBuf, sal_Size nDestBytes,
                                    sal_uInt32 nFlags, sal_uInt32* pInfo,
                                    sal_Size* pSrcCvtChars )
{
    char*               pEndDestBuf;
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

        *pDestBuf = (char)(unsigned char)(*pSrcBuf & 0x00FF);
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
        return static_cast<rtl_TextToUnicodeConverter>(const_cast<ImplTextConverter *>(&pData->maConverter));
    else
        return nullptr;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyTextToUnicodeConverter(
    SAL_UNUSED_PARAMETER rtl_TextToUnicodeConverter )
{}

/* ----------------------------------------------------------------------- */

rtl_TextToUnicodeContext SAL_CALL rtl_createTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( !pConverter )
        return nullptr;
    else if ( pConverter->mpCreateTextToUnicodeContext )
        return pConverter->mpCreateTextToUnicodeContext();
    else
        return reinterpret_cast<rtl_TextToUnicodeContext>(1);
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter,
                                               rtl_TextToUnicodeContext hContext )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( pConverter && hContext && pConverter->mpDestroyTextToUnicodeContext )
        pConverter->mpDestroyTextToUnicodeContext( hContext );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_resetTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter,
                                             rtl_TextToUnicodeContext hContext )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( pConverter && hContext && pConverter->mpResetTextToUnicodeContext )
        pConverter->mpResetTextToUnicodeContext( hContext );
}

/* ----------------------------------------------------------------------- */

sal_Size SAL_CALL rtl_convertTextToUnicode( rtl_TextToUnicodeConverter hConverter,
                                            rtl_TextToUnicodeContext hContext,
                                            const char* pSrcBuf, sal_Size nSrcBytes,
                                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtBytes )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);

    /* Only temporary, because we don't want die, if we don't have a
       converter, because not all converters are implemented yet */
    if ( !pConverter )
    {
        return ImplDummyToUnicode( pSrcBuf, nSrcBytes,
                                   pDestBuf, nDestChars,
                                   nFlags, pInfo, pSrcCvtBytes );
    }

    return pConverter->mpConvertTextToUnicodeProc( pConverter->mpConvertData,
                                                   hContext,
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
        return static_cast<rtl_TextToUnicodeConverter>(const_cast<ImplTextConverter *>(&pData->maConverter));
    else
        return nullptr;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyUnicodeToTextConverter(
    SAL_UNUSED_PARAMETER rtl_UnicodeToTextConverter )
{}

/* ----------------------------------------------------------------------- */

rtl_UnicodeToTextContext SAL_CALL rtl_createUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( !pConverter )
        return nullptr;
    else if ( pConverter->mpCreateUnicodeToTextContext )
        return pConverter->mpCreateUnicodeToTextContext();
    else
        return reinterpret_cast<rtl_UnicodeToTextContext>(1);
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_destroyUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter,
                                               rtl_UnicodeToTextContext hContext )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( pConverter && hContext && pConverter->mpDestroyUnicodeToTextContext )
        pConverter->mpDestroyUnicodeToTextContext( hContext );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_resetUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter,
                                             rtl_UnicodeToTextContext hContext )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);
    if ( pConverter && hContext && pConverter->mpResetUnicodeToTextContext )
        pConverter->mpResetUnicodeToTextContext( hContext );
}

/* ----------------------------------------------------------------------- */

sal_Size SAL_CALL rtl_convertUnicodeToText( rtl_UnicodeToTextConverter hConverter,
                                            rtl_UnicodeToTextContext hContext,
                                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                            char* pDestBuf, sal_Size nDestBytes,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtChars )
{
    const ImplTextConverter* pConverter = static_cast<const ImplTextConverter*>(hConverter);

    /* Only temporary, because we don't want die, if we don't have a
       converter, because not all converters are implemented yet */
    if ( !pConverter )
    {
        return ImplUnicodeToDummy( pSrcBuf, nSrcChars,
                                   pDestBuf, nDestBytes,
                                   nFlags, pInfo, pSrcCvtChars );
    }

    return pConverter->mpConvertUnicodeToTextProc( pConverter->mpConvertData,
                                                   hContext,
                                                   pSrcBuf, nSrcChars,
                                                   pDestBuf, nDestBytes,
                                                   nFlags, pInfo,
                                                   pSrcCvtChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
