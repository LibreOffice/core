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

using namespace com::sun::star;
using ::rtl::OUString;
using ::std::vector;
using ::std::for_each;

ScPivotLayoutDlg::FieldRect::FieldRect(const Rectangle* pRect, ScPivotFieldType eType) :
    mpRect(pRect), meType(eType) {}

namespace {

const sal_uInt16 STD_FORMAT = sal_uInt16( SCA_VALID | SCA_TAB_3D | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE );

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
    maEdInPos( this, this, ScResId( ED_INAREA) ),
    maRbInPos( this, ScResId( RB_INAREA ), &maEdInPos, this ),
    maLbOutPos( this, ScResId( LB_OUTAREA ) ),
    maFtOutArea( this, ScResId( FT_OUTAREA ) ),
    maEdOutPos( this, this, ScResId( ED_OUTAREA ) ),
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
    mpActiveEdit(NULL),

    maStrUndefined(SC_RESSTR(SCSTR_UNDEFINED)),
    maStrNewTable(SC_RESSTR(SCSTR_NEWTABLE)),
    mbIsDrag(false),
    meLastActiveType(PIVOTFIELDTYPE_SELECT),
    mnOffset(0),
    mbRefInputMode( false )
{
    mxDlgDPObject->SetAlive( true );     // needed to get structure information
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
            rtl::OUString aStr;
            maOldRange = p->GetSourceRange();
            maOldRange.Format(aStr, SCR_ABS_3D, mpDoc, mpDoc->GetAddressConvention());
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
        rtl::OUString aName;
        ScRange aRange;
        rtl::OUString aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                sal_uInt16 nInsert = maLbOutPos.InsertEntry( aName );

                aRange.aStart.Format( aRefStr, SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention() );
                maLbOutPos.SetEntryData(nInsert, new rtl::OUString(aRefStr));
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
            rtl::OUString aStr;
            ScAddress( maPivotData.nCol,
                       maPivotData.nRow,
                       maPivotData.nTab ).Format( aStr, STD_FORMAT, mpDoc, mpDoc->GetAddressConvention() );
            maEdOutPos.SetText( aStr );
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
    for (sal_uInt16 i = 2, nEntries = maLbOutPos.GetEntryCount(); i < nEntries; ++i)
        delete (OUString*)maLbOutPos.GetEntryData(i);
}

sal_Bool ScPivotLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}

void ScPivotLayoutDlg::InitWndSelect(const ScDPLabelDataVector& rLabels)
{
    size_t nLabelCount = rLabels.size();
    if (nLabelCount > SC_DP_MAX_FIELDS)
        nLabelCount = SC_DP_MAX_FIELDS;

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
            maWndSelect.AddField(maLabelData[i].getDisplayName(), i);
            maSelectArr.push_back(new ScPivotFuncData(maLabelData[i].mnCol, maLabelData[i].mnFuncMask));
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

        size_t nFieldIndex = maDataArr.size();
        maDataArr.push_back(
            new ScPivotFuncData(nCol, it->mnOriginalDim, nMask, it->mnDupCount, it->maFieldRef));

        // data field - we need to concatenate function name with the field name.
        ScDPLabelData* pData = GetLabelData(nCol);
        OSL_ENSURE( pData, "ScDPLabelData not found" );
        if (pData)
        {
            OUString aStr = pData->maLayoutName;
            if (aStr.isEmpty())
            {
                sal_uInt16 nInitMask = maDataArr.back().mnFuncMask;
                aStr = GetFuncString(nInitMask, pData->mbIsValue);
                aStr += pData->maName;
            }

            maWndData.AddField(aStr, nFieldIndex);
            pData->mnFuncMask = nMask;
        }
    }
    maWndData.ResetScrollBar();
}

void ScPivotLayoutDlg::InitFieldWindow( const vector<ScPivotField>& rFields, ScPivotFieldType eType )
{
    OSL_ASSERT(eType != PIVOTFIELDTYPE_DATA);
    ScDPFuncDataVec* pInitArr = GetFieldDataArray(eType);
    ScDPFieldControlBase* pInitWnd = GetFieldWindow(eType);

    if (!pInitArr || !pInitWnd)
        return;

    vector<ScPivotField>::const_iterator itr = rFields.begin(), itrEnd = rFields.end();
    for (; itr != itrEnd; ++itr)
    {
        SCCOL nCol = itr->nCol;
        sal_uInt16 nMask = itr->nFuncMask;
        if (nCol == PIVOT_DATA_FIELD)
            continue;

        size_t nFieldIndex = pInitArr->size();
        pInitArr->push_back(
            new ScPivotFuncData(nCol, itr->mnOriginalDim, nMask, itr->mnDupCount, itr->maFieldRef));
        pInitWnd->AddField(GetLabelString(nCol), nFieldIndex);
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
    ScPivotFuncData fData = maSelectArr[nFromIndex];

    bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );
    if (!bAllowed)
        return;

    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    ScDPFuncDataVec*    toArr = GetFieldDataArray(eToType);
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    GetOtherDataArrays(eToType, rmArr1, rmArr2);

    if (eToType == PIVOTFIELDTYPE_DATA)
    {
        // Data field allows duplicates.
        ScDPLabelData* p = GetLabelData(fData.mnCol);
        OUString aStr = p->maLayoutName;
        sal_uInt16 nMask = fData.mnFuncMask;
        if (nMask == PIVOT_FUNC_NONE)
            nMask = PIVOT_FUNC_SUM; // Use SUM by default.
        if (aStr.isEmpty())
        {
            aStr = GetFuncString(nMask);
            aStr += p->maName;
        }

        size_t nAddedAt = 0;
        sal_uInt8 nDupCount = 0;
        if (toWnd->AddField(aStr, DlgPos2WndPos(rAtPos, *toWnd), nAddedAt, nDupCount))
        {
            fData.mnFuncMask = nMask;
            fData.mnDupCount = nDupCount;
            Insert(toArr, fData, nAddedAt);
            toWnd->GrabFocus();
        }

        return;
    }

    if (!Contains(toArr, fData, nAt))
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, fData, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, fData, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        size_t nAddedAt = 0;
        sal_uInt8 nDupCount = 0;
        ScDPLabelData& rData = maLabelData[nFromIndex+mnOffset];
        if (toWnd->AddField(
            rData.getDisplayName(), DlgPos2WndPos(rAtPos, *toWnd), nAddedAt, nDupCount))
        {
            fData.mnDupCount = nDupCount;
            Insert( toArr, fData, nAddedAt );
            toWnd->GrabFocus();
        }
    }
}

void ScPivotLayoutDlg::AppendField(size_t nFromIndex, ScPivotFieldType eToType)
{
    ScPivotFuncData aFuncData = maSelectArr[nFromIndex];

    size_t nAt = 0;
    ScDPFieldControlBase* toWnd = GetFieldWindow(eToType);
    ScDPFieldControlBase* rmWnd1 = NULL;
    ScDPFieldControlBase* rmWnd2 = NULL;
    GetOtherFieldWindows(eToType, rmWnd1, rmWnd2);

    ScDPFuncDataVec*    toArr = GetFieldDataArray(eToType);
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    GetOtherDataArrays(eToType, rmArr1, rmArr2);

    bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

    if ( (!Contains( toArr, aFuncData, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, aFuncData, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, aFuncData, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        ScDPLabelData&  rData = maLabelData[nFromIndex+mnOffset];
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
            sal_uInt16 nMask = aFuncData.mnFuncMask;
            if (aStr.isEmpty())
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

        ScDPFuncDataVec*    fromArr  = GetFieldDataArray(eFromType);
        ScDPFuncDataVec*    toArr    = GetFieldDataArray(eToType);

        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        GetOtherDataArrays(eToType, rmArr1, rmArr2);

        bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScPivotFuncData fData = (*fromArr)[nFromIndex];
            bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );

            size_t nAt = 0;
            if ( bAllowed && Contains( fromArr, fData, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (!Contains( toArr, fData, nAt ))
                {
                    size_t nAddedAt = 0;
                    sal_uInt8 nDupCount = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData, nAt ) )
                            {
                                rmWnd2->DelField( nAt );
                                Remove( rmArr2, nAt );
                            }
                        }

                        if ( toWnd->AddField( GetLabelString( fData.mnCol ),
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt, nDupCount) )
                        {
                            fData.mnDupCount = nDupCount;
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                    else
                    {
                        ScDPLabelData* p = GetLabelData(fData.mnCol);
                        OUString aStr = p->maLayoutName;
                        sal_uInt16 nMask = fData.mnFuncMask;
                        if (aStr.isEmpty())
                        {
                            aStr = GetFuncString(nMask);
                            aStr += p->maName;
                        }

                        if ( toWnd->AddField( aStr,
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt, nDupCount) )
                        {
                            fData.mnFuncMask = nMask;
                            fData.mnDupCount = nDupCount;
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

        ScPivotFuncData fData = (*theArr)[nFromIndex];

        if ( Contains( theArr, fData, nAt ) )
        {
            size_t nToIndex = 0;
            aToPos = DlgPos2WndPos( rAtPos, *theWnd );
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;
                if (theWnd->MoveField(nAt, aToPos, nAddedAt))
                {
                    Remove(theArr, nAt);
                    Insert(theArr, fData, nAddedAt);
                }
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

        ScDPFuncDataVec*    fromArr  = GetFieldDataArray(eFromType);
        ScDPFuncDataVec*    toArr    = GetFieldDataArray(eToType);

        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        GetOtherDataArrays(eToType, rmArr1, rmArr2);

        bool bDataArr = eToType == PIVOTFIELDTYPE_DATA;

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScPivotFuncData fData = (*fromArr)[nFromIndex];

            size_t nAt = 0;
            if ( Contains( fromArr, fData, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (!Contains( toArr, fData, nAt ))
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData, nAt ) )
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
                        sal_uInt16 nMask = fData.mnFuncMask;
                        if (aStr.isEmpty())
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
        bool bDataArr = eFromType == PIVOTFIELDTYPE_DATA;

        ScPivotFuncData fData = (*theArr)[nFromIndex];

        if ( Contains( theArr, fData, nAt ) )
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
                    sal_uInt16 nMask = fData.mnFuncMask;
                    if (aStr.isEmpty())
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

void ScPivotLayoutDlg::RemoveField( ScPivotFieldType eFromType, size_t nIndex )
{
    ScDPFuncDataVec* pArr = GetFieldDataArray(eFromType);

    if( pArr )
    {
        ScDPFieldControlBase* pWnd = GetFieldWindow( eFromType );
        if (pWnd)
        {
            pWnd->DelField( nIndex );
            Remove( pArr, nIndex );
            if( pWnd->IsEmpty() )
                GrabFieldFocus( maWndSelect );
        }
    }
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
    ScDPFuncDataVec* pArr = GetFieldDataArray(eType);

    if ( pArr )
    {
        if ( nFieldIndex >= pArr->size() )
        {
            OSL_FAIL("invalid selection");
            return;
        }

        size_t nArrPos = 0;
        if( ScDPLabelData* pData = GetLabelData( (*pArr)[nFieldIndex].mnCol, &nArrPos ) )
        {
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
                    for( ScDPFuncDataVec::const_iterator aIt = maDataArr.begin(), aEnd = maDataArr.end();
                            (aIt != aEnd); ++aIt)
                    {
                        ScDPLabelData* pDFData = GetLabelData(aIt->mnCol);
                        if (!pDFData)
                            continue;

                        if (pDFData->maName.isEmpty())
                            continue;

                        OUString aLayoutName = pDFData->maLayoutName;
                        if (aLayoutName.isEmpty())
                        {
                            // No layout name exists.  Use the stock name.
                            sal_uInt16 nMask = aIt->mnFuncMask;
                            OUString aFuncStr = GetFuncString(nMask);
                            aLayoutName = aFuncStr + pDFData->maName;
                        }
                        aDataFieldNames.push_back(ScDPName(pDFData->maName, aLayoutName));
                    }

                    bool bLayout = (eType == PIVOTFIELDTYPE_ROW) &&
                        ((aDataFieldNames.size() > 1) || (nFieldIndex + 1 < pArr->size()));

                    AbstractScDPSubtotalDlg* pDlg = pFact->CreateScDPSubtotalDlg(
                        this, RID_SCDLG_PIVOTSUBT,
                        *mxDlgDPObject, *pData, (*pArr)[nFieldIndex], aDataFieldNames, bLayout );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        pDlg->FillLabelData( *pData );
                        (*pArr)[nFieldIndex].mnFuncMask = pData->mnFuncMask;
                    }
                    delete pDlg;
                }
                break;

                case PIVOTFIELDTYPE_DATA:
                {
                    ScPivotFuncData& rFuncData = maDataArr[nFieldIndex];
                    AbstractScDPFunctionDlg* pDlg = pFact->CreateScDPFunctionDlg(
                        this, RID_SCDLG_DPDATAFIELD,
                        maLabelData, *pData, rFuncData);

                    if ( pDlg->Execute() == RET_OK )
                    {
                        bool bFuncChanged = rFuncData.mnFuncMask != pDlg->GetFuncMask();
                        rFuncData.mnFuncMask = pData->mnFuncMask = pDlg->GetFuncMask();
                        rFuncData.maFieldRef = pDlg->GetFieldRef();

                        if (bFuncChanged)
                            // Get the new duplicate count since the function has changed.
                            rFuncData.mnDupCount = GetNextDupCount(maDataArr, rFuncData, nFieldIndex);

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

//    Point aPos = ScreenToOutputPixel(rScrPos);
    if (eFieldType == PIVOTFIELDTYPE_UNKNOWN)
        // Outside any field areas.
        return meDnDFromType == PIVOTFIELDTYPE_SELECT ? POINTER_PIVOT_FIELD : POINTER_PIVOT_DELETE;

    if (eFieldType == PIVOTFIELDTYPE_SELECT)
        return POINTER_PIVOT_FIELD;

    // check if the target orientation is allowed for this field
    ScDPFuncDataVec* fromArr = GetFieldDataArray(meDnDFromType);
    const ScPivotFuncData& rData = (*fromArr)[mnDnDFromIndex];
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

ScPivotFieldType ScPivotLayoutDlg::GetFieldTypeAtPoint( const Point& rScrPos ) const
{
    Point aOutputPos = ScreenToOutputPixel(rScrPos);
    std::vector<FieldRect>::const_iterator it = maFieldRects.begin(), itEnd = maFieldRects.end();
    for (; it != itEnd; ++it)
    {
        if (it->mpRect->IsInside(aOutputPos))
            return it->meType;
    }

    return PIVOTFIELDTYPE_UNKNOWN;
}

void ScPivotLayoutDlg::Deactivate()
{
    /*  If the dialog has been deactivated (click into document), the LoseFocus
        event from field window disables Remove/Options buttons. Re-enable them here by
        simulating a GetFocus event. Event order of LoseFocus and Deactivate is not important.
        The last event will enable the buttons in both cases (see NotifyFieldFocus). */
    NotifyFieldFocus( meLastActiveType, true );
}

bool ScPivotLayoutDlg::Contains( ScDPFuncDataVec* pArr, const ScPivotFuncData& rData, size_t& nAt )
{
    if (!pArr || pArr->empty())
        return false;

    ScDPFuncDataVec::const_iterator itr, itrBeg = pArr->begin(), itrEnd = pArr->end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        if (*itr == rData)
        {
            // found!
            nAt = ::std::distance(itrBeg, itr);
            return true;
        }
    }
    return false;
}

void ScPivotLayoutDlg::Remove( ScDPFuncDataVec* pArr, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    pArr->erase( pArr->begin() + nAt );
}

void ScPivotLayoutDlg::Insert( ScDPFuncDataVec* pArr, const ScPivotFuncData& rFData, size_t nAt )
{
    if (!pArr)
        return;

    std::auto_ptr<ScPivotFuncData> p(new ScPivotFuncData(rFData));
    if (nAt >= pArr->size())
        pArr->push_back(p);
    else
        pArr->insert(pArr->begin() + nAt, p);
}

ScDPLabelData* ScPivotLayoutDlg::GetLabelData( SCsCOL nCol, size_t* pnPos )
{
    ScDPLabelData* pData = 0;
    for( ScDPLabelDataVector::iterator aIt = maLabelData.begin(), aEnd = maLabelData.end(); !pData && (aIt != aEnd); ++aIt )
    {
        if( aIt->mnCol == nCol )
        {
            pData = &*aIt;
            if( pnPos ) *pnPos = aIt - maLabelData.begin();
        }
    }
    return pData;
}

rtl::OUString ScPivotLayoutDlg::GetLabelString( SCsCOL nCol )
{
    ScDPLabelData* pData = GetLabelData( nCol );
    OSL_ENSURE( pData, "LabelData not found" );
    if (pData)
        return pData->getDisplayName();
    return rtl::OUString();
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

rtl::OUString ScPivotLayoutDlg::GetFuncString( sal_uInt16& rFuncMask, bool bIsValue )
{
    rtl::OUStringBuffer aBuf;

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

Point ScPivotLayoutDlg::DlgPos2WndPos( const Point& rPt, Window& rWnd )
{
    Point aWndPt( rPt );
    aWndPt.X() = rPt.X()-rWnd.GetPosPixel().X();
    aWndPt.Y() = rPt.Y()-rWnd.GetPosPixel().Y();

    return aWndPt;
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

    maRectPage   = Rectangle( maWndPage.GetPosPixel(),    maWndPage.GetSizePixel() );
    maRectRow    = Rectangle( maWndRow.GetPosPixel(),     maWndRow.GetSizePixel() );
    maRectCol    = Rectangle( maWndCol.GetPosPixel(),     maWndCol.GetSizePixel() );
    maRectData   = Rectangle( maWndData.GetPosPixel(),    maWndData.GetSizePixel() );
    maRectSelect = Rectangle( maWndSelect.GetPosPixel(),  maWndSelect.GetSizePixel() );

    maFieldRects.push_back(FieldRect(&maRectPage, PIVOTFIELDTYPE_PAGE));
    maFieldRects.push_back(FieldRect(&maRectRow, PIVOTFIELDTYPE_ROW));
    maFieldRects.push_back(FieldRect(&maRectCol, PIVOTFIELDTYPE_COL));
    maFieldRects.push_back(FieldRect(&maRectData, PIVOTFIELDTYPE_DATA));
    maFieldRects.push_back(FieldRect(&maRectSelect, PIVOTFIELDTYPE_SELECT));

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

namespace {

class PivotFieldInserter : public ::std::unary_function<ScPivotFuncData, void>
{
    vector<ScPivotField>& mrFields;
public:
    explicit PivotFieldInserter(vector<ScPivotField>& r, size_t nSize) : mrFields(r)
    {
        mrFields.reserve(nSize);
    }

    PivotFieldInserter(const PivotFieldInserter& r) : mrFields(r.mrFields) {}

    void operator() (const ScPivotFuncData& r)
    {
        ScPivotField aField;
        aField.nCol = r.mnCol;
        aField.mnOriginalDim = r.mnOriginalDim;
        aField.mnDupCount = r.mnDupCount;
        aField.nFuncMask = r.mnFuncMask;
        aField.maFieldRef = r.maFieldRef;
        mrFields.push_back(aField);
    }
};

}

bool ScPivotLayoutDlg::GetPivotArrays(
    vector<ScPivotField>& rPageFields, vector<ScPivotField>& rColFields,
    vector<ScPivotField>& rRowFields, vector<ScPivotField>& rDataFields )
{
    vector<ScPivotField> aPageFields;
    for_each(maPageArr.begin(), maPageArr.end(), PivotFieldInserter(aPageFields, maPageArr.size()));

    vector<ScPivotField> aColFields;
    for_each(maColArr.begin(), maColArr.end(), PivotFieldInserter(aColFields, maColArr.size()+1));

    vector<ScPivotField> aRowFields;
    for_each(maRowArr.begin(), maRowArr.end(), PivotFieldInserter(aRowFields, maRowArr.size()+1));

    vector<ScPivotField> aDataFields;
    for_each(maDataArr.begin(), maDataArr.end(), PivotFieldInserter(aDataFields, maDataArr.size()));

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
    rtl::OUString aSrcStr = maEdInPos.GetText();
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

    maSelectArr.clear();
    maRowArr.clear();
    maColArr.clear();
    maDataArr.clear();
    maPageArr.clear();

    InitFieldWindows();
    RepaintFieldWindows();
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

ScPivotLayoutDlg::ScDPFuncDataVec* ScPivotLayoutDlg::GetFieldDataArray(ScPivotFieldType eType)
{
    switch (eType)
    {
        case PIVOTFIELDTYPE_PAGE:
            return &maPageArr;
        case PIVOTFIELDTYPE_COL:
            return &maColArr;
        case PIVOTFIELDTYPE_ROW:
            return &maRowArr;
        case PIVOTFIELDTYPE_DATA:
            return &maDataArr;
        case PIVOTFIELDTYPE_SELECT:
            return &maSelectArr;
        default:
            ;
    }
    return NULL;
}

void ScPivotLayoutDlg::GetOtherDataArrays(
    ScPivotFieldType eType, ScDPFuncDataVec*& rpArr1, ScDPFuncDataVec*& rpArr2)
{
    rpArr1 = NULL;
    rpArr2 = NULL;
    switch (eType)
    {
        case PIVOTFIELDTYPE_PAGE:
            rpArr1 = &maRowArr;
            rpArr2 = &maColArr;
            break;
        case PIVOTFIELDTYPE_COL:
            rpArr1 = &maPageArr;
            rpArr2 = &maRowArr;
            break;
        case PIVOTFIELDTYPE_ROW:
            rpArr1 = &maPageArr;
            rpArr2 = &maColArr;
            break;
        default:
            ;
    }
}

sal_uInt8 ScPivotLayoutDlg::GetNextDupCount(
    const ScDPFuncDataVec& rArr, const ScPivotFuncData& rData, size_t nDataIndex) const
{
    sal_uInt8 nDupCount = 0;
    bool bFound = false;
    for (size_t i = 0, n = rArr.size(); i < n; ++i)
    {
        const ScPivotFuncData& r = rArr[i];
        if (i == nDataIndex)
            // Skip itself.
            continue;

        if (r.mnCol != rData.mnCol || r.mnFuncMask != rData.mnFuncMask)
            continue;

        bFound = true;
        if (r.mnDupCount > nDupCount)
            nDupCount = r.mnDupCount;
    }

    return bFound ? nDupCount + 1 : 0;
}

void ScPivotLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( !mbRefInputMode || !mpActiveEdit )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( mpActiveEdit );

    if ( mpActiveEdit == &maEdInPos )
    {
        rtl::OUString aRefStr;
        rRef.Format( aRefStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
        mpActiveEdit->SetRefString( aRefStr );
    }
    else if ( mpActiveEdit == &maEdOutPos )
    {
        rtl::OUString aRefStr;
        rRef.aStart.Format( aRefStr, STD_FORMAT, pDoc, pDoc->GetAddressConvention() );
        mpActiveEdit->SetRefString( aRefStr );
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
        if ( mpActiveEdit )
            mpActiveEdit->GrabFocus();

        if ( mpActiveEdit == &maEdInPos )
            EdInModifyHdl( NULL );
        else if ( mpActiveEdit == &maEdOutPos )
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
    rtl::OUString aOutPosStr = maEdOutPos.GetText();
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
    if ( maBtnMore.GetState() )
    {
        mbRefInputMode = true;
        if ( maEdInPos.IsEnabled() )
        {
            maEdInPos.Enable();
            maEdInPos.GrabFocus();
            maEdInPos.Enable();
        }
        else
        {
        maEdOutPos.Enable();
        maEdOutPos.GrabFocus();
            maEdOutPos.Enable();
        }
    }
    else
    {
        mbRefInputMode = false;
    }
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, EdOutModifyHdl)
{
    rtl::OUString theCurPosStr = maEdOutPos.GetText();
    sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, mpDoc, mpDoc->GetAddressConvention() );

    if ( SCA_VALID == (nResult & SCA_VALID) )
    {
        rtl::OUString* pStr = NULL;
        bool bFound  = false;
        sal_uInt16  i       = 0;
        sal_uInt16  nCount  = maLbOutPos.GetEntryCount();

        for ( i=2; i<nCount && !bFound; i++ )
        {
            pStr = static_cast<rtl::OUString*>(maLbOutPos.GetEntryData(i));
            bFound = (theCurPosStr == *pStr);
        }

        if ( bFound )
            maLbOutPos.SelectEntryPos( --i );
        else
            maLbOutPos.SelectEntryPos( 0 );
    }
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, EdInModifyHdl)
{
    UpdateSrcRange();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDlg, SelAreaHdl)
{
    rtl::OUString  aString;
    sal_uInt16  nSelPos = maLbOutPos.GetSelectEntryPos();

    if ( nSelPos > 1 )
    {
        aString = *(rtl::OUString*)maLbOutPos.GetEntryData(nSelPos);
    }
    else if ( nSelPos == maLbOutPos.GetEntryCount()-1 ) // auf neue Tabelle?
    {
        maEdOutPos.Disable();
        maRbOutPos.Disable();
    }
    else
    {
        maEdOutPos.Enable();
        maRbOutPos.Enable();
    }

    maEdOutPos.SetText( aString );
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, GetFocusHdl, Control*, pCtrl )
{
    mpActiveEdit = NULL;
    if ( pCtrl == &maEdInPos )
        mpActiveEdit = &maEdInPos;
    else if ( pCtrl == &maEdOutPos )
        mpActiveEdit = &maEdOutPos;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
