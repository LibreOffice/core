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

//----------------------------------------------------------------------------

#include "pvlaydlg.hxx"
#include "dbdocfun.hxx"
#include "dpuiglobal.hxx"

#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

#include "uiitems.hxx"
#include "rangeutl.hxx"
#include "document.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "pivot.hrc"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "scmod.hxx"

#include "sc.hrc"
#include "scabstdlg.hxx"

using namespace com::sun::star;
using ::rtl::OUString;
using ::std::vector;
using ::std::for_each;

//----------------------------------------------------------------------------

#define FSTR(index) aFuncNameArr[index-1]
#define STD_FORMAT   SCA_VALID | SCA_TAB_3D \
                    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE

//============================================================================

namespace {

PointerStyle lclGetPointerForField( ScDPFieldType eType )
{
    switch( eType )
    {
        case TYPE_PAGE:     return POINTER_PIVOT_FIELD;
        case TYPE_COL:      return POINTER_PIVOT_COL;
        case TYPE_ROW:      return POINTER_PIVOT_ROW;
        case TYPE_DATA:     return POINTER_PIVOT_FIELD;
        case TYPE_SELECT:   return POINTER_PIVOT_FIELD;
    }
    return POINTER_ARROW;
}

} // namespace

//============================================================================

//----------------------------------------------------------------------------

ScDPLayoutDlg::ScDPLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const ScDPObject& rDPObject, bool bNewOutput )
    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_PIVOT_LAYOUT ),
        aFlLayout       ( this, ScResId( FL_LAYOUT ) ),
        aFtPage         ( this, ScResId( FT_PAGE ) ),
        aWndPage        ( this, ScResId( WND_PAGE ), &aFtPage ),
        aFtCol          ( this, ScResId( FT_COL ) ),
        aWndCol         ( this, ScResId( WND_COL ), &aFtCol ),
        aFtRow          ( this, ScResId( FT_ROW ) ),
        aWndRow         ( this, ScResId( WND_ROW ), &aFtRow ),
        aFtData         ( this, ScResId( FT_DATA ) ),
        aWndData        ( this, ScResId( WND_DATA ), &aFtData ),
        aWndSelect      ( this, ScResId( WND_SELECT ), NULL ),
        aFtInfo         ( this, ScResId( FT_INFO ) ),

        aFlAreas        ( this, ScResId( FL_OUTPUT ) ),

        aFtInArea       ( this, ScResId( FT_INAREA) ),
        aEdInPos        ( this, ScResId( ED_INAREA) ),
        aRbInPos        ( this, ScResId( RB_INAREA ), &aEdInPos, this ),

        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aFtOutArea      ( this, ScResId( FT_OUTAREA ) ),
        aEdOutPos       ( this, this, ScResId( ED_OUTAREA ) ),
        aRbOutPos       ( this, ScResId( RB_OUTAREA ), &aEdOutPos, this ),
        aBtnIgnEmptyRows( this, ScResId( BTN_IGNEMPTYROWS ) ),
        aBtnDetectCat   ( this, ScResId( BTN_DETECTCAT ) ),
        aBtnTotalCol    ( this, ScResId( BTN_TOTALCOL ) ),
        aBtnTotalRow    ( this, ScResId( BTN_TOTALROW ) ),
        aBtnFilter      ( this, ScResId( BTN_FILTER ) ),
        aBtnDrillDown   ( this, ScResId( BTN_DRILLDOWN ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnOptions     ( this, ScResId( BTN_OPTIONS ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrNewTable    ( ScResId( SCSTR_NEWTABLE ) ),

        bIsDrag         ( FALSE ),

        pEditActive     ( NULL ),

        eLastActiveType ( TYPE_SELECT ),
        nOffset         ( 0 ),
        //
        xDlgDPObject    ( new ScDPObject( rDPObject ) ),
        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        bRefInputMode   (false)
{
    xDlgDPObject->SetAlive( TRUE );     // needed to get structure information
    xDlgDPObject->FillOldParam( thePivotData, FALSE );
    xDlgDPObject->FillLabelData( thePivotData );

    Init(bNewOutput);
    FreeResource();
}

//----------------------------------------------------------------------------

ScDPLayoutDlg::~ScDPLayoutDlg()
{
    USHORT nEntries = aLbOutPos.GetEntryCount();
    USHORT i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)aLbOutPos.GetEntryData( i );
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::Init(bool bNewOutput)
{
    DBG_ASSERT( pViewData && pDoc,
                "Ctor-Initialisierung fehlgeschlagen!" );

    aBtnRemove.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );
    aBtnOptions.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );

    aFuncNameArr.reserve( PIVOT_MAXFUNC );
    for ( USHORT i = 0; i < PIVOT_MAXFUNC; ++i )
        aFuncNameArr.push_back( String( ScResId( i + 1 ) ) );

    aBtnMore.AddWindow( &aFlAreas );
    aBtnMore.AddWindow( &aFtInArea );
    aBtnMore.AddWindow( &aEdInPos );
    aBtnMore.AddWindow( &aRbInPos );
    aBtnMore.AddWindow( &aFtOutArea );
    aBtnMore.AddWindow( &aLbOutPos );
    aBtnMore.AddWindow( &aEdOutPos );
    aBtnMore.AddWindow( &aRbOutPos );
    aBtnMore.AddWindow( &aBtnIgnEmptyRows );
    aBtnMore.AddWindow( &aBtnDetectCat );
    aBtnMore.AddWindow( &aBtnTotalCol );
    aBtnMore.AddWindow( &aBtnTotalRow );
    aBtnMore.AddWindow( &aBtnFilter );
    aBtnMore.AddWindow( &aBtnDrillDown );
    aBtnMore.SetClickHdl( LINK( this, ScDPLayoutDlg, MoreClickHdl ) );

    CalcWndSizes();

    ScRange inRange;
    String inString;
    if (xDlgDPObject->GetSheetDesc())
    {
        aEdInPos.Enable();
        aRbInPos.Enable();
        const ScSheetSourceDesc* p = xDlgDPObject->GetSheetDesc();
        OUString aRangeName = p->GetRangeName();
        if (aRangeName.getLength())
            aEdInPos.SetText(aRangeName);
        else
        {
            aOldRange = p->GetSourceRange();
            aOldRange.Format( inString, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
            aEdInPos.SetText(inString);
        }
    }
    else
    {
        /* Data is not reachable, so could be a remote database */
        aEdInPos.Disable();
        aRbInPos.Disable();
    }

    InitFields();

    aLbOutPos .SetSelectHdl( LINK( this, ScDPLayoutDlg, SelAreaHdl ) );
    aEdOutPos .SetModifyHdl( LINK( this, ScDPLayoutDlg, EdModifyHdl ) );
    aEdInPos  .SetModifyHdl( LINK( this, ScDPLayoutDlg, EdInModifyHdl ) );
    aBtnOk    .SetClickHdl ( LINK( this, ScDPLayoutDlg, OkHdl ) );
    aBtnCancel.SetClickHdl ( LINK( this, ScDPLayoutDlg, CancelHdl ) );
    Link aLink = LINK( this, ScDPLayoutDlg, GetFocusHdl );
    if ( aEdInPos.IsEnabled() )
        // Once disabled it will never get enabled, so no need to handle focus.
        aEdInPos.SetGetFocusHdl( aLink );
    aEdOutPos.SetGetFocusHdl( aLink );

    if ( pViewData && pDoc )
    {
        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * in einem Zeiger-Array gemerkt, bei denen es sich
         * um sinnvolle Bereiche handelt
         */

        aLbOutPos.Clear();
        aLbOutPos.InsertEntry( aStrUndefined, 0 );
        aLbOutPos.InsertEntry( aStrNewTable,  1 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                USHORT nInsert = aLbOutPos.InsertEntry( aName );

                aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, pDoc->GetAddressConvention() );
                aLbOutPos.SetEntryData( nInsert, new String( aRefStr ) );
            }
        }
    }

    if (bNewOutput)
    {
        // Output to a new sheet by default for a brand-new output.
        aLbOutPos.SelectEntryPos(1);
        aEdOutPos.Disable();
        aRbOutPos.Disable();
    }
    else
    {
        // Modifying an existing dp output.

        if ( thePivotData.nTab != MAXTAB+1 )
        {
            String aStr;
            ScAddress( thePivotData.nCol,
                       thePivotData.nRow,
                       thePivotData.nTab ).Format( aStr, STD_FORMAT, pDoc, pDoc->GetAddressConvention() );
            aEdOutPos.SetText( aStr );
            EdModifyHdl(0);
        }
        else
        {
            aLbOutPos.SelectEntryPos( aLbOutPos.GetEntryCount()-1 );
            SelAreaHdl(NULL);
        }
    }

    aBtnIgnEmptyRows.Check( thePivotData.bIgnoreEmptyRows );
    aBtnDetectCat   .Check( thePivotData.bDetectCategories );
    aBtnTotalCol    .Check( thePivotData.bMakeTotalCol );
    aBtnTotalRow    .Check( thePivotData.bMakeTotalRow );

    if( const ScDPSaveData* pSaveData = xDlgDPObject->GetSaveData() )
    {
        aBtnFilter.Check( pSaveData->GetFilterButton() );
        aBtnDrillDown.Check( pSaveData->GetDrillDown() );
    }
    else
    {
        aBtnFilter.Check();
        aBtnDrillDown.Check();
    }

    aWndPage.SetHelpId( HID_SC_DPLAY_PAGE );
    aWndCol.SetHelpId( HID_SC_DPLAY_COLUMN );
    aWndRow.SetHelpId( HID_SC_DPLAY_ROW );
    aWndData.SetHelpId( HID_SC_DPLAY_DATA );
    aWndSelect.SetHelpId( HID_SC_DPLAY_SELECT );

    InitFocus();
}

//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::StateChanged( StateChangedType nStateChange )
{
    ScAnyRefDlg::StateChanged( nStateChange );

    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        // Hiding the FixedTexts and clearing the tab stop style bits
        // has to be done after assigning the mnemonics, but Paint is too late,
        // because the test tool may send key events to the dialog when it isn't visible.
        // Mnemonics are assigned in the Dialog::StateChanged for STATE_CHANGE_INITSHOW,
        // so this can be done immediately afterwards.

        aWndPage.UseMnemonic();
        aWndCol.UseMnemonic();
        aWndRow.UseMnemonic();
        aWndData.UseMnemonic();
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWndSelect( const vector<ScDPLabelDataRef>& rLabels )
{
    size_t nLabelCount = rLabels.size();
    if (nLabelCount > MAX_LABELS)
        nLabelCount = MAX_LABELS;

    aLabelDataArr.clear();
    aLabelDataArr.reserve( nLabelCount );
    for ( size_t i=0; i < nLabelCount; i++ )
    {
        aLabelDataArr.push_back(*rLabels[i]);
        aWndSelect.AddField(aLabelDataArr[i].getDisplayName(), i);
        ScDPFuncDataRef p(new ScDPFuncData(aLabelDataArr[i].mnCol, aLabelDataArr[i].mnFuncMask));
        aSelectArr.push_back(p);
    }
    aWndSelect.ResetScrollBar();
    aWndSelect.Paint(Rectangle());
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitFieldWindow( const vector<PivotField>& rFields, ScDPFieldType eType )
{
    ScDPFuncDataVec* pInitArr = GetFieldDataArray(eType);
    ScDPFieldControlBase* pInitWnd = GetFieldWindow(eType);

    if (!pInitArr || !pInitWnd)
        return;

    vector<PivotField>::const_iterator itr = rFields.begin(), itrEnd = rFields.end();
    for (; itr != itrEnd; ++itr)
    {
        SCCOL nCol = itr->nCol;
        USHORT nMask = itr->nFuncMask;
        if (nCol == PIVOT_DATA_FIELD)
            continue;

        size_t nFieldIndex = pInitArr->size();
        ScDPFuncDataRef p(new ScDPFuncData(nCol, nMask, itr->maFieldRef));
        pInitArr->push_back(p);

        if (eType == TYPE_DATA)
        {
            // data field - we need to concatenate function name with the field name.
            ScDPLabelData* pData = GetLabelData(nCol);
            DBG_ASSERT( pData, "ScDPLabelData not found" );
            if (pData)
            {
                OUString aStr = pData->maLayoutName;
                if (!aStr.getLength())
                {
                    USHORT nInitMask = pInitArr->back()->mnFuncMask;
                    aStr = GetFuncString(nInitMask, pData->mbIsValue);
                    aStr += pData->maName;
                }

                pInitWnd->AddField(aStr, nFieldIndex);
                pData->mnFuncMask = nMask;
            }
        }
        else
            pInitWnd->AddField(GetLabelString(nCol), nFieldIndex);
    }
    pInitWnd->ResetScrollBar();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitFocus()
{
    if( aWndSelect.IsEmpty() )
    {
        aBtnOk.GrabFocus();
        NotifyFieldFocus( TYPE_SELECT, FALSE );
    }
    else
        aWndSelect.GrabFocus();
}

void ScDPLayoutDlg::InitFields()
{
    InitWndSelect(thePivotData.maLabelArray);
    InitFieldWindow(thePivotData.maPageFields, TYPE_PAGE);
    InitFieldWindow(thePivotData.maColFields, TYPE_COL);
    InitFieldWindow(thePivotData.maRowFields, TYPE_ROW);
    InitFieldWindow(thePivotData.maDataFields, TYPE_DATA);
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::AddField( size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    ScDPFuncData        fData( *(aSelectArr[nFromIndex]) );
    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    ScDPFuncDataVec*    toArr = GetFieldDataArray(eToType);
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    GetOtherDataArrays(eToType, rmArr1, rmArr2);

    bool bDataArr = eToType == TYPE_DATA;

    bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );
    if ( bAllowed && (!Contains( toArr, fData.mnCol, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, fData.mnCol, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, fData.mnCol, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        ScDPLabelData&  rData = aLabelDataArr[nFromIndex+nOffset];
        size_t      nAddedAt = 0;

        if ( !bDataArr )
        {
            if ( toWnd->AddField( rData.getDisplayName(),
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
        else
        {
            ScDPLabelData* p = GetLabelData(fData.mnCol);
            OUString aStr = p->maLayoutName;
            USHORT nMask = fData.mnFuncMask;
            if (!aStr.getLength())
            {
                aStr = GetFuncString(nMask);
                aStr += p->maName;
            }

            if ( toWnd->AddField( aStr,
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                fData.mnFuncMask = nMask;
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }

    }
}

void ScDPLayoutDlg::AppendField(size_t nFromIndex, ScDPFieldType eToType)
{
    ScDPFuncData aFuncData = *aSelectArr[nFromIndex];

    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    ScDPFuncDataVec*    toArr = GetFieldDataArray(eToType);
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    GetOtherDataArrays(eToType, rmArr1, rmArr2);

    bool bDataArr = eToType == TYPE_DATA;

    if ( (!Contains( toArr, aFuncData.mnCol, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, aFuncData.mnCol, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, aFuncData.mnCol, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        ScDPLabelData&  rData = aLabelDataArr[nFromIndex+nOffset];
        size_t      nAddedAt = 0;

        if ( !bDataArr )
        {
            if ( toWnd->AppendField(rData.getDisplayName(), nAddedAt) )
            {
                Insert( toArr, aFuncData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
        else
        {
            ScDPLabelData* p = GetLabelData(aFuncData.mnCol);
            OUString aStr = p->maLayoutName;
            USHORT nMask = aFuncData.mnFuncMask;
            if (!aStr.getLength())
            {
                aStr = GetFuncString(nMask);
                aStr += p->maName;
            }

            if ( toWnd->AppendField(aStr, nAddedAt) )
            {
                aFuncData.mnFuncMask = nMask;
                Insert( toArr, aFuncData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::MoveField( ScDPFieldType eFromType, size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    if ( eFromType == TYPE_SELECT )
        AddField( nFromIndex, eToType, rAtPos );
    else if ( eFromType != eToType )
    {
        ScDPFieldControlBase* fromWnd  = GetFieldWindow(eFromType);
        ScDPFieldControlBase* toWnd    = GetFieldWindow(eToType);

        ScDPFieldControlBase* rmWnd1   = NULL;
        ScDPFieldControlBase* rmWnd2   = NULL;
        GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

        ScDPFuncDataVec*    fromArr  = GetFieldDataArray(eFromType);
        ScDPFuncDataVec*    toArr    = GetFieldDataArray(eToType);

        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        GetOtherDataArrays(eToType, rmArr1, rmArr2);

        bool bDataArr = eToType == TYPE_DATA;

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScDPFuncData fData( *((*fromArr)[nFromIndex]) );
            bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );

            size_t nAt = 0;
            if ( bAllowed && Contains( fromArr, fData.mnCol, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (!Contains( toArr, fData.mnCol, nAt ))
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData.mnCol, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData.mnCol, nAt ) )
                            {
                                rmWnd2->DelField( nAt );
                                Remove( rmArr2, nAt );
                            }
                        }

                        if ( toWnd->AddField( GetLabelString( fData.mnCol ),
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                    else
                    {
                        ScDPLabelData* p = GetLabelData(fData.mnCol);
                        OUString aStr = p->maLayoutName;
                        USHORT nMask = fData.mnFuncMask;
                        if (!aStr.getLength())
                        {
                            aStr = GetFuncString(nMask);
                            aStr += p->maName;
                        }

                        if ( toWnd->AddField( aStr,
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            fData.mnFuncMask = nMask;
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldControlBase* theWnd  = GetFieldWindow(eFromType);
        ScDPFuncDataVec*    theArr   = GetFieldDataArray(eFromType);
        size_t              nAt      = 0;
        Point               aToPos;
        BOOL                bDataArr = eFromType == TYPE_DATA;

        ScDPFuncData fData( *((*theArr)[nFromIndex]) );

        if ( Contains( theArr, fData.mnCol, nAt ) )
        {
            size_t nToIndex = 0;
            aToPos = DlgPos2WndPos( rAtPos, *theWnd );
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;

                theWnd->DelField( nAt );
                Remove( theArr, nAt );

                if ( !bDataArr )
                {
                    if ( theWnd->AddField( GetLabelString( fData.mnCol ),
                                           aToPos,
                                           nAddedAt ) )
                    {
                        Insert( theArr, fData, nAddedAt );
                    }
                }
                else
                {
                    ScDPLabelData* p = GetLabelData(fData.mnCol);
                    OUString aStr = p->maLayoutName;
                    USHORT nMask = fData.mnFuncMask;
                    if (!aStr.getLength())
                    {
                        aStr = GetFuncString(nMask);
                        aStr += p->maName;
                    }

                    if ( theWnd->AddField( aStr,
                                           DlgPos2WndPos( rAtPos, *theWnd ),
                                           nAddedAt ) )
                    {
                        fData.mnFuncMask = nMask;
                        Insert( theArr, fData, nAddedAt );
                    }
                }
            }
        }
    }
}

void ScDPLayoutDlg::MoveFieldToEnd( ScDPFieldType eFromType, size_t nFromIndex, ScDPFieldType eToType )
{
    if ( eFromType == TYPE_SELECT )
        AppendField( nFromIndex, eToType );
    else if ( eFromType != eToType )
    {
        ScDPFieldControlBase* fromWnd  = GetFieldWindow(eFromType);
        ScDPFieldControlBase* toWnd    = GetFieldWindow(eToType);

        ScDPFieldControlBase* rmWnd1   = NULL;
        ScDPFieldControlBase* rmWnd2   = NULL;
        GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

        ScDPFuncDataVec*    fromArr  = GetFieldDataArray(eFromType);
        ScDPFuncDataVec*    toArr    = GetFieldDataArray(eToType);

        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        GetOtherDataArrays(eToType, rmArr1, rmArr2);

        bool bDataArr = eToType == TYPE_DATA;

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScDPFuncData fData( *((*fromArr)[nFromIndex]) );

            size_t nAt = 0;
            if ( Contains( fromArr, fData.mnCol, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (!Contains( toArr, fData.mnCol, nAt ))
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData.mnCol, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData.mnCol, nAt ) )
                            {
                                rmWnd2->DelField( nAt );
                                Remove( rmArr2, nAt );
                            }
                        }

                        if ( toWnd->AppendField( GetLabelString( fData.mnCol ), nAddedAt ) )
                        {
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                    else
                    {
                        ScDPLabelData* p = GetLabelData(fData.mnCol);
                        OUString aStr = p->maLayoutName;
                        USHORT nMask = fData.mnFuncMask;
                        if (!aStr.getLength())
                        {
                            aStr = GetFuncString(nMask);
                            aStr += p->maName;
                        }

                        if ( toWnd->AppendField(aStr, nAddedAt) )
                        {
                            fData.mnFuncMask = nMask;
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldControlBase* theWnd  = GetFieldWindow(eFromType);
        ScDPFuncDataVec*    theArr   = GetFieldDataArray(eFromType);
        size_t              nAt      = 0;
        Point               aToPos;
        BOOL                bDataArr = eFromType == TYPE_DATA;

        ScDPFuncData fData( *((*theArr)[nFromIndex]) );

        if ( Contains( theArr, fData.mnCol, nAt ) )
        {
            size_t nToIndex = 0;
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;

                theWnd->DelField( nAt );
                Remove( theArr, nAt );

                if ( !bDataArr )
                {
                    if ( theWnd->AppendField(GetLabelString( fData.mnCol ), nAddedAt) )
                    {
                        Insert( theArr, fData, nAddedAt );
                    }
                }
                else
                {
                    ScDPLabelData* p = GetLabelData(fData.mnCol);
                    OUString aStr = p->maLayoutName;
                    USHORT nMask = fData.mnFuncMask;
                    if (!aStr.getLength())
                    {
                        aStr = GetFuncString(nMask);
                        aStr += p->maName;
                    }

                    if ( theWnd->AppendField(aStr, nAddedAt) )
                    {
                        fData.mnFuncMask = nMask;
                        Insert( theArr, fData, nAddedAt );
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::RemoveField( ScDPFieldType eFromType, size_t nIndex )
{
    ScDPFuncDataVec* pArr = GetFieldDataArray(eFromType);

    if( pArr )
    {
        ScDPFieldControlBase* pWnd = GetFieldWindow( eFromType );
        if (pWnd)
        {
            pWnd->DelField( nIndex );
            Remove( pArr, nIndex );
            if( pWnd->IsEmpty() ) InitFocus();
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMouseButtonUp( const Point& rAt )
{
    if ( bIsDrag )
    {
        bIsDrag = FALSE;

        ScDPFieldType   eDnDToType = TYPE_SELECT;
        Point           aPos = ScreenToOutputPixel( rAt );
        bool            bDel = false;

        if ( aRectPage.IsInside( aPos ) )
        {
            eDnDToType = TYPE_PAGE;
        }
        else if ( aRectCol.IsInside( aPos ) )
        {
            eDnDToType = TYPE_COL;
        }
        else if ( aRectRow.IsInside( aPos ) )
        {
            eDnDToType = TYPE_ROW;
        }
        else if ( aRectData.IsInside( aPos ) )
        {
            eDnDToType = TYPE_DATA;
        }
        else if ( aRectSelect.IsInside( aPos ) )
        {
            eDnDToType = TYPE_SELECT;
        }
        else
            bDel = true;

        if (bDel)
        {
            // We don't remove any buttons from the select field.
            if (eDnDFromType != TYPE_SELECT)
                RemoveField( eDnDFromType, nDnDFromIndex );
        }
        else
            MoveField( eDnDFromType, nDnDFromIndex, eDnDToType, aPos );
    }
}

//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseMove( const Point& rAt )
{
    PointerStyle ePtr = POINTER_ARROW;

    if ( bIsDrag )
    {
        Point aPos = ScreenToOutputPixel( rAt );
        ScDPFieldType eCheckTarget = TYPE_SELECT;

        if ( aRectPage.IsInside( aPos ) )
            eCheckTarget = TYPE_PAGE;
        else if ( aRectCol.IsInside( aPos ) )
            eCheckTarget = TYPE_COL;
        else if ( aRectRow.IsInside( aPos ) )
            eCheckTarget = TYPE_ROW;
        else if ( aRectData.IsInside( aPos ) )
            eCheckTarget = TYPE_DATA;
        else if ( eDnDFromType != TYPE_SELECT )
            ePtr = POINTER_PIVOT_DELETE;
        else if ( aRectSelect.IsInside( aPos ) )
            ePtr = lclGetPointerForField( TYPE_SELECT );
        else
            ePtr = POINTER_NOTALLOWED;

        if ( eCheckTarget != TYPE_SELECT )
        {
            // check if the target orientation is allowed for this field
            ScDPFuncDataVec* fromArr = NULL;
            switch ( eDnDFromType )
            {
                case TYPE_PAGE:   fromArr = &aPageArr;   break;
                case TYPE_COL:    fromArr = &aColArr;    break;
                case TYPE_ROW:    fromArr = &aRowArr;    break;
                case TYPE_DATA:   fromArr = &aDataArr;   break;
                case TYPE_SELECT: fromArr = &aSelectArr; break;
            }
            ScDPFuncData fData( *((*fromArr)[nDnDFromIndex]) );
            if (IsOrientationAllowed( fData.mnCol, eCheckTarget ))
                ePtr = lclGetPointerForField( eCheckTarget );
            else
                ePtr = POINTER_NOTALLOWED;
        }
    }

    return ePtr;
}

//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseButtonDown( ScDPFieldType eType, size_t nFieldIndex )
{
    bIsDrag       = TRUE;
    eDnDFromType  = eType;
    nDnDFromIndex = nFieldIndex;
    return lclGetPointerForField( eType );
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyDoubleClick( ScDPFieldType eType, size_t nFieldIndex )
{
    ScDPFuncDataVec* pArr = GetFieldDataArray(eType);

    if ( pArr )
    {
        if ( nFieldIndex >= pArr->size() )
        {
            DBG_ERROR("invalid selection");
            return;
        }

        size_t nArrPos = 0;
        if( ScDPLabelData* pData = GetLabelData( (*pArr)[nFieldIndex]->mnCol, &nArrPos ) )
        {
            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

            switch ( eType )
            {
                case TYPE_PAGE:
                case TYPE_COL:
                case TYPE_ROW:
                {
                    // list of names of all data fields
                    vector<ScDPName> aDataFieldNames;
                    for( ScDPFuncDataVec::const_iterator aIt = aDataArr.begin(), aEnd = aDataArr.end();
                            (aIt != aEnd) && aIt->get(); ++aIt )
                    {
                        ScDPLabelData* pDFData = GetLabelData((*aIt)->mnCol);
                        if (!pDFData)
                            continue;

                        if (!pDFData->maName.getLength())
                            continue;

                        OUString aLayoutName = pDFData->maLayoutName;
                        if (!aLayoutName.getLength())
                        {
                            // No layout name exists.  Use the stock name.
                            USHORT nMask = (*aIt)->mnFuncMask;
                            OUString aFuncStr = GetFuncString(nMask);
                            aLayoutName = aFuncStr + pDFData->maName;
                        }
                        aDataFieldNames.push_back(ScDPName(pDFData->maName, aLayoutName));
                    }

                    bool bLayout = (eType == TYPE_ROW) &&
                        ((aDataFieldNames.size() > 1) || ((nFieldIndex + 1 < pArr->size()) && (*pArr)[nFieldIndex+1].get()));

                    AbstractScDPSubtotalDlg* pDlg = pFact->CreateScDPSubtotalDlg(
                        this, RID_SCDLG_PIVOTSUBT,
                        *xDlgDPObject, *pData, *(*pArr)[nFieldIndex], aDataFieldNames, bLayout );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        pDlg->FillLabelData( *pData );
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask;
                    }
                    delete pDlg;
                }
                break;

                case TYPE_DATA:
                {
                    AbstractScDPFunctionDlg* pDlg = pFact->CreateScDPFunctionDlg(
                        this, RID_SCDLG_DPDATAFIELD,
                        aLabelDataArr, *pData, *(*pArr)[nFieldIndex] );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask = pDlg->GetFuncMask();
                        (*pArr)[nFieldIndex]->maFieldRef = pDlg->GetFieldRef();

                        ScDPLabelData* p = GetLabelData(aDataArr[nFieldIndex]->mnCol);
                        OUString aStr = p->maLayoutName;
                        if (!aStr.getLength())
                        {
                            // Layout name is not available.  Use default name.
                            aStr = GetFuncString (aDataArr[nFieldIndex]->mnFuncMask);
                            aStr += p->maName;
                        }
                        aWndData.SetFieldText( aStr, nFieldIndex );
                    }
                    delete pDlg;
                }
                break;

                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyFieldFocus( ScDPFieldType eType, BOOL bGotFocus )
{
    /*  Enable Remove/Options buttons on GetFocus in field window.
        Enable them also, if dialog is deactivated (click into document).
        The !IsActive() condition handles the case that a LoseFocus event of a
        field window would follow the Deactivate event of this dialog. */
    BOOL bEnable = (bGotFocus || !IsActive()) && (eType != TYPE_SELECT);

    // The TestTool may set the focus into an empty field.
    // Then the Remove/Options buttons must be disabled.
    ScDPFieldControlBase* pWnd = GetFieldWindow(eType);
    if ( bEnable && bGotFocus && pWnd && pWnd->IsEmpty() )
        bEnable = FALSE;

    aBtnRemove.Enable( bEnable );
    aBtnOptions.Enable( bEnable );
    if( bGotFocus )
        eLastActiveType = eType;
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMoveFieldToEnd( ScDPFieldType eToType )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow(eLastActiveType);
    ScDPFieldControlBase* pToWnd = GetFieldWindow(eToType);
    if (pWnd && pToWnd && (eToType != TYPE_SELECT) && !pWnd->IsEmpty())
    {
        MoveFieldToEnd(eLastActiveType, pWnd->GetSelectedField(), eToType);

        if( pWnd->IsEmpty() )
            NotifyFieldFocus( eToType, TRUE );
        else
            pWnd->GrabFocus();
        if( eLastActiveType == TYPE_SELECT )
            aWndSelect.SelectNext();
    }
    else
        InitFocus();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyRemoveField( ScDPFieldType eType, size_t nFieldIndex )
{
    if( eType != TYPE_SELECT )
        RemoveField( eType, nFieldIndex );
}

void ScDPLayoutDlg::Deactivate()
{
    /*  If the dialog has been deactivated (click into document), the LoseFocus
        event from field window disables Remove/Options buttons. Re-enable them here by
        simulating a GetFocus event. Event order of LoseFocus and Deactivate is not important.
        The last event will enable the buttons in both cases (see NotifyFieldFocus). */
    NotifyFieldFocus( eLastActiveType, TRUE );
}

//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::Contains( ScDPFuncDataVec* pArr, SCsCOL nCol, size_t& nAt )
{
    if (!pArr || pArr->empty())
        return FALSE;

    ScDPFuncDataVec::const_iterator itr, itrBeg = pArr->begin(), itrEnd = pArr->end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        if ((*itr)->mnCol == nCol)
        {
            // found!
            nAt = ::std::distance(itrBeg, itr);
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::Remove( ScDPFuncDataVec* pArr, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    pArr->erase( pArr->begin() + nAt );
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::Insert( ScDPFuncDataVec* pArr, const ScDPFuncData& rFData, size_t nAt )
{
    if (!pArr)
        return;

    ScDPFuncDataRef p (new ScDPFuncData(rFData));
    if (nAt >= pArr->size())
        pArr->push_back(p);
    else
        pArr->insert(pArr->begin() + nAt, p);
}

//----------------------------------------------------------------------------

ScDPLabelData* ScDPLayoutDlg::GetLabelData( SCsCOL nCol, size_t* pnPos )
{
    ScDPLabelData* pData = 0;
    for( ScDPLabelDataVec::iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); !pData && (aIt != aEnd); ++aIt )
    {
        if( aIt->mnCol == nCol )
        {
            pData = &*aIt;
            if( pnPos ) *pnPos = aIt - aLabelDataArr.begin();
        }
    }
    return pData;
}

//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetLabelString( SCsCOL nCol )
{
    ScDPLabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData)
        return pData->getDisplayName();
    return String();
}

//----------------------------------------------------------------------------

bool ScDPLayoutDlg::IsOrientationAllowed( SCsCOL nCol, ScDPFieldType eType )
{
    bool bAllowed = true;
    ScDPLabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData)
    {
        sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        switch (eType)
        {
            case TYPE_PAGE:   eOrient = sheet::DataPilotFieldOrientation_PAGE;   break;
            case TYPE_COL:    eOrient = sheet::DataPilotFieldOrientation_COLUMN; break;
            case TYPE_ROW:    eOrient = sheet::DataPilotFieldOrientation_ROW;    break;
            case TYPE_DATA:   eOrient = sheet::DataPilotFieldOrientation_DATA;   break;
            case TYPE_SELECT: eOrient = sheet::DataPilotFieldOrientation_HIDDEN; break;
        }
        bAllowed = ScDPObject::IsOrientationAllowed( (USHORT)eOrient, pData->mnFlags );
    }
    return bAllowed;
}

//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetFuncString( USHORT& rFuncMask, BOOL bIsValue )
{
    String aStr;

    if (   rFuncMask == PIVOT_FUNC_NONE
        || rFuncMask == PIVOT_FUNC_AUTO )
    {
        if ( bIsValue )
        {
            aStr = FSTR(PIVOTSTR_SUM);
            rFuncMask = PIVOT_FUNC_SUM;
        }
        else
        {
            aStr = FSTR(PIVOTSTR_COUNT);
            rFuncMask = PIVOT_FUNC_COUNT;
        }
    }
    else if ( rFuncMask == PIVOT_FUNC_SUM )       aStr = FSTR(PIVOTSTR_SUM);
    else if ( rFuncMask == PIVOT_FUNC_COUNT )     aStr = FSTR(PIVOTSTR_COUNT);
    else if ( rFuncMask == PIVOT_FUNC_AVERAGE )   aStr = FSTR(PIVOTSTR_AVG);
    else if ( rFuncMask == PIVOT_FUNC_MAX )       aStr = FSTR(PIVOTSTR_MAX);
    else if ( rFuncMask == PIVOT_FUNC_MIN )       aStr = FSTR(PIVOTSTR_MIN);
    else if ( rFuncMask == PIVOT_FUNC_PRODUCT )   aStr = FSTR(PIVOTSTR_PROD);
    else if ( rFuncMask == PIVOT_FUNC_COUNT_NUM ) aStr = FSTR(PIVOTSTR_COUNT2);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEV )   aStr = FSTR(PIVOTSTR_DEV);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEVP )  aStr = FSTR(PIVOTSTR_DEV2);
    else if ( rFuncMask == PIVOT_FUNC_STD_VAR )   aStr = FSTR(PIVOTSTR_VAR);
    else if ( rFuncMask == PIVOT_FUNC_STD_VARP )  aStr = FSTR(PIVOTSTR_VAR2);
    else
    {
        aStr  = ScGlobal::GetRscString( STR_TABLE_ERGEBNIS );
        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " - " ));
    }

    return aStr;
}

//----------------------------------------------------------------------------

Point ScDPLayoutDlg::DlgPos2WndPos( const Point& rPt, Window& rWnd )
{
    Point aWndPt( rPt );
    aWndPt.X() = rPt.X()-rWnd.GetPosPixel().X();
    aWndPt.Y() = rPt.Y()-rWnd.GetPosPixel().Y();

    return aWndPt;
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::CalcWndSizes()
{
    // row/column/data area sizes
    long nFldW = FIELD_BTN_WIDTH;
    long nFldH = FIELD_BTN_HEIGHT;
    aWndData.SetSizePixel(Size(338, 185));
    aWndPage.SetSizePixel(
        Size(aWndData.GetSizePixel().Width() + 85,
             aWndCol.GetPosPixel().Y() - aWndPage.GetPosPixel().Y() - FIELD_AREA_GAP));
    aWndRow.SetSizePixel(
        Size(aWndData.GetPosPixel().X()-aWndRow.GetPosPixel().X() - FIELD_AREA_GAP,
             aWndData.GetSizePixel().Height()));
    aWndCol.SetSizePixel(
        Size(aWndData.GetPosPixel().X() - aWndCol.GetPosPixel().X() + aWndData.GetSizePixel().Width(),
             aWndData.GetPosPixel().Y() - aWndCol.GetPosPixel().Y() - FIELD_AREA_GAP));

    // #i29203# align right border of page window with data window
    long nDataPosX = aWndData.GetPosPixel().X() + aWndData.GetSizePixel().Width();
    aWndPage.SetPosPixel(
        Point(nDataPosX - aWndPage.GetSizePixel().Width(),
              aWndPage.GetPosPixel().Y()));

    // selection area
    long nLineSize = 10; // number of fields per column.
    long nH = OUTER_MARGIN_VER + nLineSize* nFldH + nLineSize * ROW_FIELD_BTN_GAP;
    nH += ROW_FIELD_BTN_GAP;
    nH += GetSettings().GetStyleSettings().GetScrollBarSize() + OUTER_MARGIN_VER;
    aWndSelect.SetSizePixel(
        Size(2 * nFldW + ROW_FIELD_BTN_GAP + 10, nH));

    aRectPage   = Rectangle( aWndPage.GetPosPixel(),    aWndPage.GetSizePixel() );
    aRectRow    = Rectangle( aWndRow.GetPosPixel(),     aWndRow.GetSizePixel() );
    aRectCol    = Rectangle( aWndCol.GetPosPixel(),     aWndCol.GetSizePixel() );
    aRectData   = Rectangle( aWndData.GetPosPixel(),    aWndData.GetSizePixel() );
    aRectSelect = Rectangle( aWndSelect.GetPosPixel(),  aWndSelect.GetSizePixel() );

    aWndPage.CalcSize();
    aWndRow.CalcSize();
    aWndCol.CalcSize();
    aWndData.CalcSize();
    aWndSelect.CalcSize();
}

namespace {

class PivotFieldInserter : public ::std::unary_function<void, boost::shared_ptr<ScDPFuncData> >
{
    vector<PivotField>& mrFields;
public:
    explicit PivotFieldInserter(vector<PivotField>& r, size_t nSize) : mrFields(r)
    {
        mrFields.reserve(nSize);
    }

    PivotFieldInserter(const PivotFieldInserter& r) : mrFields(r.mrFields) {}

    void operator() (const ::boost::shared_ptr<ScDPFuncData>& p)
    {
        PivotField aField;
        aField.nCol = p->mnCol;
        aField.nFuncMask = p->mnFuncMask;
        aField.maFieldRef = p->maFieldRef;
        mrFields.push_back(aField);
    }
};

}

bool ScDPLayoutDlg::GetPivotArrays(
    vector<PivotField>& rPageFields, vector<PivotField>& rColFields,
    vector<PivotField>& rRowFields, vector<PivotField>& rDataFields )
{
    vector<PivotField> aPageFields;
    for_each(aPageArr.begin(), aPageArr.end(), PivotFieldInserter(aPageFields, aPageArr.size()));

    vector<PivotField> aColFields;
    for_each(aColArr.begin(), aColArr.end(), PivotFieldInserter(aColFields, aColArr.size()));

    // default data pilot table always has an extra row field as a data layout field.
    vector<PivotField> aRowFields;
    for_each(aRowArr.begin(), aRowArr.end(), PivotFieldInserter(aRowFields, aRowArr.size()+1));
    aRowFields.push_back(PivotField(PIVOT_DATA_FIELD, 0));

    vector<PivotField> aDataFields;
    for_each(aDataArr.begin(), aDataArr.end(), PivotFieldInserter(aDataFields, aDataArr.size()));

    rPageFields.swap(aPageFields);
    rColFields.swap(aColFields);
    rRowFields.swap(aRowFields);
    rDataFields.swap(aDataFields);

    return true;
}

void ScDPLayoutDlg::UpdateSrcRange()
{
    String  aSrcStr = aEdInPos.GetText();
    USHORT  nResult = ScRange().Parse(aSrcStr, pDoc, pDoc->GetAddressConvention());
    DataSrcType eSrcType = SRC_INVALID;
    ScRange aNewRange;

    if (SCA_VALID == (nResult & SCA_VALID))
    {
        // Valid source range.  Take it.
        ScRefAddress start, end;
        ConvertDoubleRef(pDoc, aSrcStr, 1,  start, end, pDoc->GetAddressConvention());
        aNewRange.aStart = start.GetAddress();
        aNewRange.aEnd = end.GetAddress();
        aEdInPos.SetRefValid(true);
        eSrcType = SRC_REF;
    }
    else
    {
        // invalid source range.  Check if this is a valid range name.
        bool bValid = false;
        ScRangeName* pRangeName = pDoc->GetRangeName();
        if (pRangeName)
        {
            OUString aUpper = ScGlobal::pCharClass->upper(aSrcStr);
            USHORT n;
            bValid = pRangeName->SearchNameUpper(aUpper, n);
            if (bValid)
            {
                // range name found.  Check if this is a valid reference.
                ScRangeData* pData = (*pRangeName)[n];
                bValid = pData->IsReference(aNewRange);
            }
        }

        aEdInPos.SetRefValid(bValid);
        if (!bValid)
        {
            // All attempts have failed.  Give up.
            aBtnOk.Disable();
            return;
        }

        eSrcType = SRC_NAME;
    }

    aBtnOk.Enable();

    // Now update the data src range or range name with the dp object.
    ScSheetSourceDesc inSheet = *xDlgDPObject->GetSheetDesc();

    switch (eSrcType)
    {
        case SRC_REF:
            // data source is a range reference.
            if (inSheet.GetSourceRange() == aNewRange)
                // new range is identical to the current range.  Nothing to do.
                return;
            inSheet.SetSourceRange(aNewRange);
        break;
        case SRC_NAME:
            // data source is a range name.
            inSheet.SetRangeName(aSrcStr);
        break;
        default:
            OSL_ENSURE(false, "Unknown source type.");
            return;
    }

    xDlgDPObject->SetSheetDesc(inSheet);
    xDlgDPObject->FillOldParam( thePivotData, FALSE );
    xDlgDPObject->FillLabelData(thePivotData);

    aLabelDataArr.clear();
    aWndSelect.ClearFields();
    aWndData.ClearFields();
    aWndRow.ClearFields();
    aWndCol.ClearFields();
    aWndPage.ClearFields();

    aSelectArr.clear();
    aRowArr.clear();
    aColArr.clear();
    aDataArr.clear();
    aPageArr.clear();

    InitFields();
    RepaintFields();
}

void ScDPLayoutDlg::RepaintFields()
{
    Rectangle aRect; // currently has no effect whatsoever.
    aWndPage.Paint(aRect);
    aWndCol.Paint(aRect);
    aWndRow.Paint(aRect);
    aWndData.Paint(aRect);
}

ScDPFieldControlBase* ScDPLayoutDlg::GetFieldWindow(ScDPFieldType eType)
{
    switch (eType)
    {
        case TYPE_PAGE:
            return &aWndPage;
        case TYPE_COL:
            return &aWndCol;
        case TYPE_ROW:
            return &aWndRow;
        case TYPE_DATA:
            return &aWndData;
        case TYPE_SELECT:
            return &aWndSelect;
        default:
            ;
    }
    return NULL;
}

void ScDPLayoutDlg::GetOtherFieldWindows(ScDPFieldType eType, ScDPFieldControlBase*& rpWnd1, ScDPFieldControlBase*& rpWnd2)
{
    rpWnd1 = NULL;
    rpWnd2 = NULL;
    switch (eType)
    {
        case TYPE_PAGE:
            rpWnd1 = &aWndRow;
            rpWnd2 = &aWndCol;
            break;
        case TYPE_COL:
            rpWnd1 = &aWndPage;
            rpWnd2 = &aWndRow;
            break;
        case TYPE_ROW:
            rpWnd1 = &aWndPage;
            rpWnd2 = &aWndCol;
            break;
        default:
            ;
    }
}

ScDPLayoutDlg::ScDPFuncDataVec* ScDPLayoutDlg::GetFieldDataArray(ScDPFieldType eType)
{
    switch (eType)
    {
        case TYPE_PAGE:
            return &aPageArr;
        case TYPE_COL:
            return &aColArr;
        case TYPE_ROW:
            return &aRowArr;
        case TYPE_DATA:
            return &aDataArr;
        case TYPE_SELECT:
            return &aSelectArr;
        default:
            ;
    }
    return NULL;
}

void ScDPLayoutDlg::GetOtherDataArrays(
    ScDPFieldType eType, ScDPFuncDataVec*& rpArr1, ScDPFuncDataVec*& rpArr2)
{
    rpArr1 = NULL;
    rpArr2 = NULL;
    switch (eType)
    {
        case TYPE_PAGE:
            rpArr1 = &aRowArr;
            rpArr2 = &aColArr;
            break;
        case TYPE_COL:
            rpArr1 = &aPageArr;
            rpArr2 = &aRowArr;
            break;
        case TYPE_ROW:
            rpArr1 = &aPageArr;
            rpArr2 = &aColArr;
            break;
        default:
            ;
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( !bRefInputMode || !pEditActive )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( pEditActive );

    if ( pEditActive == &aEdInPos )
    {
        String aRefStr;
        rRef.Format( aRefStr, SCR_ABS_3D, pDocP, pDocP->GetAddressConvention() );
        pEditActive->SetRefString( aRefStr );
    }
    else if ( pEditActive == &aEdOutPos )
    {
        String aRefStr;
        rRef.aStart.Format( aRefStr, STD_FORMAT, pDocP, pDocP->GetAddressConvention() );
        pEditActive->SetRefString( aRefStr );
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if ( pEditActive )
            pEditActive->GrabFocus();

        if ( pEditActive == &aEdInPos )
            EdInModifyHdl( NULL );
        else if ( pEditActive == &aEdOutPos )
        EdModifyHdl( NULL );
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, ClickHdl, PushButton *, pBtn )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow( eLastActiveType );
    if (!pWnd)
        return 0;

    if( pBtn == &aBtnRemove )
    {
        RemoveField( eLastActiveType, pWnd->GetSelectedField() );
        if( !pWnd->IsEmpty() ) pWnd->GrabFocus();
    }
    else if( pBtn == &aBtnOptions )
    {
        NotifyDoubleClick( eLastActiveType, pWnd->GetSelectedField() );
        pWnd->GrabFocus();
    }
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, OkHdl, OKButton *, EMPTYARG )
{
    String      aOutPosStr( aEdOutPos.GetText() );
    ScAddress   aAdrDest;
    BOOL        bToNewTable = (aLbOutPos.GetSelectEntryPos() == 1);
    USHORT      nResult     = !bToNewTable ? aAdrDest.Parse( aOutPosStr, pDoc, pDoc->GetAddressConvention() ) : 0;

    if (!bToNewTable && (aOutPosStr.Len() == 0 || (nResult & SCA_VALID) != SCA_VALID))
    {
        // Invalid reference.  Bail out.
        if ( !aBtnMore.GetState() )
            aBtnMore.SetState(true);

        ErrorBox(this, WinBits(WB_OK | WB_DEF_OK), ScGlobal::GetRscString(STR_INVALID_TABREF)).Execute();
        aEdOutPos.GrabFocus();
        return 0;
    }

    ScPivotParam    theOutParam;
    vector<PivotField> aPageFields;
    vector<PivotField> aColFields;
    vector<PivotField> aRowFields;
    vector<PivotField> aDataFields;

    // Convert an array of function data into an array of pivot field data.
    bool bFit = GetPivotArrays(aPageFields, aColFields, aRowFields, aDataFields);

    if (!bFit)
    {
        // General data pilot table error.  Bail out.
        ErrorBox(this, WinBits(WB_OK | WB_DEF_OK), ScGlobal::GetRscString(STR_PIVOT_ERROR)).Execute();
        return 0;
    }

    ScDPSaveData* pOldSaveData = xDlgDPObject->GetSaveData();

    ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

    ScDPSaveData aSaveData;
    aSaveData.SetIgnoreEmptyRows( aBtnIgnEmptyRows.IsChecked() );
    aSaveData.SetRepeatIfEmpty( aBtnDetectCat.IsChecked() );
    aSaveData.SetColumnGrand( aBtnTotalCol.IsChecked() );
    aSaveData.SetRowGrand( aBtnTotalRow.IsChecked() );
    aSaveData.SetFilterButton( aBtnFilter.IsChecked() );
    aSaveData.SetDrillDown( aBtnDrillDown.IsChecked() );

    uno::Reference<sheet::XDimensionsSupplier> xSource = xDlgDPObject->GetSource();

    ScDPObject::ConvertOrientation(
        aSaveData, aPageFields, sheet::DataPilotFieldOrientation_PAGE, xSource );
    ScDPObject::ConvertOrientation(
        aSaveData, aColFields, sheet::DataPilotFieldOrientation_COLUMN, xSource );
    ScDPObject::ConvertOrientation(
        aSaveData, aRowFields, sheet::DataPilotFieldOrientation_ROW, xSource );
    ScDPObject::ConvertOrientation(
        aSaveData, aDataFields, sheet::DataPilotFieldOrientation_DATA, xSource,
        &aColFields, &aRowFields, &aPageFields );

    for( ScDPLabelDataVec::const_iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); aIt != aEnd; ++aIt )
    {
        if( ScDPSaveDimension* pDim = aSaveData.GetExistingDimensionByName( aIt->maName ) )
        {
            pDim->SetUsedHierarchy( aIt->mnUsedHier );
            pDim->SetShowEmpty( aIt->mbShowAll );
            pDim->SetSortInfo( &aIt->maSortInfo );
            pDim->SetLayoutInfo( &aIt->maLayoutInfo );
            pDim->SetAutoShowInfo( &aIt->maShowInfo );
            ScDPSaveDimension* pOldDim = NULL;
            if (pOldSaveData)
            {
                // Transfer the existing layout names to new dimension instance.
                pOldDim = pOldSaveData->GetExistingDimensionByName(aIt->maName);
                if (pOldDim)
                {
                    const OUString* pLayoutName = pOldDim->GetLayoutName();
                    if (pLayoutName)
                        pDim->SetLayoutName(*pLayoutName);

                    const OUString* pSubtotalName = pOldDim->GetSubtotalName();
                    if (pSubtotalName)
                        pDim->SetSubtotalName(*pSubtotalName);
                }
            }

            bool bManualSort = ( aIt->maSortInfo.Mode == sheet::DataPilotFieldSortMode::MANUAL );

            // visibility of members
            for (vector<ScDPLabelData::Member>::const_iterator itr = aIt->maMembers.begin(), itrEnd = aIt->maMembers.end();
                  itr != itrEnd; ++itr)
            {
                ScDPSaveMember* pMember = pDim->GetMemberByName(itr->maName);

                // #i40054# create/access members only if flags are not default
                // (or in manual sorting mode - to keep the order)
                if (bManualSort || !itr->mbVisible || !itr->mbShowDetails)
                {
                    pMember->SetIsVisible(itr->mbVisible);
                    pMember->SetShowDetails(itr->mbShowDetails);
                }
                if (pOldDim)
                {
                    // Transfer the existing layout name.
                    ScDPSaveMember* pOldMember = pOldDim->GetMemberByName(itr->maName);
                    if (pOldMember)
                    {
                        const OUString* pLayoutName = pOldMember->GetLayoutName();
                        if (pLayoutName)
                            pMember->SetLayoutName(*pLayoutName);
                    }
                }
            }
        }
    }
    ScDPSaveDimension* pDim = aSaveData.GetDataLayoutDimension();
    if (pDim && pOldSaveData)
    {
        ScDPSaveDimension* pOldDim = pOldSaveData->GetDataLayoutDimension();
        if (pOldDim)
        {
            const OUString* pLayoutName = pOldDim->GetLayoutName();
            if (pLayoutName)
                pDim->SetLayoutName(*pLayoutName);
        }
    }

    USHORT nWhichPivot = SC_MOD()->GetPool().GetWhich( SID_PIVOT_TABLE );
    ScPivotItem aOutItem( nWhichPivot, &aSaveData, &aOutRange, bToNewTable );

    bRefInputMode = FALSE;      // to allow deselecting when switching sheets

    SetDispatcherLock( FALSE );
    SwitchToDocument();

    ScTabViewShell* pTabViewShell = pViewData->GetViewShell();
    pTabViewShell->SetDialogDPObject(xDlgDPObject.get());

    //  don't hide the dialog before executing the slot, instead it is used as
    //  parent for message boxes in ScTabViewShell::GetDialogParent

    const SfxPoolItem* pRet = GetBindings().GetDispatcher()->Execute(
        SID_PIVOT_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aOutItem, 0L, 0L );

    bool bSuccess = true;
    if (pRet)
    {
        const SfxBoolItem* pItem = dynamic_cast<const SfxBoolItem*>(pRet);
        if (pItem)
            bSuccess = pItem->GetValue();
    }
    if (bSuccess)
        // Table successfully inserted.
        Close();
    else
    {
        // Table insertion failed.  Keep the dialog open.
        bRefInputMode = true;
        SetDispatcherLock(true);
    }

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, CancelHdl, CancelButton *, EMPTYARG )
{
    Close();
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, MoreClickHdl, MoreButton *, EMPTYARG )
{
    if ( aBtnMore.GetState() )
    {
        bRefInputMode = TRUE;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        if ( aEdInPos.IsEnabled() )
        {
            aEdInPos.Enable();
            aEdInPos.GrabFocus();
            aEdInPos.Enable();
        }
        else
        {
        aEdOutPos.Enable();
        aEdOutPos.GrabFocus();
            aEdOutPos.Enable();
        }
    }
    else
    {
        bRefInputMode = FALSE;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
    }
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, EdModifyHdl, Edit *, EMPTYARG )
{
    String  theCurPosStr = aEdOutPos.GetText();
    USHORT  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

    if ( SCA_VALID == (nResult & SCA_VALID) )
    {
        String* pStr    = NULL;
        BOOL    bFound  = FALSE;
        USHORT  i       = 0;
        USHORT  nCount  = aLbOutPos.GetEntryCount();

        for ( i=2; i<nCount && !bFound; i++ )
        {
            pStr = (String*)aLbOutPos.GetEntryData( i );
            bFound = (theCurPosStr == *pStr);
        }

        if ( bFound )
            aLbOutPos.SelectEntryPos( --i );
        else
            aLbOutPos.SelectEntryPos( 0 );
    }
    return 0;
}

IMPL_LINK( ScDPLayoutDlg, EdInModifyHdl, Edit *, EMPTYARG )
{
    UpdateSrcRange();
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, SelAreaHdl, ListBox *, EMPTYARG )
{
    String  aString;
    USHORT  nSelPos = aLbOutPos.GetSelectEntryPos();

    if ( nSelPos > 1 )
    {
        aString = *(String*)aLbOutPos.GetEntryData( nSelPos );
    }
    else if ( nSelPos == aLbOutPos.GetEntryCount()-1 ) // auf neue Tabelle?
    {
        aEdOutPos.Disable();
        aRbOutPos.Disable();
    }
    else
    {
        aEdOutPos.Enable();
        aRbOutPos.Enable();
    }

    aEdOutPos.SetText( aString );
    return 0;
}

IMPL_LINK( ScDPLayoutDlg, GetFocusHdl, Control*, pCtrl )
{
    pEditActive = NULL;
    if ( pCtrl == &aEdInPos )
        pEditActive = &aEdInPos;
    else if ( pCtrl == &aEdOutPos )
        pEditActive = &aEdOutPos;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
