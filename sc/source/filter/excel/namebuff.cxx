/*************************************************************************
 *
 *  $RCSfile: namebuff.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <tools/urlobj.hxx>
#include <string.h>

#include "rangenam.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "scextopt.hxx"

#include "spstring.hxx"
#include "namebuff.hxx"
#include "root.hxx"
#include "tokstack.hxx"
#include "flttools.hxx"


const UINT16 RangeNameBuffer::nError = 0xFFFF;




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




void RangeNameBuffer::Store( ByteString& r, const ScTokenArray* p, UINT16 n, const BOOL b )
{
    Store( String( r, *pExcRoot->pCharset ), p, n, b );
}


void RangeNameBuffer::Store( String& rName, const ScTokenArray* pDef, UINT16 nAltSheet, const BOOL bPrintarea )
{
    if( pDef )
    {
        DBG_ASSERT( Count() < 0xFFFF,
            "*RangeNameBuffer::Store(): max. 64K Names!" );

        // Name schon vorhanden?
        UINT16          nDummy;
        if( pExcRoot->pScRangeName->SearchName( rName, nDummy ) )
        {
            rName.AppendAscii( "___" );
            rName += String::CreateFromInt32( nAltSheet );
        }

        ScRangeData*    pData = new ScRangeData( pExcRoot->pDoc, rName, *pDef );

        pData->GuessPosition();
        pData->SetIndex( ( UINT16 ) Count() );

        if( bPrintarea )
            pData->AddType( RT_PRINTAREA );

        pExcRoot->pScRangeName->Insert( pData );

        Insert( ( void* ) TRUE, LIST_APPEND );
    }
    else
        Insert( ( void* ) FALSE, LIST_APPEND );
}




#ifdef DBG_UTIL
UINT16  nShrCnt;
#endif


ShrfmlaBuffer::ShrfmlaBuffer( RootData* pRD ) :
    ExcRoot( pRD )
{
    nBase = 16384;  // Range~ und Shared~ Dingens mit jeweils der Haelfte Ids

#ifdef DBG_UTIL
    nShrCnt = 0;
#endif
}


ShrfmlaBuffer::~ShrfmlaBuffer()
{
    register ScAddress* pDel = ( ScAddress* ) List::First();

    while( pDel )
    {
        delete pDel;
        pDel = ( ScAddress* ) List::Next();
    }
}


void ShrfmlaBuffer::Store( const ScRange& rRange, const ScTokenArray& rToken )
{
    String          aName( RTL_CONSTASCII_STRINGPARAM( "SHARED_FORMULA_" ) );
    aName += String::CreateFromInt32( List::Count() );

    DBG_ASSERT( List::Count() + nBase <= 0xFFFF, "*ShrfmlaBuffer::Store(): Gleich wird mir schlecht...!" );

    ScRangeData*    pData = new ScRangeData( pExcRoot->pDoc, aName, rToken, 0, 0, 0, RT_SHARED );

    pData->GuessPosition();
    pData->SetIndex( ( UINT16 ) ( List::Count() + nBase ) );

    pExcRoot->pScRangeName->Insert( pData );

    ScRange*        pNew = new ScRange( rRange );
    Insert( pNew, LIST_APPEND );
}


UINT16 ShrfmlaBuffer::Find( const ScAddress aAddr )
{
    register ScRange*   pAkt = ( ScRange* ) List::First();
    ScAddress           aSearchAddr( aAddr );

    register UINT16     nPos = nBase;

    while( pAkt )
    {
        if( pAkt->aStart == aSearchAddr )
            return nPos;

        nPos++;
        pAkt = ( ScRange* ) List::Next();
    }

    nPos = nBase;
    pAkt = ( ScRange* ) List::First();

    while( pAkt )
    {
        if( pAkt->In( aSearchAddr ) )
            return nPos;

        nPos++;
        pAkt = ( ScRange* ) List::Next();
    }

    return nPos;
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


void ExtSheetBuffer::AddLink( const String& rComplStr )
{
    String              aAppl;
    String              aDoc;

    const sal_Unicode*  pAct = rComplStr.GetBuffer();
    sal_Unicode         cAct = *pAct;
    BOOL                bDDE = FALSE;

    while( cAct )
    {
        if( cAct == 0x03 )
        {
            *( ( sal_Unicode* ) pAct ) = 0x00;
            aAppl = rComplStr.GetBuffer();
            *( ( sal_Unicode* ) pAct ) = cAct;
            cAct = 0x00;
            bDDE = TRUE;
            aDoc = pAct + 1;
        }
        else
        {
            pAct++;
            cAct = *pAct;
        }
    }

    if( !bDDE )
        aDoc = rComplStr;   // ????????????????????????????????????????????

    Cont*               pNew = new Cont( aAppl, aDoc );
    pNew->bLink = bDDE;

    List::Insert( pNew, LIST_APPEND );
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
            UINT16  nNewTabNum;
            if( pCur->bSWB )
            {// Tabelle ist im selben Workbook!
                if( pExcRoot->pDoc->GetTable( pCur->aTab, nNewTabNum ) )
                {
                    rScIndex = rTabNum = nNewTabNum;
                    return TRUE;
                }
                else
                    rTabNum = 0xFFFD;
            }
            else
            {// Tabelle ist 'echt' extern
                if( pExcRoot->pExtDocOpt->nLinkCnt < 1 )
                {
                    String      aURL( ScGlobal::GetAbsDocName( pCur->aFile,
                                        pExcRoot->pDoc->GetDocumentShell() ) );
                    String      aTabName( ScGlobal::GetDocTabName( aURL, pCur->aTab ) );
                    if( pExcRoot->pDoc->LinkExternalTab( nNewTabNum, aTabName, aURL, pCur->aTab ) )
                    {
                        rScIndex = rTabNum = nNewTabNum;
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


BOOL ExtName::IsName( void ) const
{
    return ( nFlags & 0x0004 ) != 0;
}




const sal_Char* ExtNameBuff::pJoostTest = "Joost ist doof!";


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
    ExtName*    pNew = new ExtName( rName );
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


