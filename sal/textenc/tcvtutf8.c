/*************************************************************************
 *
 *  $RCSfile: tcvtutf8.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2001-10-17 14:35:30 $
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

#ifndef INCLUDED_RTL_TEXTENC_TENCHELP_H
#include "tenchelp.h"
#endif
#ifndef INCLUDED_RTL_TEXTENC_UNICHARS_H
#include "unichars.h"
#endif

#ifndef _RTL_TEXTCVT_H
#include "rtl/textcvt.h"
#endif

/* ----------------------------------------------------------------------- */

sal_Size ImplUTF8ToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    static sal_uInt8 const nExtraBytesFromUTF8Tab[16] =
    {
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5, 0, 0
    };
    static sal_uInt8 const nFirstByteMaskTab[3] =
    {
        0x07, 0x03, 0x01
    };

    sal_uInt8               nBytes;
    sal_uInt8               nTempBytes;
    sal_uChar               c;
    sal_uInt32              cConv;
    sal_Unicode*            pEndDestBuf;
    const sal_Char*         pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    while ( pSrcBuf < pEndSrcBuf )
    {
        if ( pDestBuf == pEndDestBuf )
        {
            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
            break;
        }

        c = (sal_uChar)*pSrcBuf;

        /* 1 Byte */
        /* 0aaaaaaa (000000000aaaaaaa) */
        if ( !(c & 0x80) )
        {
            *pDestBuf = (sal_Unicode)c;
            pDestBuf++;
            pSrcBuf++;
        }
        /* 2-3 Bytes */
        else if ( (c & 0xF0) != 0xF0 )
        {
            /* 110aaaaa 10bbbbbb (00000aaaaabbbbbb) */
            if ( (c & 0xE0) == 0xC0 )
            {
                nBytes = 2;
                c &= 0x1F; /* 00001111; */
            }
            /* 1110aaaa 10bbbbbb 10cccccc (aaaabbbbbbcccccc) */
            else if ( (c & 0xF0) == 0xE0 )
            {
                nBytes = 3;
                c &= 0x0F; /* 00001111; */
            }
            else
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) != RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                {
                    if ( pDestBuf >= pEndDestBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                        break;
                    }
                    *pDestBuf++ = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                }
                pSrcBuf++;
                continue;
            }

            if ( pSrcBuf+nBytes > pEndSrcBuf )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                break;
            }

            cConv = c;
            pSrcBuf++;
            if ( (*pSrcBuf & 0xC0) != 0x80 )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                {
                    pSrcBuf--;
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) != RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                    *pDestBuf++ = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                continue;
            }
            else
            {
                c = (sal_uChar)*pSrcBuf;
                cConv <<= 6;
                cConv += c & 0x3F; /* 00111111 */
            }
            if ( nBytes == 3 )
            {
                pSrcBuf++;
                if ( (*pSrcBuf & 0xC0) != 0x80 )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                    if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                    {
                        pSrcBuf -= 2;
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                        break;
                    }
                    else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) != RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                        *pDestBuf++
                            = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                    continue;
                }
                else
                {
                    c = (sal_uChar)*pSrcBuf;
                    cConv <<= 6;
                    cConv += c & 0x3F; /* 00111111 */
                }
            }
            *pDestBuf = (sal_Unicode)cConv;
            pDestBuf++;
            pSrcBuf++;
        }
        /* 4-6 Bytes */
        else
        {
            /* convert to ucs4 */
            nBytes = nExtraBytesFromUTF8Tab[c & 0x0F];
            if ( !nBytes )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) != RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                {
                    if ( pDestBuf >= pEndDestBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                        break;
                    }
                    *pDestBuf++ = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                }
                pSrcBuf++;
                continue;
            }
            else if ( pSrcBuf+nBytes+1 > pEndSrcBuf )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                break;
            }

            cConv = c & nFirstByteMaskTab[nBytes-3];
            nTempBytes = nBytes;
            do
            {
                pSrcBuf++;
                if ( (*pSrcBuf & 0xC0) != 0x80 )
                    break;
                c = (sal_uChar)*pSrcBuf;
                cConv <<= 6;
                cConv += c & 0x3F; /* 00111111 */
                nTempBytes--;
            }
            while ( nTempBytes );
            if ( nTempBytes )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                {
                    pSrcBuf -= nBytes-nTempBytes+1;
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                    break;
                }
                else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) != RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE )
                    *pDestBuf++ = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                pSrcBuf++;
                continue;
            }
            else
            {
                pSrcBuf++;
                if ( cConv > 0x10FFFF )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_MBUNDEFINED;
                    if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) == RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                        break;
                    }
                    else if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK) != RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE )
                        *pDestBuf++
                            = RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
                }
                else if ( pDestBuf+2 > pEndDestBuf )
                {
                    *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                    break;
                }
                else
                {
                    *pDestBuf++ = (sal_Unicode) ImplGetHighSurrogate(cConv);
                    *pDestBuf++ = (sal_Unicode) ImplGetLowSurrogate(cConv);
                }
            }
        }
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToUTF8( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars )
{
    static sal_uInt8 const nFirstByteMarkTab[6] =
    {
        0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
    };

    sal_Unicode             c;
    sal_Unicode             c2;
    sal_uInt32              nUCS4Char;
    sal_uInt8               nBytes;
    sal_Char*               pTempDestBuf;
    sal_Char*               pEndDestBuf;
    const sal_Unicode*      pEndSrcBuf;

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    while ( pSrcBuf < pEndSrcBuf )
    {
        c = *pSrcBuf;
        if ( c < 0x80 )
        {
            if ( pDestBuf == pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            *pDestBuf = (sal_Char)(sal_uChar)c;
            pDestBuf++;
            pSrcBuf++;
        }
        else
        {
            nUCS4Char = c;
            if ( nUCS4Char < 0x800 )
                nBytes = 2;
            else
            {
                if (ImplIsHighSurrogate(c))
                {
                    if ( pSrcBuf == pEndSrcBuf )
                    {
                        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
                        break;
                    }

                    c2 = *(pSrcBuf+1);
                    if (ImplIsLowSurrogate(c2))
                    {
                        nUCS4Char = ImplCombineSurrogates(c, c2);
                        pSrcBuf++;
                    }
                    else
                    {
                        *pInfo |= RTL_UNICODETOTEXT_INFO_INVALID;
                        if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR )
                        {
                            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
                            break;
                        }
                        else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE )
                        {
                            pSrcBuf++;
                            continue;
                        }
                        /* in UTF8 we save the original code. I think   */
                        /* this is better than the default char,        */
                        /* because it is a unicode format.              */
                    }
                }

                if ( nUCS4Char < 0x10000 )
                    nBytes = 3;
                else if ( nUCS4Char < 0x200000 )
                    nBytes = 4;
                else if ( nUCS4Char < 0x4000000 )
                    nBytes = 5;
                else
                    nBytes = 6;
            }

            if ( pDestBuf+nBytes > pEndDestBuf )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }
            pDestBuf += nBytes;
            pTempDestBuf = pDestBuf;
            switch ( nBytes ) /* no breaks, only jump table */
            {
                case 6: pTempDestBuf--; *pTempDestBuf = (sal_Char)((((sal_uChar)nUCS4Char) | 0x80) & 0xBF); nUCS4Char >>= 6;
                case 5: pTempDestBuf--; *pTempDestBuf = (sal_Char)((((sal_uChar)nUCS4Char) | 0x80) & 0xBF); nUCS4Char >>= 6;
                case 4: pTempDestBuf--; *pTempDestBuf = (sal_Char)((((sal_uChar)nUCS4Char) | 0x80) & 0xBF); nUCS4Char >>= 6;
                case 3: pTempDestBuf--; *pTempDestBuf = (sal_Char)((((sal_uChar)nUCS4Char) | 0x80) & 0xBF); nUCS4Char >>= 6;
                case 2: pTempDestBuf--; *pTempDestBuf = (sal_Char)((((sal_uChar)nUCS4Char) | 0x80) & 0xBF); nUCS4Char >>= 6;
            };
            pTempDestBuf--;
            *pTempDestBuf = (sal_Char)(((sal_uChar)nUCS4Char) | nFirstByteMarkTab[nBytes-1]);
            pSrcBuf++;
        }
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}
