/*************************************************************************
 *
 *  $RCSfile: string.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 14:03:39 $
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

#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#include <math.h>

sal_Int32 SAL_CALL numberToStringImplA(sal_Char * str, double d, sal_Int16 significantDigits );
sal_Int32 SAL_CALL getInfinityStrA(sal_Char * This, sal_Bool bNeg);
sal_Int32 SAL_CALL getNaNStrA(sal_Char * This);

static sal_Char RTL_STR_DIGITS[] =
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
static rtl_String aEmpty_rtl_String =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    '\0'    /* sal_Char     buffer[1];  */
};

/*************************************************************************
 *  rtl_str_getLength
 */
sal_Int32 SAL_CALL rtl_str_getLength( const sal_Char * str )
{
    const sal_Char * pTempStr = (sal_Char *)rtl_findInMemory(str, '\0', 0x80000000);

    return pTempStr - str;
}

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
 *  rtl_str_compare_WithLength
 */
sal_Int32 SAL_CALL rtl_str_compare_WithLength( const sal_Char * first,
                                               sal_Int32 firstLen,
                                               const sal_Char * second,
                                               sal_Int32 secondLen )
{
    const sal_Char * firstEnd = first + firstLen;
    const sal_Char * secondEnd = second + secondLen;
    sal_Int32 nResult = 0;
    while( first < firstEnd && second < secondEnd
        && (0 == (nResult = (sal_Int32)*first++ - (sal_Int32)*second++ ) ) )
        ;
    if( nResult )
        return nResult;

    return firstLen - secondLen;
}

/*************************************************************************
 *  rtl_str_compare
 */
sal_Int32 SAL_CALL rtl_str_compare( const sal_Char * first, const sal_Char * second )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)(*first))-((sal_Int32)(*second))) == 0) &&
            *second )
    {
        first++;
        second++;
    }

    return nRet;
}

/*************************************************************************
 *  rtl_str_hashCode_WithLength
 */
sal_Int32 SAL_CALL rtl_str_hashCode_WithLength( const sal_Char * str, sal_Int32 len )
{
    sal_Int32 h = 0;
    const sal_Char * pVal = str;
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
 *  rtl_str_hashCode
 */
sal_Int32 SAL_CALL rtl_str_hashCode( const sal_Char * str )
{
    return rtl_str_hashCode_WithLength(str, rtl_str_getLength(str));
}

/*************************************************************************
 *  rtl_str_indexOfChar_WithLength
 */
sal_Int32 SAL_CALL rtl_str_indexOfChar_WithLength( const sal_Char * str,
                                                   sal_Int32 len,
                                                   sal_Char ch )
{
    sal_Char* pTmp = (sal_Char*) rtl_findInMemory(str, ch, len);

    if (pTmp == NULL)
        /* Zeichen nicht gefunden */
        return (-1);
    else
        return(pTmp - str);
}

/*************************************************************************
 *  rtl_str_indexOfChar
 */
sal_Int32 SAL_CALL rtl_str_indexOfChar( const sal_Char * str, sal_Char ch )
{
    return rtl_str_indexOfChar_WithLength(str, rtl_str_getLength(str), ch);
}

/*************************************************************************
 *  rtl_str_lastIndexOfChar_WithLength
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar_WithLength( const sal_Char * str,
                                                       sal_Int32 len,
                                                       sal_Char ch )
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
 *  rtl_str_lastIndexOfChar
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfChar( const sal_Char * str, sal_Char ch )
{
    return rtl_str_lastIndexOfChar_WithLength(str, rtl_str_getLength(str), ch);
}

/*************************************************************************
 *  rtl_str_indexOfStr_WithLength
 */
sal_Int32 SAL_CALL rtl_str_indexOfStr_WithLength( const sal_Char * str,
                                                  sal_Int32 len,
                                                  const sal_Char * subStr,
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
 *  rtl_str_indexOfStr
 */
sal_Int32 SAL_CALL rtl_str_indexOfStr( const sal_Char * str, const sal_Char * subStr )
{
    return rtl_str_indexOfStr_WithLength( str, rtl_str_getLength(str),
                                          subStr, rtl_str_getLength(subStr) );
}

/*************************************************************************
 *  rtl_str_lastIndexOfStr_WithLength
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr_WithLength( const sal_Char * pSource,
                                                  sal_Int32 len,
                                                  const sal_Char * pSearch,
                                                  sal_Int32 nSearchLen )
{
    const sal_Char * pRunSource = pSource + len -1;
    while( pRunSource >= pSource && pRunSource - pSource >= nSearchLen )
    {
        const sal_Char * pRunSearch = pSearch + nSearchLen -1;
        const sal_Char * pSaveRunSource = pRunSource;
        while( pRunSearch >= pSearch && *pRunSearch == *(pRunSource--) )
            pRunSearch--;

        if( pRunSearch < pSearch )
            return pRunSource - pSource +1;
        pRunSource = pSaveRunSource;
        pRunSource--;
    }
    return -1;
}

/*************************************************************************
 *  rtl_str_lastIndexOfStrPtr
 */
sal_Int32 SAL_CALL rtl_str_lastIndexOfStr( const sal_Char * str, const sal_Char * subStr )
{
    return rtl_str_lastIndexOfStr_WithLength( str, rtl_str_getLength(str),
                                              subStr, rtl_str_getLength(subStr) );
}

/*************************************************************************
 *  rtl_str_replaceChar_WithLength
 */
void SAL_CALL rtl_str_replaceChar_WithLength( sal_Char * str,
                                              sal_Int32 len,
                                              sal_Char oldChar,
                                              sal_Char newChar)
{
    while ( *str )
    {
        if ( *str == oldChar)
            *str = newChar;

        str++;
    }
}

/*************************************************************************
 *  rtl_str_replaceChar
 */
void SAL_CALL rtl_str_replaceChar( sal_Char * str, sal_Char oldChar, sal_Char newChar)
{
    while ( *str )
    {
        if ( *str == oldChar)
            *str = newChar;

        str++;
    }
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
 *  rtl_str_valueOfosl_BOOL
 */
sal_Int32 SAL_CALL rtl_str_valueOfBoolean( sal_Char * str, sal_Bool b )
{
    if (b)
    {
        sal_Char *tmpStr = "True";
        rtl_copyMemory(str, tmpStr, 5);
        return 4;
    } else
    {
        sal_Char *tmpStr = "False";
        rtl_copyMemory(str, tmpStr, 6);
        return 5;
    }
}

/*************************************************************************
 *  rtl_str_valueOfChar
 */
sal_Int32 SAL_CALL rtl_str_valueOfChar( sal_Char * str, sal_Char ch )
{
    str[0] = ch;
    str[1] = L'\0';
    return 1;
}

/*************************************************************************
 *  rtl_str_valueOfosl_INT32
 */
sal_Int32 SAL_CALL rtl_str_valueOfInt32(sal_Char * str, sal_Int32 i, sal_Int16 radix )
{
    sal_Char buf[RTL_STR_MAX_VALUEOFINT32];
    sal_Bool negative = (i < 0);
    sal_Int32 charPos = RTL_STR_MAX_VALUEOFINT32 - 1;

    if (radix < RTL_STR_MIN_RADIX || radix > RTL_STR_MAX_RADIX)
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

    rtl_copyMemory(str, buf + charPos, RTL_STR_MAX_VALUEOFINT32 - charPos);

    return (RTL_STR_MAX_VALUEOFINT32 - charPos);
}

/*************************************************************************
 *  rtl_str_valueOfosl_INT64
 */
sal_Int32 SAL_CALL rtl_str_valueOfInt64(sal_Char * str, sal_Int64 l, sal_Int16 radix )
{
#ifndef SAL_INT64_IS_STRUCT
    sal_Char buf[RTL_STR_MAX_VALUEOFINT64];
    sal_Int32 charPos = RTL_STR_MAX_VALUEOFINT64 - 1;
    sal_Bool negative = (l < 0);

    if (radix < RTL_STR_MIN_RADIX || radix > RTL_STR_MAX_RADIX)
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

    rtl_copyMemory(str, buf + charPos, RTL_STR_MAX_VALUEOFINT64 - charPos );

    return (RTL_STR_MAX_VALUEOFINT64 - charPos);
#else
    return 0;
#endif
}

/*************************************************************************
 *  rtl_str_valueOfFloat
 */
#define singleSignMask  ((sal_uInt32)0x80000000)
#define singleExpMask   ((sal_uInt32)0x7f800000)
#define singleFractMask (~(singleSignMask|singleExpMask))
#define singleExpShift  ((sal_uInt32)23)

sal_Int32 SAL_CALL rtl_str_valueOfFloat( sal_Char * This, float f)
{
    /* Discover obvious special cases of NaN and Infinity
     * (like in Java Ctor FloatingDecimal( float f ) ) */
    sal_uInt32 fBits = *(sal_uInt32*)(&f);
    sal_uInt32 binExp = (sal_uInt32)( (fBits & singleExpMask) >> singleExpShift );
    sal_uInt32 fractBits = fBits & singleFractMask;
    if ( binExp == (sal_uInt32)(singleExpMask>>singleExpShift) )
    {
        if ( fractBits == 0L )
            return getInfinityStrA(This, (sal_Bool)(f < 0.0));
        else
            return getNaNStrA(This);
    }
    return numberToStringImplA( This, (double)f, 8 );
}

/*************************************************************************
 *  rtl_str_valueOfDouble
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

sal_Int32 SAL_CALL rtl_str_valueOfDouble( sal_Char * This, double d)
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
            return getInfinityStrA(This, (sal_Bool)(d < 0.0));
        else
            return getNaNStrA(This);
    }
#endif
    return numberToStringImplA( This, d, 17 );
}

/*************************************************************************
 *
 *  rtl_string_XXX Functions
 *
/*************************************************************************

/*************************************************************************
 *  rtl_string_acquire
 */
void SAL_CALL rtl_string_acquire( rtl_String * value )
{
    osl_incrementInterlockedCount(&value->refCount);
}

/*************************************************************************
 *  rtl_string_release
 */
void SAL_CALL rtl_string_release( rtl_String * value )
{
    if( 0 == osl_decrementInterlockedCount(&value->refCount) )
    {
        OSL_ENSURE( value != &aEmpty_rtl_String, "static empty string: refCount==0");
        rtl_freeMemory(value);
    }
}

/*************************************************************************
 *  rtl_string_new
 */
void SAL_CALL rtl_string_new( rtl_String ** newStr )
{
    if (*newStr)
        rtl_string_release(*newStr);

    *newStr = &aEmpty_rtl_String;
    rtl_string_acquire( *newStr );
}

/*************************************************************************
 *  rtl_string_new_WithLength
 */
void SAL_CALL rtl_string_new_WithLength( rtl_String ** newStr, sal_Int32 nLen )
{
    if (*newStr)
        rtl_string_release(*newStr);

    *newStr = (rtl_String*)rtl_allocateMemory( sizeof(rtl_String) + nLen );

    if (!(*newStr)) return;

    (*newStr)->refCount = 1;
    (*newStr)->length = 0;

    rtl_zeroMemory((*newStr)->buffer, nLen + 1);
}

/*************************************************************************
 *  rtl_string_newFromString
 */
void SAL_CALL rtl_string_newFromString( rtl_String ** newStr, rtl_String * value )
{
    rtl_String * pOrg;

    if (value->length == 0)
    {
        rtl_string_new(newStr);
        return;
    }

    pOrg = *newStr;
    *newStr = (rtl_String*)rtl_allocateMemory( sizeof(rtl_String) + value->length );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = value->length;
        rtl_copyMemory( (*newStr)->buffer, value->buffer, value->length+1 );
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_string_release(pOrg);
}

/*************************************************************************
 *  rtl_string_newFromStr
 */
void SAL_CALL rtl_string_newFromStr( rtl_String ** newStr, const sal_Char * value )
{
    rtl_String * pOrg;
    sal_Int32 length;

    if (!value)
    {
        rtl_string_new(newStr);
        return;
    }

    length = rtl_str_getLength(value);

    pOrg = *newStr;
    *newStr = (rtl_String*)rtl_allocateMemory( sizeof(rtl_String) + length );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = length;
        rtl_copyMemory((*newStr)->buffer, value, length+1 );
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_string_release(pOrg);
}

/*************************************************************************
 *  rtl_string_newFromStr_WithLength
 */
void SAL_CALL rtl_string_newFromStr_WithLength( rtl_String ** newStr, const sal_Char * value, sal_Int32 len )
{
    rtl_String * pOrg;

    if ( !value || len < 0 )
    {
        rtl_string_new(newStr);
        return;
    }

    pOrg = *newStr;
    *newStr = (rtl_String*)rtl_allocateMemory( sizeof(rtl_String) + len );
    if ( *newStr )
    {
        (*newStr)->refCount = 1;
        (*newStr)->length = len;
        rtl_copyMemory((*newStr)->buffer, value, len );
        (*newStr)->buffer[len] = 0;
    }

    /* must be done at least, if left == *newStr */
    if ( pOrg )
        rtl_string_release(pOrg);
}

/*************************************************************************
 *  rtl_string_assign
 */
void SAL_CALL rtl_string_assign( /*inout*/rtl_String ** str, rtl_String * rightValue )
{
    rtl_string_acquire(rightValue);

    if (*str)
        rtl_string_release(*str);

    *str = rightValue;
}

/*************************************************************************
 *  rtl_string_getLength
 */
sal_Int32 SAL_CALL rtl_string_getLength( rtl_String * str )
{
    return str->length;
}

/*************************************************************************
 *  rtl_string_getStr
 */
sal_Char * SAL_CALL rtl_string_getStr( rtl_String * str )
{
    return str->buffer;
}

/*************************************************************************
 *  rtl_string_newConcat
 */
void SAL_CALL rtl_string_newConcat( rtl_String ** newStr, rtl_String * left, rtl_String * right )
{
    rtl_String * pOrg = *newStr;
    if( !right->length )
    {
        *newStr = left;
        rtl_string_acquire( left );
    }
    else if( !left->length )
    {
        *newStr = right;
        rtl_string_acquire( right );
    }
    else
    {
        rtl_String *tmpStr = NULL;
        sal_Int32 nNewLen = left->length + right->length;
        rtl_string_new_WithLength(&tmpStr, nNewLen);
        tmpStr->length = nNewLen;

        rtl_copyMemory( tmpStr->buffer, left->buffer, left->length );
        rtl_copyMemory( tmpStr->buffer + left->length, right->buffer, right->length );
        tmpStr->buffer[nNewLen] = 0;
        *newStr = tmpStr;
    }
    /* must be done at least, if left == *newStr */
    if( pOrg )
        rtl_string_release(pOrg);
}

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
 *  rtl_string_newReplaceStrAt
 */
void SAL_CALL rtl_string_newReplaceStrAt( rtl_String ** newStr,
                                          rtl_String * str,
                                          sal_Int32 index,
                                          sal_Int32 count,
                                          rtl_String * newSub)
{
    sal_Char *pBuffer = NULL;

    if (count == 0 && newSub->length == 0)
    {
        rtl_string_newFromString(newStr, str);
        return;
    }

    if (count == 0)
    {
        rtl_string_new_WithLength(newStr, str->length + newSub->length);
        if (!(*newStr)) return;
        pBuffer = (*newStr)->buffer;
        (*newStr)->length = str->length + newSub->length;
        rtl_copyMemory(pBuffer, str->buffer, index);
        rtl_copyMemory(pBuffer + index, newSub->buffer, newSub->length);
        rtl_copyMemory(pBuffer + index + newSub->length, str->buffer + index, str->length - index);

        return;
    }

    if (newSub->length == 0)
    {
        rtl_string_new_WithLength(newStr, str->length - count);
        if (!(*newStr)) return;
        pBuffer = (*newStr)->buffer;
        (*newStr)->length = str->length - count;
        rtl_copyMemory(pBuffer, str->buffer, index);
        rtl_copyMemory(pBuffer + index, str->buffer + index + count, str->length - index - count);

        return;
    }

    rtl_string_new_WithLength(newStr, str->length - count + newSub->length);
    if (!(*newStr)) return;
    pBuffer = (*newStr)->buffer;
    (*newStr)->length = str->length - count + newSub->length;
    rtl_copyMemory(pBuffer, str->buffer, index);
    rtl_copyMemory(pBuffer + index, newSub->buffer, newSub->length);
    rtl_copyMemory(pBuffer + index + newSub->length, str->buffer + index + count,
                   str->length - index - count);

    return;
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

/****************************************************************************
* Internally used functions
*/
sal_Int32 SAL_CALL numberToStringImplA(sal_Char * str, double d, sal_Int16 significantDigits )
{
    /* Make it very simple without any formatting,
     * (similar to Double.toString() in Java) */
    sal_Char buf[ RTL_STR_MAX_VALUEOFDOUBLE ];
    sal_Char* charPos = buf;
    sal_Int16 i, len, dig, dotPos, tmpDot;
    sal_Int16 lastNonZeroPos;
    sal_Int16 nExpDigits;
    sal_Bool bExp, bDotSet;
    double dExp, rem;

    if( d == 0 )
    {
        *(charPos++) = '0';
        *(charPos++) = '.';
        *(charPos++) = '0';
    }
    else
    {
        if( d < 0 )
        {
            *(charPos++) = '-';
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

        bDotSet= sal_False;
        for( i = 0 ; i < significantDigits ; i++ )
        {
            // handle leading zeros
            if( dotPos < 0)
            {
                *(charPos++) = '0';
                *(charPos++) = '.';
                while( ++dotPos < 0)
                   *(charPos++) = '0';
                bDotSet= sal_True;
            }
            dig = (sal_Int16)d;
            if( dig )
                lastNonZeroPos = i;
//            *(charPos++) = L'0' + dig;
            *(charPos++) = dig + 0x30;
            if( i == dotPos && ! bDotSet)
                *(charPos++) = '.';
            d -= (double)dig;
            d *= 10.0;
        }

        /* Kill trailing zeros */
        if( lastNonZeroPos > dotPos + 1 )
            charPos -= (significantDigits - 1 - lastNonZeroPos);

        /* exponent */
        if( bExp )
        {
            *(charPos++) = 'E';
            if( dExp < 0.0 )
            {
                dExp = -dExp;
                *(charPos++) = '-';
            }

            nExpDigits = 1;
            while( dExp >= 10.0 )
            {
                nExpDigits++;
                dExp /= 10;
            }
            for( i = 0 ; i < nExpDigits ; i++ )
            {
                dig = (sal_Int16)dExp; // sometimes if the debugger shows dExp= 2, the cast produces 1
                rem= fmod( dExp, 1);
                if( rem >0.999) // max exponent is about 357
                    dig++;

                *(charPos++) = '0' + dig;
                dExp -= (double)dig;
                dExp *= 10.0;
            }
        }
    }

    *(charPos++) = 0;
    len = charPos - buf;
    rtl_copyMemory( str, buf, len * sizeof(sal_Char) );
    return len - 1;
}

/*************************************************************************/

sal_Int32 SAL_CALL getInfinityStrA(sal_Char * This, sal_Bool bNeg)
{
    sal_Char InfinityStr[] = "-Infinity";
    sal_Char* pStr = bNeg ? InfinityStr : InfinityStr + 1;
    sal_Int32 len = bNeg ? 9 : 8;
    sal_Int32 i;
    for ( i = 0; i < len+1; i++ )
        *(This+i) = *(pStr+i);
    return len;
}

sal_Int32 SAL_CALL getNaNStrA(sal_Char * This)
{
    sal_Char NaNStr[] = "NaN";
    sal_Int32 len = 3;
    sal_Int32 i;
    for ( i = 0; i < len+1; i++ )
        *(This+i) = NaNStr[i];
    return len;
}

