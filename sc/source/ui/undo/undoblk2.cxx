/*************************************************************************
 *
 *  $RCSfile: undoblk2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-20 15:38:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SVDSURO_HXX

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
                USHORT nNewStart, USHORT nNewStartTab, USHORT nNewEnd, USHORT nNewEndTab,
                ScDocument* pNewUndoDoc, USHORT nNewCnt, USHORT* pNewRanges,
                ScOutlineTable* pNewUndoTab,
                ScSizeMode eNewMode, USHORT nNewSizeTwips, BOOL bNewWidth ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nStartTab( nNewStartTab ),
    nEndTab( nNewEndTab ),
    pUndoDoc( pNewUndoDoc ),
    nRangeCnt( nNewCnt ),
    pRanges( pNewRanges ),
    pUndoTab( pNewUndoTab ),
    eMode( eNewMode ),
    nNewSize( nNewSizeTwips ),
    bWidth( bNewWidth ),
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

    USHORT nPaintStart = nStart ? nStart-1 : 0;

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo

            nPaintStart = 0;        // paint all, because of changed selection
        }
    }

    //! outlines from all tables?
    if (pUndoTab)                                           // Outlines mit gespeichert?
        pDoc->SetOutlineTable( nStartTab, pUndoTab );

    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nTab;
    for (nTab=0; nTab<nTabCount; nTab++)
        if (aMarkData.GetTableSelect(nTab))
        {
            if (bWidth) // Width
            {
                pUndoDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( nPaintStart, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_TOP );
            }
            else        // Height
            {
                pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pDoc );
                pDoc->UpdatePageBreaks( nTab );
                pDocShell->PostPaint( 0, nPaintStart, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_LEFT );
            }
        }

    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );

    if (pViewShell)
    {
        pViewShell->UpdateScrollBars();

        USHORT nCurrentTab = pViewShell->GetViewData()->GetTabNo();
        if ( nCurrentTab < nStartTab || nCurrentTab > nEndTab )
            pViewShell->SetTabNo( nStartTab );
    }

    EndUndo();
}

void __EXPORT ScUndoWidthOrHeight::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    BOOL bPaintAll = FALSE;
    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo

            bPaintAll = TRUE;       // paint all, because of changed selection
        }
    }

    if (pViewShell)
    {
        USHORT nTab = pViewShell->GetViewData()->GetTabNo();
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


