/*************************************************************************
 *
 *  $RCSfile: undoblk2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-16 13:15:06 $
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
//#include <segmentc.hxx>
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

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
        }
    }

    //! outlines from all tables?
    if (pUndoTab)                                           // Outlines mit gespeichert?
        pDoc->SetOutlineTable( nStartTab, pUndoTab );

    USHORT nPaintStart = nStart ? nStart-1 : 0;

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

    if (eMode==SC_SIZE_OPTIMAL)
    {
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 16:45:07  hr
    initial import

    Revision 1.18  2000/09/17 14:09:28  willem.vandorp
    OpenOffice header added.

    Revision 1.17  2000/08/31 16:38:39  willem.vandorp
    Header and footer replaced

    Revision 1.16  1999/10/27 14:46:07  hr
    #65293#: removed  SEG_EOFGLOBALS() SEG_FUNCDEF

    Revision 1.15  1996/09/04 17:46:18  RJ
    ueberarbeitet


      Rev 1.14   04 Sep 1996 19:46:18   RJ
   ueberarbeitet

      Rev 1.13   19 Aug 1996 21:29:44   NN
   Markierungen werden nicht mehr am Dokument gehalten

      Rev 1.12   08 Dec 1995 14:31:46   NN
   ScSimpleUndo

      Rev 1.11   10 Oct 1995 09:52:12   NN
   Paint beim Ausblenden

      Rev 1.10   09 Oct 1995 15:49:16   NN
   ScTabViewTarget fuer Repeat

      Rev 1.9   04 Oct 1995 19:51:52   NN
   Repeat mit ViewShell als RepeatTarget

      Rev 1.8   12 Jul 1995 15:07:26   NN
   __EXPORT bei Destruktoren

      Rev 1.7   11 Jul 1995 12:39:06   HJS
   add: __EXPORT

      Rev 1.6   17 Jun 1995 14:00:04   TRI
   EXPORTS

      Rev 1.5   15 Jun 1995 08:52:46   TRI
   EXPORT

      Rev 1.4   06 Jun 1995 12:35:28   NN
   Abfrage, ob ViewShell vorhanden

      Rev 1.3   24 Mar 1995 18:39:00   SC
   out of keys hack

      Rev 1.2   22 Mar 1995 16:39:10   STE
   undoblk3 abgesplittet

      Rev 1.1   21 Mar 1995 14:40:08   TRI
   Out of Keys - Defines eingebaut

      Rev 1.0   08 Mar 1995 02:51:52   SC
   aufgeteilt

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

