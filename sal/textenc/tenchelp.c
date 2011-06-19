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

#include "tenchelp.h"
#include "unichars.h"
#include "rtl/textcvt.h"
#include "sal/types.h"

static sal_Bool ImplGetUndefinedAsciiMultiByte(sal_uInt32 nFlags,
                                               sal_Char * pBuf,
                                               sal_Size nMaxLen);

static sal_Bool ImplGetInvalidAsciiMultiByte(sal_uInt32 nFlags,
                                             sal_Char * pBuf,
                                             sal_Size nMaxLen);

static int ImplIsUnicodeIgnoreChar(sal_Unicode c, sal_uInt32 nFlags);

sal_Bool ImplGetUndefinedAsciiMultiByte(sal_uInt32 nFlags,
                                        sal_Char * pBuf,
                                        sal_Size nMaxLen)
{
    if (nMaxLen == 0)
        return sal_False;
    switch (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK)
    {
    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0:
        *pBuf = 0x00;
        break;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK:
    default: /* RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT */
        *pBuf = 0x3F;
        break;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE:
        *pBuf = 0x5F;
        break;
    }
    return sal_True;
}

sal_Bool ImplGetInvalidAsciiMultiByte(sal_uInt32 nFlags,
                                      sal_Char * pBuf,
                                      sal_Size nMaxLen)
{
    if (nMaxLen == 0)
        return sal_False;
    switch (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK)
    {
    case RTL_UNICODETOTEXT_FLAGS_INVALID_0:
        *pBuf = 0x00;
        break;

    case RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK:
    default: /* RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT */
        *pBuf = 0x3F;
        break;

    case RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE:
        *pBuf = 0x5F;
        break;
    }
    return sal_True;
}

int ImplIsUnicodeIgnoreChar( sal_Unicode c, sal_uInt32 nFlags )
{
    return
        ((nFlags & RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE) != 0
         && ImplIsZeroWidth(c))
        || ((nFlags & RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE) != 0
            && ImplIsControlOrFormat(c))
        || ((nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_IGNORE) != 0
            && ImplIsPrivateUse(c));
}

/* ======================================================================= */

sal_Unicode ImplGetUndefinedUnicodeChar(sal_uChar cChar, sal_uInt32 nFlags)
{
    return ((nFlags & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK)
                   == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE) ?
               RTL_TEXTCVT_BYTE_PRIVATE_START + cChar :
               RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
}

/* ----------------------------------------------------------------------- */

sal_Bool
ImplHandleUndefinedUnicodeToTextChar(ImplTextConverterData const * pData,
                                     sal_Unicode const ** ppSrcBuf,
                                     sal_Unicode const * pEndSrcBuf,
                                     sal_Char ** ppDestBuf,
                                     sal_Char const * pEndDestBuf,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo)
{
    sal_Unicode c = **ppSrcBuf;

    (void) pData; /* unused */

    /* Should the private character map to one byte */
    if ( (c >= RTL_TEXTCVT_BYTE_PRIVATE_START) && (c <= RTL_TEXTCVT_BYTE_PRIVATE_END) )
    {
        if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 )
        {
            **ppDestBuf = (sal_Char)(sal_uChar)(c-RTL_TEXTCVT_BYTE_PRIVATE_START);
            (*ppDestBuf)++;
            (*ppSrcBuf)++;
            return sal_True;
        }
    }

    /* Should this character ignored (Private, Non Spacing, Control) */
    if ( ImplIsUnicodeIgnoreChar( c, nFlags ) )
    {
        (*ppSrcBuf)++;
        return sal_True;
    }

    /* Surrogates Characters should result in */
    /* one replacement character */
    if (ImplIsHighSurrogate(c))
    {
        if ( *ppSrcBuf == pEndSrcBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
            return sal_False;
        }

        c = *((*ppSrcBuf)+1);
        if (ImplIsLowSurrogate(c))
            (*ppSrcBuf)++;
        else
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_INVALID;
            if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
                return sal_False;
            }
            else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE )
            {
                (*ppSrcBuf)++;
                return sal_True;
            }
            else if (ImplGetInvalidAsciiMultiByte(nFlags,
                                                  *ppDestBuf,
                                                  pEndDestBuf - *ppDestBuf))
            {
                ++*ppSrcBuf;
                ++*ppDestBuf;
                return sal_True;
            }
            else
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR
                              | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                return sal_False;
            }
        }
    }

    *pInfo |= RTL_UNICODETOTEXT_INFO_UNDEFINED;
    if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR )
    {
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
        return sal_False;
    }
    else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE )
        (*ppSrcBuf)++;
    else if (ImplGetUndefinedAsciiMultiByte(nFlags,
                                            *ppDestBuf,
                                            pEndDestBuf - *ppDestBuf))
    {
        ++*ppSrcBuf;
        ++*ppDestBuf;
    }
    else
    {
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR
                      | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
        return sal_False;
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
