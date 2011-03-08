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
#include "precompiled_sc.hxx"

#include "namebuff.hxx"

#include <tools/urlobj.hxx>
#include <string.h>

#include "rangenam.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "scextopt.hxx"

#include "root.hxx"
#include "tokstack.hxx"
#include "xltools.hxx"
#include "xiroot.hxx"


UINT32 StringHashEntry::MakeHashCode( const String& r )
{
    register UINT32                 n = 0;
    const sal_Unicode*              pAkt = r.GetBuffer();
    register sal_Unicode            cAkt = *pAkt;

    while( cAkt )
    {
        n *= 70;
        n += ( UINT32 ) cAkt;
        pAkt++;
        cAkt = *pAkt;
    }

    return n;
}




NameBuffer::~NameBuffer()
{
    register StringHashEntry*   pDel = ( StringHashEntry* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( StringHashEntry* ) List::Next();
    }
}


//void NameBuffer::operator <<( const SpString &rNewString )
void NameBuffer::operator <<( const String &rNewString )
{
    DBG_ASSERT( Count() + nBase < 0xFFFF,
        "*NameBuffer::GetLastIndex(): Ich hab' die Nase voll!" );

    List::Insert( new StringHashEntry( rNewString ), LIST_APPEND );
}


#ifdef DBG_UTIL
UINT16  nShrCnt;
#endif


size_t ShrfmlaBuffer::ScAddressHashFunc::operator() (const ScAddress &addr) const
{
    // Use something simple, it is just a hash.
    return static_cast< UINT16 >( addr.Row() ) | (static_cast< UINT8 >( addr.Col() ) << 16);
}

const size_t nBase = 16384; // Range~ und Shared~ Dingens mit jeweils der Haelfte Ids
ShrfmlaBuffer::ShrfmlaBuffer( RootData* pRD ) :
    ExcRoot( pRD ),
    mnCurrIdx (nBase)
{
#ifdef DBG_UTIL
    nShrCnt = 0;
#endif
}

ShrfmlaBuffer::~ShrfmlaBuffer()
{
}

void ShrfmlaBuffer::Clear()
{
    index_hash.clear();
    // do not clear index_list, index calculation depends on complete list size...
    // do not change mnCurrIdx
}

void ShrfmlaBuffer::Store( const ScRange& rRange, const ScTokenArray& rToken )
{
    String          aName( CreateName( rRange.aStart ) );

    DBG_ASSERT( mnCurrIdx <= 0xFFFF, "*ShrfmlaBuffer::Store(): Gleich wird mir schlecht...!" );

    ScRangeData* pData = new ScRangeData( pExcRoot->pIR->GetDocPtr(), aName, rToken, rRange.aStart, RT_SHARED );
    const ScAddress& rMaxPos = pExcRoot->pIR->GetMaxPos();
    pData->SetMaxCol(rMaxPos.Col());
    pData->SetMaxRow(rMaxPos.Row());
    pData->SetIndex( static_cast< USHORT >( mnCurrIdx ) );
    pExcRoot->pIR->GetNamedRanges().insert( pData );
    index_hash[rRange.aStart] = static_cast< USHORT >( mnCurrIdx );
    index_list.push_front (rRange);
    ++mnCurrIdx;
}


USHORT ShrfmlaBuffer::Find( const ScAddress & aAddr ) const
{
    ShrfmlaHash::const_iterator hash = index_hash.find (aAddr);
    if (hash != index_hash.end())
        return hash->second;

    // It was not hashed on the top left corner ?  do a brute force search
    unsigned int ind = nBase;
    for (ShrfmlaList::const_iterator ptr = index_list.end(); ptr != index_list.begin() ; ind++)
        if ((--ptr)->In (aAddr))
            return static_cast< USHORT >( ind );
    return static_cast< USHORT >( mnCurrIdx );
}


#define SHRFMLA_BASENAME    "SHARED_FORMULA_"

String ShrfmlaBuffer::CreateName( const ScRange& r )
{
    String          aName( RTL_CONSTASCII_USTRINGPARAM( SHRFMLA_BASENAME ) );
    aName += String::CreateFromInt32( r.aStart.Col() );
    aName.Append( '_' );
    aName += String::CreateFromInt32( r.aStart.Row() );
    aName.Append( '_' );
    aName += String::CreateFromInt32( r.aEnd.Col() );
    aName.Append( '_' );
    aName += String::CreateFromInt32( r.aEnd.Row() );
    aName.Append( '_' );
    aName += String::CreateFromInt32( r.aStart.Tab() );

    return aName;
}


ExtSheetBuffer::~ExtSheetBuffer()
{
    Cont    *pAkt = ( Cont * ) List::First();
    while( pAkt )
    {
        delete pAkt;
        pAkt = ( Cont * ) List::Next();
    }
}


sal_Int16 ExtSheetBuffer::Add( const String& rFPAN, const String& rTN, const BOOL bSWB )
{
    List::Insert( new Cont( rFPAN, rTN, bSWB ), LIST_APPEND );
    // return 1-based index of EXTERNSHEET
    return static_cast< sal_Int16 >( List::Count() );
}


BOOL ExtSheetBuffer::GetScTabIndex( UINT16 nExcIndex, UINT16& rScIndex )
{
    DBG_ASSERT( nExcIndex,
        "*ExtSheetBuffer::GetScTabIndex(): Sheet-Index == 0!" );

    nExcIndex--;
    Cont*       pCur = ( Cont * ) List::GetObject( nExcIndex );
    UINT16&     rTabNum = pCur->nTabNum;

    if( pCur )
    {
        if( rTabNum < 0xFFFD )
        {
            rScIndex = rTabNum;
            return TRUE;
        }

        if( rTabNum == 0xFFFF )
        {// neue Tabelle erzeugen
            SCTAB   nNewTabNum;
            if( pCur->bSWB )
            {// Tabelle ist im selben Workbook!
                if( pExcRoot->pIR->GetDoc().GetTable( pCur->aTab, nNewTabNum ) )
                {
                    rScIndex = rTabNum = static_cast<UINT16>(nNewTabNum);
                    return TRUE;
                }
                else
                    rTabNum = 0xFFFD;
            }
            else if( pExcRoot->pIR->GetDocShell() )
            {// Tabelle ist 'echt' extern
                if( pExcRoot->pIR->GetExtDocOptions().GetDocSettings().mnLinkCnt == 0 )
                {
                    String      aURL( ScGlobal::GetAbsDocName( pCur->aFile,
                                        pExcRoot->pIR->GetDocShell() ) );
                    String      aTabName( ScGlobal::GetDocTabName( aURL, pCur->aTab ) );
                    if( pExcRoot->pIR->GetDoc().LinkExternalTab( nNewTabNum, aTabName, aURL, pCur->aTab ) )
                    {
                        rScIndex = rTabNum = static_cast<UINT16>(nNewTabNum);
                        return TRUE;
                    }
                    else
                        rTabNum = 0xFFFE;       // Tabelle einmal nicht angelegt -> wird
                                                //  wohl auch nicht mehr gehen...
                }
                else
                    rTabNum = 0xFFFE;

            }
        }
    }

    return FALSE;
}


BOOL ExtSheetBuffer::IsLink( const UINT16 nExcIndex ) const
{
    DBG_ASSERT( nExcIndex > 0, "*ExtSheetBuffer::IsLink(): Index muss >0 sein!" );
    Cont*   pRet = ( Cont * ) List::GetObject( nExcIndex - 1 );

    if( pRet )
        return pRet->bLink;
    else
        return FALSE;
}


BOOL ExtSheetBuffer::GetLink( const UINT16 nExcIndex, String& rAppl, String& rDoc ) const
{
    DBG_ASSERT( nExcIndex > 0, "*ExtSheetBuffer::GetLink(): Index muss >0 sein!" );
    Cont*   pRet = ( Cont * ) List::GetObject( nExcIndex - 1 );

    if( pRet )
    {
        rAppl = pRet->aFile;
        rDoc = pRet->aTab;
        return TRUE;
    }
    else
        return FALSE;
}


void ExtSheetBuffer::Reset( void )
{
    Cont    *pAkt = ( Cont * ) List::First();
    while( pAkt )
    {
        delete pAkt;
        pAkt = ( Cont * ) List::Next();
    }

    List::Clear();
}




BOOL ExtName::IsDDE( void ) const
{
    return ( nFlags & 0x0001 ) != 0;
}


BOOL ExtName::IsOLE( void ) const
{
    return ( nFlags & 0x0002 ) != 0;
}


ExtNameBuff::ExtNameBuff( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}


void ExtNameBuff::AddDDE( const String& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( rName, 0x0001 );
    maExtNames[ nRefIdx ].push_back( aNew );
}


void ExtNameBuff::AddOLE( const String& rName, sal_Int16 nRefIdx, UINT32 nStorageId )
{
    ExtName aNew( rName, 0x0002 );
    aNew.nStorageId = nStorageId;
    maExtNames[ nRefIdx ].push_back( aNew );
}


void ExtNameBuff::AddName( const String& rName, sal_Int16 nRefIdx )
{
    ExtName aNew( GetScAddInName( rName ), 0x0004 );
    maExtNames[ nRefIdx ].push_back( aNew );
}


const ExtName* ExtNameBuff::GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const
{
    DBG_ASSERT( nNameIdx > 0, "ExtNameBuff::GetNameByIndex() - invalid name index" );
    ExtNameMap::const_iterator aIt = maExtNames.find( nRefIdx );
    return ((aIt != maExtNames.end()) && (0 < nNameIdx) && (nNameIdx <= aIt->second.size())) ? &aIt->second[ nNameIdx - 1 ] : 0;
}


void ExtNameBuff::Reset( void )
{
    maExtNames.clear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
