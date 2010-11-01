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

#include "convertiso2022cn.h"
#include "context.h"
#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

typedef enum /* order is important: */
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
} ImplIso2022CnToUnicodeState;

typedef struct
{
    ImplIso2022CnToUnicodeState m_eState;
    sal_uInt32 m_nRow;
    sal_Bool m_bSo;
    sal_Bool m_b116431;
} ImplIso2022CnToUnicodeContext;

typedef enum
{
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE,
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312,
    IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_116431
} ImplUnicodeToIso2022CnDesignator;

typedef struct
{
    sal_Unicode m_nHighSurrogate;
    ImplUnicodeToIso2022CnDesignator m_eSoDesignator;
    sal_Bool m_b116432Designator;
    sal_Bool m_bSo;
} ImplUnicodeToIso2022CnContext;

void * ImplCreateIso2022CnToUnicodeContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplIso2022CnToUnicodeContext));
    ((ImplIso2022CnToUnicodeContext *) pContext)->m_eState
        = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
    ((ImplIso2022CnToUnicodeContext *) pContext)->m_bSo = sal_False;
    ((ImplIso2022CnToUnicodeContext *) pContext)->m_b116431 = sal_False;
    return pContext;
}

void ImplResetIso2022CnToUnicodeContext(void * pContext)
{
    if (pContext)
    {
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_eState
            = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_bSo = sal_False;
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_b116431 = sal_False;
    }
}

sal_Size ImplConvertIso2022CnToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    ImplDBCSToUniLeadTab const * pGb2312Data
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pGb2312ToUnicodeData;
    sal_uInt16 const * pCns116431992Data
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pCns116431992ToUnicodeData;
    sal_Int32 const * pCns116431992RowOffsets
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pCns116431992ToUnicodePlaneOffsets;
    ImplIso2022CnToUnicodeState eState
        = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
    sal_uInt32 nRow = 0;
    sal_Bool bSo = sal_False;
    sal_Bool b116431 = sal_False;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = ((ImplIso2022CnToUnicodeContext *) pContext)->m_eState;
        nRow = ((ImplIso2022CnToUnicodeContext *) pContext)->m_nRow;
        bSo = ((ImplIso2022CnToUnicodeContext *) pContext)->m_bSo;
        b116431 = ((ImplIso2022CnToUnicodeContext *) pContext)->m_b116431;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
        sal_uInt32 nChar = *(sal_uChar const *) pSrcBuf++;
        sal_uInt32 nPlane;
        switch (eState)
        {
        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII:
            if (nChar == 0x0E) /* SO */
            {
                bSo = sal_True;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO;
            }
            else if (nChar == 0x1B) /* ESC */
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC;
            else if (nChar < 0x80)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nChar;
                else
                    goto no_output;
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO:
            if (nChar == 0x0F) /* SI */
            {
                bSo = sal_False;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else if (nChar == 0x1B) /* ESC */
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC;
            else if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar;
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO_2;
            }
            else
            {
                bUndefined = sal_False;
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
                            *pDestBufPtr++ = (sal_Unicode) nUnicode;
                            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO;
                        }
                        else
                            goto no_output;
                    else
                        goto bad_input;
                }
            else
            {
                bUndefined = sal_False;
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
                bUndefined = sal_False;
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
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC:
            if (nChar == 0x24) /* $ */
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR;
            else if (nChar == 0x4E) /* N */
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_116432;
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR:
            if (nChar == 0x29) /* ) */
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN;
            else if (nChar == 0x2A) /* * */
                eState
                    = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_ASTERISK;
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN:
            if (nChar == 0x41) /* A */
            {
                b116431 = sal_False;
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else if (nChar == 0x47) /* G */
            {
                b116431 = sal_True;
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            }
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_CN_TO_UNICODE_STATE_ESC_DOLLAR_ASTERISK:
            if (nChar == 0x48) /* H */
                eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                               IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            else
            {
                bUndefined = sal_False;
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
                    eState = bSo ? IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO :
                                   IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
                }
            }
            continue;
        }

    bad_input:
        switch (ImplHandleBadInputTextToUnicodeConversion(
                    bUndefined, sal_True, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case IMPL_BAD_INPUT_STOP:
            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            b116431 = sal_False;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
            b116431 = sal_False;
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

    if (eState > IMPL_ISO_2022_CN_TO_UNICODE_STATE_SO
        && (nInfo & (RTL_TEXTTOUNICODE_INFO_ERROR
                         | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL))
               == 0)
    {
        if ((nFlags & RTL_TEXTTOUNICODE_FLAGS_FLUSH) == 0)
            nInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
        else
            switch (ImplHandleBadInputTextToUnicodeConversion(
                        sal_False, sal_True, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                        &nInfo))
            {
            case IMPL_BAD_INPUT_STOP:
            case IMPL_BAD_INPUT_CONTINUE:
                eState = IMPL_ISO_2022_CN_TO_UNICODE_STATE_ASCII;
                b116431 = sal_False;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_eState = eState;
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_nRow = nRow;
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_bSo = bSo;
        ((ImplIso2022CnToUnicodeContext *) pContext)->m_b116431 = b116431;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToIso2022CnContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplUnicodeToIso2022CnContext));
    ((ImplUnicodeToIso2022CnContext *) pContext)->m_nHighSurrogate = 0;
    ((ImplUnicodeToIso2022CnContext *) pContext)->m_eSoDesignator
        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
    ((ImplUnicodeToIso2022CnContext *) pContext)->m_b116432Designator
        = sal_False;
    ((ImplUnicodeToIso2022CnContext *) pContext)->m_bSo = sal_False;
    return pContext;
}

void ImplResetUnicodeToIso2022CnContext(void * pContext)
{
    if (pContext)
    {
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_nHighSurrogate = 0;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_eSoDesignator
            = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_b116432Designator
            = sal_False;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_bSo = sal_False;
    }
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

sal_Size ImplConvertUnicodeToIso2022Cn(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       sal_Char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    ImplUniToDBCSHighTab const * pGb2312Data
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pUnicodeToGb2312Data;
    sal_uInt8 const * pCns116431992Data
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pUnicodeToCns116431992Data;
    sal_Int32 const * pCns116431992PageOffsets
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = ((ImplIso2022CnConverterData const *) pData)->
              m_pUnicodeToCns116431992PlaneOffsets;
    sal_Unicode nHighSurrogate = 0;
    ImplUnicodeToIso2022CnDesignator eSoDesignator
        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
    sal_Bool b116432Designator = sal_False;
    sal_Bool bSo = sal_False;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Char * pDestBufPtr = pDestBuf;
    sal_Char * pDestBufEnd = pDestBuf + nDestBytes;
    sal_Bool bWritten;

    if (pContext)
    {
        nHighSurrogate
            = ((ImplUnicodeToIso2022CnContext *) pContext)->m_nHighSurrogate;
        eSoDesignator
            = ((ImplUnicodeToIso2022CnContext *) pContext)->m_eSoDesignator;
        b116432Designator = ((ImplUnicodeToIso2022CnContext *) pContext)->
                                m_b116432Designator;
        bSo = ((ImplUnicodeToIso2022CnContext *) pContext)->m_bSo;
    }

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

        if (nChar == 0x0A || nChar == 0x0D) /* LF, CR */
        {
            if (bSo)
            {
                if (pDestBufPtr != pDestBufEnd)
                {
                    *pDestBufPtr++ = 0x0F; /* SI */
                    bSo = sal_False;
                    eSoDesignator
                        = IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_NONE;
                    b116432Designator = sal_False;
                }
                else
                    goto no_output;
            }
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = (sal_Char) nChar;
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
                    *pDestBufPtr++ = 0x0F; /* SI */
                    bSo = sal_False;
                }
                else
                    goto no_output;
            }
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = (sal_Char) nChar;
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
                            *pDestBufPtr++ = 0x0F; /* SI */
                            bSo = sal_False;
                        }
                        else
                            goto no_output;
                    }
                    if (pDestBufEnd - pDestBufPtr >= 4)
                    {
                        *pDestBufPtr++ = 0x1B; /* ESC */
                        *pDestBufPtr++ = 0x24; /* $ */
                        *pDestBufPtr++ = 0x29; /* ) */
                        *pDestBufPtr++
                            = eNewDesignator
                              == IMPL_UNICODE_TO_ISO_2022_CN_DESIGNATOR_2312 ?
                                  0x41 : 0x47; /* A, G */
                        eSoDesignator = eNewDesignator;
                    }
                    else
                        goto no_output;
                }
                if (!bSo)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0E; /* SO */
                        bSo = sal_True;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufEnd - pDestBufPtr >= 4)
                {
                    *pDestBufPtr++ = (sal_Char) (nBytes >> 8);
                    *pDestBufPtr++ = (sal_Char) (nBytes & 0xFF);
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
                        *pDestBufPtr++ = 0x1B; /* ESC */
                        *pDestBufPtr++ = 0x24; /* $ */
                        *pDestBufPtr++ = 0x2A; /* * */
                        *pDestBufPtr++ = 0x48; /* H */
                        b116432Designator = sal_True;
                    }
                    else
                        goto no_output;
                }
                if (pDestBufEnd - pDestBufPtr >= 4)
                {
                    *pDestBufPtr++ = 0x1B; /* ESC */
                    *pDestBufPtr++ = 0x4E; /* N */
                    *pDestBufPtr++
                        = (sal_Char) (0x20 + pCns116431992Data[nOffset++]);
                    *pDestBufPtr++
                        = (sal_Char) (0x20 + pCns116431992Data[nOffset]);
                }
                else
                    goto no_output;
            }
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
                                                          "\x0F", /* SI */
                                                          bSo ? 1 : 0,
                                                          &bWritten))
        {
        case IMPL_BAD_INPUT_STOP:
            nHighSurrogate = 0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            if (bWritten)
                bSo = sal_False;
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

    if ((nInfo & (RTL_UNICODETOTEXT_INFO_ERROR
                      | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
            == 0)
    {
        sal_Bool bFlush = sal_True;
        if (nHighSurrogate != 0)
        {
            if ((nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
                nInfo |= RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
            else
                switch (ImplHandleBadInputUnicodeToTextConversion(
                            sal_False,
                            0,
                            nFlags,
                            &pDestBufPtr,
                            pDestBufEnd,
                            &nInfo,
                            "\x0F", /* SI */
                            bSo ? 1 : 0,
                            &bWritten))
                {
                case IMPL_BAD_INPUT_STOP:
                    nHighSurrogate = 0;
                    bFlush = sal_False;
                    break;

                case IMPL_BAD_INPUT_CONTINUE:
                    if (bWritten)
                        bSo = sal_False;
                    nHighSurrogate = 0;
                    break;

                case IMPL_BAD_INPUT_NO_OUTPUT:
                    nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
        }
        if (bFlush && bSo && (nFlags & RTL_UNICODETOTEXT_FLAGS_FLUSH) != 0)
        {
            if (pDestBufPtr != pDestBufEnd)
            {
                *pDestBufPtr++ = 0x0F; /* SI */
                bSo = sal_False;
            }
            else
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        }
    }

    if (pContext)
    {
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_nHighSurrogate
            = nHighSurrogate;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_eSoDesignator
            = eSoDesignator;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_b116432Designator
            = b116432Designator;
        ((ImplUnicodeToIso2022CnContext *) pContext)->m_bSo = bSo;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
