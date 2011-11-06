/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
