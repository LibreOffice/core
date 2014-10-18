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

#include "lotfilter.hxx"
#include "lotimpop.hxx"
#include <osl/mutex.hxx>

#include "attrib.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "formulacell.hxx"
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
#include "stringutil.hxx"

LOTUS_ROOT::LOTUS_ROOT( ScDocument* pDocP, rtl_TextEncoding eQ )
    :
        pDoc( pDocP),
        pRangeNames( new LotusRangeList(this)),
        pScRangeName( pDocP->GetRangeName()),
        eCharsetQ( eQ),
        eFirstType( Lotus_X),
        eActType( Lotus_X),
        pRngNmBffWK3( new RangeNameBufferWK3(this)),
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

ImportLotus::ImportLotus(LotusContext &rContext, SvStream& aStream, ScDocument* pDoc, rtl_TextEncoding eQ)
    : ImportTyp(pDoc, eQ)
    , pIn(&aStream)
    , aConv(rContext, *pIn, pDoc->GetSharedStringPool(), eQ, false)
    , nTab(0)
    , nExtTab(0)
{
    // good point to start locking of import lotus
    aLotImpSemaphore.acquire();

    rContext.pLotusRoot = new LOTUS_ROOT(pDoc, eQ);
}

ImportLotus::~ImportLotus()
{
    LotusContext &rContext = aConv.getContext();
    delete rContext.pLotusRoot;
    rContext.pLotusRoot = NULL;

    // no need 4 pLotusRoot anymore
    aLotImpSemaphore.release();
}

void ImportLotus::Bof( void )
{
    sal_uInt16  nFileCode, nFileSub, nSaveCnt;
    sal_uInt8   nMajorId, nMinorId, nFlags;

    Read( nFileCode );
    Read( nFileSub );
    LotusContext &rContext = aConv.getContext();
    Read( rContext.pLotusRoot->aActRange );
    Read( nSaveCnt );
    Read( nMajorId );
    Read( nMinorId );
    Skip( 1 );
    Read( nFlags );

    if( nFileSub == 0x0004 )
    {
        if( nFileCode == 0x1000 )
        {// <= WK3
            rContext.pLotusRoot->eFirstType = rContext.pLotusRoot->eActType = Lotus_WK3;
        }
        else if( nFileCode == 0x1002 )
        {// WK4
            rContext.pLotusRoot->eFirstType = rContext.pLotusRoot->eActType = Lotus_WK4;
        }
    }
}

bool ImportLotus::BofFm3( void )
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
    sal_uInt16    nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 2;

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
            // Attention: ambiguous Correction factor!
            pD->SetColWidth( static_cast<SCCOL> (nCol), static_cast<SCTAB> (nLTab), ( sal_uInt16 ) ( TWIPS_PER_CHAR * 1.28 * nSpaces ) );

            nCnt--;
        }
    }
}

void ImportLotus::Hiddencolumn( sal_uInt16 nRecLen )
{
    OSL_ENSURE( nRecLen >= 4, "*ImportLotus::Hiddencolumn(): Record too short!" );

    sal_uInt8    nLTab, nWindow2;
    sal_uInt16    nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 2;

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
    OUString      aName( aBuffer, strlen(aBuffer), eQuellChar );

    Read( aScRange );

    LotusContext &rContext = aConv.getContext();
    rContext.pLotusRoot->pRngNmBffWK3->Add( aName, aScRange );
}

void ImportLotus::Errcell( void )
{
    ScAddress   aA;

    Read( aA );

    ScSetStringParam aParam;
    aParam.setTextInput();
    pD->EnsureTable(aA.Tab());
    pD->SetString(aA, "#ERR!", &aParam);
}

void ImportLotus::Nacell( void )
{
    ScAddress   aA;

    Read( aA );

    ScSetStringParam aParam;
    aParam.setTextInput();
    pD->EnsureTable(aA.Tab());
    pD->SetString(aA, "#NA!", &aParam);
}

void ImportLotus::Labelcell( void )
{
    ScAddress   aA;
    OUString    aLabel;
    sal_Char    cAlign;

    Read( aA );
    Read( cAlign );
    Read( aLabel );

    ScSetStringParam aParam;
    aParam.setTextInput();
    pD->EnsureTable(aA.Tab());
    pD->SetString(aA, aLabel, &aParam);
}

void ImportLotus::Numbercell( void )
{
    ScAddress   aAddr;
    double      fVal;

    Read( aAddr );
    Read( fVal );

    pD->EnsureTable(aAddr.Tab());
    pD->SetValue(aAddr, fVal);
}

void ImportLotus::Smallnumcell( void )
{
    ScAddress   aAddr;
    sal_Int16       nVal;

    Read( aAddr );
    Read( nVal );

    pD->EnsureTable(aAddr.Tab());
    pD->SetValue(aAddr, SnumToDouble(nVal));
}

ScFormulaCell *ImportLotus::Formulacell( sal_uInt16 n )
{
    OSL_ENSURE( pIn, "-ImportLotus::Formulacell(): Null-Stream!" );

    ScAddress           aAddr;

    Read( aAddr );
    Skip( 10 );

    n -= (n > 14) ? 14 : n;

    const ScTokenArray* pErg;
    sal_Int32 nRest = n;

    aConv.Reset( aAddr );
    aConv.SetWK3();
    aConv.Convert( pErg, nRest );

    ScFormulaCell* pCell = pErg ? new ScFormulaCell(pD, aAddr, *pErg) : new ScFormulaCell(pD, aAddr);
    pCell->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );
    pD->EnsureTable(aAddr.Tab());
    pD->SetFormulaCell(aAddr, pCell);

    return NULL;
}

void ImportLotus::Read( OUString &r )
{
    ScfTools::AppendCString( *pIn, r, eQuellChar );
}

void ImportLotus::RowPresentation( sal_uInt16 nRecLen )
{
    OSL_ENSURE( nRecLen > 4, "*ImportLotus::RowPresentation(): Record too short!" );

    sal_uInt8    nLTab, nFlags;
    sal_uInt16  nRow, nHeight;
    sal_uInt16    nCnt = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 8;

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
    OUString    aName;

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
    OUString    aName;

    Read( nNum );

    if( nNum >= LotusFontBuffer::nSize )
        return;     // nonsense

    Read( aName );

    LotusContext &rContext = aConv.getContext();
    rContext.pLotusRoot->pFontBuff->SetName( nNum, aName );
}

void ImportLotus::Font_Type( void )
{
    LotusContext &rContext = aConv.getContext();
    for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nType;
        Read( nType );
        rContext.pLotusRoot->pFontBuff->SetType( nCnt, nType );
    }
}

void ImportLotus::Font_Ysize( void )
{
    LotusContext &rContext = aConv.getContext();
    for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nSize;
        Read( nSize );
        rContext.pLotusRoot->pFontBuff->SetHeight( nCnt, nSize );
    }
}

void ImportLotus::_Row( const sal_uInt16 nRecLen )
{
    OSL_ENSURE( nExtTab >= 0, "*ImportLotus::_Row(): not possible!" );

    sal_uInt16          nRow;
    sal_uInt16          nHeight;
    sal_uInt16            nCntDwn = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 5;
    SCCOL           nColCnt = 0;
    sal_uInt8           nRepeats;
    LotAttrWK3      aAttr;

    bool            bCenter = false;
    SCCOL           nCenterStart = 0, nCenterEnd = 0;

    Read( nRow );
    Read( nHeight );

    nHeight &= 0x0FFF;
    nHeight *= 22;

    if( nHeight )
        pD->SetRowHeight( static_cast<SCROW> (nRow), static_cast<SCTAB> (nExtTab), nHeight );

    LotusContext &rContext = aConv.getContext();
    while( nCntDwn )
    {
        Read( aAttr );
        Read( nRepeats );

        if( aAttr.HasStyles() )
            rContext.pLotusRoot->pAttrTable->SetAttr(
                nColCnt, static_cast<SCCOL> ( nColCnt + nRepeats ), static_cast<SCROW> (nRow), aAttr );

        // Do this here and NOT in class LotAttrTable, as we only add attributes if the other
        // attributes are set
        //  -> for Center-Attribute default is centered
        if( aAttr.IsCentered() )
            {
            if( bCenter )
                {
                if( pD->HasData( nColCnt, static_cast<SCROW> (nRow), static_cast<SCTAB> (nExtTab) ) )
                    {// new Center after previous Center
                    pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
                    nCenterStart = nColCnt;
                    }
                }
            else
                {// fully new Center
                bCenter = true;
                nCenterStart = nColCnt;
                }
            nCenterEnd = nColCnt + static_cast<SCCOL>(nRepeats);
            }
        else
            {
            if( bCenter )
                {// possibly reset old Center
                pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
                bCenter = false;
                }
            }

        nColCnt = nColCnt + static_cast<SCCOL>(nRepeats);
        nColCnt++;

        nCntDwn--;
    }

    if( bCenter )
        // possibly reset old Center
        pD->DoMerge( static_cast<SCTAB> (nExtTab), nCenterStart, static_cast<SCROW> (nRow), nCenterEnd, static_cast<SCROW> (nRow) );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
