/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <math.h>
#include <stdlib.h>

#include <sal/types.h>

#include <rtl/ustring.h>
#include <rtl/string.hxx>
#include <rtl_String_Utils_Const.h>

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
bool cmpstr( const sal_Char* str1, const sal_Char* str2, sal_uInt32 len )
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
bool cmpstr( const sal_Char* str1, const sal_Char* str2 )
{
    const sal_Char* pBuf1 = str1;
    const sal_Char* pBuf2 = str2;
    bool res = true;

    while ( (*pBuf1 == *pBuf2) && *pBuf1 !='\0' && *pBuf2 != '\0')
    {
        (pBuf1)++;
        (pBuf2)++;
    }
    if (*pBuf1 == '\0' && *pBuf2 == '\0')
        res = true;
    else
        res = false;
    return (res);
}
//------------------------------------------------------------------------
bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2, sal_uInt32 len )
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
bool cmpustr( const sal_Unicode* str1, const sal_Unicode* str2 )
{
    const sal_Unicode* pBuf1 = str1;
    const sal_Unicode* pBuf2 = str2;
    bool res = true;

    while ( (*pBuf1 == *pBuf2) && *pBuf1 !='\0' && *pBuf2 != '\0')
    {
        (pBuf1)++;
        (pBuf2)++;
    }
    if (*pBuf1 == '\0' && *pBuf2 == '\0')
        res = true;
    else
        res = false;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
