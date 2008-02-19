/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoblk2.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 15:33:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
                ScSizeMode eNewMode, USHORT nNewSizeTwips, BOOL bNewWidth ) :
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
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCCOLROW nPaintStart = nStart > 0 ? nStart-1 : static_cast<SCCOLROW>(0);

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->SetMarkData( aMarkData );

            nPaintStart = 0;        // paint all, because of changed selection
        }
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
                        FALSE, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( static_cast<SCCOL>(nPaintStart), 0, nTab,
                        MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_TOP );
            }
            else        // Height
            {
                pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( 0, nPaintStart, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_LEFT );
            }
        }

    DoSdrUndoAction( pDrawUndo, pDoc );

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

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    BOOL bPaintAll = FALSE;
    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->SetMarkData( aMarkData );

            bPaintAll = TRUE;       // paint all, because of changed selection
        }
    }

    if (pViewShell)
    {
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        if ( nTab < nStartTab || nTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );
    }

    // SetWidthOrHeight aendert aktuelle Tabelle !
    pViewShell->SetWidthOrHeight( bWidth, nRangeCnt, pRanges, eMode, nNewSize, FALSE, TRUE, &aMarkData );

    // paint grid if selection was changed directly at the MarkData
    if (bPaintAll)
        pDocShell->PostPaint( 0, 0, nStartTab, MAXCOL, MAXROW, nEndTab, PAINT_GRID );

    EndRedo();
}

void __EXPORT ScUndoWidthOrHeight::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SetMarkedWidthOrHeight( bWidth, eMode, nNewSize, TRUE );
}

BOOL __EXPORT ScUndoWidthOrHeight::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


