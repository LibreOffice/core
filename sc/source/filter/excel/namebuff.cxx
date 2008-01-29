/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namebuff.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:24:45 $
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

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif


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


void NameBuffer::Reset()
{
    register StringHashEntry*   pDel = ( StringHashEntry* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( StringHashEntry* ) List::Next();
    }
    Clear();
}


BOOL NameBuffer::Find( const sal_Char* pRefName, UINT16& rIndex )
{
    StringHashEntry             aRefEntry( String::CreateFromAscii( pRefName ) );

    register StringHashEntry*   pFind = ( StringHashEntry* ) List::First();
    register UINT16             nCnt = nBase;
    while( pFind )
    {
        if( *pFind == aRefEntry )
        {
            rIndex = nCnt;
            return TRUE;
        }
        pFind = ( StringHashEntry* ) List::Next();
        nCnt++;
    }

    return FALSE;
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
    pData->SetIndex( static_cast< USHORT >( mnCurrIdx ) );
    pExcRoot->pIR->GetNamedRanges().Insert( pData );
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


void ExtSheetBuffer::Add( const String& rFPAN, const String& rTN, const BOOL bSWB )
{
    List::Insert( new Cont( rFPAN, rTN, bSWB ), LIST_APPEND );
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


BOOL ExtSheetBuffer::IsExternal( UINT16 nExcIndex ) const
{
    DBG_ASSERT( nExcIndex > 0, "*ExtSheetBuffer::IsExternal(): Index muss >0 sein!" );
    Cont*   pRet = ( Cont * ) List::GetObject( nExcIndex - 1 );

    if( pRet )
        return !pRet->bSWB;
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


BOOL ExtName::IsName( void ) const
{
    return ( nFlags & 0x0004 ) != 0;
}




const sal_Char* ExtNameBuff::pJoostTest = "Joost ist immer noch doof!";


ExtNameBuff::~ExtNameBuff()
{
    ExtName*    pDel = ( ExtName* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( ExtName* ) List::Next();
    }
}


void ExtNameBuff::AddDDE( const String& rName )
{
    ExtName*    pNew = new ExtName( rName );
    pNew->nFlags = 0x0001;

    List::Insert( pNew, LIST_APPEND );
}


void ExtNameBuff::AddOLE( const String& rName, UINT32 nStorageId )
{
    ExtName*    pNew = new ExtName( rName );
    pNew->nFlags = 0x0002;
    pNew->nStorageId = nStorageId;

    List::Insert( pNew, LIST_APPEND );
}


void ExtNameBuff::AddName( const String& rName )
{
    ExtName* pNew = new ExtName( pExcRoot->pIR->GetScAddInName( rName ) );
    pNew->nFlags = 0x0004;

    List::Insert( pNew, LIST_APPEND );
}


const ExtName* ExtNameBuff::GetName( const UINT16 nExcelIndex ) const
{
    DBG_ASSERT( nExcelIndex > 0, "*ExtNameBuff::GetName(): Index kann nur >0 sein!" );

    return ( const ExtName* ) List::GetObject( nExcelIndex - 1 );
}


void ExtNameBuff::Reset( void )
{
    ExtName*    pDel = ( ExtName* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( ExtName* ) List::Next();
    }

    sal_Char cTmp = *pJoostTest;
    cTmp++;

    List::Clear();
}


