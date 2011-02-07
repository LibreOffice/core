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

// System - Includes -----------------------------------------------------



#ifndef PCH
#include "scitems.hxx"              // SearchItem
#endif

// INCLUDE ---------------------------------------------------------------

#include "undoblk.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "olinetab.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "target.hxx"

#include "undoolk.hxx"              //! GetUndo ins Document verschieben!


// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoWidthOrHeight,      SfxUndoAction);

// -----------------------------------------------------------------------



//
//      Spaltenbreiten oder Zeilenhoehen aendern
//

ScUndoWidthOrHeight::ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOLROW nNewStart, SCTAB nNewStartTab, SCCOLROW nNewEnd, SCTAB nNewEndTab,
                ScDocument* pNewUndoDoc, SCCOLROW nNewCnt, SCCOLROW* pNewRanges,
                ScOutlineTable* pNewUndoTab,
                ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips, sal_Bool bNewWidth ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nStartTab( nNewStartTab ),
    nEndTab( nNewEndTab ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTab( pNewUndoTab ),
    nRangeCnt( nNewCnt ),
    pRanges( pNewRanges ),
    nNewSize( nNewSizeTwips ),
    bWidth( bNewWidth ),
    eMode( eNewMode ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

__EXPORT ScUndoWidthOrHeight::~ScUndoWidthOrHeight()
{
    delete[] pRanges;
    delete pUndoDoc;
    delete pUndoTab;
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoWidthOrHeight::GetComment() const
{
    // [ "optimale " ] "Spaltenbreite" | "Zeilenhoehe"
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

void __EXPORT ScUndoWidthOrHeight::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    SCCOLROW nPaintStart = nStart > 0 ? nStart-1 : static_cast<SCCOLROW>(0);

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if ( SetViewMarkData( aMarkData ) )
            nPaintStart = 0;        // paint all, because of changed selection
    }

    //! outlines from all tables?
    if (pUndoTab)                                           // Outlines mit gespeichert?
        pDoc->SetOutlineTable( nStartTab, pUndoTab );

    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTab;
    for (nTab=0; nTab<nTabCount; nTab++)
        if (aMarkData.GetTableSelect(nTab))
        {
            if (bWidth) // Width
            {
                pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                        static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE,
                        sal_False, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( static_cast<SCCOL>(nPaintStart), 0, nTab,
                        MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_TOP );
            }
            else        // Height
            {
                pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, sal_False, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( 0, nPaintStart, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_LEFT );
            }
        }

    DoSdrUndoAction( pDrawUndo, pDoc );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateScrollBars();

        SCTAB nCurrentTab = pViewShell->GetViewData()->GetTabNo();
        if ( nCurrentTab < nStartTab || nCurrentTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );
    }

    EndUndo();
}

void __EXPORT ScUndoWidthOrHeight::Redo()
{
    BeginRedo();

    sal_Bool bPaintAll = sal_False;
    if (eMode==SC_SIZE_OPTIMAL)
    {
        if ( SetViewMarkData( aMarkData ) )
            bPaintAll = sal_True;       // paint all, because of changed selection
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        if ( nTab < nStartTab || nTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );
    }

    // SetWidthOrHeight aendert aktuelle Tabelle !
    if ( pViewShell )
        pViewShell->SetWidthOrHeight( bWidth, nRangeCnt, pRanges, eMode, nNewSize, FALSE, TRUE, &aMarkData );

    // paint grid if selection was changed directly at the MarkData
    if (bPaintAll)
        pDocShell->PostPaint( 0, 0, nStartTab, MAXCOL, MAXROW, nEndTab, PAINT_GRID );

    EndRedo();
}

void __EXPORT ScUndoWidthOrHeight::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SetMarkedWidthOrHeight( bWidth, eMode, nNewSize, sal_True );
}

sal_Bool __EXPORT ScUndoWidthOrHeight::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


