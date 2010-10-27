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

#include "convertiso2022kr.h"
#include "context.h"
#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

typedef enum /* order is important: */
{
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001_2,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR,
    IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN
} ImplIso2022KrToUnicodeState;

typedef struct
{
    ImplIso2022KrToUnicodeState m_eState;
    sal_uInt32 m_nRow;
} ImplIso2022KrToUnicodeContext;

typedef enum
{
    IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE,
    IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII,
    IMPL_UNICODE_TO_ISO_2022_KR_SET_1001
} ImplUnicodeToIso2022KrSet;

typedef struct
{
    sal_Unicode m_nHighSurrogate;
    ImplUnicodeToIso2022KrSet m_eSet;
} ImplUnicodeToIso2022KrContext;

void * ImplCreateIso2022KrToUnicodeContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplIso2022KrToUnicodeContext));
    ((ImplIso2022KrToUnicodeContext *) pContext)->m_eState
        = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
    return pContext;
}

void ImplResetIso2022KrToUnicodeContext(void * pContext)
{
    if (pContext)
        ((ImplIso2022KrToUnicodeContext *) pContext)->m_eState
            = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
}

sal_Size ImplConvertIso2022KrToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
{
    ImplDBCSToUniLeadTab const * pKsX1001Data
        = ((ImplIso2022KrConverterData const *) pData)->
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
        eState = ((ImplIso2022KrToUnicodeContext *) pContext)->m_eState;
        nRow = ((ImplIso2022KrToUnicodeContext *) pContext)->m_nRow;
    }

    for (; nConverted < nSrcBytes; ++nConverted)
    {
        sal_Bool bUndefined = sal_True;
        sal_uInt32 nChar = *(sal_uChar const *) pSrcBuf++;
        switch (eState)
        {
        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII:
            if (nChar == 0x0E) /* SO */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001;
            else if (nChar == 0x1B) /* ESC */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC;
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

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001:
            if (nChar == 0x0F) /* SI */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
            else if (nChar >= 0x21 && nChar <= 0x7E)
            {
                nRow = nChar + 0x80;
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001_2;
            }
            else
            {
                bUndefined = sal_False;
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
                        *pDestBufPtr++ = (sal_Unicode) nUnicode;
                        eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001;
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

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC:
            if (nChar == 0x24) /* $ */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR;
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR:
            if (nChar == 0x29) /* ) */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN;
            else
            {
                bUndefined = sal_False;
                goto bad_input;
            }
            break;

        case IMPL_ISO_2022_KR_TO_UNICODE_STATE_ESC_DOLLAR_RPAREN:
            if (nChar == 0x43) /* C */
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
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
            eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
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

    if (eState > IMPL_ISO_2022_KR_TO_UNICODE_STATE_1001
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
                eState = IMPL_ISO_2022_KR_TO_UNICODE_STATE_ASCII;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext)
    {
        ((ImplIso2022KrToUnicodeContext *) pContext)->m_eState = eState;
        ((ImplIso2022KrToUnicodeContext *) pContext)->m_nRow = nRow;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtBytes)
        *pSrcCvtBytes = nConverted;

    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToIso2022KrContext(void)
{
    void * pContext
        = rtl_allocateMemory(sizeof (ImplUnicodeToIso2022KrContext));
    ((ImplUnicodeToIso2022KrContext *) pContext)->m_nHighSurrogate = 0;
    ((ImplUnicodeToIso2022KrContext *) pContext)->m_eSet
        = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    return pContext;
}

void ImplResetUnicodeToIso2022KrContext(void * pContext)
{
    if (pContext)
    {
        ((ImplUnicodeToIso2022KrContext *) pContext)->m_nHighSurrogate = 0;
        ((ImplUnicodeToIso2022KrContext *) pContext)->m_eSet
            = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    }
}

sal_Size ImplConvertUnicodeToIso2022Kr(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       sal_Char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
{
    ImplUniToDBCSHighTab const * pKsX1001Data
        = ((ImplIso2022KrConverterData const *) pData)->
              m_pUnicodeToKsX1001Data;
    sal_Unicode nHighSurrogate = 0;
    ImplUnicodeToIso2022KrSet eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE;
    sal_uInt32 nInfo = 0;
    sal_Size nConverted = 0;
    sal_Char * pDestBufPtr = pDestBuf;
    sal_Char * pDestBufEnd = pDestBuf + nDestBytes;
    sal_Bool bWritten;

    if (pContext)
    {
        nHighSurrogate
            = ((ImplUnicodeToIso2022KrContext *) pContext)->m_nHighSurrogate;
        eSet = ((ImplUnicodeToIso2022KrContext *) pContext)->m_eSet;
    }

    if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_NONE)
    {
        if (pDestBufEnd - pDestBufPtr >= 4)
        {
            *pDestBufPtr++ = 0x1B; /* ESC */
            *pDestBufPtr++ = 0x24; /* $ */
            *pDestBufPtr++ = 0x29; /* ) */
            *pDestBufPtr++ = 0x43; /* C */
            eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
        }
        else
            nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
    }

    if ((nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL) == 0)
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
                if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_1001)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0F; /* SI */
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
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
                if (eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_1001)
                {
                    if (pDestBufPtr != pDestBufEnd)
                    {
                        *pDestBufPtr++ = 0x0F; /* SI */
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
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
                            *pDestBufPtr++ = 0x0E; /* SO */
                            eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_1001;
                        }
                        else
                            goto no_output;
                    }
                    if (pDestBufEnd - pDestBufPtr >= 2)
                    {
                        *pDestBufPtr++ = (sal_Char) ((nBytes >> 8) & 0x7F);
                        *pDestBufPtr++ = (sal_Char) (nBytes & 0x7F);
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
            switch (ImplHandleBadInputUnicodeToTextConversion(
                        bUndefined,
                        nChar,
                        nFlags,
                        &pDestBufPtr,
                        pDestBufEnd,
                        &nInfo,
                        "\x0F", /* SI */
                        eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII ? 0 : 1,
                        &bWritten))
            {
            case IMPL_BAD_INPUT_STOP:
                nHighSurrogate = 0;
                break;

            case IMPL_BAD_INPUT_CONTINUE:
                if (bWritten)
                    eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
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
                            eSet == IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII ?
                                0 : 1,
                            &bWritten))
                {
                case IMPL_BAD_INPUT_STOP:
                    nHighSurrogate = 0;
                    bFlush = sal_False;
                    break;

                case IMPL_BAD_INPUT_CONTINUE:
                    if (bWritten)
                        eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
                    nHighSurrogate = 0;
                    break;

                case IMPL_BAD_INPUT_NO_OUTPUT:
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
                *pDestBufPtr++ = 0x0F; /* SI */
                eSet = IMPL_UNICODE_TO_ISO_2022_KR_SET_ASCII;
            }
            else
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        }
    }

    if (pContext)
    {
        ((ImplUnicodeToIso2022KrContext *) pContext)->m_nHighSurrogate
            = nHighSurrogate;
        ((ImplUnicodeToIso2022KrContext *) pContext)->m_eSet = eSet;
    }
    if (pInfo)
        *pInfo = nInfo;
    if (pSrcCvtChars)
        *pSrcCvtChars = nConverted;

    return pDestBufPtr - pDestBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
