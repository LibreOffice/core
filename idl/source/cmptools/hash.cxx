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
#include "precompiled_idl.hxx"

// C and C++ includes
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// program-sensitive includes
#include <hash.hxx>
#include <tools/debug.hxx>

SvStringHashEntry::~SvStringHashEntry() { };

SvHashTable::SvHashTable( sal_uInt32 nMaxEntries )
{
    nMax = nMaxEntries;     // set max entries
    nFill = 0;              // no entries
    lTry = 0;
    lAsk = 0;
}

SvHashTable::~SvHashTable()
{
}

sal_Bool SvHashTable::Test_Insert( const void * pElement, sal_Bool bInsert,
                               sal_uInt32 * pInsertPos )
{
    sal_uInt32    nHash;
    sal_uInt32    nIndex;
    sal_uInt32    nLoop;

    lAsk++;
    lTry++;

    nHash =  HashFunc( pElement );
    nIndex = nHash % nMax;

    nLoop = 0;                                      // divide to range
    while( (nMax != nLoop) && IsEntry( nIndex ) )
    {                     // is place occupied
        if( COMPARE_EQUAL == Compare( pElement, nIndex ) )
        {
            if( pInsertPos )
                *pInsertPos = nIndex;               // place of Element
            return sal_True;
        }
        nLoop++;
        lTry++;
        nIndex = (sal_uInt16)(nIndex + nHash + 7) % nMax;
    }

    if( bInsert )
    {
        DBG_ASSERT( nMax != nLoop, "Hash table full" );
        if( nMax != nLoop )
        {
            nFill++;
            *pInsertPos = nIndex;                       // return free place
            return sal_True;
        }
    }
    return( sal_False );
}

SvStringHashTable::SvStringHashTable( sal_uInt32 nMaxEntries )
        : SvHashTable( nMaxEntries )
{
    pEntries = new SvStringHashEntry[ nMaxEntries ];

    // set RefCount to one
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + nMaxEntries;
    while( pPos != pEnd )
    {
        pPos->AddRef();
        pPos++;
    }
}

SvStringHashTable::~SvStringHashTable()
{
#ifdef DBG_UTIL
    // set RefCount to one
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + GetMax();
    while( pPos != pEnd )
    {
        DBG_ASSERT( pPos->GetRefCount() == 1, "Reference count != 1" );
        pPos++;
    }
#endif

    delete [] pEntries;
}

sal_uInt32 SvStringHashTable::HashFunc( const void * pElement ) const
{
    sal_uInt32          nHash = 0;  // hash value
    const char *    pStr = ((const ByteString * )pElement)->GetBuffer();

    int nShift = 0;
    while( *pStr )
    {
        if( isupper( *pStr ) )
            nHash ^= sal_uInt32(*pStr - 'A' + 26) << nShift;
        else
            nHash ^= sal_uInt32(*pStr - 'a') << nShift;
        if( nShift == 28 )
            nShift = 0;
        else
            nShift += 4;
        pStr++;
    }
    return( nHash );
}

ByteString SvStringHashTable::GetNearString( const ByteString & rName ) const
{
    for( sal_uInt32 i = 0; i < GetMax(); i++ )
    {
        SvStringHashEntry * pE = Get( i );
        if( pE )
        {
            if( pE->GetName().EqualsIgnoreCaseAscii( rName ) && !pE->GetName().Equals( rName ) )
                return pE->GetName();
        }
    }
    return ByteString();
}

sal_Bool SvStringHashTable::IsEntry( sal_uInt32 nIndex ) const
{
    if( nIndex >= GetMax() )
        return sal_False;
    return pEntries[ nIndex ].HasId();
}

sal_Bool SvStringHashTable::Insert( const ByteString & rName, sal_uInt32 * pIndex )
{
    sal_uInt32 nIndex;

    if( !pIndex ) pIndex = &nIndex;

    if( !SvHashTable::Test_Insert( &rName, sal_True, pIndex ) )
        return sal_False;

    if( !IsEntry( *pIndex ) )
        pEntries[ *pIndex ] = SvStringHashEntry( rName, *pIndex );
    return sal_True;
}

sal_Bool SvStringHashTable::Test( const ByteString & rName, sal_uInt32 * pPos ) const
{
    return ((SvStringHashTable *)this)->SvHashTable::
                Test_Insert( &rName, sal_False, pPos );
}

SvStringHashEntry * SvStringHashTable::Get( sal_uInt32 nIndex ) const
{
    if( IsEntry( nIndex ) )
        return pEntries + nIndex;
    return( NULL );
}

StringCompare SvStringHashTable::Compare( const void * pElement,
                                          sal_uInt32 nIndex ) const
{
    return ((const ByteString *)pElement)->CompareTo( pEntries[ nIndex ].GetName() );
}

void SvStringHashTable::FillHashList( SvStringHashList * pList ) const
{
    for( sal_uInt32 n = 0; n < GetMax(); n++ )
    {
        if( IsEntry( n ) )
            pList->push_back( Get( n ) );
    }
    // hash order, sort now
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
