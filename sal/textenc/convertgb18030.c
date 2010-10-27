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

#include "convertgb18030.h"
#include "context.h"
#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

typedef enum
{
    IMPL_GB_18030_TO_UNICODE_STATE_0,
    IMPL_GB_18030_TO_UNICODE_STATE_1,
    IMPL_GB_18030_TO_UNICODE_STATE_2,
    IMPL_GB_18030_TO_UNICODE_STATE_3
} ImplGb18030ToUnicodeState;

typedef struct
{
    ImplGb18030ToUnicodeState m_eState;
    sal_uInt32 m_nCode;
} ImplGb18030ToUnicodeContext;

void * ImplCreateGb18030ToUnicodeContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplGb18030ToUnicodeContext));
    ((ImplGb18030ToUnicodeContext *) pContext)->m_eState
        = IMPL_GB_18030_TO_UNICODE_STATE_0;
    return pContext;
}

void ImplResetGb18030ToUnicodeContext(void * pContext)
{
    if (pContext)
        ((ImplGb18030ToUnicodeContext *) pContext)->m_eState
            = IMPL_GB_18030_TO_UNICODE_STATE_0;
}

sal_Size ImplConvertGb18030ToUnicode(ImplTextConverterData const * pData,
                                     void * pContext,
                                     sal_Char const * pSrcBuf,
                                     sal_Size nSrcBytes,
                                     sal_Unicode * pDestBuf,
                                     sal_Size nDestChars,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtBytes)
{
    sal_Unicode const * pGb18030Data
        = ((ImplGb18030ConverterData const *) pData)->m_pGb18030ToUnicodeData;
    ImplGb180302000ToUnicodeRange const * pGb18030Ranges
        = ((ImplGb18030ConverterData const *) pData)->
              m_pGb18030ToUnicodeRanges;
    ImplGb18030ToUnicodeState eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
    sal_uInt32 nCode = 0;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBuf + nDestChars;

    if (pContext)
    {
        eState = ((ImplGb18030ToUnicodeContext *) pContext)->m_eState;
        nCode = ((ImplGb18030ToUnicodeContext *) pContext)->m_nCode;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
        sal_uInt32 nChar = *(sal_uChar const *) pSrcBuf++;
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
                bUndefined = sal_False;
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
                bUndefined = sal_False;
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
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_GB_18030_TO_UNICODE_STATE_3:
            if (nChar >= 0x30 && nChar <= 0x39)
            {
                nCode = nCode * 10 + (nChar - 0x30);

                /* 90 30 81 30 to E3 32 9A 35 maps to U+10000 to U+10FFFF: */
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
                bUndefined = sal_False;
                goto bad_input;
            }
            break;
        }
        continue;

    bad_input:
        switch (ImplHandleBadInputTextToUnicodeConversion(
                    bUndefined, sal_True, 0, nFlags, &pDestBufPtr, pDestBufEnd,
                    &nInfo))
        {
        case IMPL_BAD_INPUT_STOP:
            eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
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

    if (eState != IMPL_GB_18030_TO_UNICODE_STATE_0
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
                eState = IMPL_GB_18030_TO_UNICODE_STATE_0;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        ((ImplGb18030ToUnicodeContext *) pContext)->m_eState = eState;
        ((ImplGb18030ToUnicodeContext *) pContext)->m_nCode = nCode;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

sal_Size ImplConvertUnicodeToGb18030(ImplTextConverterData const * pData,
                                     void * pContext,
                                     sal_Unicode const * pSrcBuf,
                                     sal_Size nSrcChars,
                                     sal_Char * pDestBuf,
                                     sal_Size nDestBytes,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtChars)
{
    sal_uInt32 const * pGb18030Data
        = ((ImplGb18030ConverterData const *) pData)->
              m_pUnicodeToGb18030Data;
    ImplUnicodeToGb180302000Range const * pGb18030Ranges
        = ((ImplGb18030ConverterData const *) pData)->
              m_pUnicodeToGb18030Ranges;
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
                            *pDestBufPtr++ = (sal_Char) (nCode >> 24);
                            *pDestBufPtr++ = (sal_Char) (nCode >> 16 & 0xFF);
                        }
                        *pDestBufPtr++ = (sal_Char) (nCode >> 8 & 0xFF);
                        *pDestBufPtr++ = (sal_Char) (nCode & 0xFF);
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
                        *pDestBufPtr++ = (sal_Char) (nCode / 12600 + 0x81);
                        *pDestBufPtr++
                            = (sal_Char) (nCode / 1260 % 10 + 0x30);
                        *pDestBufPtr++ = (sal_Char) (nCode / 10 % 126 + 0x81);
                        *pDestBufPtr++ = (sal_Char) (nCode % 10 + 0x30);
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
                *pDestBufPtr++ = (sal_Char) (nCode / 12600 + 0x90);
                *pDestBufPtr++ = (sal_Char) (nCode / 1260 % 10 + 0x30);
                *pDestBufPtr++ = (sal_Char) (nCode / 10 % 126 + 0x81);
                *pDestBufPtr++ = (sal_Char) (nCode % 10 + 0x30);
            }
            else
                goto no_output;
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
