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

#include <string.h>

#include "boost/static_assert.hpp"

#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <rtl/tencinfo.h>
#include <rtl/instance.hxx>
#include <tools/string.hxx>

#include <impstrg.hxx>

#include <tools/debug.hxx>

DBG_NAME( UniString )

#define STRCODE         sal_Unicode
#define STRCODEU        sal_Unicode
#define STRING          UniString
#define STRINGDATA      UniStringData
#define STRING_TYPE     rtl_uString
#define STRING_ACQUIRE  rtl_uString_acquire
#define STRING_RELEASE  rtl_uString_release
#define STRING_NEW      rtl_uString_new

#if defined DBG_UTIL
#define DBGCHECKSTRING  DbgCheckUniString
#endif

#include <strimp.cxx>
#include <strucvt.cxx>
#include <strascii.cxx>

UniString::UniString(char c): mpData(ImplAllocData(1)) { mpData->maStr[0] = c; }

sal_Int32 UniString::ToInt32() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toInt32( mpData->maStr, 10 );
}

xub_StrLen STRING::SearchChar( const STRCODE* pChars, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        STRCODE         c = *pStr;
        const STRCODE*  pCompStr = pChars;
        while ( *pCompStr )
        {
            if ( *pCompStr == c )
                return nIndex;
            ++pCompStr;
        }
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

xub_StrLen STRING::SearchAndReplace( STRCODE c, STRCODE cRep, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
        {
            ImplCopyData();
            mpData->maStr[nIndex] = cRep;
            return nIndex;
        }
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

STRING& STRING::Insert( const STRING& rStr, xub_StrLen nPos, xub_StrLen nLen,
                        xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Determine string length
    if ( nPos > rStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // Correct length if necessary
        sal_Int32 nMaxLen = rStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    // Detect overflow
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, nLen );

    if ( !nCopyLen )
        return *this;

    // Correct index if necessary
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Determine new length and allocate string
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // copy string to newdata
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr+nPos, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // release old data
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

static sal_Int32 ImplStringICompareWithoutZero( const STRCODE* pStr1, const STRCODE* pStr2,
                                                sal_Int32 nCount )
{
    sal_Int32   nRet = 0;
    STRCODE     c1;
    STRCODE     c2;
    do
    {
        if ( !nCount )
            break;

        // convert if char is between 'A' and 'Z'
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)((STRCODEU)c1))-((sal_Int32)((STRCODEU)c2));

        ++pStr1,
        ++pStr2,
        --nCount;
    }
    while ( nRet == 0 );

    return nRet;
}

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRING& rStr, xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (rStr.mpData->mnLen == 0);
    sal_Int32 nMaxLen = mpData->mnLen-nIndex;
    if ( nMaxLen < nLen )
    {
        if ( rStr.mpData->mnLen != nMaxLen )
            return sal_False;
        nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    return (ImplStringICompareWithoutZero( mpData->maStr+nIndex, rStr.mpData->maStr, nLen ) == 0);
}

StringCompare STRING::CompareIgnoreCaseToAscii( const STRING& rStr,
                                                xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    if ( mpData == rStr.mpData )
        return COMPARE_EQUAL;

    // determine maximal length
    if ( mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(mpData->mnLen+1);
    if ( rStr.mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(rStr.mpData->mnLen+1);

    sal_Int32 nCompare = ImplStringICompareWithoutZero( mpData->maStr, rStr.mpData->maStr, nLen );

    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

STRING& STRING::Insert( STRCODE c, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Don't insert 0 char or string size is maximum
    if ( !c || (mpData->mnLen == STRING_MAXLEN) )
        return *this;

    // Adjust string index
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // allocate string of new size
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+1 );

    // copy string
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    pNewData->maStr[nIndex] = c;
    memcpy( pNewData->maStr+nIndex+1, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // free old data
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

StringCompare STRING::CompareTo( const STRING& rStr, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    if ( mpData == rStr.mpData )
        return COMPARE_EQUAL;

    // determine maximal length
    if ( mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(mpData->mnLen+1);
    if ( rStr.mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(rStr.mpData->mnLen+1);

    sal_Int32 nCompare = ImplStringCompareWithoutZero( mpData->maStr, rStr.mpData->maStr, nLen );

    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

sal_Bool STRING::Equals( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    if ( mpData == rStr.mpData )
        return sal_True;

    if ( mpData->mnLen != rStr.mpData->mnLen )
        return sal_False;

    return (ImplStringCompareWithoutZero( mpData->maStr, rStr.mpData->maStr, mpData->mnLen ) == 0);
}

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    if ( mpData == rStr.mpData )
        return sal_True;

    if ( mpData->mnLen != rStr.mpData->mnLen )
        return sal_False;

    // compare string while ignoring case
    return (ImplStringICompareWithoutZero( mpData->maStr, rStr.mpData->maStr, mpData->mnLen ) == 0);
}

sal_Bool STRING::Equals( const STRING& rStr, xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (rStr.mpData->mnLen == 0);
    sal_Int32 nMaxLen = mpData->mnLen-nIndex;
    if ( nMaxLen < nLen )
    {
        if ( rStr.mpData->mnLen != nMaxLen )
            return sal_False;
        nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    return (ImplStringCompareWithoutZero( mpData->maStr+nIndex, rStr.mpData->maStr, nLen ) == 0);
}

xub_StrLen STRING::Match( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // return if string is empty
    if ( !mpData->mnLen )
        return STRING_MATCH;

    // Search the string for unmatching chars
    const STRCODE*  pStr1 = mpData->maStr;
    const STRCODE*  pStr2 = rStr.mpData->maStr;
    xub_StrLen      i = 0;
    while ( i < mpData->mnLen )
    {
        // Abort on the first unmatching char
        if ( *pStr1 != *pStr2 )
            return i;
        ++pStr1,
        ++pStr2,
        ++i;
    }

    return STRING_MATCH;
}

xub_StrLen STRING::SearchBackward( STRCODE c, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    if ( nIndex > mpData->mnLen )
        nIndex = (xub_StrLen)mpData->mnLen;

    const STRCODE* pStr = mpData->maStr;
    pStr += nIndex;

    while ( nIndex )
    {
        nIndex--;
        pStr--;
        if ( *pStr == c )
            return nIndex;
    }

    return STRING_NOTFOUND;
}

void STRING::SearchAndReplaceAll( const STRING& rStr, const STRING& rRepStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nSPos = Search( rStr, 0 );
    while ( nSPos != STRING_NOTFOUND )
    {
        Replace( nSPos, rStr.Len(), rRepStr );
        nSPos = nSPos + rRepStr.Len();
        nSPos = Search( rStr, nSPos );
    }
}

void STRING::SetToken( xub_StrLen nToken, STRCODE cTok, const STRING& rStr,
                       xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    const STRCODE*  pStr            = mpData->maStr;
    xub_StrLen      nLen            = (xub_StrLen)mpData->mnLen;
    xub_StrLen      nTok            = 0;
    xub_StrLen      nFirstChar      = nIndex;
    xub_StrLen      i               = nFirstChar;

    // Determine token position and length
    pStr += i;
    while ( i < nLen )
    {
        // Increase token count if match
        if ( *pStr == cTok )
        {
            ++nTok;

            if ( nTok == nToken )
                nFirstChar = i+1;
            else
            {
                if ( nTok > nToken )
                    break;
            }
        }

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
        Replace( nFirstChar, i-nFirstChar, rStr );
}

STRING STRING::GetToken( xub_StrLen nToken, STRCODE cTok, sal_Int32& rIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    const STRCODE*  pStr            = mpData->maStr;
    xub_StrLen      nLen            = (xub_StrLen)mpData->mnLen;
    xub_StrLen      nTok            = 0;
    sal_Int32      nFirstChar      = rIndex;
    xub_StrLen      i               = nFirstChar;

    // Determine token position and length
    pStr += i;
    while ( i < nLen )
    {
        // Increase token count if match
        if ( *pStr == cTok )
        {
            ++nTok;

            if ( nTok == nToken )
                nFirstChar = i+1;
            else
            {
                if ( nTok > nToken )
                    break;
            }
        }

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = -1;
        return Copy( nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = -1;
        return STRING();
    }
}

STRING& STRING::Append( const STRCODE* pCharStr, xub_StrLen nCharLen )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Append() - pCharStr is NULL" );

    if ( nCharLen == STRING_LEN )
        nCharLen = ImplStringLen( pCharStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nCharLen; i++ )
        {
            if ( !pCharStr[i] )
            {
                OSL_FAIL( "String::Append() : nLen is wrong" );
            }
        }
    }
#endif

    // Catch overflow
    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nCopyLen = ImplGetCopyLen( nLen, nCharLen );

    if ( nCopyLen )
    {
        // allocate string of new size
        STRINGDATA* pNewData = ImplAllocData( nLen+nCopyLen );

        // copy string
        memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
        memcpy( pNewData->maStr+nLen, pCharStr, nCopyLen*sizeof( STRCODE ) );

        // free old string
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

STRING& STRING::Append( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // don't append null characters and keep string length < maxlen
    sal_Int32 nLen = mpData->mnLen;
    if ( c && (nLen < STRING_MAXLEN) )
    {
        // allocate string of new size
        STRINGDATA* pNewData = ImplAllocData( nLen+1 );

        // copy string
        memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
        pNewData->maStr[nLen] = c;

        // free old string
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

STRING& STRING::Assign( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( c, "String::Assign() - c is 0" );

    // initialize maintenance data
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = ImplAllocData( 1 );
    mpData->maStr[0] = c;
    return *this;
}

xub_StrLen STRING::SearchAndReplace( const STRING& rStr, const STRING& rRepStr,
                                     xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nSPos = Search( rStr, nIndex );
    if ( nSPos != STRING_NOTFOUND )
        Replace( nSPos, rStr.Len(), rRepStr );

    return nSPos;
}

STRING& STRING::Assign( const STRCODE* pCharStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Assign() - pCharStr is NULL" );

    xub_StrLen nLen = ImplStringLen( pCharStr );

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // copy without allocation if string length is identical
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        else
        {
            // free old string
            STRING_RELEASE((STRING_TYPE *)mpData);

            // allocate string of new size and copy
            mpData = ImplAllocData( nLen );
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        }
    }

    return *this;
}

xub_StrLen ImplStringLen( const sal_Char* pStr )
{
    const sal_Char* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

xub_StrLen ImplStringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
