/*************************************************************************
 *
 *  $RCSfile: tpsubt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <vcl/system.hxx>
#include <segmentc.hxx>
#endif

//#include <checklbx.hxx>

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

// STATIC DATA -----------------------------------------------------------

static USHORT pSubTotalsRanges[] =
{
    SID_SUBTOTALS,
    SID_SUBTOTALS,
    0
};

SEG_EOFGLOBALS()


//========================================================================
// Zwischenergebnisgruppen-Tabpage:
#pragma SEG_FUNCDEF(tpsubt_01)

ScTpSubTotalGroup::ScTpSubTotalGroup( Window* pParent, USHORT nResId,
                                      const SfxItemSet& rArgSet )
        :   SfxTabPage      ( pParent,
                              ScResId( nResId ),
                              rArgSet ),
            //
            aLbGroup        ( this, ScResId( LB_GROUP ) ),
            aLbColumns      ( this, ScResId( WND_COLUMNS ) ),
            aLbFunctions    ( this, ScResId( LB_FUNCTIONS ) ),
            aFtGroup        ( this, ScResId( FT_GROUP ) ),
            aFtColumns      ( this, ScResId( FT_COLUMNS ) ),
            aFtFunctions    ( this, ScResId( FT_FUNCTIONS ) ),
            aStrNone        ( ScResId( SCSTR_NONE ) ),
            aStrColumn      ( ScResId( SCSTR_COLUMN ) ),
            //
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( ((const ScSubTotalItem&)
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() ),
            nFieldCount     ( 0 ),
            pViewData       ( NULL ),
            pDoc            ( NULL )
{
    aLbColumns.SetFont( System::GetStandardFont( STDFONT_APP ) );
    aLbColumns.SetSelectionMode( SINGLE_SELECTION );
    aLbColumns.SetDragDropMode( SV_DRAGDROP_NONE );
    aLbColumns.SetSpaceBetweenEntries( 0 );
    aLbColumns.Show();

    Init ();
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_02)

__EXPORT ScTpSubTotalGroup::~ScTpSubTotalGroup()
{
    USHORT  nCount = (USHORT)aLbColumns.GetEntryCount();

    if ( nCount > 0 )
    {
        USHORT* pData = NULL;

        for ( USHORT i=0; i<nCount; i++ )
        {
            pData = (USHORT*)(aLbColumns.GetEntryData( i ));
            DBG_ASSERT( pData, "EntryData not found" );

            delete pData;
        }
    }
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_03)

void ScTpSubTotalGroup::Init()
{
    const ScSubTotalItem& rSubTotalItem = (const ScSubTotalItem&)
                                          GetItemSet().Get( nWhichSubTotals );

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    DBG_ASSERT( pViewData && pDoc, "ViewData or Document not found :-(" );

    aLbGroup.SetSelectHdl       ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );
    aLbColumns.SetSelectHdl     ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );
    aLbColumns.SetCheckButtonHdl    ( LINK( this, ScTpSubTotalGroup, CheckHdl ) );
    aLbFunctions.SetSelectHdl   ( LINK( this, ScTpSubTotalGroup, SelectHdl ) );

    nFieldArr[0] = 0;
    FillListBoxes();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_1d)

USHORT* __EXPORT ScTpSubTotalGroup::GetRanges()
{
    return pSubTotalsRanges;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_04)

BOOL ScTpSubTotalGroup::DoReset( USHORT             nGroupNo,
                                 const SfxItemSet&  rArgSet  )
{
    USHORT nGroupIdx = 0;

    DBG_ASSERT( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );

    if ( (nGroupNo > 3) || (nGroupNo == 0) )
        return FALSE;
    else
        nGroupIdx = nGroupNo-1;

    //----------------------------------------------------------

    ScSubTotalParam theSubTotalData( ((const ScSubTotalItem&)
                                      rArgSet.Get( nWhichSubTotals )).
                                            GetSubTotalData() );

    if ( theSubTotalData.bGroupActive[nGroupIdx] )
    {
        USHORT          nField      = theSubTotalData.nField[nGroupIdx];
        USHORT          nSubTotals  = theSubTotalData.nSubTotals[nGroupIdx];
        USHORT*         pSubTotals  = theSubTotalData.pSubTotals[nGroupIdx];
        ScSubTotalFunc* pFunctions  = theSubTotalData.pFunctions[nGroupIdx];

        aLbGroup.SelectEntryPos( GetFieldSelPos( nField )+1 );

        for ( USHORT i=0; i<nSubTotals; i++ )
        {
            USHORT  nCheckPos = GetFieldSelPos( pSubTotals[i] );
            USHORT* pFunction = (USHORT*)aLbColumns.GetEntryData( nCheckPos );

            aLbColumns.CheckEntryPos( nCheckPos );
            *pFunction = FuncToLbPos( pFunctions[i] );
            aLbFunctions.SelectEntryPos( *pFunction );
        }
    }
    else
    {
        aLbGroup.SelectEntryPos( (nGroupNo == 1) ? 1 : 0 );
        aLbColumns.SelectEntryPos( 0 );
        aLbFunctions.SelectEntryPos( 0 );
    }

    return TRUE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_05)

BOOL ScTpSubTotalGroup::DoFillItemSet( USHORT       nGroupNo,
                                       SfxItemSet&  rArgSet  )
{
    USHORT nGroupIdx = 0;

    DBG_ASSERT( (nGroupNo<=3) && (nGroupNo>0), "Invalid group" );
    DBG_ASSERT(    (aLbGroup.GetEntryCount() > 0)
                && (aLbColumns.GetEntryCount() > 0)
                && (aLbFunctions.GetEntryCount() > 0),
                "Non-initialized Lists" );


    if (  (nGroupNo > 3) || (nGroupNo == 0)
        || (aLbGroup.GetEntryCount() == 0)
        || (aLbColumns.GetEntryCount() == 0)
        || (aLbFunctions.GetEntryCount() == 0)
       )
        return FALSE;
    else
        nGroupIdx = nGroupNo-1;

    //----------------------------------------------------------

    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, TRUE, &pItem ) == SFX_ITEM_SET )
            theSubTotalData = ((const ScSubTotalItem*)pItem)->GetSubTotalData();
    }

    ScSubTotalFunc* pFunctions  = NULL;
    USHORT*         pSubTotals  = NULL;
    USHORT          nGroup      = aLbGroup.GetSelectEntryPos();
    USHORT          nEntryCount = (USHORT)aLbColumns.GetEntryCount();
    USHORT          nCheckCount = aLbColumns.GetCheckedEntryCount();

    theSubTotalData.nCol1                   = rSubTotalData.nCol1;
    theSubTotalData.nRow1                   = rSubTotalData.nRow1;
    theSubTotalData.nCol2                   = rSubTotalData.nCol2;
    theSubTotalData.nRow2                   = rSubTotalData.nRow2;
    theSubTotalData.bGroupActive[nGroupIdx] = (nGroup != 0);
    theSubTotalData.nField[nGroupIdx]       = (nGroup != 0)
                                                ? nFieldArr[nGroup-1]
                                                : 0;

    if ( nEntryCount>0 && nCheckCount>0 && nGroup!=0 )
    {
        USHORT nFunction    = 0;

        pSubTotals = new USHORT         [nCheckCount];
        pFunctions = new ScSubTotalFunc [nCheckCount];

        for ( USHORT i=0, nCheck=0; i<nEntryCount; i++ )
        {
            if ( aLbColumns.IsChecked( i ) )
            {
                DBG_ASSERT( nCheck <= nCheckCount,
                            "Range error :-(" );
                nFunction = *((USHORT*)aLbColumns.GetEntryData( i ));
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

    if ( pSubTotals ) delete pSubTotals;
    if ( pFunctions ) delete pFunctions;

    return TRUE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_06)

void ScTpSubTotalGroup::FillListBoxes()
{
    DBG_ASSERT( pViewData && pDoc, "ViewData or Document not found :-/" );

    if ( pViewData && pDoc )
    {
        USHORT  nFirstCol   = rSubTotalData.nCol1;
        USHORT  nFirstRow   = rSubTotalData.nRow1;
        USHORT  nTab        = pViewData->GetTabNo();
        USHORT  nMaxCol     = rSubTotalData.nCol2;
        USHORT  col;
        USHORT  i=0;
        String  aFieldName;

        aLbGroup.Clear();
        aLbColumns.Clear();
        aLbGroup.InsertEntry( aStrNone, 0 );

        i=0;
        for ( col=nFirstCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
        {
            pDoc->GetString( col, nFirstRow, nTab, aFieldName );
            if ( aFieldName.Len() == 0 )
            {
                aFieldName  = aStrColumn;
                aFieldName += ' ';
                if ( col < 26 )
                    aFieldName += (sal_Unicode)( 'A' + col );
                else
                {
                    aFieldName += (sal_Unicode)( 'A' + ( col / 26 ) - 1 );
                    aFieldName += (sal_Unicode)( 'A' + ( col % 26 ) );
                }
            }
            nFieldArr[i] = col;
            aLbGroup.InsertEntry( aFieldName, i+1 );
            aLbColumns.InsertEntry( aFieldName, i );
            aLbColumns.SetEntryData( i, new USHORT(0) );
            i++;
        }
        // Nachtraegliche "Konstanteninitialisierung":
        (USHORT&)nFieldCount = i;
    }
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_18)

USHORT ScTpSubTotalGroup::GetFieldSelPos( USHORT nField )
{
    USHORT  nFieldPos   = 0;
    BOOL    bFound      = FALSE;

    for ( USHORT n=0; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = TRUE;
        }
    }

    return nFieldPos;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_19)

ScSubTotalFunc ScTpSubTotalGroup::LbPosToFunc( USHORT nPos )
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
            DBG_ERROR( "ScTpSubTotalGroup::LbPosToFunc" );
            return SUBTOTAL_FUNC_NONE;
    }
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_1a)

USHORT ScTpSubTotalGroup::FuncToLbPos( ScSubTotalFunc eFunc )
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
            DBG_ERROR( "ScTpSubTotalGroup::FuncToLbPos" );
            return 0;
    }
}

// -----------------------------------------------------------------------
// Handler:
//---------
#pragma SEG_FUNCDEF(tpsubt_07)

IMPL_LINK( ScTpSubTotalGroup, SelectHdl, ListBox *, pLb )
{
    if (   (aLbColumns.GetEntryCount() > 0)
        && (aLbColumns.GetSelectionCount() > 0) )
    {
        USHORT      nFunction   = aLbFunctions.GetSelectEntryPos();
        USHORT      nColumn     = aLbColumns.GetSelectEntryPos();
        USHORT*     pFunction   = (USHORT*)aLbColumns.GetEntryData( nColumn );

        DBG_ASSERT( pFunction, "EntryData nicht gefunden!" );
        if ( !pFunction )
            return 0;

        if ( ((SvxCheckListBox*)pLb) == &aLbColumns )
        {
            aLbFunctions.SelectEntryPos( *pFunction );
        }
        else if ( pLb == &aLbFunctions )
        {
            *pFunction = nFunction;
//          aLbColumns.CheckEntryPos( nColumn, (nFunction != 0) );//XXX
            aLbColumns.CheckEntryPos( nColumn, TRUE );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_20)

IMPL_LINK( ScTpSubTotalGroup, CheckHdl, ListBox *, pLb )
{
    if ( ((SvxCheckListBox*)pLb) == &aLbColumns )
    {
        SvLBoxEntry* pEntry = aLbColumns.GetHdlEntry();

        if ( pEntry )
        {
            aLbColumns.SelectEntryPos( (USHORT)aLbColumns.GetModel()->GetAbsPos( pEntry ) );
            SelectHdl( pLb );
        }
    }
    return 0;
}

//========================================================================
// Abgeleitete Gruppen-TabPages:
#pragma SEG_FUNCDEF(tpsubt_08)

SfxTabPage* __EXPORT ScTpSubTotalGroup1::Create( Window*            pParent,
                                                 const SfxItemSet&  rArgSet )
    { return ( new ScTpSubTotalGroup1( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_09)

SfxTabPage* __EXPORT ScTpSubTotalGroup2::Create( Window*             pParent,
                                       const SfxItemSet&    rArgSet )
    { return ( new ScTpSubTotalGroup2( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_0a)

SfxTabPage* __EXPORT ScTpSubTotalGroup3::Create( Window*             pParent,
                                       const SfxItemSet&    rArgSet )
    { return ( new ScTpSubTotalGroup3( pParent, rArgSet ) ); }

// -----------------------------------------------------------------------

#pragma SEG_FUNCDEF(tpsubt_21)

ScTpSubTotalGroup1::ScTpSubTotalGroup1( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP1, rArgSet )
{}

#pragma SEG_FUNCDEF(tpsubt_22)

ScTpSubTotalGroup2::ScTpSubTotalGroup2( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP2, rArgSet )
{}

#pragma SEG_FUNCDEF(tpsubt_23)

ScTpSubTotalGroup3::ScTpSubTotalGroup3( Window* pParent, const SfxItemSet& rArgSet ) :
    ScTpSubTotalGroup( pParent, RID_SCPAGE_SUBT_GROUP3, rArgSet )
{}

// -----------------------------------------------------------------------


#define RESET(i) (ScTpSubTotalGroup::DoReset( (i), rArgSet ))

#pragma SEG_FUNCDEF(tpsubt_0b)

void __EXPORT ScTpSubTotalGroup1::Reset( const SfxItemSet& rArgSet ) { RESET(1); }

#pragma SEG_FUNCDEF(tpsubt_1b)

void __EXPORT ScTpSubTotalGroup2::Reset( const SfxItemSet& rArgSet ) { RESET(2); }

#pragma SEG_FUNCDEF(tpsubt_1c)

void __EXPORT ScTpSubTotalGroup3::Reset( const SfxItemSet& rArgSet ) { RESET(3); }

#undef RESET

// -----------------------------------------------------------------------

#define FILLSET(i) (ScTpSubTotalGroup::DoFillItemSet( (i), rArgSet ))

#pragma SEG_FUNCDEF(tpsubt_0c)

BOOL __EXPORT ScTpSubTotalGroup1::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(1); }

#pragma SEG_FUNCDEF(tpsubt_0d)

BOOL __EXPORT ScTpSubTotalGroup2::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(2); }

#pragma SEG_FUNCDEF(tpsubt_0e)

BOOL __EXPORT ScTpSubTotalGroup3::FillItemSet( SfxItemSet& rArgSet ) { return FILLSET(3); }

#undef FILL

//========================================================================
// Optionen-Tabpage:
#pragma SEG_FUNCDEF(tpsubt_0f)

ScTpSubTotalOptions::ScTpSubTotalOptions( Window*               pParent,
                                          const SfxItemSet&     rArgSet )

        :   SfxTabPage      ( pParent,
                              ScResId( RID_SCPAGE_SUBT_OPTIONS ),
                              rArgSet ),
            //
            aBtnPagebreak   ( this, ScResId( BTN_PAGEBREAK ) ),
            aBtnCase        ( this, ScResId( BTN_CASE ) ),
            aBtnSort        ( this, ScResId( BTN_SORT ) ),
            aBtnFormats     ( this, ScResId( BTN_FORMATS ) ),
            aBtnUserDef     ( this, ScResId( BTN_USERDEF ) ),
            aLbUserDef      ( this, ScResId( LB_USERDEF ) ),
            aBtnAscending   ( this, ScResId( BTN_ASCENDING ) ),
            aBtnDescending  ( this, ScResId( BTN_DESCENDING ) ),
            aGbOptions      ( this, ScResId( GB_OPTIONS ) ),
            //
            nWhichSubTotals ( rArgSet.GetPool()->GetWhich( SID_SUBTOTALS ) ),
            rSubTotalData   ( ((const ScSubTotalItem&)
                              rArgSet.Get( nWhichSubTotals )).
                                GetSubTotalData() ),
            pViewData       ( NULL ),
            pDoc            ( NULL )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_10)

__EXPORT ScTpSubTotalOptions::~ScTpSubTotalOptions()
{
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_11)

void ScTpSubTotalOptions::Init()
{
    const ScSubTotalItem& rSubTotalItem = (const ScSubTotalItem&)
                                          GetItemSet().Get( nWhichSubTotals );

    pViewData   = rSubTotalItem.GetViewData();
    pDoc        = ( pViewData ) ? pViewData->GetDocument() : NULL;

    DBG_ASSERT( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    aBtnSort.SetClickHdl    ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );
    aBtnUserDef.SetClickHdl ( LINK( this, ScTpSubTotalOptions, CheckHdl ) );

    FillUserSortListBox();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_12)

SfxTabPage* __EXPORT ScTpSubTotalOptions::Create( Window*                pParent,
                                          const SfxItemSet&     rArgSet )
{
    return ( new ScTpSubTotalOptions( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_13)

void __EXPORT ScTpSubTotalOptions::Reset( const SfxItemSet& rArgSet )
{
    aBtnPagebreak.Check ( rSubTotalData.bPagebreak );
    aBtnCase.Check      ( rSubTotalData.bCaseSens );
    aBtnFormats.Check   ( rSubTotalData.bIncludePattern );
    aBtnSort.Check      ( rSubTotalData.bDoSort );
    aBtnAscending.Check ( rSubTotalData.bAscending );
    aBtnDescending.Check( !rSubTotalData.bAscending );

    if ( rSubTotalData.bUserDef )
    {
        aBtnUserDef.Check( TRUE );
        aLbUserDef.Enable();
        aLbUserDef.SelectEntryPos( rSubTotalData.nUserIndex );
    }
    else
    {
        aBtnUserDef.Check( FALSE );
        aLbUserDef.Disable();
        aLbUserDef.SelectEntryPos( 0 );
    }

    CheckHdl( &aBtnSort );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_14)

BOOL __EXPORT ScTpSubTotalOptions::FillItemSet( SfxItemSet& rArgSet )
{
    ScSubTotalParam theSubTotalData;            // auslesen, wenn schon teilweise gefuellt
    SfxTabDialog* pDlg = GetTabDialog();
    if ( pDlg )
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSubTotals, TRUE, &pItem ) == SFX_ITEM_SET )
            theSubTotalData = ((const ScSubTotalItem*)pItem)->GetSubTotalData();
    }

    theSubTotalData.bPagebreak      = aBtnPagebreak.IsChecked();
    theSubTotalData.bReplace        = TRUE;
    theSubTotalData.bCaseSens       = aBtnCase.IsChecked();
    theSubTotalData.bIncludePattern = aBtnFormats.IsChecked();
    theSubTotalData.bDoSort         = aBtnSort.IsChecked();
    theSubTotalData.bAscending      = aBtnAscending.IsChecked();
    theSubTotalData.bUserDef        = aBtnUserDef.IsChecked();
    theSubTotalData.nUserIndex      = (aBtnUserDef.IsChecked())
                                    ? aLbUserDef.GetSelectEntryPos()
                                    : 0;

    rArgSet.Put( ScSubTotalItem( nWhichSubTotals, &theSubTotalData ) );

    return TRUE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpsubt_15)

void ScTpSubTotalOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    aLbUserDef.Clear();
    if ( pUserLists )
    {
        USHORT nCount = pUserLists->GetCount();
        if ( nCount > 0 )
            for ( USHORT i=0; i<nCount; i++ )
                aLbUserDef.InsertEntry( (*pUserLists)[i]->GetString() );
    }
}

// -----------------------------------------------------------------------
// Handler:
#pragma SEG_FUNCDEF(tpsubt_16)

IMPL_LINK( ScTpSubTotalOptions, CheckHdl, CheckBox *, pBox )
{
    if ( pBox == &aBtnSort )
    {
        if ( aBtnSort.IsChecked() )
        {
            aGbOptions      .Enable();
            aBtnFormats     .Enable();
            aBtnUserDef     .Enable();
            aBtnAscending   .Enable();
            aBtnDescending  .Enable();

            if ( aBtnUserDef.IsChecked() )
                aLbUserDef.Enable();
        }
        else
        {
            aGbOptions      .Disable();
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

#pragma SEG_FUNCDEF(tpsubt_17)

__EXPORT ScTpSubTotalGroup1::~ScTpSubTotalGroup1()
{
}

#pragma SEG_FUNCDEF(tpsubt_1e)

__EXPORT ScTpSubTotalGroup2::~ScTpSubTotalGroup2()
{
}

#pragma SEG_FUNCDEF(tpsubt_1f)

__EXPORT ScTpSubTotalGroup3::~ScTpSubTotalGroup3()
{
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.41  2000/09/17 14:08:59  willem.vandorp
    OpenOffice header added.

    Revision 1.40  2000/08/31 16:38:21  willem.vandorp
    Header and footer replaced

    Revision 1.39  2000/04/14 17:38:03  nn
    unicode changes

    Revision 1.38  2000/02/11 12:24:18  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.37  1999/05/28 16:05:24  NN
    #66447# MAXFIELDS ist fuer Feldanzahl, nicht Spaltennummer


      Rev 1.36   28 May 1999 18:05:24   NN
   #66447# MAXFIELDS ist fuer Feldanzahl, nicht Spaltennummer

      Rev 1.35   17 Dec 1997 19:49:54   ER
   #46215# NULL-Pointer abfangen (aus nem Paint den SelectHandler gerufen?!?)

      Rev 1.34   05 Dec 1997 20:02:36   ANK
   Includes geaendert

      Rev 1.33   05 Feb 1997 21:29:32   NN
   SfxTabPage Umstellung: FillItemSet bekommt leeren Set

      Rev 1.32   07 Nov 1996 19:51:58   NN
   eigene RIDs fuer die unterschiedlichen Pages

      Rev 1.31   29 Oct 1996 14:04:12   NN
   ueberall ScResId statt ResId

      Rev 1.30   23 May 1996 09:00:50   TRI
   GetAbsPos jetzt uber GetModel aufgerufen

      Rev 1.29   29 Jan 1996 15:12:36   MO
   neuer Link

      Rev 1.28   14 Nov 1995 11:30:10   MO
   SvxCheckListBox verwenden

      Rev 1.27   08 Nov 1995 13:06:24   MO
   301-Aenderungen

      Rev 1.26   25 Sep 1995 14:16:48   JN
   Funktion _Keine_ entfernt

      Rev 1.25   13 Sep 1995 12:27:58   MO
   Funktionszuordnung bei einem Feld-Eintrag (BugIId: 18867)

      Rev 1.24   08 Sep 1995 14:42:44   MO
   Replace CheckBox entfernt (BugId: 18821)

      Rev 1.23   04 Sep 1995 12:00:28   MO
   GetStandardFont() statt GetAppFont()

      Rev 1.22   24 Aug 1995 17:38:52   HJS
   DRAGDROP_NONE => SV_DRAGDROP_NONE

      Rev 1.21   24 Jul 1995 14:17:14   MO
   EXPORT

      Rev 1.20   18 Jul 1995 14:08:30   MO
   CheckEntry impliziert SelectEntry an TreeListBox

      Rev 1.19   04 Jul 1995 18:28:46   MO
   __EXPORTs

      Rev 1.18   30 Jun 1995 17:50:10   HJS
   exports fuer create

      Rev 1.17   04 May 1995 08:16:10   TRI
   pApp -> Application::

      Rev 1.16   07 Apr 1995 17:58:40   MO
   Erste Seite Selektiert erste Spaltenueberschrift in Gruppen-ListBox

      Rev 1.15   24 Mar 1995 13:29:02   TRI
   Segmentierung

      Rev 1.14   07 Mar 1995 16:46:54   MO
   * Enanble/Disable-Fehler auf der Optionen-Seite beseitigt

      Rev 1.13   21 Feb 1995 03:31:08   NN
   Absturz ausgebaut

      Rev 1.12   05 Feb 1995 12:17:00   MO
   * Reset() aus den Ctord entfernt

      Rev 1.11   31 Jan 1995 13:40:36   TRI
   __EXPORT vor virtuelle Funktion eingebaut

      Rev 1.10   31 Jan 1995 11:56:34   MO
   * User-Listen einlesen/auswerten

      Rev 1.9   30 Jan 1995 16:40:50   MO
   Funktions-enum-Zuordnung korrigiert

      Rev 1.8   27 Jan 1995 16:16:08   MO
   * Umstellung auf Slot-IDs
   * GetRanges()-Methode

      Rev 1.7   26 Jan 1995 14:09:48   MO
   * Auswertung des Param-structs in den Gruppen-Pages (eigene Reset()-Metoden)

      Rev 1.6   25 Jan 1995 13:49:32   MO
   Segmentierungspragmas eingefuegt

      Rev 1.5   25 Jan 1995 13:42:02   MO
   Umbenennung tp_* tp*

      Rev 1.4   25 Jan 1995 13:28:26   MO
   Check in der TreeListBox, wenn Funktion selektiert wird
   (funktioniert noch nicht so richtig (TreeListBox?))

      Rev 1.3   22 Jan 1995 19:38:44   NN
   Position des DB-Bereichs im Output-Item initialisieren

      Rev 1.2   22 Jan 1995 15:10:14   SC
   2.37 Anpassung

      Rev 1.1   20 Jan 1995 18:34:02   MO
   Gruppen-TabPages 2 und 3

      Rev 1.0   19 Jan 1995 11:42:20   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


