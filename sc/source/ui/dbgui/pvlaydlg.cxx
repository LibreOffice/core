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
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 ************************************************************************/

#include <config_lgpl.h>

#include "pvlaydlg.hxx"
#include "dpuiglobal.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include "dbdocfun.hxx"
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

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::for_each;

namespace {

const sal_uInt16 STD_FORMAT = sal_uInt16( SCA_VALID | SCA_TAB_3D | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE );

Point DlgPos2WndPos( const Point& rPt, const Window& rWnd )
{
    Point aWndPt( rPt );
    aWndPt.X() = rPt.X()-rWnd.GetPosPixel().X();
    aWndPt.Y() = rPt.Y()-rWnd.GetPosPixel().Y();

    return aWndPt;
}

} // namespace

ScPivotLayoutDlg::ScPivotLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, const ScDPObject& rDPObject, bool bNewOutput ) :
    ScAnyRefDlg( pB, pCW, pParent, RID_SCDLG_PIVOT_LAYOUT ),

    maFlLayout( this, ScResId( FL_LAYOUT ) ),
    maFtPage( this, ScResId( FT_PAGE ) ),
    maWndPage( this, ScResId( WND_PAGE ), &maFtPage, HID_SC_DPLAY_PAGE ),
    maFtCol( this, ScResId( FT_COL ) ),
    maWndCol( this, ScResId( WND_COL ), &maFtCol, HID_SC_DPLAY_COLUMN ),
    maFtRow( this, ScResId( FT_ROW ) ),
    maWndRow( this, ScResId( WND_ROW ), &maFtRow, HID_SC_DPLAY_ROW ),
    maFtData( this, ScResId( FT_DATA ) ),
    maWndData( this, ScResId( WND_DATA ), &maFtData, HID_SC_DPLAY_DATA ),
    maWndSelect( this, ScResId( WND_SELECT ), NULL, HID_SC_DPLAY_SELECT ),
    maFtInfo( this, ScResId( FT_INFO ) ),

    maFlAreas( this, ScResId( FL_OUTPUT ) ),
    maFtInArea( this, ScResId( FT_INAREA) ),
    maEdInPos( this, this, &maFtInArea, ScResId( ED_INAREA) ),
    maRbInPos( this, ScResId( RB_INAREA ), &maEdInPos, this ),
    maLbOutPos( this, ScResId( LB_OUTAREA ) ),
    maFtOutArea( this, ScResId( FT_OUTAREA ) ),
    maEdOutPos( this, this, &maFtOutArea, ScResId( ED_OUTAREA ) ),
    maRbOutPos( this, ScResId( RB_OUTAREA ), &maEdOutPos, this ),
    maBtnIgnEmptyRows( this, ScResId( BTN_IGNEMPTYROWS ) ),
    maBtnDetectCat( this, ScResId( BTN_DETECTCAT ) ),
    maBtnTotalCol( this, ScResId( BTN_TOTALCOL ) ),
    maBtnTotalRow( this, ScResId( BTN_TOTALROW ) ),
    maBtnFilter( this, ScResId( BTN_FILTER ) ),
    maBtnDrillDown( this, ScResId( BTN_DRILLDOWN ) ),

    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp( this, ScResId( BTN_HELP ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOptions( this, ScResId( BTN_OPTIONS ) ),
    maBtnMore( this, ScResId( BTN_MORE ) ),

    mxDlgDPObject( new ScDPObject( rDPObject ) ),
    mpViewData( ((ScTabViewShell*)SfxViewShell::Current())->GetViewData() ),
    mpDoc( ((ScTabViewShell*)SfxViewShell::Current())->GetViewData()->GetDocument() ),
    mpRefInputEdit(NULL),

    maStrUndefined(SC_RESSTR(SCSTR_UNDEFINED)),
    maStrNewTable(SC_RESSTR(SCSTR_NEWTABLE)),
    mbIsDrag(false),
    meLastActiveType(PIVOTFIELDTYPE_SELECT),
    mnOffset(0),
    mbRefInputMode( false )
{
    mxDlgDPObject->FillOldParam( maPivotData );
    mxDlgDPObject->FillLabelData( maPivotData );

    maBtnRemove.SetClickHdl( LINK( this, ScPivotLayoutDlg, ClickHdl ) );
    maBtnOptions.SetClickHdl( LINK( this, ScPivotLayoutDlg, ClickHdl ) );

    maFuncNames.reserve( PIVOT_MAXFUNC );
    for ( sal_uInt16 i = 1; i <= PIVOT_MAXFUNC; ++i )
        maFuncNames.push_back(ScResId(i).toString());

    maBtnMore.AddWindow( &maFlAreas );
    maBtnMore.AddWindow( &maFtInArea );
    maBtnMore.AddWindow( &maEdInPos );
    maBtnMore.AddWindow( &maRbInPos );
    maBtnMore.AddWindow( &maFtOutArea );
    maBtnMore.AddWindow( &maLbOutPos );
    maBtnMore.AddWindow( &maEdOutPos );
    maBtnMore.AddWindow( &maRbOutPos );
    maBtnMore.AddWindow( &maBtnIgnEmptyRows );
    maBtnMore.AddWindow( &maBtnDetectCat );
    maBtnMore.AddWindow( &maBtnTotalCol );
    maBtnMore.AddWindow( &maBtnTotalRow );
    maBtnMore.AddWindow( &maBtnFilter );
    maBtnMore.AddWindow( &maBtnDrillDown );
    maBtnMore.SetClickHdl( LINK( this, ScPivotLayoutDlg, MoreClickHdl ) );

    maFieldCtrls.reserve(5);
    maFieldCtrls.push_back(&maWndPage);
    maFieldCtrls.push_back(&maWndCol);
    maFieldCtrls.push_back(&maWndRow);
    maFieldCtrls.push_back(&maWndData);
    maFieldCtrls.push_back(&maWndSelect);

    InitControlAndDlgSizes();

    if (mxDlgDPObject->GetSheetDesc())
    {
        maEdInPos.Enable();
        maRbInPos.Enable();
        const ScSheetSourceDesc* p = mxDlgDPObject->GetSheetDesc();
        OUString aRangeName = p->GetRangeName();
        if (!aRangeName.isEmpty())
            maEdInPos.SetText(aRangeName);
        else
        {
            maOldRange = p->GetSourceRange();
            OUString aStr(maOldRange.Format(SCR_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
            maEdInPos.SetText(aStr);
        }
    }
    else
    {
        // data is not reachable, so could be a remote database
        maEdInPos.Disable();
        maRbInPos.Disable();
    }

    InitFieldWindows();

    maLbOutPos.SetSelectHdl( LINK( this, ScPivotLayoutDlg, SelAreaHdl ) );
    maEdOutPos.SetModifyHdl( LINK( this, ScPivotLayoutDlg, EdOutModifyHdl ) );
    maEdInPos.SetModifyHdl( LINK( this, ScPivotLayoutDlg, EdInModifyHdl ) );
    maBtnOk.SetClickHdl( LINK( this, ScPivotLayoutDlg, OkHdl ) );
    maBtnCancel.SetClickHdl( LINK( this, ScPivotLayoutDlg, CancelHdl ) );

    // Set focus handler for the reference edit text boxes.
    Link aGetFocusLink = LINK(this, ScPivotLayoutDlg, GetRefEditFocusHdl);
    if (maEdInPos.IsEnabled())
        maEdInPos.SetGetFocusHdl(aGetFocusLink);
    maEdOutPos.SetGetFocusHdl(aGetFocusLink);

    if ( mpViewData && mpDoc )
    {
        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * in einem Zeiger-Array gemerkt, bei denen es sich
         * um sinnvolle Bereiche handelt
         */

        maLbOutPos.Clear();
        maLbOutPos.InsertEntry( maStrUndefined, 0 );
        maLbOutPos.InsertEntry( maStrNewTable,  1 );

        ScAreaNameIterator aIter( mpDoc );
        OUString aName;
        ScRange aRange;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                sal_uInt16 nInsert = maLbOutPos.InsertEntry( aName );

                OUString aRefStr(aRange.aStart.Format(SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
                maRefStrs.push_back(new OUString(aRefStr));
                maLbOutPos.SetEntryData(nInsert, &maRefStrs.back());
            }
        }
    }

    if (bNewOutput)
    {
        // Output to a new sheet by default for a brand-new output.
        maLbOutPos.SelectEntryPos(1);
        maEdOutPos.Disable();
        maRbOutPos.Disable();
    }
    else
    {
        // Modifying an existing dp output.

        if ( maPivotData.nTab != MAXTAB+1 )
        {
            OUString aStr =
                ScAddress( maPivotData.nCol,
                           maPivotData.nRow,
                           maPivotData.nTab ).Format(STD_FORMAT, mpDoc, mpDoc->GetAddressConvention());
            maEdOutPos.SetText( aStr );
            maOutputRefStr = aStr;
            EdOutModifyHdl(0);
        }
        else
        {
            maLbOutPos.SelectEntryPos( maLbOutPos.GetEntryCount()-1 );
            SelAreaHdl(NULL);
        }
    }

    maBtnIgnEmptyRows.Check( maPivotData.bIgnoreEmptyRows );
    maBtnDetectCat   .Check( maPivotData.bDetectCategories );
    maBtnTotalCol    .Check( maPivotData.bMakeTotalCol );
    maBtnTotalRow    .Check( maPivotData.bMakeTotalRow );

    const ScDPSaveData* pSaveData = mxDlgDPObject->GetSaveData();
    maBtnFilter.Check( !pSaveData || pSaveData->GetFilterButton() );
    maBtnDrillDown.Check( !pSaveData || pSaveData->GetDrillDown() );

    GrabFieldFocus( maWndSelect );

    FreeResource();
}

ScPivotLayoutDlg::~ScPivotLayoutDlg()
{
}

sal_Bool ScPivotLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}

void ScPivotLayoutDlg::InitWndSelect(const ScDPLabelDataVector& rLabels)
{
    size_t nLabelCount = rLabels.size();

    maLabelData.clear();
    maLabelData.reserve( nLabelCount );
    for ( size_t i=0; i < nLabelCount; i++ )
    {
        const ScDPLabelData& r = rLabels[i];

        maLabelData.push_back(new ScDPLabelData(r));
        if (r.mnOriginalDim < 0 && !r.mbDataLayout)
        {
            // For dimension with duplicates, use the layout name of the
            // original dimension if available.  Be aware that duplicate
            // dimensions may have different layout names.
            ScPivotFuncData aFunc(maLabelData[i].mnCol, maLabelData[i].mnFuncMask);
            maWndSelect.AppendField(maLabelData[i].getDisplayName(), aFunc);
        }
    }
    maWndSelect.ResetScrollBar();
    maWndSelect.Paint(Rectangle());
}

void ScPivotLayoutDlg::InitWndData(const vector<ScPivotField>& rFields)
{
    vector<ScPivotField>::const_iterator it = rFields.begin(), itEnd = rFields.end();
    for (; it != itEnd; ++it)
    {
        SCCOL nCol = it->nCol;
        sal_uInt16 nMask = it->nFuncMask;
        if (nCol == PIVOT_DATA_FIELD)
            continue;

        // data field - we need to concatenate function name with the field name.
        ScDPLabelData* pData = GetLabelData(nCol);
        OSL_ENSURE( pData, "ScDPLabelData not found" );
        if (pData)
        {
            ScPivotFuncData aFunc(nCol, it->mnOriginalDim, nMask, it->mnDupCount, it->maFieldRef);
            OUString aStr = pData->maLayoutName;
            if (aStr.isEmpty())
            {
                aStr = GetFuncString(aFunc.mnFuncMask, pData->mbIsValue);
                aStr += pData->maName;
            }

            maWndData.AppendField(aStr, aFunc);
            pData->mnFuncMask = nMask;
        }
    }
    maWndData.ResetScrollBar();
}

void ScPivotLayoutDlg::InitFieldWindow( const vector<ScPivotField>& rFields, ScPivotFieldType eType )
{
    OSL_ASSERT(eType != PIVOTFIELDTYPE_DATA);
    ScDPFieldControlBase* pInitWnd = GetFieldWindow(eType);

    if (!pInitWnd)
        return;

    vector<ScPivotField>::const_iterator itr = rFields.begin(), itrEnd = rFields.end();
    for (; itr != itrEnd; ++itr)
    {
        SCCOL nCol = itr->nCol;
        sal_uInt16 nMask = itr->nFuncMask;
        if (nCol == PIVOT_DATA_FIELD)
            continue;

        ScPivotFuncData aFunc(nCol, itr->mnOriginalDim, nMask, itr->mnDupCount, itr->maFieldRef);
        pInitWnd->AppendField(GetLabelString(nCol), aFunc);
    }
    pInitWnd->ResetScrollBar();
}

void ScPivotLayoutDlg::InitFieldWindows()
{
    InitWndSelect(maPivotData.maLabelArray);
    InitFieldWindow(maPivotData.maPageFields, PIVOTFIELDTYPE_PAGE);
    InitFieldWindow(maPivotData.maColFields, PIVOTFIELDTYPE_COL);
    InitFieldWindow(maPivotData.maRowFields, PIVOTFIELDTYPE_ROW);
    InitWndData(maPivotData.maDataFields);
}

void ScPivotLayoutDlg::GrabFieldFocus( ScDPFieldControlBase& rFieldWindow )
{
    if( rFieldWindow.IsEmpty() )
    {
        if( maWndSelect.IsEmpty() )
            maBtnOk.GrabFocus();
        else
            maWndSelect.GrabFocus();
    }
    else
        rFieldWindow.GrabFocus();
}

void ScPivotLayoutDlg::AddField( size_t nFromIndex, ScPivotFieldType eToType, const Point& rAtPos )
{
    ScPivotFuncData aFunc = maWndSelect.GetFuncData(nFromIndex); // local copy

    bool bAllowed = IsOrientationAllowed(aFunc.mnCol, eToType);
    if (!bAllowed)
        return;

    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    if (eToType == PIVOTFIELDTYPE_DATA)
    {
        // Data field allows duplicates.
        ScDPLabelData* p = GetLabelData(aFunc.mnCol);
        OUString aStr = p->maLayoutName;
        sal_uInt16 nMask = aFunc.mnFuncMask;
        if (nMask == PIVOT_FUNC_NONE)
            nMask = PIVOT_FUNC_SUM; // Use SUM by default.
        if (aStr.isEmpty())
        {
            aStr = GetFuncString(nMask);
            aStr += p->maName;
        }

        aFunc.mnFuncMask = nMask;
        size_t nAddedAt = toWnd->AddField(aStr, DlgPos2WndPos(rAtPos, *toWnd), aFunc);
        if (nAddedAt != PIVOTFIELD_INVALID)
            toWnd->GrabFocus();

        return;
    }

    nAt = toWnd->GetFieldIndexByData(aFunc);
    if (nAt == PIVOTFIELD_INVALID)
    {
        if (rmWnd1)
        {
            nAt = rmWnd1->GetFieldIndexByData(aFunc);
            if (nAt != PIVOTFIELD_INVALID)
                rmWnd1->DeleteFieldByIndex(nAt);
        }
        if (rmWnd2)
        {
            nAt = rmWnd2->GetFieldIndexByData(aFunc);
            if (nAt != PIVOTFIELD_INVALID)
                rmWnd2->DeleteFieldByIndex(nAt);
        }

        const ScDPLabelData& rData = maLabelData[nFromIndex+mnOffset];
        size_t nAddedAt = toWnd->AddField(rData.getDisplayName(), DlgPos2WndPos(rAtPos, *toWnd), aFunc);
        if (nAddedAt != PIVOTFIELD_INVALID)
            toWnd->GrabFocus();
    }
}

void ScPivotLayoutDlg::AppendField(size_t nFromIndex, ScPivotFieldType eToType)
{
    ScPivotFuncData aFunc = maWndSelect.GetFuncData(nFromIndex); // local copy

    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

    nAt = toWnd->GetFieldIndexByData(aFunc);
    if (nAt == PIVOTFIELD_INVALID)
    {
        if (rmWnd1)
        {
            nAt = rmWnd1->GetFieldIndexByData(aFunc);
            if (nAt != PIVOTFIELD_INVALID)
                rmWnd1->DeleteFieldByIndex(nAt);
        }
        if (rmWnd2)
        {
            nAt = rmWnd2->GetFieldIndexByData(aFunc);
            if (nAt != PIVOTFIELD_INVALID)
                rmWnd2->DeleteFieldByIndex(nAt);
        }

        ScDPLabelData&  rData = maLabelData[nFromIndex+mnOffset];

        if ( !bDataArr )
        {
            toWnd->AppendField(rData.getDisplayName(), aFunc);
            toWnd->GrabFocus();
        }
        else
        {
            ScDPLabelData* p = GetLabelData(aFunc.mnCol);
            OUString aStr = p->maLayoutName;
            sal_uInt16 nMask = aFunc.mnFuncMask;
            if (aStr.isEmpty())
            {
                aStr = GetFuncString(nMask);
                aStr += p->maName;
            }

            aFunc.mnFuncMask = nMask;
            toWnd->AppendField(aStr, aFunc);
            toWnd->GrabFocus();
        }
    }
}

void ScPivotLayoutDlg::MoveField( ScPivotFieldType eFromType, size_t nFromIndex, ScPivotFieldType eToType, const Point& rAtPos )
{
    if ( eFromType == PIVOTFIELDTYPE_SELECT )
        AddField( nFromIndex, eToType, rAtPos );
    else if (eFromType != PIVOTFIELDTYPE_SELECT && eToType == PIVOTFIELDTYPE_SELECT)
        RemoveField(eFromType, nFromIndex);
    else if ( eFromType != eToType )
    {
        ScDPFieldControlBase* fromWnd  = GetFieldWindow(eFromType);
        ScDPFieldControlBase* toWnd    = GetFieldWindow(eToType);

        ScDPFieldControlBase* rmWnd1   = NULL;
        ScDPFieldControlBase* rmWnd2   = NULL;
        GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

        bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

        if (fromWnd && toWnd)
        {
            ScPivotFuncData aFunc = fromWnd->GetFuncData(nFromIndex); // local copy
            bool bAllowed = IsOrientationAllowed(aFunc.mnCol, eToType);

            size_t nAt = fromWnd->GetFieldIndexByData(aFunc);
            if (bAllowed && nAt != PIVOTFIELD_INVALID)
            {
                fromWnd->DeleteFieldByIndex(nAt);

                nAt = toWnd->GetFieldIndexByData(aFunc);
                if (nAt == PIVOTFIELD_INVALID)
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        if (rmWnd1)
                        {
                            nAt = rmWnd1->GetFieldIndexByData(aFunc);
                            if (nAt != PIVOTFIELD_INVALID)
                                rmWnd1->DeleteFieldByIndex(nAt);
                        }
                        if (rmWnd2)
                        {
                            nAt = rmWnd2->GetFieldIndexByData(aFunc);
                            if (nAt != PIVOTFIELD_INVALID)
                                rmWnd2->DeleteFieldByIndex(nAt);
                        }

                        nAddedAt = toWnd->AddField(
                            GetLabelString(aFunc.mnCol), DlgPos2WndPos(rAtPos, *toWnd), aFunc);
                        if (nAddedAt != PIVOTFIELD_INVALID)
                            toWnd->GrabFocus();
                    }
                    else
                    {
                        ScDPLabelData* p = GetLabelData(aFunc.mnCol);
                        OUString aStr = p->maLayoutName;
                        sal_uInt16 nMask = aFunc.mnFuncMask;
                        if (aStr.isEmpty())
                        {
                            aStr = GetFuncString(nMask);
                            aStr += p->maName;
                        }

                        aFunc.mnFuncMask = nMask;
                        nAddedAt = toWnd->AddField(aStr, DlgPos2WndPos(rAtPos, *toWnd), aFunc);
                        if (nAddedAt != PIVOTFIELD_INVALID)
                            toWnd->GrabFocus();
                    }
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldControlBase* pWnd  = GetFieldWindow(eFromType);
        if (!pWnd)
            return;

        const ScPivotFuncData& rFunc = pWnd->GetFuncData(nFromIndex);

        size_t nAt = pWnd->GetFieldIndexByData(rFunc);
        if (nAt != PIVOTFIELD_INVALID)
        {
            Point aToPos = DlgPos2WndPos( rAtPos, *pWnd );
            size_t nToIndex = 0;
            pWnd->GetExistingIndex(aToPos, nToIndex);

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;
                pWnd->MoveField(nAt, aToPos, nAddedAt);
            }
        }
    }
}

void ScPivotLayoutDlg::MoveFieldToEnd( ScPivotFieldType eFromType, size_t nFromIndex, ScPivotFieldType eToType )
{
    if ( eFromType == PIVOTFIELDTYPE_SELECT )
        AppendField( nFromIndex, eToType );
    else if ( eFromType != eToType )
    {
        ScDPFieldControlBase* fromWnd  = GetFieldWindow(eFromType);
        ScDPFieldControlBase* toWnd    = GetFieldWindow(eToType);

        ScDPFieldControlBase* rmWnd1   = NULL;
        ScDPFieldControlBase* rmWnd2   = NULL;
        GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

        bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

        if (!fromWnd || !toWnd)
            return;

        ScPivotFuncData aFunc = fromWnd->GetFuncData(nFromIndex); // local copy

        size_t nAt = fromWnd->GetFieldIndexByData(aFunc);
        if (nAt != PIVOTFIELD_INVALID)
        {
            fromWnd->DeleteFieldByIndex(nAt);

            nAt = toWnd->GetFieldIndexByData(aFunc);
            if (nAt == PIVOTFIELD_INVALID)
            {
                if ( !bDataArr )
                {
                    if (rmWnd1)
                    {
                        nAt = rmWnd1->GetFieldIndexByData(aFunc);
                        if (nAt != PIVOTFIELD_INVALID)
                            rmWnd1->DeleteFieldByIndex(nAt);
                    }
                    if (rmWnd2)
                    {
                        nAt = rmWnd2->GetFieldIndexByData(aFunc);
                        if (nAt != PIVOTFIELD_INVALID)
                            rmWnd2->DeleteFieldByIndex(nAt);
                    }

                    toWnd->AppendField(GetLabelString(aFunc.mnCol), aFunc);
                    toWnd->GrabFocus();
                }
                else
                {
                    ScDPLabelData* p = GetLabelData(aFunc.mnCol);
                    OUString aStr = p->maLayoutName;
                    sal_uInt16 nMask = aFunc.mnFuncMask;
                    if (aStr.isEmpty())
                    {
                        aStr = GetFuncString(nMask);
                        aStr += p->maName;
                    }

                    aFunc.mnFuncMask = nMask;
                    toWnd->AppendField(aStr, aFunc);
                    toWnd->GrabFocus();
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldControlBase* pWnd  = GetFieldWindow(eFromType);
        if (!pWnd)
            return;

        Point aToPos;
        bool bDataArr = eFromType == PIVOTFIELDTYPE_DATA;

        ScPivotFuncData aFunc = pWnd->GetFuncData(nFromIndex); // local copy
        size_t nAt = pWnd->GetFieldIndexByData(aFunc);
        if (nAt != PIVOTFIELD_INVALID)
        {
            size_t nToIndex = 0;
            pWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                pWnd->DeleteFieldByIndex(nAt);

                if ( !bDataArr )
                    pWnd->AppendField(GetLabelString(aFunc.mnCol), aFunc);
                else
                {
                    ScDPLabelData* p = GetLabelData(aFunc.mnCol);
                    OUString aStr = p->maLayoutName;
                    sal_uInt16 nMask = aFunc.mnFuncMask;
                    if (aStr.isEmpty())
                    {
                        aStr = GetFuncString(nMask);
                        aStr += p->maName;
                    }

                    aFunc.mnFuncMask = nMask;
                    pWnd->AppendField(aStr, aFunc);
                }
            }
        }
    }
}

void ScPivotLayoutDlg::RemoveField( ScPivotFieldType eFromType, size_t nIndex )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow(eFromType);
    if (!pWnd)
        return;

    if (nIndex >= pWnd->GetFieldCount())
        // out of bound
        return;

    pWnd->DeleteFieldByIndex(nIndex);
    if (pWnd->IsEmpty())
        GrabFieldFocus(maWndSelect);
}

PointerStyle ScPivotLayoutDlg::NotifyMouseButtonDown( ScPivotFieldType eType, size_t nFieldIndex )
{
    mbIsDrag       = true;
    meDnDFromType  = eType;
    mnDnDFromIndex = nFieldIndex;

    switch (eType)
    {
        case PIVOTFIELDTYPE_PAGE:
            return POINTER_PIVOT_FIELD;
        case PIVOTFIELDTYPE_COL:
            return POINTER_PIVOT_COL;
        case PIVOTFIELDTYPE_ROW:
            return POINTER_PIVOT_ROW;
        case PIVOTFIELDTYPE_DATA:
            return POINTER_PIVOT_FIELD;
        case PIVOTFIELDTYPE_SELECT:
            return POINTER_PIVOT_FIELD;
        default:
            ;
    }
    return POINTER_ARROW;
}

void ScPivotLayoutDlg::NotifyDoubleClick( ScPivotFieldType eType, size_t nFieldIndex )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow(eType);
    if (!pWnd)
        return;

    if (nFieldIndex >= pWnd->GetFieldCount())
    {
        OSL_FAIL("invalid selection");
        return;
    }

    ScPivotFuncData& rFunc = pWnd->GetFuncData(nFieldIndex);
    ScDPLabelData* pData = GetLabelData(rFunc.mnCol);
    if (!pData)
        return;

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    switch ( eType )
    {
        case PIVOTFIELDTYPE_PAGE:
        case PIVOTFIELDTYPE_COL:
        case PIVOTFIELDTYPE_ROW:
        {
            // list of names of all data fields
            vector<ScDPName> aDataFieldNames;
            vector<ScDPFieldControlBase::FuncItem> aFuncItems;
            maWndData.GetAllFuncItems(aFuncItems);
            vector<ScDPFieldControlBase::FuncItem>::const_iterator it = aFuncItems.begin(), itEnd = aFuncItems.end();
            for (; it != itEnd; ++it)
            {
                ScDPLabelData* pDFData = GetLabelData(it->mnCol);
                if (!pDFData)
                    continue;

                if (pDFData->maName.isEmpty())
                    continue;

                OUString aLayoutName = pDFData->maLayoutName;
                if (aLayoutName.isEmpty())
                {
                    // No layout name exists.  Use the stock name.
                    sal_uInt16 nMask = it->mnFuncMask;
                    OUString aFuncStr = GetFuncString(nMask);
                    aLayoutName = aFuncStr + pDFData->maName;
                }
                aDataFieldNames.push_back(ScDPName(pDFData->maName, aLayoutName));
            }

            bool bLayout = (eType == PIVOTFIELDTYPE_ROW) &&
                ((aDataFieldNames.size() > 1) || (nFieldIndex + 1 < pWnd->GetFieldCount()));

            boost::scoped_ptr<AbstractScDPSubtotalDlg> pDlg(
                pFact->CreateScDPSubtotalDlg(
                    this, RID_SCDLG_PIVOTSUBT, *mxDlgDPObject, *pData, rFunc,
                    aDataFieldNames, bLayout));

            if ( pDlg->Execute() == RET_OK )
            {
                pDlg->FillLabelData( *pData );
                rFunc.mnFuncMask = pData->mnFuncMask;
            }
        }
        break;

        case PIVOTFIELDTYPE_DATA:
        {
            ScPivotFuncData& rFuncData = maWndData.GetFuncData(nFieldIndex);
            boost::scoped_ptr<AbstractScDPFunctionDlg> pDlg(
                pFact->CreateScDPFunctionDlg(
                    this, RID_SCDLG_DPDATAFIELD, maLabelData, *pData, rFuncData));

            if ( pDlg->Execute() == RET_OK )
            {
                bool bFuncChanged = rFuncData.mnFuncMask != pDlg->GetFuncMask();
                rFuncData.mnFuncMask = pData->mnFuncMask = pDlg->GetFuncMask();
                rFuncData.maFieldRef = pDlg->GetFieldRef();

                if (bFuncChanged)
                    // Get the new duplicate count since the function has changed.
                    rFuncData.mnDupCount = maWndData.GetNextDupCount(rFuncData, nFieldIndex);

                ScDPLabelData* p = GetLabelData(rFuncData.mnCol);
                OUString aStr = p->maLayoutName;
                if (aStr.isEmpty())
                {
                    // Layout name is not available.  Use default name.
                    aStr = GetFuncString (rFuncData.mnFuncMask);
                    aStr += p->maName;
                }
                maWndData.SetFieldText(aStr, nFieldIndex, rFuncData.mnDupCount);
            }
        }
        break;

        default:
        {
            // added to avoid warnings
        }
    }
}

void ScPivotLayoutDlg::NotifyFieldFocus( ScPivotFieldType eType, bool bGotFocus )
{
    /*  Enable Remove/Options buttons on GetFocus in field window.
        Enable them also, if dialog is deactivated (click into document).
        The !IsActive() condition handles the case that a LoseFocus event of a
        field window would follow the Deactivate event of this dialog. */
    bool bEnable = (bGotFocus || !IsActive()) && (eType != PIVOTFIELDTYPE_SELECT);

    // The TestTool may set the focus into an empty field.
    // Then the Remove/Options buttons must be disabled.
    ScDPFieldControlBase* pWnd = GetFieldWindow(eType);
    if ( bEnable && bGotFocus && pWnd && pWnd->IsEmpty() )
        bEnable = false;

    maBtnRemove.Enable( bEnable );
    maBtnOptions.Enable( bEnable );
    if( bGotFocus )
        meLastActiveType = eType;
}

void ScPivotLayoutDlg::NotifyMoveFieldToEnd( ScPivotFieldType eToType )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow(meLastActiveType);
    ScDPFieldControlBase* pToWnd = GetFieldWindow(eToType);
    if (pWnd && pToWnd && (eToType != PIVOTFIELDTYPE_SELECT) && !pWnd->IsEmpty())
    {
        MoveFieldToEnd(meLastActiveType, pWnd->GetSelectedField(), eToType);

        if( pWnd->IsEmpty() )
            NotifyFieldFocus( eToType, true );
        else
            pWnd->GrabFocus();
        if( meLastActiveType == PIVOTFIELDTYPE_SELECT )
            maWndSelect.SelectNext();
    }
    else
        GrabFieldFocus( maWndSelect );
}

void ScPivotLayoutDlg::NotifyRemoveField( ScPivotFieldType eType, size_t nFieldIndex )
{
    if( eType != PIVOTFIELDTYPE_SELECT )
        RemoveField( eType, nFieldIndex );
}

Size ScPivotLayoutDlg::GetStdFieldBtnSize() const
{
    // This size is static but is platform dependent.  The field button size
    // is calculated relative to the size of the OK button.
    double w = static_cast<double>(maBtnOk.GetSizePixel().Width()) * 0.70;
    return Size(static_cast<long>(w), FIELD_BTN_HEIGHT);
}

void ScPivotLayoutDlg::DropFieldItem( const Point& rScrPos, ScPivotFieldType eToType )
{
    if (!mbIsDrag)
        // Not in drag mode. Do nothing.
        return;

    mbIsDrag = false;

    if (eToType == PIVOTFIELDTYPE_UNKNOWN)
    {
        // We don't remove any buttons from the select field.
        if (meDnDFromType != PIVOTFIELDTYPE_SELECT)
            RemoveField(meDnDFromType, mnDnDFromIndex);
    }
    else
    {
        Point aOutPos = ScreenToOutputPixel(rScrPos);
        MoveField(meDnDFromType, mnDnDFromIndex, eToType, aOutPos);
    }
}

PointerStyle ScPivotLayoutDlg::GetPointerStyleAtPoint( const Point& /* rScrPos */, ScPivotFieldType eFieldType )
{
    if (!mbIsDrag)
        return POINTER_ARROW;

    if (eFieldType == PIVOTFIELDTYPE_UNKNOWN)
        // Outside any field areas.
        return meDnDFromType == PIVOTFIELDTYPE_SELECT ? POINTER_PIVOT_FIELD : POINTER_PIVOT_DELETE;

    if (eFieldType == PIVOTFIELDTYPE_SELECT)
        return POINTER_PIVOT_FIELD;

    // check if the target orientation is allowed for this field
    ScDPFieldControlBase* pWnd = GetFieldWindow(meDnDFromType);
    if (!pWnd)
        return POINTER_ARROW;

    const ScPivotFuncData& rData = pWnd->GetFuncData(mnDnDFromIndex);
    if (!IsOrientationAllowed(rData.mnCol, eFieldType))
        return POINTER_NOTALLOWED;

    switch (eFieldType)
    {
        case PIVOTFIELDTYPE_COL:
            return POINTER_PIVOT_COL;
        case PIVOTFIELDTYPE_ROW:
            return POINTER_PIVOT_ROW;
        case PIVOTFIELDTYPE_DATA:
        case PIVOTFIELDTYPE_SELECT:
        case PIVOTFIELDTYPE_PAGE:
        default:
            ;
    }

    return POINTER_PIVOT_FIELD;
}

namespace {

class InsideFieldControl : std::unary_function<ScDPFieldControlBase*, bool>
{
    Point maOutPos;
public:
    InsideFieldControl(const Point& rOutPos) : maOutPos(rOutPos) {}

    bool operator() (const ScDPFieldControlBase* p) const
    {
        Rectangle aRect(p->GetPosPixel(), p->GetSizePixel());
        return aRect.IsInside(maOutPos);
    }
};

}

ScPivotFieldType ScPivotLayoutDlg::GetFieldTypeAtPoint( const Point& rScrPos ) const
{
    Point aOutputPos = ScreenToOutputPixel(rScrPos);
    std::vector<ScDPFieldControlBase*>::const_iterator it =
        std::find_if(maFieldCtrls.begin(), maFieldCtrls.end(), InsideFieldControl(aOutputPos));

    return it == maFieldCtrls.end() ? PIVOTFIELDTYPE_UNKNOWN : (*it)->GetFieldType();
}

void ScPivotLayoutDlg::Deactivate()
{
    /*  If the dialog has been deactivated (click into document), the LoseFocus
        event from field window disables Remove/Options buttons. Re-enable them here by
        simulating a GetFocus event. Event order of LoseFocus and Deactivate is not important.
        The last event will enable the buttons in both cases (see NotifyFieldFocus). */
    NotifyFieldFocus( meLastActiveType, true );
}

namespace {

class FindLabelDataByCol : std::unary_function<ScDPLabelData, bool>
{
    SCCOL mnCol;
public:
    FindLabelDataByCol(SCCOL nCol) : mnCol(nCol) {}

    bool operator() (const ScDPLabelData& r) const
    {
        return r.mnCol == mnCol;
    }
};

}

ScDPLabelData* ScPivotLayoutDlg::GetLabelData( SCCOL nCol )
{
    ScDPLabelDataVector::iterator it =
        std::find_if(maLabelData.begin(), maLabelData.end(), FindLabelDataByCol(nCol));
    return it == maLabelData.end() ? NULL : &(*it);
}

OUString ScPivotLayoutDlg::GetLabelString( SCsCOL nCol )
{
    ScDPLabelData* pData = GetLabelData( nCol );
    OSL_ENSURE( pData, "LabelData not found" );
    if (pData)
        return pData->getDisplayName();
    return OUString();
}

bool ScPivotLayoutDlg::IsOrientationAllowed( SCsCOL nCol, ScPivotFieldType eType )
{
    bool bAllowed = true;
    ScDPLabelData* pData = GetLabelData( nCol );
    OSL_ENSURE( pData, "LabelData not found" );
    if (pData)
    {
        sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        switch (eType)
        {
            case PIVOTFIELDTYPE_PAGE:   eOrient = sheet::DataPilotFieldOrientation_PAGE;   break;
            case PIVOTFIELDTYPE_COL:    eOrient = sheet::DataPilotFieldOrientation_COLUMN; break;
            case PIVOTFIELDTYPE_ROW:    eOrient = sheet::DataPilotFieldOrientation_ROW;    break;
            case PIVOTFIELDTYPE_DATA:   eOrient = sheet::DataPilotFieldOrientation_DATA;   break;
            case PIVOTFIELDTYPE_SELECT: eOrient = sheet::DataPilotFieldOrientation_HIDDEN; break;
            default:
                ;
        }
        bAllowed = ScDPObject::IsOrientationAllowed( (sal_uInt16)eOrient, pData->mnFlags );
    }
    return bAllowed;
}

OUString ScPivotLayoutDlg::GetFuncString( sal_uInt16& rFuncMask, bool bIsValue )
{
    OUStringBuffer aBuf;

    if (   rFuncMask == PIVOT_FUNC_NONE
        || rFuncMask == PIVOT_FUNC_AUTO )
    {
        if ( bIsValue )
        {
            aBuf.append(GetFuncName(PIVOTSTR_SUM));
            rFuncMask = PIVOT_FUNC_SUM;
        }
        else
        {
            aBuf.append(GetFuncName(PIVOTSTR_COUNT));
            rFuncMask = PIVOT_FUNC_COUNT;
        }
    }
    else if ( rFuncMask == PIVOT_FUNC_SUM )       aBuf = GetFuncName(PIVOTSTR_SUM);
    else if ( rFuncMask == PIVOT_FUNC_COUNT )     aBuf = GetFuncName(PIVOTSTR_COUNT);
    else if ( rFuncMask == PIVOT_FUNC_AVERAGE )   aBuf = GetFuncName(PIVOTSTR_AVG);
    else if ( rFuncMask == PIVOT_FUNC_MAX )       aBuf = GetFuncName(PIVOTSTR_MAX);
    else if ( rFuncMask == PIVOT_FUNC_MIN )       aBuf = GetFuncName(PIVOTSTR_MIN);
    else if ( rFuncMask == PIVOT_FUNC_PRODUCT )   aBuf = GetFuncName(PIVOTSTR_PROD);
    else if ( rFuncMask == PIVOT_FUNC_COUNT_NUM ) aBuf = GetFuncName(PIVOTSTR_COUNT2);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEV )   aBuf = GetFuncName(PIVOTSTR_DEV);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEVP )  aBuf = GetFuncName(PIVOTSTR_DEV2);
    else if ( rFuncMask == PIVOT_FUNC_STD_VAR )   aBuf = GetFuncName(PIVOTSTR_VAR);
    else if ( rFuncMask == PIVOT_FUNC_STD_VARP )  aBuf = GetFuncName(PIVOTSTR_VAR2);
    else
    {
        aBuf.append(ScGlobal::GetRscString(STR_TABLE_ERGEBNIS));
        aBuf.appendAscii(" - ");
    }

    return aBuf.makeStringAndClear();
}

void ScPivotLayoutDlg::InitControlAndDlgSizes()
{
    // The pivot.src file only specifies the positions of the controls. Here,
    // we calculate appropriate size of each control based on how they are
    // positioned relative to each other.

    // row/column/data area sizes
    long nFldW = GetStdFieldBtnSize().Width();
    long nFldH = GetStdFieldBtnSize().Height();

    maWndData.SetSizePixel(
        Size(maWndSelect.GetPosPixel().X() - maWndData.GetPosPixel().X() - FIELD_AREA_GAP*4,
             185));

    maWndPage.SetSizePixel(
        Size(maWndData.GetSizePixel().Width() + 85,
             maWndCol.GetPosPixel().Y() - maWndPage.GetPosPixel().Y() - FIELD_AREA_GAP));
    maWndRow.SetSizePixel(
        Size(maWndData.GetPosPixel().X()-maWndRow.GetPosPixel().X() - FIELD_AREA_GAP,
             maWndData.GetSizePixel().Height()));
    maWndCol.SetSizePixel(
        Size(maWndData.GetPosPixel().X() - maWndCol.GetPosPixel().X() + maWndData.GetSizePixel().Width(),
             maWndData.GetPosPixel().Y() - maWndCol.GetPosPixel().Y() - FIELD_AREA_GAP));

    // #i29203# align right border of page window with data window
    long nDataPosX = maWndData.GetPosPixel().X() + maWndData.GetSizePixel().Width();
    maWndPage.SetPosPixel(
        Point(nDataPosX - maWndPage.GetSizePixel().Width(),
              maWndPage.GetPosPixel().Y()));

    // selection area
    long nLineSize = 10; // number of fields per column.
    long nH = OUTER_MARGIN_VER + nLineSize* nFldH + nLineSize * ROW_FIELD_BTN_GAP;
    nH += ROW_FIELD_BTN_GAP;
    nH += GetSettings().GetStyleSettings().GetScrollBarSize() + OUTER_MARGIN_VER;
    maWndSelect.SetSizePixel(
        Size(2 * nFldW + ROW_FIELD_BTN_GAP + 10, nH));

    maWndPage.CalcSize();
    maWndRow.CalcSize();
    maWndCol.CalcSize();
    maWndData.CalcSize();
    maWndSelect.CalcSize();

    AdjustDlgSize();
}

namespace {

class MoveWndDown : public std::unary_function<Window*, void>
{
    long mnDelta;
public:
    MoveWndDown(long nDelta) : mnDelta(nDelta) {}
    void operator() (Window* p) const
    {
        Point aPos = p->GetPosPixel();
        aPos.Y() += mnDelta;
        p->SetPosPixel(aPos);
    }
};

}

void ScPivotLayoutDlg::AdjustDlgSize()
{
    // On some platforms such as Windows XP, the dialog is not large enough to
    // show the 'Drag the fields from the right...' text at the bottom. Check
    // if it overlaps, and if it does, make the dialog size larger.
    Size aWndSize = GetSizePixel();

    Point aPosText = maFtInfo.GetPosPixel();
    Size aSizeText = maFtInfo.GetSizePixel();
    long nYRef = maWndData.GetPosPixel().Y() + maWndData.GetSizePixel().Height();
    if (aPosText.Y() > nYRef)
        // This text is visible. No need to adjust.
        return;

    // Calculate the extra height necessary.
    long nBottomMargin = aWndSize.Height() - (aPosText.Y() + aSizeText.Height());
    long nHeightNeeded = nYRef + TEXT_INFO_GAP + aSizeText.Height() + nBottomMargin;
    long nDelta = nHeightNeeded - aWndSize.Height();
    if (nDelta <= 0)
        // This should never happen but just in case....
        return;

    // Make the main dialog taller.
    aWndSize.Height() += nDelta;
    SetSizePixel(aWndSize);

    // Move the relevant controls downward.
    std::vector<Window*> aWndToMove;
    aWndToMove.reserve(16);
    aWndToMove.push_back(&maFtInfo);
    aWndToMove.push_back(&maBtnMore);
    aWndToMove.push_back(&maFlAreas);
    aWndToMove.push_back(&maFtInArea);
    aWndToMove.push_back(&maEdInPos);
    aWndToMove.push_back(&maRbInPos);
    aWndToMove.push_back(&maFtOutArea);
    aWndToMove.push_back(&maLbOutPos);
    aWndToMove.push_back(&maEdOutPos);
    aWndToMove.push_back(&maRbOutPos);
    aWndToMove.push_back(&maBtnIgnEmptyRows);
    aWndToMove.push_back(&maBtnDetectCat);
    aWndToMove.push_back(&maBtnTotalCol);
    aWndToMove.push_back(&maBtnTotalRow);
    aWndToMove.push_back(&maBtnFilter);
    aWndToMove.push_back(&maBtnDrillDown);
    std::for_each(aWndToMove.begin(), aWndToMove.end(), MoveWndDown(nDelta));
}

bool ScPivotLayoutDlg::GetPivotArrays(
    vector<ScPivotField>& rPageFields, vector<ScPivotField>& rColFields,
    vector<ScPivotField>& rRowFields, vector<ScPivotField>& rDataFields )
{
    vector<ScPivotField> aPageFields;
    maWndPage.ConvertToPivotArray(aPageFields);

    vector<ScPivotField> aColFields;
    maWndCol.ConvertToPivotArray(aColFields);

    vector<ScPivotField> aRowFields;
    maWndRow.ConvertToPivotArray(aRowFields);

    vector<ScPivotField> aDataFields;
    maWndData.ConvertToPivotArray(aDataFields);

    sheet::DataPilotFieldOrientation eOrientDataLayout = sheet::DataPilotFieldOrientation_ROW;
    ScDPSaveData* pSaveData = mxDlgDPObject->GetSaveData();
    if (pSaveData)
    {
        const ScDPSaveDimension* p = pSaveData->GetExistingDataLayoutDimension();
        if (p)
            // Try to preserve the orientation of existing data layout dimension.
            eOrientDataLayout = static_cast<sheet::DataPilotFieldOrientation>(p->GetOrientation());
    }
    switch (eOrientDataLayout)
    {
        case sheet::DataPilotFieldOrientation_COLUMN:
            aColFields.push_back(ScPivotField(PIVOT_DATA_FIELD, 0));
        break;
        default:
            // data layout dimension can only be row or column.
            aRowFields.push_back(ScPivotField(PIVOT_DATA_FIELD, 0));
    }

    rPageFields.swap(aPageFields);
    rColFields.swap(aColFields);
    rRowFields.swap(aRowFields);
    rDataFields.swap(aDataFields);

    return true;
}

void ScPivotLayoutDlg::UpdateSrcRange()
{
    OUString aSrcStr = maEdInPos.GetText();
    sal_uInt16  nResult = ScRange().Parse(aSrcStr, mpDoc, mpDoc->GetAddressConvention());
    DataSrcType eSrcType = SRC_INVALID;
    ScRange aNewRange;

    if (SCA_VALID == (nResult & SCA_VALID))
    {
        // Valid source range.  Take it.
        ScRefAddress start, end;
        ConvertDoubleRef(mpDoc, aSrcStr, 1,  start, end, mpDoc->GetAddressConvention());
        aNewRange.aStart = start.GetAddress();
        aNewRange.aEnd = end.GetAddress();
        maEdInPos.SetRefValid(true);
        eSrcType = SRC_REF;
    }
    else
    {
        // invalid source range.  Check if this is a valid range name.
        bool bValid = false;
        ScRangeName* pRangeName = mpDoc->GetRangeName();
        if (pRangeName)
        {
            OUString aUpper = ScGlobal::pCharClass->uppercase(aSrcStr);
            const ScRangeData* pData = pRangeName->findByUpperName(aUpper);
            if (pData)
            {
                // range name found.  Check if this is a valid reference.
                bValid = pData->IsReference(aNewRange);
            }
        }

        maEdInPos.SetRefValid(bValid);
        if (!bValid)
        {
            // All attempts have failed.  Give up.
            maBtnOk.Disable();
            return;
        }

        eSrcType = SRC_NAME;
    }

    maBtnOk.Enable();

    // Now update the data src range or range name with the dp object.
    ScSheetSourceDesc inSheet = *mxDlgDPObject->GetSheetDesc();

    switch (eSrcType)
    {
        case SRC_REF:
        {
            // data source is a range reference.
            if (inSheet.GetSourceRange() == aNewRange)
                // new range is identical to the current range.  Nothing to do.
                return;
            inSheet.SetSourceRange(aNewRange);
            sal_uLong nError = inSheet.CheckSourceRange();
            if (nError)
            {
                // The error number corresponds with string ID for the error
                // message.  In the future we should display the error message
                // somewhere in the dialog to let the user know of the reason
                // for error.
                maEdInPos.SetRefValid(false);
                maBtnOk.Disable();
                return;
            }
        }
        break;
        case SRC_NAME:
            // data source is a range name.
            inSheet.SetRangeName(aSrcStr);
        break;
        default:
            OSL_FAIL( "Unknown source type.");
            return;
    }

    mxDlgDPObject->SetSheetDesc(inSheet);
    mxDlgDPObject->FillOldParam( maPivotData );
    mxDlgDPObject->FillLabelData(maPivotData);

    maLabelData.clear();
    maWndSelect.ClearFields();
    maWndData.ClearFields();
    maWndRow.ClearFields();
    maWndCol.ClearFields();
    maWndPage.ClearFields();

    InitFieldWindows();
    RepaintFieldWindows();
}

void ScPivotLayoutDlg::UpdateOutputPos()
{
    sal_uInt16 nSelPos = maLbOutPos.GetSelectEntryPos();
    OUString aEntryStr = maLbOutPos.GetEntry(nSelPos);

    if (aEntryStr == maStrNewTable)
    {
        // New sheet as output.
        maEdOutPos.Disable();
        maRbOutPos.Disable();
        maEdOutPos.SetText(OUString()); // Clear the reference text.
    }
    else if (aEntryStr == maStrUndefined)
    {
        maEdOutPos.Enable();
        maRbOutPos.Enable();
        maEdOutPos.SetText(maOutputRefStr);
        OutputPosUpdated();
    }
    else
    {
        // Named range as output. Get its corresponding reference string.
        const OUString* p = (const OUString*)maLbOutPos.GetEntryData(nSelPos);
        if (p)
            maEdOutPos.SetText(*p);
    }
}

void ScPivotLayoutDlg::OutputPosUpdated()
{
    OUString aOutPosStr = maEdOutPos.GetText();
    sal_uInt16 nResult = ScAddress().Parse(aOutPosStr, mpDoc, mpDoc->GetAddressConvention());

    if (!(nResult & SCA_VALID))
    {
        // Not a valid reference.
        maEdOutPos.SetRefValid(false);
        return;
    }

    maEdOutPos.SetRefValid(true);

    boost::ptr_vector<OUString>::const_iterator it =
        std::find(maRefStrs.begin(), maRefStrs.end(), aOutPosStr);

    if (it == maRefStrs.end())
    {
        // This is NOT one of the named ranges.
        maLbOutPos.SelectEntryPos(0);
        return;
    }

    // Select the corresponding named range item in the list box.  Be sure to
    // offset for the top two entries which are reserved for something else.
    boost::ptr_vector<OUString>::const_iterator itBeg = maRefStrs.begin();
    size_t nPos = std::distance(itBeg, it);
    maLbOutPos.SelectEntryPos(nPos+2);
}

namespace {

void EnableAndGrabFocus(formula::RefEdit& rEdit)
{
    rEdit.Enable();
    rEdit.GrabFocus();
    rEdit.Enable();
}

}

void ScPivotLayoutDlg::MoreBtnClicked()
{
    mbRefInputMode = maBtnMore.GetState();
    if (!maBtnMore.GetState())
        return;

    formula::RefEdit* p = maEdInPos.IsEnabled() ? &maEdInPos : &maEdOutPos;
    EnableAndGrabFocus(*p);
}

void ScPivotLayoutDlg::RepaintFieldWindows()
{
    Rectangle aRect; // currently has no effect whatsoever.
    maWndPage.Paint(aRect);
    maWndCol.Paint(aRect);
    maWndRow.Paint(aRect);
    maWndData.Paint(aRect);
}

ScDPFieldControlBase* ScPivotLayoutDlg::GetFieldWindow(ScPivotFieldType eType)
{
    switch (eType)
    {
        case PIVOTFIELDTYPE_PAGE:
            return &maWndPage;
        case PIVOTFIELDTYPE_COL:
            return &maWndCol;
        case PIVOTFIELDTYPE_ROW:
            return &maWndRow;
        case PIVOTFIELDTYPE_DATA:
            return &maWndData;
        case PIVOTFIELDTYPE_SELECT:
            return &maWndSelect;
        default:
            ;
    }
    return NULL;
}

void ScPivotLayoutDlg::GetOtherFieldWindows(ScPivotFieldType eType, ScDPFieldControlBase*& rpWnd1, ScDPFieldControlBase*& rpWnd2)
{
    rpWnd1 = NULL;
    rpWnd2 = NULL;
    switch (eType)
    {
        case PIVOTFIELDTYPE_PAGE:
            rpWnd1 = &maWndRow;
            rpWnd2 = &maWndCol;
            break;
        case PIVOTFIELDTYPE_COL:
            rpWnd1 = &maWndPage;
            rpWnd2 = &maWndRow;
            break;
        case PIVOTFIELDTYPE_ROW:
            rpWnd1 = &maWndPage;
            rpWnd2 = &maWndCol;
            break;
        default:
            ;
    }
}

void ScPivotLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if (!mbRefInputMode || !mpRefInputEdit)
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(mpRefInputEdit);

    if (mpRefInputEdit == &maEdInPos)
    {
        OUString aRefStr(rRef.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));
        mpRefInputEdit->SetRefString(aRefStr);
    }
    else if (mpRefInputEdit == &maEdOutPos)
    {
        OUString aRefStr(rRef.aStart.Format(STD_FORMAT, pDoc, pDoc->GetAddressConvention()));
        mpRefInputEdit->SetRefString(aRefStr);
        maOutputRefStr = aRefStr;
        OutputPosUpdated();
    }
}

sal_Bool ScPivotLayoutDlg::IsRefInputMode() const
{
    return mbRefInputMode;
}

void ScPivotLayoutDlg::SetActive()
{
    if ( mbRefInputMode )
    {
        if (mpRefInputEdit)
            mpRefInputEdit->GrabFocus();

        if (mpRefInputEdit == &maEdInPos)
            EdInModifyHdl( NULL );
        else if (mpRefInputEdit == &maEdOutPos)
            EdOutModifyHdl( NULL );
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

IMPL_LINK( ScPivotLayoutDlg, ClickHdl, PushButton *, pBtn )
{
    ScDPFieldControlBase* pWnd = GetFieldWindow( meLastActiveType );
    if (!pWnd)
        return 0;

    if( pBtn == &maBtnRemove )
    {
        RemoveField( meLastActiveType, pWnd->GetSelectedField() );
        if( !pWnd->IsEmpty() ) pWnd->GrabFocus();
    }
    else if( pBtn == &maBtnOptions )
    {
        NotifyDoubleClick( meLastActiveType, pWnd->GetSelectedField() );
        pWnd->GrabFocus();
    }
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, OkHdl)
{
    OUString aOutPosStr = maEdOutPos.GetText();
    ScAddress   aAdrDest;
    bool bToNewTable = (maLbOutPos.GetSelectEntryPos() == 1);
    sal_uInt16      nResult     = !bToNewTable ? aAdrDest.Parse( aOutPosStr, mpDoc, mpDoc->GetAddressConvention() ) : 0;

    if (!bToNewTable && (aOutPosStr.isEmpty() || (nResult & SCA_VALID) != SCA_VALID))
    {
        // Invalid reference.  Bail out.
        if ( !maBtnMore.GetState() )
            maBtnMore.SetState(true);

        ErrorBox(this, WinBits(WB_OK | WB_DEF_OK), ScGlobal::GetRscString(STR_INVALID_TABREF)).Execute();
        maEdOutPos.GrabFocus();
        return 0;
    }

    ScPivotParam    theOutParam;
    vector<ScPivotField> aPageFields;
    vector<ScPivotField> aColFields;
    vector<ScPivotField> aRowFields;
    vector<ScPivotField> aDataFields;

    // Convert an array of function data into an array of pivot field data.
    bool bFit = GetPivotArrays(aPageFields, aColFields, aRowFields, aDataFields);

    if (!bFit)
    {
        // General data pilot table error.  Bail out.
        ErrorBox(this, WinBits(WB_OK | WB_DEF_OK), ScGlobal::GetRscString(STR_PIVOT_ERROR)).Execute();
        return 0;
    }

    ScDPSaveData* pOldSaveData = mxDlgDPObject->GetSaveData();

    ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

    ScDPSaveData aSaveData;
    aSaveData.SetIgnoreEmptyRows( maBtnIgnEmptyRows.IsChecked() );
    aSaveData.SetRepeatIfEmpty( maBtnDetectCat.IsChecked() );
    aSaveData.SetColumnGrand( maBtnTotalCol.IsChecked() );
    aSaveData.SetRowGrand( maBtnTotalRow.IsChecked() );
    aSaveData.SetFilterButton( maBtnFilter.IsChecked() );
    aSaveData.SetDrillDown( maBtnDrillDown.IsChecked() );

    uno::Reference<sheet::XDimensionsSupplier> xSource = mxDlgDPObject->GetSource();

    ScDPObject::ConvertOrientation(
        aSaveData, aPageFields, sheet::DataPilotFieldOrientation_PAGE, xSource, maLabelData);
    ScDPObject::ConvertOrientation(
        aSaveData, aColFields, sheet::DataPilotFieldOrientation_COLUMN, xSource, maLabelData);
    ScDPObject::ConvertOrientation(
        aSaveData, aRowFields, sheet::DataPilotFieldOrientation_ROW, xSource, maLabelData);
    ScDPObject::ConvertOrientation(
        aSaveData, aDataFields, sheet::DataPilotFieldOrientation_DATA, xSource, maLabelData,
        &aColFields, &aRowFields, &aPageFields );

    for( ScDPLabelDataVector::const_iterator aIt = maLabelData.begin(), aEnd = maLabelData.end(); aIt != aEnd; ++aIt )
    {
        ScDPSaveDimension* pDim = aSaveData.GetExistingDimensionByName(aIt->maName);

        if (!pDim)
            continue;

        pDim->SetUsedHierarchy( aIt->mnUsedHier );
        pDim->SetShowEmpty( aIt->mbShowAll );
        pDim->SetSortInfo( &aIt->maSortInfo );
        pDim->SetLayoutInfo( &aIt->maLayoutInfo );
        pDim->SetAutoShowInfo( &aIt->maShowInfo );
        ScDPSaveDimension* pOldDim = NULL;

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

            if (!pOldDim)
                continue;

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

    // also transfer grand total name
    if (pOldSaveData)
    {
        const OUString* pGrandTotalName = pOldSaveData->GetGrandTotalName();
        if (pGrandTotalName)
            aSaveData.SetGrandTotalName(*pGrandTotalName);
    }

    sal_uInt16 nWhichPivot = SC_MOD()->GetPool().GetWhich( SID_PIVOT_TABLE );
    ScPivotItem aOutItem( nWhichPivot, &aSaveData, &aOutRange, bToNewTable );

    mbRefInputMode = false;      // to allow deselecting when switching sheets

    SetDispatcherLock( false );
    SwitchToDocument();

    ScTabViewShell* pTabViewShell = mpViewData->GetViewShell();
    pTabViewShell->SetDialogDPObject(mxDlgDPObject.get());

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
        mbRefInputMode = true;
        SetDispatcherLock(true);
    }

    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, CancelHdl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, MoreClickHdl)
{
    MoreBtnClicked();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, EdOutModifyHdl)
{
    OutputPosUpdated();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, EdInModifyHdl)
{
    UpdateSrcRange();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, SelAreaHdl)
{
    UpdateOutputPos();
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, GetRefEditFocusHdl, formula::RefEdit*, pEdit )
{
    mpRefInputEdit = pEdit;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
