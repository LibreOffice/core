/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "rtl/textcvt.h"
#include "sal/types.h"

#include "handleundefinedunicodetotextchar.hxx"
#include "tenchelp.hxx"

namespace {

bool ImplIsUnicodeIgnoreChar(sal_Unicode c, sal_uInt32 nFlags)
{
    return
        ((nFlags & RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE) != 0
         && ImplIsZeroWidth(c))
        || ((nFlags & RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE) != 0
            && ImplIsControlOrFormat(c))
        || ((nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_IGNORE) != 0
            && ImplIsPrivateUse(c));
}

bool ImplGetUndefinedAsciiMultiByte(sal_uInt32 nFlags,
                                        char * pBuf,
                                        sal_Size nMaxLen)
{
    if (nMaxLen == 0)
        return false;
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
    return true;
}

bool ImplGetInvalidAsciiMultiByte(sal_uInt32 nFlags,
                                      char * pBuf,
                                      sal_Size nMaxLen)
{
    if (nMaxLen == 0)
        return false;
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
    return true;
}

}

bool sal::detail::textenc::handleUndefinedUnicodeToTextChar(
    sal_Unicode const ** ppSrcBuf, sal_Unicode const * pEndSrcBuf,
    char ** ppDestBuf, char const * pEndDestBuf, sal_uInt32 nFlags,
    sal_uInt32 * pInfo)
{
    sal_Unicode c = **ppSrcBuf;

    /* Should the private character map to one byte */
    if ( (c >= RTL_TEXTCVT_BYTE_PRIVATE_START) && (c <= RTL_TEXTCVT_BYTE_PRIVATE_END) )
    {
        if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 )
        {
            **ppDestBuf = (char)(unsigned char)(c-RTL_TEXTCVT_BYTE_PRIVATE_START);
            (*ppDestBuf)++;
            (*ppSrcBuf)++;
            return true;
        }
    }

    /* Should this character ignored (Private, Non Spacing, Control) */
    if ( ImplIsUnicodeIgnoreChar( c, nFlags ) )
    {
        (*ppSrcBuf)++;
        return true;
    }

    /* Surrogates Characters should result in */
    /* one replacement character */
    if (ImplIsHighSurrogate(c))
    {
        if ( *ppSrcBuf == pEndSrcBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
            return false;
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
                return false;
            }
            else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE )
            {
                (*ppSrcBuf)++;
                return true;
            }
            else if (ImplGetInvalidAsciiMultiByte(nFlags,
                                                  *ppDestBuf,
                                                  pEndDestBuf - *ppDestBuf))
            {
                ++*ppSrcBuf;
                ++*ppDestBuf;
                return true;
            }
            else
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR
                              | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                return false;
            }
        }
    }

    *pInfo |= RTL_UNICODETOTEXT_INFO_UNDEFINED;
    if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR )
    {
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
        return false;
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
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
