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

#include "sal/types.h"
#include "rtl/textcvt.h"

#include "converter.hxx"
#include "tcvtutf8.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

struct ImplUtf8ToUnicodeContext
{
    sal_uInt32 nUtf32;
    int nShift;
    bool bCheckBom;
};

struct ImplUnicodeToUtf8Context
{
    sal_Unicode nHighSurrogate; /* 0xFFFF: write BOM */
};

void * ImplCreateUtf8ToUnicodeContext()
{
    ImplUtf8ToUnicodeContext * p = new ImplUtf8ToUnicodeContext;
    ImplResetUtf8ToUnicodeContext(p);
    return p;
}

void ImplResetUtf8ToUnicodeContext(void * pContext)
{
    if (pContext != nullptr)
    {
        static_cast< ImplUtf8ToUnicodeContext * >(pContext)->nShift = -1;
        static_cast< ImplUtf8ToUnicodeContext * >(pContext)->bCheckBom = true;
    }
}

void ImplDestroyUtf8ToUnicodeContext(void * pContext)
{
    delete static_cast< ImplUtf8ToUnicodeContext * >(pContext);
}

sal_Size ImplConvertUtf8ToUnicode(
    void const * pData, void * pContext, char const * pSrcBuf,
    sal_Size nSrcBytes, sal_Unicode * pDestBuf, sal_Size nDestChars,
    sal_uInt32 nFlags, sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes)
{
    /*
       This function is very liberal with the UTF-8 input.  Accepted are:
       - non-shortest forms (e.g., C0 41 instead of 41 to represent U+0041)
       - surrogates (e.g., ED A0 80 to represent U+D800)
       - encodings with up to six bytes (everything outside the range
         U+0000..10FFFF is considered "undefined")
       The first two of these points allow this routine to translate from both
       RTL_TEXTENCODING_UTF8 and RTL_TEXTENCODING_JAVA_UTF8.
      */

    bool bJavaUtf8 = pData != nullptr;
    sal_uInt32 nUtf32 = 0;
    int nShift = -1;
    bool bCheckBom = true;
    sal_uInt32 nInfo = 0;
    unsigned char const * pSrcBufPtr = reinterpret_cast<unsigned char const *>(pSrcBuf);
    unsigned char const * pSrcBufEnd = pSrcBufPtr + nSrcBytes;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBufPtr + nDestChars;

    if (pContext != nullptr)
    {
        nUtf32 = static_cast< ImplUtf8ToUnicodeContext * >(pContext)->nUtf32;
        nShift = static_cast< ImplUtf8ToUnicodeContext * >(pContext)->nShift;
        bCheckBom = static_cast< ImplUtf8ToUnicodeContext * >(pContext)->bCheckBom;
    }

    while (pSrcBufPtr < pSrcBufEnd)
    {
        bool bUndefined = false;
        bool bConsume = true;
        sal_uInt32 nChar = *pSrcBufPtr++;
        if (nShift < 0)
            if (nChar <= 0x7F)
            {
                nUtf32 = nChar;
                goto transform;
            }
            else if (nChar <= 0xBF)
                goto bad_input;
            else if (nChar <= 0xDF)
            {
                nUtf32 = (nChar & 0x1F) << 6;
                nShift = 0;
            }
            else if (nChar <= 0xEF)
            {
                nUtf32 = (nChar & 0x0F) << 12;
                nShift = 6;
            }
            else if (nChar <= 0xF7)
            {
                nUtf32 = (nChar & 0x07) << 18;
                nShift = 12;
            }
            else if (nChar <= 0xFB)
            {
                nUtf32 = (nChar & 0x03) << 24;
                nShift = 18;
            }
            else if (nChar <= 0xFD)
            {
                nUtf32 = (nChar & 0x01) << 30;
                nShift = 24;
            }
            else
                goto bad_input;
        else if ((nChar & 0xC0) == 0x80)
        {
            nUtf32 |= (nChar & 0x3F) << nShift;
            if (nShift == 0)
                goto transform;
            else
                nShift -= 6;
        }
        else
        {
            /*
             This byte is preceded by a broken UTF-8 sequence; if this byte
             is neither in the range [0x80..0xBF] nor in the range
             [0xFE..0xFF], assume that this byte does not belong to that
             broken sequence, but instead starts a new, legal UTF-8 sequence:
             */
            bConsume = nChar >= 0xFE;
            goto bad_input;
        }
        continue;

    transform:
        if (!bCheckBom || nUtf32 != 0xFEFF
            || (nFlags & RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE) == 0
            || bJavaUtf8)
        {
            if (nUtf32 <= 0xFFFF)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nUtf32;
                else
                    goto no_output;
            else if (rtl::isUnicodeCodePoint(nUtf32))
                if (pDestBufEnd - pDestBufPtr >= 2)
                {
                    *pDestBufPtr++ = (sal_Unicode) ImplGetHighSurrogate(nUtf32);
                    *pDestBufPtr++ = (sal_Unicode) ImplGetLowSurrogate(nUtf32);
                }
                else
                    goto no_output;
            else
            {
                bUndefined = true;
                goto bad_input;
            }
        }
        nShift = -1;
        bCheckBom = false;
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                    bUndefined, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            nShift = -1;
            bCheckBom = false;
            if (!bConsume)
                --pSrcBufPtr;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            nShift = -1;
            bCheckBom = false;
            if (!bConsume)
                --pSrcBufPtr;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;

    no_output:
        --pSrcBufPtr;
        nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        break;
    }

    if (nShift >= 0
        && (nInfo & (RTL_TEXTTOUNICODE_INFO_ERROR
                         | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL))
               == 0)
    {
        if ((nFlags & RTL_TEXTTOUNICODE_FLAGS_FLUSH) == 0)
            nInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
        else
            switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                        false, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                        &nInfo))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                nShift = -1;
                bCheckBom = false;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext != nullptr)
    {
        static_cast< ImplUtf8ToUnicodeContext * >(pContext)->nUtf32 = nUtf32;
        static_cast< ImplUtf8ToUnicodeContext * >(pContext)->nShift = nShift;
        static_cast< ImplUtf8ToUnicodeContext * >(pContext)->bCheckBom = bCheckBom;
    }
    if (pInfo != nullptr)
        *pInfo = nInfo;
    if (pSrcCvtBytes != nullptr)
        *pSrcCvtBytes = reinterpret_cast< char const * >(pSrcBufPtr) - pSrcBuf;
    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToUtf8Context()
{
    ImplUnicodeToUtf8Context * p = new ImplUnicodeToUtf8Context;
    ImplResetUnicodeToUtf8Context(p);
    return p;
}

void ImplResetUnicodeToUtf8Context(void * pContext)
{
    if (pContext != nullptr)
        static_cast< ImplUnicodeToUtf8Context * >(pContext)->nHighSurrogate = 0xFFFF;
}

void ImplDestroyUnicodeToUtf8Context(void * pContext)
{
    delete static_cast< ImplUnicodeToUtf8Context * >(pContext);
}

sal_Size ImplConvertUnicodeToUtf8(
    void const * pData, void * pContext, sal_Unicode const * pSrcBuf,
    sal_Size nSrcChars, char * pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size * pSrcCvtChars)
{
    bool bJavaUtf8 = pData != nullptr;
    sal_Unicode nHighSurrogate = 0xFFFF;
    sal_uInt32 nInfo = 0;
    sal_Unicode const * pSrcBufPtr = pSrcBuf;
    sal_Unicode const * pSrcBufEnd = pSrcBufPtr + nSrcChars;
    char * pDestBufPtr = pDestBuf;
    char * pDestBufEnd = pDestBufPtr + nDestBytes;

    if (pContext != nullptr)
        nHighSurrogate
            = static_cast< ImplUnicodeToUtf8Context * >(pContext)->nHighSurrogate;

    if (nHighSurrogate == 0xFFFF)
    {
        if ((nFlags & RTL_UNICODETOTEXT_FLAGS_GLOBAL_SIGNATURE) != 0
            && !bJavaUtf8)
        {
            if (pDestBufEnd - pDestBufPtr >= 3)
            {
                /* Write BOM (U+FEFF) as UTF-8: */
                *pDestBufPtr++ = static_cast< char >(static_cast< unsigned char >(0xEF));
                *pDestBufPtr++ = static_cast< char >(static_cast< unsigned char >(0xBB));
                *pDestBufPtr++ = static_cast< char >(static_cast< unsigned char >(0xBF));
            }
            else
            {
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                goto done;
            }
        }
        nHighSurrogate = 0;
    }

    while (pSrcBufPtr < pSrcBufEnd)
    {
        sal_uInt32 nChar = *pSrcBufPtr++;
        if (nHighSurrogate == 0)
        {
            if (ImplIsHighSurrogate(nChar) && !bJavaUtf8)
            {
                nHighSurrogate = (sal_Unicode) nChar;
                continue;
            }
        }
        else if (ImplIsLowSurrogate(nChar) && !bJavaUtf8)
            nChar = ImplCombineSurrogates(nHighSurrogate, nChar);
        else
            goto bad_input;

        if ((ImplIsLowSurrogate(nChar) && !bJavaUtf8)
            || ImplIsNoncharacter(nChar))
            goto bad_input;

        if (nChar <= 0x7F && (!bJavaUtf8 || nChar != 0))
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = static_cast< char >(nChar);
            else
                goto no_output;
        else if (nChar <= 0x7FF)
            if (pDestBufEnd - pDestBufPtr >= 2)
            {
                *pDestBufPtr++ = static_cast< char >(0xC0 | (nChar >> 6));
                *pDestBufPtr++ = static_cast< char >(0x80 | (nChar & 0x3F));
            }
            else
                goto no_output;
        else if (nChar <= 0xFFFF)
            if (pDestBufEnd - pDestBufPtr >= 3)
            {
                *pDestBufPtr++ = static_cast< char >(0xE0 | (nChar >> 12));
                *pDestBufPtr++ = static_cast< char >(0x80 | ((nChar >> 6) & 0x3F));
                *pDestBufPtr++ = static_cast< char >(0x80 | (nChar & 0x3F));
            }
            else
                goto no_output;
        else if (pDestBufEnd - pDestBufPtr >= 4)
        {
            *pDestBufPtr++ = static_cast< char >(0xF0 | (nChar >> 18));
            *pDestBufPtr++ = static_cast< char >(0x80 | ((nChar >> 12) & 0x3F));
            *pDestBufPtr++ = static_cast< char >(0x80 | ((nChar >> 6) & 0x3F));
            *pDestBufPtr++ = static_cast< char >(0x80 | (nChar & 0x3F));
        }
        else
            goto no_output;
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    false, 0, nFlags, &pDestBufPtr, pDestBufEnd, &nInfo, nullptr,
                    0, nullptr))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            nHighSurrogate = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            nHighSurrogate = 0;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;

    no_output:
        --pSrcBufPtr;
        nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        break;
    }

    if (nHighSurrogate != 0
        && (nInfo & (RTL_UNICODETOTEXT_INFO_ERROR
                         | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
               == 0)
    {
        if ((nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
            nInfo |= RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
        else
            switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                        false, 0, nFlags, &pDestBufPtr, pDestBufEnd, &nInfo,
                        nullptr, 0, nullptr))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                nHighSurrogate = 0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

 done:
    if (pContext != nullptr)
        static_cast< ImplUnicodeToUtf8Context * >(pContext)->nHighSurrogate
            = nHighSurrogate;
    if (pInfo != nullptr)
        *pInfo = nInfo;
    if (pSrcCvtChars != nullptr)
        *pSrcCvtChars = pSrcBufPtr - pSrcBuf;
    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
