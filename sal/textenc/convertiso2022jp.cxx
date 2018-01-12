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
#include <sal/types.h>

#include "converter.hxx"
#include "convertiso2022jp.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

enum ImplIso2022JpToUnicodeState // order is important:
{
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_JIS_ROMAN,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208_2,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_LPAREN,
    IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_DOLLAR
};

struct ImplIso2022JpToUnicodeContext
{
    ImplIso2022JpToUnicodeState m_eState;
    sal_uInt32 m_nRow;
};

struct ImplUnicodeToIso2022JpContext
{
    sal_Unicode m_nHighSurrogate;
    bool m_b0208;
};

}

void * ImplCreateIso2022JpToUnicodeContext()
{
    ImplIso2022JpToUnicodeContext * pContext =
        new ImplIso2022JpToUnicodeContext;
    pContext->m_eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
    return pContext;
}

void ImplResetIso2022JpToUnicodeContext(void * pContext)
{
    if (pContext)
        static_cast< ImplIso2022JpToUnicodeContext * >(pContext)->m_eState
            = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
}

void ImplDestroyIso2022JpToUnicodeContext(void * pContext)
{
    delete static_cast< ImplIso2022JpToUnicodeContext * >(pContext);
}

sal_Size ImplConvertIso2022JpToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    ImplDBCSToUniLeadTab const * pJisX0208Data
        = static_cast< ImplIso2022JpConverterData const * >(pData)->
              m_pJisX0208ToUnicodeData;
    ImplIso2022JpToUnicodeState eState
        = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
    sal_uInt32 nRow = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = static_cast< ImplIso2022JpToUnicodeContext * >(pContext)->m_eState;
        nRow = static_cast< ImplIso2022JpToUnicodeContext * >(pContext)->m_nRow;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        switch (eState)
        {
        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII:
            if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC;
            else if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = static_cast<sal_Unicode>(nChar);
                else
                    goto no_output;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_JIS_ROMAN:
            if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC;
            else if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                {
                    switch (nChar)
                    {
                    case 0x5C: // REVERSE SOLIDUS (\)
                        nChar = 0xA5; // YEN SIGN
                        break;

                    case 0x7E: // ~
                        nChar = 0xAF; // MACRON
                        break;
                    }
                    *pDestBufPtr++ = static_cast<sal_Unicode>(nChar);
                }
                else
                    goto no_output;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208:
            if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC;
            else if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar;
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208_2;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208_2:
            if (nChar >= 0x21 && nChar <= 0x7E)
            {
                sal_uInt16 nUnicode = 0;
                sal_uInt32 nFirst = pJisX0208Data[nRow].mnTrailStart;
                if (nChar >= nFirst
                    && nChar <= pJisX0208Data[nRow].mnTrailEnd)
                    nUnicode = pJisX0208Data[nRow].
                                   mpToUniTrailTab[nChar - nFirst];
                if (nUnicode != 0)
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = static_cast<sal_Unicode>(nUnicode);
                        eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208;
                    }
                    else
                        goto no_output;
                else
                    goto bad_input;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC:
            switch (nChar)
            {
            case 0x24: // $
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_DOLLAR;
                break;

            case 0x28: // (
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_LPAREN;
                break;

            default:
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_LPAREN:
            switch (nChar)
            {
            case 0x42: // A
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
                break;

            case 0x4A: // J
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_JIS_ROMAN;
                break;

            default:
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_JP_TO_UNICODE_STATE_ESC_DOLLAR:
            switch (nChar)
            {
            case 0x40: // @
            case 0x42: // B
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208;
                break;

            default:
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
            eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
            continue;

        case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;

    no_output:
        --pSrcBuf;
        nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
        break;
    }

    if (eState > IMPL_ISO_2022_JP_TO_UNICODE_STATE_0208
        && (nInfo & (RTL_TEXTTOUNICODE_INFO_ERROR
                         | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL))
               == 0)
    {
        if ((nFlags & RTL_TEXTTOUNICODE_FLAGS_FLUSH) == 0)
            nInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL;
        else
            switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                        false, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                        &nInfo))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                eState = IMPL_ISO_2022_JP_TO_UNICODE_STATE_ASCII;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                break;
            }
    }

    if (pContext)
    {
        static_cast< ImplIso2022JpToUnicodeContext * >(pContext)->m_eState = eState;
        static_cast< ImplIso2022JpToUnicodeContext * >(pContext)->m_nRow = nRow;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToIso2022JpContext()
{
    ImplUnicodeToIso2022JpContext * pContext =
        new ImplUnicodeToIso2022JpContext;
    pContext->m_nHighSurrogate = 0;
    pContext->m_b0208 = false;
    return pContext;
}

void ImplResetUnicodeToIso2022JpContext(void * pContext)
{
    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_nHighSurrogate = 0;
        static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_b0208 = false;
    }
}

void ImplDestroyUnicodeToIso2022JpContext(void * pContext)
{
    delete static_cast< ImplUnicodeToIso2022JpContext * >(pContext);
}

sal_Size ImplConvertUnicodeToIso2022Jp(void const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    ImplUniToDBCSHighTab const * pJisX0208Data
        = static_cast< ImplIso2022JpConverterData const * >(pData)->
              m_pUnicodeToJisX0208Data;
    sal_Unicode nHighSurrogate = 0;
    bool b0208 = false;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    char * pDestBufPtr = pDestBuf;
    char * pDestBufEnd = pDestBuf + nDestBytes;
    bool bWritten;

    if (pContext)
    {
        nHighSurrogate
            = static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_nHighSurrogate;
        b0208 = static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_b0208;
    }

    for (; nConverted < nSrcChars; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *pSrcBuf++;
        if (nHighSurrogate == 0)
        {
            if (ImplIsHighSurrogate(nChar))
            {
                nHighSurrogate = static_cast<sal_Unicode>(nChar);
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

        if (nChar == 0x0A || nChar == 0x0D) // LF, CR
        {
            if (b0208)
            {
                if (pDestBufEnd - pDestBufPtr >= 3)
                {
                    *pDestBufPtr++ = 0x1B; // ESC
                    *pDestBufPtr++ = 0x28; // (
                    *pDestBufPtr++ = 0x42; // B
                    b0208 = false;
                }
                else
                    goto no_output;
            }
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = static_cast< char >(nChar);
            else
                goto no_output;
        }
        else if (nChar == 0x1B)
            goto bad_input;
        else if (nChar < 0x80)
        {
            if (b0208)
            {
                if (pDestBufEnd - pDestBufPtr >= 3)
                {
                    *pDestBufPtr++ = 0x1B; // ESC
                    *pDestBufPtr++ = 0x28; // (
                    *pDestBufPtr++ = 0x42; // B
                    b0208 = false;
                }
                else
                    goto no_output;
            }
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = static_cast< char >(nChar);
            else
                goto no_output;
        }
        else
        {
            sal_uInt16 nBytes = 0;
            sal_uInt32 nIndex1 = nChar >> 8;
            if (nIndex1 < 0x100)
            {
                sal_uInt32 nIndex2 = nChar & 0xFF;
                sal_uInt32 nFirst = pJisX0208Data[nIndex1].mnLowStart;
                if (nIndex2 >= nFirst
                    && nIndex2 <= pJisX0208Data[nIndex1].mnLowEnd)
                {
                    nBytes = pJisX0208Data[nIndex1].
                                 mpToUniTrailTab[nIndex2 - nFirst];
                    if (nBytes == 0)
                        // For some reason, the tables in tcvtjp4.tab do not
                        // include these two conversions:
                        switch (nChar)
                        {
                        case 0xA5: // YEN SIGN
                            nBytes = 0x216F;
                            break;

                        case 0xAF: // MACRON
                            nBytes = 0x2131;
                            break;
                        }
                }
            }
            if (nBytes != 0)
            {
                if (!b0208)
                {
                    if (pDestBufEnd - pDestBufPtr >= 3)
                    {
                        *pDestBufPtr++ = 0x1B; // ESC
                        *pDestBufPtr++ = 0x24; // $
                        *pDestBufPtr++ = 0x42; // B
                        b0208 = true;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufEnd - pDestBufPtr >= 2)
                {
                    *pDestBufPtr++ = static_cast< char >(nBytes >> 8);
                    *pDestBufPtr++ = static_cast< char >(nBytes & 0xFF);
                }
                else
                    goto no_output;
            }
            else
                goto bad_input;
        }
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    bUndefined, nChar, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo, "\x1B(B", b0208 ? 3 : 0, &bWritten))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            nHighSurrogate = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            if (bWritten)
                b0208 = false;
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

    if ((nInfo & (RTL_UNICODETOTEXT_INFO_ERROR
                      | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
            == 0)
    {
        bool bFlush = true;
        if (nHighSurrogate != 0)
        {
            if ((nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
                nInfo |= RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
            else
                switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                            false, 0, nFlags, &pDestBufPtr, pDestBufEnd, &nInfo,
                            "\x1B(B", b0208 ? 3 : 0, &bWritten))
                {
                case sal::detail::textenc::BAD_INPUT_STOP:
                    nHighSurrogate = 0;
                    bFlush = false;
                    break;

                case sal::detail::textenc::BAD_INPUT_CONTINUE:
                    if (bWritten)
                        b0208 = false;
                    nHighSurrogate = 0;
                    break;

                case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                    nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
        }
        if (bFlush
            && b0208
            && (nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
        {
            if (pDestBufEnd - pDestBufPtr >= 3)
            {
                *pDestBufPtr++ = 0x1B; // ESC
                *pDestBufPtr++ = 0x28; // (
                *pDestBufPtr++ = 0x42; // B
                b0208 = false;
            }
            else
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        }
    }

    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_nHighSurrogate
            = nHighSurrogate;
        static_cast< ImplUnicodeToIso2022JpContext * >(pContext)->m_b0208 = b0208;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
