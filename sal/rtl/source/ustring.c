/*************************************************************************
 *
 *  $RCSfile: ustring.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:24 $
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

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_LOCALE_H_
#include <rtl/locale.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#include <math.h>


static sal_Char const RTL_STR_DIGITS[] =
{
    '0' , '1' , '2' , '3' , '4' , '5' ,
    '6' , '7' , '8' , '9' , 'a' , 'b' ,
    'c' , 'd' , 'e' , 'f' , 'g' , 'h' ,
    'i' , 'j' , 'k' , 'l' , 'm' , 'n' ,
    'o' , 'p' , 'q' , 'r' , 's' , 't' ,
    'u' , 'v' , 'w' , 'x' , 'y' , 'z'
};

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
static rtl_uString aEmpty_rtl_wString =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    0       /* sal_Unicode  buffer[1];  */
};

/*************************************************************************
 *  rtl_ustr_getLength
 */
sal_Int32 SAL_CALL rtl_ustr_getLength( const sal_Unicode * str )
{
    const sal_Unicode * pTempStr = str;
    while( *pTempStr ) pTempStr++;
    return pTempStr - str;
}

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
sal_Int32 SAL_CALL rtl_ustr_compare_WithLength( const sal_Unicode * first,
                                                sal_Int32 firstLen,
                                                const sal_Unicode * second,
                                                sal_Int32 secondLen )
{
    const sal_Unicode * firstEnd = first + firstLen;
    const sal_Unicode * secondEnd = second + secondLen;
    sal_Int32 nResult = 0;
    while( first < firstEnd && second < secondEnd
        && (0 == (nResult = (sal_Int32)*first++ - (sal_Int32)*second++ ) ) )
        ;
    if( nResult )
        return nResult;
    return firstLen - secondLen;
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
 *  rtl_ustr_compare
 */
sal_Int32 SAL_CALL rtl_ustr_compare( const sal_Unicode * first, const sal_Unicode * second )
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
        OSL_ENSHURE( (*(second-1) & 0x80) == 0, "Found ASCII char > 127");
    }
    if( !nResult && (shortenedLength != -1) )
    {
        if( *second )
        {
            OSL_ENSHURE( first == firstEnd, "first == firstEnd failed" );
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
 *  rtl_ustr_hashCode_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_hashCode_WithLength( const sal_Unicode * str, sal_Int32 len )
{
    sal_Int32 h = 0;
    const sal_Unicode * pVal = str;
    sal_Int32 length = len;

    if( length < 16 )
    {
        while( *pVal )
            h = (h * 37) + *pVal++;
    }
    else
    {
        sal_Int32 off = 0;
        sal_Int32 i;
        /* only sample some characters */
        sal_Int32 skip = length / 8;
        for( i = length ; i > 0; i -= skip, off += skip )
        {
            h = (h * 39) + pVal[off];
        }
    }

    return h;
}

/*************************************************************************
 *  rtl_ustr_hashCode
 */
sal_Int32 SAL_CALL rtl_ustr_hashCode( const sal_Unicode * str )
{
    return rtl_ustr_hashCode_WithLength(str, rtl_ustr_getLength(str));
}

/*************************************************************************
 *  rtl_ustr_indexOfChar_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfChar_WithLength( const sal_Unicode * str,
                                                    sal_Int32 len,
                                                    sal_Unicode ch )
{
    sal_Int32 index = 0;

    while ( index != len )
    {
        if (!(*str))
            return -1;;

        if ( *str == ch )
            /* Zeichen gefunden */
            return index;

        /* Naechstes Zeichen */
        index++;
        str++;
    }

    /* Zeichen nicht gefunden */
    return -1;
}

/*************************************************************************
 *  rtl_ustr_indexOfChar
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfChar( const sal_Unicode * str, sal_Unicode ch )
{
    return rtl_ustr_indexOfChar_WithLength(str, rtl_ustr_getLength(str), ch);
}

/*************************************************************************
 *  rtl_ustr_lastIndexOfChar_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar_WithLength( const sal_Unicode * str,
                                                        sal_Int32 len,
                                                        sal_Unicode ch )
{
    sal_Int32 index = len - 1;

    while ( index >= 0 )
    {
        if ( *(str + index) == ch )
            /* Zeichen gefunden */
            return index;

        /* Naechstes Zeichen */
        index--;
    }

    /* Zeichen nicht gefunden */
    return -1;
}

/*************************************************************************
 *  rtl_ustr_lastIndexOfChar
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfChar( const sal_Unicode * str, sal_Unicode ch )
{
    return rtl_ustr_lastIndexOfChar_WithLength(str, rtl_ustr_getLength(str), ch);
}

/*************************************************************************
 *  rtl_ustr_indexOfStr_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfStr_WithLength( const sal_Unicode * str,
                                                   sal_Int32 len,
                                                   const sal_Unicode * subStr,
                                                   sal_Int32 subLen )
{
    sal_Int32 index = 0;

    if (len < subLen)
        return -1;

    while ( *str )
    {
        if ( *str == *subStr)
        {
            sal_Int32 offset = 0;

            /* wenn die restliche Laenge kleiner als der zu suchende string ist
               kann der nicht mehr gefunden werden */
            if ((len - index) < subLen)
                return -1;

            while ( offset != subLen)
            {
                if ( *(str + offset) != *(subStr + offset))
                    break;

                offset++;
            }

            /* Schleife komplett durchlaufen, d.h. string gefunden */
            if ( offset == subLen)
                return index;
        }

        index++;
        str++;
    }

    return -1;
}

/*************************************************************************
 *  rtl_ustr_indexOfStr
 */
sal_Int32 SAL_CALL rtl_ustr_indexOfStr( const sal_Unicode * str, const sal_Unicode * subStr )
{
    return rtl_ustr_indexOfStr_WithLength( str, rtl_ustr_getLength(str),
                                           subStr, rtl_ustr_getLength(subStr) );
}

/*************************************************************************
 *  rtl_ustr_lastIndexOfStr_WithLength
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr_WithLength( const sal_Unicode * pSource,
                                                       sal_Int32 len,
                                                       const sal_Unicode * pSearch,
                                                       sal_Int32 nSearchLen )
{
    const sal_Unicode * pRunSource = pSource + len -1;
    while( pRunSource >= pSource && pRunSource - pSource >= nSearchLen )
    {
        const sal_Unicode * pRunSearch = pSearch + nSearchLen -1;
        const sal_Unicode * pSaveRunSource = pRunSource;
        while( pRunSearch >= pSearch && *(pRunSearch--) == *(pRunSource--) )
            ;
        if( pRunSearch < pSearch )
            return pRunSource - pSource +1;
        pRunSource = pSaveRunSource;
        pRunSource--;
    }
    return -1;
}

/*************************************************************************
 *  rtl_ustr_lastIndexOfStrPtr
 */
sal_Int32 SAL_CALL rtl_ustr_lastIndexOfStr( const sal_Unicode * str, const sal_Unicode * subStr )
{
    return rtl_ustr_lastIndexOfStr_WithLength( str, rtl_ustr_getLength(str),
                                               subStr, rtl_ustr_getLength(subStr) );
}

/*************************************************************************
 *  rtl_ustr_replaceChar_WithLength
 */
void SAL_CALL rtl_ustr_replaceChar_WithLength( sal_Unicode * str,
                                               sal_Int32 len,
                                               sal_Unicode oldChar,
                                               sal_Unicode newChar)
{
    while ( *str )
    {
        if ( *str == oldChar)
            *str = newChar;

        str++;
    }
}

/*************************************************************************
 *  rtl_ustr_replaceChar
 */
void SAL_CALL rtl_ustr_replaceChar( sal_Unicode * str, sal_Unicode oldChar, sal_Unicode newChar)
{
    while ( *str )
    {
        if ( *str == oldChar)
            *str = newChar;

        str++;
    }
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
 *  rtl_ustr_valueOfosl_BOOL
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfBoolean( sal_Unicode * str, sal_Bool b )
{
    if (b)
    {
        sal_Unicode *tmpStr = L"True";
        rtl_copyMemory(str, tmpStr, 5 * sizeof(sal_Unicode));
        return 4;
    } else
    {
        sal_Unicode *tmpStr = L"False";
        rtl_copyMemory(str, tmpStr, 6 * sizeof(sal_Unicode));
        return 5;
    }
}

/*************************************************************************
 *  rtl_ustr_valueOfChar
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfChar( sal_Unicode * str, sal_Unicode ch )
{
    str[0] = ch;
    str[1] = L'\0';
    return 1;
}

/*************************************************************************
 *  rtl_ustr_valueOfosl_INT32
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfInt32(sal_Unicode * str, sal_Int32 i, sal_Int16 radix )
{
    sal_Unicode buf[RTL_USTR_MAX_VALUEOFINT32];
    sal_Bool negative = (i < 0);
    sal_Int32 charPos = RTL_USTR_MAX_VALUEOFINT32 - 1;

    if (radix < RTL_USTR_MIN_RADIX || radix > RTL_USTR_MAX_RADIX)
        radix = 10;

    if (!negative) {
        i = -i;
    }

    while (i <= -radix) {
        buf[charPos--] = RTL_STR_DIGITS[-(i % radix)];
        i = i / radix;
    }
    buf[charPos] = RTL_STR_DIGITS[-i];

    if (negative) {
        buf[--charPos] = '-';
    }

    rtl_copyMemory(str, buf + charPos, (RTL_USTR_MAX_VALUEOFINT32 - charPos) * sizeof(sal_Unicode));

    return (RTL_USTR_MAX_VALUEOFINT32 - charPos);
}

/*************************************************************************
 *  rtl_ustr_valueOfosl_INT64
 */
sal_Int32 SAL_CALL rtl_ustr_valueOfInt64(sal_Unicode * str, sal_Int64 l, sal_Int16 radix )
{
#ifndef SAL_INT64_IS_STRUCT
    sal_Unicode buf[RTL_USTR_MAX_VALUEOFINT64];
    sal_Int32 charPos = RTL_USTR_MAX_VALUEOFINT64 - 1;
    sal_Bool negative = (l < 0);

    if (radix < RTL_USTR_MIN_RADIX || radix > RTL_USTR_MAX_RADIX)
        radix = 10;

    if (!negative) {
        l = -l;
    }

    while (l <= -radix) {
        buf[charPos--] = RTL_STR_DIGITS[(sal_Int32)(-(l % radix))];
        l = l / radix;
    }
    buf[charPos] = RTL_STR_DIGITS[(sal_Int32)(-l)];

    if (negative) {
        buf[--charPos] = '-';
    }

    rtl_copyMemory(str, buf + charPos, (RTL_USTR_MAX_VALUEOFINT64 - charPos) * sizeof(sal_Unicode));

    return (RTL_USTR_MAX_VALUEOFINT64 - charPos);
#else
    return 0;
#endif
}

/*************************************************************************
 *  numberToStringImpl
 */

#define FLOAT_MAX_POS       ( 3.402823e+38)
#define FLOAT_MAX_NEG       (-3.402823e+38)
#define FLOAT_MIN_POS       ( 1.175494351e-38)
#define FLOAT_MIN_NEG       (-1.175494351e-38)


sal_Int32 SAL_CALL numberToStringImpl(sal_Unicode * str, double d, sal_Int16 significantDigits )
{
    /* Make it very simple without any formatting,
     * (similar to Double.toString() in Java) */
    sal_Unicode buf[ RTL_USTR_MAX_VALUEOFDOUBLE ];
    sal_Unicode* charPos = buf;
    sal_Int16 i, len, dig, dotPos;
    sal_Int16 lastNonZeroPos;
    sal_Int16 nExpDigits;
    sal_Bool bExp;
    double dExp;

    if( d == 0 )
    {
        *(charPos++) = L'0';
        *(charPos++) = L'.';
        *(charPos++) = L'0';
    }
    else
    {
        if( d < 0 )
        {
            *(charPos++) = L'-';
            d = -d;
        }

        dExp = log10( d );
        bExp = sal_False;
        if( dExp > 7 || dExp <= -7 )
            bExp = sal_True;

        dExp = floor( dExp );
        d /= pow( 10, dExp );
        while( d > 10 )
        {
            d /= 10.0;
            dExp += 1.0;
        }

        if( d < 1.0 )
            significantDigits++;

        dotPos = bExp ? 0 : (sal_Int16)dExp;
        lastNonZeroPos = 0;
        for( i = 0 ; i < significantDigits ; i++ )
        {
            dig = (sal_Int16)d;
            if( dig )
                lastNonZeroPos = i;
            *(charPos++) = L'0' + dig;
            if( i == dotPos )
                *(charPos++) = L'.';
            d -= (double)dig;
            d *= 10.0;
        }

        /* Kill trailing zeros */
        if( lastNonZeroPos > dotPos + 1 )
            charPos -= (significantDigits - 1 - lastNonZeroPos);

        /* exponent */
        if( bExp )
        {
            *(charPos++) = L'E';
            if( dExp < 0.0 )
            {
                dExp = -dExp;
                *(charPos++) = L'-';
            }

            nExpDigits = 1;
            while( dExp >= 10.0 )
            {
                nExpDigits++;
                dExp /= 10;
            }
            for( i = 0 ; i < nExpDigits ; i++ )
            {
                dig = (sal_Int16)dExp;
                *(charPos++) = L'0' + dig;
                dExp -= (double)dig;
                dExp *= 10.0;
            }
        }
    }

    *(charPos++) = 0;
    len = charPos - buf;
    rtl_copyMemory( str, buf, len * sizeof(sal_Unicode) );
    return len - 1;
}

/*************************************************************************/

sal_Int32 SAL_CALL getInfinityStr(sal_Unicode * This, sal_Bool bNeg)
{
    sal_Char InfinityStr[] = "-Infinity";
    sal_Char* pStr = bNeg ? InfinityStr : InfinityStr + 1;
    sal_Int32 len = bNeg ? 9 : 8;
    sal_Int32 i;
    for ( i = 0; i < len+1; i++ )
        *(This+i) = *(pStr+i);
    return len;
}

sal_Int32 SAL_CALL getNaNStr(sal_Unicode * This)
{
    sal_Char NaNStr[] = "NaN";
    sal_Int32 len = 3;
    sal_Int32 i;
    for ( i = 0; i < len+1; i++ )
        *(This+i) = NaNStr[i];
    return len;
}

/*************************************************************************
 *  rtl_ustr_valueOfFloat
 */
#define singleSignMask  ((sal_uInt32)0x80000000)
#define singleExpMask   ((sal_uInt32)0x7f800000)
#define singleFractMask (~(singleSignMask|singleExpMask))
#define singleExpShift  ((sal_uInt32)23)

sal_Int32 SAL_CALL rtl_ustr_valueOfFloat(sal_Unicode * This, float f)
{
    /* Discover obvious special cases of NaN and Infinity
     * (like in Java Ctor FloatingDecimal( float f ) ) */
    sal_uInt32 fBits = *(sal_uInt32*)(&f);
    sal_uInt32 binExp = (sal_uInt32)( (fBits & singleExpMask) >> singleExpShift );
    sal_uInt32 fractBits = fBits & singleFractMask;
    if ( binExp == (sal_uInt32)(singleExpMask>>singleExpShift) )
    {
        if ( fractBits == 0L )
            return getInfinityStr(This, (sal_Bool)(f < 0.0));
        else
            return getNaNStr(This);
    }
    return numberToStringImpl( This, (double)f, 8 );
}

/*************************************************************************
 *  rtl_ustr_valueOfDouble
 */
#ifndef WNT
#define signMask    ((sal_uInt64)0x8000000000000000LL)
#define expMask     ((sal_uInt64)0x7ff0000000000000LL)
#else
#define signMask    ((sal_uInt64)0x8000000000000000L)
#define expMask     ((sal_uInt64)0x7ff0000000000000L)
#endif

#define fractMask   (~(signMask|expMask))
#define expShift    ((sal_uInt32)52)

sal_Int32 SAL_CALL rtl_ustr_valueOfDouble(sal_Unicode * This, double d)
{
#ifndef SAL_INT64_IS_STRUCT
    /* Discover obvious special cases of NaN and Infinity.
     * (like in Java Ctor FloatingDecimal( double d ) ) */
    sal_uInt64 dBits = *(sal_uInt64*)(&d);
    sal_uInt32 binExp = (sal_uInt32)( (dBits & expMask) >> expShift );
    sal_uInt64 fractBits = dBits & fractMask;
    if ( binExp == (int)(expMask >> expShift) )
    {
        if ( fractBits == 0L )
            return getInfinityStr(This, (sal_Bool)(d < 0.0));
        else
            return getNaNStr(This);
    }
#endif
    return numberToStringImpl( This, d, 17 );
}

/*************************************************************************
 *  rtl_ustr_toInt32
 */
sal_Int32 SAL_CALL rtl_ustr_toInt32( sal_Unicode * str, sal_Int16 radix )
{
    sal_Char* stopstring;
    sal_Char* CharStr;
    sal_Char* pChar;
    sal_Int32 length, i, nVal;
    sal_Unicode wc;

    /* simple conversion to 8 bit string */
    length = rtl_ustr_getLength( str );
    CharStr = (sal_Char*)rtl_allocateMemory( length + 1 );
    pChar = CharStr;
    for( i = 0 ; i < length ; i++ )
    {
        wc = *(str++);
        *(pChar++) = ( wc <= 255 ) ? (sal_Char)wc : 0;
    }
    CharStr[ length ] = 0;
    nVal = strtol( CharStr, &stopstring, radix );
    rtl_freeMemory( CharStr );
    return nVal;
}

/*************************************************************************
 *  Help function for rtl_ustr_toInt64
 */
sal_Int16 getDigit( sal_Unicode ch, sal_Int16 radix )
{
    sal_Int16 value = -1;
    if( ch >= L'0' && ch <= L'9' )
        value = ch - L'0';
    else if( ch >= L'a' && ch <= L'z' )
        value = ch - L'a' + 10;
    else if( ch >= L'A' && ch <= L'Z' )
        value = ch - L'A' + 10;
    return ( value < radix ) ? value : -1;
}

#ifndef WNT
#define INT64_MIN_VALUE 0x8000000000000000LL;
#define INT64_MAX_VALUE 0x7fffffffffffffffLL;
#else
#define INT64_MIN_VALUE 0x8000000000000000L;
#define INT64_MAX_VALUE 0x7fffffffffffffffL;
#endif

/*************************************************************************
 *  rtl_ustr_toInt64 (like in Java)
 */
sal_Int64 SAL_CALL rtl_ustr_toInt64( sal_Unicode * str, sal_Int16 radix )
{
#ifndef SAL_INT64_IS_STRUCT
    sal_Int64 result = 0;
    sal_Bool negative = sal_False;
    sal_Int32 i = 0;
    sal_Int32 max = rtl_ustr_getLength( str );
    sal_Int64 limit;
    sal_Int64 multmin;
    sal_Int32 digit;

    if( max == 0 )
        return 0;

    if( radix < RTL_USTR_MIN_RADIX || radix > RTL_USTR_MAX_RADIX )
        radix = 10;

    while( i < max && (str[i] == L' ' || str[i] == L'\t') )
        i++;

    if( str[i] == L'-' )
    {
        negative = sal_True;
        limit = INT64_MIN_VALUE;
        i++;
    }
    else
    {
        limit = -INT64_MAX_VALUE;
    }
    multmin = limit / radix;
    if( i < max )
    {
        digit = getDigit( str[i++], radix );
        if( digit < 0 )
            return 0;
        result = -digit;
    }
    while( i < max )
    {
        // Accumulating negatively avoids surprises near MAX_VALUE
        digit = getDigit( str[i++], radix );
        if( digit < 0 )
            break;

        if( result < multmin )
            break;

        result *= radix;
        if( result < limit + digit )
            break;

        result -= digit;
    }
    if( negative )
        return result;
    else
        return -result;

#endif
    return 0;
}

/*************************************************************************
 *  rtl_ustr_toFloat
 */
float SAL_CALL rtl_ustr_toFloat( sal_Unicode * str )
{
    sal_Char* stopstring;
    sal_Char* CharStr;
    sal_Char* pChar;
    sal_Int32 length, i;
    sal_Unicode wc;
    double dVal;

    /* simple conversion to 8 bit string */
    length = rtl_ustr_getLength( str );
    CharStr = (sal_Char*)rtl_allocateMemory( length + 1);
    pChar = CharStr;
    for( i = 0 ; i < length ; i++ )
    {
        wc = *(str++);
        *(pChar++) = ( wc <= 255 ) ? (sal_Char)wc : 0;
    }
    CharStr[ length ] = 0;
    dVal = strtod( CharStr, &stopstring );

    /* float range? */
    if( dVal > FLOAT_MAX_POS ||
        dVal < FLOAT_MAX_NEG ||
        ( dVal > 0 && dVal < FLOAT_MIN_POS ) ||
        ( dVal < 0 && dVal > FLOAT_MIN_NEG ) )
    {
        dVal = 0.0;
    }
    rtl_freeMemory( CharStr );
    return (float)dVal;
}

/*************************************************************************
 *  rtl_ustr_toDouble
 */
double SAL_CALL rtl_ustr_toDouble( sal_Unicode * str )
{
    sal_Char* stopstring;
    sal_Char* CharStr;
    sal_Char* pChar;
    sal_Int32 length, i;
    sal_Unicode wc;
    double dVal;

    /* simple conversion to 8 bit string */
    length = rtl_ustr_getLength( str );
    CharStr = (sal_Char*)rtl_allocateMemory( length + 1 );
    pChar = CharStr;
    for( i = 0 ; i < length ; i++ )
    {
        wc = *(str++);
        *(pChar++) = ( wc <= 255 ) ? (sal_Char)wc : 0;
    }
    CharStr[ length ] = 0;
    dVal = strtod( CharStr, &stopstring );
    rtl_freeMemory( CharStr );
    return dVal;
}


/*************************************************************************
 *
 *  rtl_uString_XXX Functions
 *
/*************************************************************************

/*************************************************************************
 *  rtl_uString_acquire
 */
void SAL_CALL rtl_uString_acquire( rtl_uString * value )
{
    osl_incrementInterlockedCount(&value->refCount);
}

/*************************************************************************
 *  rtl_uString_release
 */
void SAL_CALL rtl_uString_release( rtl_uString * value )
{
    if( 0 == osl_decrementInterlockedCount(&value->refCount) )
    {
        OSL_ENSHURE( value != &aEmpty_rtl_wString, "static empty string: refCount==0");
        rtl_freeMemory(value);
    }
}

/*************************************************************************
 *  rtl_uString_new
 */
void SAL_CALL rtl_uString_new( rtl_uString ** newStr )
{
    if (*newStr)
        rtl_uString_release(*newStr);

    *newStr = &aEmpty_rtl_wString;
    rtl_uString_acquire( *newStr );
}

/*************************************************************************
 *  rtl_uString_new_WithLength
 */
void SAL_CALL rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen )
{
    if (*newStr)
        rtl_uString_release(*newStr);

    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (nLen * sizeof(sal_Unicode)) );

    if (!(*newStr)) return;

    (*newStr)->refCount = 1;
    (*newStr)->length = 0;
    *((*newStr)->buffer + nLen) = L'\0';

    rtl_zeroMemory((*newStr)->buffer, nLen * sizeof(sal_Unicode));
}

/*************************************************************************
 *  rtl_uString_newFromString
 */
void SAL_CALL rtl_uString_newFromString( rtl_uString ** newStr, rtl_uString * value )
{
    rtl_uString * pOrg;

    if (value->length == 0)
    {
        rtl_uString_new(newStr);
        return;
    }

    pOrg = *newStr;
    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (value->length * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = value->length;
        rtl_copyMemory((*newStr)->buffer, value->buffer, (value->length+1) * sizeof(sal_Unicode));
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_uString_release( pOrg );
}

/*************************************************************************
 *  rtl_uString_newFromStr
 */
void SAL_CALL rtl_uString_newFromStr( rtl_uString ** newStr, const sal_Unicode * value )
{
    rtl_uString * pOrg;
    sal_Int32 length;

    if (!value)
    {
        rtl_uString_new(newStr);
        return;
    }

    length = rtl_ustr_getLength(value);

    pOrg = *newStr;
    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (length * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = length;
        rtl_copyMemory((*newStr)->buffer, value, (length+1) * sizeof(sal_Unicode));
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_uString_release( pOrg );
}

/*************************************************************************
 *  rtl_uString_newFromStr_WithLength
 */
void SAL_CALL rtl_uString_newFromStr_WithLength( rtl_uString ** newStr,
                                                 const sal_Unicode * value,
                                                 sal_Int32 len)
{
    rtl_uString * pOrg;

    if  (!value || len < 0 )
    {
        rtl_uString_new(newStr);
        return;
    }

    pOrg = *newStr;
    *newStr = (rtl_uString*)rtl_allocateMemory( sizeof(rtl_uString) + (len * sizeof(sal_Unicode)) );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = len;
        rtl_copyMemory((*newStr)->buffer, value, len * sizeof(sal_Unicode));
        (*newStr)->buffer[len] = 0;
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_uString_release( pOrg );
}

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
            OSL_ENSHURE( (*value & 0x80) == 0, "Found ASCII char > 127");

            *p = (sal_Unicode)(*value);
            p++;
            value++;
            length--;
        }
        *p = 0;
    }
}

/*************************************************************************
 *  rtl_uString_assign
 */
void SAL_CALL rtl_uString_assign( /*inout*/rtl_uString ** str, rtl_uString * rightValue )
{
    rtl_uString_acquire(rightValue);

    if (*str)
        rtl_uString_release(*str);

    *str = rightValue;
}

/*************************************************************************
 *  rtl_uString_getLength
 */
sal_Int32 SAL_CALL rtl_uString_getLength( rtl_uString * str )
{
    return str->length;
}

/*************************************************************************
 *  rtl_uString_getStr
 */
sal_Unicode * SAL_CALL rtl_uString_getStr( rtl_uString * str )
{
    return str->buffer;
}

/*************************************************************************
 *  rtl_uString_newConcat
 */
void SAL_CALL rtl_uString_newConcat( rtl_uString ** newStr, rtl_uString * left, rtl_uString * right )
{
    rtl_uString * pOrg = *newStr;
    if( !right->length )
    {
        *newStr = left;
        rtl_uString_acquire( left );
    }
    else if( !left->length )
    {
        *newStr = right;
        rtl_uString_acquire( right );
    }
    else
    {
        rtl_uString *tmpStr = NULL;
        sal_Int32 nNewLen = left->length + right->length;
        rtl_uString_new_WithLength(&tmpStr, nNewLen);
        tmpStr->length = nNewLen;

        rtl_copyMemory( tmpStr->buffer, left->buffer, left->length * sizeof(sal_Unicode));
        rtl_copyMemory( tmpStr->buffer + left->length, right->buffer, right->length * sizeof(sal_Unicode));
        tmpStr->buffer[nNewLen] = 0;
        *newStr = tmpStr;
    }
    /* must be done at least, if left == *newStr */
    if( pOrg )
        rtl_uString_release(pOrg);
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
 *  rtl_uString_newReplaceStrAt
 */
void SAL_CALL rtl_uString_newReplaceStrAt( rtl_uString ** newStr,
                                           rtl_uString * str,
                                           sal_Int32 index,
                                           sal_Int32 count,
                                           rtl_uString * newSub)
{
    sal_Unicode *pBuffer = NULL;
    sal_Int32 size = sizeof(sal_Unicode);

    if (count == 0 && newSub->length == 0)
    {
        rtl_uString_newFromString(newStr, str);
        return;
    }

    if (count == 0)
    {
        rtl_uString_new_WithLength(newStr, str->length + newSub->length);
        if (!(*newStr)) return;
        pBuffer = (*newStr)->buffer;
        (*newStr)->length = str->length + newSub->length;
        rtl_copyMemory(pBuffer, str->buffer, index * size);
        rtl_copyMemory(pBuffer + index, newSub->buffer, newSub->length * size);
        rtl_copyMemory(pBuffer + index + newSub->length, str->buffer + index,
                       (str->length - index) * size);

        return;
    }

    if (newSub->length == 0)
    {
        rtl_uString_new_WithLength(newStr, str->length - count);
        if (!(*newStr)) return;
        pBuffer = (*newStr)->buffer;
        (*newStr)->length = str->length - count;
        rtl_copyMemory(pBuffer, str->buffer, index * size);
        rtl_copyMemory(pBuffer + index, str->buffer + index + count,
                       (str->length - index - count) * size);

        return;
    }

    rtl_uString_new_WithLength(newStr, str->length - count + newSub->length);
    if (!(*newStr)) return;
    pBuffer = (*newStr)->buffer;
    (*newStr)->length = str->length - count + newSub->length;
    rtl_copyMemory(pBuffer, str->buffer, index * size);
    rtl_copyMemory(pBuffer + index, newSub->buffer, newSub->length * size);
    rtl_copyMemory(pBuffer + index + newSub->length, str->buffer + index + count,
                   (str->length - index - count) * size);

    return;
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


void SAL_CALL rtl_string2UString( rtl_uString** newStr, const sal_Char* pStr, sal_Int32 nLen,
                                  rtl_TextEncoding encoding, sal_uInt32 nCvtFlags )
{
    if (nLen)
    {
        sal_Unicode*                pData;
        rtl_TextEncodingInfo        aTextEncInfo;
        rtl_TextToUnicodeConverter  hConverter = rtl_createTextToUnicodeConverter( encoding );
        sal_uInt32                  nInfo;
        sal_Size                    nSrcBytes;
        sal_Size                    nDestChars;
        sal_Size                    nNewLen;

        /* get TextEncodingInfo */
        aTextEncInfo.StructSize = sizeof( aTextEncInfo );
        rtl_getTextEncodingInfo( encoding, &aTextEncInfo );

        /* Zuerst konvertieren wir mit der wahrscheinlichen Anzahl */
        /* der zu konvertierenden Zeichen */
        nNewLen = nLen; /* (aTextEncInfo.AverageCharSize ? aTextEncInfo.AverageCharSize : 1); */

        pData = (sal_Unicode*)rtl_allocateMemory( (1 + nNewLen) * sizeof(sal_Unicode) );
        nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                               pStr, nLen,
                                               pData, nNewLen,
                                               nCvtFlags,
                                               &nInfo, &nSrcBytes );

        /* String entsprechend der durch das Konvertieren tatsaechlich */
        /* entstehenden Bytes anpassen */
        pData[nDestChars] = L'\0';

        rtl_uString_newFromStr_WithLength(newStr, pData, nDestChars);

        rtl_destroyTextToUnicodeConverter( hConverter );
        rtl_freeMemory( pData );
    } else
    {
        rtl_uString_new(newStr);
    }
}

void SAL_CALL rtl_uString2String( rtl_String** newStr, const sal_Unicode * pWStr, sal_Int32 nWLen,
                                  rtl_TextEncoding encoding, sal_uInt32 nCvtFlags )
{
    if (nWLen)
    {
        sal_Char*                   pData;
        rtl_TextEncodingInfo        aTextEncInfo;
        rtl_UnicodeToTextConverter  hConverter = rtl_createUnicodeToTextConverter( encoding );
        sal_Size                    nSrcChars;
        sal_Size                    nDestBytes;
        sal_Size                    nNewLen;
        sal_uInt32                  nInfo;

        /* get TextEncodingInfo */
        aTextEncInfo.StructSize = sizeof( aTextEncInfo );
        rtl_getTextEncodingInfo( encoding, &aTextEncInfo );

        /* Zuerst konvertieren wir mit der wahrscheinlichen Anzahl */
        /* der zu konvertierenden Zeichen */
        nNewLen = nWLen * (aTextEncInfo.AverageCharSize ? aTextEncInfo.AverageCharSize : 1) * 6;

        pData = (sal_Char*)rtl_allocateMemory( nNewLen );
        nDestBytes = rtl_convertUnicodeToText( hConverter, 0,
                                               pWStr, nWLen,
                                               pData, nNewLen,
                                               nCvtFlags,
                                               &nInfo, &nSrcChars );
        pData[nDestBytes] = '\0';

        rtl_string_newFromStr_WithLength(newStr, pData, nDestBytes);

        rtl_destroyUnicodeToTextConverter( hConverter );
        rtl_freeMemory( pData );
    } else
    {
        rtl_string_new(newStr);
    }
}


