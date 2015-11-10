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

#include <cassert>

#include "rtl/textcvt.h"
#include "sal/types.h"

#include "context.hxx"
#include "convertbig5hkscs.hxx"
#include "converter.hxx"
#include "tenchelp.hxx"
#include "unichars.hxx"

namespace {

struct ImplBig5HkscsToUnicodeContext
{
    sal_Int32 m_nRow; // 0--255; 0 means none
};

}

void * ImplCreateBig5HkscsToUnicodeContext()
{
    ImplBig5HkscsToUnicodeContext * pContext =
        new ImplBig5HkscsToUnicodeContext;
    pContext->m_nRow = 0;
    return pContext;
}

void ImplResetBig5HkscsToUnicodeContext(void * pContext)
{
    if (pContext)
        static_cast< ImplBig5HkscsToUnicodeContext * >(pContext)->m_nRow = 0;
}

void ImplDestroyBig5HkscsToUnicodeContext(void * pContext)
{
    delete static_cast< ImplBig5HkscsToUnicodeContext * >(pContext);
}

sal_Size ImplConvertBig5HkscsToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    sal_uInt16 const * pBig5Hkscs2001Data
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pBig5Hkscs2001ToUnicodeData;
    sal_Int32 const * pBig5Hkscs2001RowOffsets
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pBig5Hkscs2001ToUnicodeRowOffsets;
    ImplDBCSToUniLeadTab const * pBig5Data
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pBig5ToUnicodeData;
    sal_Int32 nRow = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
        nRow = static_cast< ImplBig5HkscsToUnicodeContext * >(pContext)->m_nRow;

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        bool bUndefined = true;
        sal_uInt32 nChar = *reinterpret_cast<unsigned char const *>(pSrcBuf++);
        if (nRow == 0)
            if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nChar;
                else
                    goto no_output;
            else if (nChar >= 0x81 && nChar <= 0xFE)
                nRow = nChar;
            else
            {
                bUndefined = false;
                goto bad_input;
            }
        else
            if ((nChar >= 0x40 && nChar <= 0x7E)
                || (nChar >= 0xA1 && nChar <= 0xFE))
            {
                sal_uInt32 nUnicode = 0xFFFF;
                sal_Int32 nOffset = pBig5Hkscs2001RowOffsets[nRow];
                sal_uInt32 nFirst=0;
                sal_uInt32 nLast=0;
                if (nOffset != -1)
                {
                    sal_uInt32 nFirstLast = pBig5Hkscs2001Data[nOffset++];
                    nFirst = nFirstLast & 0xFF;
                    nLast = nFirstLast >> 8;
                    if (nChar >= nFirst && nChar <= nLast)
                        nUnicode
                            = pBig5Hkscs2001Data[nOffset + (nChar - nFirst)];
                }
                if (nUnicode == 0xFFFF)
                {
                    sal_uInt32 n = pBig5Data[nRow].mnTrailStart;
                    if (nChar >= n && nChar <= pBig5Data[nRow].mnTrailEnd)
                    {
                        nUnicode = pBig5Data[nRow].mpToUniTrailTab[nChar - n];
                        if (nUnicode == 0)
                            nUnicode = 0xFFFF;
                        assert(!ImplIsHighSurrogate(nUnicode));
                    }
                }
                if (nUnicode == 0xFFFF)
                {
                    ImplDBCSEUDCData const * p
                        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
                              m_pEudcData;
                    sal_uInt32 nCount
                        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
                              m_nEudcCount;
                    sal_uInt32 i;
                    for (i = 0; i < nCount; ++i)
                    {
                        if (nRow >= p->mnLeadStart && nRow <= p->mnLeadEnd)
                        {
                            if (nChar < p->mnTrail1Start)
                                break;
                            if (nChar <= p->mnTrail1End)
                            {
                                nUnicode
                                    = p->mnUniStart
                                          + (nRow - p->mnLeadStart)
                                                * p->mnTrailRangeCount
                                          + (nChar - p->mnTrail1Start);
                                break;
                            }
                            if (p->mnTrailCount < 2
                                || nChar < p->mnTrail2Start)
                                break;
                            if (nChar <= p->mnTrail2End)
                            {
                                nUnicode
                                    = p->mnUniStart
                                          + (nRow - p->mnLeadStart)
                                                * p->mnTrailRangeCount
                                          + (nChar - p->mnTrail2Start)
                                          + (p->mnTrail1End - p->mnTrail1Start
                                                 + 1);
                                break;
                            }
                            if (p->mnTrailCount < 3
                                || nChar < p->mnTrail3Start)
                                break;
                            if (nChar <= p->mnTrail3End)
                            {
                                nUnicode
                                    = p->mnUniStart
                                          + (nRow - p->mnLeadStart)
                                                * p->mnTrailRangeCount
                                          + (nChar - p->mnTrail3Start)
                                          + (p->mnTrail1End - p->mnTrail1Start
                                                 + 1)
                                          + (p->mnTrail2End - p->mnTrail2Start
                                                 + 1);
                                break;
                            }
                            break;
                        }
                        ++p;
                    }
                    assert(!ImplIsHighSurrogate(nUnicode));
                }
                if (nUnicode == 0xFFFF)
                    goto bad_input;
                if (ImplIsHighSurrogate(nUnicode))
                    if (pDestBufEnd - pDestBufPtr >= 2)
                    {
                        nOffset += nLast - nFirst + 1;
                        nFirst = pBig5Hkscs2001Data[nOffset++];
                        *pDestBufPtr++ = (sal_Unicode) nUnicode;
                        *pDestBufPtr++
                            = (sal_Unicode) pBig5Hkscs2001Data[
                                                nOffset + (nChar - nFirst)];
                    }
                    else
                        goto no_output;
                else
                    if (pDestBufPtr != pDestBufEnd)
                        *pDestBufPtr++ = (sal_Unicode) nUnicode;
                    else
                        goto no_output;
                nRow = 0;
            }
            else
            {
                bUndefined = false;
                goto bad_input;
            }
        continue;

    bad_input:
        switch (sal::detail::textenc::handleBadInputTextToUnicodeConversion(
                    bUndefined, true, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case sal::detail::textenc::BAD_INPUT_STOP:
            nRow = 0;
            break;

        case sal::detail::textenc::BAD_INPUT_CONTINUE:
            nRow = 0;
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

    if (nRow != 0
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
                nRow = 0;
                break;

            case sal::detail::textenc::BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
        static_cast< ImplBig5HkscsToUnicodeContext * >(pContext)->m_nRow = nRow;
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToBig5Hkscs(void const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    sal_uInt16 const * pBig5Hkscs2001Data
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pUnicodeToBig5Hkscs2001Data;
    sal_Int32 const * pBig5Hkscs2001PageOffsets
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pUnicodeToBig5Hkscs2001PageOffsets;
    sal_Int32 const * pBig5Hkscs2001PlaneOffsets
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pUnicodeToBig5Hkscs2001PlaneOffsets;
    ImplUniToDBCSHighTab const * pBig5Data
        = static_cast< ImplBig5HkscsConverterData const * >(pData)->
              m_pUnicodeToBig5Data;
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
            sal_uInt32 nBytes = 0;
            sal_Int32 nOffset = pBig5Hkscs2001PlaneOffsets[nChar >> 16];
            if (nOffset != -1)
            {
                nOffset
                    = pBig5Hkscs2001PageOffsets[nOffset + ((nChar & 0xFF00)
                                                               >> 8)];
                if (nOffset != -1)
                {
                    sal_uInt32 nFirstLast = pBig5Hkscs2001Data[nOffset++];
                    sal_uInt32 nFirst = nFirstLast & 0xFF;
                    sal_uInt32 nLast = nFirstLast >> 8;
                    sal_uInt32 nIndex = nChar & 0xFF;
                    if (nIndex >= nFirst && nIndex <= nLast)
                    {
                        nBytes
                            = pBig5Hkscs2001Data[nOffset + (nIndex - nFirst)];
                    }
                }
            }
            if (nBytes == 0)
            {
                sal_uInt32 nIndex1 = nChar >> 8;
                if (nIndex1 < 0x100)
                {
                    sal_uInt32 nIndex2 = nChar & 0xFF;
                    sal_uInt32 nFirst = pBig5Data[nIndex1].mnLowStart;
                    if (nIndex2 >= nFirst
                        && nIndex2 <= pBig5Data[nIndex1].mnLowEnd)
                        nBytes = pBig5Data[nIndex1].
                                     mpToUniTrailTab[nIndex2 - nFirst];
                }
            }
            if (nBytes == 0)
            {
                ImplDBCSEUDCData const * p
                    = static_cast< ImplBig5HkscsConverterData const * >(pData)->
                          m_pEudcData;
                sal_uInt32 nCount
                    = static_cast< ImplBig5HkscsConverterData const * >(pData)->
                          m_nEudcCount;
                sal_uInt32 i;
                for (i = 0; i < nCount; ++i) {
                    if (nChar >= p->mnUniStart && nChar <= p->mnUniEnd)
                    {
                        sal_uInt32 nIndex = nChar - p->mnUniStart;
                        sal_uInt32 nLeadOff = nIndex / p->mnTrailRangeCount;
                        sal_uInt32 nTrailOff = nIndex % p->mnTrailRangeCount;
                        sal_uInt32 nSize;
                        nBytes = (p->mnLeadStart + nLeadOff) << 8;
                        nSize = p->mnTrail1End - p->mnTrail1Start + 1;
                        if (nTrailOff < nSize)
                        {
                            nBytes |= p->mnTrail1Start + nTrailOff;
                            break;
                        }
                        nTrailOff -= nSize;
                        nSize = p->mnTrail2End - p->mnTrail2Start + 1;
                        if (nTrailOff < nSize)
                        {
                            nBytes |= p->mnTrail2Start + nTrailOff;
                            break;
                        }
                        nTrailOff -= nSize;
                        nBytes |= p->mnTrail3Start + nTrailOff;
                        break;
                    }
                    ++p;
                }
            }
            if (nBytes == 0)
                goto bad_input;
            if (pDestBufEnd - pDestBufPtr >= 2)
            {
                *pDestBufPtr++ = static_cast< char >(nBytes >> 8);
                *pDestBufPtr++ = static_cast< char >(nBytes & 0xFF);
            }
            else
                goto no_output;
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
