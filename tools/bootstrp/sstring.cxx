/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/stream.hxx>
#include "bootstrp/sstring.hxx"

SByteStringList::SByteStringList()
{
}

SByteStringList::~SByteStringList()
{
    CleanUp();
}

size_t SByteStringList::size() const {
    return maList.size();
}

ByteString* SByteStringList::erase( size_t i ) {
    ByteString* result = NULL;
    if ( i < maList.size() ) {
        ByteStringList::iterator it = maList.begin();
        ::std::advance( it, i );
        result = *it;
        maList.erase( it );
    }
    return result;
}


ByteString* SByteStringList::operator[]( size_t i ) const {
    return ( i < maList.size() ) ? maList[ i ] : NULL;
}

ByteString* SByteStringList::at( size_t i ) const {
    return ( i < maList.size() ) ? maList[ i ] : NULL;
}

size_t SByteStringList::IsString( ByteString* pStr )
{
    size_t nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        ByteString* pString = maList[ nRet ];
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        ByteString* pString = maList.empty() ? NULL : maList[ 0 ];
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
}

size_t SByteStringList::GetPrevString( ByteString* pStr )
{
    size_t nRet = 0;
    BOOL bFound = FALSE;
    size_t nCountMember = maList.size();
    size_t nUpper = nCountMember;
    size_t nLower = 0;
    size_t nCurrent = nUpper / 2;
    size_t nRem = 0;
    ByteString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = maList[ nCurrent ];
        StringCompare nResult =  pStr->CompareTo( *pString );
        if ( nResult == COMPARE_LESS )
        {
            nUpper = nCurrent;
            nCurrent = (nCurrent + nLower) /2;
        }
        else if ( nResult == COMPARE_GREATER )
        {
            nLower = nCurrent;
            nCurrent = (nUpper + nCurrent) /2;
        }
        else if ( nResult == COMPARE_EQUAL )
            return nCurrent;
        if ( nRem == nCurrent )
            return nCurrent;
        nRem = nCurrent;
    }
    while ( !bFound );
    return nRet;
}

/**************************************************************************
*
*   Sortiert einen ByteString in die Liste ein und gibt die Position,
*   an der einsortiert wurde, zurueck
*
**************************************************************************/

size_t SByteStringList::PutString( ByteString* pStr )
{
    size_t nPos = GetPrevString ( pStr );
    if ( maList.size() )
    {
        ByteString* pString = NULL;
        pString = maList[ 0 ];
        if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
        {
            maList.insert( maList.begin(), pStr );
            return 0;
        }
        pString = maList[ nPos ];
        if ( *pStr != *pString )
        {
            if ( ++nPos < maList.size() ) {
                ByteStringList::iterator it = maList.begin();
                ::std::advance( it, nPos );
                maList.insert( it, pStr );
                return nPos;
            } else {
                maList.push_back( pStr );
                return maList.size() - 1;
            }
        }
    }
    else
    {
        maList.push_back( pStr );
        return 0;
    }

    return NOT_THERE;
}

ByteString* SByteStringList::RemoveString( const ByteString& rName )
{
    ByteString* pReturn = NULL;

    for ( ByteStringList::iterator it = maList.begin(); it < maList.end(); ++it )
    {
        if ( rName == *(*it) )
        {
            pReturn = *it;
            maList.erase( it );
            return pReturn;
        }
    }

    return pReturn;
}

void SByteStringList::CleanUp()
{
    for ( size_t i = 0, n = maList.size(); i < n; ++i ) {
        delete maList[ i ];
    }
    maList.clear();
}

SByteStringList& SByteStringList::operator<<  ( SvStream& rStream )
{
    sal_uInt32 nListCount;
    rStream >> nListCount;
    for ( sal_uInt32 i = 0; i < nListCount; i++ ) {
        ByteString* pByteString = new ByteString();
        rStream >> *pByteString;
        maList.push_back( pByteString );
    }
    return *this;
}

SByteStringList& SByteStringList::operator>>  ( SvStream& rStream )
{
    sal_uInt32 nListCount = maList.size();
    rStream << nListCount;
    for ( size_t i = 0; i < nListCount; ++i ) {
        rStream << *( maList[ i ] );
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
