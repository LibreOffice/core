/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
            **ppDestBuf = (char)(sal_uChar)(c-RTL_TEXTCVT_BYTE_PRIVATE_START);
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
