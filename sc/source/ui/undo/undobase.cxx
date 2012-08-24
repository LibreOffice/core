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

#include <vcl/virdev.hxx>

#include "undobase.hxx"
#include "refundo.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "undoolk.hxx"
#include "undodraw.hxx"
#include "dbdata.hxx"
#include "attrib.hxx"
#include "queryparam.hxx"
#include "subtotalparam.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScSimpleUndo,     SfxUndoAction);
TYPEINIT1(ScBlockUndo,      ScSimpleUndo);
TYPEINIT1(ScMultiBlockUndo, ScSimpleUndo);
TYPEINIT1(ScMoveUndo,       ScSimpleUndo);
TYPEINIT1(ScDBFuncUndo,     ScSimpleUndo);
TYPEINIT1(ScUndoWrapper,    SfxUndoAction);

// -----------------------------------------------------------------------

ScSimpleUndo::ScSimpleUndo( ScDocShell* pDocSh ) :
    pDocShell( pDocSh ),
    pDetectiveUndo( NULL )
{
}

ScSimpleUndo::~ScSimpleUndo()
{
    delete pDetectiveUndo;
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

sal_Bool ScSimpleUndo::Merge( SfxUndoAction *pNextAction )
{
    // A SdrUndoGroup for updating detective arrows can belong
    // to each Undo-Action.
    // DetectiveRefresh is always called next,
    // the SdrUndoGroup is encapsulated in a ScUndoDraw action.
    // AddUndoAction is only called with bTryMerg=sal_True
    // for automatic update.

    if ( !pDetectiveUndo && pNextAction->ISA(ScUndoDraw) )
    {
        // Take SdrUndoAction from ScUndoDraw Action,
        // ScUndoDraw is later deleted by the UndoManager

        ScUndoDraw* pCalcUndo = (ScUndoDraw*)pNextAction;
        pDetectiveUndo = pCalcUndo->GetDrawUndo();
        pCalcUndo->ForgetDrawUndo();
        return sal_True;
    }

    return false;
}

void ScSimpleUndo::BeginUndo()
{
    pDocShell->SetInUndo( sal_True );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // for example due to merged cells

    //  detective updates happened last, must be undone first
    if (pDetectiveUndo)
        pDetectiveUndo->Undo();
}

void ScSimpleUndo::EndUndo()
{
    pDocShell->SetDocumentModified();

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
    pDocShell->SetInUndo( sal_True );   //! own Flag for Redo?

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // for example due to merged cells
}

void ScSimpleUndo::EndRedo()
{
    if (pDetectiveUndo)
        pDetectiveUndo->Redo();

    pDocShell->SetDocumentModified();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateAutoFillMark();
        pViewShell->UpdateInputHandler();
        pViewShell->ShowAllCursors();
    }

    pDocShell->SetInUndo( false );
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
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        if ( nTab < nStart || nTab > nEnd )                     // if not in range:
            pViewShell->SetTabNo( nStart );                     // at beginning of the range
    }
}


// -----------------------------------------------------------------------

ScBlockUndo::ScBlockUndo( ScDocShell* pDocSh, const ScRange& rRange,
                                            ScBlockUndoMode eBlockMode ) :
    ScSimpleUndo( pDocSh ),
    aBlockRange( rRange ),
    eMode( eBlockMode )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScBlockUndo::~ScBlockUndo()
{
    DeleteSdrUndoAction( pDrawUndo );
}

void ScBlockUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    EnableDrawAdjust( pDocShell->GetDocument(), false );
}

void ScBlockUndo::EndUndo()
{
    if (eMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    EnableDrawAdjust( pDocShell->GetDocument(), sal_True );
    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );

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

sal_Bool ScBlockUndo::AdjustHeight()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    VirtualDevice aVirtDev;
    Fraction aZoomX( 1, 1 );
    Fraction aZoomY = aZoomX;
    double nPPTX, nPPTY;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ScViewData* pData = pViewShell->GetViewData();
        nPPTX = pData->GetPPTX();
        nPPTY = pData->GetPPTY();
        aZoomX = pData->GetZoomX();
        aZoomY = pData->GetZoomY();
    }
    else
    {
        // Leave zoom at 100
        nPPTX = ScGlobal::nScreenPPTX;
        nPPTY = ScGlobal::nScreenPPTY;
    }

    sal_Bool bRet = pDoc->SetOptimalHeight( aBlockRange.aStart.Row(), aBlockRange.aEnd.Row(),
/*!*/                                   aBlockRange.aStart.Tab(), 0, &aVirtDev,
                                        nPPTX, nPPTY, aZoomX, aZoomY, false );

    if (bRet)
        pDocShell->PostPaint( 0,      aBlockRange.aStart.Row(), aBlockRange.aStart.Tab(),
                              MAXCOL, MAXROW,                   aBlockRange.aEnd.Tab(),
                              PAINT_GRID | PAINT_LEFT );

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
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        ScRange aRange = aBlockRange;
        aRange.aStart.SetTab( nTab );
        aRange.aEnd.SetTab( nTab );
        pViewShell->MarkRange( aRange );

        // not through SetMarkArea to MarkData, due to possibly lacking paint
    }
}

ScMultiBlockUndo::ScMultiBlockUndo(
    ScDocShell* pDocSh, const ScRangeList& rRanges, ScBlockUndoMode eBlockMode) :
    ScSimpleUndo(pDocSh),
    maBlockRanges(rRanges),
    meMode(eBlockMode)
{
    mpDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScMultiBlockUndo::~ScMultiBlockUndo()
{
    DeleteSdrUndoAction( mpDrawUndo );
}

void ScMultiBlockUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    EnableDrawAdjust(pDocShell->GetDocument(), false);
}

void ScMultiBlockUndo::EndUndo()
{
    if (meMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    EnableDrawAdjust(pDocShell->GetDocument(), true);
    DoSdrUndoAction(mpDrawUndo, pDocShell->GetDocument());

    ShowBlock();
    ScSimpleUndo::EndUndo();
}

void ScMultiBlockUndo::EndRedo()
{
    if (meMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    ShowBlock();
    ScSimpleUndo::EndRedo();
}

void ScMultiBlockUndo::AdjustHeight()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    VirtualDevice aVirtDev;
    Fraction aZoomX( 1, 1 );
    Fraction aZoomY = aZoomX;
    double nPPTX, nPPTY;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ScViewData* pData = pViewShell->GetViewData();
        nPPTX = pData->GetPPTX();
        nPPTY = pData->GetPPTY();
        aZoomX = pData->GetZoomX();
        aZoomY = pData->GetZoomY();
    }
    else
    {
        // Leave zoom at 100
        nPPTX = ScGlobal::nScreenPPTX;
        nPPTY = ScGlobal::nScreenPPTY;
    }

    for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
    {
        const ScRange& r = *maBlockRanges[i];
        bool bRet = pDoc->SetOptimalHeight(
            r.aStart.Row(), r.aEnd.Row(), r.aStart.Tab(), 0, &aVirtDev,
            nPPTX, nPPTY, aZoomX, aZoomY, false);

        if (bRet)
            pDocShell->PostPaint(
                0, r.aStart.Row(), r.aStart.Tab(), MAXCOL, MAXROW, r.aEnd.Tab(),
                PAINT_GRID | PAINT_LEFT);
    }
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
    ScRange aRange = *maBlockRanges.front();
    ShowTable(aRange);
    pViewShell->MoveCursorAbs(
        aRange.aStart.Col(), aRange.aStart.Row(), SC_FOLLOW_JUMP, false, false);
    SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
    aRange.aStart.SetTab(nTab);
    aRange.aEnd.SetTab(nTab);
    pViewShell->MarkRange(aRange, false, false);

    for (size_t i = 1, n = maBlockRanges.size(); i < n; ++i)
    {
        aRange = *maBlockRanges[i];
        aRange.aStart.SetTab(nTab);
        aRange.aEnd.SetTab(nTab);
        pViewShell->MarkRange(aRange, false, true);
    }
}

// -----------------------------------------------------------------------

ScMoveUndo::ScMoveUndo( ScDocShell* pDocSh, ScDocument* pRefDoc, ScRefUndoData* pRefData,
                                                ScMoveUndoMode eRefMode ) :
    ScSimpleUndo( pDocSh ),
    pRefUndoDoc( pRefDoc ),
    pRefUndoData( pRefData ),
    eMode( eRefMode )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    if (pRefUndoData)
        pRefUndoData->DeleteUnchanged(pDoc);
    pDrawUndo = GetSdrUndoAction( pDoc );
}

ScMoveUndo::~ScMoveUndo()
{
    delete pRefUndoData;
    delete pRefUndoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}

void ScMoveUndo::UndoRef()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScRange aRange(0,0,0, MAXCOL,MAXROW,pRefUndoDoc->GetTableCount()-1);
    pRefUndoDoc->CopyToDocument( aRange, IDF_FORMULA, false, pDoc, NULL, false );
    if (pRefUndoData)
        pRefUndoData->DoUndo( pDoc, (eMode == SC_UNDO_REFFIRST) );
        // HACK: ScDragDropUndo is the only one with REFFIRST.
        // If not, results possibly in a too frequent adjustment
        // of ChartRefs. Not that pretty, but not too bad either..
}

void ScMoveUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();

    EnableDrawAdjust( pDocShell->GetDocument(), false );

    if (pRefUndoDoc && eMode == SC_UNDO_REFFIRST)
        UndoRef();
}

void ScMoveUndo::EndUndo()
{
    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );     // must also be called when pointer is null

    if (pRefUndoDoc && eMode == SC_UNDO_REFLAST)
        UndoRef();

    EnableDrawAdjust( pDocShell->GetDocument(), sal_True );

    ScSimpleUndo::EndUndo();
}

// -----------------------------------------------------------------------

ScDBFuncUndo::ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal, SdrUndoAction* pDrawUndo ) :
    ScSimpleUndo( pDocSh ),
    aOriginalRange( rOriginal ),
    mpDrawUndo( pDrawUndo )
{
    pAutoDBRange = pDocSh->GetOldAutoDBRange();
}

ScDBFuncUndo::~ScDBFuncUndo()
{
    DeleteSdrUndoAction( mpDrawUndo );
    delete pAutoDBRange;
}

void ScDBFuncUndo::SetDrawUndoAction( SdrUndoAction* pDrawUndo )
{
    DeleteSdrUndoAction( mpDrawUndo );
    mpDrawUndo = pDrawUndo;
}

void ScDBFuncUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    DoSdrUndoAction( mpDrawUndo, pDocShell->GetDocument() );
}

void ScDBFuncUndo::EndUndo()
{
    ScSimpleUndo::EndUndo();

    if ( pAutoDBRange )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTab = pDoc->GetVisibleTab();
        ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
        if (pNoNameData )
        {
            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            pDocShell->DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

            *pNoNameData = *pAutoDBRange;

            if ( pAutoDBRange->HasAutoFilter() )
            {
                // restore AutoFilter buttons
                pAutoDBRange->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
                pDoc->ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, SC_MF_AUTO );
                pDocShell->PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PAINT_GRID );
            }
        }
    }
}

void ScDBFuncUndo::BeginRedo()
{
    RedoSdrUndoAction( mpDrawUndo );
    if ( pAutoDBRange )
    {
        // move the database range to this function's position again (see ScDocShell::GetDBData)

        ScDocument* pDoc = pDocShell->GetDocument();
        ScDBData* pNoNameData = pDoc->GetAnonymousDBData(aOriginalRange.aStart.Tab());
        if ( pNoNameData )
        {

            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            pDocShell->DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

            pNoNameData->SetSortParam( ScSortParam() );
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( aOriginalRange.aStart.Tab(),
                                  aOriginalRange.aStart.Col(), aOriginalRange.aStart.Row(),
                                  aOriginalRange.aEnd.Col(), aOriginalRange.aEnd.Row() );

            pNoNameData->SetByRow( sal_True );
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

// -----------------------------------------------------------------------

ScUndoWrapper::ScUndoWrapper( SfxUndoAction* pUndo ) :
    pWrappedUndo( pUndo )
{
}

ScUndoWrapper::~ScUndoWrapper()
{
    delete pWrappedUndo;
}

void ScUndoWrapper::ForgetWrappedUndo()
{
    pWrappedUndo = NULL;    // don't delete in dtor - pointer must be stored outside
}

rtl::OUString ScUndoWrapper::GetComment() const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetComment();
    return rtl::OUString();
}

String ScUndoWrapper::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetRepeatComment(rTarget);
    else
        return String();
}

sal_uInt16 ScUndoWrapper::GetId() const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetId();
    else
        return 0;
}

sal_Bool ScUndoWrapper::IsLinked()
{
    if (pWrappedUndo)
        return pWrappedUndo->IsLinked();
    else
        return false;
}

void ScUndoWrapper::SetLinked( sal_Bool bIsLinked )
{
    if (pWrappedUndo)
        pWrappedUndo->SetLinked(bIsLinked);
}

sal_Bool ScUndoWrapper::Merge( SfxUndoAction* pNextAction )
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

sal_Bool ScUndoWrapper::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->CanRepeat(rTarget);
    else
        return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
