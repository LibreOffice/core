/*************************************************************************
 *
 *  $RCSfile: undoblk3.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:50:18 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//----------------------------------------------------------------------------

#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SVDSURO_HXX

// INCLUDE -------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/virdev.hxx>

#include "undoblk.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "rangenam.hxx"
#include "arealink.hxx"
#include "patattr.hxx"
#include "target.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "table.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "undoolk.hxx"
#include "undoutil.hxx"
#include "chgtrack.hxx"
#include "dociter.hxx"
#include "cell.hxx"

// STATIC DATA ---------------------------------------------------------------

TYPEINIT1(ScUndoDeleteContents,     SfxUndoAction);
TYPEINIT1(ScUndoFillTable,          SfxUndoAction);
TYPEINIT1(ScUndoSelectionAttr,      SfxUndoAction);
TYPEINIT1(ScUndoAutoFill,           SfxUndoAction);
TYPEINIT1(ScUndoMerge,              SfxUndoAction);
TYPEINIT1(ScUndoAutoFormat,         SfxUndoAction);
TYPEINIT1(ScUndoReplace,            SfxUndoAction);
TYPEINIT1(ScUndoTabOp,              SfxUndoAction);
TYPEINIT1(ScUndoSpelling,           SfxUndoAction);
TYPEINIT1(ScUndoRefreshLink,        SfxUndoAction);
TYPEINIT1(ScUndoInsertAreaLink,     SfxUndoAction);
TYPEINIT1(ScUndoRemoveAreaLink,     SfxUndoAction);
TYPEINIT1(ScUndoUpdateAreaLink,     SfxUndoAction);


// To Do:
/*A*/   // SetOptimalHeight auf Dokument, wenn keine View


//============================================================================
//  class ScUndoDeleteContents
//
//  Inhalte loeschen

//----------------------------------------------------------------------------

ScUndoDeleteContents::ScUndoDeleteContents(
                ScDocShell* pNewDocShell,
                const ScMarkData& rMark, const ScRange& rRange,
                ScDocument* pNewUndoDoc, BOOL bNewMulti,
                USHORT nNewFlags, BOOL bObjects )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aMarkData   ( rMark ),
        aRange      ( rRange ),
        pUndoDoc    ( pNewUndoDoc ),
        bMulti      ( bNewMulti ),  // ueberliquid
        nFlags      ( nNewFlags ),
        pDrawUndo   ( NULL )
{
    if (bObjects)
        pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    if ( !(aMarkData.IsMarked() || aMarkData.IsMultiMarked()) )     // keine Zelle markiert:
        aMarkData.SetMarkArea( aRange );                            // Zelle unter Cursor markieren

    SetChangeTrack();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoDeleteContents::~ScUndoDeleteContents()
{
    delete pUndoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoDeleteContents::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );    // "Loeschen"
}


void ScUndoDeleteContents::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
        pChangeTrack->AppendContentRange( aRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}


//----------------------------------------------------------------------------

void ScUndoDeleteContents::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
    }

    BOOL bPaintExt = FALSE;

    if (bUndo)  // nur Undo
    {
        USHORT nUndoFlags = IDF_NONE;       //  entweder alle oder keine Inhalte kopieren
        if (nFlags & IDF_CONTENTS)          //  (es sind nur die richtigen ins UndoDoc kopiert worden)
            nUndoFlags |= IDF_CONTENTS;
        if (nFlags & IDF_ATTRIB)
            nUndoFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
            nUndoFlags |= IDF_STRING;       // -> Zellen werden geaendert

        ScRange aCopyRange = aRange;
        USHORT nTabCount = pDoc->GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        pUndoDoc->CopyToDocument( aCopyRange, nUndoFlags, bMulti, pDoc, &aMarkData );

        if (pDrawUndo)
            DoSdrUndoAction( pDrawUndo );

        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

        bPaintExt = pDoc->HasAttrib( aRange, HASATTR_PAINTEXT );    // hinterher
    }
    else        // nur Redo
    {
        bPaintExt = pDoc->HasAttrib( aRange, HASATTR_PAINTEXT );    // vorher

        aMarkData.MarkToMulti();
        if (pDrawUndo)
            pDoc->DeleteObjectsInSelection( aMarkData );
        pDoc->DeleteSelection( nFlags, aMarkData );
        aMarkData.MarkToSimple();

        SetChangeTrack();
    }

    USHORT nExtFlags = bPaintExt ? SC_PF_LINES : 0;
    if ( !( (pViewShell) && pViewShell->AdjustRowHeight(
                                aRange.aStart.Row(), aRange.aEnd.Row() ) ) )
/*A*/   pDocShell->PostPaint( aRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( aRange );
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoDeleteContents::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoDeleteContents::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoDeleteContents::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteContents( nFlags, TRUE );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoDeleteContents::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoFillTable
//
//  Tabellen ausfuellen
//  (Bearbeiten|Ausfuellen|...)

//----------------------------------------------------------------------------

ScUndoFillTable::ScUndoFillTable( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                ScDocument* pNewUndoDoc, BOOL bNewMulti, USHORT nSrc,
                USHORT nFlg, USHORT nFunc, BOOL bSkip, BOOL bLink )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aMarkData   ( rMark ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc    ( pNewUndoDoc ),
        bMulti      ( bNewMulti ),
        nSrcTab     ( nSrc ),
        nFlags      ( nFlg ),
        nFunction   ( nFunc ),
        bSkipEmpty  ( bSkip ),
        bAsLink     ( bLink )
{
    SetChangeTrack();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoFillTable::~ScUndoFillTable()
{
    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoFillTable::GetComment() const
{
    return ScGlobal::GetRscString( STR_FILL_TAB );
}


void ScUndoFillTable::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        USHORT nTabCount = pDocShell->GetDocument()->GetTableCount();
        ScRange aWorkRange(aRange);
        nStartChangeAction = 0;
        ULONG nTmpAction;
        for ( USHORT i = 0; i < nTabCount; i++ )
        {
            if (i != nSrcTab && aMarkData.GetTableSelect(i))
            {
                aWorkRange.aStart.SetTab(i);
                aWorkRange.aEnd.SetTab(i);
                pChangeTrack->AppendContentRange( aWorkRange, pUndoDoc,
                    nTmpAction, nEndChangeAction );
                if ( !nStartChangeAction )
                    nStartChangeAction = nTmpAction;
            }
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}


//----------------------------------------------------------------------------

void ScUndoFillTable::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
    }

    if (bUndo)  // nur Undo
    {
        USHORT nTabCount = pDoc->GetTableCount();
        ScRange aWorkRange(aRange);
        for ( USHORT i = 0; i < nTabCount; i++ )
            if (i != nSrcTab && aMarkData.GetTableSelect(i))
            {
                aWorkRange.aStart.SetTab(i);
                aWorkRange.aEnd.SetTab(i);
                if (bMulti)
                    pDoc->DeleteSelectionTab( i, IDF_ALL, aMarkData );
                else
                    pDoc->DeleteAreaTab( aWorkRange, IDF_ALL );
                pUndoDoc->CopyToDocument( aWorkRange, IDF_ALL, bMulti, pDoc, &aMarkData );
            }

        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else        // nur Redo
    {
        aMarkData.MarkToMulti();
        pDoc->FillTabMarked( nSrcTab, aMarkData, nFlags, nFunction, bSkipEmpty, bAsLink );
        aMarkData.MarkToSimple();
        SetChangeTrack();
    }

    pDocShell->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_EXTRAS);
    pDocShell->PostDataChanged();

    //  CellContentChanged kommt mit der Markierung

    if (pViewShell)
    {
        USHORT nTab = pViewShell->GetViewData()->GetTabNo();
        if ( !aMarkData.GetTableSelect(nTab) )
            pViewShell->SetTabNo( nSrcTab );

        pViewShell->DoneBlockMode();    // gibt sonst Probleme, weil Markierung auf falscher Tabelle
    }
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoFillTable::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoFillTable::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoFillTable::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->FillTab( nFlags, nFunction, bSkipEmpty, bAsLink );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoFillTable::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoSelectionAttr
//
//  Zellformat aendern

//----------------------------------------------------------------------------

ScUndoSelectionAttr::ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                ScDocument* pNewUndoDoc, BOOL bNewMulti,
                const ScPatternAttr* pNewApply,
                const SvxBoxItem* pNewOuter, const SvxBoxInfoItem* pNewInner )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aMarkData   ( rMark ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc    ( pNewUndoDoc ),
        bMulti      ( bNewMulti )
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pApplyPattern = (ScPatternAttr*) &pPool->Put( *pNewApply );
    pLineOuter = pNewOuter ? (SvxBoxItem*) &pPool->Put( *pNewOuter ) : NULL;
    pLineInner = pNewInner ? (SvxBoxInfoItem*) &pPool->Put( *pNewInner ) : NULL;
}


//----------------------------------------------------------------------------

__EXPORT ScUndoSelectionAttr::~ScUndoSelectionAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pPool->Remove(*pApplyPattern);
    if (pLineOuter)
        pPool->Remove(*pLineOuter);
    if (pLineInner)
        pPool->Remove(*pLineInner);

    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoSelectionAttr::GetComment() const
{
    //"Attribute" "/Linien"
    return ScGlobal::GetRscString( pLineOuter ? STR_UNDO_SELATTRLINES : STR_UNDO_SELATTR );
}


//----------------------------------------------------------------------------

void ScUndoSelectionAttr::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
    }

    ScRange aEffRange( aRange );
    if ( pDoc->HasAttrib( aEffRange, HASATTR_MERGED ) )         // zusammengefasste Zellen?
        pDoc->ExtendMerge( aEffRange );

    BOOL bHasLines = pDoc->HasAttrib( aEffRange, HASATTR_PAINTEXT );

    if (bUndo)  // nur bei Undo
    {
        ScRange aCopyRange = aRange;
        USHORT nTabCount = pDoc->GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pDoc, &aMarkData );
    }
    else        // nur bei Redo
    {
        aMarkData.MarkToMulti();
        pDoc->ApplySelectionPattern( *pApplyPattern, aMarkData );
        aMarkData.MarkToSimple();

        if (pLineOuter)
            pDoc->ApplySelectionFrame( aMarkData, pLineOuter, pLineInner );
    }

    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aEffRange, PAINT_GRID | PAINT_EXTRAS, (bHasLines ? SC_PF_LINES : 0) );

    ShowTable( aRange );
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSelectionAttr::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSelectionAttr::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSelectionAttr::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        if (pLineOuter)
            rViewShell.ApplyPatternLines( *pApplyPattern, pLineOuter, pLineInner, TRUE );
        else
            rViewShell.ApplySelectionPattern( *pApplyPattern, TRUE );
    }
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoSelectionAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoAutoFill
//
//  Auto-Fill (nur einfache Bloecke)

//----------------------------------------------------------------------------

ScUndoAutoFill::ScUndoAutoFill( ScDocShell* pNewDocShell,
                const ScRange& rRange, const ScRange& rSourceArea,
                ScDocument* pNewUndoDoc, const ScMarkData& rMark,
                FillDir eNewFillDir, FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                double fNewStartValue, double fNewStepValue, double fNewMaxValue,
                USHORT nMaxShIndex )
        //
    :   ScBlockUndo( pNewDocShell, rRange, SC_UNDO_AUTOHEIGHT ),
        //
        aSource         ( rSourceArea ),
        aMarkData       ( rMark ),
        pUndoDoc        ( pNewUndoDoc ),
        eFillDir        ( eNewFillDir ),
        eFillCmd        ( eNewFillCmd ),
        eFillDateCmd    ( eNewFillDateCmd ),
        fStartValue     ( fNewStartValue ),
        fStepValue      ( fNewStepValue ),
        fMaxValue       ( fNewMaxValue ),
        nMaxSharedIndex ( nMaxShIndex)
{
    SetChangeTrack();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoAutoFill::~ScUndoAutoFill()
{
    pDocShell->GetDocument()->EraseNonUsedSharedNames(nMaxSharedIndex);
    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoAutoFill::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFILL ); //"Ausfuellen"
}


void ScUndoAutoFill::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFill::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        if (aMarkData.GetTableSelect(nTab))
        {
            ScRange aWorkRange = aBlockRange;
            aWorkRange.aStart.SetTab(nTab);
            aWorkRange.aEnd.SetTab(nTab);

            BOOL bHasLines = pDoc->HasAttrib( aWorkRange, HASATTR_PAINTEXT );
            pDoc->DeleteAreaTab( aWorkRange, IDF_ALL );
            pUndoDoc->CopyToDocument( aWorkRange, IDF_ALL, FALSE, pDoc );

            pDoc->ExtendMerge( aWorkRange, TRUE );
            pDocShell->PostPaint( aWorkRange, PAINT_GRID, (bHasLines ? SC_PF_LINES : 0) );
        }
    }
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

// Shared-Names loeschen
// Falls Undo ins Dokument gespeichert
// => automatisches Loeschen am Ende
// umarbeiten!!

    String aName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("___SC_"));
    aName += String::CreateFromInt32(nMaxSharedIndex);
    aName += '_';
    ScRangeName* pRangeName = pDoc->GetRangeName();
    BOOL bHasFound = FALSE;
    for (USHORT i = 0; i < pRangeName->GetCount(); i++)
    {
        ScRangeData* pRangeData = (*pRangeName)[i];
        if (pRangeData)
        {
            String aRName;
            pRangeData->GetName(aRName);
            if (aRName.Search(aName) != STRING_NOTFOUND)
            {
                pRangeName->AtFree(i);
                bHasFound = TRUE;
            }
        }
    }
    if (bHasFound)
        pRangeName->SetSharedMaxIndex(pRangeName->GetSharedMaxIndex()-1);

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFill::Redo()
{
    BeginRedo();

//! Tabellen selektieren

    USHORT nCount;
    switch (eFillDir)
    {
        case FILL_TO_BOTTOM:
            nCount = aBlockRange.aEnd.Row() - aSource.aEnd.Row();
            break;
        case FILL_TO_RIGHT:
            nCount = aBlockRange.aEnd.Col() - aSource.aEnd.Col();
            break;
        case FILL_TO_TOP:
            nCount = aSource.aStart.Row() - aBlockRange.aStart.Row();
            break;
        case FILL_TO_LEFT:
            nCount = aSource.aStart.Col() - aBlockRange.aStart.Col();
            break;
    }

    ScDocument* pDoc = pDocShell->GetDocument();
    if ( fStartValue != MAXDOUBLE )
    {
        USHORT nValX = (eFillDir == FILL_TO_LEFT) ? aSource.aEnd.Col() : aSource.aStart.Col();
        USHORT nValY = (eFillDir == FILL_TO_TOP ) ? aSource.aEnd.Row() : aSource.aStart.Row();
        USHORT nTab = aSource.aStart.Tab();
        pDoc->SetValue( nValX, nValY, nTab, fStartValue );
    }
    pDoc->Fill( aSource.aStart.Col(), aSource.aStart.Row(),
                aSource.aEnd.Col(),   aSource.aEnd.Row(),
                aMarkData, nCount,
                eFillDir, eFillCmd, eFillDateCmd,
                fStepValue, fMaxValue );

    SetChangeTrack();

    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFill::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        if (eFillCmd==FILL_SIMPLE)
            rViewShell.FillSimple( eFillDir, TRUE );
        else
            rViewShell.FillSeries( eFillDir, eFillCmd, eFillDateCmd,
                                   fStartValue, fStepValue, fMaxValue, TRUE );
    }
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoAutoFill::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoMerge
//
//  Zellen zusammenfassen / Zusammenfassung aufheben

//----------------------------------------------------------------------------

ScUndoMerge::ScUndoMerge( ScDocShell* pNewDocShell,
                            USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                            USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                            BOOL bNewDoMerge, ScDocument* pNewUndoDoc )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aRange  ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        bDoMerge( bNewDoMerge ),
        pUndoDoc( pNewUndoDoc )
{
}


//----------------------------------------------------------------------------

__EXPORT ScUndoMerge::~ScUndoMerge()
{
    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoMerge::GetComment() const
{
    // "Zusammenfassen" "Zusammenfassung aufheben"
    return bDoMerge ?
        ScGlobal::GetRscString( STR_UNDO_MERGE ) :
        ScGlobal::GetRscString( STR_UNDO_REMERGE );
}


//----------------------------------------------------------------------------

void ScUndoMerge::DoChange( const BOOL bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    if (bDoMerge == bUndo)
        pDoc->RemoveMerge( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab() );
//!     pDoc->RemoveMerge( aRange.aStart );
    else
/*!*/   pDoc->DoMerge( aRange.aStart.Tab(),
                       aRange.aStart.Col(), aRange.aStart.Row(),
                       aRange.aEnd.Col(),   aRange.aEnd.Row()   );

    if (pUndoDoc)
        if (bUndo)
        {
            pDoc->DeleteAreaTab( aRange, IDF_CONTENTS );
            pUndoDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pDoc );
        }
        else
/*!*/       pDoc->DoMergeContents( aRange.aStart.Tab(),
                                   aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(),   aRange.aEnd.Row()   );

    BOOL bDidPaint = FALSE;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( pViewShell )
    {
        pViewShell->SetTabNo( aRange.aStart.Tab() );
        bDidPaint = pViewShell->AdjustRowHeight( aRange.aStart.Row(), aRange.aEnd.Row() );
    }

    if (!bDidPaint)
        ScUndoUtil::PaintMore( pDocShell, aRange );

    ShowTable( aRange );
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoMerge::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoMerge::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoMerge::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bDoMerge)
        {
            BOOL bCont = FALSE;
            rViewShell.MergeCells( FALSE, bCont, TRUE );
        }
        else
            rViewShell.RemoveMerge( TRUE );
    }
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoAutoFormat
//
//      Auto-Format (nur einfache Bloecke)

//----------------------------------------------------------------------------

ScUndoAutoFormat::ScUndoAutoFormat( ScDocShell* pNewDocShell,
                        const ScRange& rRange, ScDocument* pNewUndoDoc,
                        const ScMarkData& rMark, BOOL bNewSize, USHORT nNewFormatNo )
        //
    :   ScBlockUndo( pNewDocShell, rRange, bNewSize ? SC_UNDO_MANUALHEIGHT : SC_UNDO_AUTOHEIGHT ),
        //
        aMarkData   ( rMark ),
        pUndoDoc    ( pNewUndoDoc ),
        bSize       ( bNewSize ),
        nFormatNo   ( nNewFormatNo )
{
}


//----------------------------------------------------------------------------

__EXPORT ScUndoAutoFormat::~ScUndoAutoFormat()
{
    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoAutoFormat::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFORMAT );   //"Auto-Format"
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFormat::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    // Attribute
//  pDoc->DeleteAreaTab( aBlockRange, IDF_ATTRIB );
//  pUndoDoc->CopyToDocument( aBlockRange, IDF_ATTRIB, FALSE, pDoc );

    USHORT nTabCount = pDoc->GetTableCount();
    pDoc->DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                      aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(),
                      aMarkData, IDF_ATTRIB );
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, FALSE, pDoc, &aMarkData );

    // Zellhoehen und -breiten (IDF_NONE)
    if (bSize)
    {
        USHORT nStartX = aBlockRange.aStart.Col();
        USHORT nStartY = aBlockRange.aStart.Row();
        USHORT nStartZ = aBlockRange.aStart.Tab();
        USHORT nEndX = aBlockRange.aEnd.Col();
        USHORT nEndY = aBlockRange.aEnd.Row();
        USHORT nEndZ = aBlockRange.aEnd.Tab();

        pUndoDoc->CopyToDocument( nStartX, 0, 0, nEndX, MAXROW, nTabCount-1,
                                    IDF_NONE, FALSE, pDoc, &aMarkData );
        pUndoDoc->CopyToDocument( 0, nStartY, 0, MAXCOL, nEndY, nTabCount-1,
                                    IDF_NONE, FALSE, pDoc, &aMarkData );
        pDocShell->PostPaint( 0, 0, nStartZ, MAXCOL, MAXROW, nEndZ,
                              PAINT_GRID | PAINT_LEFT | PAINT_TOP, SC_PF_LINES );
    }
    else
        pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES );

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFormat::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nStartX = aBlockRange.aStart.Col();
    USHORT nStartY = aBlockRange.aStart.Row();
    USHORT nStartZ = aBlockRange.aStart.Tab();
    USHORT nEndX = aBlockRange.aEnd.Col();
    USHORT nEndY = aBlockRange.aEnd.Row();
    USHORT nEndZ = aBlockRange.aEnd.Tab();

    pDoc->AutoFormat( nStartX, nStartY, nEndX, nEndY, nFormatNo, aMarkData );

    if (bSize)
    {
        VirtualDevice aVirtDev;
        Fraction aZoomX(1,1);
        Fraction aZoomY = aZoomX;
        double nPPTX,nPPTY;
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
            //  Zoom auf 100 lassen
            nPPTX = ScGlobal::nScreenPPTX;
            nPPTY = ScGlobal::nScreenPPTY;
        }

        BOOL bFormula = FALSE;  //! merken

        for (USHORT nTab=nStartZ; nTab<=nEndZ; nTab++)
        {
            ScMarkData aDestMark;
            aDestMark.SelectOneTable( nTab );
            aDestMark.SetMarkArea( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            aDestMark.MarkToMulti();

            // wie SC_SIZE_VISOPT
            for (USHORT nRow=nStartY; nRow<=nEndY; nRow++)
            {
                BYTE nOld = pDoc->GetRowFlags(nRow,nTab);
                if ( (nOld & CR_HIDDEN) == 0 && ( nOld & CR_MANUALSIZE ) )
                    pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
            }
            pDoc->SetOptimalHeight( nStartY, nEndY, nTab, 0, &aVirtDev,
                                        nPPTX, nPPTY, aZoomX, aZoomY, FALSE );

            for (USHORT nCol=nStartX; nCol<=nEndX; nCol++)
                if ((pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN) == 0)
                {
                    USHORT nThisSize = STD_EXTRA_WIDTH + pDoc->GetOptimalColWidth( nCol, nTab,
                                                &aVirtDev, nPPTX, nPPTY, aZoomX, aZoomY, bFormula,
                                                &aDestMark );
                    pDoc->SetColWidth( nCol, nTab, nThisSize );
                    pDoc->ShowCol( nCol, nTab, TRUE );
                }
        }

        pDocShell->PostPaint( 0,      0,      nStartZ,
                              MAXCOL, MAXROW, nEndZ,
                              PAINT_GRID | PAINT_LEFT | PAINT_TOP, SC_PF_LINES);
    }
    else
        pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES );

    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoAutoFormat::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->AutoFormat( nFormatNo, TRUE );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoAutoFormat::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoReplace
//
//      Ersetzen

//----------------------------------------------------------------------------

ScUndoReplace::ScUndoReplace( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                    const String& rNewUndoStr, ScDocument* pNewUndoDoc,
                                    const SvxSearchItem* pItem )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aMarkData   ( rMark ),
        aCursorPos  ( nCurX, nCurY, nCurZ ),
        aUndoStr    ( rNewUndoStr ),
        pUndoDoc    ( pNewUndoDoc )
{
    pSearchItem = new SvxSearchItem( *pItem );
    SetChangeTrack();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoReplace::~ScUndoReplace()
{
    delete pUndoDoc;
    delete pSearchItem;
}


//----------------------------------------------------------------------------

void ScUndoReplace::SetChangeTrack()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
        {   //! im UndoDoc stehen nur die geaenderten Zellen,
            // deswegen per Iterator moeglich
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
                nStartChangeAction, nEndChangeAction );
        }
        else
        {
            nStartChangeAction = pChangeTrack->GetActionMax() + 1;
            ScAddress aPos( aCursorPos.GetCol(), aCursorPos.GetRow(),
                aCursorPos.GetTab() );
            ScChangeActionContent* pContent = new ScChangeActionContent(
                ScRange( aPos, aPos ) );
            pContent->SetOldValue( aUndoStr, pDoc );
            pContent->SetNewValue( pDoc->GetCell( aPos ), pDoc );
            pChangeTrack->Append( pContent );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

//----------------------------------------------------------------------------

String __EXPORT ScUndoReplace::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REPLACE );  // "Ersetzen"
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoReplace::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ShowTable( aCursorPos.GetTab() );

    if (pUndoDoc)       // nur bei ReplaceAll !!
    {
        DBG_ASSERT(pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL,
                   "ScUndoReplace:: Falscher Modus");

        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
        }

//! markierte Tabellen
//! Bereich merken ?

        //  Undo-Dokument hat keine Zeilen-/Spalten-Infos, also mit bColRowFlags = FALSE
        //  kopieren, um Outline-Gruppen nicht kaputtzumachen.

        USHORT nUndoFlags = (pSearchItem->GetPattern()) ? IDF_ATTRIB : IDF_CONTENTS;
        pUndoDoc->CopyToDocument( 0,      0,      0,
                                  MAXCOL, MAXROW, MAXTAB,
                                  nUndoFlags, FALSE, pDoc, NULL, FALSE );   // ohne Row-Flags
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
    {
        String aTempStr = pSearchItem->GetSearchString();       // vertauschen
        pSearchItem->SetSearchString(pSearchItem->GetReplaceString());
        pSearchItem->SetReplaceString(aTempStr);
        pDoc->ReplaceStyle( *pSearchItem,
                            aCursorPos.GetCol(), aCursorPos.GetRow(), aCursorPos.GetTab(),
                            aMarkData, TRUE);
        pSearchItem->SetReplaceString(pSearchItem->GetSearchString());
        pSearchItem->SetSearchString(aTempStr);
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.GetCol(), aCursorPos.GetRow(),
                                       SC_FOLLOW_JUMP, FALSE, FALSE );
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetCellType() == SVX_SEARCHIN_NOTE)
    {
        ScPostIt aNote;
        if (pDoc->GetNote(aCursorPos.GetCol(), aCursorPos.GetRow(),
                          aCursorPos.GetTab(), aNote))
        {
            aNote.SetText(aUndoStr);
            pDoc->SetNote(aCursorPos.GetCol(), aCursorPos.GetRow(),
                          aCursorPos.GetTab(), aNote);
        }
        else
            DBG_ERROR("ScUndoReplace: Hier ist keine Notizzelle");
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.GetCol(), aCursorPos.GetRow(),
                                       SC_FOLLOW_JUMP, FALSE, FALSE );
    }
    else
    {
        // #78889# aUndoStr may contain line breaks
        if ( aUndoStr.Search('\n') != STRING_NOTFOUND )
            pDoc->PutCell( aCursorPos.GetCol(), aCursorPos.GetRow(), aCursorPos.GetTab(),
                            new ScEditCell( aUndoStr, pDoc ) );
        else
            pDoc->SetString( aCursorPos.GetCol(), aCursorPos.GetRow(), aCursorPos.GetTab(), aUndoStr );
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.GetCol(), aCursorPos.GetRow(),
                                       SC_FOLLOW_JUMP, FALSE, FALSE );
        pDocShell->PostPaintGridAll();
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoReplace::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->MoveCursorAbs( aCursorPos.GetCol(), aCursorPos.GetRow(),
                                   SC_FOLLOW_JUMP, FALSE, FALSE );
    if (pUndoDoc)
    {
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo

            pViewShell->SearchAndReplace( pSearchItem, FALSE, TRUE );
        }
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
    {
        pDoc->ReplaceStyle( *pSearchItem,
                            aCursorPos.GetCol(), aCursorPos.GetRow(), aCursorPos.GetTab(),
                            aMarkData, TRUE);
        pDocShell->PostPaintGridAll();
    }
    else
        if (pViewShell)
            pViewShell->SearchAndReplace( pSearchItem, FALSE, TRUE );

    SetChangeTrack();

    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoReplace::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SearchAndReplace( pSearchItem, TRUE, FALSE );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoReplace::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoTabOp
//
//  Mehrfachoperation (nur einfache Bloecke)

//----------------------------------------------------------------------------

ScUndoTabOp::ScUndoTabOp( ScDocShell* pNewDocShell,
                USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                USHORT nEndX, USHORT nEndY, USHORT nEndZ, ScDocument* pNewUndoDoc,
                const ScRefTripel& rFormulaCell,
                const ScRefTripel& rFormulaEnd,
                const ScRefTripel& rRowCell,
                const ScRefTripel& rColCell,
                BYTE nMd )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aRange          ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc        ( pNewUndoDoc ),
        theFormulaCell  ( rFormulaCell ),
        theFormulaEnd   ( rFormulaEnd ),
        theRowCell      ( rRowCell ),
        theColCell      ( rColCell ),
        nMode           ( nMd )
{
}


//----------------------------------------------------------------------------

__EXPORT ScUndoTabOp::~ScUndoTabOp()
{
    delete pUndoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoTabOp::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TABOP );    // "Mehrfachoperation"
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoTabOp::Undo()
{
    BeginUndo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bHasLines = pDoc->HasAttrib( aRange, HASATTR_PAINTEXT );

    pDoc->DeleteAreaTab( aRange,IDF_ALL );
    pUndoDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pDoc );
    pDocShell->PostPaint( aRange, PAINT_GRID, (bHasLines ? SC_PF_LINES : 0) );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoTabOp::Redo()
{
    BeginRedo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    ScTabOpParam aParam( theFormulaCell, theFormulaEnd,
                         theRowCell,     theColCell,
                         nMode );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->TabOp( aParam, FALSE);

    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoTabOp::Repeat(SfxRepeatTarget& rTarget)
{
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoTabOp::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//============================================================================
//  class ScUndoSpelling
//
//  Spelling

//----------------------------------------------------------------------------

ScUndoSpelling::ScUndoSpelling( ScDocShell* pNewDocShell,
                                const ScMarkData& rMark,
                                USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                ScDocument* pNewUndoDoc,
                                USHORT nNewX, USHORT nNewY, USHORT nNewZ,
                                ScDocument* pNewRedoDoc)
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        aMarkData       ( rMark ),
        aCursorPos      ( nCurX, nCurY, nCurZ ),
        aNewCursorPos   ( nNewX, nNewY, nNewZ ),
        pUndoDoc        ( pNewUndoDoc ),
        pRedoDoc        ( pNewRedoDoc )
{
    SetChangeTrack();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoSpelling::~ScUndoSpelling()
{
    delete pUndoDoc;
    delete pRedoDoc;
}


//----------------------------------------------------------------------------

void ScUndoSpelling::SetChangeTrack()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
                nStartChangeAction, nEndChangeAction );
        else
        {
            DBG_ERROR( "ScUndoSpelling::SetChangeTrack: kein UndoDoc" );
            nStartChangeAction = nEndChangeAction = 0;
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

//----------------------------------------------------------------------------

String __EXPORT ScUndoSpelling::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SPELLING);  // "Rechtschreibung"
}


//----------------------------------------------------------------------------

void ScUndoSpelling::DoChange( ScDocument* pRefDoc, const ScTripel& rCursorPos )
{
    if (pRefDoc)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ShowTable( rCursorPos.GetTab() );

        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        if (pViewShell)
        {
            pViewShell->DoneBlockMode();
            pViewShell->InitOwnBlockMode();
            pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
        }

        USHORT nTabCount = pDoc->GetTableCount();
        //  Undo/Redo-doc has only selected tables

        BOOL bMulti = aMarkData.IsMultiMarked();
        pRefDoc->CopyToDocument( 0,      0,      0,
                                 MAXCOL, MAXROW, nTabCount-1,
                                 IDF_CONTENTS, bMulti, pDoc, &aMarkData );
        pDocShell->PostPaintGridAll();
    }
    else
    {
        DBG_ERROR("Kein Un-/RedoDoc bei Un-/RedoSpelling");
    }
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSpelling::Undo()
{
    BeginUndo();
    DoChange( pUndoDoc, aCursorPos );
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSpelling::Redo()
{
    BeginRedo();
    DoChange( pRedoDoc, aNewCursorPos );
    SetChangeTrack();
    EndRedo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoSpelling::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DoSpellingChecker( TRUE );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoSpelling::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//============================================================================
//  class ScUndoRefreshLink
//
//  Link aktualisieren / aendern

//----------------------------------------------------------------------------

ScUndoRefreshLink::ScUndoRefreshLink( ScDocShell* pNewDocShell,
                                    ScDocument* pNewUndoDoc )
        //
    :   ScSimpleUndo( pNewDocShell ),
        //
        pUndoDoc( pNewUndoDoc ),
        pRedoDoc( NULL )
{
    ScDocument* pDoc = pDocShell->GetDocument();
}


//----------------------------------------------------------------------------

__EXPORT ScUndoRefreshLink::~ScUndoRefreshLink()
{
    delete pUndoDoc;
    delete pRedoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoRefreshLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRefreshLink::Undo()
{
    BeginUndo();

    BOOL bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );

    BOOL bFirst = TRUE;
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
        if (pUndoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
            if (bMakeRedo)
            {
                if (bFirst)
                    pRedoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
                else
                    pRedoDoc->AddUndoTab( nTab, nTab, TRUE, TRUE );
                bFirst = FALSE;
                pDoc->CopyToDocument(aRange, IDF_ALL, FALSE, pRedoDoc);
//              pRedoDoc->TransferDrawPage( pDoc, nTab, nTab );
                pRedoDoc->SetLink( nTab,
                                   pDoc->GetLinkMode(nTab),
                                   pDoc->GetLinkDoc(nTab),
                                   pDoc->GetLinkFlt(nTab),
                                   pDoc->GetLinkOpt(nTab),
                                   pDoc->GetLinkTab(nTab) );
            }

            pDoc->DeleteAreaTab( aRange,IDF_ALL );
//          pDoc->ClearDrawPage(nTab);
            pUndoDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pDoc );
//          pDoc->TransferDrawPage( pUndoDoc, nTab, nTab );
            pDoc->SetLink( nTab, pUndoDoc->GetLinkMode(nTab), pUndoDoc->GetLinkDoc(nTab),
                                 pUndoDoc->GetLinkFlt(nTab),  pUndoDoc->GetLinkOpt(nTab),
                                 pUndoDoc->GetLinkTab(nTab) );
        }

    pDocShell->PostPaintGridAll();

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRefreshLink::Redo()
{
    DBG_ASSERT(pRedoDoc, "Kein RedoDoc bei ScUndoRefreshLink::Redo");

    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
        if (pRedoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);

            pDoc->DeleteAreaTab( aRange, IDF_ALL );
//          pDoc->ClearDrawPage(nTab);
            pRedoDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pDoc );
//          pDoc->TransferDrawPage( pRedoDoc, nTab, nTab );
            pDoc->SetLink( nTab,
                           pRedoDoc->GetLinkMode(nTab),
                           pRedoDoc->GetLinkDoc(nTab),
                           pRedoDoc->GetLinkFlt(nTab),
                           pRedoDoc->GetLinkOpt(nTab),
                           pRedoDoc->GetLinkTab(nTab) );
        }

    pDocShell->PostPaintGridAll();

    EndUndo();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRefreshLink::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoRefreshLink::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//----------------------------------------------------------------------------

ScAreaLink* lcl_FindAreaLink( SvxLinkManager* pLinkManager, const String& rDoc,
                            const String& rFlt, const String& rOpt,
                            const String& rSrc, const ScRange& rDest )
{
    const ::so3::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = pLinkManager->GetLinks().Count();
    for (USHORT i=0; i<nCount; i++)
    {
        ::so3::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            if ( ((ScAreaLink*)pBase)->IsEqual( rDoc, rFlt, rOpt, rSrc, rDest ) )
                return (ScAreaLink*)pBase;
    }

    DBG_ERROR("ScAreaLink nicht gefunden");
    return NULL;
}


//============================================================================
//  class ScUndoInsertAreaLink
//
//  Bereichs-Verknuepfung einfuegen

//----------------------------------------------------------------------------

ScUndoInsertAreaLink::ScUndoInsertAreaLink( ScDocShell* pShell,
                            const String& rDoc,
                            const String& rFlt, const String& rOpt,
                            const String& rArea, const ScRange& rDestRange,
                            ULONG nRefresh )
        //
    :   ScSimpleUndo    ( pShell ),
        //
        aDocName        ( rDoc ),
        aFltName        ( rFlt ),
        aOptions        ( rOpt ),
        aAreaName       ( rArea ),
        aRange          ( rDestRange ),
        nRefreshDelay   ( nRefresh )
{
}


//----------------------------------------------------------------------------

__EXPORT ScUndoInsertAreaLink::~ScUndoInsertAreaLink()
{
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoInsertAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERTAREALINK );
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoInsertAreaLink::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoInsertAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                            aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( TRUE );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( FALSE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoInsertAreaLink::Repeat(SfxRepeatTarget& rTarget)
{
    //! ....
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoInsertAreaLink::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//============================================================================
//  class ScUndoRemoveAreaLink
//
//  Bereichs-Verknuepfung loeschen

//----------------------------------------------------------------------------

ScUndoRemoveAreaLink::ScUndoRemoveAreaLink( ScDocShell* pShell,
                            const String& rDoc, const String& rFlt, const String& rOpt,
                            const String& rArea, const ScRange& rDestRange,
                            ULONG nRefresh )
        //
    :   ScSimpleUndo    ( pShell ),
        //
        aDocName        ( rDoc ),
        aFltName        ( rFlt ),
        aOptions        ( rOpt ),
        aAreaName       ( rArea ),
        aRange          ( rDestRange ),
        nRefreshDelay   ( nRefresh )
{
}


//----------------------------------------------------------------------------

__EXPORT ScUndoRemoveAreaLink::~ScUndoRemoveAreaLink()
{
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoRemoveAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVELINK );   //! eigener Text ??
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRemoveAreaLink::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                        aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( TRUE );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( FALSE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRemoveAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoRemoveAreaLink::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoRemoveAreaLink::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//============================================================================
//  class ScUndoUpdateAreaLink
//
//  Bereichs-Verknuepfung aktualisieren

//----------------------------------------------------------------------------

ScUndoUpdateAreaLink::ScUndoUpdateAreaLink( ScDocShell* pShell,
                            const String& rOldD, const String& rOldF, const String& rOldO,
                            const String& rOldA, const ScRange& rOldR, ULONG nOldRD,
                            const String& rNewD, const String& rNewF, const String& rNewO,
                            const String& rNewA, const ScRange& rNewR, ULONG nNewRD,
                            ScDocument* pUndo, ScDocument* pRedo, BOOL bDoInsert )
        //
    :   ScSimpleUndo( pShell ),
        //
        aOldDoc     ( rOldD ),
        aOldFlt     ( rOldF ),
        aOldOpt     ( rOldO ),
        aOldArea    ( rOldA ),
        aOldRange   ( rOldR ),
        aNewDoc     ( rNewD ),
        aNewFlt     ( rNewF ),
        aNewOpt     ( rNewO ),
        aNewArea    ( rNewA ),
        aNewRange   ( rNewR ),
        pUndoDoc    ( pUndo ),
        pRedoDoc    ( pRedo ),
        nOldRefresh ( nOldRD ),
        nNewRefresh ( nNewRD ),
        bWithInsert ( bDoInsert )
{
    DBG_ASSERT( aOldRange.aStart == aNewRange.aStart, "AreaLink verschoben ??!??" );
}


//----------------------------------------------------------------------------

__EXPORT ScUndoUpdateAreaLink::~ScUndoUpdateAreaLink()
{
    delete pUndoDoc;
    delete pRedoDoc;
}


//----------------------------------------------------------------------------

String __EXPORT ScUndoUpdateAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );   //! eigener Text ??
}


//----------------------------------------------------------------------------

void ScUndoUpdateAreaLink::DoChange( const BOOL bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nEndX = Max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
    USHORT nEndY = Max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );
    USHORT nEndZ = Max( aOldRange.aEnd.Tab(), aNewRange.aEnd.Tab() );   //?

    if ( bUndo )
    {
        if ( bWithInsert )
        {
            pDoc->FitBlock( aNewRange, aOldRange );
            pUndoDoc->UndoToDocument( aOldRange, IDF_ALL, FALSE, pDoc );
        }
        else
            pUndoDoc->CopyToDocument(
                        ScRange(aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ)),
                        IDF_ALL, FALSE, pDoc );
    }
    else
    {
        if ( bWithInsert )
        {
            pDoc->FitBlock( aOldRange, aNewRange );
            pRedoDoc->CopyToDocument( aNewRange, IDF_ALL, FALSE, pDoc );
        }
        else
            pRedoDoc->CopyToDocument(
                        ScRange(aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ)),
                        IDF_ALL, FALSE, pDoc );
    }

    ScRange aWorkRange( aNewRange.aStart, ScTripel( nEndX, nEndY, nEndZ ) );
    pDoc->ExtendMerge( aWorkRange, TRUE );

    //  Paint

    if ( aNewRange.aEnd.Col() != aOldRange.aEnd.Col() )
        aWorkRange.aEnd.SetCol(MAXCOL);
    if ( aNewRange.aEnd.Row() != aOldRange.aEnd.Row() )
        aWorkRange.aEnd.SetRow(MAXROW);
    pDocShell->PostPaint( aWorkRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoUpdateAreaLink::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aNewDoc, aNewFlt, aNewOpt,
                                            aNewArea, aNewRange );
    if (pLink)
    {
        pLink->SetSource( aOldDoc, aOldFlt, aOldOpt, aOldArea );        // alte Werte im Link
        pLink->SetDestArea( aOldRange );
        pLink->SetRefreshDelay( nOldRefresh );
    }

    DoChange(TRUE);
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoUpdateAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aOldDoc, aOldFlt, aOldOpt,
                                            aOldArea, aOldRange );
    if (pLink)
    {
        pLink->SetSource( aNewDoc, aNewFlt, aNewOpt, aNewArea );        // neue Werte im Link
        pLink->SetDestArea( aNewRange );
        pLink->SetRefreshDelay( nNewRefresh );
    }

    DoChange(FALSE);
}


//----------------------------------------------------------------------------

void __EXPORT ScUndoUpdateAreaLink::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScUndoUpdateAreaLink::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}




