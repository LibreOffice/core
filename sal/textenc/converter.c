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

#include "converter.h"
#include "tenchelp.h"
#include "unichars.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

ImplBadInputConversionAction ImplHandleBadInputTextToUnicodeConversion(
    sal_Bool bUndefined, sal_Bool bMultiByte, sal_Char cByte, sal_uInt32 nFlags,
    sal_Unicode ** pDestBufPtr, sal_Unicode * pDestBufEnd, sal_uInt32 * pInfo)
{
    *pInfo |= bUndefined
        ? (bMultiByte
           ? RTL_TEXTTOUNICODE_INFO_MBUNDEFINED
           : RTL_TEXTTOUNICODE_INFO_UNDEFINED)
        : RTL_TEXTTOUNICODE_INFO_INVALID;
    switch (nFlags
            & (bUndefined
               ? (bMultiByte
                  ? RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK
                  : RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK)
               : RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK))
    {
    case RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR:
    case RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR:
    case RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR:
        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
        return IMPL_BAD_INPUT_STOP;

    case RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE:
    case RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE:
    case RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE:
        return IMPL_BAD_INPUT_CONTINUE;

    case RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE:
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = RTL_TEXTCVT_BYTE_PRIVATE_START
                | ((sal_uChar) cByte);
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;

    default: /* RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT,
                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT,
                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT */
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;
    }
}

ImplBadInputConversionAction
ImplHandleBadInputUnicodeToTextConversion(sal_Bool bUndefined,
                                          sal_uInt32 nUtf32,
                                          sal_uInt32 nFlags,
                                          sal_Char ** pDestBufPtr,
                                          sal_Char * pDestBufEnd,
                                          sal_uInt32 * pInfo,
                                          sal_Char const * pPrefix,
                                          sal_Size nPrefixLen,
                                          sal_Bool * pPrefixWritten)
{
    /* TODO! RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE
             RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR */

    sal_Char cReplace;

    if (bUndefined)
    {
        if (ImplIsControlOrFormat(nUtf32))
        {
            if ((nFlags & RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE) != 0)
                nFlags = RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE;
        }
        else if (ImplIsPrivateUse(nUtf32))
        {
            if ((nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_IGNORE) != 0)
                nFlags = RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE;
            else if ((nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0) != 0)
                nFlags = RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0;
        }
        else if (ImplIsZeroWidth(nUtf32))
        {
            if ((nFlags & RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE) != 0)
                nFlags = RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE;
        }
    }
    *pInfo |= bUndefined ? RTL_UNICODETOTEXT_INFO_UNDEFINED :
                           RTL_UNICODETOTEXT_INFO_INVALID;
    switch (nFlags & (bUndefined ? RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK :
                                   RTL_UNICODETOTEXT_FLAGS_INVALID_MASK))
    {
    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR:
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
        return IMPL_BAD_INPUT_STOP;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE:
        if (pPrefixWritten)
            *pPrefixWritten = sal_False;
        return IMPL_BAD_INPUT_CONTINUE;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_0:
        cReplace = 0;
        break;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK:
    default: /* RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT,
                RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT */
        cReplace = '?';
        break;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE:
        cReplace = '_';
        break;
    }
    if ((sal_Size) (pDestBufEnd - *pDestBufPtr) > nPrefixLen)
    {
        while (nPrefixLen-- > 0)
            *(*pDestBufPtr)++ = *pPrefix++;
        *(*pDestBufPtr)++ = cReplace;
        if (pPrefixWritten)
            *pPrefixWritten = sal_True;
        return IMPL_BAD_INPUT_CONTINUE;
    }
    else
        return IMPL_BAD_INPUT_NO_OUTPUT;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
