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
#include "convertiso2022cn.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

enum ImplIso2022CnToUnicodeState // order is important:
{
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO_2,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432_2,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN,
    IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_ASTERISK
};

struct ImplIso2022CnToUnicodeContext
{
    ImplIso2022CnToUnicodeState m_eState;
    sal_uInt32 m_nRow;
    bool m_bSo;
    bool m_b116431;
};

enum ImplUnicodeToIso2022CnDesignator
{
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE,
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312,
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_116431
};

struct ImplUnicodeToIso2022CnContext
{
    sal_Unicode m_nHighSurrogate;
    ImplUnicodeToIso2022CnDesignator m_eSoDesignator;
    bool m_b116432Designator;
    bool m_bSo;
};

}

void * ImplCreateIso2022CnToUnicodeContext()
{
    ImplIso2022CnToUnicodeContext * pContext =
        new ImplIso2022CnToUnicodeContext;
    pContext->m_eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
    pContext->m_bSo = false;
    pContext->m_b116431 = false;
    return pContext;
}

void ImplResetIso2022CnToUnicodeContext(void * pContext)
{
    if (pContext)
    {
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_eState
            = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_bSo = false;
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_b116431 = false;
    }
}

void ImplDestroyIso2022CnToUnicodeContext(void * pContext)
{
    delete static_cast< ImplIso2022CnToUnicodeContext * >(pContext);
}

sal_Size ImplConvertIso2022CnToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    ImplDBCSToUniLeadTab const * pGb2312Data
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pGb2312ToUnicodeData;
    sal_uInt16 const * pCns116431992Data
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pCns116431992ToUnicodeData;
    sal_Int32 const * pCns116431992RowOffsets
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pCns116431992ToUnicodePlaneOffsets;
    ImplIso2022CnToUnicodeState eState
        = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
    sal_uInt32 nRow = 0;
    bool bSo = false;
    bool b116431 = false;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_eState;
        nRow = static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_nRow;
        bSo = static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_bSo;
        b116431 = static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_b116431;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        sal_uInt32 nPlane;
        switch (eState)
        {
        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII:
            if (nChar == 0x0E) // SO
            {
                bSo = true;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO;
            }
            else if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC;
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

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO:
            if (nChar == 0x0F) // SI
            {
                bSo = false;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else if (nChar == 0x1B) // ESC
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC;
            else if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO_2;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO_2:
            if (nChar >= 0x21 && nChar <= 0x7E)
                if (b116431)
                {
                    nPlane = 0;
                    goto transform;
                }
                else
                {
                    sal_uInt16 nUnicode = 0;
                    sal_uInt32 nFirst;
                    nRow += 0x80;
                    nChar += 0x80;
                    nFirst = pGb2312Data[nRow].mnTrailStart;
                    if (nChar >= nFirst
                        && nChar <= pGb2312Data[nRow].mnTrailEnd)
                        nUnicode = pGb2312Data[nRow].
                                       mpToUniTrailTab[nChar - nFirst];
                    if (nUnicode != 0)
                        if (pDestBufPtr != pDestBufEnd)
                        {
                            *pDestBufPtr++ = static_cast<sal_Unicode>(nUnicode);
                            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO;
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

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432:
            if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432_2;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432_2:
            if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nPlane = 1;
                goto transform;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC:
            if (nChar == 0x24) // $
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR;
            else if (nChar == 0x4E) // N
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR:
            if (nChar == 0x29) // )
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN;
            else if (nChar == 0x2A) // *
                eState
                    = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_ASTERISK;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN:
            if (nChar == 0x41) // A
            {
                b116431 = false;
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else if (nChar == 0x47) // G
            {
                b116431 = true;
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_ASTERISK:
            if (nChar == 0x48) // H
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
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
                    = pCns116431992RowOffsets[nPlaneOffset + (nRow - 0x21)];
                if (nOffset == -1)
                    goto bad_input;
                else
                {
                    sal_uInt32 nFirstLast = pCns116431992Data[nOffset++];
                    sal_uInt32 nFirst = nFirstLast & 0xFF;
                    sal_uInt32 nLast = nFirstLast >> 8;
                    nChar -= 0x20;
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
                                *pDestBufPtr++ = static_cast<sal_Unicode>(nUnicode);
                                *pDestBufPtr++
                                    = static_cast<sal_Unicode>(pCns116431992Data[
                                              nOffset + (nChar - nFirst)]);
                            }
                            else
                                goto no_output;
                        else
                            if (pDestBufPtr != pDestBufEnd)
                                *pDestBufPtr++ = static_cast<sal_Unicode>(nUnicode);
                            else
                                goto no_output;
                    }
                    else
                        goto bad_input;
                    eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                                   IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
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
            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            b116431 = false;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            b116431 = false;
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

    if (eState > IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO
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
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
                b116431 = false;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL;
                break;
            }
    }

    if (pContext)
    {
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_eState = eState;
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_nRow = nRow;
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_bSo = bSo;
        static_cast< ImplIso2022CnToUnicodeContext * >(pContext)->m_b116431 = b116431;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToIso2022CnContext()
{
    ImplUnicodeToIso2022CnContext * pContext =
        new ImplUnicodeToIso2022CnContext;
    pContext->m_nHighSurrogate = 0;
    pContext->m_eSoDesignator = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
    pContext->m_b116432Designator = false;
    pContext->m_bSo = false;
    return pContext;
}

void ImplResetUnicodeToIso2022CnContext(void * pContext)
{
    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_nHighSurrogate = 0;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_eSoDesignator
            = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_b116432Designator
            = false;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_bSo = false;
    }
}

void ImplDestroyUnicodeToIso2022CnContext(void * pContext)
{
    delete static_cast< ImplUnicodeToIso2022CnContext * >(pContext);
}

static sal_uInt32 ImplIso2022CnTranslateTo2312(ImplUniToDBCSHighTab const *
                                                   pGb2312Data,
                                               sal_uInt32 nChar)
{
    sal_uInt32 nIndex1 = nChar >> 8;
    if (nIndex1 < 0x100)
    {
        sal_uInt32 nIndex2 = nChar & 0xFF;
        sal_uInt32 nFirst = pGb2312Data[nIndex1].mnLowStart;
        if (nIndex2 >= nFirst && nIndex2 <= pGb2312Data[nIndex1].mnLowEnd)
            return pGb2312Data[nIndex1].mpToUniTrailTab[nIndex2 - nFirst]
                       & 0x7F7F;
    }
    return 0;
}

static sal_uInt32
ImplIso2022CnTranslateTo116431(sal_uInt8 const * pCns116431992Data,
                               sal_Int32 const * pCns116431992PageOffsets,
                               sal_Int32 const * pCns116431992PlaneOffsets,
                               sal_uInt32 nChar)
{
    sal_Int32 nOffset = pCns116431992PlaneOffsets[nChar >> 16];
    sal_uInt32 nFirst;
    sal_uInt32 nLast;
    sal_uInt32 nPlane;
    if (nOffset == -1)
        return 0;
    nOffset = pCns116431992PageOffsets[nOffset + ((nChar & 0xFF00) >> 8)];
    if (nOffset == -1)
        return 0;
    nFirst = pCns116431992Data[nOffset++];
    nLast = pCns116431992Data[nOffset++];
    nChar &= 0xFF;
    if (nChar < nFirst || nChar > nLast)
        return 0;
    nOffset += 3 * (nChar - nFirst);
    nPlane = pCns116431992Data[nOffset++];
    if (nPlane != 1)
        return 0;
    return (0x20 + pCns116431992Data[nOffset]) << 8
               | (0x20 + pCns116431992Data[nOffset + 1]);
}

sal_Size ImplConvertUnicodeToIso2022Cn(void const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    ImplUniToDBCSHighTab const * pGb2312Data
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pUnicodeToGb2312Data;
    sal_uInt8 const * pCns116431992Data
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pUnicodeToCns116431992Data;
    sal_Int32 const * pCns116431992PageOffsets
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = static_cast< ImplIso2022CnConverterData const * >(pData)->
              m_pUnicodeToCns116431992PlaneOffsets;
    sal_Unicode nHighSurrogate = 0;
    ImplUnicodeToIso2022CnDesignator eSoDesignator
        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
    bool b116432Designator = false;
    bool bSo = false;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    char * pDestBufPtr = pDestBuf;
    char * pDestBufEnd = pDestBuf + nDestBytes;
    bool bWritten;

    if (pContext)
    {
        nHighSurrogate
            = static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_nHighSurrogate;
        eSoDesignator
            = static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_eSoDesignator;
        b116432Designator = static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->
                                m_b116432Designator;
        bSo = static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_bSo;
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
            if (bSo)
            {
                if (pDestBufPtr != pDestBufEnd)
                {
                    *pDestBufPtr++ = 0x0F; // SI
                    bSo = false;
                    eSoDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
                    b116432Designator = false;
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
            if (bSo)
            {
                if (pDestBufPtr != pDestBufEnd)
                {
                    *pDestBufPtr++ = 0x0F; // SI
                    bSo = false;
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
            sal_uInt32 nBytes = 0;
            ImplUnicodeToIso2022CnDesignator eNewDesignator =
                         IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
            switch (eSoDesignator)
            {
            case IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE:
                nBytes = ImplIso2022CnTranslateTo2312(pGb2312Data, nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312;
                    break;
                }
                nBytes = ImplIso2022CnTranslateTo116431(
                             pCns116431992Data,
                             pCns116431992PageOffsets,
                             pCns116431992PlaneOffsets,
                             nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_116431;
                    break;
                }
                break;

            case IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312:
                nBytes = ImplIso2022CnTranslateTo2312(pGb2312Data, nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
                    break;
                }
                nBytes = ImplIso2022CnTranslateTo116431(
                             pCns116431992Data,
                             pCns116431992PageOffsets,
                             pCns116431992PlaneOffsets,
                             nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_116431;
                    break;
                }
                break;

            case IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_116431:
                nBytes = ImplIso2022CnTranslateTo116431(
                             pCns116431992Data,
                             pCns116431992PageOffsets,
                             pCns116431992PlaneOffsets,
                             nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
                    break;
                }
                nBytes = ImplIso2022CnTranslateTo2312(pGb2312Data, nChar);
                if (nBytes != 0)
                {
                    eNewDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312;
                    break;
                }
                break;
            }
            if (nBytes != 0)
            {
                if (eNewDesignator
                        != IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE)
                {
                    if (bSo)
                    {
                        if (pDestBufPtr != pDestBufEnd)
                        {
                            *pDestBufPtr++ = 0x0F; // SI
                            bSo = false;
                        }
                        else
                            goto no_output;
                    }
                    if (pDestBufEnd - pDestBufPtr >= 4)
                    {
                        *pDestBufPtr++ = 0x1B; // ESC
                        *pDestBufPtr++ = 0x24; // $
                        *pDestBufPtr++ = 0x29; // )
                        *pDestBufPtr++
                            = eNewDesignator
                              == IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312 ?
                                  0x41 : 0x47; // A, G
                        eSoDesignator = eNewDesignator;
                    }
                    else
                        goto no_output;
                }
                if (!bSo)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0E; // SO
                        bSo = true;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufEnd - pDestBufPtr >= 4)
                {
                    *pDestBufPtr++ = static_cast< char >(nBytes >> 8);
                    *pDestBufPtr++ = static_cast< char >(nBytes & 0xFF);
                }
                else
                    goto no_output;
            }
            else
            {
                sal_Int32 nOffset = pCns116431992PlaneOffsets[nChar >> 16];
                sal_uInt32 nFirst;
                sal_uInt32 nLast;
                sal_uInt32 nPlane;
                if (nOffset == -1)
                    goto bad_input;
                nOffset
                    = pCns116431992PageOffsets[nOffset
                                                   + ((nChar & 0xFF00) >> 8)];
                if (nOffset == -1)
                    goto bad_input;
                nFirst = pCns116431992Data[nOffset++];
                nLast = pCns116431992Data[nOffset++];
                nChar &= 0xFF;
                if (nChar < nFirst || nChar > nLast)
                    goto bad_input;
                nOffset += 3 * (nChar - nFirst);
                nPlane = pCns116431992Data[nOffset++];
                if (nPlane != 2)
                    goto bad_input;
                if (!b116432Designator)
                {
                    if (pDestBufEnd - pDestBufPtr >= 4)
                    {
                        *pDestBufPtr++ = 0x1B; // ESC
                        *pDestBufPtr++ = 0x24; // $
                        *pDestBufPtr++ = 0x2A; // *
                        *pDestBufPtr++ = 0x48; // H
                        b116432Designator = true;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufEnd - pDestBufPtr >= 4)
                {
                    *pDestBufPtr++ = 0x1B; // ESC
                    *pDestBufPtr++ = 0x4E; // N
                    *pDestBufPtr++
                        = static_cast< char >(0x20 + pCns116431992Data[nOffset++]);
                    *pDestBufPtr++
                        = static_cast< char >(0x20 + pCns116431992Data[nOffset]);
                }
                else
                    goto no_output;
            }
        }
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputUnicodeToTextConversion(
                    bUndefined, nChar, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo, "\x0F" /* SI */, bSo ? 1 : 0, &bWritten))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            nHighSurrogate = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            if (bWritten)
                bSo = false;
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
                            "\x0F" /* SI */, bSo ? 1 : 0, &bWritten))
                {
                case sal::detail::textenc::BAD_INPUT_STOP:
                    nHighSurrogate = 0;
                    bFlush = false;
                    break;

                case sal::detail::textenc::BAD_INPUT_CONTINUE:
                    if (bWritten)
                        bSo = false;
                    nHighSurrogate = 0;
                    break;

                case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                    nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
        }
        if (bFlush && bSo && (nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
        {
            if (pDestBufPtr != pDestBufEnd)
            {
                *pDestBufPtr++ = 0x0F; // SI
                bSo = false;
            }
            else
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        }
    }

    if (pContext)
    {
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_nHighSurrogate
            = nHighSurrogate;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_eSoDesignator
            = eSoDesignator;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_b116432Designator
            = b116432Designator;
        static_cast< ImplUnicodeToIso2022CnContext * >(pContext)->m_bSo = bSo;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
