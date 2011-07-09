/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
#
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
#*************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <math.h>
#include <stdlib.h>

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H_
    #include <rtl/ustring.h>
#endif

#ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
#endif

#ifndef _RTL_STRING_UTILS_CONST_H_
    #include <rtl_String_Utils_Const.h>
#endif

using ::rtl::OString;
sal_uInt32 AStringLen( const sal_Char *pAStr )
{
    sal_uInt32  nStrLen = 0;

    if ( pAStr != NULL )
    {
        const sal_Char *pTempStr = pAStr;

        while( *pTempStr )
        {
            pTempStr++;
        } // while

        nStrLen = (sal_uInt32)( pTempStr - pAStr );
    } // if

    return nStrLen;
} // AStringLen
/* disable assignment within condition expression */
#ifdef WNT
#pragma warning( disable : 4706 )
#endif
sal_Char* cpystr( sal_Char* dst, const sal_Char* src )
{
    const sal_Char* psrc = src;
    sal_Char* pdst = dst;

    while( (*pdst++ = *psrc++) ) {}

    return dst;
}

sal_Char* cpynstr( sal_Char* dst, const sal_Char* src, sal_uInt32 cnt )
{

    const sal_Char* psrc = src;
    sal_Char* pdst = dst;
    sal_uInt32 len = cnt;
    sal_uInt32 i;

    if ( len >= AStringLen(src) )
    {
        return( cpystr( dst, src ) );
    }

    // copy string by char
    for( i = 0; i < len; i++ )
        *pdst++ = *psrc++;
    *pdst = '\0';

    return ( dst );
}

//------------------------------------------------------------------------
sal_Bool cmpstr( const sal_Char* str1, const sal_Char* str2, sal_uInt32 len )
{
    const sal_Char* pBuf1 = str1;
    const sal_Char* pBuf2 = str2;
    sal_uInt32 i = 0;

    while ( (*pBuf1 == *pBuf2) && i < len )
    {
        (pBuf1)++;
        (pBuf2)++;
        i++;
    }
    return( i == len );
}
//-----------------------------------------------------------------------
sal_Bool cmpstr( const sal_Char* str1, const sal_Char* str2 )
{
    const sal_Char* pBuf1 = str1;
    const sal_Char* pBuf2 = str2;
    sal_Bool res = sal_True;

    while ( (*pBuf1 == *pBuf2) && *pBuf1 !='\0' && *pBuf2 != '\0')
    {
        (pBuf1)++;
        (pBuf2)++;
    }
    if (*pBuf1 == '\0' && *pBuf2 == '\0')
        res = sal_True;
    else
        res = sal_False;
    return (res);
}
//------------------------------------------------------------------------
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2, sal_uInt32 len )
{
    const sal_Unicode* pBuf1 = str1;
    const sal_Unicode* pBuf2 = str2;
    sal_uInt32 i = 0;

    while ( (*pBuf1 == *pBuf2) && i < len )
    {
        (pBuf1)++;
        (pBuf2)++;
        i++;
    }
    return( i == len );
}

//-----------------------------------------------------------------------
sal_Bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2 )
{
    const sal_Unicode* pBuf1 = str1;
    const sal_Unicode* pBuf2 = str2;
    sal_Bool res = sal_True;

    while ( (*pBuf1 == *pBuf2) && *pBuf1 !='\0' && *pBuf2 != '\0')
    {
        (pBuf1)++;
        (pBuf2)++;
    }
    if (*pBuf1 == '\0' && *pBuf2 == '\0')
        res = sal_True;
    else
        res = sal_False;
    return (res);
}

sal_Char* createName( sal_Char* dst, const sal_Char* meth, sal_uInt32 cnt )
{
    sal_Char* pdst = dst;
    sal_Char nstr[16];
    sal_Char* pstr = nstr;
    rtl_str_valueOfInt32( pstr, cnt, 10 );

    cpystr( pdst, meth );
    cpystr( pdst+ AStringLen(meth), "_" );

    if ( cnt < 100 )
    {
        cpystr(pdst + AStringLen(pdst), "0" );
    }
    if ( cnt < 10 )
    {
        cpystr(pdst + AStringLen(pdst), "0" );
    }

    cpystr( pdst + AStringLen(pdst), nstr );
    return( pdst );
}

//------------------------------------------------------------------------
//  testing the method compareTo( const OString & aStr )
//------------------------------------------------------------------------
void makeComment( char *com, const char *str1, const char *str2,
                                                            sal_Int32 sgn )
{
    cpystr(com, str1);
    int str1Length = AStringLen( str1 );
    const char *sign = (sgn == 0) ? " == " : (sgn > 0) ? " > " : " < " ;
    cpystr(com + str1Length, sign);
    int signLength = AStringLen(sign);
    cpystr(com + str1Length + signLength, str2);
    com[str1Length + signLength + AStringLen(str2)] = 0;
}

//------------------------------------------------------------------------

sal_uInt32 UStringLen( const sal_Unicode *pUStr )
{
    sal_uInt32 nUStrLen = 0;

    if ( pUStr != NULL )
    {
        const sal_Unicode *pTempUStr = pUStr;

        while( *pTempUStr )
        {
            pTempUStr++;
        } // while

        nUStrLen = (sal_uInt32)( pTempUStr - pUStr );
    } // if

    return nUStrLen;
} // UStringLen

//------------------------------------------------------------------------

static inline sal_Int32 ACharToUCharCompare( const sal_Unicode *pUStr,
                                             const sal_Char    *pAStr
                                           )
{
    sal_Int32  nCmp   = 0;
    sal_Int32  nUChar = (sal_Int32)*pUStr;
    sal_Int32  nChar  = (sal_Int32)((unsigned char)*pAStr);

    nCmp = nUChar - nChar;

    return  nCmp;
} // ACharToUCharCompare

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
