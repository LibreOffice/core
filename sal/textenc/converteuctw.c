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

#include "converteuctw.h"
#include "context.h"
#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

typedef enum
{
    IMPL_EUC_TW_TO_UNICODE_STATE_0,
    IMPL_EUC_TW_TO_UNICODE_STATE_1,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_1,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_2,
    IMPL_EUC_TW_TO_UNICODE_STATE_2_3
} ImplEucTwToUnicodeState;

typedef struct
{
    ImplEucTwToUnicodeState m_eState;
    sal_Int32 m_nPlane; /* 0--15 */
    sal_Int32 m_nRow; /* 0--93 */
} ImplEucTwToUnicodeContext;

void * ImplCreateEucTwToUnicodeContext(void)
{
    void * pContext = rtl_allocateMemory(sizeof (ImplEucTwToUnicodeContext));
    ((ImplEucTwToUnicodeContext *) pContext)->m_eState
        = IMPL_EUC_TW_TO_UNICODE_STATE_0;
    return pContext;
}

void ImplResetEucTwToUnicodeContext(void * pContext)
{
    if (pContext)
        ((ImplEucTwToUnicodeContext *) pContext)->m_eState
            = IMPL_EUC_TW_TO_UNICODE_STATE_0;
}

sal_Size ImplConvertEucTwToUnicode(ImplTextConverterData const * pData,
                                   void * pContext,
                                   sal_Char const * pSrcBuf,
                                   sal_Size nSrcBytes,
                                   sal_Unicode * pDestBuf,
                                   sal_Size nDestChars,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtBytes)
{
    sal_uInt16 const * pCns116431992Data
        = ((ImplEucTwConverterData const *) pData)->
              m_pCns116431992ToUnicodeData;
    sal_Int32 const * pCns116431992RowOffsets
        = ((ImplEucTwConverterData const *) pData)->
              m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = ((ImplEucTwConverterData const *) pData)->
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
        eState = ((ImplEucTwToUnicodeContext *) pContext)->m_eState;
        nPlane = ((ImplEucTwToUnicodeContext *) pContext)->m_nPlane;
        nRow = ((ImplEucTwToUnicodeContext *) pContext)->m_nRow;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
        sal_uInt32 nChar = *(sal_uChar const *) pSrcBuf++;
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
                bUndefined = sal_False;
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
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_1:
            if (nChar >= 0xA1 && nChar <= 0xB0)
            {
                nPlane = nChar - 0xA1;
                ++eState;
            }
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_2:
            if (nChar >= 0xA1 && nChar <= 0xFE)
            {
                nRow = nChar - 0xA1;
                ++eState;
            }
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_EUC_TW_TO_UNICODE_STATE_2_3:
            if (nChar >= 0xA1 && nChar <= 0xFE)
                goto transform;
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
        switch (ImplHandleBadInputTextToUnicodeConversion(
                    bUndefined, sal_True, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case IMPL_BAD_INPUT_STOP:
            eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
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

    if (eState != IMPL_EUC_TW_TO_UNICODE_STATE_0
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
                eState = IMPL_EUC_TW_TO_UNICODE_STATE_0;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        ((ImplEucTwToUnicodeContext *) pContext)->m_eState = eState;
        ((ImplEucTwToUnicodeContext *) pContext)->m_nPlane = nPlane;
        ((ImplEucTwToUnicodeContext *) pContext)->m_nRow = nRow;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToEucTw(ImplTextConverterData const * pData,
                                   void * pContext,
                                   sal_Unicode const * pSrcBuf,
                                   sal_Size nSrcChars,
                                   sal_Char * pDestBuf,
                                   sal_Size nDestBytes,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtChars)
{
    sal_uInt8 const * pCns116431992Data
        = ((ImplEucTwConverterData const *) pData)->
              m_pUnicodeToCns116431992Data;
    sal_Int32 const * pCns116431992PageOffsets
        = ((ImplEucTwConverterData const *) pData)->
              m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * pCns116431992PlaneOffsets
        = ((ImplEucTwConverterData const *) pData)->
              m_pUnicodeToCns116431992PlaneOffsets;
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
                *pDestBufPtr++ = (sal_Char) (unsigned char) 0x8E;
                *pDestBufPtr++ = (sal_Char) (0xA0 + nPlane);
            }
            *pDestBufPtr++ = (sal_Char) (0xA0 + pCns116431992Data[nOffset++]);
            *pDestBufPtr++ = (sal_Char) (0xA0 + pCns116431992Data[nOffset]);
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
