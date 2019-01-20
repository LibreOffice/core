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
#include <lotimpop.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include <document.hxx>
#include <formulacell.hxx>
#include <global.hxx>

#include <root.hxx>
#include <lotfntbf.hxx>
#include <lotform.hxx>
#include <tool.h>
#include <namebuff.hxx>
#include <lotattr.hxx>
#include <stringutil.hxx>

LOTUS_ROOT::LOTUS_ROOT( ScDocument* pDocP, rtl_TextEncoding eQ )
    :
        pDoc( pDocP),
        maRangeNames(),
        eCharsetQ( eQ),
        eFirstType( Lotus123Typ::X),
        eActType( Lotus123Typ::X),
        pRngNmBffWK3( new RangeNameBufferWK3() ),
        maAttrTable( this )
{
}

LOTUS_ROOT::~LOTUS_ROOT()
{
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
    rContext.pLotusRoot = nullptr;

    // no need 4 pLotusRoot anymore
    aLotImpSemaphore.release();
}

void ImportLotus::Bof()
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
            rContext.pLotusRoot->eFirstType = rContext.pLotusRoot->eActType = Lotus123Typ::WK3;
        }
        else if( nFileCode == 0x1002 )
        {// WK4
            rContext.pLotusRoot->eFirstType = rContext.pLotusRoot->eActType = Lotus123Typ::WK4;
        }
    }
}

bool ImportLotus::BofFm3()
{
    sal_uInt16  nFileCode, nFileSub;

    Read( nFileCode );
    Read( nFileSub );

    return ( nFileCode == 0x8007 && ( nFileSub == 0x0000 || nFileSub == 0x00001 ) );
}

void ImportLotus::Columnwidth( sal_uInt16 nRecLen )
{
    SAL_WARN_IF( nRecLen < 4, "sc.filter", "*ImportLotus::Columnwidth(): Record too short!" );

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
            pD->SetColWidth( static_cast<SCCOL> (nCol), static_cast<SCTAB> (nLTab), static_cast<sal_uInt16>( TWIPS_PER_CHAR * 1.28 * nSpaces ) );

            nCnt--;
        }
    }
}

void ImportLotus::Hiddencolumn( sal_uInt16 nRecLen )
{
    SAL_WARN_IF( nRecLen < 4, "sc.filter", "*ImportLotus::Hiddencolumn(): Record too short!" );

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

void ImportLotus::Userrange()
{
    sal_uInt16      nRangeType;
    ScRange     aScRange;

    Read( nRangeType );

    sal_Char aBuffer[ 17 ];
    pIn->ReadBytes(aBuffer, 16);
    aBuffer[ 16 ] = 0;
    OUString      aName( aBuffer, strlen(aBuffer), eQuellChar );

    Read( aScRange );

    LotusContext &rContext = aConv.getContext();
    rContext.pLotusRoot->pRngNmBffWK3->Add( aName, aScRange );
}

void ImportLotus::Errcell()
{
    ScAddress   aA;

    Read( aA );

    ScSetStringParam aParam;
    aParam.setTextInput();
    pD->EnsureTable(aA.Tab());
    pD->SetString(aA, "#ERR!", &aParam);
}

void ImportLotus::Nacell()
{
    ScAddress   aA;

    Read( aA );

    ScSetStringParam aParam;
    aParam.setTextInput();
    pD->EnsureTable(aA.Tab());
    pD->SetString(aA, "#NA!", &aParam);
}

void ImportLotus::Labelcell()
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

void ImportLotus::Numbercell()
{
    ScAddress   aAddr;
    double      fVal;

    Read( aAddr );
    Read( fVal );

    pD->EnsureTable(aAddr.Tab());
    pD->SetValue(aAddr, fVal);
}

void ImportLotus::Smallnumcell()
{
    ScAddress   aAddr;
    sal_Int16       nVal;

    Read( aAddr );
    Read( nVal );

    pD->EnsureTable(aAddr.Tab());
    pD->SetValue(aAddr, SnumToDouble(nVal));
}

void ImportLotus::Formulacell( sal_uInt16 n )
{
    SAL_WARN_IF( !pIn, "sc.filter", "-ImportLotus::Formulacell(): Null-Stream!" );

    ScAddress           aAddr;

    Read( aAddr );
    Skip( 10 );

    n -= std::min<sal_uInt16>(n, 14);

    std::unique_ptr<ScTokenArray> pErg;
    sal_Int32 nRest = n;

    aConv.Reset( aAddr );
    aConv.SetWK3();
    aConv.Convert( pErg, nRest );
    if (!aConv.good())
        return;

    ScFormulaCell* pCell = pErg ? new ScFormulaCell(pD, aAddr, std::move(pErg)) : new ScFormulaCell(pD, aAddr);
    pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE );
    pD->EnsureTable(aAddr.Tab());
    pD->SetFormulaCell(aAddr, pCell);
}

void ImportLotus::Read( OUString &r )
{
    ScfTools::AppendCString( *pIn, r, eQuellChar );
}

void ImportLotus::RowPresentation( sal_uInt16 nRecLen )
{
    SAL_WARN_IF( nRecLen < 5, "sc.filter", "*ImportLotus::RowPresentation(): Record too short!" );

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

        if( nFlags & 0x02 )     // Fixed / Stretch to fit fonts
        {   // fixed
            // Height in Lotus in 1/32 Points
            nHeight *= 20;  // -> 32 * TWIPS
            nHeight /= 32;  // -> TWIPS

            pD->SetRowFlags( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab), pD->GetRowFlags( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab) ) | CRFlags::ManualSize );

            pD->SetRowHeight( static_cast<SCROW> (nRow), static_cast<SCTAB> (nLTab), nHeight );
        }

        nCnt--;
    }
}

void ImportLotus::NamedSheet()
{
    sal_uInt16 nTmpTab(0);
    Read(nTmpTab);
    OUString aName;
    Read(aName);

    SCTAB nLTab(SanitizeTab(static_cast<SCTAB>(nTmpTab)));

    if (pD->HasTable(nLTab))
        pD->RenameTab(nLTab, aName);
    else
        pD->InsertTab(nLTab, aName);
}

void ImportLotus::Font_Face()
{
    sal_uInt8   nNum;
    OUString    aName;

    Read( nNum );

    if( nNum >= LotusFontBuffer::nSize )
        return;     // nonsense

    Read( aName );

    LotusContext &rContext = aConv.getContext();
    rContext.pLotusRoot->maFontBuff.SetName( nNum, aName );
}

void ImportLotus::Font_Type()
{
    LotusContext &rContext = aConv.getContext();
    for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nType;
        Read( nType );
        rContext.pLotusRoot->maFontBuff.SetType( nCnt, nType );
    }
}

void ImportLotus::Font_Ysize()
{
    LotusContext &rContext = aConv.getContext();
    for( sal_uInt16 nCnt = 0 ; nCnt < LotusFontBuffer::nSize ; nCnt++ )
    {
        sal_uInt16 nSize;
        Read( nSize );
        rContext.pLotusRoot->maFontBuff.SetHeight( nCnt, nSize );
    }
}

void ImportLotus::Row_( const sal_uInt16 nRecLen )
{
    SAL_WARN_IF( nExtTab < 0, "sc.filter", "*ImportLotus::Row_(): not possible!" );

    sal_uInt16            nCntDwn = (nRecLen < 4) ? 0 : ( nRecLen - 4 ) / 5;
    SCCOL           nColCnt = 0;
    sal_uInt8           nRepeats;
    LotAttrWK3      aAttr;

    bool            bCenter = false;
    SCCOL           nCenterStart = 0, nCenterEnd = 0;

    sal_uInt16 nTmpRow(0);
    Read(nTmpRow);
    SCROW nRow(SanitizeRow(static_cast<SCROW>(nTmpRow)));
    sal_uInt16 nHeight(0);
    Read(nHeight);

    nHeight &= 0x0FFF;
    nHeight *= 22;

    SCTAB nDestTab(static_cast<SCTAB>(nExtTab));

    if( nHeight )
        pD->SetRowHeight(nRow, nDestTab, nHeight);

    LotusContext &rContext = aConv.getContext();
    while( nCntDwn )
    {
        Read( aAttr );
        Read( nRepeats );

        if( aAttr.HasStyles() )
            rContext.pLotusRoot->maAttrTable.SetAttr(
                nColCnt, static_cast<SCCOL> ( nColCnt + nRepeats ), nRow, aAttr );

        // Do this here and NOT in class LotAttrTable, as we only add attributes if the other
        // attributes are set
        //  -> for Center-Attribute default is centered
        if( aAttr.IsCentered() )
        {
            if( bCenter )
            {
                if (pD->HasData(nColCnt, nRow, nDestTab))
                {
                    // new Center after previous Center
                    pD->DoMerge(nDestTab, nCenterStart, nRow, nCenterEnd, nRow);
                    nCenterStart = nColCnt;
                }
            }
            else
            {
                // fully new Center
                bCenter = true;
                nCenterStart = nColCnt;
            }
            nCenterEnd = nColCnt + static_cast<SCCOL>(nRepeats);
        }
        else
        {
            if( bCenter )
            {
                // possibly reset old Center
                pD->DoMerge(nDestTab, nCenterStart, nRow, nCenterEnd, nRow);
                bCenter = false;
            }
        }

        nColCnt = nColCnt + static_cast<SCCOL>(nRepeats);
        nColCnt++;

        nCntDwn--;
    }

    if( bCenter )
        // possibly reset old Center
        pD->DoMerge(nDestTab, nCenterStart, nRow, nCenterEnd, nRow);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
