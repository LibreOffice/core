/*************************************************************************
 *
 *  $RCSfile: ustring.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:04:21 $
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

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif
#ifndef _RTL_LOCALE_H_
#include <rtl/locale.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _RTL_STRIMP_H_
#include "strimp.h"
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

/* ======================================================================= */

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
static rtl_uString aImplEmpty_rtl_uString =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    0       /* sal_Unicode  buffer[1];  */
};

/* ======================================================================= */

/*************************************************************************
 *  rtl_ustr_equalsIgnoreCase_WithLength
 */
sal_Bool SAL_CALL rtl_ustr_equalsIgnoreCase_WithLength( const sal_Unicode * first,
                                                        sal_Int32 firstLen,
                                                        const sal_Unicode * second,
                                                        sal_Int32 secondLen )
{
    sal_Unicode a;
    sal_Unicode b;

    if (firstLen != secondLen)
        return sal_False;

    while ( firstLen )
    {
        a = rtl_char_toUpperCase(*first);
        b = rtl_char_toUpperCase(*second);

        /* Naechstes Zeichen */
        first++;
        second++;
        firstLen--;

        /* If characters don't match but case may be ignored,
           try converting both characters to uppercase.
           If the results match, then the comparison scan should
           continue. */
        if ( a == b )
            continue;

        /* Unfortunately, conversion to uppercase does not work properly
           for the Georgian alphabet, which has strange rules about case
           conversion.  So we need to make one last check before
           exiting. */
        if ( rtl_char_toLowerCase(a) == rtl_char_toLowerCase(b) )
            continue;

        return sal_False;
    }

    return sal_True;      /* Strings sind gleich */
}

/*************************************************************************
 *  rtl_ustr_equalsIgnoreCase
 */
sal_Bool SAL_CALL rtl_ustr_equalsIgnoreCase( const sal_Unicode * first, const sal_Unicode * second )
{
    return rtl_ustr_equalsIgnoreCase_WithLength( first, rtl_ustr_getLength(first),
                                                 second, rtl_ustr_getLength(second) );
}

/*************************************************************************
 *  rtl_ustr_compare_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_shortenedCompare_WithLength( const sal_Unicode * first,
                                                        sal_Int32 firstLen,
                                                        const sal_Unicode * second,
                                                        sal_Int32 secondLen,
                                                        sal_Int32 shortenedLength )
{
    const sal_Unicode * firstEnd = first + firstLen;
    const sal_Unicode * secondEnd = second + secondLen;
    sal_Int32 nResult = 0;
    while( shortenedLength-- && first < firstEnd && second < secondEnd
        && (0 == (nResult = (sal_Int32)*first++ - (sal_Int32)*second++ ) ) )
        ;
    if( nResult || !(shortenedLength != -1) )
        return nResult;
    return firstLen - secondLen;
}

/*************************************************************************
 *  rtl_ustr_ascii_compare_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compare_WithLength( const sal_Unicode * first,
                                                      sal_Int32 firstLen,
                                                      const sal_Char * second )
{
    return rtl_ustr_ascii_shortenedCompare_WithLength( first, firstLen, second, 0x7FFFFFF );
}

/*************************************************************************
 *  rtl_ustr_ascii_compare
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_compare( const sal_Unicode * first, const sal_Char * second )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)*first)-((sal_Int32)*second)) == 0) &&
            *second )
    {
        first++;
        second++;
    }

    return nRet;
}

/*************************************************************************
 *  rtl_ustr_ascii_shortenedCompare_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_ascii_shortenedCompare_WithLength( const sal_Unicode * first,
                                                            sal_Int32 firstLen,
                                                            const sal_Char * second,
                                                            sal_Int32 shortenedLength )
{
    const sal_Unicode * firstEnd = first + firstLen;
    sal_Int32 nResult = 0;
    while( shortenedLength--
        && first < firstEnd
        && *second // necessary if 0 is allowed in Unicode
        && (0 == (nResult = (sal_Int32)*first++ - (sal_Int32)*second++ ) ) )
    {
        /* Check ASCII range */
        OSL_ENSURE( (*(second-1) & 0x80) == 0, "Found ASCII char > 127");
    }
    if( !nResult && (shortenedLength != -1) )
    {
        if( *second )
        {
            OSL_ENSURE( first == firstEnd, "first == firstEnd failed" );
            // first is a substring of the second string => less (negative value)
            nResult = -1;
        }
        else
            // greater or equal
            nResult = firstEnd - first;
    }

    return nResult;
}

/*************************************************************************
 *  rtl_ustr_asciil_reverseCompare_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_asciil_reverseCompare_WithLength( const sal_Unicode * first, sal_Int32 firstLen,
                                                            const sal_Char * second, sal_Int32 secondLen )
{
    const sal_Unicode * firstRun = first + firstLen;
    const sal_Char * secondRun = second + secondLen;
    sal_Int32 nResult = 0;
    while( first < firstRun && second < secondRun
        && (0 == (nResult = (sal_Int32)*--firstRun - (sal_Int32)*--secondRun ) ) )
        ;
    if( nResult )
        return nResult;
    return firstLen - secondLen;
}


/*************************************************************************
 *  rtl_ustr_toAsciiLowerCase_WithLength
 */
void SAL_CALL rtl_ustr_toAsciiLowerCase_WithLength( sal_Unicode * str, sal_Int32 len )
{
    sal_Unicode ch;

    while ( *str )
    {
        ch = *str;
        *str = rtl_char_toLowerCase(ch);
        str++;
    }
}

/*************************************************************************
 *  rtl_ustr_toAsciiLowerCase
 */
void SAL_CALL rtl_ustr_toAsciiLowerCase( sal_Unicode * str )
{
    sal_Unicode ch;

    while ( *str )
    {
        ch = *str;
        *str = rtl_char_toLowerCase(ch);
        str++;
    }
}

/*************************************************************************
 *  rtl_ustr_toAsciiUpperCase_WithLength
 */
void SAL_CALL rtl_ustr_toAsciiUpperCase_WithLength(sal_Unicode * str, sal_Int32 len)
{
    sal_Unicode ch;

    while ( *str )
    {
        ch = *str;
        *str = rtl_char_toUpperCase(ch);
        str++;
    }
}

/*************************************************************************
 *  rtl_ustr_toAsciiUpperCase
 */
void SAL_CALL rtl_ustr_toAsciiUpperCase(sal_Unicode * str)
{
    sal_Unicode ch;

    while ( *str )
    {
        ch = *str;
        *str = rtl_char_toUpperCase(ch);
        str++;
    }
}

/*************************************************************************
 *  rtl_ustr_trim_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_trim_WithLength( sal_Unicode * str, sal_Int32 len )
{
    sal_Int32 preSpaces = 0;
    sal_Int32 postSpaces = 0;
    sal_Int32 newLen = 0;
    sal_Int32 index = len - 1;

    while ( (preSpaces < len) && rtl_char_isWhitespace(*(str+preSpaces)) )
        preSpaces++;

    while ( (index > preSpaces) && rtl_char_isWhitespace(*(str+index)) )
    {
        postSpaces++;
        index--;
    }

    newLen = len - preSpaces - postSpaces;

    if ( newLen != len )
    {
        sal_Unicode *newStr = str + preSpaces;
        index = 0;

        while ( index != newLen )
        {
            *(str + index) = *(newStr + index);
            index++;
        }
        *(str + index) = 0;
    }

    return newLen;
}

/*************************************************************************
 *  rtl_ustr_trim
 */
sal_Int32 SAL_CALL rtl_ustr_trim( sal_Unicode * str )
{
    return rtl_ustr_trim_WithLength(str, rtl_ustr_getLength(str));
}

/*************************************************************************
 *
 *  rtl_uString_XXX Functions
 *
/*************************************************************************

/*************************************************************************
 *  rtl_wstr_getLength
 */
static sal_Int32 SAL_CALL rtl_wstr_getLength( const wchar_t* str )
{
    const wchar_t * pTempStr = str;
    while( *pTempStr ) pTempStr++;
    return pTempStr - str;
}

/*************************************************************************
 *  rtl_uString_newFromWStr
 */
void SAL_CALL rtl_uString_newFromWStr( rtl_uString ** newStr, const wchar_t * value )
{
    sal_Int32 length;
    sal_Unicode* p;

    if (!value)
    {
        rtl_uString_new(newStr);
        return;
    }

    length = rtl_wstr_getLength(value);

    if (*newStr)
        rtl_uString_release(*newStr);

    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (length * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = length;

        p = (*newStr)->buffer;
        while ( length )
        {
            *p = (sal_Unicode)(*value);
            p++;
            value++;
            length--;
        }
        *p = 0;
    }
}

/*************************************************************************
 *  rtl_uString_newFromWStr_WithLength
 */
void SAL_CALL rtl_uString_newFromWStr_WithLength( rtl_uString ** newStr,
                                                  const wchar_t * value,
                                                  sal_Int32 len)
{
    sal_Unicode* p;

    if (!value || len < 0)
    {
        rtl_uString_new(newStr);
        return;
    }

    if (*newStr)
        rtl_uString_release(*newStr);

    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (len * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = len;

        p = (*newStr)->buffer;
        while ( len )
        {
            *p = (sal_Unicode)(*value);
            p++;
            value++;
            len--;
        }
        *p = 0;
    }
}

/*************************************************************************
 *  rtl_uString_newFromASCII
 */
void SAL_CALL rtl_uString_newFromAscii( rtl_uString ** newStr, const sal_Char * value )
{
    sal_Int32 length;
    sal_Unicode* p;

    if (!value)
    {
        rtl_uString_new(newStr);
        return;
    }

    length = rtl_str_getLength( value );

    if (*newStr)
        rtl_uString_release(*newStr);

    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (length * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = length;

        p = (*newStr)->buffer;
        while ( length )
        {
            /* Check ASCII range */
            OSL_ENSURE( (*value & 0x80) == 0, "Found ASCII char > 127");

            *p = (sal_Unicode)(*value);
            p++;
            value++;
            length--;
        }
        *p = 0;
    }
}

/*************************************************************************
 *  rtl_uString_newReplace
 */
void SAL_CALL rtl_uString_newReplace( rtl_uString ** newStr,
                                      rtl_uString * str,
                                      sal_Unicode oldChar,
                                      sal_Unicode newChar)
{
    rtl_uString_newFromString(newStr, str);

    if (!(*newStr)) return;

    rtl_ustr_replaceChar_WithLength((*newStr)->buffer, (*newStr)->length, oldChar, newChar);
}

/*************************************************************************
 *  rtl_uString_newToLowerCase
 */
void SAL_CALL rtl_uString_newToLowerCase( rtl_uString ** newStr,
                                          rtl_uString * str,
                                          struct _rtl_Locale * locale )
{
    if (locale && locale->Language->length == 2
      && locale->Language->buffer[0] == 't'
      && locale->Language->buffer[0] == 'r' )
    {
        sal_Int32 i;
        sal_Int32 len = str->length;

        rtl_uString_new_WithLength(newStr, str->length);

        if (!(*newStr)) return;

        (*newStr)->length = str->length;

        /* special loop for Turkey */
        for (i = 0; i < len; i++)
        {
            sal_Unicode ch = str->buffer[i];
            if (ch == L'I')
            {
                (*newStr)->buffer[i] = (sal_Unicode)0x0131; /* dotless small i */
                continue;
            }
            if ((sal_uInt16)ch == 0x0130)               /* dotted I */
            {
                (*newStr)->buffer[i] = L'i';    /* dotted i */
                continue;
            }
            (*newStr)->buffer[i] = rtl_char_toLowerCase(ch);
        }
    } else
    {
        rtl_uString_newFromString(newStr, str);

        if (!(*newStr)) return;

        rtl_ustr_toAsciiLowerCase_WithLength((*newStr)->buffer, (*newStr)->length);
    }
}

/*************************************************************************
 *  rtl_uString_newToUpperCase
 */
void SAL_CALL rtl_uString_newToUpperCase( rtl_uString ** newStr,
                                          rtl_uString * str,
                                          struct _rtl_Locale * locale )
{
    sal_Int32 i;
    sal_Int32 len = str->length;
    sal_Int32 resultOffset = 0;
    sal_Unicode *result = (sal_Unicode*)rtl_allocateMemory((len + 1)* sizeof(sal_Unicode));

    if (locale && locale->Language->length == 2
      && locale->Language->buffer[0] == 't'
      && locale->Language->buffer[0] == 'r' )
    {
        /* special loop for Turkey */
        for (i = 0; i < len; i++)
        {
            sal_Unicode ch = str->buffer[i];
            if ((sal_uInt16)ch == 0x0131)            /* dotless i */
            {
                result[i+resultOffset] = L'I';          /* cap I */
                continue;
            }
            if (ch == L'i')
            {
                result[i+resultOffset] = (sal_Unicode)0x0130;       /* dotted cap i */
                continue;
            }
            if ((sal_uInt16)ch == 0x00DF)               /* sharp s */
            {
                /* Grow result. */
                sal_Unicode *result2 = (sal_Unicode*)rtl_allocateMemory((len + 1 + resultOffset + 1) * sizeof(sal_Unicode));
                rtl_copyMemory(result2, result, (i + resultOffset) * sizeof(sal_Unicode));
                result2[i+resultOffset] = L'S';
                resultOffset++;
                result2[i+resultOffset] = L'S';
                rtl_freeMemory(result);
                result = result2;
                continue;
            }
            result[i+resultOffset] = rtl_char_toUpperCase(ch);
        }
    } else
    {
        for (i = 0; i < len; i++)
        {
            sal_Unicode ch = str->buffer[i];

            if ((sal_uInt16)ch == 0x00DF)               /* sharp s */
            {
                /* Grow result. */
                sal_Unicode *result2 = (sal_Unicode*)rtl_allocateMemory((len + 1 + resultOffset + 1) * sizeof(sal_Unicode));
                rtl_copyMemory(result2, result, (i + resultOffset) * sizeof(sal_Unicode));
                result2[i+resultOffset] = L'S';
                resultOffset++;
                result2[i+resultOffset] = L'S';
                rtl_freeMemory(result);
                result = result2;
                continue;
            }
            result[i+resultOffset] = rtl_char_toUpperCase(ch);
        }

    }

    result[len + resultOffset] = L'\0';
    rtl_uString_newFromStr_WithLength(newStr, result, len + resultOffset);
    rtl_freeMemory(result);
}

/*************************************************************************
 *  rtl_uString_newTrim
 */
void SAL_CALL rtl_uString_newTrim( rtl_uString ** newStr, rtl_uString * str )
{
    rtl_uString_newFromString(newStr, str);

    if (!(*newStr)) return;

    (*newStr)->length = rtl_ustr_trim_WithLength((*newStr)->buffer, (*newStr)->length);
}

/*************************************************************************
 *  rtl_uString_getTokenCount
 */
sal_Int32 SAL_CALL rtl_uString_getTokenCount( rtl_uString * str , sal_Unicode cTok)
{
    sal_Int32 count = 0;
    sal_Int32 index1 = 0;
    sal_Int32 index2 = 0;
    sal_Unicode * buffer = str->buffer;

    while ((index2 = rtl_ustr_indexOfChar_WithLength(buffer + index1, str->length - index1, cTok)) >= 0)
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
 *  rtl_uString_getToken
 */
void SAL_CALL rtl_uString_getToken( rtl_uString ** newStr , rtl_uString * str, sal_Int32 nToken, sal_Unicode cTok)
{
    sal_Int32 count = 0;
    sal_Int32 index1 = 0;
    sal_Int32 index2 = 0;
    sal_Unicode * buffer = str->buffer;

    while ((index2 = rtl_ustr_indexOfChar_WithLength(buffer + index1, str->length - index1, cTok)) >= 0)
    {
        if (count == nToken)
        {
            rtl_uString_newFromStr_WithLength( newStr, buffer + index1, index2 );
            return;
        }

        if (index2 <= (str->length - index1) && index2 >= 0)
            count++;

        index1 += index2 + 1;
    }

    if (nToken == 0 || nToken == count)
        rtl_uString_newFromStr_WithLength( newStr, buffer + index1, str->length - index1 );
    else
        rtl_uString_new(newStr);

    return;
}


/* ======================================================================= */

#define IMPL_RTL_STRCODE            sal_Unicode
#define IMPL_RTL_USTRCODE( c )      (c)
#define IMPL_RTL_STRNAME( n )       rtl_ustr_ ## n

#define IMPL_RTL_STRINGNAME( n )    rtl_uString_ ## n
#define IMPL_RTL_STRINGDATA         rtl_uString
#define IMPL_RTL_EMPTYSTRING        aImplEmpty_rtl_uString

/* ======================================================================= */

/* Include String/UString template code */

#include "strtmpl.c"

/* ======================================================================= */

static int rtl_ImplGetFastUTF8UnicodeLen( const sal_Char* pStr, sal_Int32 nLen )
{
    int             n;
    sal_uChar       c;
    const sal_Char* pEndStr;

    n = 0;
    pEndStr  = pStr+nLen;
    while ( pStr < pEndStr )
    {
        c = (sal_uChar)*pStr;

        if ( !(c & 0x80) )
            pStr++;
        else if ( (c & 0xE0) == 0xC0 )
            pStr += 2;
        else if ( (c & 0xF0) == 0xE0 )
            pStr += 3;
        else if ( (c & 0xF8) == 0xF0 )
            pStr += 4;
        else if ( (c & 0xFC) == 0xF8 )
            pStr += 5;
        else if ( (c & 0xFE) == 0xFC )
            pStr += 6;
        else
            pStr++;

        n++;
    }

    return n;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL rtl_string2UString( rtl_uString** ppThis,
                                  const sal_Char* pStr,
                                  sal_Int32 nLen,
                                  rtl_TextEncoding eTextEncoding,
                                  sal_uInt32 nCvtFlags )
{
    OSL_ENSURE( (eTextEncoding != 9) &&
                (eTextEncoding != RTL_TEXTENCODING_DONTKNOW) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS2) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS4),
                "rtl_string2UString() - Wrong TextEncoding" );

    if ( !nLen )
        rtl_uString_new( ppThis );
    else
    {
        if ( *ppThis )
            IMPL_RTL_STRINGNAME( release )( *ppThis );

        /* Optimization for US-ASCII */
        if ( eTextEncoding == RTL_TEXTENCODING_ASCII_US )
        {
            IMPL_RTL_STRCODE* pBuffer;
            *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
            pBuffer = (*ppThis)->buffer;
            do
            {
                /* Check ASCII range */
                OSL_ENSURE( ((unsigned char)*pStr) <= 127,
                            "rtl_string2UString() - Found char > 127 and RTL_TEXTENCODING_ASCII_US is specified" );

                *pBuffer = *pStr;
                pBuffer++;
                pStr++;
                nLen--;
            }
            while ( nLen );
        }
        else
        {
            rtl_uString*                pTemp;
            rtl_TextToUnicodeConverter  hConverter;
            sal_uInt32                  nInfo;
            sal_Size                    nSrcBytes;
            sal_Size                    nDestChars;
            sal_Size                    nNewLen;

            /* Optimization for UTF-8 - we try to calculate the exact length */
            /* For all other encoding we try the maximum - and reallocate
               the buffer if needed */
            if ( eTextEncoding == RTL_TEXTENCODING_UTF8 )
            {
                nNewLen = rtl_ImplGetFastUTF8UnicodeLen( pStr, nLen );
                /* Includes the string only ASCII, then we could copy
                   the buffer faster */
                if ( nNewLen == (sal_Size)nLen )
                {
                    IMPL_RTL_STRCODE* pBuffer;
                    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
                    pBuffer = (*ppThis)->buffer;
                    do
                    {
                        /* Check ASCII range */
                        OSL_ENSURE( ((unsigned char)*pStr) <= 127,
                                    "rtl_string2UString() - UTF8 test is encoding is wrong" );

                        *pBuffer = *pStr;
                        pBuffer++;
                        pStr++;
                        nLen--;
                    }
                    while ( nLen );
                    return;
                }
            }
            else
                nNewLen = nLen;

            nCvtFlags |= RTL_TEXTTOUNICODE_FLAGS_FLUSH;
            hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );

            pTemp = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
            nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                                   pStr, nLen,
                                                   pTemp->buffer, nNewLen,
                                                   nCvtFlags,
                                                   &nInfo, &nSrcBytes );

            /* Buffer not big enough, try again with enough space */
            /* Shouldn't be the case, but if we get textencoding which
               could results in more unicode characters we have this
               code here. Could be the case for apple encodings */
            while ( nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL )
            {
                rtl_freeMemory( pTemp );
                nNewLen += 8;
                pTemp = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
                nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                                       pStr, nLen,
                                                       pTemp->buffer, nNewLen,
                                                       nCvtFlags,
                                                       &nInfo, &nSrcBytes );
            }

            /* Set the buffer to the correct size or is there to
               much overhead, reallocate to the correct size */
            if ( nNewLen > nDestChars+8 )
            {
                rtl_uString* pTemp2 = IMPL_RTL_STRINGNAME( ImplAlloc )( nDestChars );
                rtl_str_ImplCopy( pTemp2->buffer, pTemp->buffer, nDestChars );
                rtl_freeMemory( pTemp );
                pTemp = pTemp2;
            }
            else
            {
                pTemp->length = nDestChars;
                pTemp->buffer[nDestChars] = 0;
            }

            rtl_destroyTextToUnicodeConverter( hConverter );
            *ppThis = pTemp;

            /* Results the conversion in an empty buffer -
               create an empty string */
            if ( pTemp && !nDestChars )
                rtl_uString_new( ppThis );
        }
    }
}
