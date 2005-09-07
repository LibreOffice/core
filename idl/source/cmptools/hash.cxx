/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hash.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:48:28 $
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

/****************** I N C L U D E S **************************************/
// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// Programmabh„ngige Includes.
#include <hash.hxx>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#pragma hdrstop

/****************** C O D E **********************************************/
/*************************************************************************
|*
|*    SvStringHashEntry::~SvStringHashEntry()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashEntry::~SvStringHashEntry() { };

/*************************************************************************
|*
|*    SvHashTable::SvHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvHashTable::SvHashTable( UINT32 nMaxEntries )
{
    nMax = nMaxEntries;     // set max entries
    nFill = 0;              // no entries
    lTry = 0;
    lAsk = 0;
}

/*************************************************************************
|*
|*    SvHashTable::~SvHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvHashTable::~SvHashTable()
{
#ifdef DOS_NIE
    printf( "Maximum: %ld, Fllung: %ld\n", (ULONG)nMax, (ULONG)nFill );
    printf( "Anfragen: %ld, Versuche: %ld", (ULONG)lAsk, (ULONG)lTry );
    if( lTry != 0 )
        printf( ", V/E = %ld\n", lTry / lAsk );
#endif
}

/*************************************************************************
|*
|*    SvHashTable::Test_Insert()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvHashTable::Test_Insert( const void * pElement, BOOL bInsert,
                               UINT32 * pInsertPos )
{
    UINT32    nHash;
    UINT32    nIndex;
    UINT32    nLoop;

    lAsk++;
    lTry++;

    nHash =  HashFunc( pElement );
    nIndex = nHash % nMax;

//  const char* s = ((ByteString*) pElement)->GetStr();
//  fprintf(stderr,"### Hash: %lu , Name: %s\n",nIndex,s );

    nLoop = 0;                                      // divide to range
    while( (nMax != nLoop) && IsEntry( nIndex ) )
    {                     // is place occupied
        if( COMPARE_EQUAL == Compare( pElement, nIndex ) )
        {
            if( pInsertPos )
                *pInsertPos = nIndex;               // place of Element
            return TRUE;
        }
        nLoop++;
        lTry++;
        nIndex = (USHORT)(nIndex + nHash + 7) % nMax;
    }

    if( bInsert )
    {
        DBG_ASSERT( nMax != nLoop, "Hash table full" )
        if( nMax != nLoop )
        {
            nFill++;
            *pInsertPos = nIndex;                       // return free place
            return TRUE;
        }
    }
    return( FALSE );
}

/************************************************************************/
/*************************************************************************
|*
|*    SvStringHashTable::SvStringHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashTable::SvStringHashTable( UINT32 nMaxEntries )
        : SvHashTable( nMaxEntries )
{
#ifdef WIN
    DBG_ASSERT( (UINT32)nMaxEntries * sizeof( SvStringHashEntry ) <= 0xFF00,
                "Hash table size cannot exeed 64k byte" )
#endif
    pEntries = new SvStringHashEntry[ nMaxEntries ];

    // RefCount auf eins setzen
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + nMaxEntries;
    while( pPos != pEnd )
    {
        pPos->AddRef();
        pPos++;
    }
}

/*************************************************************************
|*
|*    ~SvStringHashTable::SvStringHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashTable::~SvStringHashTable()
{
    // RefCount auf eins setzen
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + GetMax();
#ifdef DBG_UTIL
    while( pPos != pEnd )
    {
        DBG_ASSERT( pPos->GetRefCount() == 1, "Reference count != 1" )
        pPos++;
    }
#endif

#ifdef MPW
    // der MPW-Compiler ruft sonst keine Dtoren!
    for ( USHORT n = 0; n < GetMax(); ++n )
        (pEntries+n)->SvStringHashEntry::~SvStringHashEntry();
    delete (void*) pEntries;
#else
    delete [] pEntries;
#endif
}

/*************************************************************************
|*
|*    SvStringHashTable::HashFunc()
|*
|*    Beschreibung
|*
*************************************************************************/
UINT32 SvStringHashTable::HashFunc( const void * pElement ) const
{
    UINT32          nHash = 0;  // hash value
    const char *    pStr = ((const ByteString * )pElement)->GetBuffer();

    int nShift = 0;
    while( *pStr )
    {
        if( isupper( *pStr ) )
            nHash ^= UINT32(*pStr - 'A' + 26) << nShift;
        else
            nHash ^= UINT32(*pStr - 'a') << nShift;
        if( nShift == 28 )
            nShift = 0;
        else
            nShift += 4;
        pStr++;
    }
    return( nHash );
}

/*************************************************************************
|*
|*    SvStringHashTable::GetNearString()
|*
|*    Beschreibung
|*
*************************************************************************/
ByteString SvStringHashTable::GetNearString( const ByteString & rName ) const
{
    for( UINT32 i = 0; i < GetMax(); i++ )
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

/*************************************************************************
|*
|*    SvStringHashTable::IsEntry()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvStringHashTable::IsEntry( UINT32 nIndex ) const
{
    if( nIndex >= GetMax() )
        return FALSE;
    return pEntries[ nIndex ].HasId();
}

/*************************************************************************
|*
|*    SvStringHashTable::Insert()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvStringHashTable::Insert( const ByteString & rName, UINT32 * pIndex )
{
    UINT32 nIndex;

    if( !pIndex ) pIndex = &nIndex;

    if( !SvHashTable::Test_Insert( &rName, TRUE, pIndex ) )
        return FALSE;

    if( !IsEntry( *pIndex ) )
        pEntries[ *pIndex ] = SvStringHashEntry( rName, *pIndex );
    return TRUE;
}

/*************************************************************************
|*
|*    SvStringHashTable::Test()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvStringHashTable::Test( const ByteString & rName, UINT32 * pPos ) const
{
    return ((SvStringHashTable *)this)->SvHashTable::
                Test_Insert( &rName, FALSE, pPos );
}

/*************************************************************************
|*
|*    SvStringHashTable::Get()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashEntry * SvStringHashTable::Get( UINT32 nIndex ) const
{
    if( IsEntry( nIndex ) )
        return pEntries + nIndex;
    return( NULL );
}

/*************************************************************************
|*
|*    SvStringHashTable::Get()
|*
|*    Beschreibung
|*
*************************************************************************/
StringCompare SvStringHashTable::Compare( const void * pElement,
                                          UINT32 nIndex ) const
{
    return ((const ByteString *)pElement)->CompareTo( pEntries[ nIndex ].GetName() );
}

/*************************************************************************
|*
|*    SvStringHashTable::FillHashList()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvStringHashTable::FillHashList( SvStringHashList * pList ) const
{
    for( UINT32 n = 0; n < GetMax(); n++ )
    {
        if( IsEntry( n ) )
            pList->Insert( Get( n ), LIST_APPEND );
    }
    // Hash Reihenfolge, jetzt sortieren
}
