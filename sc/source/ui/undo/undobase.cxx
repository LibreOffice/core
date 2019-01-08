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

#include <vcl/virdev.hxx>
#include <svx/svdundo.hxx>

#include <undobase.hxx>
#include <refundo.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <undoolk.hxx>
#include <undodraw.hxx>
#include <dbdata.hxx>
#include <attrib.hxx>
#include <queryparam.hxx>
#include <subtotalparam.hxx>
#include <bcaslot.hxx>
#include <rowheightcontext.hxx>
#include <column.hxx>
#include <sortparam.hxx>
#include <columnspanset.hxx>


ScSimpleUndo::ScSimpleUndo( ScDocShell* pDocSh ) :
    pDocShell( pDocSh ),
    mnViewShellId(-1)
{
    if (ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell())
        mnViewShellId = pViewShell->GetViewShellId();
}

ViewShellId ScSimpleUndo::GetViewShellId() const
{
    return mnViewShellId;
}

bool ScSimpleUndo::SetViewMarkData( const ScMarkData& rMarkData )
{
    if ( IsPaintLocked() )
        return false;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !pViewShell )
        return false;

    pViewShell->SetMarkData( rMarkData );
    return true;
}

bool ScSimpleUndo::Merge( SfxUndoAction *pNextAction )
{
    // A SdrUndoGroup for updating detective arrows can belong
    // to each Undo-Action.
    // DetectiveRefresh is always called next,
    // the SdrUndoGroup is encapsulated in a ScUndoDraw action.
    // AddUndoAction is only called with bTryMerg=sal_True
    // for automatic update.

    if ( !pDetectiveUndo && dynamic_cast<const ScUndoDraw*>( pNextAction) !=  nullptr )
    {
        // Take SdrUndoAction from ScUndoDraw Action,
        // ScUndoDraw is later deleted by the UndoManager

        ScUndoDraw* pCalcUndo = static_cast<ScUndoDraw*>(pNextAction);
        pDetectiveUndo = pCalcUndo->ReleaseDrawUndo();
        return true;
    }

    return false;
}

void ScSimpleUndo::BeginUndo()
{
    pDocShell->SetInUndo( true );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // for example due to merged cells

    //  detective updates happened last, must be undone first
    if (pDetectiveUndo)
        pDetectiveUndo->Undo();
}

namespace
{
    class DisableUndoGuard
    {
    private:
        ScDocument& m_rDoc;
        bool const m_bUndoEnabled;
    public:
        explicit DisableUndoGuard(ScDocShell *pDocShell)
            : m_rDoc(pDocShell->GetDocument())
            , m_bUndoEnabled(m_rDoc.IsUndoEnabled())
        {
            m_rDoc.EnableUndo(false);
        }

        ~DisableUndoGuard()
        {
            m_rDoc.EnableUndo(m_bUndoEnabled);
        }
    };
}

void ScSimpleUndo::EndUndo()
{
    {
        // rhbz#1352881 Temporarily turn off undo generation during
        // SetDocumentModified
        DisableUndoGuard aGuard(pDocShell);
        pDocShell->SetDocumentModified();
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateAutoFillMark();
        pViewShell->UpdateInputHandler();
        pViewShell->ShowAllCursors();
    }

    pDocShell->SetInUndo( false );
}

void ScSimpleUndo::BeginRedo()
{
    pDocShell->SetInUndo( true );   //! own Flag for Redo?

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // for example due to merged cells
}

void ScSimpleUndo::EndRedo()
{
    if (pDetectiveUndo)
        pDetectiveUndo->Redo();

    {
        // rhbz#1352881 Temporarily turn off undo generation during
        // SetDocumentModified
        DisableUndoGuard aGuard(pDocShell);
        pDocShell->SetDocumentModified();
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateAutoFillMark();
        pViewShell->UpdateInputHandler();
        pViewShell->ShowAllCursors();
    }

    pDocShell->SetInUndo( false );
}

void ScSimpleUndo::BroadcastChanges( const ScRange& rRange )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.BroadcastCells(rRange, SfxHintId::ScDataChanged);
}

namespace {

class SpanBroadcaster : public sc::ColumnSpanSet::ColumnAction
{
    ScDocument& mrDoc;
    SCTAB mnCurTab;
    SCCOL mnCurCol;

public:
    explicit SpanBroadcaster( ScDocument& rDoc ) : mrDoc(rDoc), mnCurTab(-1), mnCurCol(-1) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        mnCurTab = pCol->GetTab();
        mnCurCol = pCol->GetCol();
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
    {
        if (!bVal)
            return;

        ScRange aRange(mnCurCol, nRow1, mnCurTab, mnCurCol, nRow2, mnCurTab);
        mrDoc.BroadcastCells(aRange, SfxHintId::ScDataChanged);
    };
};

}

void ScSimpleUndo::BroadcastChanges( const DataSpansType& rSpans )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    SpanBroadcaster aBroadcaster(rDoc);

    for (const auto& rEntry : rSpans)
    {
        const sc::ColumnSpanSet& rSet = *rEntry.second;
        rSet.executeColumnAction(rDoc, aBroadcaster);
    }
}

void ScSimpleUndo::ShowTable( SCTAB nTab )
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nTab );
}

void ScSimpleUndo::ShowTable( const ScRange& rRange )
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nStart = rRange.aStart.Tab();
        SCTAB nEnd   = rRange.aEnd.Tab();
        SCTAB nTab = pViewShell->GetViewData().GetTabNo();
        if ( nTab < nStart || nTab > nEnd )                     // if not in range:
            pViewShell->SetTabNo( nStart );                     // at beginning of the range
    }
}

ScBlockUndo::ScBlockUndo( ScDocShell* pDocSh, const ScRange& rRange,
                                            ScBlockUndoMode eBlockMode ) :
    ScSimpleUndo( pDocSh ),
    aBlockRange( rRange ),
    eMode( eBlockMode )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScBlockUndo::~ScBlockUndo()
{
    pDrawUndo.reset();
}

void ScBlockUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    EnableDrawAdjust( &pDocShell->GetDocument(), false );
}

void ScBlockUndo::EndUndo()
{
    if (eMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    EnableDrawAdjust( &pDocShell->GetDocument(), true );
    DoSdrUndoAction( pDrawUndo.get(), &pDocShell->GetDocument() );

    ShowBlock();
    ScSimpleUndo::EndUndo();
}

void ScBlockUndo::EndRedo()
{
    if (eMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    ShowBlock();
    ScSimpleUndo::EndRedo();
}

bool ScBlockUndo::AdjustHeight()
{
    ScDocument& rDoc = pDocShell->GetDocument();

    ScopedVclPtrInstance< VirtualDevice > pVirtDev;
    Fraction aZoomX( 1, 1 );
    Fraction aZoomY = aZoomX;
    double nPPTX, nPPTY;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ScViewData& rData = pViewShell->GetViewData();
        nPPTX = rData.GetPPTX();
        nPPTY = rData.GetPPTY();
        aZoomX = rData.GetZoomX();
        aZoomY = rData.GetZoomY();
    }
    else
    {
        // Leave zoom at 100
        nPPTX = ScGlobal::nScreenPPTX;
        nPPTY = ScGlobal::nScreenPPTY;
    }

    sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoomX, aZoomY, pVirtDev);
    bool bRet = rDoc.SetOptimalHeight(
        aCxt, aBlockRange.aStart.Row(), aBlockRange.aEnd.Row(), aBlockRange.aStart.Tab());

    if (bRet)
    {
        // tdf#76183: recalculate objects' positions
        rDoc.SetDrawPageSize(aBlockRange.aStart.Tab());

        pDocShell->PostPaint( 0,      aBlockRange.aStart.Row(), aBlockRange.aStart.Tab(),
                              MAXCOL, MAXROW,                   aBlockRange.aEnd.Tab(),
                              PaintPartFlags::Grid | PaintPartFlags::Left );
    }
    return bRet;
}

void ScBlockUndo::ShowBlock()
{
    if ( IsPaintLocked() )
        return;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ShowTable( aBlockRange );       // with multiple sheets in range each of them is good
        pViewShell->MoveCursorAbs( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                                   SC_FOLLOW_JUMP, false, false );
        SCTAB nTab = pViewShell->GetViewData().GetTabNo();
        ScRange aRange = aBlockRange;
        aRange.aStart.SetTab( nTab );
        aRange.aEnd.SetTab( nTab );
        pViewShell->MarkRange( aRange );

        // not through SetMarkArea to MarkData, due to possibly lacking paint
    }
}

ScMultiBlockUndo::ScMultiBlockUndo(
    ScDocShell* pDocSh, const ScRangeList& rRanges) :
    ScSimpleUndo(pDocSh),
    maBlockRanges(rRanges)
{
    mpDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScMultiBlockUndo::~ScMultiBlockUndo()
{
    mpDrawUndo.reset();
}

void ScMultiBlockUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    EnableDrawAdjust(&pDocShell->GetDocument(), false);
}

void ScMultiBlockUndo::EndUndo()
{
    EnableDrawAdjust(&pDocShell->GetDocument(), true);
    DoSdrUndoAction(mpDrawUndo.get(), &pDocShell->GetDocument());

    ShowBlock();
    ScSimpleUndo::EndUndo();
}

void ScMultiBlockUndo::EndRedo()
{
    ShowBlock();
    ScSimpleUndo::EndRedo();
}

void ScMultiBlockUndo::ShowBlock()
{
    if ( IsPaintLocked() )
        return;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (!pViewShell)
        return;

    if (maBlockRanges.empty())
        return;

    // Move to the sheet of the first range.
    ScRange aRange = maBlockRanges.front();
    ShowTable(aRange);
    pViewShell->MoveCursorAbs(
        aRange.aStart.Col(), aRange.aStart.Row(), SC_FOLLOW_JUMP, false, false);
    SCTAB nTab = pViewShell->GetViewData().GetTabNo();
    aRange.aStart.SetTab(nTab);
    aRange.aEnd.SetTab(nTab);
    pViewShell->MarkRange(aRange, false);

    for (size_t i = 1, n = maBlockRanges.size(); i < n; ++i)
    {
        aRange = maBlockRanges[i];
        aRange.aStart.SetTab(nTab);
        aRange.aEnd.SetTab(nTab);
        pViewShell->MarkRange(aRange, false, true);
    }
}

ScMoveUndo::ScMoveUndo( ScDocShell* pDocSh, ScDocumentUniquePtr pRefDoc, std::unique_ptr<ScRefUndoData> pRefData ) :
    ScSimpleUndo( pDocSh ),
    pRefUndoDoc( std::move(pRefDoc) ),
    pRefUndoData( std::move(pRefData) )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    if (pRefUndoData)
        pRefUndoData->DeleteUnchanged(&rDoc);
    pDrawUndo = GetSdrUndoAction( &rDoc );
}

ScMoveUndo::~ScMoveUndo()
{
    pRefUndoData.reset();
    pRefUndoDoc.reset();
    pDrawUndo.reset();
}

void ScMoveUndo::UndoRef()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScRange aRange(0,0,0, MAXCOL,MAXROW,pRefUndoDoc->GetTableCount()-1);
    pRefUndoDoc->CopyToDocument(aRange, InsertDeleteFlags::FORMULA, false, rDoc, nullptr, false);
    if (pRefUndoData)
        pRefUndoData->DoUndo( &rDoc, false );
}

void ScMoveUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();

    EnableDrawAdjust( &pDocShell->GetDocument(), false );
}

void ScMoveUndo::EndUndo()
{
    DoSdrUndoAction( pDrawUndo.get(), &pDocShell->GetDocument() );     // must also be called when pointer is null

    if (pRefUndoDoc)
        UndoRef();

    EnableDrawAdjust( &pDocShell->GetDocument(), true );

    ScSimpleUndo::EndUndo();
}

ScDBFuncUndo::ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal ) :
    ScSimpleUndo( pDocSh ),
    aOriginalRange( rOriginal )
{
    pAutoDBRange = pDocSh->GetOldAutoDBRange();
}

ScDBFuncUndo::~ScDBFuncUndo()
{
    pAutoDBRange.reset();
}

void ScDBFuncUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    DoSdrUndoAction( nullptr, &pDocShell->GetDocument() );
}

void ScDBFuncUndo::EndUndo()
{
    ScSimpleUndo::EndUndo();

    if ( pAutoDBRange )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nTab = rDoc.GetVisibleTab();
        ScDBData* pNoNameData = rDoc.GetAnonymousDBData(nTab);
        if (pNoNameData )
        {
            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            pDocShell->DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2 );

            *pNoNameData = *pAutoDBRange;

            if ( pAutoDBRange->HasAutoFilter() )
            {
                // restore AutoFilter buttons
                pAutoDBRange->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
                rDoc.ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, ScMF::Auto );
                pDocShell->PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PaintPartFlags::Grid );
            }
        }
    }
}

void ScDBFuncUndo::BeginRedo()
{
    RedoSdrUndoAction( nullptr );
    if ( pAutoDBRange )
    {
        // move the database range to this function's position again (see ScDocShell::GetDBData)

        ScDocument& rDoc = pDocShell->GetDocument();
        ScDBData* pNoNameData = rDoc.GetAnonymousDBData(aOriginalRange.aStart.Tab());
        if ( pNoNameData )
        {

            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            pDocShell->DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2 );

            pNoNameData->SetSortParam( ScSortParam() );
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( aOriginalRange.aStart.Tab(),
                                  aOriginalRange.aStart.Col(), aOriginalRange.aStart.Row(),
                                  aOriginalRange.aEnd.Col(), aOriginalRange.aEnd.Row() );

            pNoNameData->SetByRow( true );
            pNoNameData->SetAutoFilter( false );
            // header is always set with the operation in redo
        }
    }

    ScSimpleUndo::BeginRedo();
}

void ScDBFuncUndo::EndRedo()
{
    ScSimpleUndo::EndRedo();
}

ScUndoWrapper::ScUndoWrapper( std::unique_ptr<SfxUndoAction> pUndo ) :
    pWrappedUndo( std::move(pUndo) ),
    mnViewShellId( -1 )
{
    if (pWrappedUndo)
        mnViewShellId = pWrappedUndo->GetViewShellId();
}

ScUndoWrapper::~ScUndoWrapper()
{
}

void ScUndoWrapper::ForgetWrappedUndo()
{
    pWrappedUndo = nullptr;    // don't delete in dtor - pointer must be stored outside
}

OUString ScUndoWrapper::GetComment() const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetComment();
    return OUString();
}

ViewShellId ScUndoWrapper::GetViewShellId() const
{
    return mnViewShellId;
}

OUString ScUndoWrapper::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetRepeatComment(rTarget);
    return OUString();
}

bool ScUndoWrapper::Merge( SfxUndoAction* pNextAction )
{
    if (pWrappedUndo)
        return pWrappedUndo->Merge(pNextAction);
    else
        return false;
}

void ScUndoWrapper::Undo()
{
    if (pWrappedUndo)
        pWrappedUndo->Undo();
}

void ScUndoWrapper::Redo()
{
    if (pWrappedUndo)
        pWrappedUndo->Redo();
}

void ScUndoWrapper::Repeat(SfxRepeatTarget& rTarget)
{
    if (pWrappedUndo)
        pWrappedUndo->Repeat(rTarget);
}

bool ScUndoWrapper::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->CanRepeat(rTarget);
    else
        return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
