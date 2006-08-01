/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sstring.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:20:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _TOOLS_STRINGLIST
#  define _TOOLS_STRINGLIST
#endif

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include "stream.hxx"
#include "bootstrp/sstring.hxx"

SByteStringList::SByteStringList()
{
}

SByteStringList::~SByteStringList()
{
}

ULONG SByteStringList::IsString( ByteString* pStr )
{
    ULONG nRet = NOT_THERE;
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

ULONG SByteStringList::GetPrevString( ByteString* pStr )
{
    ULONG nRet = 0;
    BOOL bFound = FALSE;
    ULONG nCountMember = Count();
    ULONG nUpper = nCountMember;
    ULONG nLower = 0;
    ULONG nCurrent = nUpper / 2;
    ULONG nRem = 0;
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

ULONG SByteStringList::PutString( ByteString* pStr )
{
    ULONG nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            ByteString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (ULONG)0 );
                return (ULONG)0;
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
        return (ULONG)0;
    }

    return NOT_THERE;
}

ByteString* SByteStringList::RemoveString( const ByteString& rName )
{
    ULONG i;
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
    ULONG nCount;
    rStream >> nCount;
    for ( USHORT i = 0; i < nCount; i++ ) {
        ByteString* pByteString = new ByteString();
        rStream >> *pByteString;
        Insert (pByteString, LIST_APPEND);
    }
    return *this;
}

SByteStringList& SByteStringList::operator>>  ( SvStream& rStream )
{
    ULONG nCount = Count();
    rStream << nCount;
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

ULONG SUniStringList::IsString( UniString* pStr )
{
    ULONG nRet = NOT_THERE;
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

ULONG SUniStringList::GetPrevString( UniString* pStr )
{
    ULONG nRet = 0;
    BOOL bFound = FALSE;
    ULONG nCountMember = Count();
    ULONG nUpper = nCountMember;
    ULONG nLower = 0;
    ULONG nCurrent = nUpper / 2;
    ULONG nRem = 0;
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

ULONG SUniStringList::PutString( UniString* pStr )
{
    ULONG nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            UniString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (ULONG)0);
                return (ULONG)0;
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
        return (ULONG)0;
    }

    return NOT_THERE;
}

UniString* SUniStringList::RemoveString( const UniString& rName )
{
    ULONG i;
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
