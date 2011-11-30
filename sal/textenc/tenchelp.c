/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

