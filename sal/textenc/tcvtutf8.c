/*************************************************************************
 *
 *  $RCSfile: tcvtutf8.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:58:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sal/types.h"
#include "rtl/alloc.h"
#include "rtl/textcvt.h"

#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"

struct ImplUtf8ToUnicodeContext
{
    sal_uInt32 nUtf32;
    int nShift;
    sal_Bool bCheckBom;
};

struct ImplUnicodeToUtf8Context
{
    sal_Unicode nHighSurrogate; /* 0xFFFF: write BOM */
};

void * ImplCreateUtf8ToUnicodeContext(void)
{
    void * p = rtl_allocateMemory(sizeof (struct ImplUtf8ToUnicodeContext));
    ImplResetUtf8ToUnicodeContext(p);
    return p;
}

void ImplResetUtf8ToUnicodeContext(void * pContext)
{
    if (pContext != NULL)
    {
        ((struct ImplUtf8ToUnicodeContext *) pContext)->nShift = -1;
        ((struct ImplUtf8ToUnicodeContext *) pContext)->bCheckBom = sal_True;
    }
}

sal_Size ImplConvertUtf8ToUnicode(ImplTextConverterData const * pData,
                                  void * pContext, sal_Char const * pSrcBuf,
                                  sal_Size nSrcBytes, sal_Unicode * pDestBuf,
                                  sal_Size nDestChars, sal_uInt32 nFlags,
                                  sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes)
{
    /*
       This function is very liberal with the UTF-8 input.  Accepted are:
       - non-shortest forms (e.g., C0 41 instead of 41 to represent U+0041)
       - surrogates (e.g., ED A0 80 to represent U+D800)
       - encodings with up to six bytes (everything outside the range
         U+0000..10FFFF is considered "undefined")
       The first two of these points allow this routine to translate from both
       RTL_TEXTENCODING_UTF8 and RTL_TEXTENCODING_JAVA_UTF8.
      */

    sal_Bool bJavaUtf8 = pData != NULL;
    sal_uInt32 nUtf32 = 0;
    int nShift = -1;
    sal_Bool bCheckBom = sal_True;
    sal_uInt32 nInfo = 0;
    sal_uChar const * pSrcBufPtr = (sal_uChar const *) pSrcBuf;
    sal_uChar const * pSrcBufEnd = pSrcBufPtr + nSrcBytes;
    sal_Unicode * pDestBufPtr = pDestBuf;
    sal_Unicode * pDestBufEnd = pDestBufPtr + nDestChars;

    if (pContext != NULL)
    {
        nUtf32 = ((struct ImplUtf8ToUnicodeContext *) pContext)->nUtf32;
        nShift = ((struct ImplUtf8ToUnicodeContext *) pContext)->nShift;
        bCheckBom = ((struct ImplUtf8ToUnicodeContext *) pContext)->bCheckBom;
    }

    while (pSrcBufPtr < pSrcBufEnd)
    {
        sal_Bool bUndefined = sal_False;
        sal_Bool bConsume = sal_True;
        sal_uInt32 nChar = *pSrcBufPtr++;
        if (nShift < 0)
            if (nChar <= 0x7F)
            {
                nUtf32 = nChar;
                goto transform;
            }
            else if (nChar <= 0xBF)
                goto bad_input;
            else if (nChar <= 0xDF)
            {
                nUtf32 = (nChar & 0x1F) << 6;
                nShift = 0;
            }
            else if (nChar <= 0xEF)
            {
                nUtf32 = (nChar & 0x0F) << 12;
                nShift = 6;
            }
            else if (nChar <= 0xF7)
            {
                nUtf32 = (nChar & 0x07) << 18;
                nShift = 12;
            }
            else if (nChar <= 0xFB)
            {
                nUtf32 = (nChar & 0x03) << 24;
                nShift = 18;
            }
            else if (nChar <= 0xFD)
            {
                nUtf32 = (nChar & 0x01) << 30;
                nShift = 24;
            }
            else
                goto bad_input;
        else if ((nChar & 0xC0) == 0x80)
        {
            nUtf32 |= (nChar & 0x3F) << nShift;
            if (nShift == 0)
                goto transform;
            else
                nShift -= 6;
        }
        else
        {
            /*
             This byte is preceeded by a broken UTF-8 sequence; if this byte
             is neither in the range [0x80..0xBF] nor in the range
             [0xFE..0xFF], assume that this byte does not belong to that
             broken sequence, but instead starts a new, legal UTF-8 sequence:
             */
            bConsume = nChar >= 0xFE;
            goto bad_input;
        }
        continue;

    transform:
        if (!bCheckBom || nUtf32 != 0xFEFF
            || (nFlags & RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE) == 0
            || bJavaUtf8)
        {
            if (nUtf32 <= 0xFFFF)
                if (pDestBufPtr != pDestBufEnd)
                    *pDestBufPtr++ = (sal_Unicode) nUtf32;
                else
                    goto no_output;
            else if (nUtf32 <= 0x10FFFF)
                if (pDestBufEnd - pDestBufPtr >= 2)
                {
                    *pDestBufPtr++ = (sal_Unicode) ImplGetHighSurrogate(nUtf32);
                    *pDestBufPtr++ = (sal_Unicode) ImplGetLowSurrogate(nUtf32);
                }
                else
                    goto no_output;
            else
            {
                bUndefined = sal_True;
                goto bad_input;
            }
        }
        nShift = -1;
        bCheckBom = sal_False;
        continue;

    bad_input:
        switch (ImplHandleBadInputMbTextToUnicodeConversion(bUndefined, nFlags,
                                                            &pDestBufPtr,
                                                            pDestBufEnd,
                                                            &nInfo))
        {
        case IMPL_BAD_INPUT_STOP:
            nShift = -1;
            bCheckBom = sal_False;
            if (!bConsume)
                --pSrcBufPtr;
            break;

        case IMPL_BAD_INPUT_CONTINUE:
            nShift = -1;
            bCheckBom = sal_False;
            if (!bConsume)
                --pSrcBufPtr;
            continue;

        case IMPL_BAD_INPUT_NO_OUTPUT:
            goto no_output;
        }
        break;

    no_output:
        --pSrcBufPtr;
        nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
        break;
    }

    if (nShift >= 0
        && (nInfo & (RTL_TEXTTOUNICODE_INFO_ERROR
                         | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL))
               == 0)
    {
        if ((nFlags & RTL_TEXTTOUNICODE_FLAGS_FLUSH) == 0)
            nInfo |= RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
        else
            switch (ImplHandleBadInputMbTextToUnicodeConversion(sal_False,
                                                                nFlags,
                                                                &pDestBufPtr,
                                                                pDestBufEnd,
                                                                &nInfo))
            {
            case IMPL_BAD_INPUT_STOP:
            case IMPL_BAD_INPUT_CONTINUE:
                nShift = -1;
                bCheckBom = sal_False;
                break;

            case IMPL_BAD_INPUT_NO_OUTPUT:
                nInfo |= RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }
    }

    if (pContext != NULL)
    {
        ((struct ImplUtf8ToUnicodeContext *) pContext)->nUtf32 = nUtf32;
        ((struct ImplUtf8ToUnicodeContext *) pContext)->nShift = nShift;
        ((struct ImplUtf8ToUnicodeContext *) pContext)->bCheckBom = bCheckBom;
    }
    if (pInfo != NULL)
        *pInfo = nInfo;
    if (pSrcCvtBytes != NULL)
        *pSrcCvtBytes = (sal_Char const *) pSrcBufPtr - pSrcBuf;
    return pDestBufPtr - pDestBuf;
}

void * ImplCreateUnicodeToUtf8Context(void)
{
    void * p = rtl_allocateMemory(sizeof (struct ImplUnicodeToUtf8Context));
    ImplResetUnicodeToUtf8Context(p);
    return p;
}

void ImplResetUnicodeToUtf8Context(void * pContext)
{
    if (pContext != NULL)
        ((struct ImplUnicodeToUtf8Context *) pContext)->nHighSurrogate = 0xFFFF;
}

sal_Size ImplConvertUnicodeToUtf8(ImplTextConverterData const * pData,
                                  void * pContext, sal_Unicode const * pSrcBuf,
                                  sal_Size nSrcChars, sal_Char * pDestBuf,
                                  sal_Size nDestBytes, sal_uInt32 nFlags,
                                  sal_uInt32 * pInfo, sal_Size* pSrcCvtChars)
{
    sal_Bool bJavaUtf8 = pData != NULL;
    sal_Unicode nHighSurrogate = 0xFFFF;
    sal_uInt32 nInfo = 0;
    sal_Unicode const * pSrcBufPtr = pSrcBuf;
    sal_Unicode const * pSrcBufEnd = pSrcBufPtr + nSrcChars;
    sal_Char * pDestBufPtr = pDestBuf;
    sal_Char * pDestBufEnd = pDestBufPtr + nDestBytes;

    if (pContext != NULL)
        nHighSurrogate
            = ((struct ImplUnicodeToUtf8Context *) pContext)->nHighSurrogate;

    if (nHighSurrogate == 0xFFFF)
    {
        if ((nFlags & RTL_UNICODETOTEXT_FLAGS_GLOBAL_SIGNATURE) != 0
            && !bJavaUtf8)
        {
            if (pDestBufEnd - pDestBufPtr >= 3)
            {
                /* Write BOM (U+FEFF) as UTF-8: */
                *pDestBufPtr++ = (sal_Char) 0xEF;
                *pDestBufPtr++ = (sal_Char) 0xBB;
                *pDestBufPtr++ = (sal_Char) 0xBF;
            }
            else
            {
                nInfo |= RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                goto done;
            }
        }
        nHighSurrogate = 0;
    }

    while (pSrcBufPtr < pSrcBufEnd)
    {
        sal_uInt32 nChar = *pSrcBufPtr++;
        if (nHighSurrogate == 0)
        {
            if (ImplIsHighSurrogate(nChar) && !bJavaUtf8)
            {
                nHighSurrogate = (sal_Unicode) nChar;
                continue;
            }
        }
        else if (ImplIsLowSurrogate(nChar) && !bJavaUtf8)
            nChar = ImplCombineSurrogates(nHighSurrogate, nChar);
        else
            goto bad_input;

        if ((ImplIsLowSurrogate(nChar) && !bJavaUtf8)
            || ImplIsNoncharacter(nChar))
            goto bad_input;

        if (nChar <= 0x7F && (!bJavaUtf8 || nChar != 0))
            if (pDestBufPtr != pDestBufEnd)
                *pDestBufPtr++ = (sal_Char) nChar;
            else
                goto no_output;
        else if (nChar <= 0x7FF)
            if (pDestBufEnd - pDestBufPtr >= 2)
            {
                *pDestBufPtr++ = (sal_Char) (0xC0 | (nChar >> 6));
                *pDestBufPtr++ = (sal_Char) (0x80 | (nChar & 0x3F));
            }
            else
                goto no_output;
        else if (nChar <= 0xFFFF)
            if (pDestBufEnd - pDestBufPtr >= 3)
            {
                *pDestBufPtr++ = (sal_Char) (0xE0 | (nChar >> 12));
                *pDestBufPtr++ = (sal_Char) (0x80 | ((nChar >> 6) & 0x3F));
                *pDestBufPtr++ = (sal_Char) (0x80 | (nChar & 0x3F));
            }
            else
                goto no_output;
        else if (pDestBufEnd - pDestBufPtr >= 4)
        {
            *pDestBufPtr++ = (sal_Char) (0xF0 | (nChar >> 18));
            *pDestBufPtr++ = (sal_Char) (0x80 | ((nChar >> 12) & 0x3F));
            *pDestBufPtr++ = (sal_Char) (0x80 | ((nChar >> 6) & 0x3F));
            *pDestBufPtr++ = (sal_Char) (0x80 | (nChar & 0x3F));
        }
        else
            goto no_output;
        nHighSurrogate = 0;
        continue;

    bad_input:
        switch (ImplHandleBadInputUnicodeToTextConversion(sal_False, 0, nFlags,
                                                          &pDestBufPtr,
                                                          pDestBufEnd, &nInfo,
                                                          NULL, 0, NULL))
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
        --pSrcBufPtr;
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
            switch (ImplHandleBadInputUnicodeToTextConversion(sal_False, 0,
                                                              nFlags,
                                                              &pDestBufPtr,
                                                              pDestBufEnd,
                                                              &nInfo, NULL, 0,
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

 done:
    if (pContext != NULL)
        ((struct ImplUnicodeToUtf8Context *) pContext)->nHighSurrogate
            = nHighSurrogate;
    if (pInfo != NULL)
        *pInfo = nInfo;
    if (pSrcCvtChars != NULL)
        *pSrcCvtChars = pSrcBufPtr - pSrcBuf;
    return pDestBufPtr - pDestBuf;
}
