/*************************************************************************
 *
 *  $RCSfile: tenchelp.c,v $
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

#define _RTL_TENCHELP_C

#include <string.h>

#ifndef _RTL_MEMORY_H
#include <rtl/memory.h>
#endif
#ifndef _RTL_TENCHELP_H
#include <tenchelp.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

/* ======================================================================= */

sal_Unicode ImplGetUndefinedUnicodeChar( sal_uChar c, sal_uInt32 nFlags, const ImplTextConverterData* pData )
{
    sal_Unicode nUniChar;

    nFlags &= RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK;
    if ( nFlags == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE )
        nUniChar = 0xF100+c;
    else
    {
        if ( pData )
            nUniChar = pData->mcUnicodeDefChar;
        else
            nUniChar = RTL_UNICODE_CHAR_DEFAULT;
    }

    return nUniChar;
}

/* ----------------------------------------------------------------------- */

sal_Size ImplGetUndefinedAsciiMultiByte( sal_uInt32 nFlags, const ImplTextConverterData* pData, sal_Char* pBuf, sal_Size nMaxLen )
{
    sal_Size nLen = 0;

    if ( nMaxLen )
    {
        switch ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK )
        {
            case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0:
                nLen = 1;
                *pBuf = (sal_Char)0x00;
                break;
            case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK:
                nLen = 1;
                *pBuf = (sal_Char)0x3F;
                break;
            case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE:
                nLen = 1;
                *pBuf = (sal_Char)0x5F;
                break;
            default: /* RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT */
                if ( pData && pData->mpTextDefChar )
                {
                    nLen = strlen( pData->mpTextDefChar );
                    if ( nLen < nMaxLen )
                        rtl_copyMemory( pBuf, pData->mpTextDefChar, nLen );
                    else
                        nLen = 0;
                }
                else
                {
                    nLen = 1;
                    *pBuf = (sal_Char)0x3F;
                }
                break;
        }
    }

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Size ImplGetInvalidAsciiMultiByte( sal_uInt32 nFlags, const ImplTextConverterData* pData, sal_Char* pBuf, sal_Size nMaxLen )
{
    sal_Size nLen = 0;

    if ( nMaxLen )
    {
        switch ( nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK )
        {
            case RTL_UNICODETOTEXT_FLAGS_INVALID_0:
                nLen = 1;
                *pBuf = (sal_Char)0x00;
                break;
            case RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK:
                nLen = 1;
                *pBuf = (sal_Char)0x3F;
                break;
            case RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE:
                nLen = 1;
                *pBuf = (sal_Char)0x5F;
                break;
            default: /* RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT */
                if ( pData && pData->mpTextDefChar )
                {
                    nLen = strlen( pData->mpTextDefChar );
                    if ( nLen < nMaxLen )
                        rtl_copyMemory( pBuf, pData->mpTextDefChar, nLen );
                    else
                        nLen = 0;
                }
                else
                {
                    nLen = 1;
                    *pBuf = (sal_Char)0x3F;
                }
                break;
        }
    }

    return nLen;
}

/* ----------------------------------------------------------------------- */

int ImplHandleUndefinedUnicodeToTextChar( const ImplTextConverterData* pData,
                                          const sal_Unicode** ppSrcBuf, const sal_Unicode* pEndSrcBuf,
                                          sal_Char** ppDestBuf, const sal_Char* pEndDestBuf,
                                          sal_uInt32 nFlags, sal_uInt32* pInfo )
{
    sal_Unicode c = **ppSrcBuf;

    /* Should the private character map to one byte */
    if ( (c >= RTL_TEXTCVT_BYTE_PRIVATE_START) && (c <= RTL_TEXTCVT_BYTE_PRIVATE_END) )
    {
        if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 )
        {
            **ppDestBuf = (sal_Char)(sal_uChar)(c-RTL_TEXTCVT_BYTE_PRIVATE_START);
            (*ppDestBuf)++;
            (*ppSrcBuf)++;
            return IMPL_TEXTCVT_CONTINUE;
        }
    }

    /* Should this character ignored (Private, Non Spacing, Control) */
    if ( ImplIsUnicodeIgnoreChar( c, nFlags ) )
    {
        (*ppSrcBuf)++;
        return IMPL_TEXTCVT_CONTINUE;
    }

    /* Surrogates Characters should result in */
    /* one replacement character */
    if ( (c >= RTL_UNICODE_START_HIGH_SURROGATES) &&
         (c <= RTL_UNICODE_END_HIGH_SURROGATES) )
    {
        if ( *ppSrcBuf == pEndSrcBuf )
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL;
            return IMPL_TEXTCVT_BREAK;
        }

        c = *((*ppSrcBuf)+1);
        if ( (c >= RTL_UNICODE_START_LOW_SURROGATES) &&
             (c <= RTL_UNICODE_END_LOW_SURROGATES) )
            (*ppSrcBuf)++;
        else
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_INVALID;
            if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR )
            {
                *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
                return IMPL_TEXTCVT_BREAK;
            }
            else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_INVALID_MASK) == RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE )
            {
                (*ppSrcBuf)++;
                return IMPL_TEXTCVT_CONTINUE;
            }
            else
            {
                sal_Size nDefLen = ImplGetInvalidAsciiMultiByte( nFlags, pData,
                                                                 *ppDestBuf, pEndDestBuf-*ppDestBuf );
                if ( nDefLen )
                {
                    *ppDestBuf += nDefLen;
                    (*ppSrcBuf)++;
                    return IMPL_TEXTCVT_CONTINUE;
                }
                else
                {
                    *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
                    return IMPL_TEXTCVT_BREAK;
                }
            }
        }
    }

    *pInfo |= RTL_UNICODETOTEXT_INFO_UNDEFINED;
    if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR )
    {
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
        return IMPL_TEXTCVT_BREAK;
    }
    else if ( (nFlags & RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK) == RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE )
        (*ppSrcBuf)++;
    else
    {
        sal_Size nDefLen = ImplGetUndefinedAsciiMultiByte( nFlags, pData,
                                                           *ppDestBuf, pEndDestBuf-*ppDestBuf );
        if ( nDefLen )
        {
            *ppDestBuf += nDefLen;
            (*ppSrcBuf)++;
        }
        else
        {
            *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR | RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL;
            return IMPL_TEXTCVT_BREAK;
        }
    }

    return IMPL_TEXTCVT_CONTINUE;
}

/* ----------------------------------------------------------------------- */

int ImplIsUnicodeIgnoreChar( sal_Unicode c, sal_uInt32 nFlags )
{
    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE )
    {
        /* !!! */
    }

    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE )
    {
        /* !!! */
    }

    if ( nFlags & RTL_UNICODETOTEXT_FLAGS_PRIVATE_IGNORE )
    {
        if ( (c >= 0xE000) && (c <= 0xF8FF) )
            return sal_True;
    }

    return sal_False;
}

/* ----------------------------------------------------------------------- */

typedef struct _ImplReplaceCharData
{
    sal_uInt16      mnUniChar;
    sal_uInt16      mnReplaceChar;
} ImplReplaceCharData;

static ImplReplaceCharData const aImplRepCharTab[] =
{
  { 0x00A0, 0x0020 },   /* NO-BREAK-SPACE */
  { 0x00A1, 0x0021 },   /* INVERTED EXCLAMATION MARK */
  { 0x00B7, 0x0045 },   /* MIDDLE DOT */
  { 0x00BF, 0x003F },   /* INVERTED QUESTION MARK */
  { 0x00D7, 0x002A },   /* MULTIPLIKATION SIGN */
  { 0x00F7, 0x002F },   /* DIVISION SIGN */
  { 0x2000, 0x0020 },   /* EN QUAD */
  { 0x2001, 0x0020 },   /* EM QUAD */
  { 0x2002, 0x0020 },   /* EN SPACE */
  { 0x2003, 0x0020 },   /* EM SPACE */
  { 0x2004, 0x0020 },   /* THREE-PER-EM SPACE */
  { 0x2005, 0x0020 },   /* FOUR-PER-EM SPACE */
  { 0x2006, 0x0020 },   /* SIX-PER-EM SPACE */
  { 0x2007, 0x0020 },   /* FIGURE SPACE */
  { 0x2008, 0x0020 },   /* PUNCTATION SPACE */
  { 0x2009, 0x0020 },   /* THIN SPACE */
  { 0x200A, 0x0020 },   /* HAIR SPACE */
  { 0x2010, 0x002D },   /* HYPHEN */
  { 0x2011, 0x002D },   /* NON-BREAKING HYPHEN */
  { 0x2012, 0x002D },   /* FIGURE DASH */
  { 0x2013, 0x002D },   /* EN DASH */
  { 0x2014, 0x002D },   /* EM DASH */
  { 0x2015, 0x002D },   /* HORIZONTAL BAR */
  { 0x2018, 0x0027 },   /* LEFT SINGLE QUOTATION MARK */
  { 0x2019, 0x0027 },   /* RIGHT SINGLE QUOTATION MARK */
  { 0x201A, 0x002C },   /* SINGLE LOW-9 QUOTATION MARK */
  { 0x201B, 0x0027 },   /* SINGLE HIGH-RESERVED-9 QUOTATION MARK */
  { 0x201C, 0x0022 },   /* LEFT DOUBLE QUOTATION MARK */
  { 0x201D, 0x0022 },   /* RIGHT DOUBLE QUOTATION MARK */
  { 0x201E, 0x0022 },   /* DOUBLE LOW-9 QUOTATION MARK */
  { 0x201F, 0x0022 },   /* DOUBLE HIGH-RESERVED-9 QUOTATION MARK */
  { 0x2022, 0x002D },   /* BULLET */
  { 0x2023, 0x002D },   /* TRIANGULAR BULLET */
  { 0x2024, 0x002D },   /* ONE DOT LEADER */
  { 0x2027, 0x002D },   /* HYPHENATION POINT */
  { 0x2028, 0x000A },   /* LINE SEPARATOR */
  { 0x2029, 0x000D },   /* PARAGRAPH SEPARATOR */
  { 0x2032, 0x0027 },   /* PRIME */
  { 0x2033, 0x0022 },   /* DOUBLE PRIME */
  { 0x2035, 0x0027 },   /* RESERVED PRIME */
  { 0x2036, 0x0022 },   /* RESERVED DOUBLE PRIME */
  { 0x2039, 0x003C },   /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
  { 0x203A, 0x003E },   /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
  { 0x2043, 0x002D },   /* HYPHEN BULLET */
  { 0x2044, 0x002F },   /* FRACTION SLASH */
  { 0x2160, 0x0049 },   /* ROMAN NUMERAL ONE */
  { 0x2164, 0x0056 },   /* ROMAN NUMERAL FIVE */
  { 0x2169, 0x0058 },   /* ROMAN NUMERAL TEN */
  { 0x216C, 0x004C },   /* ROMAN NUMERAL FIFTY */
  { 0x216D, 0x0043 },   /* ROMAN NUMERAL ONE HUNDRED */
  { 0x216E, 0x0044 },   /* ROMAN NUMERAL FIVE HUNDRED */
  { 0x216F, 0x004D },   /* ROMAN NUMERAL ONE THOUSAND */
  { 0x2170, 0x0069 },   /* SMALL ROMAN NUMERAL ONE */
  { 0x2174, 0x0076 },   /* SMALL ROMAN NUMERAL FIVE */
  { 0x2179, 0x0078 },   /* SMALL ROMAN NUMERAL TEN */
  { 0x217C, 0x006C },   /* SMALL ROMAN NUMERAL FIFTY */
  { 0x217D, 0x0063 },   /* SMALL ROMAN NUMERAL ONE HUNDRED */
  { 0x217E, 0x0064 },   /* SMALL ROMAN NUMERAL FIVE HUNDRED */
  { 0x217F, 0x006D },   /* SMALL ROMAN NUMERAL ONE THOUSAND */
  { 0x2215, 0x002F },   /* DIVISION SLASH */
  { 0x2217, 0x002A },   /* ASTERIX OPERATOR */
  { 0xFF00, 0x0020 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF01, 0x0021 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF02, 0x0022 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF03, 0x0023 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF04, 0x0024 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF05, 0x0025 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF06, 0x0026 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF07, 0x0027 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF08, 0x0028 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF09, 0x0029 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF0A, 0x002A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF0B, 0x002B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF0C, 0x002C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF0D, 0x002D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF0E, 0x002E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF0F, 0x002F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF10, 0x0030 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF11, 0x0031 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF12, 0x0032 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF13, 0x0033 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF14, 0x0034 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF15, 0x0035 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF16, 0x0036 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF17, 0x0037 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF18, 0x0038 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF19, 0x0039 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF1A, 0x003A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF1B, 0x003B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF1C, 0x003C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF1D, 0x003D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF1E, 0x003E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF1F, 0x003F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF20, 0x0040 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF21, 0x0041 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF22, 0x0042 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF23, 0x0043 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF24, 0x0044 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF25, 0x0045 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF26, 0x0046 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF27, 0x0047 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF28, 0x0048 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF29, 0x0049 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF2A, 0x004A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF2B, 0x004B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF2C, 0x004C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF2D, 0x004D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF2E, 0x004E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF2F, 0x004F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF30, 0x0050 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF31, 0x0051 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF32, 0x0052 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF33, 0x0053 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF34, 0x0054 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF35, 0x0055 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF36, 0x0056 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF37, 0x0057 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF38, 0x0058 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF39, 0x0059 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF3A, 0x005A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF3B, 0x005B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF3C, 0x005C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF3D, 0x005D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF3E, 0x005E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF3F, 0x005F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF40, 0x0060 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF41, 0x0061 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF42, 0x0062 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF43, 0x0063 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF44, 0x0064 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF45, 0x0065 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF46, 0x0066 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF47, 0x0067 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF48, 0x0068 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF49, 0x0069 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF4A, 0x006A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF4B, 0x006B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF4C, 0x006C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF4D, 0x006D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF4E, 0x006E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF4F, 0x006F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF50, 0x0070 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF51, 0x0071 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF52, 0x0072 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF53, 0x0073 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF54, 0x0074 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF55, 0x0075 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF56, 0x0076 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF57, 0x0077 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF58, 0x0078 },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF59, 0x0079 },   /* FULLWIDTH ASCII FORMS */
  { 0xFF5A, 0x007A },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF5B, 0x007B },   /* FULLWIDTH ASCII FORMS */
  { 0xFF5C, 0x007C },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF5D, 0x007D },   /* FULLWIDTH ASCII FORMS */
  { 0xFF5E, 0x007E },   /* FULLWIDTH ASCII FORMS*/
  { 0xFF5F, 0x007F },   /* FULLWIDTH ASCII FORMS */
  { 0xFF61, 0x3002 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF62, 0x300C },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF63, 0x300D },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF64, 0x3001 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF65, 0x30FB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF66, 0x30F2 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF67, 0x30A1 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF68, 0x30A3 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF69, 0x30A5 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6A, 0x30A7 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6B, 0x30A9 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6C, 0x30E3 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6D, 0x30E5 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6E, 0x30E7 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF6F, 0x30C3 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF70, 0x30FC },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF71, 0x30A2 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF72, 0x30A4 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF73, 0x30A6 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF74, 0x30A8 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF75, 0x30AA },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF76, 0x30AB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF77, 0x30AD },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF78, 0x30AF },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF79, 0x30B1 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7A, 0x30B3 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7B, 0x30B5 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7C, 0x30B7 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7D, 0x30B9 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7E, 0x30BB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF7F, 0x30BD },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF80, 0x30BF },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF81, 0x30C1 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF82, 0x30C4 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF83, 0x30C6 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF84, 0x30C8 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF85, 0x30CA },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF86, 0x30CB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF87, 0x30CC },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF88, 0x30CD },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF89, 0x30CE },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8A, 0x30CF },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8B, 0x30D2 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8C, 0x30D5 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8D, 0x30D8 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8E, 0x30DB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF8F, 0x30DE },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF90, 0x30DF },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF91, 0x30E0 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF92, 0x30E1 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF93, 0x30E2 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF94, 0x30E4 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF95, 0x30E6 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF96, 0x30E8 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF97, 0x30E9 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF98, 0x30EA },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF99, 0x30EB },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9A, 0x30EC },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9B, 0x30ED },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9C, 0x30EF },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9D, 0x30F3 },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9E, 0x309B },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFF9F, 0x309C },   /* HALFWIDTH KATAKANA FORMS */
  { 0xFFA0, 0x3164 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA1, 0x3131 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA2, 0x3132 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA3, 0x3133 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA4, 0x3134 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA5, 0x3135 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA6, 0x3136 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA7, 0x3137 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA8, 0x3138 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFA9, 0x3139 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAA, 0x313A },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAB, 0x313B },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAC, 0x313C },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAD, 0x313D },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAE, 0x313E },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFAF, 0x313F },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB0, 0x3140 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB1, 0x3141 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB2, 0x3142 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB3, 0x3143 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB4, 0x3144 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB5, 0x3145 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB6, 0x3146 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB7, 0x3147 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB8, 0x3148 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFB9, 0x3149 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFBA, 0x314A },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFBB, 0x314B },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFBC, 0x314C },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFBD, 0x314D },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFBE, 0x314E },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC2, 0x314F },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC3, 0x3150 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC4, 0x3151 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC5, 0x3152 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC6, 0x3153 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFC7, 0x3154 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCA, 0x3155 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCB, 0x3156 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCC, 0x3157 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCD, 0x3158 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCE, 0x3159 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFCF, 0x315A },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD2, 0x315B },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD3, 0x315C },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD4, 0x315D },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD5, 0x315E },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD6, 0x315F },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFD7, 0x3160 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFDA, 0x3161 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFDB, 0x3162 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFDC, 0x3163 },   /* HALFWIDTH HANGUL FORMS */
  { 0xFFE0, 0x00A2 },   /* FULLWIDTH CENT SIGN */
  { 0xFFE1, 0x00A3 },   /* FULLWIDTH POUND SIGN */
  { 0xFFE2, 0x00AC },   /* FULLWIDTH NOT SIGN */
  { 0xFFE3, 0x00AF },   /* FULLWIDTH MACRON */
  { 0xFFE4, 0x00A6 },   /* FULLWIDTH BROKEN BAR */
  { 0xFFE5, 0x00A5 },   /* FULLWIDTH YEN SIGN */
  { 0xFFE6, 0x20A9 },   /* FULLWIDTH WON SIGN */
  { 0xFFE8, 0x2502 },   /* HALFWIDTH FORMS LIGHT VERTICAL */
  { 0xFFE9, 0x2190 },   /* HALFWIDTH LEFTWARDS ARROW */
  { 0xFFEA, 0x2191 },   /* HALFWIDTH UPWARDS ARROW */
  { 0xFFEB, 0x2192 },   /* HALFWIDTH RIGHTWARDS ARROW */
  { 0xFFEC, 0x2193 },   /* HALFWIDTH DOWNWARDS ARROW */
  { 0xFFED, 0x25A0 },   /* HALFWIDTH BLACK SQUARE */
  { 0xFFEE, 0x25CB },   /* HALFWIDTH WHITE CIRCLE */
  { 0xFFFD, 0x003F }    /* REPLACEMENT CHARACTER */
};

sal_uInt16 ImplGetReplaceChar( sal_Unicode c )
{
    sal_uInt16                  nLow;
    sal_uInt16                  nHigh;
    sal_uInt16                  nMid;
    sal_uInt16                  nCompareChar;
    const ImplReplaceCharData*  pCharData;

    nLow = 0;
    nHigh = (sizeof( aImplRepCharTab )/sizeof( ImplReplaceCharData ))-1;
    do
    {
        nMid = (nLow+nHigh)/2;
        pCharData = aImplRepCharTab+nMid;
        nCompareChar = pCharData->mnUniChar;
        if ( c < nCompareChar )
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if ( c > nCompareChar )
                nLow = nMid+1;
            else
                return pCharData->mnReplaceChar;
        }
    }
    while ( nLow <= nHigh );

    return 0;
}

/* ----------------------------------------------------------------------- */

typedef struct _ImplReplaceCharStrData
{
    sal_uInt16      mnUniChar;
    sal_uInt16      maReplaceChars[IMPL_MAX_REPLACECHAR];
} ImplReplaceCharStrData;

static ImplReplaceCharStrData const aImplRepCharStrTab[] =
{
  { 0x00A9, { 0x0028, 0x0063, 0x0029, 0x0000, 0x0000  } },  /* COPYRIGHT SIGN */
  { 0x00AB, { 0x003C, 0x003C, 0x0000, 0x0000, 0x0000  } },  /* LEFT-POINTING-DOUBLE ANGLE QUOTATION MARK */
  { 0x0AE0, { 0x0028, 0x0072, 0x0029, 0x0000, 0x0000  } },  /* REGISTERED SIGN */
  { 0x00BB, { 0x003E, 0x003E, 0x0000, 0x0000, 0x0000  } },  /* RIGHT-POINTING-DOUBLE ANGLE QUOTATION MARK */
  { 0x00BC, { 0x0031, 0x002F, 0x0034, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE QUARTER */
  { 0x00BD, { 0x0031, 0x002F, 0x0032, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE HALF */
  { 0x00BE, { 0x0033, 0x002F, 0x0034, 0x0000, 0x0000  } },  /* VULGAR FRACTION THREE QUARTERS */
  { 0x00C6, { 0x0041, 0x0045, 0x0000, 0x0000, 0x0000  } },  /* LATIN CAPITAL LETTER AE */
  { 0x00E6, { 0x0061, 0x0065, 0x0000, 0x0000, 0x0000  } },  /* LATIN SMALL LETTER AE */
  { 0x0152, { 0x004F, 0x0045, 0x0000, 0x0000, 0x0000  } },  /* LATIN CAPITAL LIGATURE OE */
  { 0x0153, { 0x006F, 0x0065, 0x0000, 0x0000, 0x0000  } },  /* LATIN SMALL LIGATURE OE */
  { 0x2025, { 0x002E, 0x002E, 0x0000, 0x0000, 0x0000  } },  /* TWO DOT LEADER */
  { 0x2026, { 0x002E, 0x002E, 0x002E, 0x0000, 0x0000  } },  /* HORIZONTAL ELLIPSES */
  { 0x2034, { 0x0027, 0x0027, 0x0027, 0x0000, 0x0000  } },  /* TRIPPLE PRIME */
  { 0x2037, { 0x0027, 0x0027, 0x0027, 0x0000, 0x0000  } },  /* RESERVED TRIPPLE PRIME */
  { 0x20AC, { 0x0045, 0x0055, 0x0052, 0x0000, 0x0000  } },  /* EURO SIGN */
  { 0x2122, { 0x0028, 0x0074, 0x006D, 0x0029, 0x0000  } },  /* TRADE MARK SIGN */
  { 0x2153, { 0x0031, 0x002F, 0x0033, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE THIRD */
  { 0x2154, { 0x0032, 0x002F, 0x0033, 0x0000, 0x0000  } },  /* VULGAR FRACTION TWO THIRD */
  { 0x2155, { 0x0031, 0x002F, 0x0035, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE FIFTH */
  { 0x2156, { 0x0032, 0x002F, 0x0035, 0x0000, 0x0000  } },  /* VULGAR FRACTION TWO FIFTH */
  { 0x2157, { 0x0033, 0x002F, 0x0035, 0x0000, 0x0000  } },  /* VULGAR FRACTION THREE FIFTH */
  { 0x2158, { 0x0034, 0x002F, 0x0035, 0x0000, 0x0000  } },  /* VULGAR FRACTION FOUR FIFTH */
  { 0x2159, { 0x0031, 0x002F, 0x0036, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE SIXTH */
  { 0x215A, { 0x0035, 0x002F, 0x0036, 0x0000, 0x0000  } },  /* VULGAR FRACTION FIVE SIXTH */
  { 0x215B, { 0x0031, 0x002F, 0x0038, 0x0000, 0x0000  } },  /* VULGAR FRACTION ONE EIGHTH */
  { 0x215C, { 0x0033, 0x002F, 0x0038, 0x0000, 0x0000  } },  /* VULGAR FRACTION THREE EIGHTH */
  { 0x215D, { 0x0035, 0x002F, 0x0038, 0x0000, 0x0000  } },  /* VULGAR FRACTION FIVE EIGHTH */
  { 0x215E, { 0x0037, 0x002F, 0x0038, 0x0000, 0x0000  } },  /* VULGAR FRACTION SEVEN EIGHTH */
  { 0x215F, { 0x0031, 0x002F, 0x0000, 0x0000, 0x0000  } },  /* FRACTION NUMERATOR ONE */
  { 0x2161, { 0x0049, 0x0049, 0x0000, 0x0000, 0x0000  } },  /* ROMAN NUMERAL TWO */
  { 0x2162, { 0x0049, 0x0049, 0x0049, 0x0000, 0x0000  } },  /* ROMAN NUMERAL THREE */
  { 0x2163, { 0x0049, 0x0056, 0x0000, 0x0000, 0x0000  } },  /* ROMAN NUMERAL FOUR */
  { 0x2165, { 0x0056, 0x0049, 0x0000, 0x0000, 0x0000  } },  /* ROMAN NUMERAL SIX */
  { 0x2166, { 0x0056, 0x0049, 0x0049, 0x0000, 0x0000  } },  /* ROMAN NUMERAL SEVEN */
  { 0x2168, { 0x0056, 0x0049, 0x0049, 0x0049, 0x0000  } },  /* ROMAN NUMERAL EIGHT */
  { 0x2169, { 0x0049, 0x0058, 0x0000, 0x0000, 0x0000  } },  /* ROMAN NUMERAL NINE */
  { 0x216A, { 0x0058, 0x0049, 0x0000, 0x0000, 0x0000  } },  /* ROMAN NUMERAL ELEVEN */
  { 0x216B, { 0x0058, 0x0049, 0x0049, 0x0000, 0x0000  } },  /* ROMAN NUMERAL TWELVE */
  { 0x2171, { 0x0069, 0x0069, 0x0000, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL TWO */
  { 0x2172, { 0x0069, 0x0069, 0x0069, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL THREE */
  { 0x2173, { 0x0069, 0x0076, 0x0000, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL FOUR */
  { 0x2175, { 0x0076, 0x0069, 0x0000, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL SIX */
  { 0x2176, { 0x0076, 0x0069, 0x0069, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL SEVEN */
  { 0x2178, { 0x0076, 0x0069, 0x0069, 0x0069, 0x0000  } },  /* SMALL ROMAN NUMERAL EIGHT */
  { 0x2179, { 0x0069, 0x0078, 0x0000, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL NINE */
  { 0x217A, { 0x0078, 0x0069, 0x0000, 0x0000, 0x0000  } },  /* SMALL ROMAN NUMERAL ELEVEN */
  { 0x217B, { 0x0058, 0x0069, 0x0069, 0x0000, 0x0000  } }   /* SMALL ROMAN NUMERAL TWELVE */
};

const sal_uInt16* ImplGetReplaceString( sal_Unicode c )
{
    sal_uInt16                      nLow;
    sal_uInt16                      nHigh;
    sal_uInt16                      nMid;
    sal_uInt16                      nCompareChar;
    const ImplReplaceCharStrData*   pCharData;

    nLow = 0;
    nHigh = (sizeof( aImplRepCharStrTab )/sizeof( ImplReplaceCharStrData ))-1;
    do
    {
        nMid = (nLow+nHigh)/2;
        pCharData = aImplRepCharStrTab+nMid;
        nCompareChar = pCharData->mnUniChar;
        if ( c < nCompareChar )
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if ( c > nCompareChar )
                nLow = nMid+1;
            else
                return pCharData->maReplaceChars;
        }
    }
    while ( nLow <= nHigh );

    return 0;
}
