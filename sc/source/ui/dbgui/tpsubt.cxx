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
        sal_uInt16  i=0;
        rtl::OUString  aFieldName;

        aLbGroup.Clear();
        aLbColumns.Clear();
        aLbGroup.InsertEntry( aStrNone, 0 );

        i=0;
        for ( col=nFirstCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
        {
            pDoc->GetString( col, nFirstRow, nTab, aFieldName );
            if ( aFieldName.isEmpty() )
            {
               rtl::OUStringBuffer aBuf;
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
        SvLBoxEntry* pEntry = aLbColumns.GetHdlEntry();

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
                              ScResId( RID_SCPAGE_SUBT_OPTIONS ),
                              rArgSet ),
            aFlGroup        ( this, ScResId( FL_GROUP ) ),
            aBtnPagebreak   ( this, ScResId( BTN_PAGEBREAK ) ),
            aBtnCase        ( this, ScResId( BTN_CASE ) ),
            aBtnSort        ( this, ScResId( BTN_SORT ) ),
            aFlSort         ( this, ScResId( FL_SORT ) ),
            aBtnAscending   ( this, ScResId( BTN_ASCENDING ) ),
            aBtnDescending  ( this, ScResId( BTN_DESCENDING ) ),
            aBtnFormats     ( this, ScResId( BTN_FORMATS ) ),
            aBtnUserDef     ( this, ScResId( BTN_USERDEF ) ),
            aLbUserDef      ( this, ScResId( LB_USERDEF ) ),
            pViewData       ( NULL ),
            pDoc            ( NULL ),
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( ((const ScSubTotalItem&)
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() )
{
    Init();
    FreeResource();

    aLbUserDef.SetAccessibleRelationLabeledBy(&aBtnUserDef);
    aLbUserDef.SetAccessibleName(aBtnUserDef.GetText());
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

    aBtnSort.SetClickHdl    ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );
    aBtnUserDef.SetClickHdl ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );

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
    aBtnPagebreak.Check ( rSubTotalData.bPagebreak );
    aBtnCase.Check      ( rSubTotalData.bCaseSens );
    aBtnFormats.Check   ( rSubTotalData.bIncludePattern );
    aBtnSort.Check      ( rSubTotalData.bDoSort );
    aBtnAscending.Check ( rSubTotalData.bAscending );
    aBtnDescending.Check( !rSubTotalData.bAscending );

    if ( rSubTotalData.bUserDef )
    {
        aBtnUserDef.Check( true );
        aLbUserDef.Enable();
        aLbUserDef.SelectEntryPos( rSubTotalData.nUserIndex );
    }
    else
    {
        aBtnUserDef.Check( false );
        aLbUserDef.Disable();
        aLbUserDef.SelectEntryPos( 0 );
    }

    CheckHdl( &aBtnSort );
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

    theSubTotalData.bPagebreak      = aBtnPagebreak.IsChecked();
    theSubTotalData.bReplace        = true;
    theSubTotalData.bCaseSens       = aBtnCase.IsChecked();
    theSubTotalData.bIncludePattern = aBtnFormats.IsChecked();
    theSubTotalData.bDoSort         = aBtnSort.IsChecked();
    theSubTotalData.bAscending      = aBtnAscending.IsChecked();
    theSubTotalData.bUserDef        = aBtnUserDef.IsChecked();
    theSubTotalData.nUserIndex      = (aBtnUserDef.IsChecked())
                                    ? aLbUserDef.GetSelectEntryPos()
                                    : 0;

    rArgSet.Put( ScSubTotalItem( nWhichSubTotals, &theSubTotalData ) );

    return true;
}

// -----------------------------------------------------------------------

void ScTpSubTotalOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    aLbUserDef.Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        for ( size_t i=0; i<nCount; ++i )
            aLbUserDef.InsertEntry( (*pUserLists)[i]->GetString() );
    }
}

// -----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTpSubTotalOptions, CheckHdl, CheckBox *, pBox )
{
    if ( pBox == &aBtnSort )
    {
        if ( aBtnSort.IsChecked() )
        {
            aFlSort         .Enable();
            aBtnFormats     .Enable();
            aBtnUserDef     .Enable();
            aBtnAscending   .Enable();
            aBtnDescending  .Enable();

            if ( aBtnUserDef.IsChecked() )
                aLbUserDef.Enable();
        }
        else
        {
            aFlSort         .Disable();
            aBtnFormats     .Disable();
            aBtnUserDef     .Disable();
            aBtnAscending   .Disable();
            aBtnDescending  .Disable();
            aLbUserDef      .Disable();
        }
    }
    else if ( pBox == &aBtnUserDef )
    {
        if ( aBtnUserDef.IsChecked() )
        {
            aLbUserDef.Enable();
            aLbUserDef.GrabFocus();
        }
        else
            aLbUserDef.Disable();
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
