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
#include "convertiso2022kr.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

enum ImplIso2022KrToUnicodeState // order is important:
{
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001_2,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN
};

struct ImplIso2022KrToUnicodeContext
{
    ImplIso2022KrToUnicodeState m_eState;
    sal_uInt32 m_nRow;
};

enum ImplUnicodeToIso2022KrSet
{
    IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE,
    IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII,
    IMPL_UNICODE_TO_ISO_2022_KR_SET_1001
};

struct ImplUnicodeToIso2022KrContext
{
    sal_Unicode m_nHighSurrogate;
    ImplUnicodeToIso2022KrSet m_eSet;
};

}

void * ImplCreateIso2022KrToUnicodeContext()
{
    ImplIso2022KrToUnicodeContext * pContext =
        new ImplIso2022KrToUnicodeContext;
    pContext->m_eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
    return pContext;
}

void ImplResetIso2022KrToUnicodeContext(void * pContext)
{
    if (pContext)
        static_cast< ImplIso2022KrToUnicodeContext * >(pContext)->m_eState
            = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
}

void ImplDestroyIso2022KrToUnicodeContext(void * pContext)
{
    delete static_cast< ImplIso2022KrToUnicodeContext * >(pContext);
}

sal_Size ImplConvertIso2022KrToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    ImplDBCSToUniLeadTab const * pKsX1001Data
        = static_cast< ImplIso2022KrConverterData const * >(pData)->
              m_pKsX1001ToUnicodeData;
    ImplIso2022KrToUnicodeState eState
        = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
    sal_uInt32 nRow = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = static_cast< ImplIso2022KrToUnicodeContext * >(pContext)->m_eState;
        nRow = static_cast< ImplIso2022KrToUnicodeContext * >(pContext)->m_nRow;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        switch (eState)
        {
        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII:
            if (nChar == 0x0E) // SO
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001;
            else if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC;
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

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001:
            if (nChar == 0x0F) // SI
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
            else if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar + 0x80;
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001_2;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001_2:
            if (nChar >= 0x21 && nChar <= 0x7E)
            {
                sal_uInt16 nUnicode = 0;
                sal_uInt32 nFirst = pKsX1001Data[nRow].mnTrailStart;
                nChar += 0x80;
                if (nChar >= nFirst && nChar <= pKsX1001Data[nRow].mnTrailEnd)
                    nUnicode = pKsX1001Data[nRow].
                                   mpToUniTrailTab[nChar - nFirst];
                if (nUnicode != 0)
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = static_cast<sal_Unicode>(nUnicode);
                        eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001;
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

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC:
            if (nChar == 0x24) // $
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR:
            if (nChar == 0x29) // )
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN:
            if (nChar == 0x43) // C
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
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
            eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
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

    if (eState > IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001
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
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                break;
            }
    }

    if (pContext)
    {
        static_cast< ImplIso2022KrToUnicodeContext * >(pContext)->m_eState = eState;
        static_cast< ImplIso2022KrToUnicodeContext * >(pContext)->m_nRow = nRow;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToIso2022KrContext()
{
    ImplUnicodeToIso2022KrContext * pContext =
        new ImplUnicodeToIso2022KrContext;
    pContext->m_nHighSurrogate = 0;
    pContext->m_eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    return pContext;
}

void ImplResetUnicodeToIso2022KrContext(void * pContext)
{
    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_nHighSurrogate = 0;
        static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_eSet
            = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    }
}

void ImplDestroyUnicodeToIso2022KrContext(void * pContext)
{
    delete static_cast< ImplUnicodeToIso2022KrContext * >(pContext);
}

sal_Size ImplConvertUnicodeToIso2022Kr(void const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    ImplUniToDBCSHighTab const * pKsX1001Data
        = static_cast< ImplIso2022KrConverterData const * >(pData)->
              m_pUnicodeToKsX1001Data;
    sal_Unicode nHighSurrogate = 0;
    ImplUnicodeToIso2022KrSet eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    char * pDestBufPtr = pDestBuf;
    char * pDestBufEnd = pDestBuf + nDestBytes;
    bool bWritten;

    if (pContext)
    {
        nHighSurrogate
            = static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_nHighSurrogate;
        eSet = static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_eSet;
    }

    if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE)
    {
        if (pDestBufEnd - pDestBufPtr >= 4)
        {
            *pDestBufPtr++ = 0x1B; // ESC
            *pDestBufPtr++ = 0x24; // $
            *pDestBufPtr++ = 0x29; // )
            *pDestBufPtr++ = 0x43; // C
            eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
        }
        else
            nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
    }

    if ((nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL) == 0)
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
                if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_1001)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0F; // SI
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = static_cast< char >(nChar);
                else
                    goto no_output;
            }
            else if (nChar == 0x0E || nChar == 0x0F || nChar == 0x1B)
                goto bad_input;
            else if (nChar < 0x80)
            {
                if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_1001)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0F; // SI
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
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
                    sal_uInt32 nFirst = pKsX1001Data[nIndex1].mnLowStart;
                    if (nIndex2 >= nFirst
                        && nIndex2 <= pKsX1001Data[nIndex1].mnLowEnd)
                        nBytes = pKsX1001Data[nIndex1].
                                     mpToUniTrailTab[nIndex2 - nFirst];
                }
                if (nBytes != 0)
                {
                    if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII)
                    {
                        if (pDestBufPtr != pDestBufEnd)
                        {
                            *pDestBufPtr++ = 0x0E; // SO
                            eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_1001;
                        }
                        else
                            goto no_output;
                    }
                    if (pDestBufEnd - pDestBufPtr >= 2)
                    {
                        *pDestBufPtr++ = static_cast< char >((nBytes >> 8) & 0x7F);
                        *pDestBufPtr++ = static_cast< char >(nBytes & 0x7F);
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
                        &nInfo, "\x0F" /* SI */,
                        eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII ? 0 : 1,
                        &bWritten))
            {
            case sal::detail::textenc::BAD_INPUT_STOP:
                nHighSurrogate = 0;
                break;

            case sal::detail::textenc::BAD_INPUT_CONTINUE:
                if (bWritten)
                    eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
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
                            "\x0F" /* SI */,
                            (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII
                             ? 0 : 1),
                            &bWritten))
                {
                case sal::detail::textenc::BAD_INPUT_STOP:
                    nHighSurrogate = 0;
                    bFlush = false;
                    break;

                case sal::detail::textenc::BAD_INPUT_CONTINUE:
                    if (bWritten)
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
                    nHighSurrogate = 0;
                    break;

                case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                    nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
        }
        if (bFlush
            && eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_1001
            && (nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
        {
            if (pDestBufPtr != pDestBufEnd)
            {
                *pDestBufPtr++ = 0x0F; // SI
                eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
            }
            else
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        }
    }

    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_nHighSurrogate
            = nHighSurrogate;
        static_cast< ImplUnicodeToIso2022KrContext * >(pContext)->m_eSet = eSet;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
