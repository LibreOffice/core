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

#include "lotimpop.hxx"
#include <osl/mutex.hxx>

#include "attrib.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "compiler.hxx"
#include "global.hxx"

#include "root.hxx"
#include "lotfntbf.hxx"
#include "lotform.hxx"
#include "tool.h"
#include "namebuff.hxx"
#include "lotrange.hxx"
#include "lotattr.hxx"

LOTUS_ROOT::LOTUS_ROOT( ScDocument* pDocP, CharSet eQ )
    :
        pDoc( pDocP),
        pRangeNames( new LotusRangeList),
        pScRangeName( pDocP->GetRangeName()),
        eCharsetQ( eQ),
        eFirstType( Lotus_X),
        eActType( Lotus_X),
        pRngNmBffWK3( new RangeNameBufferWK3),
        pFontBuff( new LotusFontBuffer),
        pAttrTable( new LotAttrTable(this))
{
}


LOTUS_ROOT::~LOTUS_ROOT()
{
	delete pRangeNames;
	delete pRngNmBffWK3;
	delete pFontBuff;
	delete pAttrTable;
}


static osl::Mutex aLotImpSemaphore;


ImportLotus::ImportLotus( SvStream& aStream, ScDocument* pDoc, CharSet eQ ) :
    ImportTyp( pDoc, eQ ),
    pIn( &aStream ),
    aConv( *pIn, eQ, false )
{
    // good point to start locking of import lotus
    aLotImpSemaphore.acquire();

	pLotusRoot = new LOTUS_ROOT( pDoc, eQ);
}


ImportLotus::~ImportLotus()
{
    delete pLotusRoot;
    pLotusRoot = NULL;

    // no need 4 pLotusRoot anymore
    aLotImpSemaphore.release();
}


void ImportLotus::Bof( void )
{
    sal_uInt16  nFileCode, nFileSub, nSaveCnt;
    sal_uInt8   nMajorId, nMinorId, nFlags;

    Read( nFileCode );
    Read( nFileSub );
    Read( pLotusRoot->aActRange );
    Read( nSaveCnt );
    Read( nMajorId );
    Read( nMinorId );
    Skip( 1 );
    Read( nFlags );

    if( nFileSub == 0x0004 )
    {
        if( nFileCode == 0x1000 )
        {// <= WK3
            pLotusRoot->eFirstType = pLotusRoot->eActType = Lotus_WK3;
        }
        else if( nFileCode == 0x1002 )
        {// WK4
            pLotusRoot->eFirstType = pLotusRoot->eActType = Lotus_WK4;
        }
    }
}


sal_Bool ImportLotus::BofFm3( void )
{
    sal_uInt16  nFileCode, nFileSub;

    Read( nFileCode );
    Read( nFileSub );

    return ( nFileCode == 0x8007 && ( nFileSub == 0x0000 || nFileSub == 0x00001 ) );
}


void ImportLotus::Columnwidth( sal_uInt16 nRecLen )
{
    OSL_ENSURE( nRecLen >= 4, "*ImportLotus::Columnwidth(): Record zu kurz!" );

    sal_uInt8    nLTab, nWindow2;
	sal_uInt16	nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 2;

    Read( nLTab );
    Read( nWindow2 );

    if( !pD->HasTable( static_cast<SCTAB> (nLTab) ) )
        pD->MakeTable( static_cast<SCTAB> (nLTab) );

    if( !nWindow2 )
    {
        Skip( 2 );

        sal_uInt8   nCol, nSpaces;

        while( nCnt )
        {
            Read( nCol );
            Read( nSpaces );
            // ACHTUNG: Korrekturfaktor nach 'Augenmass' ermittelt!
            pD->SetColWidth( static_cast<SCCOL> (nCol), static_cast<SCTAB> (nLTab), ( sal_uInt16 ) ( TWIPS_PER_CHAR * 1.28 * nSpaces ) );

            nCnt--;
        }
    }
}


void ImportLotus::Hiddencolumn( sal_uInt16 nRecLen )
{
    OSL_ENSURE( nRecLen >= 4, "*ImportLotus::Hiddencolumn(): Record zu kurz!" );

    sal_uInt8    nLTab, nWindow2;
	sal_uInt16	nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 2;

    Read( nLTab );
    Read( nWindow2 );

    if( !nWindow2 )
    {
        Skip( 2 );

        sal_uInt8   nCol;

        while( nCnt )
        {
            Read( nCol );

            pD->SetColHidden(static_cast<SCCOL>(nCol), static_cast<SCCOL>(nCol), static_cast<SCTAB>(nLTab), true);
            nCnt--;
        }
    }
}


void ImportLotus::Userrange( void )
{
    sal_uInt16      nRangeType;
    ScRange     aScRange;

    Read( nRangeType );

	sal_Char aBuffer[ 17 ];
	pIn->Read( aBuffer, 16 );
	aBuffer[ 16 ] = 0;
	String		aName( aBuffer, eQuellChar );

    Read( aScRange );

    pLotusRoot->pRngNmBffWK3->Add( aName, aScRange );
}


void ImportLotus::Errcell( void )
{
    ScAddress   aA;

    Read( aA );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( CREATE_STRING( "#ERR!" ) ), true );
}


void ImportLotus::Nacell( void )
{
    ScAddress   aA;

    Read( aA );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( CREATE_STRING( "#NA!" ) ), true );
}


void ImportLotus::Labelcell( void )
{
    ScAddress   aA;
    String      aLabel;
    sal_Char    cAlign;

    Read( aA );
    Read( cAlign );
    Read( aLabel );

//  aLabel.Convert( pLotusRoot->eCharsetQ );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( aLabel ), true );
}


void ImportLotus::Numbercell( void )
{
    ScAddress   aAddr;
    double      fVal;

    Read( aAddr );
    Read( fVal );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
        new ScValueCell( fVal ), true );
    }


void ImportLotus::Smallnumcell( void )
    {
    ScAddress   aAddr;
    sal_Int16       nVal;

    Read( aAddr );
    Read( nVal );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
        new ScValueCell( SnumToDouble( nVal ) ), true );
    }


ScFormulaCell *ImportLotus::Formulacell( sal_uInt16 n )
    {
    OSL_ENSURE( pIn, "-ImportLotus::Formulacell(): Null-Stream -> Rums!" );

    ScAddress           aAddr;

    Read( aAddr );
    Skip( 10 );

	n -= (n > 14) ? 14 : n;

    const ScTokenArray* pErg;
    sal_Int32               nRest = n;

    aConv.Reset( aAddr );
    aConv.SetWK3();
    aConv.Convert( pErg, nRest );

    ScFormulaCell*      pZelle = new ScFormulaCell( pD, aAddr, pErg );

    pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(), pZelle, true );

    return NULL;
}


void ImportLotus::Read( String &r )
{
    ScfTools::AppendCString( *pIn, r, eQuellChar );
}


void ImportLotus::RowPresentation( sal_uInt16 nRecLen )
{
    OSL_ENSURE( nRecLen > 4, "*ImportLotus::RowPresentation(): Record zu kurz!" );

    sal_uInt8    nLTab, nFlags;
    sal_uInt16  nRow, nHeight;
	sal_uInt16	nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 8;

    Read( nLTab );
    Skip( 1 );

    while( nCnt )
    {
        Read( nRow );
        Read( nHeight );
        Skip( 2 );
        Read( nFlags );
        Skip( 1 );

        if( nFlags & 0x02 )     // Fixed / Strech to fit fonts
        {   // fixed
            // Height in Lotus in 1/32 Points
            nHeight *= 20;  // -> 32 * TWIPS
            nHeight /= 32;  // -> TWIPS

            pD->SetRowFlags( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab), pD->GetRowFlags( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab) ) | CR_MANUALSIZE );

            pD->SetRowHeight( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab), nHeight );
        }

        nCnt--;
    }
}


void ImportLotus::NamedSheet( void )
{
    sal_uInt16  nLTab;
    String  aName;

    Read( nLTab );
    Read( aName );

    if( pD->HasTable( static_cast<SCTAB> (nLTab) ) )
        pD->RenameTab( static_cast<SCTAB> (nLTab), aName );
    else
        pD->InsertTab( static_cast<SCTAB> (nLTab), aName );
}


void ImportLotus::Font_Face( void )
{
    sal_uInt8   nNum;
    String  aName;

    Read( nNum );

	if( nNum >= LotusFontBuffer::nSize )
		return;     // nonsense

    Read( aName );

	pLotusRoot->pFontBuff->SetName( nNum, aName );
}


void ImportLotus::Font_Type( void )
{
	for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nType;
        Read( nType );
		pLotusRoot->pFontBuff->SetType( nCnt, nType );
    }
}


void ImportLotus::Font_Ysize( void )
{
	for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nSize;
        Read( nSize );
		pLotusRoot->pFontBuff->SetHeight( nCnt, nSize );
    }
}


void ImportLotus::_Row( const sal_uInt16 nRecLen )
{
    OSL_ENSURE( nExtTab >= 0, "*ImportLotus::_Row(): Kann hier nicht sein!" );

    sal_uInt16          nRow;
    sal_uInt16          nHeight;
	sal_uInt16			nCntDwn = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 5;
    SCCOL           nColCnt = 0;
    sal_uInt8           nRepeats;
    LotAttrWK3      aAttr;

    sal_Bool            bCenter = false;
    SCCOL           nCenterStart = 0, nCenterEnd = 0;

    Read( nRow );
    Read( nHeight );

    nHeight &= 0x0FFF;
    nHeight *= 22;

    if( nHeight )
        pD->SetRowHeight( static_cast<SCROW> (nRow), static_cast<SCTAB> (nExtTab), nHeight );

    while( nCntDwn )
        {
        Read( aAttr );
        Read( nRepeats );

        if( aAttr.HasStyles() )
            pLotusRoot->pAttrTable->SetAttr(
                nColCnt, static_cast<SCCOL> ( nColCnt + nRepeats ), static_cast<SCROW> (nRow), aAttr );

        // hier und NICHT in class LotAttrTable, weil nur Attributiert wird,
        // wenn die anderen Attribute gesetzt sind
        //  -> bei Center-Attribute wird generell zentriert gesetzt
        if( aAttr.IsCentered() )
            {
            if( bCenter )
                {
                if( pD->HasData( nColCnt, static_cast<SCROW> (nRow), static_cast<SCTAB> (nExtTab) ) )
                    {// neue Center nach vorheriger Center
                    pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
                    nCenterStart = nColCnt;
                    }
                }
            else
                {// ganz neue Center
                bCenter = sal_True;
                nCenterStart = nColCnt;
                }
            nCenterEnd = nColCnt + static_cast<SCCOL>(nRepeats);
            }
        else
            {
            if( bCenter )
                {// evtl. alte Center bemachen
                pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
                bCenter = false;
                }
            }

        nColCnt = nColCnt + static_cast<SCCOL>(nRepeats);
        nColCnt++;

        nCntDwn--;
        }

    if( bCenter )
        // evtl. alte Center bemachen
        pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
