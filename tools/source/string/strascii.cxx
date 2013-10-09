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

#ifdef DBG_UTIL
static sal_Bool ImplDbgCheckAsciiStr( const sal_Char* pAsciiStr, sal_Int32 nLen )
{
    while ( nLen && *pAsciiStr )
    {
        if ( ((unsigned char)*pAsciiStr) > 127 )
            return sal_False;
        ++pAsciiStr,
        --nLen;
    }

    return sal_True;
}
#endif

static void ImplCopyAsciiStr( sal_Unicode* pDest, const sal_Char* pSrc,
                              sal_Int32 nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pSrc, nLen ),
                "UniString::CopyAsciiStr() - pAsciiStr include characters > 127" );

    while ( nLen )
    {
        *pDest = (unsigned char)*pSrc;
        ++pDest,
        ++pSrc,
        --nLen;
    }
}

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                         xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

static sal_Int32 ImplStringCompareWithoutZeroAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                                    xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    // determine string length
    sal_Int32 nCopyLen = ImplStringLen( pAsciiStr );

    // detect overflow
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // If appended string is not empty
    if ( nCopyLen )
    {
        // Allocate new string
        UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

        // copy string data
        memcpy( pNewData->maStr, mpData->maStr, mpData->mnLen*sizeof( sal_Unicode ) );
        ImplCopyAsciiStr( pNewData->maStr+mpData->mnLen, pAsciiStr, nCopyLen );

        // release old string
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; ++i )
        {
            if ( !pAsciiStr[i] )
            {
                OSL_FAIL( "UniString::AppendAscii() : nLen is wrong" );
            }
        }
    }
#endif

    // detect overflow
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, nLen );

    // If appended string is not empty
    if ( nCopyLen )
    {
        // Allocate new string
        UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

        // copy string data
        memcpy( pNewData->maStr, mpData->maStr, mpData->mnLen*sizeof( sal_Unicode ) );
        ImplCopyAsciiStr( pNewData->maStr+mpData->mnLen, pAsciiStr, nCopyLen );

        // release old string
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

StringCompare UniString::CompareToAscii( const sal_Char* pAsciiStr,
                                         xub_StrLen nLen ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "UniString::CompareToAscii() - pAsciiStr include characters > 127" );

    // String vergleichen
    sal_Int32 nCompare = ImplStringCompareAscii( mpData->maStr, pAsciiStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

xub_StrLen UniString::SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::SearchAscii() - pAsciiStr include characters > 127" );

    sal_Int32 nLen = mpData->mnLen;
    xub_StrLen nStrLen  = ImplStringLen( pAsciiStr );

    // If length of pAsciiStr is 0 or index exceeds length, it was not found
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const sal_Unicode* pStr = mpData->maStr;
    pStr += nIndex;

    if ( nStrLen == 1 )
    {
        sal_Unicode cSearch = (unsigned char)*pAsciiStr;
        while ( nIndex < nLen )
        {
            if ( *pStr == cSearch )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }
    else
    {
        // Only search within string
        while ( nLen - nIndex >= nStrLen )
        {
            // Check if string matches
            if ( ImplStringCompareWithoutZeroAscii( pStr, pAsciiStr, nStrLen ) == 0 )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
