/*************************************************************************
 *
 *  $RCSfile: string.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obr $ $Date: 2001-03-30 14:36:27 $
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

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _RTL_STRIMP_H_
#include "strimp.h"
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

/* ======================================================================= */

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
static rtl_String aImplEmpty_rtl_String =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    { 0 }   /* sal_Char     buffer[1];  */
};

/* ======================================================================= */

/*************************************************************************
 *  rtl_str_compareIgnoreCase_WithLength
 */
sal_Int32 SAL_CALL
rtl_str_compareIgnoreCase_WithLength( const sal_Char *first,
                                      const sal_Char *second,
                                      sal_Int32 len )
{
    sal_Char  a;
    sal_Char  b;

    while ( len )
    {
        a = *first;
        b = *second;
        if ( (a >= 97) && (a <= 122) )
            a -= 32;

        if ( (b >= 97) && (b <= 122) )
            b -= 32;

        /* Naechstes Zeichen */
        first++;
        second++;
        len--;

        /* If characters don't match but case may be ignored,
           try converting both characters to uppercase.
           If the results match, then the comparison scan should
           continue. */

        if ( a == b )
            continue;

        return (sal_Int32)a - (sal_Int32)b;
    }

    return 0; /* strings are equal */
}

/*************************************************************************
 *  rtl_str_compareIgnoreCase
 */
sal_Int32 SAL_CALL
rtl_str_compareIgnoreCase( const sal_Char * first, const sal_Char * second )
{
    sal_Int32 firstLen    = rtl_str_getLength(first);
    sal_Int32 secondLen   = rtl_str_getLength(second);
    sal_Int32 shortestLen = 1 + (firstLen < secondLen ? firstLen : secondLen);

    return rtl_str_compareIgnoreCase_WithLength( first, second, shortestLen );
}

/*************************************************************************
 *  rtl_str_equalsIgnoreCase_WithLength
 */
sal_Bool SAL_CALL
rtl_str_equalsIgnoreCase_WithLength( const sal_Char * first,
                                     sal_Int32 firstLen,
                                     const sal_Char * second,
                                     sal_Int32 secondLen )
{
    if (firstLen != secondLen)
        return sal_False;

    return rtl_str_compareIgnoreCase_WithLength(first, second, firstLen) == 0;
}


/*************************************************************************
 *  rtl_str_equalsIgnoreCase
 */
sal_Bool SAL_CALL
rtl_str_equalsIgnoreCase( const sal_Char * first, const sal_Char * second )
{
    return rtl_str_equalsIgnoreCase_WithLength(
                                        first,  rtl_str_getLength(first),
                                        second, rtl_str_getLength(second));
}

/*************************************************************************
 *  rtl_str_toAsciiLowerCase_WithLength
 */
void SAL_CALL rtl_str_toAsciiLowerCase_WithLength( sal_Char * str, sal_Int32 len )
{
    while ( *str )
    {
        if ( (*str >= 65) && (*str <= 90) )
            *str += 32;

        str++;
    }
}

/*************************************************************************
 *  rtl_str_toAsciiLowerCase
 */
void SAL_CALL rtl_str_toAsciiLowerCase( sal_Char * str )
{
    while ( *str )
    {
        if ( (*str >= 65) && (*str <= 90) )
            *str += 32;

        str++;
    }
}

/*************************************************************************
 *  rtl_str_toAsciiUpperCase_WithLength
 */
void SAL_CALL rtl_str_toAsciiUpperCase_WithLength( sal_Char * str, sal_Int32 len )
{
    while ( *str )
    {
        if ( (*str >= 97) && (*str <= 122) )
            *str -= 32;

        str++;
    }
}

/*************************************************************************
 *  rtl_str_toAsciiUpperCase
 */
void SAL_CALL rtl_str_toAsciiUpperCase( sal_Char * str )
{
    while ( *str )
    {
        if ( (*str >= 97) && (*str <= 122) )
            *str -= 32;

        str++;
    }
}

/*************************************************************************
 *  rtl_str_trim_WithLength
 */
sal_Int32 SAL_CALL rtl_str_trim_WithLength( sal_Char * str, sal_Int32 len )
{
    sal_Int32 preSpaces = 0;
    sal_Int32 postSpaces = 0;
    sal_Int32 newLen = 0;
    sal_Int32 index = len - 1;

    while ( (preSpaces < len) && (*(str+preSpaces) == ' ') )
        preSpaces++;

    while ( (index > preSpaces) && (*(str+index) == ' ') )
    {
        postSpaces++;
        index--;
    }

    newLen = len - preSpaces - postSpaces;

    if ( newLen != len )
    {
        sal_Char *newStr = str + preSpaces;
        index = 0;

        while ( index != newLen )
        {
            *(str + index) = *(newStr + index);
            index++;
        }
        *(str + index) = '\0';
    }

    return newLen;
}

/*************************************************************************
 *  rtl_str_trim
 */
sal_Int32 SAL_CALL rtl_str_trim( sal_Char * str )
{
    return rtl_str_trim_WithLength(str, rtl_str_getLength(str));
}

/*************************************************************************
 *
 *  rtl_string_XXX Functions
 *
 ************************************************************************/


/*************************************************************************
 *  rtl_string_newReplace
 */
void SAL_CALL rtl_string_newReplace( rtl_String ** newStr,
                                     rtl_String * str,
                                     sal_Char oldChar,
                                     sal_Char newChar)
{
    rtl_string_newFromString(newStr, str);

    if (!(*newStr)) return;

    rtl_str_replaceChar_WithLength( (*newStr)->buffer, (*newStr)->length, oldChar, newChar );
}

/*************************************************************************
 *  rtl_String_newToLowerCase
 */
void SAL_CALL rtl_string_newToLowerCase( rtl_String ** newStr, rtl_String * str)
{
    rtl_string_newFromString(newStr, str);

    if (!(*newStr)) return;

    rtl_str_toAsciiLowerCase_WithLength((*newStr)->buffer, (*newStr)->length);
}

/*************************************************************************
 *  rtl_String_newToUpperCase
 */
void SAL_CALL rtl_string_newToUpperCase( rtl_String ** newStr, rtl_String * str)
{
    rtl_string_newFromString(newStr, str);

    if (!(*newStr)) return;

    rtl_str_toAsciiUpperCase_WithLength((*newStr)->buffer, (*newStr)->length);
}

/*************************************************************************
 *  rtl_string_newTrim
 */
void SAL_CALL rtl_string_newTrim( rtl_String ** newStr, rtl_String * str )
{
    rtl_string_newFromString(newStr, str);

    if (!(*newStr)) return;

    (*newStr)->length = rtl_str_trim_WithLength((*newStr)->buffer, (*newStr)->length);
}

/*************************************************************************
 *  rtl_string_getTokenCount
 */
sal_Int32 SAL_CALL rtl_string_getTokenCount( rtl_String * str , sal_Char cTok)
{
    sal_Int32 count = 0;
    sal_Int32 index1 = 0;
    sal_Int32 index2 = 0;
    sal_Char *buffer = str->buffer;

    while ((index2 = rtl_str_indexOfChar_WithLength(buffer + index1, str->length - index1, cTok)) >= 0)
    {
        if (index2 <= (str->length - index1) && index2 >= 0)
            count++;

        index1 += index2 + 1;
    }

    if (index2 < 0 && index1 <= str->length)
        count++;

    return count;
}

/*************************************************************************
 *  rtl_string_getToken
 */
void SAL_CALL rtl_string_getToken( rtl_String ** newStr , rtl_String * str, sal_Int32 nToken, sal_Char cTok)
{
    sal_Int32 count = 0;
    sal_Int32 index1 = 0;
    sal_Int32 index2 = 0;
    sal_Char *buffer = str->buffer;

    while ((index2 = rtl_str_indexOfChar_WithLength(buffer + index1, str->length - index1, cTok)) >= 0)
    {
        if (count == nToken)
        {
            rtl_string_newFromStr_WithLength( newStr, buffer + index1, index2 );
            return;
        }

        if (index2 <= (str->length - index1) && index2 >= 0)
            count++;

        index1 += index2 + 1;
    }

    if (nToken == 0 || nToken == count)
        rtl_string_newFromStr_WithLength( newStr, buffer + index1, str->length - index1 );
    else
        rtl_string_new(newStr);

    return;
}

/* ======================================================================= */

#define IMPL_RTL_STRCODE            sal_Char
#define IMPL_RTL_USTRCODE( c )      ((unsigned char)c)
#define IMPL_RTL_STRNAME( n )       rtl_str_ ## n

#define IMPL_RTL_STRINGNAME( n )    rtl_string_ ## n
#define IMPL_RTL_STRINGDATA         rtl_String
#define IMPL_RTL_EMPTYSTRING        aImplEmpty_rtl_String

/* ======================================================================= */

/* Include String/UString template code */

#include "strtmpl.c"

/* ======================================================================= */

#define RTL_UNICODE_START_HIGH_SURROGATES                   0xD800
#define RTL_UNICODE_END_HIGH_SURROGATES                     0xDBFF
#define RTL_UNICODE_START_LOW_SURROGATES                    0xDC00
#define RTL_UNICODE_END_LOW_SURROGATES                      0xDFFF

#define RTL_UNICODE_SURROGATES_HALFMASK                     0x03FFUL
#define RTL_UNICODE_SURROGATES_HALFBASE                     0x0010000UL
#define RTL_UNICODE_SURROGATES_HALFSHIFT                    10

/* ----------------------------------------------------------------------- */

static int rtl_ImplGetFastUTF8ByteLen( const sal_Unicode* pStr, sal_Int32 nLen )
{
    int                 n;
    sal_Unicode         c;
    sal_uInt32          nUCS4Char;
    const sal_Unicode*  pEndStr;

    n = 0;
    pEndStr  = pStr+nLen;
    while ( pStr < pEndStr )
    {
        c = *pStr;

        if ( c < 0x80 )
            n++;
        else if ( c < 0x800 )
            n += 2;
        else
        {
            if ( (c < RTL_UNICODE_START_HIGH_SURROGATES) ||
                 (c > RTL_UNICODE_END_HIGH_SURROGATES) )
                n += 3;
            else
            {
                nUCS4Char = c;

                if ( pStr+1 < pEndStr )
                {
                    c = *(pStr+1);
                    if ( (c >= RTL_UNICODE_START_LOW_SURROGATES) &&
                         (c <= RTL_UNICODE_END_LOW_SURROGATES) )
                    {
                        nUCS4Char -= RTL_UNICODE_START_HIGH_SURROGATES;
                        nUCS4Char <<= RTL_UNICODE_SURROGATES_HALFSHIFT;
                        nUCS4Char += (c-RTL_UNICODE_START_LOW_SURROGATES) + RTL_UNICODE_SURROGATES_HALFBASE;
                        pStr++;
                    }
                }

                if ( nUCS4Char < 0x10000 )
                    n += 3;
                else if ( nUCS4Char < 0x200000 )
                    n += 4;
                else if ( nUCS4Char < 0x4000000 )
                    n += 5;
                else
                    n += 6;
            }
        }

        pStr++;
    }

    return n;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_uString2String( rtl_String** ppThis,
                                  const sal_Unicode* pUStr,
                                  sal_Int32 nULen,
                                  rtl_TextEncoding eTextEncoding,
                                  sal_uInt32 nCvtFlags )
{
    OSL_ENSURE( (eTextEncoding != 9) &&
                (eTextEncoding != RTL_TEXTENCODING_DONTKNOW) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS2) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS4),
                "rtl_uString2String() - Wrong TextEncoding" );

    if ( !nULen )
        rtl_string_new( ppThis );
    else
    {
        rtl_String*                 pTemp;
        rtl_UnicodeToTextConverter  hConverter;
        sal_uInt32                  nInfo;
        sal_Size                    nSrcChars;
        sal_Size                    nDestBytes;
        sal_Size                    nNewLen;
        sal_Size                    nNotConvertedChars;
        sal_Size                    nMaxCharLen;

        if ( *ppThis )
            IMPL_RTL_STRINGNAME( release )( *ppThis );

        /* Optimization for UTF-8 - we try to calculate the exact length */
        /* For all other encoding we try an good estimation */
        if ( eTextEncoding == RTL_TEXTENCODING_UTF8 )
        {
            nNewLen = rtl_ImplGetFastUTF8ByteLen( pUStr, nULen );
            /* Includes the string only ASCII, then we could copy
               the buffer faster */
            if ( nNewLen == (sal_Size)nULen )
            {
                IMPL_RTL_STRCODE* pBuffer;
                *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nULen );
                pBuffer = (*ppThis)->buffer;
                do
                {
                    /* Check ASCII range */
                    OSL_ENSURE( *pUStr <= 127,
                                "rtl_uString2String() - UTF8 test is encoding is wrong" );

                    *pBuffer = (IMPL_RTL_STRCODE)(unsigned char)*pUStr;
                    pBuffer++;
                    pUStr++;
                    nULen--;
                }
                while ( nULen );
                return;
            }

            nMaxCharLen = 6;
        }
        else
        {
            rtl_TextEncodingInfo aTextEncInfo;
            aTextEncInfo.StructSize = sizeof( aTextEncInfo );
            if ( !rtl_getTextEncodingInfo( eTextEncoding, &aTextEncInfo ) )
            {
                aTextEncInfo.AverageCharSize    = 1;
                aTextEncInfo.MaximumCharSize    = 8;
            }

            nNewLen = nULen*aTextEncInfo.AverageCharSize;
            nMaxCharLen = aTextEncInfo.MaximumCharSize;
        }

        nCvtFlags |= RTL_UNICODETOTEXT_FLAGS_FLUSH;
        hConverter = rtl_createUnicodeToTextConverter( eTextEncoding );

        pTemp = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
        nDestBytes = rtl_convertUnicodeToText( hConverter, 0,
                                               pUStr, nULen,
                                               pTemp->buffer, nNewLen,
                                               nCvtFlags,
                                               &nInfo, &nSrcChars );

        /* Buffer not big enough, try again with enough space */
        while ( nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
        {
            rtl_freeMemory( pTemp );

            /* Try with the max. count of characters with
               additional overhead for replacing functionality */
            nNotConvertedChars = nULen-nSrcChars;
            nNewLen = nDestBytes+(nNotConvertedChars*nMaxCharLen)+nNotConvertedChars+4;
            pTemp = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
            nDestBytes = rtl_convertUnicodeToText( hConverter, 0,
                                                   pUStr, nULen,
                                                   pTemp->buffer, nNewLen,
                                                   nCvtFlags,
                                                   &nInfo, &nSrcChars );
        }

        /* Set the buffer to the correct size or is there to
           much overhead, reallocate to the correct size */
        if ( nNewLen > nDestBytes+8 )
        {
            rtl_String* pTemp2 = IMPL_RTL_STRINGNAME( ImplAlloc )( nDestBytes );
            rtl_str_ImplCopy( pTemp2->buffer, pTemp->buffer, nDestBytes );
            rtl_freeMemory( pTemp );
            pTemp = pTemp2;
        }
        else
        {
            pTemp->length = nDestBytes;
            pTemp->buffer[nDestBytes] = 0;
        }

        rtl_destroyUnicodeToTextConverter( hConverter );
        *ppThis = pTemp;

        /* Results the conversion in an empty buffer -
           create an empty string */
        if ( pTemp && !nDestBytes )
            rtl_string_new( ppThis );
    }
}

