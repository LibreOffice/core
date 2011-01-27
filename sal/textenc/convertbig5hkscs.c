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

#include "convertbig5hkscs.h"
#include "context.h"
#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "osl/diagnose.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

typedef struct
{
    sal_Int32 m_nRow; /* 0--255; 0 means none */
} ImplBig5HkscsToUnicodeContext;

void * ImplCreateBig5HkscsToUnicodeContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplBig5HkscsToUnicodeContext));
    ((ImplBig5HkscsToUnicodeContext *) pContext)->m_nRow = 0;
    return pContext;
}

void ImplResetBig5HkscsToUnicodeContext(void * pContext)
{
    if (pContext)
        ((ImplBig5HkscsToUnicodeContext *) pContext)->m_nRow = 0;
}

sal_Size ImplConvertBig5HkscsToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    sal_uInt16 const * pBig5Hkscs2001Data
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pBig5Hkscs2001ToUnicodeData;
    sal_Int32 const * pBig5Hkscs2001RowOffsets
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pBig5Hkscs2001ToUnicodeRowOffsets;
    ImplDBCSToUniLeadTab const * pBig5Data
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pBig5ToUnicodeData;
    sal_Int32 nRow = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
        nRow = ((ImplBig5HkscsToUnicodeContext *) pContext)->m_nRow;

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
        sal_uInt32 nChar = *(sal_uChar const *) pSrcBuf++;
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
                bUndefined = sal_False;
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
                    sal_uInt32 nFirst = pBig5Data[nRow].mnTrailStart;
                    if (nChar >= nFirst
                        && nChar <= pBig5Data[nRow].mnTrailEnd)
                    {
                        nUnicode
                            = pBig5Data[nRow].mpToUniTrailTab[nChar - nFirst];
                        if (nUnicode == 0)
                            nUnicode = 0xFFFF;
                        OSL_VERIFY(!ImplIsHighSurrogate(nUnicode));
                    }
                }
                if (nUnicode == 0xFFFF)
                {
                    ImplDBCSEUDCData const * p
                        = ((ImplBig5HkscsConverterData const *) pData)->
                              m_pEudcData;
                    sal_uInt32 nCount
                        = ((ImplBig5HkscsConverterData const *) pData)->
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
                    OSL_VERIFY(!ImplIsHighSurrogate(nUnicode));
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
                bUndefined = sal_False;
                goto bad_input;
            }
        continue;

    bad_input:
        switch (ImplHandleBadInputTextToUnicodeConversion(
                    bUndefined, sal_True, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case IMPL_BAD_INPUT_STOP:
            nRow = 0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            nRow = 0;
            continue;

        case IMPL_BAD_INPUT_NO_OUTPUT:
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
            switch (ImplHandleBadInputTextToUnicodeConversion(
                        sal_False, sal_True, 0, nFlags, &pDestBufPtr,
                        pDestBufEnd, &nInfo))
            {
            case IMPL_BAD_INPUT_STOP:
            case IMPL_BAD_INPUT_CONTINUE:
                nRow = 0;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
        ((ImplBig5HkscsToUnicodeContext *) pContext)->m_nRow = nRow;
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToBig5Hkscs(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       sal_Char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    sal_uInt16 const * pBig5Hkscs2001Data
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pUnicodeToBig5Hkscs2001Data;
    sal_Int32 const * pBig5Hkscs2001PageOffsets
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pUnicodeToBig5Hkscs2001PageOffsets;
    sal_Int32 const * pBig5Hkscs2001PlaneOffsets
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pUnicodeToBig5Hkscs2001PlaneOffsets;
    ImplUniToDBCSHighTab const * pBig5Data
        = ((ImplBig5HkscsConverterData const *) pData)->
              m_pUnicodeToBig5Data;
    sal_Unicode nHighSurrogate = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Char * pDestBufPtr = pDestBuf;
    sal_Char * pDestBufEnd = pDestBuf + nDestBytes;

    if (pContext)
        nHighSurrogate
            = ((ImplUnicodeToTextContext *) pContext)->m_nHighSurrogate;

    for (; nConverted < nSrcChars; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
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
            bUndefined = sal_False;
            goto bad_input;
        }

        if (ImplIsLowSurrogate(nChar) || ImplIsNoncharacter(nChar))
        {
            bUndefined = sal_False;
            goto bad_input;
        }

        if (nChar < 0x80)
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = (sal_Char) nChar;
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
                    = ((ImplBig5HkscsConverterData const *) pData)->
                          m_pEudcData;
                sal_uInt32 nCount
                    = ((ImplBig5HkscsConverterData const *) pData)->
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
                *pDestBufPtr++ = (sal_Char) (nBytes >> 8);
                *pDestBufPtr++ = (sal_Char) (nBytes & 0xFF);
            }
            else
                goto no_output;
        }
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (ImplHandleBadInputUnicodeToTextConversion(bUndefined,
                                                          nChar,
                                                          nFlags,
                                                          &pDestBufPtr,
                                                          pDestBufEnd,
                                                          &nInfo,
                                                          NULL,
                                                          0,
                                                          NULL))
        {
        case IMPL_BAD_INPUT_STOP:
            nHighSurrogate = 0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            nHighSurrogate = 0;
            continue;

        case IMPL_BAD_INPUT_NO_OUTPUT:
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
            switch (ImplHandleBadInputUnicodeToTextConversion(sal_False,
                                                              0,
                                                              nFlags,
                                                              &pDestBufPtr,
                                                              pDestBufEnd,
                                                              &nInfo,
                                                              NULL,
                                                              0,
                                                              NULL))
            {
            case IMPL_BAD_INPUT_STOP:
            case IMPL_BAD_INPUT_CONTINUE:
                nHighSurrogate = 0;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
        ((ImplUnicodeToTextContext *) pContext)->m_nHighSurrogate
            = nHighSurrogate;
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
