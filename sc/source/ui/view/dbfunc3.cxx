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

#include "dbfunc.hxx"
#include "scitems.hxx"
#include <sfx2/bindings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svl/zforlist.hxx>
#include <sfx2/app.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>

#include "global.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "undotab.hxx"
#include "undodat.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "docsh.hxx"
#include "olinetab.hxx"
#include "consoli.hxx"
#include "olinefun.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dbdocfun.hxx"
#include "dpoutput.hxx"
#include "dptabsrc.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "editable.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "unonames.hxx"
#include "formulacell.hxx"
#include "userlist.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include "stringutil.hxx"
#include "tabvwsh.hxx"

#include <list>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::sheet::XDimensionsSupplier;
using ::std::list;
using ::std::vector;

//          outliner

// create outline grouping

void ScDBFunc::MakeOutline( bool bColumns, bool bRecord )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.MakeOutline( aRange, bColumns, bRecord, false );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

// delete outline grouping

void ScDBFunc::RemoveOutline( bool bColumns, bool bRecord )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveOutline( aRange, bColumns, bRecord, false );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//  menu status: delete outlines

void ScDBFunc::TestRemoveOutline( bool& rCol, bool& rRow )
{
    bool bColFound = false;
    bool bRowFound = false;

    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    SCTAB nStartTab, nEndTab;
    if (GetViewData().GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
    {
        SCTAB nTab = nStartTab;
        ScDocument* pDoc = GetViewData().GetDocument();
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
        {
            ScOutlineEntry* pEntry;
            SCCOLROW nStart;
            SCCOLROW nEnd;
            bool bColMarked = ( nStartRow == 0 && nEndRow == MAXROW );
            bool bRowMarked = ( nStartCol == 0 && nEndCol == MAXCOL );

            // columns

            if ( !bRowMarked || bColMarked )        // not when entire rows are marked
            {
                ScOutlineArray& rArray = pTable->GetColArray();
                ScSubOutlineIterator aColIter( &rArray );
                while ((pEntry=aColIter.GetNext()) != nullptr && !bColFound)
                {
                    nStart = pEntry->GetStart();
                    nEnd   = pEntry->GetEnd();
                    if ( nStartCol<=static_cast<SCCOL>(nEnd) && nEndCol>=static_cast<SCCOL>(nStart) )
                        bColFound = true;
                }
            }

            // rows

            if ( !bColMarked || bRowMarked )        // not when entire columns are marked
            {
                ScOutlineArray& rArray = pTable->GetRowArray();
                ScSubOutlineIterator aRowIter( &rArray );
                while ((pEntry=aRowIter.GetNext()) != nullptr && !bRowFound)
                {
                    nStart = pEntry->GetStart();
                    nEnd   = pEntry->GetEnd();
                    if ( nStartRow<=nEnd && nEndRow>=nStart )
                        bRowFound = true;
                }
            }
        }
    }

    rCol = bColFound;
    rRow = bRowFound;
}

void ScDBFunc::RemoveAllOutlines( bool bRecord )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    bool bOk = aFunc.RemoveAllOutlines( nTab, bRecord );

    if (bOk)
        UpdateScrollBars();
}

// auto outlines

void ScDBFunc::AutoOutline( )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScRange aRange( 0,0,nTab, MAXCOL,MAXROW,nTab );     // the complete sheet, if nothing is marked
    ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        rMark.MarkToMulti();
        rMark.GetMultiMarkArea( aRange );
    }

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);
    aFunc.AutoOutline( aRange, true );
}

// select outline level

void ScDBFunc::SelectLevel( bool bColumns, sal_uInt16 nLevel, bool bRecord )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    bool bOk = aFunc.SelectLevel( nTab, bColumns, nLevel, bRecord, true/*bPaint*/ );

    if (bOk)
        UpdateScrollBars();
}

// show individual outline groups

void ScDBFunc::ShowOutline( bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry, bool bRecord, bool bPaint )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    bool bOk = aFunc.ShowOutline( nTab, bColumns, nLevel, nEntry, bRecord, bPaint );

    if ( bOk && bPaint )
        UpdateScrollBars();
}

// hide individual outline groups

void ScDBFunc::HideOutline( bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry, bool bRecord, bool bPaint )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    bool bOk = aFunc.HideOutline( nTab, bColumns, nLevel, nEntry, bRecord, bPaint );

    if ( bOk && bPaint )
        UpdateScrollBars();
}

// menu status: show/hide marked range

bool ScDBFunc::OutlinePossible(bool bHide)
{
    bool bEnable = false;

    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;

    if (GetViewData().GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        SCTAB nTab = GetViewData().GetTabNo();
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
        {
            ScOutlineEntry* pEntry;
            SCCOLROW nStart;
            SCCOLROW nEnd;

            // columns

            ScOutlineArray& rColArray = pTable->GetColArray();
            ScSubOutlineIterator aColIter( &rColArray );
            while ((pEntry=aColIter.GetNext()) != nullptr && !bEnable)
            {
                nStart = pEntry->GetStart();
                nEnd   = pEntry->GetEnd();
                if ( bHide )
                {
                    if ( nStartCol<=static_cast<SCCOL>(nEnd) && nEndCol>=static_cast<SCCOL>(nStart) )
                        if (!pEntry->IsHidden())
                            bEnable = true;
                }
                else
                {
                    if ( nStart>=nStartCol && nEnd<=nEndCol )
                        if (pEntry->IsHidden())
                            bEnable = true;
                }
            }

            // rows

            ScOutlineArray& rRowArray = pTable->GetRowArray();
            ScSubOutlineIterator aRowIter( &rRowArray );
            while ((pEntry=aRowIter.GetNext()) != nullptr)
            {
                nStart = pEntry->GetStart();
                nEnd   = pEntry->GetEnd();
                if ( bHide )
                {
                    if ( nStartRow<=nEnd && nEndRow>=nStart )
                        if (!pEntry->IsHidden())
                            bEnable = true;
                }
                else
                {
                    if ( nStart>=nStartRow && nEnd<=nEndRow )
                        if (pEntry->IsHidden())
                            bEnable = true;
                }
            }
        }
    }

    return bEnable;
}

// show marked range

void ScDBFunc::ShowMarkedOutlines( bool bRecord )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        bool bDone = aFunc.ShowMarkedOutlines( aRange, bRecord );
        if (bDone)
            UpdateScrollBars();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

// hide marked range

void ScDBFunc::HideMarkedOutlines( bool bRecord )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        bool bDone = aFunc.HideMarkedOutlines( aRange, bRecord );
        if (bDone)
            UpdateScrollBars();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

// sub totals

void ScDBFunc::DoSubTotals( const ScSubTotalParam& rParam, bool bRecord,
                            const ScSortParam* pForceNewSort )
{
    bool bDo = !rParam.bRemoveOnly;                         // sal_False = only delete

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    SCTAB nTab = GetViewData().GetTabNo();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "SubTotals: no DBData" );
        return;
    }

    ScEditableTester aTester( &rDoc, nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    if (rDoc.HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
    {
        ErrorMessage(STR_MSSG_INSERTCELLS_0);   // do not insert into merged
        return;
    }

    WaitObject aWait( GetViewData().GetDialogParent() );
    bool bOk = true;
    if (rParam.bReplace)
        if (rDoc.TestRemoveSubTotals( nTab, rParam ))
        {
            bOk = ( ScopedVclPtrInstance<MessBox>( GetViewData().GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                // "StarCalc" "delete data?"
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) )->Execute()
                == RET_YES );
        }

    if (bOk)
    {
        ScDocShellModificator aModificator( *pDocSh );

        ScSubTotalParam aNewParam( rParam );        // change end of range
        ScDocument*     pUndoDoc = nullptr;
        ScOutlineTable* pUndoTab = nullptr;
        ScRangeName*    pUndoRange = nullptr;
        ScDBCollection* pUndoDB = nullptr;

        if (bRecord)                                        // record old data
        {
            bool bOldFilter = bDo && rParam.bDoSort;
            SCTAB nTabCount = rDoc.GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                SCCOLROW nOutStartCol;                          // row/column status
                SCCOLROW nOutStartRow;
                SCCOLROW nOutEndCol;
                SCCOLROW nOutEndRow;
                pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                rDoc.CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
                rDoc.CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, bOldFilter );

            // record data range - including filter results
            rDoc.CopyToDocument( 0,rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                    InsertDeleteFlags::ALL, false, *pUndoDoc );

            // all formulas for reference
            rDoc.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        InsertDeleteFlags::FORMULA, false, *pUndoDoc );

            // data base and othe ranges
            ScRangeName* pDocRange = rDoc.GetRangeName();
            if (!pDocRange->empty())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = rDoc.GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

        ScOutlineTable* pOut = rDoc.GetOutlineTable( nTab );
        if (pOut)
        {
            // Remove all existing outlines in the specified range.
            ScOutlineArray& rRowArray = pOut->GetRowArray();
            sal_uInt16 nDepth = rRowArray.GetDepth();
            for (sal_uInt16 i = 0; i < nDepth; ++i)
            {
                bool bSize;
                rRowArray.Remove(aNewParam.nRow1, aNewParam.nRow2, bSize);
            }
        }

        if (rParam.bReplace)
            rDoc.RemoveSubTotals( nTab, aNewParam );
        bool bSuccess = true;
        if (bDo)
        {
            // Sort
            if ( rParam.bDoSort || pForceNewSort )
            {
                pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

                // set subtotal fields before sorting
                // (duplicate values are dropped, so that they can be called again)

                ScSortParam aOldSort;
                pDBData->GetSortParam( aOldSort );
                ScSortParam aSortParam( aNewParam, pForceNewSort ? *pForceNewSort : aOldSort );
                Sort( aSortParam, false, false );
            }

            bSuccess = rDoc.DoSubTotals( nTab, aNewParam );
        }
        ScRange aDirtyRange( aNewParam.nCol1, aNewParam.nRow1, nTab,
            aNewParam.nCol2, aNewParam.nRow2, nTab );
        rDoc.SetDirty( aDirtyRange, true );

        if (bRecord)
        {
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSubTotals( pDocSh, nTab,
                                        rParam, aNewParam.nRow2,
                                        pUndoDoc, pUndoTab, // pUndoDBData,
                                        pUndoRange, pUndoDB ) );
        }

        if (!bSuccess)
        {
            // "Can not insert any rows"
            ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
        }

                                                    // store
        pDBData->SetSubTotalParam( aNewParam );
        pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );
        rDoc.CompileDBFormula();

        DoneBlockMode();
        InitOwnBlockMode();
        rMark.SetMarkArea( ScRange( aNewParam.nCol1,aNewParam.nRow1,nTab,
                                    aNewParam.nCol2,aNewParam.nRow2,nTab ) );
        MarkDataChanged();

        pDocSh->PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab),
                          PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);

        aModificator.SetDocumentModified();

        SelectionChanged();
    }
}

// consolidate

void ScDBFunc::Consolidate( const ScConsolidateParam& rParam )
{
    ScDocShell* pDocShell = GetViewData().GetDocShell();
    pDocShell->DoConsolidate( rParam );
    SetTabNo( rParam.nTab, true );
}

// pivot

static OUString lcl_MakePivotTabName( const OUString& rPrefix, SCTAB nNumber )
{
    OUString aName = rPrefix + OUString::number( nNumber );
    return aName;
}

bool ScDBFunc::MakePivotTable(
    const ScDPSaveData& rData, const ScRange& rDest, bool bNewTable,
    const ScDPObject& rSource )
{
    //  error message if no fields are set
    //  this must be removed when drag&drop of fields from a toolbox is available

    if ( rData.IsEmpty() )
    {
        ErrorMessage(STR_PIVOT_NODATA);
        return false;
    }

    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScDocument* pDoc    = GetViewData().GetDocument();
    bool bUndo = pDoc->IsUndoEnabled();

    ScRange aDestRange = rDest;
    if ( bNewTable )
    {
        SCTAB nSrcTab = GetViewData().GetTabNo();

        OUString aName( ScGlobal::GetRscString(STR_PIVOT_TABLE) );
        OUString aStr;

        pDoc->GetName( nSrcTab, aStr );
        aName += "_";
        aName += aStr;
        aName += "_";

        SCTAB nNewTab = nSrcTab+1;

        SCTAB i=1;
        while ( !pDoc->InsertTab( nNewTab, lcl_MakePivotTabName( aName, i ) ) && i <= MAXTAB )
            i++;

        bool bAppend = ( nNewTab+1 == pDoc->GetTableCount() );
        if (bUndo)
        {
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( pDocSh, nNewTab, bAppend, lcl_MakePivotTabName( aName, i ) ));
        }

        GetViewData().InsertTab( nNewTab );
        SetTabNo(nNewTab, true);

        aDestRange = ScRange( 0, 0, nNewTab );
    }

    ScDPObject* pDPObj = pDoc->GetDPAtCursor(
                            aDestRange.aStart.Col(), aDestRange.aStart.Row(), aDestRange.aStart.Tab() );

    ScDPObject aObj( rSource );
    aObj.SetOutRange( aDestRange );
    if ( pDPObj && !rData.GetExistingDimensionData() )
    {
        // copy dimension data from old object - lost in the dialog
        //! change the dialog to keep the dimension data

        ScDPSaveData aNewData( rData );
        const ScDPSaveData* pOldData = pDPObj->GetSaveData();
        if ( pOldData )
        {
            const ScDPDimensionSaveData* pDimSave = pOldData->GetExistingDimensionData();
            aNewData.SetDimensionData( pDimSave );
        }
        aObj.SetSaveData( aNewData );
    }
    else
        aObj.SetSaveData( rData );

    bool bAllowMove = (pDPObj != nullptr);   // allow re-positioning when editing existing table

    ScDBDocFunc aFunc( *pDocSh );
    bool bSuccess = aFunc.DataPilotUpdate(pDPObj, &aObj, true, false, bAllowMove);

    CursorPosChanged();     // shells may be switched

    if ( bNewTable )
    {
        pDocSh->PostPaintExtras();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    }

    return bSuccess;
}

void ScDBFunc::DeletePivotTable()
{
    ScDocShell* pDocSh    = GetViewData().GetDocShell();
    ScDocument& rDoc      = pDocSh->GetDocument();
    ScDPObject* pDPObj    = rDoc.GetDPAtCursor( GetViewData().GetCurX(),
                                                  GetViewData().GetCurY(),
                                                  GetViewData().GetTabNo() );
    if ( pDPObj )
    {
        ScDBDocFunc aFunc( *pDocSh );
        aFunc.RemovePivotTable(*pDPObj, true, false);
        CursorPosChanged();     // shells may be switched
    }
    else
        ErrorMessage(STR_PIVOT_NOTFOUND);
}

void ScDBFunc::RecalcPivotTable()
{
    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScDocument* pDoc    = GetViewData().GetDocument();

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor( GetViewData().GetCurX(),
                                                  GetViewData().GetCurY(),
                                                  GetViewData().GetTabNo() );
    if (pDPObj)
    {
        // Remove existing data cache for the data that this datapilot uses,
        // to force re-build data cache.
        ScDBDocFunc aFunc(*pDocSh);
        aFunc.RefreshPivotTables(pDPObj, false);

        CursorPosChanged();     // shells may be switched
    }
    else
        ErrorMessage(STR_PIVOT_NOTFOUND);
}

void ScDBFunc::GetSelectedMemberList(ScDPUniqueStringSet& rEntries, long& rDimension)
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if ( !pDPObj )
        return;

    long nStartDimension = -1;
    long nStartHierarchy = -1;
    long nStartLevel     = -1;

    ScRangeListRef xRanges;
    GetViewData().GetMultiArea( xRanges );         // incl. cursor if nothing is selected
    size_t nRangeCount = xRanges->size();
    bool bContinue = true;

    for (size_t nRangePos=0; nRangePos < nRangeCount && bContinue; nRangePos++)
    {
        ScRange aRange = *(*xRanges)[nRangePos];
        SCCOL nStartCol = aRange.aStart.Col();
        SCROW nStartRow = aRange.aStart.Row();
        SCCOL nEndCol = aRange.aEnd.Col();
        SCROW nEndRow = aRange.aEnd.Row();
        SCTAB nTab = aRange.aStart.Tab();

        for (SCROW nRow=nStartRow; nRow<=nEndRow && bContinue; nRow++)
            for (SCCOL nCol=nStartCol; nCol<=nEndCol && bContinue; nCol++)
            {
                sheet::DataPilotTableHeaderData aData;
                pDPObj->GetHeaderPositionData(ScAddress(nCol, nRow, nTab), aData);
                if ( aData.Dimension < 0 )
                    bContinue = false;              // not part of any dimension
                else
                {
                    if ( nStartDimension < 0 )      // first member?
                    {
                        nStartDimension = aData.Dimension;
                        nStartHierarchy = aData.Hierarchy;
                        nStartLevel     = aData.Level;
                    }
                    if ( aData.Dimension != nStartDimension ||
                         aData.Hierarchy != nStartHierarchy ||
                         aData.Level     != nStartLevel )
                    {
                        bContinue = false;          // cannot mix dimensions
                    }
                }
                if ( bContinue )
                {
                    // accept any part of a member description, also subtotals,
                    // but don't stop if empty parts are contained
                    if ( aData.Flags & sheet::MemberResultFlags::HASMEMBER )
                        rEntries.insert(aData.MemberName);
                }
            }
    }

    rDimension = nStartDimension;   // dimension from which the found members came
    if (!bContinue)
        rEntries.clear();         // remove all if not valid
}

bool ScDBFunc::HasSelectionForDateGroup( ScDPNumGroupInfo& rOldInfo, sal_Int32& rParts )
{
    // determine if the date group dialog has to be shown for the current selection

    bool bFound = false;

    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocument* pDoc = GetViewData().GetDocument();

    ScDPObject* pDPObj = pDoc->GetDPAtCursor( nCurX, nCurY, nTab );
    if ( pDPObj )
    {
        ScDPUniqueStringSet aEntries;
        long nSelectDimension = -1;
        GetSelectedMemberList( aEntries, nSelectDimension );

        if (!aEntries.empty())
        {
            bool bIsDataLayout;
            OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );
            OUString aBaseDimName( aDimName );

            bool bInGroupDim = false;
            bool bFoundParts = false;

            ScDPDimensionSaveData* pDimData =
                const_cast<ScDPDimensionSaveData*>( pDPObj->GetSaveData()->GetExistingDimensionData() );
            if ( pDimData )
            {
                const ScDPSaveNumGroupDimension* pNumGroupDim = pDimData->GetNumGroupDim( aDimName );
                const ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDim( aDimName );
                if ( pNumGroupDim )
                {
                    //  existing num group dimension

                    if ( pNumGroupDim->GetDatePart() != 0 )
                    {
                        //  dimension has date info -> edit settings of this dimension
                        //  (parts are collected below)

                        rOldInfo = pNumGroupDim->GetDateInfo();
                        bFound = true;
                    }
                    else if ( pNumGroupDim->GetInfo().mbDateValues )
                    {
                        //  Numerical grouping with DateValues flag is used for grouping
                        //  of days with a "Number of days" value.

                        rOldInfo = pNumGroupDim->GetInfo();
                        rParts = css::sheet::DataPilotFieldGroupBy::DAYS;               // not found in CollectDateParts
                        bFoundParts = true;
                        bFound = true;
                    }
                    bInGroupDim = true;
                }
                else if ( pGroupDim )
                {
                    //  existing additional group dimension

                    if ( pGroupDim->GetDatePart() != 0 )
                    {
                        //  dimension has date info -> edit settings of this dimension
                        //  (parts are collected below)

                        rOldInfo = pGroupDim->GetDateInfo();
                        aBaseDimName = pGroupDim->GetSourceDimName();
                        bFound = true;
                    }
                    bInGroupDim = true;
                }
            }
            if ( bFound && !bFoundParts )
            {
                // collect date parts from all group dimensions
                rParts = pDimData->CollectDateParts( aBaseDimName );
            }
            if ( !bFound && !bInGroupDim )
            {
                // create new date group dimensions if the selection is a single cell
                // in a normal dimension with date content

                ScRange aSelRange;
                if ( (GetViewData().GetSimpleArea( aSelRange ) == SC_MARK_SIMPLE) &&
                        aSelRange.aStart == aSelRange.aEnd )
                {
                    SCCOL nSelCol = aSelRange.aStart.Col();
                    SCROW nSelRow = aSelRange.aStart.Row();
                    SCTAB nSelTab = aSelRange.aStart.Tab();
                    if ( pDoc->HasValueData( nSelCol, nSelRow, nSelTab ) )
                    {
                        sal_uLong nIndex = static_cast<const SfxUInt32Item*>(pDoc->GetAttr(
                                        nSelCol, nSelRow, nSelTab, ATTR_VALUE_FORMAT))->GetValue();
                        short nType = pDoc->GetFormatTable()->GetType(nIndex);
                        if ( nType == css::util::NumberFormat::DATE || nType == css::util::NumberFormat::TIME || nType == css::util::NumberFormat::DATETIME )
                        {
                            bFound = true;
                            // use currently selected value for automatic limits
                            if( rOldInfo.mbAutoStart )
                                rOldInfo.mfStart = pDoc->GetValue( aSelRange.aStart );
                            if( rOldInfo.mbAutoEnd )
                                rOldInfo.mfEnd = pDoc->GetValue( aSelRange.aStart );
                        }
                    }
                }
            }
        }
    }

    return bFound;
}

bool ScDBFunc::HasSelectionForNumGroup( ScDPNumGroupInfo& rOldInfo )
{
    // determine if the numeric group dialog has to be shown for the current selection

    bool bFound = false;

    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocument* pDoc = GetViewData().GetDocument();

    ScDPObject* pDPObj = pDoc->GetDPAtCursor( nCurX, nCurY, nTab );
    if ( pDPObj )
    {
        ScDPUniqueStringSet aEntries;
        long nSelectDimension = -1;
        GetSelectedMemberList( aEntries, nSelectDimension );

        if (!aEntries.empty())
        {
            bool bIsDataLayout;
            OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );

            bool bInGroupDim = false;

            ScDPDimensionSaveData* pDimData =
                const_cast<ScDPDimensionSaveData*>( pDPObj->GetSaveData()->GetExistingDimensionData() );
            if ( pDimData )
            {
                const ScDPSaveNumGroupDimension* pNumGroupDim = pDimData->GetNumGroupDim( aDimName );
                if ( pNumGroupDim )
                {
                    //  existing num group dimension
                    //  -> edit settings of this dimension

                    rOldInfo = pNumGroupDim->GetInfo();
                    bFound = true;
                }
                else if ( pDimData->GetNamedGroupDim( aDimName ) )
                    bInGroupDim = true;                                    // in a group dimension
            }
            if ( !bFound && !bInGroupDim )
            {
                // create a new num group dimension if the selection is a single cell
                // in a normal dimension with numeric content

                ScRange aSelRange;
                if ( (GetViewData().GetSimpleArea( aSelRange ) == SC_MARK_SIMPLE) &&
                        aSelRange.aStart == aSelRange.aEnd )
                {
                    if ( pDoc->HasValueData( aSelRange.aStart.Col(), aSelRange.aStart.Row(),
                                             aSelRange.aStart.Tab() ) )
                    {
                        bFound = true;
                        // use currently selected value for automatic limits
                        if( rOldInfo.mbAutoStart )
                            rOldInfo.mfStart = pDoc->GetValue( aSelRange.aStart );
                        if( rOldInfo.mbAutoEnd )
                            rOldInfo.mfEnd = pDoc->GetValue( aSelRange.aStart );
                    }
                }
            }
        }
    }

    return bFound;
}

void ScDBFunc::DateGroupDataPilot( const ScDPNumGroupInfo& rInfo, sal_Int32 nParts )
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if (!pDPObj)
        return;

    ScDPUniqueStringSet aEntries;
    long nSelectDimension = -1;
    GetSelectedMemberList( aEntries, nSelectDimension );

    if (aEntries.empty())
        return;

    std::vector<OUString> aDeletedNames;
    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );

    ScDPSaveData aData( *pDPObj->GetSaveData() );
    ScDPDimensionSaveData* pDimData = aData.GetDimensionData();     // created if not there

    // find the source dimension name.
    OUString aBaseDimName = aDimName;
    if( const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName ) )
        aBaseDimName = pBaseGroupDim->GetSourceDimName();

    // Remove all group dimensions associated with this source dimension. For
    // date grouping, we need to remove all existing groups for the affected
    // source dimension and build new one(s) from scratch.  Keep the deleted
    // names so that they can be reused during re-construction.
    aData.RemoveAllGroupDimensions(aBaseDimName, &aDeletedNames);

    if ( nParts )
    {
        // create date group dimensions

        ScDPNumGroupInfo aEmpty;
        bool bFirst = true;
        sal_Int32 nMask = 1;
        for (sal_uInt16 nBit=0; nBit<32; nBit++)
        {
            if ( nParts & nMask )
            {
                if ( bFirst )
                {
                    // innermost part: create NumGroupDimension (replacing original values)
                    // Dimension name is left unchanged

                    if ( (nParts == sheet::DataPilotFieldGroupBy::DAYS) && (rInfo.mfStep >= 1.0) )
                    {
                        // only days, and a step value specified: use numerical grouping
                        // with DateValues flag, not date grouping

                        ScDPNumGroupInfo aNumInfo( rInfo );
                        aNumInfo.mbDateValues = true;

                        ScDPSaveNumGroupDimension aNumGroupDim( aBaseDimName, aNumInfo );
                        pDimData->AddNumGroupDimension( aNumGroupDim );
                    }
                    else
                    {
                        ScDPSaveNumGroupDimension aNumGroupDim( aBaseDimName, rInfo, nMask );
                        pDimData->AddNumGroupDimension( aNumGroupDim );
                    }

                    bFirst = false;
                }
                else
                {
                    // additional parts: create GroupDimension (shown as additional dimensions)
                    OUString aGroupDimName =
                        pDimData->CreateDateGroupDimName(nMask, *pDPObj, true, &aDeletedNames);
                    ScDPSaveGroupDimension aGroupDim( aBaseDimName, aGroupDimName );
                    aGroupDim.SetDateInfo( rInfo, nMask );
                    pDimData->AddGroupDimension( aGroupDim );

                    // set orientation
                    ScDPSaveDimension* pSaveDimension = aData.GetDimensionByName( aGroupDimName );
                    if ( pSaveDimension->GetOrientation() == sheet::DataPilotFieldOrientation_HIDDEN )
                    {
                        ScDPSaveDimension* pOldDimension = aData.GetDimensionByName( aBaseDimName );
                        pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
                        long nPosition = 0;     //! before (immediate) base
                        aData.SetPosition( pSaveDimension, nPosition );
                    }
                }
            }
            nMask *= 2;
        }
    }

    // apply changes
    ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
    pDPObj->SetSaveData( aData );
    aFunc.RefreshPivotTableGroups(pDPObj);

    // unmark cell selection
    Unmark();
}

void ScDBFunc::NumGroupDataPilot( const ScDPNumGroupInfo& rInfo )
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if (!pDPObj)
        return;

    ScDPUniqueStringSet aEntries;
    long nSelectDimension = -1;
    GetSelectedMemberList( aEntries, nSelectDimension );

    if (aEntries.empty())
        return;

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );

    ScDPSaveData aData( *pDPObj->GetSaveData() );
    ScDPDimensionSaveData* pDimData = aData.GetDimensionData();     // created if not there

    ScDPSaveNumGroupDimension* pExisting = pDimData->GetNumGroupDimAcc( aDimName );
    if ( pExisting )
    {
        // modify existing group dimension
        pExisting->SetGroupInfo( rInfo );
    }
    else
    {
        // create new group dimension
        ScDPSaveNumGroupDimension aNumGroupDim( aDimName, rInfo );
        pDimData->AddNumGroupDimension( aNumGroupDim );
    }

    // apply changes
    ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
    pDPObj->SetSaveData( aData );
    aFunc.RefreshPivotTableGroups(pDPObj);

    // unmark cell selection
    Unmark();
}

void ScDBFunc::GroupDataPilot()
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if (!pDPObj)
        return;

    ScDPUniqueStringSet aEntries;
    long nSelectDimension = -1;
    GetSelectedMemberList( aEntries, nSelectDimension );

    if (aEntries.empty())
        return;

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );

    ScDPSaveData aData( *pDPObj->GetSaveData() );
    ScDPDimensionSaveData* pDimData = aData.GetDimensionData();     // created if not there

    // find original base
    OUString aBaseDimName = aDimName;
    const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName );
    if ( pBaseGroupDim )
    {
        // any entry's SourceDimName is the original base
        aBaseDimName = pBaseGroupDim->GetSourceDimName();
    }

    // find existing group dimension
    // (using the selected dim, can be intermediate group dim)
    ScDPSaveGroupDimension* pGroupDimension = pDimData->GetGroupDimAccForBase( aDimName );

    // remove the selected items from their groups
    // (empty groups are removed, too)
    if ( pGroupDimension )
    {
        ScDPUniqueStringSet::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
        for (; it != itEnd; ++it)
        {
            const OUString& aEntryName = *it;
            if ( pBaseGroupDim )
            {
                // for each selected (intermediate) group, remove all its items
                // (same logic as for adding, below)
                const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                if ( pBaseGroup )
                    pBaseGroup->RemoveElementsFromGroups( *pGroupDimension );   // remove all elements
                else
                    pGroupDimension->RemoveFromGroups( aEntryName );
            }
            else
                pGroupDimension->RemoveFromGroups( aEntryName );
        }
    }

    ScDPSaveGroupDimension* pNewGroupDim = nullptr;
    if ( !pGroupDimension )
    {
        // create a new group dimension
        OUString aGroupDimName =
            pDimData->CreateGroupDimName(aBaseDimName, *pDPObj, false, nullptr);
        pNewGroupDim = new ScDPSaveGroupDimension( aBaseDimName, aGroupDimName );

        pGroupDimension = pNewGroupDim;     // make changes to the new dim if none existed

        if ( pBaseGroupDim )
        {
            // If it's a higher-order group dimension, pre-allocate groups for all
            // non-selected original groups, so the individual base members aren't
            // used for automatic groups (this would make the original groups hard
            // to find).
            //! Also do this when removing groups?
            //! Handle this case dynamically with automatic groups?

            long nGroupCount = pBaseGroupDim->GetGroupCount();
            for ( long nGroup = 0; nGroup < nGroupCount; nGroup++ )
            {
                const ScDPSaveGroupItem& rBaseGroup = pBaseGroupDim->GetGroupByIndex( nGroup );

                if (!aEntries.count(rBaseGroup.GetGroupName()))
                {
                    // add an additional group for each item that is not in the selection
                    ScDPSaveGroupItem aGroup( rBaseGroup.GetGroupName() );
                    aGroup.AddElementsFromGroup( rBaseGroup );
                    pGroupDimension->AddGroupItem( aGroup );
                }
            }
        }
    }
    OUString aGroupDimName = pGroupDimension->GetGroupDimName();

    OUString aGroupName = pGroupDimension->CreateGroupName(ScGlobal::GetRscString(STR_PIVOT_GROUP));
    ScDPSaveGroupItem aGroup( aGroupName );
    ScDPUniqueStringSet::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
    for (; it != itEnd; ++it)
    {
        const OUString& aEntryName = *it;
        if ( pBaseGroupDim )
        {
            // for each selected (intermediate) group, add all its items
            const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
            if ( pBaseGroup )
                aGroup.AddElementsFromGroup( *pBaseGroup );
            else
                aGroup.AddElement( aEntryName );    // no group found -> automatic group, add the item itself
        }
        else
            aGroup.AddElement( aEntryName );        // no group dimension, add all items directly
    }

    pGroupDimension->AddGroupItem( aGroup );

    if ( pNewGroupDim )
    {
        pDimData->AddGroupDimension( *pNewGroupDim );
        delete pNewGroupDim;        // AddGroupDimension copies the object
        // don't access pGroupDimension after here
    }
    pGroupDimension = pNewGroupDim = nullptr;

    // set orientation
    ScDPSaveDimension* pSaveDimension = aData.GetDimensionByName( aGroupDimName );
    if ( pSaveDimension->GetOrientation() == sheet::DataPilotFieldOrientation_HIDDEN )
    {
        ScDPSaveDimension* pOldDimension = aData.GetDimensionByName( aDimName );
        pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
        long nPosition = 0;     //! before (immediate) base
        aData.SetPosition( pSaveDimension, nPosition );
    }

    // apply changes
    ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
    pDPObj->SetSaveData( aData );
    aFunc.RefreshPivotTableGroups(pDPObj);

    // unmark cell selection
    Unmark();
}

void ScDBFunc::UngroupDataPilot()
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if (!pDPObj)
        return;

    ScDPUniqueStringSet aEntries;
    long nSelectDimension = -1;
    GetSelectedMemberList( aEntries, nSelectDimension );

    if (aEntries.empty())
        return;

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );

    ScDPSaveData aData( *pDPObj->GetSaveData() );
    if (!aData.GetExistingDimensionData())
        // There is nothing to ungroup.
        return;

    ScDPDimensionSaveData* pDimData = aData.GetDimensionData();

    ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDimAcc( aDimName );
    const ScDPSaveNumGroupDimension* pNumGroupDim = pDimData->GetNumGroupDim( aDimName );
    if ( ( pGroupDim && pGroupDim->GetDatePart() != 0 ) ||
         ( pNumGroupDim && pNumGroupDim->GetDatePart() != 0 ) )
    {
        // Date grouping: need to remove all affected group dimensions.
        // This is done using DateGroupDataPilot with nParts=0.

        DateGroupDataPilot( ScDPNumGroupInfo(), 0 );
        return;
    }

    if ( pGroupDim )
    {
        ScDPUniqueStringSet::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
        for (; it != itEnd; ++it)
            pGroupDim->RemoveGroup(*it);

        // remove group dimension if empty
        bool bEmptyDim = pGroupDim->IsEmpty();
        if ( !bEmptyDim )
        {
            // If all remaining groups in the dimension aren't shown, remove
            // the dimension too, as if it was completely empty.
            ScDPUniqueStringSet aVisibleEntries;
            pDPObj->GetMemberResultNames( aVisibleEntries, nSelectDimension );
            bEmptyDim = pGroupDim->HasOnlyHidden( aVisibleEntries );
        }
        if ( bEmptyDim )
        {
            pDimData->RemoveGroupDimension( aDimName );     // pGroupDim is deleted

            // also remove SaveData settings for the dimension that no longer exists
            aData.RemoveDimensionByName( aDimName );
        }
    }
    else if ( pNumGroupDim )
    {
        // remove the numerical grouping
        pDimData->RemoveNumGroupDimension( aDimName );
        // SaveData settings can remain unchanged - the same dimension still exists
    }

    // apply changes
    ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
    pDPObj->SetSaveData( aData );
    aFunc.RefreshPivotTableGroups(pDPObj);

    // unmark cell selection
    Unmark();
}

static OUString lcl_replaceMemberNameInSubtotal(const OUString& rSubtotal, const OUString& rMemberName)
{
    sal_Int32 n = rSubtotal.getLength();
    const sal_Unicode* p = rSubtotal.getStr();
    OUStringBuffer aBuf, aWordBuf;
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode c = p[i];
        if (c == ' ')
        {
            OUString aWord = aWordBuf.makeStringAndClear();
            if (aWord.equals(rMemberName))
                aBuf.append('?');
            else
                aBuf.append(aWord);
            aBuf.append(c);
        }
        else if (c == '\\')
        {
            // Escape a backslash character.
            aWordBuf.append(c);
            aWordBuf.append(c);
        }
        else if (c == '?')
        {
            // A literal '?' must be escaped with a backslash ('\');
            aWordBuf.append('\\');
            aWordBuf.append(c);
        }
        else
            aWordBuf.append(c);
    }

    if (!aWordBuf.isEmpty())
    {
        OUString aWord = aWordBuf.makeStringAndClear();
        if (aWord.equals(rMemberName))
            aBuf.append('?');
        else
            aBuf.append(aWord);
    }

    return aBuf.makeStringAndClear();
}

void ScDBFunc::DataPilotInput( const ScAddress& rPos, const OUString& rString )
{
    using namespace ::com::sun::star::sheet;

    ScDocument* pDoc = GetViewData().GetDocument();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor( rPos.Col(), rPos.Row(), rPos.Tab() );
    if (!pDPObj)
        return;

    OUString aOldText = pDoc->GetString(rPos.Col(), rPos.Row(), rPos.Tab());

    if ( aOldText == rString )
    {
        // nothing to do: silently exit
        return;
    }

    sal_uInt16 nErrorId = 0;

    pDPObj->BuildAllDimensionMembers();
    ScDPSaveData aData( *pDPObj->GetSaveData() );
    bool bChange = false;

    sal_uInt16 nOrient = DataPilotFieldOrientation_HIDDEN;
    long nField = pDPObj->GetHeaderDim( rPos, nOrient );
    if ( nField >= 0 )
    {
        // changing a field title
        if ( aData.GetExistingDimensionData() )
        {
            // only group dimensions can be renamed

            ScDPDimensionSaveData* pDimData = aData.GetDimensionData();
            ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDimAcc( aOldText );
            if ( pGroupDim )
            {
                // valid name: not empty, no existing dimension (group or other)
                if (!rString.isEmpty() && !pDPObj->IsDimNameInUse(rString))
                {
                    pGroupDim->Rename( rString );

                    // also rename in SaveData to preserve the field settings
                    ScDPSaveDimension* pSaveDim = aData.GetDimensionByName( aOldText );
                    pSaveDim->SetName( rString );

                    bChange = true;
                }
                else
                    nErrorId = STR_INVALIDNAME;
            }
        }
        else if (nOrient == DataPilotFieldOrientation_COLUMN || nOrient == DataPilotFieldOrientation_ROW)
        {
            bool bDataLayout = false;
            OUString aDimName = pDPObj->GetDimName(nField, bDataLayout);
            ScDPSaveDimension* pDim = bDataLayout ? aData.GetDataLayoutDimension() : aData.GetDimensionByName(aDimName);
            if (pDim)
            {
                if (!rString.isEmpty())
                {
                    if (rString.equalsIgnoreAsciiCase(aDimName))
                    {
                        pDim->RemoveLayoutName();
                        bChange = true;
                    }
                    else if (!pDPObj->IsDimNameInUse(rString))
                    {
                        pDim->SetLayoutName(rString);
                        bChange = true;
                    }
                    else
                        nErrorId = STR_INVALIDNAME;
                }
                else
                    nErrorId = STR_INVALIDNAME;
            }
        }
    }
    else if (pDPObj->IsDataDescriptionCell(rPos))
    {
        // There is only one data dimension.
        ScDPSaveDimension* pDim = aData.GetFirstDimension(sheet::DataPilotFieldOrientation_DATA);
        if (pDim)
        {
            if (!rString.isEmpty())
            {
                if (pDim->GetName().equalsIgnoreAsciiCase(rString))
                {
                    pDim->RemoveLayoutName();
                    bChange = true;
                }
                else if (!pDPObj->IsDimNameInUse(rString))
                {
                    pDim->SetLayoutName(rString);
                    bChange = true;
                }
                else
                    nErrorId = STR_INVALIDNAME;
            }
            else
                nErrorId = STR_INVALIDNAME;
        }
    }
    else
    {
        // This is not a field header.
        sheet::DataPilotTableHeaderData aPosData;
        pDPObj->GetHeaderPositionData(rPos, aPosData);

        if ((aPosData.Flags & MemberResultFlags::HASMEMBER) && !aOldText.isEmpty())
        {
            if ( aData.GetExistingDimensionData() && !(aPosData.Flags & MemberResultFlags::SUBTOTAL))
            {
                bool bIsDataLayout;
                OUString aDimName = pDPObj->GetDimName( aPosData.Dimension, bIsDataLayout );

                ScDPDimensionSaveData* pDimData = aData.GetDimensionData();
                ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDimAcc( aDimName );
                if ( pGroupDim )
                {
                    // valid name: not empty, no existing group in this dimension
                    //! ignore case?
                    if (!rString.isEmpty() && !pGroupDim->GetNamedGroup(rString))
                    {
                        ScDPSaveGroupItem* pGroup = pGroupDim->GetNamedGroupAcc( aOldText );
                        if ( pGroup )
                            pGroup->Rename( rString );     // rename the existing group
                        else
                        {
                            // create a new group to replace the automatic group
                            ScDPSaveGroupItem aGroup( rString );
                            aGroup.AddElement( aOldText );
                            pGroupDim->AddGroupItem( aGroup );
                        }

                        // in both cases also adjust savedata, to preserve member settings (show details)
                        ScDPSaveDimension* pSaveDim = aData.GetDimensionByName( aDimName );
                        ScDPSaveMember* pSaveMember = pSaveDim->GetExistingMemberByName( aOldText );
                        if ( pSaveMember )
                            pSaveMember->SetName( rString );

                        bChange = true;
                    }
                    else
                        nErrorId = STR_INVALIDNAME;
                 }
            }
            else if ((aPosData.Flags & MemberResultFlags::GRANDTOTAL))
            {
                aData.SetGrandTotalName(rString);
                bChange = true;
            }
            else if (aPosData.Dimension >= 0 && !aPosData.MemberName.isEmpty())
            {
                bool bDataLayout = false;
                OUString aDimName = pDPObj->GetDimName(static_cast<long>(aPosData.Dimension), bDataLayout);
                if (bDataLayout)
                {
                    // data dimension
                    do
                    {
                        if ((aPosData.Flags & MemberResultFlags::SUBTOTAL))
                            break;

                        ScDPSaveDimension* pDim = aData.GetDimensionByName(aPosData.MemberName);
                        if (!pDim)
                            break;

                        if (rString.isEmpty())
                        {
                            nErrorId = STR_INVALIDNAME;
                            break;
                        }

                        if (aPosData.MemberName.equalsIgnoreAsciiCase(rString))
                        {
                            pDim->RemoveLayoutName();
                            bChange = true;
                        }
                        else if (!pDPObj->IsDimNameInUse(rString))
                        {
                            pDim->SetLayoutName(rString);
                            bChange = true;
                        }
                        else
                            nErrorId = STR_INVALIDNAME;
                    }
                    while (false);
                }
                else
                {
                    // field member
                    do
                    {
                        ScDPSaveDimension* pDim = aData.GetDimensionByName(aDimName);
                        if (!pDim)
                            break;

                        ScDPSaveMember* pMem = pDim->GetExistingMemberByName(aPosData.MemberName);
                        if (!pMem)
                            break;

                        if ((aPosData.Flags & MemberResultFlags::SUBTOTAL))
                        {
                            // Change subtotal only when the table has one data dimension.
                            if (aData.GetDataDimensionCount() > 1)
                                break;

                            // display name for subtotal is allowed only if the subtotal type is 'Automatic'.
                            if (pDim->GetSubTotalsCount() != 1)
                                break;

                            if (pDim->GetSubTotalFunc(0) != sheet::GeneralFunction_AUTO)
                                break;

                            const OUString* pLayoutName = pMem->GetLayoutName();
                            OUString aMemberName;
                            if (pLayoutName)
                                aMemberName = *pLayoutName;
                            else
                                aMemberName = aPosData.MemberName;

                            OUString aNew = lcl_replaceMemberNameInSubtotal(rString, aMemberName);
                            pDim->SetSubtotalName(aNew);
                            bChange = true;
                        }
                        else
                        {
                            // Check to make sure the member name isn't
                            // already used.
                            if (!rString.isEmpty())
                            {
                                if (rString.equalsIgnoreAsciiCase(pMem->GetName()))
                                {
                                    pMem->RemoveLayoutName();
                                    bChange = true;
                                }
                                else if (!pDim->IsMemberNameInUse(rString))
                                {
                                    pMem->SetLayoutName(rString);
                                    bChange = true;
                                }
                                else
                                    nErrorId = STR_INVALIDNAME;
                            }
                            else
                                nErrorId = STR_INVALIDNAME;
                        }
                    }
                    while (false);
                }
            }
        }
    }

    if ( bChange )
    {
        // apply changes
        ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
        pDPObj->SetSaveData( aData );
        aFunc.UpdatePivotTable(*pDPObj, true, false);
    }
    else
    {
        if ( !nErrorId )
            nErrorId = STR_ERR_DATAPILOT_INPUT;
        ErrorMessage( nErrorId );
    }
}

static void lcl_MoveToEnd( ScDPSaveDimension& rDim, const OUString& rItemName )
{
    ScDPSaveMember* pNewMember = nullptr;
    const ScDPSaveMember* pOldMember = rDim.GetExistingMemberByName( rItemName );
    if ( pOldMember )
        pNewMember = new ScDPSaveMember( *pOldMember );
    else
        pNewMember = new ScDPSaveMember( rItemName );
    rDim.AddMember( pNewMember );
    // AddMember takes ownership of the new pointer,
    // puts it to the end of the list even if it was in the list before.
}

struct ScOUStringCollate
{
    CollatorWrapper* mpCollator;

    explicit ScOUStringCollate(CollatorWrapper* pColl) : mpCollator(pColl) {}

    bool operator()(const OUString& rStr1, const OUString& rStr2) const
    {
        return ( mpCollator->compareString(rStr1, rStr2) < 0 );
    }
};

void ScDBFunc::DataPilotSort(ScDPObject* pDPObj, long nDimIndex, bool bAscending, sal_uInt16* pUserListId)
{
    if (!pDPObj)
        return;

    // We need to run this to get all members later.
    if ( pUserListId )
        pDPObj->BuildAllDimensionMembers();

    if (nDimIndex < 0)
        // Invalid dimension index.  Bail out.
        return;

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    if (!pSaveData)
        return;

    ScDPSaveData aNewSaveData(*pSaveData);
    bool bDataLayout;
    OUString aDimName = pDPObj->GetDimName(nDimIndex, bDataLayout);
    ScDPSaveDimension* pSaveDim = aNewSaveData.GetDimensionByName(aDimName);
    if (!pSaveDim)
        return;

    // manual evaluation of sort order is only needed if a user list id is given
    if ( pUserListId )
    {
        typedef ScDPSaveDimension::MemberList MemList;
        const MemList& rDimMembers = pSaveDim->GetMembers();
        list<OUString> aMembers;
        std::unordered_set<OUString, OUStringHash> aMemberSet;
        size_t nMemberCount = 0;
        for (MemList::const_iterator itr = rDimMembers.begin(), itrEnd = rDimMembers.end();
              itr != itrEnd; ++itr)
        {
            ScDPSaveMember* pMem = *itr;
            aMembers.push_back(pMem->GetName());
            aMemberSet.insert(pMem->GetName());
            ++nMemberCount;
        }

        // Sort the member list in ascending order.
        ScOUStringCollate aCollate( ScGlobal::GetCollator() );
        aMembers.sort(aCollate);

        // Collect and rank those custom sort strings that also exist in the member name list.

        typedef std::unordered_map<OUString, sal_uInt16, OUStringHash> UserSortMap;
        UserSortMap aSubStrs;
        sal_uInt16 nSubCount = 0;
        if (pUserListId)
        {
            ScUserList* pUserList = ScGlobal::GetUserList();
            if (!pUserList)
                return;

            {
                size_t n = pUserList->size();
                if (!n || *pUserListId >= static_cast<sal_uInt16>(n))
                    return;
            }

            const ScUserListData& rData = (*pUserList)[*pUserListId];
            sal_uInt16 n = rData.GetSubCount();
            for (sal_uInt16 i = 0; i < n; ++i)
            {
                OUString aSub = rData.GetSubStr(i);
                if (!aMemberSet.count(aSub))
                    // This string doesn't exist in the member name set.  Don't add this.
                    continue;

                aSubStrs.insert(UserSortMap::value_type(aSub, nSubCount++));
            }
        }

        // Rank all members.

        vector<OUString> aRankedNames(nMemberCount);
        sal_uInt16 nCurStrId = 0;
        for (list<OUString>::const_iterator itr = aMembers.begin(), itrEnd = aMembers.end();
              itr != itrEnd; ++itr)
        {
            OUString aName = *itr;
            sal_uInt16 nRank = 0;
            UserSortMap::const_iterator itrSub = aSubStrs.find(aName);
            if (itrSub == aSubStrs.end())
                nRank = nSubCount + nCurStrId++;
            else
                nRank = itrSub->second;

            if (!bAscending)
                nRank = static_cast< sal_uInt16 >( nMemberCount - nRank - 1 );

            aRankedNames[nRank] = aName;
        }

        // Re-order ScDPSaveMember instances with the new ranks.

        for (vector<OUString>::const_iterator itr = aRankedNames.begin(), itrEnd = aRankedNames.end();
              itr != itrEnd; ++itr)
        {
            const ScDPSaveMember* pOldMem = pSaveDim->GetExistingMemberByName(*itr);
            if (!pOldMem)
                // All members are supposed to be present.
                continue;

            ScDPSaveMember* pNewMem = new ScDPSaveMember(*pOldMem);
            pSaveDim->AddMember(pNewMem);
        }

        // Set the sorting mode to manual for now.  We may introduce a new sorting
        // mode later on.

        sheet::DataPilotFieldSortInfo aSortInfo;
        aSortInfo.Mode = sheet::DataPilotFieldSortMode::MANUAL;
        pSaveDim->SetSortInfo(&aSortInfo);
    }
    else
    {
        // without user list id, just apply sorting mode

        sheet::DataPilotFieldSortInfo aSortInfo;
        aSortInfo.Mode = sheet::DataPilotFieldSortMode::NAME;
        aSortInfo.IsAscending = bAscending;
        pSaveDim->SetSortInfo(&aSortInfo);
    }

    // Update the datapilot with the newly sorted field members.

    std::unique_ptr<ScDPObject> pNewObj(new ScDPObject(*pDPObj));
    pNewObj->SetSaveData(aNewSaveData);
    ScDBDocFunc aFunc(*GetViewData().GetDocShell());

    aFunc.DataPilotUpdate(pDPObj, pNewObj.get(), true, false);
}

void ScDBFunc::DataPilotSort( const ScAddress& rPos, bool bAscending, sal_uInt16* pUserListId )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(rPos.Col(), rPos.Row(), rPos.Tab());
    if (!pDPObj)
        return;

    sal_uInt16 nOrientation;
    long nDimIndex = pDPObj->GetHeaderDim(rPos, nOrientation);
    if (nDimIndex < 0)
        // Invalid dimension index.  Bail out.
        return;

    DataPilotSort(pDPObj, nDimIndex, bAscending, pUserListId);
}

bool ScDBFunc::DataPilotMove( const ScRange& rSource, const ScAddress& rDest )
{
    bool bRet = false;
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor( rSource.aStart.Col(), rSource.aStart.Row(), rSource.aStart.Tab() );
    if ( pDPObj && pDPObj == pDoc->GetDPAtCursor( rDest.Col(), rDest.Row(), rDest.Tab() ) )
    {
        sheet::DataPilotTableHeaderData aDestData;
        pDPObj->GetHeaderPositionData( rDest, aDestData );
        bool bValid = ( aDestData.Dimension >= 0 );        // dropping onto a field

        // look through the source range
        std::unordered_set< OUString, OUStringHash > aMembersSet;   // for lookup
        std::vector< OUString > aMembersVector;  // members in original order, for inserting
        aMembersVector.reserve( std::max( static_cast<SCSIZE>( rSource.aEnd.Col() - rSource.aStart.Col() + 1 ),
                                          static_cast<SCSIZE>( rSource.aEnd.Row() - rSource.aStart.Row() + 1 ) ) );
        for (SCROW nRow = rSource.aStart.Row(); bValid && nRow <= rSource.aEnd.Row(); ++nRow )
            for (SCCOL nCol = rSource.aStart.Col(); bValid && nCol <= rSource.aEnd.Col(); ++nCol )
            {
                sheet::DataPilotTableHeaderData aSourceData;
                pDPObj->GetHeaderPositionData( ScAddress( nCol, nRow, rSource.aStart.Tab() ), aSourceData );
                if ( aSourceData.Dimension == aDestData.Dimension && !aSourceData.MemberName.isEmpty() )
                {
                    if ( aMembersSet.find( aSourceData.MemberName ) == aMembersSet.end() )
                    {
                        aMembersSet.insert( aSourceData.MemberName );
                        aMembersVector.push_back( aSourceData.MemberName );
                    }
                    // duplicates are ignored
                }
                else
                    bValid = false;     // empty (subtotal) or different field
            }

        if ( bValid )
        {
            bool bIsDataLayout;
            OUString aDimName = pDPObj->GetDimName( aDestData.Dimension, bIsDataLayout );
            if ( !bIsDataLayout )
            {
                ScDPSaveData aData( *pDPObj->GetSaveData() );
                ScDPSaveDimension* pDim = aData.GetDimensionByName( aDimName );

                // get all member names in source order
                uno::Sequence<OUString> aMemberNames;
                pDPObj->GetMemberNames( aDestData.Dimension, aMemberNames );

                bool bInserted = false;

                sal_Int32 nMemberCount = aMemberNames.getLength();
                for (sal_Int32 nMemberPos=0; nMemberPos<nMemberCount; ++nMemberPos)
                {
                    OUString aMemberStr( aMemberNames[nMemberPos] );

                    if ( !bInserted && aMemberNames[nMemberPos] == aDestData.MemberName )
                    {
                        // insert dragged items before this item
                        for ( std::vector<OUString>::const_iterator aIter = aMembersVector.begin();
                              aIter != aMembersVector.end(); ++aIter )
                            lcl_MoveToEnd( *pDim, *aIter );
                        bInserted = true;
                    }

                    if ( aMembersSet.find( aMemberStr ) == aMembersSet.end() )  // skip dragged items
                        lcl_MoveToEnd( *pDim, aMemberStr );
                }
                // insert dragged item at end if dest wasn't found (for example, empty)
                if ( !bInserted )
                    for ( std::vector<OUString>::const_iterator aIter = aMembersVector.begin();
                          aIter != aMembersVector.end(); ++aIter )
                        lcl_MoveToEnd( *pDim, *aIter );

                // Items that were in SaveData, but not in the source, end up at the start of the list.

                // set flag for manual sorting
                sheet::DataPilotFieldSortInfo aSortInfo;
                aSortInfo.Mode = sheet::DataPilotFieldSortMode::MANUAL;
                pDim->SetSortInfo( &aSortInfo );

                // apply changes
                ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
                std::unique_ptr<ScDPObject> pNewObj(new ScDPObject( *pDPObj ));
                pNewObj->SetSaveData( aData );
                aFunc.DataPilotUpdate( pDPObj, pNewObj.get(), true, false );      //! bApi for drag&drop?
                pNewObj.reset();

                Unmark();       // entry was moved - no use in leaving the old cell selected

                bRet = true;
            }
        }
    }

    return bRet;
}

bool ScDBFunc::HasSelectionForDrillDown( sal_uInt16& rOrientation )
{
    bool bRet = false;

    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if ( pDPObj )
    {
        ScDPUniqueStringSet aEntries;
        long nSelectDimension = -1;
        GetSelectedMemberList( aEntries, nSelectDimension );

        if (!aEntries.empty())
        {
            bool bIsDataLayout;
            OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );
            if ( !bIsDataLayout )
            {
                ScDPSaveData* pSaveData = pDPObj->GetSaveData();
                ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName( aDimName );
                if ( pDim )
                {
                    sal_uInt16 nDimOrient = pDim->GetOrientation();
                    ScDPSaveDimension* pInner = pSaveData->GetInnermostDimension( nDimOrient );
                    if ( pDim == pInner )
                    {
                        rOrientation = nDimOrient;
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

void ScDBFunc::SetDataPilotDetails(bool bShow, const OUString* pNewDimensionName)
{
    ScDPObject* pDPObj = GetViewData().GetDocument()->GetDPAtCursor( GetViewData().GetCurX(),
                                        GetViewData().GetCurY(), GetViewData().GetTabNo() );
    if ( pDPObj )
    {
        ScDPUniqueStringSet aEntries;
        long nSelectDimension = -1;
        GetSelectedMemberList( aEntries, nSelectDimension );

        if (!aEntries.empty())
        {
            bool bIsDataLayout;
            OUString aDimName = pDPObj->GetDimName( nSelectDimension, bIsDataLayout );
            if ( !bIsDataLayout )
            {
                ScDPSaveData aData( *pDPObj->GetSaveData() );
                ScDPSaveDimension* pDim = aData.GetDimensionByName( aDimName );

                if ( bShow && pNewDimensionName )
                {
                    //  add the new dimension with the same orientation, at the end

                    ScDPSaveDimension* pNewDim = aData.GetDimensionByName( *pNewDimensionName );
                    ScDPSaveDimension* pDuplicated = nullptr;
                    if ( pNewDim->GetOrientation() == sheet::DataPilotFieldOrientation_DATA )
                    {
                        // Need to duplicate the dimension, create column/row in addition to data:
                        // The duplicated dimension inherits the existing settings, pNewDim is modified below.
                        pDuplicated = aData.DuplicateDimension( *pNewDimensionName );
                    }

                    sal_uInt16 nOrientation = pDim->GetOrientation();
                    pNewDim->SetOrientation( nOrientation );

                    long nPosition = LONG_MAX;
                    aData.SetPosition( pNewDim, nPosition );

                    ScDPSaveDimension* pDataLayout = aData.GetDataLayoutDimension();
                    if ( pDataLayout->GetOrientation() == nOrientation &&
                         aData.GetDataDimensionCount() <= 1 )
                    {
                        // If there is only one data dimension, the data layout dimension
                        // must still be the last one in its orientation.
                        aData.SetPosition( pDataLayout, nPosition );
                    }

                    if ( pDuplicated )
                    {
                        // The duplicated (data) dimension needs to be behind the original dimension
                        aData.SetPosition( pDuplicated, nPosition );
                    }

                    //  Hide details for all visible members (selected are changed below).
                    //! Use all members from source level instead (including non-visible)?

                    ScDPUniqueStringSet aVisibleEntries;
                    pDPObj->GetMemberResultNames( aVisibleEntries, nSelectDimension );

                    ScDPUniqueStringSet::const_iterator it = aVisibleEntries.begin(), itEnd = aVisibleEntries.end();
                    for (; it != itEnd; ++it)
                    {
                        const OUString& aVisName = *it;
                        ScDPSaveMember* pMember = pDim->GetMemberByName( aVisName );
                        pMember->SetShowDetails( false );
                    }
                }

                ScDPUniqueStringSet::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
                for (; it != itEnd; ++it)
                {
                    ScDPSaveMember* pMember = pDim->GetMemberByName(*it);
                    pMember->SetShowDetails( bShow );
                }

                // apply changes
                ScDBDocFunc aFunc( *GetViewData().GetDocShell() );
                std::unique_ptr<ScDPObject> pNewObj(new ScDPObject( *pDPObj ));
                pNewObj->SetSaveData( aData );
                aFunc.DataPilotUpdate( pDPObj, pNewObj.get(), true, false );
                pNewObj.reset();

                // unmark cell selection
                Unmark();
            }
        }
    }
}

void ScDBFunc::ShowDataPilotSourceData( ScDPObject& rDPObj, const Sequence<sheet::DataPilotFieldFilter>& rFilters )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    if (pDoc->GetDocumentShell()->IsReadOnly())
    {
        ErrorMessage(STR_READONLYERR);
        return;
    }

    Reference<sheet::XDimensionsSupplier> xDimSupplier = rDPObj.GetSource();
    Reference<container::XNameAccess> xDims = xDimSupplier->getDimensions();
    Reference<sheet::XDrillDownDataSupplier> xDDSupplier(xDimSupplier, UNO_QUERY);
    if (!xDDSupplier.is())
        return;

    Sequence< Sequence<Any> > aTabData = xDDSupplier->getDrillDownData(rFilters);
    sal_Int32 nRowSize = aTabData.getLength();
    if (nRowSize <= 1)
        // There is no data to show.  Bail out.
        return;

    SCCOL nColSize = aTabData[0].getLength();

    SCTAB nNewTab = GetViewData().GetTabNo();

    std::unique_ptr<ScDocument> pInsDoc(new ScDocument(SCDOCMODE_CLIP));
    pInsDoc->ResetClip( pDoc, nNewTab );
    for (SCROW nRow = 0; nRow < nRowSize; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < nColSize; ++nCol)
        {
            const Any& rAny = aTabData[nRow][nCol];
            OUString aStr;
            double fVal;
            if (rAny >>= aStr)
            {
                ScSetStringParam aParam;
                aParam.setTextInput();
                pInsDoc->SetString(ScAddress(nCol,nRow,nNewTab), aStr);
            }
            else if (rAny >>= fVal)
                pInsDoc->SetValue(nCol, nRow, nNewTab, fVal);
        }
    }

    // set number format (important for dates)
    for (SCCOL nCol = 0; nCol < nColSize; ++nCol)
    {
        OUString aStr;
        if (!(aTabData[0][nCol] >>= aStr))
            continue;

        Reference<XPropertySet> xPropSet(xDims->getByName(aStr), UNO_QUERY);
        if (!xPropSet.is())
            continue;

        Any any = xPropSet->getPropertyValue( SC_UNO_DP_NUMBERFO );
        sal_Int32 nNumFmt = 0;
        if (!(any >>= nNumFmt))
            continue;

        ScPatternAttr aPattern( pInsDoc->GetPool() );
        aPattern.GetItemSet().Put( SfxUInt32Item(ATTR_VALUE_FORMAT, static_cast<sal_uInt32>(nNumFmt)) );
        pInsDoc->ApplyPatternAreaTab(nCol, 1, nCol, nRowSize-1, nNewTab, aPattern);
    }

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    pInsDoc->GetCellArea( nNewTab, nEndCol, nEndRow );
    pInsDoc->SetClipArea( ScRange( 0, 0, nNewTab, nEndCol, nEndRow, nNewTab ) );

    ::svl::IUndoManager* pMgr = GetViewData().GetDocShell()->GetUndoManager();
    OUString aUndo = ScGlobal::GetRscString( STR_UNDO_DOOUTLINE );
    pMgr->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );

    OUString aNewTabName;
    pDoc->CreateValidTabName(aNewTabName);
    if ( InsertTable(aNewTabName, nNewTab) )
        PasteFromClip( InsertDeleteFlags::ALL, pInsDoc.get() );

    pMgr->LeaveListAction();
}

// repeat data base operations (sorting, filtering, subtotals)

void ScDBFunc::RepeatDB( bool bRecord )
{
    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDBData* pDBData = GetDBData();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    bool bQuery = aQueryParam.GetEntry(0).bDoQuery;

    ScSortParam aSortParam;
    pDBData->GetSortParam( aSortParam );
    bool bSort = aSortParam.maKeyState[0].bDoSort;

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    bool bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;

    if ( bQuery || bSort || bSubTotal )
    {
        bool bQuerySize = false;
        ScRange aOldQuery;
        ScRange aNewQuery;
        if (bQuery && !aQueryParam.bInplace)
        {
            ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                    aQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
            if (pDest && pDest->IsDoSize())
            {
                pDest->GetArea( aOldQuery );
                bQuerySize = true;
            }
        }

        SCTAB nDummy;
        SCCOL nStartCol;
        SCROW nStartRow;
        SCCOL nEndCol;
        SCROW nEndRow;
        pDBData->GetArea( nDummy, nStartCol, nStartRow, nEndCol, nEndRow );

        //! undo only needed data ?

        ScDocument* pUndoDoc = nullptr;
        ScOutlineTable* pUndoTab = nullptr;
        ScRangeName* pUndoRange = nullptr;
        ScDBCollection* pUndoDB = nullptr;

        if (bRecord)
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                SCCOLROW nOutStartCol;                          // row/column status
                SCCOLROW nOutStartRow;
                SCCOLROW nOutEndCol;
                SCCOLROW nOutEndRow;
                pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( pDoc, nTab, nTab, true, true );
                pDoc->CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
                pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( pDoc, nTab, nTab, false, true );

            // Record data range - including filter results
            pDoc->CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, InsertDeleteFlags::ALL, false, *pUndoDoc );

            // all formulas for reference
            pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, InsertDeleteFlags::FORMULA, false, *pUndoDoc );

            // data base and other ranges
            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (!pDocRange->empty())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

        if (bSort && bSubTotal)
        {
            // sort without subtotals

            aSubTotalParam.bRemoveOnly = true;      // is resetted below
            DoSubTotals( aSubTotalParam, false );
        }

        if (bSort)
        {
            pDBData->GetSortParam( aSortParam );            // range may have chaged
            Sort( aSortParam, false, false);
        }
        if (bQuery)
        {
            pDBData->GetQueryParam( aQueryParam );          // range may have chaged
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
            {
                pDoc->CreateQueryParam(
                    aAdvSource.aStart.Col(), aAdvSource.aStart.Row(),
                    aAdvSource.aEnd.Col(), aAdvSource.aEnd.Row(),
                    aAdvSource.aStart.Tab(), aQueryParam );
                Query( aQueryParam, &aAdvSource, false );
            }
            else
                Query( aQueryParam, nullptr, false );

            // if not inplace the sheet may have changed
            if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
                SetTabNo( nTab );
        }
        if (bSubTotal)
        {
            pDBData->GetSubTotalParam( aSubTotalParam );    // range may have chaged
            aSubTotalParam.bRemoveOnly = false;
            DoSubTotals( aSubTotalParam, false );
        }

        if (bRecord)
        {
            SCTAB nDummyTab;
            SCCOL nDummyCol;
            SCROW nDummyRow, nNewEndRow;
            pDBData->GetArea( nDummyTab, nDummyCol,nDummyRow, nDummyCol,nNewEndRow );

            const ScRange* pOld = nullptr;
            const ScRange* pNew = nullptr;
            if (bQuerySize)
            {
                ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                        aQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
                if (pDest)
                {
                    pDest->GetArea( aNewQuery );
                    pOld = &aOldQuery;
                    pNew = &aNewQuery;
                }
            }

            GetViewData().GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoRepeatDB( GetViewData().GetDocShell(), nTab,
                                        nStartCol, nStartRow, nEndCol, nEndRow,
                                        nNewEndRow,
                                        nCurX, nCurY,
                                        pUndoDoc, pUndoTab,
                                        pUndoRange, pUndoDB,
                                        pOld, pNew ) );
        }

        GetViewData().GetDocShell()->PostPaint(
            ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab),
            PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);
    }
    else        // "no not execute any operations"
        ErrorMessage(STR_MSSG_REPEATDB_0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
