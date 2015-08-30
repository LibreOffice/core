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

#undef SC_DLLIMPLEMENTATION

#include "scitems.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "userlist.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "subtdlg.hxx"
#include "tpsubt.hxx"
#include <memory>

// Subtotals group tabpage:

ScTpSubTotalGroup::ScTpSubTotalGroup( vcl::Window* pParent,
                                      const SfxItemSet& rArgSet )
        :   SfxTabPage      ( pParent,
                              "SubTotalGrpPage", "modules/scalc/ui/subtotalgrppage.ui",
                              &rArgSet ),
            aStrNone        ( SC_RESSTR( SCSTR_NONE ) ),
            aStrColumn      ( SC_RESSTR( SCSTR_COLUMN ) ),
            pViewData       ( NULL ),
            pDoc            ( NULL ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( static_cast<const ScSubTotalItem&>(
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() ),
            nFieldCount     ( 0 )
{
    get(mpLbGroup, "group_by");
    get(mpLbColumns, "columns");
    get(mpLbFunctions, "functions");

    long nHeight = mpLbColumns->GetTextHeight() * 14;
    mpLbColumns->set_height_request(nHeight);
    mpLbFunctions->set_height_request(nHeight);

    // Font is correctly initialized by SvTreeListBox ctor
    mpLbColumns->SetSelectionMode( SINGLE_SELECTION );
    mpLbColumns->SetDragDropMode( DragDropMode::NONE );
    mpLbColumns->SetSpaceBetweenEntries( 0 );

    Init ();
}

ScTpSubTotalGroup::~ScTpSubTotalGroup()
{
    disposeOnce();
}

void ScTpSubTotalGroup::dispose()
{
    sal_uLong  nCount = mpLbColumns->GetEntryCount();

    if ( nCount > 0 )
    {
        for ( sal_uLong i=0; i<nCount; i++ )
        {
            sal_uInt16* pData = static_cast<sal_uInt16*>(mpLbColumns->GetEntryData( i ));
            OSL_ENSURE( pData, "EntryData not found" );

            delete pData;
        }
    }
    mpLbGroup.clear();
    mpLbColumns.clear();
    mpLbFunctions.clear();
    SfxTabPage::dispose();
}

void ScTpSubTotalGroup::Init()
{
    const ScSubTotalItem& rSubTotalItem = static_cast<const ScSubTotalItem&>(
                                          GetItemSet().Get( nWhichSubTotals ));

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found :-(" );

    mpLbGroup->SetSelectHdl       ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );
    mpLbColumns->SetSelectHdl     ( LINK( this, ScTpSubTotalGroup, SelectTreeListBoxHdl ) );
    mpLbColumns->SetCheckButtonHdl( LINK( this, ScTpSubTotalGroup, CheckHdl ) );
    mpLbFunctions->SetSelectHdl   ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );

    nFieldArr[0] = 0;
    FillListBoxes();
}

bool ScTpSubTotalGroup::DoReset( sal_uInt16             nGroupNo,
                                 const SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );

    if ( (nGroupNo > 3) || (nGroupNo == 0) )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    // first we have to clear the listboxes...
    for ( sal_uLong nLbEntry = 0; nLbEntry < mpLbColumns->GetEntryCount(); ++nLbEntry )
    {
        mpLbColumns->CheckEntryPos( nLbEntry, false );
        *static_cast<sal_uInt16*>(mpLbColumns->GetEntryData( nLbEntry )) = 0;
    }
    mpLbFunctions->SelectEntryPos( 0 );

    ScSubTotalParam theSubTotalData( static_cast<const ScSubTotalItem&>(
                                      rArgSet.Get( nWhichSubTotals )).
                                            GetSubTotalData() );

    if ( theSubTotalData.bGroupActive[nGroupIdx] )
    {
        SCCOL           nField      = theSubTotalData.nField[nGroupIdx];
        SCCOL           nSubTotals  = theSubTotalData.nSubTotals[nGroupIdx];
        SCCOL*          pSubTotals  = theSubTotalData.pSubTotals[nGroupIdx];
        ScSubTotalFunc* pFunctions  = theSubTotalData.pFunctions[nGroupIdx];

        mpLbGroup->SelectEntryPos( GetFieldSelPos( nField )+1 );

        sal_uInt16 nFirstChecked = 0;
        for ( sal_uInt16 i=0; i<nSubTotals; i++ )
        {
            sal_uInt16  nCheckPos = GetFieldSelPos( pSubTotals[i] );
            sal_uInt16* pFunction = static_cast<sal_uInt16*>(mpLbColumns->GetEntryData( nCheckPos ));

            mpLbColumns->CheckEntryPos( nCheckPos );
            *pFunction = FuncToLbPos( pFunctions[i] );

            if (i == 0 || (i > 0 && nCheckPos < nFirstChecked))
                nFirstChecked = nCheckPos;
        }
        // Select the first checked field from the top.
        mpLbColumns->SelectEntryPos(nFirstChecked);
    }
    else
    {
        mpLbGroup->SelectEntryPos( (nGroupNo == 1) ? 1 : 0 );
        mpLbColumns->SelectEntryPos( 0 );
        mpLbFunctions->SelectEntryPos( 0 );
    }

    return true;
}

bool ScTpSubTotalGroup::DoFillItemSet( sal_uInt16       nGroupNo,
                                       SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );
    OSL_ENSURE(    (mpLbGroup->GetEntryCount() > 0)
                && (mpLbColumns->GetEntryCount() > 0)
                && (mpLbFunctions->GetEntryCount() > 0),
                "Non-initialized Lists" );

    if (  (nGroupNo > 3) || (nGroupNo == 0)
        || (mpLbGroup->GetEntryCount() == 0)
        || (mpLbColumns->GetEntryCount() == 0)
        || (mpLbFunctions->GetEntryCount() == 0)
       )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, true, &pItem ) == SfxItemState::SET )
            theSubTotalData = static_cast<const ScSubTotalItem*>(pItem)->GetSubTotalData();
    }

    std::unique_ptr<ScSubTotalFunc[]> pFunctions;
    std::unique_ptr<SCCOL[]>          pSubTotals;
    const sal_Int32 nGroup      = mpLbGroup->GetSelectEntryPos();
    const sal_Int32 nEntryCount = mpLbColumns->GetEntryCount();
    const sal_Int32 nCheckCount = mpLbColumns->GetCheckedEntryCount();

    theSubTotalData.nCol1                   = rSubTotalData.nCol1;
    theSubTotalData.nRow1                   = rSubTotalData.nRow1;
    theSubTotalData.nCol2                   = rSubTotalData.nCol2;
    theSubTotalData.nRow2                   = rSubTotalData.nRow2;
    theSubTotalData.bGroupActive[nGroupIdx] = (nGroup != 0);
    theSubTotalData.nField[nGroupIdx]       = (nGroup != 0)
                                                ? nFieldArr[nGroup-1]
                                                : static_cast<SCCOL>(0);

    if ( nEntryCount>0 && nCheckCount>0 && nGroup!=0 )
    {
        sal_uInt16 nFunction    = 0;

        pSubTotals.reset(new SCCOL          [nCheckCount]);
        pFunctions.reset(new ScSubTotalFunc [nCheckCount]);

        for ( sal_Int32 i=0, nCheck=0; i<nEntryCount; i++ )
        {
            if ( mpLbColumns->IsChecked( i ) )
            {
                OSL_ENSURE( nCheck <= nCheckCount,
                            "Range error :-(" );
                nFunction = *static_cast<sal_uInt16*>(mpLbColumns->GetEntryData( i ));
                pSubTotals[nCheck] = nFieldArr[i];
                pFunctions[nCheck] = LbPosToFunc( nFunction );
                nCheck++;
            }
        }
        theSubTotalData.SetSubTotals( nGroupNo,      // Gruppen-Nr.
                                      pSubTotals.get(),
                                      pFunctions.get(),
                                      nCheckCount ); // Anzahl der Array-Elemente

    }

    rArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, &theSubTotalData ) );

    return true;
}

void ScTpSubTotalGroup::FillListBoxes()
{
    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found :-/" );

    if ( pViewData && pDoc )
    {
        SCCOL   nFirstCol   = rSubTotalData.nCol1;
        SCROW   nFirstRow   = rSubTotalData.nRow1;
        SCTAB   nTab        = pViewData->GetTabNo();
        SCCOL   nMaxCol     = rSubTotalData.nCol2;
        SCCOL   col;
        OUString  aFieldName;

        mpLbGroup->Clear();
        mpLbColumns->Clear();
        mpLbGroup->InsertEntry( aStrNone, 0 );

        sal_uInt16 i=0;
        for ( col=nFirstCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if ( aFieldName.isEmpty() )
            {
                aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
            }
            nFieldArr[i] = col;
            mpLbGroup->InsertEntry( aFieldName, i+1 );
            mpLbColumns->InsertEntry( aFieldName, i );
            mpLbColumns->SetEntryData( i, new sal_uInt16(0) );
            i++;
        }
        // subsequent initialization of the constant:
        (sal_uInt16&)nFieldCount = i;
    }
}

sal_uInt16 ScTpSubTotalGroup::GetFieldSelPos( SCCOL nField )
{
    sal_uInt16  nFieldPos   = 0;
    bool    bFound      = false;

    for ( sal_uInt16 n=0; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = true;
        }
    }

    return nFieldPos;
}

ScSubTotalFunc ScTpSubTotalGroup::LbPosToFunc( sal_uInt16 nPos )
{
    switch ( nPos )
    {
//      case  0:    return SUBTOTAL_FUNC_NONE;
        case  2:    return SUBTOTAL_FUNC_AVE;
        case  6:    return SUBTOTAL_FUNC_CNT;
        case  1:    return SUBTOTAL_FUNC_CNT2;
        case  3:    return SUBTOTAL_FUNC_MAX;
        case  4:    return SUBTOTAL_FUNC_MIN;
        case  5:    return SUBTOTAL_FUNC_PROD;
        case  7:    return SUBTOTAL_FUNC_STD;
        case  8:    return SUBTOTAL_FUNC_STDP;
        case  0:    return SUBTOTAL_FUNC_SUM;
        case  9:    return SUBTOTAL_FUNC_VAR;
        case 10:    return SUBTOTAL_FUNC_VARP;
        default:
            OSL_FAIL( "ScTpSubTotalGroup::LbPosToFunc" );
            return SUBTOTAL_FUNC_NONE;
    }
}

sal_uInt16 ScTpSubTotalGroup::FuncToLbPos( ScSubTotalFunc eFunc )
{
    switch ( eFunc )
    {
//      case SUBTOTAL_FUNC_NONE:    return 0;
        case SUBTOTAL_FUNC_AVE:     return 2;
        case SUBTOTAL_FUNC_CNT:     return 6;
        case SUBTOTAL_FUNC_CNT2:    return 1;
        case SUBTOTAL_FUNC_MAX:     return 3;
        case SUBTOTAL_FUNC_MIN:     return 4;
        case SUBTOTAL_FUNC_PROD:    return 5;
        case SUBTOTAL_FUNC_STD:     return 7;
        case SUBTOTAL_FUNC_STDP:    return 8;
        case SUBTOTAL_FUNC_SUM:     return 0;
        case SUBTOTAL_FUNC_VAR:     return 9;
        case SUBTOTAL_FUNC_VARP:    return 10;
        default:
            OSL_FAIL( "ScTpSubTotalGroup::FuncToLbPos" );
            return 0;
    }
}

// Handler:

IMPL_LINK_TYPED( ScTpSubTotalGroup, SelectTreeListBoxHdl, SvTreeListBox*, pLb, void )
{
    SelectHdl(pLb);
}
IMPL_LINK( ScTpSubTotalGroup, SelectHdl, void *, pLb )
{
    if (   (mpLbColumns->GetEntryCount() > 0)
        && (mpLbColumns->GetSelectionCount() > 0) )
    {
        const sal_Int32 nFunction   = mpLbFunctions->GetSelectEntryPos();
        const sal_Int32 nColumn     = mpLbColumns->GetSelectEntryPos();
        sal_uInt16*     pFunction   = static_cast<sal_uInt16*>(mpLbColumns->GetEntryData( nColumn ));

        OSL_ENSURE( pFunction, "EntryData not found!" );
        if ( !pFunction )
            return 0;

        if ( pLb == mpLbColumns )
        {
            mpLbFunctions->SelectEntryPos( *pFunction );
        }
        else if ( pLb == mpLbFunctions )
        {
            *pFunction = static_cast<sal_uInt16>(nFunction);
            mpLbColumns->CheckEntryPos( nColumn );
        }
    }
    return 0;
}

IMPL_LINK_TYPED( ScTpSubTotalGroup, CheckHdl, SvTreeListBox*, pLb, void )
{
    if ( pLb == mpLbColumns )
    {
        SvTreeListEntry* pEntry = mpLbColumns->GetHdlEntry();

        if ( pEntry )
        {
            mpLbColumns->SelectEntryPos( (sal_uInt16)mpLbColumns->GetModel()->GetAbsPos( pEntry ) );
            SelectHdl( pLb );
        }
    }
}

// Derived Group TabPages:

VclPtr<SfxTabPage> ScTpSubTotalGroup1::Create( vcl::Window*         pParent,
                                                 const SfxItemSet*  rArgSet )
{
    return VclPtr<ScTpSubTotalGroup1>::Create( pParent, *rArgSet );
}

VclPtr<SfxTabPage> ScTpSubTotalGroup2::Create( vcl::Window*          pParent,
                                       const SfxItemSet*    rArgSet )
{
    return VclPtr<ScTpSubTotalGroup2>::Create( pParent, *rArgSet );
}

VclPtr<SfxTabPage> ScTpSubTotalGroup3::Create( vcl::Window*          pParent,
                                       const SfxItemSet*    rArgSet )
{
    return VclPtr<ScTpSubTotalGroup3>::Create( pParent, *rArgSet );
}

ScTpSubTotalGroup1::ScTpSubTotalGroup1( vcl::Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

ScTpSubTotalGroup2::ScTpSubTotalGroup2( vcl::Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

ScTpSubTotalGroup3::ScTpSubTotalGroup3( vcl::Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, rArgSet )
{}

#define RESET(i) (ScTpSubTotalGroup::DoReset( (i), *rArgSet ))
void ScTpSubTotalGroup1::Reset( const SfxItemSet* rArgSet ) { RESET(1); }
void ScTpSubTotalGroup2::Reset( const SfxItemSet* rArgSet ) { RESET(2); }
void ScTpSubTotalGroup3::Reset( const SfxItemSet* rArgSet ) { RESET(3); }
#undef RESET

#define FILLSET(i) (ScTpSubTotalGroup::DoFillItemSet( (i), *rArgSet ))
bool ScTpSubTotalGroup1::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(1); }
bool ScTpSubTotalGroup2::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(2); }
bool ScTpSubTotalGroup3::FillItemSet( SfxItemSet* rArgSet ) { return FILLSET(3); }
#undef FILL

// Optionen-Tabpage:

ScTpSubTotalOptions::ScTpSubTotalOptions( vcl::Window*               pParent,
                                          const SfxItemSet&     rArgSet )

        :   SfxTabPage      ( pParent,
                              "SubTotalOptionsPage", "modules/scalc/ui/subtotaloptionspage.ui" ,
                              &rArgSet ),
            pViewData       ( NULL ),
            pDoc            ( NULL ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( static_cast<const ScSubTotalItem&>(
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() )
{
    get(pBtnPagebreak,"pagebreak");
    get(pBtnCase,"case");
    get(pBtnSort,"sort");
    get(pFlSort,"label2");
    get(pBtnAscending,"ascending");
    get(pBtnDescending,"descending");
    get(pBtnFormats,"formats");
    get(pBtnUserDef,"btnuserdef");
    get(pLbUserDef,"lbuserdef");

    Init();
}

ScTpSubTotalOptions::~ScTpSubTotalOptions()
{
    disposeOnce();
}

void ScTpSubTotalOptions::dispose()
{
    pBtnPagebreak.clear();
    pBtnCase.clear();
    pBtnSort.clear();
    pFlSort.clear();
    pBtnAscending.clear();
    pBtnDescending.clear();
    pBtnFormats.clear();
    pBtnUserDef.clear();
    pLbUserDef.clear();
    SfxTabPage::dispose();
}

void ScTpSubTotalOptions::Init()
{
    const ScSubTotalItem& rSubTotalItem = static_cast<const ScSubTotalItem&>(
                                          GetItemSet().Get( nWhichSubTotals ));

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found!" );

    pBtnSort->SetClickHdl    ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );
    pBtnUserDef->SetClickHdl ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );

    FillUserSortListBox();
}

VclPtr<SfxTabPage> ScTpSubTotalOptions::Create( vcl::Window* pParent,
                                                const SfxItemSet* rArgSet )
{
    return VclPtr<ScTpSubTotalOptions>::Create( pParent, *rArgSet );
}

void ScTpSubTotalOptions::Reset( const SfxItemSet* /* rArgSet */ )
{
    pBtnPagebreak->Check ( rSubTotalData.bPagebreak );
    pBtnCase->Check      ( rSubTotalData.bCaseSens );
    pBtnFormats->Check   ( rSubTotalData.bIncludePattern );
    pBtnSort->Check      ( rSubTotalData.bDoSort );
    pBtnAscending->Check ( rSubTotalData.bAscending );
    pBtnDescending->Check( !rSubTotalData.bAscending );

    if ( rSubTotalData.bUserDef )
    {
        pBtnUserDef->Check();
        pLbUserDef->Enable();
        pLbUserDef->SelectEntryPos( rSubTotalData.nUserIndex );
    }
    else
    {
        pBtnUserDef->Check( false );
        pLbUserDef->Disable();
        pLbUserDef->SelectEntryPos( 0 );
    }

    CheckHdl( pBtnSort );
}

bool ScTpSubTotalOptions::FillItemSet( SfxItemSet* rArgSet )
{
    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, true, &pItem ) == SfxItemState::SET )
            theSubTotalData = static_cast<const ScSubTotalItem*>(pItem)->GetSubTotalData();
    }

    theSubTotalData.bPagebreak      = pBtnPagebreak->IsChecked();
    theSubTotalData.bReplace        = true;
    theSubTotalData.bCaseSens       = pBtnCase->IsChecked();
    theSubTotalData.bIncludePattern = pBtnFormats->IsChecked();
    theSubTotalData.bDoSort         = pBtnSort->IsChecked();
    theSubTotalData.bAscending      = pBtnAscending->IsChecked();
    theSubTotalData.bUserDef        = pBtnUserDef->IsChecked();
    theSubTotalData.nUserIndex      = (pBtnUserDef->IsChecked())
                                    ? pLbUserDef->GetSelectEntryPos()
                                    : 0;

    rArgSet->Put( ScSubTotalItem( nWhichSubTotals, &theSubTotalData ) );

    return true;
}

void ScTpSubTotalOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    pLbUserDef->Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        for ( size_t i=0; i<nCount; ++i )
            pLbUserDef->InsertEntry( (*pUserLists)[i].GetString() );
    }
}

// Handler:

IMPL_LINK_TYPED( ScTpSubTotalOptions, CheckHdl, Button*, pBox, void )
{
    if ( pBox == pBtnSort )
    {
        if ( pBtnSort->IsChecked() )
        {
            pFlSort->Enable();
            pBtnFormats->Enable();
            pBtnUserDef->Enable();
            pBtnAscending->Enable();
            pBtnDescending->Enable();

            if ( pBtnUserDef->IsChecked() )
                pLbUserDef->Enable();
        }
        else
        {
            pFlSort->Disable();
            pBtnFormats->Disable();
            pBtnUserDef->Disable();
            pBtnAscending->Disable();
            pBtnDescending->Disable();
            pLbUserDef->Disable();
        }
    }
    else if ( pBox == pBtnUserDef )
    {
        if ( pBtnUserDef->IsChecked() )
        {
            pLbUserDef->Enable();
            pLbUserDef->GrabFocus();
        }
        else
            pLbUserDef->Disable();
    }
}

ScTpSubTotalGroup1::~ScTpSubTotalGroup1()
{
}

ScTpSubTotalGroup2::~ScTpSubTotalGroup2()
{
}

ScTpSubTotalGroup3::~ScTpSubTotalGroup3()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
