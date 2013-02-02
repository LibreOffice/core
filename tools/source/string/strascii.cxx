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
                "String::CopyAsciiStr() - pAsciiStr include characters > 127" );

    while ( nLen )
    {
        *pDest = (unsigned char)*pSrc;
        ++pDest,
        ++pSrc,
        --nLen;
    }
}

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2 )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2;
    }

    return nRet;
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

static sal_Int32 ImplStringICompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2 )
{
    sal_Int32   nRet;
    sal_Unicode c1;
    sal_Char    c2;
    do
    {
        // Convert if char is between 'A' and 'Z'
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)c1)-((sal_Int32)((unsigned char)c2));
        if ( nRet != 0 )
            break;

        ++pStr1,
        ++pStr2;
    }
    while ( c2 );

    return nRet;
}

static sal_Int32 ImplStringICompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                          xub_StrLen nCount )
{
    sal_Int32   nRet = 0;
    sal_Unicode c1;
    sal_Char    c2;
    do
    {
        if ( !nCount )
            break;

        // Convert if char is between 'A' and 'Z'
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)c1)-((sal_Int32)((unsigned char)c2));
        if ( nRet != 0 )
            break;

        ++pStr1,
        ++pStr2,
        --nCount;
    }
    while ( c2 );

    return nRet;
}

String& String::AssignAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::AssignAscii() - pAsciiStr is NULL" );

    // Determine string length
    xub_StrLen nLen = ImplStringLen( pAsciiStr );

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // Replace string in-place if new size is equal
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // release old string
            STRING_RELEASE((STRING_TYPE *)mpData);

            // copy new string
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

String& String::AssignAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::AssignAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; ++i )
        {
            if ( !pAsciiStr[i] )
            {
                OSL_FAIL( "String::AssignAscii() : nLen is wrong" );
            }
        }
    }
#endif

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // Replace string in-place if new size is equal
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // release old string
            STRING_RELEASE((STRING_TYPE *)mpData);

            // copy new string
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

String& String::AppendAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::AppendAscii() - pAsciiStr is NULL" );

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

String& String::AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::AppendAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; ++i )
        {
            if ( !pAsciiStr[i] )
            {
                OSL_FAIL( "String::AppendAscii() : nLen is wrong" );
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

String& String::InsertAscii( const char* pAsciiStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::InsertAscii() - pAsciiStr is NULL" );

    // Determine string length
    sal_Int32 nCopyLen = ImplStringLen( pAsciiStr );

    // detect overflow
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // If appended string is not empty
    if ( !nCopyLen )
        return *this;

    // Adjust index if exceeds length
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Allocate new string
    UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // copy string data
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( sal_Unicode ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, nCopyLen );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( sal_Unicode ) );

    // release old string
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

String& String::ReplaceAscii( xub_StrLen nIndex, xub_StrLen nCount,
                                    const sal_Char* pAsciiStr, xub_StrLen nStrLen )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "String::ReplaceAscii() - pAsciiStr is NULL" );

    // Use append if index >= length
    if ( nIndex >= mpData->mnLen )
    {
        AppendAscii( pAsciiStr, nStrLen );
        return *this;
    }

    // Use assign if index = 0 and count >= length
    if ( (nIndex == 0) && (nCount >= mpData->mnLen) )
    {
        AssignAscii( pAsciiStr, nStrLen );
        return *this;
    }

    // Use erase if length is equal
    if ( nStrLen == STRING_LEN )
        nStrLen = ImplStringLen( pAsciiStr );
    if ( !nStrLen )
        return Erase( nIndex, nCount );

    // nCount must not exceed string length
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    // Use assign if length matches
    if ( nCount == nStrLen )
    {
        ImplCopyData();
        ImplCopyAsciiStr( mpData->maStr+nIndex, pAsciiStr, nStrLen );
        return *this;
    }

    // detect overflow
    sal_Int32 n = ImplGetCopyLen( mpData->mnLen-nCount, nStrLen );

    // allocate new string
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount+n );

    // copy string data
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, n );
    memcpy( pNewData->maStr+nIndex+n, mpData->maStr+nIndex+nCount,
            (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

    // release old string
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

StringCompare String::CompareToAscii( const sal_Char* pAsciiStr,
                                         xub_StrLen nLen ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "String::CompareToAscii() - pAsciiStr include characters > 127" );

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

StringCompare String::CompareIgnoreCaseToAscii( const sal_Char* pAsciiStr,
                                                   xub_StrLen nLen ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "String::CompareIgnoreCaseToAscii() - pAsciiStr include characters > 127" );

    // compare strings
    sal_Int32 nCompare = ImplStringICompareAscii( mpData->maStr, pAsciiStr, nLen );

    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

sal_Bool String::EqualsAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "String::EqualsAscii() - pAsciiStr include characters > 127" );

    return (ImplStringCompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

sal_Bool String::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "String::EqualsIgnoreCaseAscii() - pAsciiStr include characters > 127" );

    return (ImplStringICompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

sal_Bool String::EqualsAscii( const sal_Char* pAsciiStr,
                             xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "String::EqualsAscii() - pAsciiStr include characters > 127" );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pAsciiStr == 0);

    return (ImplStringCompareAscii( mpData->maStr+nIndex, pAsciiStr, nLen ) == 0);
}

sal_Bool String::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr,
                                       xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "String::EqualsIgnoreCaseAscii() - pAsciiStr include characters > 127" );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pAsciiStr == 0);

    return (ImplStringICompareAscii( mpData->maStr+nIndex, pAsciiStr, nLen ) == 0);
}

xub_StrLen String::SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "String::SearchAscii() - pAsciiStr include characters > 127" );

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

xub_StrLen String::SearchAndReplaceAscii( const sal_Char* pAsciiStr, const String& rRepStr,
                                             xub_StrLen nIndex )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "String::SearchAndReplaceAscii() - pAsciiStr include characters > 127" );

    xub_StrLen nSPos = SearchAscii( pAsciiStr, nIndex );
    if ( nSPos != STRING_NOTFOUND )
        Replace( nSPos, ImplStringLen( pAsciiStr ), rRepStr );

    return nSPos;
}

void String::SearchAndReplaceAllAscii( const sal_Char* pAsciiStr, const String& rRepStr )
{
    DBG_CHKTHIS( String, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "String::SearchAndReplaceAllAscii() - pAsciiStr include characters > 127" );

    xub_StrLen nCharLen = ImplStringLen( pAsciiStr );
    xub_StrLen nSPos = SearchAscii( pAsciiStr, 0 );
    while ( nSPos != STRING_NOTFOUND )
    {
        Replace( nSPos, nCharLen, rRepStr );
        nSPos = nSPos + rRepStr.Len();
        nSPos = SearchAscii( pAsciiStr, nSPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
