/*************************************************************************
 *
 *  $RCSfile: tcvtutf7.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:30 $
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

#define _RTL_CCVTUTF7_C

#ifndef _RTL_ALLOC_H
#include <rtl/alloc.h>
#endif

#ifndef _RTL_TENCHELP_H
#include <tenchelp.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

/* ======================================================================= */

static sal_uChar const aImplBase64Tab[64] =
{
    /* A-Z */
          0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A,
    /* a-z */
          0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A,
    /* 0-9,+,/ */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x2B, 0x2F
};

/* Index in Base64Tab or 0xFF, when is a invalid character */
static sal_uChar const aImplBase64IndexTab[128] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x00-0x07 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x08-0x0F */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x10-0x17 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x18-0x1F */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x20-0x27  !"#$%&' */
    0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,     /* 0x28-0x2F ()*+,-./ */
      52,   53,   54,   55,   56,   57,   58,   59,     /* 0x30-0x37 01234567 */
      60,   61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x38-0x3F 89:;<=>? */
    0xFF,    0,    1,    2,    3,    4,    5,    6,     /* 0x40-0x47 @ABCDEFG */
       7,    8,    9,   10,   11,   12,   13,   14,     /* 0x48-0x4F HIJKLMNO */
      15,   16,   17,   18,   19,   20,   21,   22,     /* 0x50-0x57 PQRSTUVW */
      23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,     /* 0x58-0x5F XYZ[\]^_ */
    0xFF,   26,   27,   28,   29,   30,   31,   32,     /* 0x60-0x67 `abcdefg */
      33,   34,   35,   36,   37,   38,   39,   40,     /* 0x68-0x6F hijklmno */
      41,   42,   43,   44,   45,   46,   47,   48,     /* 0x70-0x77 pqrstuvw */
      49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF      /* 0x78-0x7F xyz{|}~ */
};

static sal_uChar const aImplMustShiftTab[128] =
{
    1, 1, 1, 1, 1, 1, 1, 1,     /* 0x00-0x07 */
    1, 0, 0, 1, 0, 1, 1, 1,     /* 0x08-0x0F 0x09 == HTAB, 0x0A == LF 0x0C == CR */
    1, 1, 1, 1, 1, 1, 1, 1,     /* 0x10-0x17 */
    1, 1, 1, 1, 1, 1, 1, 1,     /* 0x18-0x1F */
    0, 1, 1, 1, 1, 1, 1, 0,     /* 0x20-0x27  !"#$%&' */
    0, 0, 1, 1, 0, 1, 0, 0,     /* 0x28-0x2F ()*+,-./ */
    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x30-0x37 01234567 */
    0, 0, 0, 1, 1, 1, 1, 0,     /* 0x38-0x3F 89:;<=>? */
    1, 0, 0, 0, 0, 0, 0, 0,     /* 0x40-0x47 @ABCDEFG */
    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x48-0x4F HIJKLMNO */
    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x50-0x57 PQRSTUVW */
    0, 0, 0, 1, 1, 1, 1, 1,     /* 0x58-0x5F XYZ[\]^_ */
    1, 0, 0, 0, 0, 0, 0, 0,     /* 0x60-0x67 `abcdefg */
    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x68-0x6F hijklmno */
    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x70-0x77 pqrstuvw */
    0, 0, 0, 1, 1, 1, 1, 1      /* 0x78-0x7F xyz{|}~ */
};

/* + */
#define IMPL_SHIFT_IN_CHAR      0x2B
/* - */
#define IMPL_SHIFT_OUT_CHAR     0x2D

/* ----------------------------------------------------------------------- */

typedef struct _ImplUTF7ToUCContextData
{
    int                     mbShifted;
    int                     mbFirst;
    int                     mbWroteOne;
    sal_uInt32              mnBitBuffer;
    sal_uInt32              mnBufferBits;
} ImplUTF7ToUCContextData;

/* ----------------------------------------------------------------------- */

void* ImplUTF7CreateUTF7TextToUnicodeContext( void )
{
    ImplUTF7ToUCContextData* pContextData;
    pContextData = (ImplUTF7ToUCContextData*)rtl_allocateMemory( sizeof( ImplUTF7ToUCContextData ) );
    pContextData->mbShifted         = sal_False;
    pContextData->mbFirst           = sal_False;
    pContextData->mbWroteOne        = sal_False;
    pContextData->mnBitBuffer       = 0;
    pContextData->mnBufferBits      = 0;
    return (void*)pContextData;
}

/* ----------------------------------------------------------------------- */

void ImplUTF7DestroyTextToUnicodeContext( void* pContext )
{
    rtl_freeMemory( pContext );
}

/* ----------------------------------------------------------------------- */

void ImplUTF7ResetTextToUnicodeContext( void* pContext )
{
    ImplUTF7ToUCContextData* pContextData = (ImplUTF7ToUCContextData*)pContext;
    pContextData->mbShifted         = sal_False;
    pContextData->mbFirst           = sal_False;
    pContextData->mbWroteOne        = sal_False;
    pContextData->mnBitBuffer       = 0;
    pContextData->mnBufferBits      = 0;
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUTF7ToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes )
{
    ImplUTF7ToUCContextData*    pContextData = (ImplUTF7ToUCContextData*)pContext;
    sal_uChar                   c;
    sal_uChar                   nBase64Value;
    int                         bEnd = sal_False;
    int                         bShifted;
    int                         bFirst;
    int                         bWroteOne;
    int                         bBase64End;
    sal_uInt32                  nBitBuffer;
    sal_uInt32                  nBitBufferTemp;
    sal_uInt32                  nBufferBits;
    sal_Unicode*                pEndDestBuf;
    const sal_Char*             pEndSrcBuf;

/* !!! Implementation not finnished !!!
    if ( pContextData )
    {
        bShifted        = pContextData->mbShifted;
        bFirst          = pContextData->mbFirst;
        bWroteOne       = pContextData->mbWroteOne;
        nBitBuffer      = pContextData->mnBitBuffer;
        nBufferBits     = pContextData->mnBufferBits;
    }
    else
*/
    {
        bShifted        = sal_False;
        bFirst          = sal_False;
        bWroteOne       = sal_False;
        nBitBuffer      = 0;
        nBufferBits     = 0;
    }

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestChars;
    pEndSrcBuf  = pSrcBuf+nSrcBytes;
    do
    {
        if ( pSrcBuf < pEndSrcBuf )
        {
            c = (sal_uChar)*pSrcBuf;

            /* End, when not a base64 character */
            bBase64End = sal_False;
            if ( c <= 0x7F )
            {
                nBase64Value = aImplBase64IndexTab[c];
                if ( nBase64Value == 0xFF )
                    bBase64End = sal_True;
            }
        }
        else
        {
            bEnd = sal_True;
            bBase64End = sal_True;
        }

        if ( bShifted )
        {
            if ( bBase64End )
            {
                bShifted = sal_False;

                /* If the character causing us to drop out was SHIFT_IN */
                /* or SHIFT_OUT, it may be a special escape for SHIFT_IN. */
                /* The test for SHIFT_IN is not necessary, but allows */
                /* an alternate form of UTF-7 where SHIFT_IN is escaped */
                /* by SHIFT_IN. This only works for some values of */
                /* SHIFT_IN. It is so implemented, because this comes */
                /* from the officel unicode book (The Unicode Standard, */
                /* Version 2.0) and so I think, that someone of the */
                /* world has used this feature. */
                if ( !bEnd )
                {
                    if ( (c == IMPL_SHIFT_IN_CHAR) || (c == IMPL_SHIFT_OUT_CHAR) )
                    {
                        /* If no base64 character, and the terminating */
                        /* character of the shift sequence was the */
                        /* SHIFT_OUT_CHAR, then it't a special escape */
                        /* for SHIFT_IN_CHAR. */
                        if ( bFirst && (c == IMPL_SHIFT_OUT_CHAR) )
                        {
                            if ( pDestBuf >= pEndDestBuf )
                            {
                                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                                break;
                            }
                            *pDestBuf = IMPL_SHIFT_IN_CHAR;
                            pDestBuf++;
                            bWroteOne = sal_True;
                        }

                        /* Skip character */
                        pSrcBuf++;
                        if ( pSrcBuf < pEndSrcBuf )
                            c = (sal_uChar)*pSrcBuf;
                        else
                            bEnd = sal_True;
                    }
                }

                /* Empty sequence not allowed, so when we don't write one */
                /* valid char, then the sequence is corrupt */
                if ( !bWroteOne )
                {
                    /* When no more bytes in the source buffer, then */
                    /* this buffer may be to small */
                    if ( bEnd )
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
                    else
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_INVALID;
                        if ( (nFlags & RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK) == RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR )
                        {
                            *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
                            break;
                        }
                        /* We insert here no default char, because I think */
                        /* this is better to ignore this */
                    }
                }
            }
            else
            {
                /* Add 6 Bits from character to the bit buffer */
                nBufferBits += 6;
                nBitBuffer |= ((sal_uInt32)(nBase64Value & 0x3F)) << (32-nBufferBits);
                bFirst = sal_False;
            }

            /* Extract as many full 16 bit characters as possible from the */
            /* bit buffer. */
            while ( (pDestBuf < pEndDestBuf) && (nBufferBits >= 16) )
            {
                nBitBufferTemp = nBitBuffer >> (32-16);
                *pDestBuf = (sal_Unicode)((nBitBufferTemp) & 0xFFFF);
                pDestBuf++;
                nBitBuffer <<= 16;
                nBufferBits -= 16;
                bWroteOne = sal_True;
            }

            if ( nBufferBits >= 16 )
            {
                *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                break;
            }

            if ( bBase64End )
            {
                /* Sequence ended and we have some bits, then the */
                /* sequence is corrupted */
                if ( nBufferBits && nBitBuffer )
                {
                    /* When no more bytes in the source buffer, then */
                    /* this buffer may be to small */
                    if ( bEnd )
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL;
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
                            *pDestBuf = RTL_UNICODE_CHAR_DEFAULT;
                            pDestBuf++;
                        }
                    }

                }

                nBitBuffer = 0;
                nBufferBits = 0;
            }
        }

        if ( !bEnd )
        {
            if ( !bShifted )
            {
                if ( c == IMPL_SHIFT_IN_CHAR )
                {
                    bShifted    = sal_True;
                    bFirst      = sal_True;
                    bWroteOne   = sal_False;
                }
                else
                {
                    /* No direct encoded charcater, then the buffer is */
                    /* corrupt */
                    if ( c > 0x7F )
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
                            *pDestBuf = RTL_UNICODE_CHAR_DEFAULT;
                            pDestBuf++;
                        }
                    }

                    /* Write char to unicode buffer */
                    if ( pDestBuf >= pEndDestBuf )
                    {
                        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR | RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL;
                        break;
                    }
                    *pDestBuf = c;
                    pDestBuf++;
                }
            }

            pSrcBuf++;
        }
    }
    while ( !bEnd );

    if ( pContextData )
    {
        pContextData->mbShifted         = bShifted;
        pContextData->mbFirst           = bFirst;
        pContextData->mbWroteOne        = bWroteOne;
        pContextData->mnBitBuffer       = nBitBuffer;
        pContextData->mnBufferBits      = nBufferBits;
    }

    *pSrcCvtBytes = nSrcBytes - (pEndSrcBuf-pSrcBuf);
    return (nDestChars - (pEndDestBuf-pDestBuf));
}

/* ======================================================================= */

typedef struct _ImplUTF7FromUCContextData
{
    int                     mbShifted;
    sal_uInt32              mnBitBuffer;
    sal_uInt32              mnBufferBits;
} ImplUTF7FromUCContextData;

/* ----------------------------------------------------------------------- */

void* ImplUTF7CreateUnicodeToTextContext( void )
{
    ImplUTF7FromUCContextData* pContextData;
    pContextData = (ImplUTF7FromUCContextData*)rtl_allocateMemory( sizeof( ImplUTF7FromUCContextData ) );
    pContextData->mbShifted         = sal_False;
    pContextData->mnBitBuffer       = 0;
    pContextData->mnBufferBits      = 0;
    return (void*)pContextData;
}

/* ----------------------------------------------------------------------- */

void ImplUTF7DestroyUnicodeToTextContext( void* pContext )
{
    rtl_freeMemory( pContext );
}

/* ----------------------------------------------------------------------- */

void ImplUTF7ResetUnicodeToTextContext( void* pContext )
{
    ImplUTF7FromUCContextData* pContextData = (ImplUTF7FromUCContextData*)pContext;
    pContextData->mbShifted         = sal_False;
    pContextData->mnBitBuffer       = 0;
    pContextData->mnBufferBits      = 0;
}

/* ----------------------------------------------------------------------- */

sal_Size ImplUnicodeToUTF7( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars )
{
    ImplUTF7FromUCContextData*  pContextData = (ImplUTF7FromUCContextData*)pContext;
    sal_Unicode                 c;
    int                         bEnd = sal_False;
    int                         bShifted;
    int                         bNeedShift;
    sal_uInt32                  nBitBuffer;
    sal_uInt32                  nBitBufferTemp;
    sal_uInt32                  nBufferBits;
    sal_Char*                   pEndDestBuf;
    const sal_Unicode*          pEndSrcBuf;

/* !!! Implementation not finnished !!!
    if ( pContextData )
    {
        bShifted        = pContextData->mbShifted;
        nBitBuffer      = pContextData->mnBitBuffer;
        nBufferBits     = pContextData->mnBufferBits;
    }
    else
*/
    {
        bShifted        = sal_False;
        nBitBuffer      = 0;
        nBufferBits     = 0;
    }

    *pInfo = 0;
    pEndDestBuf = pDestBuf+nDestBytes;
    pEndSrcBuf  = pSrcBuf+nSrcChars;
    do
    {
        if ( pSrcBuf < pEndSrcBuf )
        {
            c = *pSrcBuf;

            bNeedShift = (c > 0x7F) || aImplMustShiftTab[c];
            if ( bNeedShift && !bShifted )
            {
                if ( pDestBuf >= pEndDestBuf )
                {
                    *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
                *pDestBuf = IMPL_SHIFT_IN_CHAR;
                pDestBuf++;
                /* Special case handling for SHIFT_IN_CHAR */
                if ( c == IMPL_SHIFT_IN_CHAR )
                {
                    if ( pDestBuf >= pEndDestBuf )
                    {
                        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                        break;
                    }
                    *pDestBuf = IMPL_SHIFT_OUT_CHAR;
                    pDestBuf++;
                }
                else
                    bShifted = sal_True;
            }
        }
        else
        {
            bEnd = sal_True;
            bNeedShift = sal_False;
        }

        if ( bShifted )
        {
            /* Write the character to the bit buffer, or pad the bit */
            /* buffer out to a full base64 character */
            if ( bNeedShift )
            {
                nBufferBits += 16;
                nBitBuffer |= ((sal_uInt32)c) << (32-nBufferBits);
            }
            else
                nBufferBits += (6-(nBufferBits%6))%6;

            /* Flush out as many full base64 characters as possible */
            while ( (pDestBuf < pEndDestBuf) && (nBufferBits >= 6) )
            {
                nBitBufferTemp = nBitBuffer >> (32-6);
                *pDestBuf = aImplBase64Tab[nBitBufferTemp];
                pDestBuf++;
                nBitBuffer <<= 6;
                nBufferBits -= 6;
            }

            if ( nBufferBits >= 6 )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                break;
            }

            /* Write SHIFT_OUT_CHAR, when needed */
            if ( !bNeedShift )
            {
                if ( pDestBuf >= pEndDestBuf )
                {
                    *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
                *pDestBuf = IMPL_SHIFT_OUT_CHAR;
                pDestBuf++;
                bShifted = sal_False;
            }
        }

        if ( !bEnd )
        {
            /* Character can be directly endcoded */
            if ( !bNeedShift )
            {
                if ( pDestBuf >= pEndDestBuf )
                {
                    *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    break;
                }
                *pDestBuf = (sal_Char)(sal_uChar)c;
                pDestBuf++;
            }

            pSrcBuf++;
        }
    }
    while ( !bEnd );

    if ( pContextData )
    {
        pContextData->mbShifted     = bShifted;
        pContextData->mnBitBuffer   = nBitBuffer;
        pContextData->mnBufferBits  = nBufferBits;
    }

    *pSrcCvtChars = nSrcChars - (pEndSrcBuf-pSrcBuf);
    return (nDestBytes - (pEndDestBuf-pDestBuf));
}
