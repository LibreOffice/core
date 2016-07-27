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

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>

#include "dbfunc.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "sc.hrc"
#include "undodat.hxx"
#include "dbdata.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"
#include "editable.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"

ScDBFunc::ScDBFunc( vcl::Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScViewFunc( pParent, rDocSh, pViewShell )
{
}

ScDBFunc::~ScDBFunc()
{
}

//      auxiliary functions

void ScDBFunc::GotoDBArea( const OUString& rDBName )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDBCollection* pDBCol = pDoc->GetDBCollection();
    ScDBData* pData = pDBCol->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rDBName));
    if (pData)
    {
        SCTAB nTab = 0;
        SCCOL nStartCol = 0;
        SCROW nStartRow = 0;
        SCCOL nEndCol = 0;
        SCROW nEndRow = 0;

        pData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
        SetTabNo( nTab );

        MoveCursorAbs( nStartCol, nStartRow, ScFollowMode( SC_FOLLOW_JUMP ),
                       false, false );  // bShift,bControl
        DoneBlockMode();
        InitBlockMode( nStartCol, nStartRow, nTab );
        MarkCursor( nEndCol, nEndRow, nTab );
        SelectionChanged();
    }
}

//  search current datarange for sort / filter

ScDBData* ScDBFunc::GetDBData( bool bMark, ScGetDBMode eMode, ScGetDBSelection eSel )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDBData* pData = nullptr;
    ScRange aRange;
    ScMarkType eMarkType = GetViewData().GetSimpleArea(aRange);
    if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
        bool bShrinkColumnsOnly = false;
        if (eSel == SC_DBSEL_ROW_DOWN)
        {
            // Don't alter row range, additional rows may have been selected on
            // purpose to append data, or to have a fake header row.
            bShrinkColumnsOnly = true;
            // Select further rows only if only one row or a portion thereof is
            // selected.
            if (aRange.aStart.Row() != aRange.aEnd.Row())
            {
                // If an area is selected shrink that to the actual used
                // columns, don't draw filter buttons for empty columns.
                eSel = SC_DBSEL_SHRINK_TO_USED_DATA;
            }
            else if (aRange.aStart.Col() == aRange.aEnd.Col())
            {
                // One cell only, if it is not marked obtain entire used data
                // area.
                const ScMarkData& rMarkData = GetViewData().GetMarkData();
                if (!(rMarkData.IsMarked() || rMarkData.IsMultiMarked()))
                    eSel = SC_DBSEL_KEEP;
            }
        }
        switch (eSel)
        {
            case SC_DBSEL_SHRINK_TO_SHEET_DATA:
                {
                    // Shrink the selection to sheet data area.
                    ScDocument& rDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    if (rDoc.ShrinkToDataArea( aRange.aStart.Tab(), nCol1, nRow1, nCol2, nRow2))
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            case SC_DBSEL_SHRINK_TO_USED_DATA:
            case SC_DBSEL_ROW_DOWN:
                {
                    // Shrink the selection to actual used area.
                    ScDocument& rDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    bool bShrunk;
                    rDoc.ShrinkToUsedDataArea( bShrunk, aRange.aStart.Tab(),
                            nCol1, nRow1, nCol2, nRow2, bShrinkColumnsOnly);
                    if (bShrunk)
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            default:
                ;   // nothing
        }
        pData = pDocSh->GetDBData( aRange, eMode, eSel );
    }
    else if ( eMode != SC_DB_OLD )
        pData = pDocSh->GetDBData(
                    ScRange( GetViewData().GetCurX(), GetViewData().GetCurY(),
                             GetViewData().GetTabNo() ),
                    eMode, SC_DBSEL_KEEP );

    if (!pData)
        return nullptr;

    if (bMark)
    {
        ScRange aFound;
        pData->GetArea(aFound);
        MarkRange( aFound, false );
    }
    return pData;
}

ScDBData* ScDBFunc::GetAnonymousDBData()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScRange aRange;
    ScMarkType eMarkType = GetViewData().GetSimpleArea(aRange);
    if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
        return nullptr;

    // Expand to used data area if not explicitly marked.
    const ScMarkData& rMarkData = GetViewData().GetMarkData();
    if (!rMarkData.IsMarked() && !rMarkData.IsMultiMarked())
    {
        SCCOL nCol1 = aRange.aStart.Col();
        SCCOL nCol2 = aRange.aEnd.Col();
        SCROW nRow1 = aRange.aStart.Row();
        SCROW nRow2 = aRange.aEnd.Row();
        pDocSh->GetDocument().GetDataArea(aRange.aStart.Tab(), nCol1, nRow1, nCol2, nRow2, false, false);
        aRange.aStart.SetCol(nCol1);
        aRange.aStart.SetRow(nRow1);
        aRange.aEnd.SetCol(nCol2);
        aRange.aEnd.SetRow(nRow2);
    }

    return pDocSh->GetAnonymousDBData(aRange);
}

//      main functions

// Sort

void ScDBFunc::UISort( const ScSortParam& rSortParam )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: no DBData" );
        return;
    }

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    if (aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly)
    {
        //  repeat subtotals, with new sortorder

        DoSubTotals( aSubTotalParam, true/*bRecord*/, &rSortParam );
    }
    else
    {
        Sort( rSortParam );        // just sort
    }
}

void ScDBFunc::Sort( const ScSortParam& rSortParam, bool bRecord, bool bPaint )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    bool bSuccess = aDBDocFunc.Sort( nTab, rSortParam, bRecord, bPaint, false );
    if ( bSuccess && !rSortParam.bInplace )
    {
        //  mark target
        ScRange aDestRange( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab,
                            rSortParam.nDestCol + rSortParam.nCol2 - rSortParam.nCol1,
                            rSortParam.nDestRow + rSortParam.nRow2 - rSortParam.nRow1,
                            rSortParam.nDestTab );
        MarkRange( aDestRange );
    }

    ResetAutoSpell();
}

//  filters

void ScDBFunc::Query( const ScQueryParam& rQueryParam, const ScRange* pAdvSource, bool bRecord )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    bool bSuccess = aDBDocFunc.Query( nTab, rQueryParam, pAdvSource, bRecord, false );

    if (bSuccess)
    {
        bool bCopy = !rQueryParam.bInplace;
        if (bCopy)
        {
            //  mark target range (data base range has been set up if applicable)
            ScDocument& rDoc = pDocSh->GetDocument();
            ScDBData* pDestData = rDoc.GetDBAtCursor(
                                            rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
            if (pDestData)
            {
                ScRange aDestRange;
                pDestData->GetArea(aDestRange);
                MarkRange( aDestRange );
            }
        }

        if (!bCopy)
        {
            UpdateScrollBars();
            SelectionChanged();     // for attribute states (filtered rows are ignored)
        }

        GetViewData().GetBindings().Invalidate( SID_UNFILTER );
    }
}

//  autofilter-buttons show / hide

void ScDBFunc::ToggleAutoFilter()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScQueryParam    aParam;
    ScDocument*     pDoc    = GetViewData().GetDocument();
    ScDBData*       pDBData = GetDBData(false, SC_DB_AUTOFILTER, SC_DBSEL_ROW_DOWN);

    pDBData->SetByRow( true );              //! undo, retrieve beforehand ??
    pDBData->GetQueryParam( aParam );

    SCCOL  nCol;
    SCROW  nRow = aParam.nRow1;
    SCTAB  nTab = GetViewData().GetTabNo();
    ScMF   nFlag;
    bool   bHasAuto = true;
    bool   bHeader  = pDBData->HasHeader();
    bool   bPaint   = false;

    //!     instead retrieve from DB-range?

    for (nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAuto; nCol++)
    {
        nFlag = static_cast<const ScMergeFlagAttr*>( pDoc->
                GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();

        if ( !(nFlag & ScMF::Auto) )
            bHasAuto = false;
    }

    if (bHasAuto)                               // remove
    {
        //  hide filter buttons

        for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
        {
            nFlag = static_cast<const ScMergeFlagAttr*>( pDoc->
                    GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag & ~ScMF::Auto ) );
        }

        // use a list action for the AutoFilter buttons (ScUndoAutoFilter) and the filter operation

        OUString aUndo = ScGlobal::GetRscString( STR_UNDO_QUERY );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

        ScRange aRange;
        pDBData->GetArea( aRange );
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), false ) );

        pDBData->SetAutoFilter(false);

        //  remove filter (incl. Paint / Undo)

        SCSIZE nEC = aParam.GetEntryCount();
        for (SCSIZE i=0; i<nEC; i++)
            aParam.GetEntry(i).bDoQuery = false;
        aParam.bDuplicate = true;
        Query( aParam, nullptr, true );

        pDocSh->GetUndoManager()->LeaveListAction();

        bPaint = true;
    }
    else                                    // show filter buttons
    {
        if ( !pDoc->IsBlockEmpty( nTab,
                                  aParam.nCol1, aParam.nRow1,
                                  aParam.nCol2, aParam.nRow2 ) )
        {
            if (!bHeader)
            {
                if ( ScopedVclPtrInstance<MessBox>( GetViewData().GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       // "StarCalc"
                        ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 )     // header from first row?
                    )->Execute() == RET_YES )
                {
                    pDBData->SetHeader( true );     //! Undo ??
                }
            }

            ScRange aRange;
            pDBData->GetArea( aRange );
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), true ) );

            pDBData->SetAutoFilter(true);

            for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
            {
                nFlag = static_cast<const ScMergeFlagAttr*>( pDoc->
                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
                pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag | ScMF::Auto ) );
            }
            pDocSh->PostPaint(ScRange(aParam.nCol1, nRow, nTab, aParam.nCol2, nRow, nTab),
                              PAINT_GRID);
            bPaint = true;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog> aErrorBox(GetViewData().GetDialogParent(),
                                ScGlobal::GetRscString(STR_ERR_AUTOFILTER));
            aErrorBox->Execute();
        }
    }

    if ( bPaint )
    {
        aModificator.SetDocumentModified();

        SfxBindings& rBindings = GetViewData().GetBindings();
        rBindings.Invalidate( SID_AUTO_FILTER );
        rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    }
}

//      just hide, no data change

void ScDBFunc::HideAutoFilter()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScDocument& rDoc = pDocSh->GetDocument();

    ScQueryParam aParam;
    ScDBData* pDBData = GetDBData( false );

    SCTAB nTab;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    pDBData->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);

    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
    {
        ScMF nFlag = static_cast<const ScMergeFlagAttr*>( rDoc.
                                GetAttr( nCol, nRow1, nTab, ATTR_MERGE_FLAG ))->GetValue();
        rDoc.ApplyAttr( nCol, nRow1, nTab, ScMergeFlagAttr( nFlag & ~ScMF::Auto ) );
    }

    ScRange aRange;
    pDBData->GetArea( aRange );
    pDocSh->GetUndoManager()->AddUndoAction(
        new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), false ) );

    pDBData->SetAutoFilter(false);

    pDocSh->PostPaint(ScRange(nCol1, nRow1, nTab, nCol2, nRow1, nTab), PAINT_GRID );
    aModificator.SetDocumentModified();

    SfxBindings& rBindings = GetViewData().GetBindings();
    rBindings.Invalidate( SID_AUTO_FILTER );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
}

//      Re-Import

bool ScDBFunc::ImportData( const ScImportParam& rParam )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScEditableTester aTester( pDoc, GetViewData().GetTabNo(), rParam.nCol1,rParam.nRow1,
                                                            rParam.nCol2,rParam.nRow2 );
    if ( !aTester.IsEditable() )
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScDBDocFunc aDBDocFunc( *GetViewData().GetDocShell() );
    return aDBDocFunc.DoImport( GetViewData().GetTabNo(), rParam, nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
