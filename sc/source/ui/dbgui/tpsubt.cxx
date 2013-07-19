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
#include "sc.hrc"       // -> Slot IDs

#include "subtdlg.hxx"
#include "subtdlg.hrc"
#include "tpsubt.hxx"

//========================================================================
// Zwischenergebnisgruppen-Tabpage:

ScTpSubTotalGroup::ScTpSubTotalGroup( Window* pParent, sal_uInt16 nResId,
                                      const SfxItemSet& rArgSet )
        :   SfxTabPage      ( pParent,
                              ScResId( nResId ),
                              rArgSet ),
            aFtGroup        ( this, ScResId( FT_GROUP ) ),
            aLbGroup        ( this, ScResId( LB_GROUP ) ),
            aFtColumns      ( this, ScResId( FT_COLUMNS ) ),
            aLbColumns      ( this, ScResId( WND_COLUMNS ) ),
            aFtFunctions    ( this, ScResId( FT_FUNCTIONS ) ),
            aLbFunctions    ( this, ScResId( LB_FUNCTIONS ) ),
            aStrNone        ( SC_RESSTR( SCSTR_NONE ) ),
            aStrColumn      ( SC_RESSTR( SCSTR_COLUMN ) ),
            pViewData       ( NULL ),
            pDoc            ( NULL ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( ((const ScSubTotalItem&)
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() ),
            nFieldCount     ( 0 )
{
    // Font is correctly initialized by SvTreeListBox ctor
    aLbColumns.SetSelectionMode( SINGLE_SELECTION );
    aLbColumns.SetDragDropMode( SV_DRAGDROP_NONE );
    aLbColumns.SetSpaceBetweenEntries( 0 );
    aLbColumns.Show();

    Init ();
    FreeResource();
}

// -----------------------------------------------------------------------

ScTpSubTotalGroup::~ScTpSubTotalGroup()
{
    sal_uInt16  nCount = (sal_uInt16)aLbColumns.GetEntryCount();

    if ( nCount > 0 )
    {
        sal_uInt16* pData = NULL;

        for ( sal_uInt16 i=0; i<nCount; i++ )
        {
            pData = (sal_uInt16*)(aLbColumns.GetEntryData( i ));
            OSL_ENSURE( pData, "EntryData not found" );

            delete pData;
        }
    }
}

// -----------------------------------------------------------------------

void ScTpSubTotalGroup::Init()
{
    const ScSubTotalItem& rSubTotalItem = (const ScSubTotalItem&)
                                          GetItemSet().Get( nWhichSubTotals );

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found :-(" );

    aLbGroup.SetSelectHdl       ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );
    aLbColumns.SetSelectHdl     ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );
    aLbColumns.SetCheckButtonHdl    ( LINK( this, ScTpSubTotalGroup, CheckHdl ) );
    aLbFunctions.SetSelectHdl   ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );

    nFieldArr[0] = 0;
    FillListBoxes();
}

// -----------------------------------------------------------------------

bool ScTpSubTotalGroup::DoReset( sal_uInt16             nGroupNo,
                                 const SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );

    if ( (nGroupNo > 3) || (nGroupNo == 0) )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    //----------------------------------------------------------

    // first we have to clear the listboxes...
    for ( sal_uInt16 nLbEntry = 0; nLbEntry < aLbColumns.GetEntryCount(); ++nLbEntry )
    {
        aLbColumns.CheckEntryPos( nLbEntry, false );
        *((sal_uInt16*)aLbColumns.GetEntryData( nLbEntry )) = 0;
    }
    aLbFunctions.SelectEntryPos( 0 );

    ScSubTotalParam theSubTotalData( ((const ScSubTotalItem&)
                                      rArgSet.Get( nWhichSubTotals )).
                                            GetSubTotalData() );

    if ( theSubTotalData.bGroupActive[nGroupIdx] )
    {
        SCCOL           nField      = theSubTotalData.nField[nGroupIdx];
        SCCOL           nSubTotals  = theSubTotalData.nSubTotals[nGroupIdx];
        SCCOL*          pSubTotals  = theSubTotalData.pSubTotals[nGroupIdx];
        ScSubTotalFunc* pFunctions  = theSubTotalData.pFunctions[nGroupIdx];

        aLbGroup.SelectEntryPos( GetFieldSelPos( nField )+1 );

        sal_uInt16 nFirstChecked = 0;
        for ( sal_uInt16 i=0; i<nSubTotals; i++ )
        {
            sal_uInt16  nCheckPos = GetFieldSelPos( pSubTotals[i] );
            sal_uInt16* pFunction = (sal_uInt16*)aLbColumns.GetEntryData( nCheckPos );

            aLbColumns.CheckEntryPos( nCheckPos );
            *pFunction = FuncToLbPos( pFunctions[i] );

            if (i == 0 || (i > 0 && nCheckPos < nFirstChecked))
                nFirstChecked = nCheckPos;
        }
        // Select the first checked field from the top.
        aLbColumns.SelectEntryPos(nFirstChecked);
    }
    else
    {
        aLbGroup.SelectEntryPos( (nGroupNo == 1) ? 1 : 0 );
        aLbColumns.SelectEntryPos( 0 );
        aLbFunctions.SelectEntryPos( 0 );
    }

    return true;
}

// -----------------------------------------------------------------------

bool ScTpSubTotalGroup::DoFillItemSet( sal_uInt16       nGroupNo,
                                       SfxItemSet&  rArgSet  )
{
    sal_uInt16 nGroupIdx = 0;

    OSL_ENSURE( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );
    OSL_ENSURE(    (aLbGroup.GetEntryCount() > 0)
                && (aLbColumns.GetEntryCount() > 0)
                && (aLbFunctions.GetEntryCount() > 0),
                "Non-initialized Lists" );


    if (  (nGroupNo > 3) || (nGroupNo == 0)
        || (aLbGroup.GetEntryCount() == 0)
        || (aLbColumns.GetEntryCount() == 0)
        || (aLbFunctions.GetEntryCount() == 0)
       )
        return false;
    else
        nGroupIdx = nGroupNo-1;

    //----------------------------------------------------------

    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, true, &pItem ) == SFX_ITEM_SET )
            theSubTotalData = ((const ScSubTotalItem*)pItem)->GetSubTotalData();
    }

    ScSubTotalFunc* pFunctions  = NULL;
    SCCOL*          pSubTotals  = NULL;
    sal_uInt16          nGroup      = aLbGroup.GetSelectEntryPos();
    sal_uInt16          nEntryCount = (sal_uInt16)aLbColumns.GetEntryCount();
    sal_uInt16          nCheckCount = aLbColumns.GetCheckedEntryCount();

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

        pSubTotals = new SCCOL          [nCheckCount];
        pFunctions = new ScSubTotalFunc [nCheckCount];

        for ( sal_uInt16 i=0, nCheck=0; i<nEntryCount; i++ )
        {
            if ( aLbColumns.IsChecked( i ) )
            {
                OSL_ENSURE( nCheck <= nCheckCount,
                            "Range error :-(" );
                nFunction = *((sal_uInt16*)aLbColumns.GetEntryData( i ));
                pSubTotals[nCheck] = nFieldArr[i];
                pFunctions[nCheck] = LbPosToFunc( nFunction );
                nCheck++;
            }
        }
        theSubTotalData.SetSubTotals( nGroupNo,      // Gruppen-Nr.
                                      pSubTotals,
                                      pFunctions,
                                      nCheckCount ); // Anzahl der Array-Elemente

    }

    rArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, &theSubTotalData ) );

    if ( pSubTotals ) delete [] pSubTotals;
    if ( pFunctions ) delete [] pFunctions;

    return true;
}

// -----------------------------------------------------------------------

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

        aLbGroup.Clear();
        aLbColumns.Clear();
        aLbGroup.InsertEntry( aStrNone, 0 );

        sal_uInt16 i=0;
        for ( col=nFirstCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if ( aFieldName.isEmpty() )
            {
                OUStringBuffer aBuf;
                aBuf.append(aStrColumn);
                aFieldName = aBuf.makeStringAndClear().replaceAll("%1", ScColToAlpha( col ));
            }
            nFieldArr[i] = col;
            aLbGroup.InsertEntry( aFieldName, i+1 );
            aLbColumns.InsertEntry( aFieldName, i );
            aLbColumns.SetEntryData( i, new sal_uInt16(0) );
            i++;
        }
        // Nachtraegliche "Konstanteninitialisierung":
        (sal_uInt16&)nFieldCount = i;
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScTpSubTotalGroup, SelectHdl, ListBox *, pLb )
{
    if (   (aLbColumns.GetEntryCount() > 0)
        && (aLbColumns.GetSelectionCount() > 0) )
    {
        sal_uInt16      nFunction   = aLbFunctions.GetSelectEntryPos();
        sal_uInt16      nColumn     = aLbColumns.GetSelectEntryPos();
        sal_uInt16*     pFunction   = (sal_uInt16*)aLbColumns.GetEntryData( nColumn );

        OSL_ENSURE( pFunction, "EntryData nicht gefunden!" );
        if ( !pFunction )
            return 0;

        if ( ((SvxCheckListBox*)pLb) == &aLbColumns )
        {
            aLbFunctions.SelectEntryPos( *pFunction );
        }
        else if ( pLb == &aLbFunctions )
        {
            *pFunction = nFunction;
            aLbColumns.CheckEntryPos( nColumn, true );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpSubTotalGroup, CheckHdl, ListBox *, pLb )
{
    if ( ((SvxCheckListBox*)pLb) == &aLbColumns )
    {
        SvTreeListEntry* pEntry = aLbColumns.GetHdlEntry();

        if ( pEntry )
        {
            aLbColumns.SelectEntryPos( (sal_uInt16)aLbColumns.GetModel()->GetAbsPos( pEntry ) );
            SelectHdl( pLb );
        }
    }
    return 0;
}

//========================================================================
// Abgeleitete Gruppen-TabPages:

SfxTabPage* ScTpSubTotalGroup1::Create( Window*         pParent,
                                                 const SfxItemSet&  rArgSet )
    { return ( new ScTpSubTotalGroup1( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------

SfxTabPage* ScTpSubTotalGroup2::Create( Window*          pParent,
                                       const SfxItemSet&    rArgSet )
    { return ( new ScTpSubTotalGroup2( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------

SfxTabPage* ScTpSubTotalGroup3::Create( Window*          pParent,
                                       const SfxItemSet&    rArgSet )
    { return ( new ScTpSubTotalGroup3( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------

ScTpSubTotalGroup1::ScTpSubTotalGroup1( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP1, rArgSet )
{}

ScTpSubTotalGroup2::ScTpSubTotalGroup2( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP2, rArgSet )
{}

ScTpSubTotalGroup3::ScTpSubTotalGroup3( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP3, rArgSet )
{}

// -----------------------------------------------------------------------


#define RESET(i) (ScTpSubTotalGroup::DoReset( (i), rArgSet ))
void ScTpSubTotalGroup1::Reset( const SfxItemSet& rArgSet ) { RESET(1); }
void ScTpSubTotalGroup2::Reset( const SfxItemSet& rArgSet ) { RESET(2); }
void ScTpSubTotalGroup3::Reset( const SfxItemSet& rArgSet ) { RESET(3); }
#undef RESET

// -----------------------------------------------------------------------

#define FILLSET(i) (ScTpSubTotalGroup::DoFillItemSet( (i), rArgSet ))
sal_Bool ScTpSubTotalGroup1::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(1); }
sal_Bool ScTpSubTotalGroup2::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(2); }
sal_Bool ScTpSubTotalGroup3::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(3); }
#undef FILL

//========================================================================
// Optionen-Tabpage:

ScTpSubTotalOptions::ScTpSubTotalOptions( Window*               pParent,
                                          const SfxItemSet&     rArgSet )

        :   SfxTabPage      ( pParent,
                              "SubTotalOptionsPage", "modules/scalc/ui/subtotaloptionspage.ui" ,
                              rArgSet ),
            pViewData       ( NULL ),
            pDoc            ( NULL ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( ((const ScSubTotalItem&)
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

    pLbUserDef->SetAccessibleRelationLabeledBy(pBtnUserDef);
    pLbUserDef->SetAccessibleName(pBtnUserDef->GetText());
}

// -----------------------------------------------------------------------

ScTpSubTotalOptions::~ScTpSubTotalOptions()
{
}

// -----------------------------------------------------------------------

void ScTpSubTotalOptions::Init()
{
    const ScSubTotalItem& rSubTotalItem = (const ScSubTotalItem&)
                                          GetItemSet().Get( nWhichSubTotals );

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    pBtnSort->SetClickHdl    ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );
    pBtnUserDef->SetClickHdl ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );

    FillUserSortListBox();
}

// -----------------------------------------------------------------------

SfxTabPage* ScTpSubTotalOptions::Create( Window*                 pParent,
                                          const SfxItemSet&     rArgSet )
{
    return ( new ScTpSubTotalOptions( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTpSubTotalOptions::Reset( const SfxItemSet& /* rArgSet */ )
{
    pBtnPagebreak->Check ( rSubTotalData.bPagebreak );
    pBtnCase->Check      ( rSubTotalData.bCaseSens );
    pBtnFormats->Check   ( rSubTotalData.bIncludePattern );
    pBtnSort->Check      ( rSubTotalData.bDoSort );
    pBtnAscending->Check ( rSubTotalData.bAscending );
    pBtnDescending->Check( !rSubTotalData.bAscending );

    if ( rSubTotalData.bUserDef )
    {
        pBtnUserDef->Check( true );
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

// -----------------------------------------------------------------------

sal_Bool ScTpSubTotalOptions::FillItemSet( SfxItemSet& rArgSet )
{
    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, true, &pItem ) == SFX_ITEM_SET )
            theSubTotalData = ((const ScSubTotalItem*)pItem)->GetSubTotalData();
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

    rArgSet.Put( ScSubTotalItem( nWhichSubTotals, &theSubTotalData ) );

    return true;
}

// -----------------------------------------------------------------------

void ScTpSubTotalOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    pLbUserDef->Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        for ( size_t i=0; i<nCount; ++i )
            pLbUserDef->InsertEntry( (*pUserLists)[i]->GetString() );
    }
}

// -----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTpSubTotalOptions, CheckHdl, CheckBox *, pBox )
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

    return 0;
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
