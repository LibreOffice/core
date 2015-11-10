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

#include "undoblk.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "olinetab.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "target.hxx"

#include "undoolk.hxx"

TYPEINIT1(ScUndoWidthOrHeight,      SfxUndoAction);

/** Change column widths or row heights */
ScUndoWidthOrHeight::ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOLROW nNewStart, SCTAB nNewStartTab, SCCOLROW nNewEnd, SCTAB nNewEndTab,
                ScDocument* pNewUndoDoc, const std::vector<sc::ColRowSpan>& rRanges,
                ScOutlineTable* pNewUndoTab,
                ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips, bool bNewWidth ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nStartTab( nNewStartTab ),
    nEndTab( nNewEndTab ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTab( pNewUndoTab ),
    maRanges(rRanges),
    nNewSize( nNewSizeTwips ),
    bWidth( bNewWidth ),
    eMode( eNewMode ),
    pDrawUndo( nullptr )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScUndoWidthOrHeight::~ScUndoWidthOrHeight()
{
    delete pUndoDoc;
    delete pUndoTab;
    DeleteSdrUndoAction( pDrawUndo );
}

OUString ScUndoWidthOrHeight::GetComment() const
{
    // [ "optimal " ] "Column width" | "row height"
    return ( bWidth ?
        ( ( eMode == SC_SIZE_OPTIMAL )?
        ScGlobal::GetRscString( STR_UNDO_OPTCOLWIDTH ) :
        ScGlobal::GetRscString( STR_UNDO_COLWIDTH )
        ) :
        ( ( eMode == SC_SIZE_OPTIMAL )?
        ScGlobal::GetRscString( STR_UNDO_OPTROWHEIGHT ) :
        ScGlobal::GetRscString( STR_UNDO_ROWHEIGHT )
        ) );
}

void ScUndoWidthOrHeight::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCCOLROW nPaintStart = nStart > 0 ? nStart-1 : static_cast<SCCOLROW>(0);

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if ( SetViewMarkData( aMarkData ) )
            nPaintStart = 0;        // paint all, because of changed selection
    }

    //! outlines from all tables?
    if (pUndoTab)                                           // Outlines are included when saving ?
        rDoc.SetOutlineTable( nStartTab, pUndoTab );

    SCTAB nTabCount = rDoc.GetTableCount();
    ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        if (bWidth) // Width
        {
            pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, *itr,
                    static_cast<SCCOL>(nEnd), MAXROW, *itr, InsertDeleteFlags::NONE,
                    false, &rDoc );
            rDoc.UpdatePageBreaks( *itr );
            pDocShell->PostPaint( static_cast<SCCOL>(nPaintStart), 0, *itr,
                    MAXCOL, MAXROW, *itr, PAINT_GRID | PAINT_TOP );
        }
        else        // Height
        {
            pUndoDoc->CopyToDocument( 0, nStart, *itr, MAXCOL, nEnd, *itr, InsertDeleteFlags::NONE, false, &rDoc );
            rDoc.UpdatePageBreaks( *itr );
            pDocShell->PostPaint( 0, nPaintStart, *itr, MAXCOL, MAXROW, *itr, PAINT_GRID | PAINT_LEFT );
        }
    }

    DoSdrUndoAction( pDrawUndo, &rDoc );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateScrollBars();

        SCTAB nCurrentTab = pViewShell->GetViewData().GetTabNo();
        if ( nCurrentTab < nStartTab || nCurrentTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );
    }

    EndUndo();
}

void ScUndoWidthOrHeight::Redo()
{
    BeginRedo();

    bool bPaintAll = false;
    if (eMode==SC_SIZE_OPTIMAL)
    {
        if ( SetViewMarkData( aMarkData ) )
            bPaintAll = true;       // paint all, because of changed selection
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nTab = pViewShell->GetViewData().GetTabNo();
        if ( nTab < nStartTab || nTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );

        // SetWidthOrHeight changes current sheet!
        pViewShell->SetWidthOrHeight(
            bWidth, maRanges, eMode, nNewSize, false, true, &aMarkData);
    }

    // paint grid if selection was changed directly at the MarkData
    if (bPaintAll)
        pDocShell->PostPaint( 0, 0, nStartTab, MAXCOL, MAXROW, nEndTab, PAINT_GRID );

    EndRedo();
}

void ScUndoWidthOrHeight::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->SetMarkedWidthOrHeight( bWidth, eMode, nNewSize );
}

bool ScUndoWidthOrHeight::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
