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
#include "converteuctw.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

enum ImplEucTwToUnicodeState
{
    IMPL_EUC_TW_TO_UNICODE_STATE_0,
    IMPL_EUC_TW_TO_UNICODE_STATE_1,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_1,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_2,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_3
};

struct ImplEucTwToUnicodeContext
{
    ImplEucTwToUnicodeState m_eState;
    sal_Int32 m_nPlane; // 0--15
    sal_Int32 m_nRow; // 0--93
};

}

void * ImplCreateEucTwToUnicodeContext()
{
    ImplEucTwToUnicodeContext * pContext = new ImplEucTwToUnicodeContext;
    pContext->m_eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
    return pContext;
}

void ImplResetEucTwToUnicodeContext(void * pContext)
{
    if (pContext)
        static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_eState
            = IMPL_EUC_TW_TO_UNICODE_STATE_0;
}

void ImplDestroyEucTwToUnicodeContext(void * pContext)
{
    delete static_cast< ImplEucTwToUnicodeContext * >(pContext);
}

sal_Size ImplConvertEucTwToUnicode(void const * pData,
                                   void * pContext,
                                   char const * pSrcBuf,
                                   sal_Size nSrcBytes,
                                   sal_Unicode * pDestBuf,
                                   sal_Size nDestChars,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtBytes)
{
    sal_uInt16 const * pCns116431992Data
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pCns116431992ToUnicodeData;
    sal_Int32 const * pCns116431992RowOffsets
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pCns116431992ToUnicodePlaneOffsets;
    ImplEucTwToUnicodeState eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
    sal_Int32 nPlane = 0;
    sal_Int32 nRow = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_eState;
        nPlane = static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_nPlane;
        nRow = static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_nRow;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        switch (eState)
        {
        case IMPL_EUC_TW_TO_UNICODE_STATE_0:
            if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nChar;
                else
                    goto no_output;
            else if (nChar >= 0xA1 && nChar <= 0xFE)
            {
                nRow = nChar - 0xA1;
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_1;
            }
            else if (nChar == 0x8E)
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_2_1;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_1:
            if (nChar >= 0xA1 && nChar <= 0xFE)
            {
                nPlane = 0;
                goto transform;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_1:
            if (nChar >= 0xA1 && nChar <= 0xB0)
            {
                nPlane = nChar - 0xA1;
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_2_2;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_2:
            if (nChar >= 0xA1 && nChar <= 0xFE)
            {
                nRow = nChar - 0xA1;
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_2_3;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_3:
            if (nChar >= 0xA1 && nChar <= 0xFE)
                goto transform;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;
        }
        continue;

    transform:
        {
            sal_Int32 nPlaneOffset = pCns116431992PlaneOffsets[nPlane];
            if (nPlaneOffset == -1)
                goto bad_input;
            else
            {
                sal_Int32 nOffset
                    = pCns116431992RowOffsets[nPlaneOffset + nRow];
                if (nOffset == -1)
                    goto bad_input;
                else
                {
                    sal_uInt32 nFirstLast = pCns116431992Data[nOffset++];
                    sal_uInt32 nFirst = nFirstLast & 0xFF;
                    sal_uInt32 nLast = nFirstLast >> 8;
                    nChar -= 0xA0;
                    if (nChar >= nFirst && nChar <= nLast)
                    {
                        sal_uInt32 nUnicode
                            = pCns116431992Data[nOffset + (nChar - nFirst)];
                        if (nUnicode == 0xFFFF)
                            goto bad_input;
                        else if (ImplIsHighSurrogate(nUnicode))
                            if (pDestBufEnd - pDestBufPtr >= 2)
                            {
                                nOffset += nLast - nFirst + 1;
                                nFirst = pCns116431992Data[nOffset++];
                                *pDestBufPtr++ = (sal_Unicode) nUnicode;
                                *pDestBufPtr++
                                    = (sal_Unicode)
                                          pCns116431992Data[
                                              nOffset + (nChar - nFirst)];
                            }
                            else
                                goto no_output;
                        else
                            if (pDestBufPtr != pDestBufEnd)
                                *pDestBufPtr++ = (sal_Unicode) nUnicode;
                            else
                                goto no_output;
                    }
                    else
                        goto bad_input;
                    eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
                }
            }
            continue;
        }

    bad_input:
        switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                    bUndefined, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
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

    if (eState != IMPL_EUC_TW_TO_UNICODE_STATE_0
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
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_eState = eState;
        static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_nPlane = nPlane;
        static_cast< ImplEucTwToUnicodeContext * >(pContext)->m_nRow = nRow;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToEucTw(void const * pData,
                                   void * pContext,
                                   sal_Unicode const * pSrcBuf,
                                   sal_Size nSrcChars,
                                   char * pDestBuf,
                                   sal_Size nDestBytes,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtChars)
{
    sal_uInt8 const * pCns116431992Data
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pUnicodeToCns116431992Data;
    sal_Int32 const * pCns116431992PageOffsets
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = static_cast< ImplEucTwConverterData const * >(pData)->
              m_pUnicodeToCns116431992PlaneOffsets;
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
        else
        {
            sal_Int32 nOffset = pCns116431992PlaneOffsets[nChar >> 16];
            sal_uInt32 nFirst;
            sal_uInt32 nLast;
            sal_uInt32 nPlane;
            if (nOffset == -1)
                goto bad_input;
            nOffset
                = pCns116431992PageOffsets[nOffset + ((nChar & 0xFF00) >> 8)];
            if (nOffset == -1)
                goto bad_input;
            nFirst = pCns116431992Data[nOffset++];
            nLast = pCns116431992Data[nOffset++];
            nChar &= 0xFF;
            if (nChar < nFirst || nChar > nLast)
                goto bad_input;
            nOffset += 3 * (nChar - nFirst);
            nPlane = pCns116431992Data[nOffset++];
            if (nPlane == 0)
                goto bad_input;
            if (pDestBufEnd - pDestBufPtr < (nPlane == 1 ? 2 : 4))
                goto no_output;
            if (nPlane != 1)
            {
                *pDestBufPtr++ = static_cast< char >(static_cast< unsigned char >(0x8E));
                *pDestBufPtr++ = static_cast< char >(0xA0 + nPlane);
            }
            *pDestBufPtr++ = static_cast< char >(0xA0 + pCns116431992Data[nOffset++]);
            *pDestBufPtr++ = static_cast< char >(0xA0 + pCns116431992Data[nOffset]);
        }
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
