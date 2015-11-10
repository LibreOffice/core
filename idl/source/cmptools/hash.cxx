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

bool SvHashTable::Test_Insert( const OString& rElement, bool bInsert,
                               sal_uInt32 * pInsertPos )
{
    sal_uInt32    nHash;
    sal_uInt32    nIndex;
    sal_uInt32    nLoop;

    lAsk++;
    lTry++;

    nHash =  HashFunc( rElement );
    nIndex = nHash % nMax;

    nLoop = 0;                                      // divide to range
    while( (nMax != nLoop) && IsEntry( nIndex ) )
    {                     // is place occupied
        if( equals( rElement, nIndex ) )
        {
            if( pInsertPos )
                *pInsertPos = nIndex;               // place of Element
            return true;
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
            return true;
        }
    }
    return false;
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
        pPos->AddFirstRef();
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

sal_uInt32 SvStringHashTable::HashFunc( const OString& rElement ) const
{
    sal_uInt32          nHash = 0;  // hash value
    const char *    pStr = rElement.getStr();

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
    return nHash;
}

OString SvStringHashTable::GetNearString( const OString& rName ) const
{
    for( sal_uInt32 i = 0; i < GetMax(); i++ )
    {
        SvStringHashEntry * pE = Get( i );
        if( pE )
        {
            if( pE->GetName().equalsIgnoreAsciiCase( rName ) && !pE->GetName().equals( rName ) )
                return pE->GetName();
        }
    }
    return OString();
}

bool SvStringHashTable::IsEntry( sal_uInt32 nIndex ) const
{
    if( nIndex >= GetMax() )
        return false;
    return pEntries[ nIndex ].HasId();
}

bool SvStringHashTable::Insert( const OString& rName, sal_uInt32 * pIndex )
{
    sal_uInt32 nIndex;

    if( !pIndex ) pIndex = &nIndex;

    if( !SvHashTable::Test_Insert( rName, true, pIndex ) )
        return false;

    if( !IsEntry( *pIndex ) )
        pEntries[ *pIndex ] = SvStringHashEntry( rName, *pIndex );
    return true;
}

bool SvStringHashTable::Test( const OString& rName, sal_uInt32 * pPos ) const
{
    return const_cast<SvStringHashTable*>(this)->Test_Insert( rName, false, pPos );
}

SvStringHashEntry * SvStringHashTable::Get( sal_uInt32 nIndex ) const
{
    if( IsEntry( nIndex ) )
        return pEntries + nIndex;
    return nullptr;
}

bool SvStringHashTable::equals( const OString& rElement,
                                          sal_uInt32 nIndex ) const
{
    return rElement.equals( pEntries[ nIndex ].GetName() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
