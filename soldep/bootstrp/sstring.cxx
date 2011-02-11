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

#ifndef _TOOLS_STRINGLIST
#  define _TOOLS_STRINGLIST
#endif

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/stream.hxx>
#include "soldep/sstring.hxx"

SByteStringList::SByteStringList()
{
}

SByteStringList::~SByteStringList()
{
}

sal_uIntPtr SByteStringList::IsString( ByteString* pStr )
{
    sal_uIntPtr nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        ByteString* pString = GetObject( nRet );
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        ByteString* pString = GetObject( 0 );
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
}

sal_uIntPtr SByteStringList::GetPrevString( ByteString* pStr )
{
    sal_uIntPtr nRet = 0;
    sal_Bool bFound = sal_False;
    sal_uIntPtr nCountMember = Count();
    sal_uIntPtr nUpper = nCountMember;
    sal_uIntPtr nLower = 0;
    sal_uIntPtr nCurrent = nUpper / 2;
    sal_uIntPtr nRem = 0;
    ByteString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = GetObject( nCurrent );
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

sal_uIntPtr SByteStringList::PutString( ByteString* pStr )
{
    sal_uIntPtr nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            ByteString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (sal_uIntPtr)0 );
                return (sal_uIntPtr)0;
            }
        }
        ByteString* pString = GetObject( nPos );
        if ( *pStr != *pString )
        {
            Insert( pStr, nPos+1 );
            return ( nPos +1 );
        }
    }
    else
    {
        Insert( pStr );
        return (sal_uIntPtr)0;
    }

    return NOT_THERE;
}

ByteString* SByteStringList::RemoveString( const ByteString& rName )
{
    sal_uIntPtr i;
    ByteString* pReturn;

    for( i = 0 ; i < Count(); i++ )
    {
        if ( rName == *GetObject( i ) )
        {
            pReturn = GetObject(i);
            Remove(i);
            return pReturn;
        }
    }

    return NULL;
}

void SByteStringList::CleanUp()
{
    ByteString* pByteString = First();
    while (pByteString) {
        delete pByteString;
        pByteString = Next();
    }
    Clear();
}

SByteStringList& SByteStringList::operator<<  ( SvStream& rStream )
{
    sal_uInt32 nListCount;
    rStream >> nListCount;
    for ( sal_uInt16 i = 0; i < nListCount; i++ ) {
        ByteString* pByteString = new ByteString();
        rStream >> *pByteString;
        Insert (pByteString, LIST_APPEND);
    }
    return *this;
}

SByteStringList& SByteStringList::operator>>  ( SvStream& rStream )
{
    sal_uInt32 nListCount = Count();
    rStream << nListCount;
    ByteString* pByteString = First();
    while (pByteString) {
        rStream << *pByteString;
        pByteString = Next();
    }
    return *this;
}







SUniStringList::SUniStringList()
{
}

SUniStringList::~SUniStringList()
{
}

sal_uIntPtr SUniStringList::IsString( UniString* pStr )
{
    sal_uIntPtr nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        UniString* pString = GetObject( nRet );
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        UniString* pString = GetObject( 0 );
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
}

sal_uIntPtr SUniStringList::GetPrevString( UniString* pStr )
{
    sal_uIntPtr nRet = 0;
    sal_Bool bFound = sal_False;
    sal_uIntPtr nCountMember = Count();
    sal_uIntPtr nUpper = nCountMember;
    sal_uIntPtr nLower = 0;
    sal_uIntPtr nCurrent = nUpper / 2;
    sal_uIntPtr nRem = 0;
    UniString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = GetObject( nCurrent );
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
*   Sortiert einen UniString in die Liste ein und gibt die Position,
*   an der einsortiert wurde, zurueck
*
**************************************************************************/

sal_uIntPtr SUniStringList::PutString( UniString* pStr )
{
    sal_uIntPtr nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            UniString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (sal_uIntPtr)0);
                return (sal_uIntPtr)0;
            }
        }
        UniString* pString = GetObject( nPos );
        if ( *pStr != *pString )
        {
            Insert( pStr, nPos+1 );
            return ( nPos +1 );
        }
    }
    else
    {
        Insert( pStr );
        return (sal_uIntPtr)0;
    }

    return NOT_THERE;
}

UniString* SUniStringList::RemoveString( const UniString& rName )
{
    sal_uIntPtr i;
    UniString* pReturn;

    for( i = 0 ; i < Count(); i++ )
    {
        if ( rName == *GetObject( i ) )
        {
            pReturn = GetObject(i);
            Remove(i);
            return pReturn;
        }
    }

    return NULL;
}
