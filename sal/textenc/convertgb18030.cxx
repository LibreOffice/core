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
#include "sal/types.h"

#include "context.hxx"
#include "converter.hxx"
#include "convertgb18030.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

enum ImplGb18030ToUnicodeState
{
    IMPL_GB_18030_TO_UNICODE_STATE_0,
    IMPL_GB_18030_TO_UNICODE_STATE_1,
    IMPL_GB_18030_TO_UNICODE_STATE_2,
    IMPL_GB_18030_TO_UNICODE_STATE_3
};

struct ImplGb18030ToUnicodeContext
{
    ImplGb18030ToUnicodeState m_eState;
    sal_uInt32 m_nCode;
};

}

void * ImplCreateGb18030ToUnicodeContext()
{
    ImplGb18030ToUnicodeContext * pContext = new ImplGb18030ToUnicodeContext;
    pContext->m_eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
    return pContext;
}

void ImplResetGb18030ToUnicodeContext(void * pContext)
{
    if (pContext)
        static_cast< ImplGb18030ToUnicodeContext * >(pContext)->m_eState
            = IMPL_GB_18030_TO_UNICODE_STATE_0;
}

void ImplDestroyGb18030ToUnicodeContext(void * pContext)
{
    delete static_cast< ImplGb18030ToUnicodeContext * >(pContext);
}

sal_Size ImplConvertGb18030ToUnicode(void const * pData,
                                     void * pContext,
                                     char const * pSrcBuf,
                                     sal_Size nSrcBytes,
                                     sal_Unicode * pDestBuf,
                                     sal_Size nDestChars,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtBytes)
{
    sal_Unicode const * pGb18030Data
        = static_cast< ImplGb18030ConverterData const * >(pData)->m_pGb18030ToUnicodeData;
    ImplGb180302000ToUnicodeRange const * pGb18030Ranges
        = static_cast< ImplGb18030ConverterData const * >(pData)->
              m_pGb18030ToUnicodeRanges;
    ImplGb18030ToUnicodeState eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
    sal_uInt32 nCode = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = static_cast< ImplGb18030ToUnicodeContext * >(pContext)->m_eState;
        nCode = static_cast< ImplGb18030ToUnicodeContext * >(pContext)->m_nCode;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        switch (eState)
        {
        case IMPL_GB_18030_TO_UNICODE_STATE_0:
            if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nChar;
                else
                    goto no_output;
            else if (nChar == 0x80)
                goto bad_input;
            else if (nChar <= 0xFE)
            {
                nCode = nChar - 0x81;
                eState = IMPL_GB_18030_TO_UNICODE_STATE_1;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_GB_18030_TO_UNICODE_STATE_1:
            if (nChar >= 0x30 && nChar <= 0x39)
            {
                nCode = nCode * 10 + (nChar - 0x30);
                eState = IMPL_GB_18030_TO_UNICODE_STATE_2;
            }
            else if ((nChar >= 0x40 && nChar <= 0x7E)
                     || (nChar >= 0x80 && nChar <= 0xFE))
            {
                nCode = nCode * 190 + (nChar <= 0x7E ? nChar - 0x40 :
                                                       nChar - 0x80 + 63);
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = pGb18030Data[nCode];
                else
                    goto no_output;
                eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_GB_18030_TO_UNICODE_STATE_2:
            if (nChar >= 0x81 && nChar <= 0xFE)
            {
                nCode = nCode * 126 + (nChar - 0x81);
                eState = IMPL_GB_18030_TO_UNICODE_STATE_3;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_GB_18030_TO_UNICODE_STATE_3:
            if (nChar >= 0x30 && nChar <= 0x39)
            {
                nCode = nCode * 10 + (nChar - 0x30);

                // 90 30 81 30 to E3 32 9A 35 maps to U+10000 to U+10FFFF:
                if (nCode >= 189000 && nCode <= 1237575)
                    if (pDestBufEnd - pDestBufPtr >= 2)
                    {
                        nCode -= 189000 - 0x10000;
                        *pDestBufPtr++
                            = (sal_Unicode) ImplGetHighSurrogate(nCode);
                        *pDestBufPtr++
                            = (sal_Unicode) ImplGetLowSurrogate(nCode);
                    }
                    else
                        goto no_output;
                else
                {
                    ImplGb180302000ToUnicodeRange const * pRange
                        = pGb18030Ranges;
                    sal_uInt32 nFirstNonRange = 0;
                    for (;;)
                    {
                        if (pRange->m_nNonRangeDataIndex == -1)
                            goto bad_input;
                        else if (nCode < pRange->m_nFirstLinear)
                        {
                            if (pDestBufPtr != pDestBufEnd)
                                *pDestBufPtr++
                                    = pGb18030Data[
                                          pRange->m_nNonRangeDataIndex
                                              + (nCode - nFirstNonRange)];
                            else
                                goto no_output;
                            break;
                        }
                        else if (nCode < pRange->m_nPastLinear)
                        {
                            if (pDestBufPtr != pDestBufEnd)
                                *pDestBufPtr++
                                    = (sal_Unicode)
                                          (pRange->m_nFirstUnicode
                                               + (nCode
                                                      - pRange->
                                                            m_nFirstLinear));
                            else
                                goto no_output;
                            break;
                        }
                        nFirstNonRange = (pRange++)->m_nPastLinear;
                    }
                }
                eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;
        }
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                    bUndefined, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;

    no_output:
        --pSrcBuf;
        nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        break;
    }

    if (eState != IMPL_GB_18030_TO_UNICODE_STATE_0
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
                eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        static_cast< ImplGb18030ToUnicodeContext * >(pContext)->m_eState = eState;
        static_cast< ImplGb18030ToUnicodeContext * >(pContext)->m_nCode = nCode;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToGb18030(void const * pData,
                                     void * pContext,
                                     sal_Unicode const * pSrcBuf,
                                     sal_Size nSrcChars,
                                     char * pDestBuf,
                                     sal_Size nDestBytes,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtChars)
{
    sal_uInt32 const * pGb18030Data
        = static_cast< ImplGb18030ConverterData const * >(pData)->
              m_pUnicodeToGb18030Data;
    ImplUnicodeToGb180302000Range const * pGb18030Ranges
        = static_cast< ImplGb18030ConverterData const * >(pData)->
              m_pUnicodeToGb18030Ranges;
    sal_Unicode nHighSurrogate = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    char * pDestBufPtr = pDestBuf;
    char * pDestBufEnd = pDestBuf + nDestBytes;

    if (pContext)
        nHighSurrogate
            = static_cast<ImplUnicodeToTextContext *>(pContext)->m_nHighSurrogate;

    for (; nConverted < nSrcChars; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *pSrcBuf++;
        if (nHighSurrogate == 0)
        {
            if (ImplIsHighSurrogate(nChar))
            {
                nHighSurrogate = (sal_Unicode) nChar;
                continue;
            }
        }
        else if (ImplIsLowSurrogate(nChar))
            nChar = ImplCombineSurrogates(nHighSurrogate, nChar);
        else
        {
            bUndefined = false;
            goto bad_input;
        }

        if (ImplIsLowSurrogate(nChar) || ImplIsNoncharacter(nChar))
        {
            bUndefined = false;
            goto bad_input;
        }

        if (nChar < 0x80)
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = static_cast< char >(nChar);
            else
                goto no_output;
        else if (nChar < 0x10000)
        {
            ImplUnicodeToGb180302000Range const * pRange = pGb18030Ranges;
            sal_Unicode nFirstNonRange = 0x80;
            for (;;)
            {
                if (nChar < pRange->m_nFirstUnicode)
                {
                    sal_uInt32 nCode
                        = pGb18030Data[pRange->m_nNonRangeDataIndex
                                           + (nChar - nFirstNonRange)];
                    if (pDestBufEnd - pDestBufPtr
                            >= (nCode <= 0xFFFF ? 2 : 4))
                    {
                        if (nCode > 0xFFFF)
                        {
                            *pDestBufPtr++ = static_cast< char >(nCode >> 24);
                            *pDestBufPtr++ = static_cast< char >(nCode >> 16 & 0xFF);
                        }
                        *pDestBufPtr++ = static_cast< char >(nCode >> 8 & 0xFF);
                        *pDestBufPtr++ = static_cast< char >(nCode & 0xFF);
                    }
                    else
                        goto no_output;
                    break;
                }
                else if (nChar <= pRange->m_nLastUnicode)
                {
                    if (pDestBufEnd - pDestBufPtr >= 4)
                    {
                        sal_uInt32 nCode
                            = pRange->m_nFirstLinear
                                  + (nChar - pRange->m_nFirstUnicode);
                        *pDestBufPtr++ = static_cast< char >(nCode / 12600 + 0x81);
                        *pDestBufPtr++
                            = static_cast< char >(nCode / 1260 % 10 + 0x30);
                        *pDestBufPtr++ = static_cast< char >(nCode / 10 % 126 + 0x81);
                        *pDestBufPtr++ = static_cast< char >(nCode % 10 + 0x30);
                    }
                    else
                        goto no_output;
                    break;
                }
                nFirstNonRange
                    = (sal_Unicode) ((pRange++)->m_nLastUnicode + 1);
            }
        }
        else
            if (pDestBufEnd - pDestBufPtr >= 4)
            {
                sal_uInt32 nCode = nChar - 0x10000;
                *pDestBufPtr++ = static_cast< char >(nCode / 12600 + 0x90);
                *pDestBufPtr++ = static_cast< char >(nCode / 1260 % 10 + 0x30);
                *pDestBufPtr++ = static_cast< char >(nCode / 10 % 126 + 0x81);
                *pDestBufPtr++ = static_cast< char >(nCode % 10 + 0x30);
            }
            else
                goto no_output;
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    bUndefined, nChar, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo, nullptr, 0, nullptr))
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
        --pSrcBuf;
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

    if (pContext)
        static_cast<ImplUnicodeToTextContext *>(pContext)->m_nHighSurrogate
            = nHighSurrogate;
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
