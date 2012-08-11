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

static sal_Int32 ImplStringCompare( const STRCODE* pStr1, const STRCODE* pStr2,
                                    xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

static sal_Int32 ImplStringCompareWithoutZero( const STRCODE* pStr1, const STRCODE* pStr2,
                                               sal_Int32 nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

#ifdef DBG_UTIL
const char* DBGCHECKSTRING( const void* pString )
{
    STRING* p = (STRING*)pString;

    if ( p->GetBuffer()[p->Len()] != 0 )
        return "String damaged: aStr[nLen] != 0";

    return NULL;
}
#endif

static STRINGDATA* ImplAllocData( sal_Int32 nLen )
{
    STRINGDATA* pData   = (STRINGDATA*)rtl_allocateMemory( sizeof(STRINGDATA)+(nLen*sizeof( STRCODE )) );
    pData->mnRefCount   = 1;
    pData->mnLen        = nLen;
    pData->maStr[nLen]  = 0;
    return pData;
}

static STRINGDATA* _ImplCopyData( STRINGDATA* pData )
{
    unsigned int    nSize       = sizeof(STRINGDATA)+(pData->mnLen*sizeof( STRCODE ));
    STRINGDATA*     pNewData    = (STRINGDATA*)rtl_allocateMemory( nSize );
    memcpy( pNewData, pData, nSize );
    pNewData->mnRefCount = 1;
    STRING_RELEASE((STRING_TYPE *)pData);
    return pNewData;
}

inline void STRING::ImplCopyData()
{
    DBG_ASSERT( (mpData->mnRefCount != 0), "String::ImplCopyData() - RefCount == 0" );

    // Dereference data if this string is referenced
    if ( mpData->mnRefCount != 1 )
        mpData = _ImplCopyData( mpData );
}

inline STRCODE* STRING::ImplCopyStringData( STRCODE* pStr )
{
    if ( mpData->mnRefCount != 1 ) {
        DBG_ASSERT( (pStr >= mpData->maStr) &&
                    ((pStr-mpData->maStr) < mpData->mnLen),
                    "ImplCopyStringData - pStr from other String-Instanz" );
        unsigned int nIndex = (unsigned int)(pStr-mpData->maStr);
        mpData = _ImplCopyData( mpData );
        pStr = mpData->maStr + nIndex;
    }
    return pStr;
}

inline sal_Int32 ImplGetCopyLen( sal_Int32 nStrLen, sal_Int32 nCopyLen )
{
    OSL_ASSERT(nStrLen <= STRING_MAXLEN && nCopyLen <= STRING_MAXLEN);
    if ( nCopyLen > STRING_MAXLEN-nStrLen )
        nCopyLen = STRING_MAXLEN-nStrLen;
    return nCopyLen;
}

STRING::STRING()
    : mpData(NULL)
{
    DBG_CTOR( STRING, DBGCHECKSTRING );

    STRING_NEW((STRING_TYPE **)&mpData);
}

STRING::STRING( const STRING& rStr )
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Set pointer to argument string and increase reference counter
    STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
    mpData = rStr.mpData;
}

STRING::STRING( const STRING& rStr, xub_StrLen nPos, xub_StrLen nLen )
: mpData( NULL )
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    if ( nPos > rStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // correct length if necessary
        sal_Int32 nMaxLen = rStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    if ( nLen )
    {
        // Increase reference counter if it suffices
        if ( (nPos == 0) && (nLen == rStr.mpData->mnLen) )
        {
            STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
            mpData = rStr.mpData;
        }
        else
        {
            // otherwise, copy string
            mpData = ImplAllocData( nLen );
            memcpy( mpData->maStr, rStr.mpData->maStr+nPos, nLen*sizeof( STRCODE ) );
        }
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

STRING::~STRING()
{
    DBG_DTOR( STRING, DBGCHECKSTRING );

    // free string data
    STRING_RELEASE((STRING_TYPE *)mpData);
}

STRING& STRING::Assign( const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = rStr.mpData;
    return *this;
}

STRING& STRING::Append( const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Assignment is sufficient if string is empty
    sal_Int32 nLen = mpData->mnLen;
    if ( !nLen )
    {
        STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = rStr.mpData;
    }
    else
    {
        // Detect overflow
        sal_Int32 nCopyLen = ImplGetCopyLen( nLen, rStr.mpData->mnLen );

        if ( nCopyLen )
        {
            // allocate string of new size
            STRINGDATA* pNewData = ImplAllocData( nLen+nCopyLen );

            // copy string
            memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
            memcpy( pNewData->maStr+nLen, rStr.mpData->maStr, nCopyLen*sizeof( STRCODE ) );

            // free old string
            STRING_RELEASE((STRING_TYPE *)mpData);
            mpData = pNewData;
        }
    }

    return *this;
}

STRING& STRING::Append( const STRCODE* pCharStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Append() - pCharStr is NULL" );

    // determine string length
    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nCopyLen = ImplStringLen( pCharStr );

    // detect overflow
    nCopyLen = ImplGetCopyLen( nLen, nCopyLen );

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

void STRING::SetChar( xub_StrLen nIndex, STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( nIndex < mpData->mnLen, "String::SetChar() - nIndex > String.Len()" );

    // copy data if necessary
    ImplCopyData();
    mpData->maStr[nIndex] = c;
}

STRING& STRING::Insert( const STRING& rStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // detect overflow
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, rStr.mpData->mnLen );

    if ( !nCopyLen )
        return *this;

    // adjust index if necessary
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // allocate string of new size
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // copy string
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // free old string
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

STRING& STRING::Replace( xub_StrLen nIndex, xub_StrLen nCount, const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Append if index > current length
    if ( nIndex >= mpData->mnLen )
    {
        Append( rStr );
        return *this;
    }

    // assign if index = 0 and length >= stringlen
    if ( (nIndex == 0) && (nCount >= mpData->mnLen) )
    {
        Assign( rStr );
        return *this;
    }

    // Use erase if replacestring is empty
    sal_Int32 nStrLen = rStr.mpData->mnLen;
    if ( !nStrLen )
        return Erase( nIndex, nCount );

    // Adjust nCount if it's larger than the string
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    if ( !nCount )
        return Insert( rStr, nIndex );

    // Use character-based assignment if length is equal
    if ( nCount == nStrLen )
    {
        ImplCopyData();
        memcpy( mpData->maStr+nIndex, rStr.mpData->maStr, nCount*sizeof( STRCODE ) );
        return *this;
    }

    // detect overflow
    nStrLen = ImplGetCopyLen( mpData->mnLen-nCount, nStrLen );

    // allocate string of new size
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount+nStrLen );

    // copy string
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr, nStrLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nStrLen, mpData->maStr+nIndex+nCount,
            (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

    // free old string
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

STRING& STRING::Erase( xub_StrLen nIndex, xub_StrLen nCount )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Return if index outside string or count = 0
    if ( (nIndex >= mpData->mnLen) || !nCount )
        return *this;

    // Adjust nCount if it's larger than the string
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    if ( mpData->mnLen - nCount )
    {
        // allocate string of new size
        STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount );

        // copy string
        memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
        memcpy( pNewData->maStr+nIndex, mpData->maStr+nIndex+nCount,
                (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

        // free old string
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
}

STRING& STRING::ToLowerAscii()
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    STRCODE*    pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        // Convert if char is between 'A' and 'Z'
        if ( (*pStr >= 65) && (*pStr <= 90) )
        {
            // allocate string of new size
            pStr = ImplCopyStringData( pStr );
            *pStr += 32;
        }

        ++pStr,
        ++nIndex;
    }

    return *this;
}

xub_StrLen STRING::Search( STRCODE c, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
            return nIndex;
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

xub_StrLen STRING::Search( const STRING& rStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nStrLen = rStr.mpData->mnLen;

    // rStr was not found if its length is zero
    // or index is larger than searched string
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const STRCODE* pStr1 = mpData->maStr;
    pStr1 += nIndex;

    if ( nStrLen == 1 )
    {
        STRCODE cSearch = rStr.mpData->maStr[0];
        while ( nIndex < nLen )
        {
            if ( *pStr1 == cSearch )
                return nIndex;
            ++pStr1,
            ++nIndex;
        }
    }
    else
    {
        const STRCODE* pStr2 = rStr.mpData->maStr;

        // search only within string
        while ( nLen - nIndex >= nStrLen )
        {
            // increase match if found
            if ( ImplStringCompareWithoutZero( pStr1, pStr2, nStrLen ) == 0 )
                return nIndex;
            ++pStr1,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
}

xub_StrLen STRING::Search( const STRCODE* pCharStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nLen = mpData->mnLen;
    xub_StrLen nStrLen  = ImplStringLen( pCharStr );

    // rStr was not found if its length is zero
    // or index is larger than searched string
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const STRCODE* pStr = mpData->maStr;
    pStr += nIndex;

    if ( nStrLen == 1 )
    {
        STRCODE cSearch = *pCharStr;
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
        // search only within string
        while ( nLen - nIndex >= nStrLen )
        {
            // increase match if found
            if ( ImplStringCompareWithoutZero( pStr, pCharStr, nStrLen ) == 0 )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
}

void STRING::SearchAndReplaceAll( STRCODE c, STRCODE cRep )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen    = mpData->mnLen;
    const STRCODE*  pStr    = mpData->maStr;
    sal_Int32       nIndex  = 0;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
        {
            ImplCopyData();
            mpData->maStr[nIndex] = cRep;
        }
        ++pStr,
        ++nIndex;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
