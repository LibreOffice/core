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

#include <scitems.hxx>

#include <undoblk.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <olinetab.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <target.hxx>
#include <columnspanset.hxx>

#include <undoolk.hxx>

#include <sfx2/lokhelper.hxx>
#include <svx/svdundo.hxx>

/** Change column widths or row heights */
ScUndoWidthOrHeight::ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOLROW nNewStart, SCTAB nNewStartTab, SCCOLROW nNewEnd, SCTAB nNewEndTab,
                ScDocumentUniquePtr pNewUndoDoc, const std::vector<sc::ColRowSpan>& rRanges,
                std::unique_ptr<ScOutlineTable> pNewUndoTab,
                ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips, bool bNewWidth ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nStartTab( nNewStartTab ),
    nEndTab( nNewEndTab ),
    pUndoDoc( std::move(pNewUndoDoc) ),
    pUndoTab( std::move(pNewUndoTab) ),
    maRanges(rRanges),
    nNewSize( nNewSizeTwips ),
    bWidth( bNewWidth ),
    eMode( eNewMode )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScUndoWidthOrHeight::~ScUndoWidthOrHeight()
{
    pUndoDoc.reset();
    pUndoTab.reset();
    pDrawUndo.reset();
}

OUString ScUndoWidthOrHeight::GetComment() const
{
    // [ "optimal " ] "Column width" | "row height"
    return ( bWidth ?
        ( ( eMode == SC_SIZE_OPTIMAL )?
        ScResId( STR_UNDO_OPTCOLWIDTH ) :
        ScResId( STR_UNDO_COLWIDTH )
        ) :
        ( ( eMode == SC_SIZE_OPTIMAL )?
        ScResId( STR_UNDO_OPTROWHEIGHT ) :
        ScResId( STR_UNDO_ROWHEIGHT )
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
        rDoc.SetOutlineTable( nStartTab, pUndoTab.get() );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : aMarkData)
    {
        if (rTab >= nTabCount)
            break;

        if (pViewShell)
            pViewShell->OnLOKSetWidthOrHeight(nStart, bWidth);

        if (bWidth) // Width
        {
            pUndoDoc->CopyToDocument(static_cast<SCCOL>(nStart), 0, rTab,
                                     static_cast<SCCOL>(nEnd), MAXROW, rTab, InsertDeleteFlags::NONE,
                                     false, rDoc);
            rDoc.UpdatePageBreaks( rTab );
            pDocShell->PostPaint( static_cast<SCCOL>(nPaintStart), 0, rTab,
                    MAXCOL, MAXROW, rTab, PaintPartFlags::Grid | PaintPartFlags::Top );
        }
        else        // Height
        {
            pUndoDoc->CopyToDocument(0, nStart, rTab, MAXCOL, nEnd, rTab, InsertDeleteFlags::NONE, false, rDoc);
            rDoc.UpdatePageBreaks( rTab );
            pDocShell->PostPaint( 0, nPaintStart, rTab, MAXCOL, MAXROW, rTab, PaintPartFlags::Grid | PaintPartFlags::Left );
        }
    }

    DoSdrUndoAction( pDrawUndo.get(), &rDoc );

    if (pViewShell)
    {
        pViewShell->UpdateScrollBars(bWidth ? COLUMN_HEADER : ROW_HEADER);

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
            bWidth, maRanges, eMode, nNewSize, false, &aMarkData);
    }

    // paint grid if selection was changed directly at the MarkData
    if (bPaintAll)
        pDocShell->PostPaint( 0, 0, nStartTab, MAXCOL, MAXROW, nEndTab, PaintPartFlags::Grid );

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
