/*************************************************************************
 *
 *  $RCSfile: undobase.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 10:53:34 $
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

// INCLUDE ---------------------------------------------------------------

#include <vcl/virdev.hxx>

#include "undobase.hxx"
#include "refundo.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "undoolk.hxx"
#include "undodraw.hxx"
#include "dbcolect.hxx"
#include "attrib.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScSimpleUndo,     SfxUndoAction);
TYPEINIT1(ScBlockUndo,      SfxUndoAction);
TYPEINIT1(ScMoveUndo,       SfxUndoAction);
TYPEINIT1(ScDBFuncUndo,     SfxUndoAction);
TYPEINIT1(ScUndoWrapper,    SfxUndoAction);

// -----------------------------------------------------------------------

ScSimpleUndo::ScSimpleUndo( ScDocShell* pDocSh ) :
    pDocShell( pDocSh ),
    pDetectiveUndo( NULL )
{
}

__EXPORT ScSimpleUndo::~ScSimpleUndo()
{
    delete pDetectiveUndo;
}

BOOL __EXPORT ScSimpleUndo::Merge( SfxUndoAction *pNextAction )
{
    //  Zu jeder Undo-Action kann eine SdrUndoGroup fuer das Aktualisieren
    //  der Detektiv-Pfeile gehoeren.
    //  DetectiveRefresh kommt immer hinterher, die SdrUndoGroup ist in
    //  eine ScUndoDraw Action verpackt.
    //  Nur beim automatischen Aktualisieren wird AddUndoAction mit
    //  bTryMerg=TRUE gerufen.

    if ( !pDetectiveUndo && pNextAction->ISA(ScUndoDraw) )
    {
        //  SdrUndoAction aus der ScUndoDraw Action uebernehmen,
        //  ScUndoDraw wird dann vom UndoManager geloescht

        ScUndoDraw* pCalcUndo = (ScUndoDraw*)pNextAction;
        pDetectiveUndo = pCalcUndo->GetDrawUndo();
        pCalcUndo->ForgetDrawUndo();
        return TRUE;
    }

    return FALSE;
}

void ScSimpleUndo::BeginUndo()
{
    pDocShell->SetInUndo( TRUE );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // z.B. wegen zusammengefassten Zellen

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

    pDocShell->SetInUndo( FALSE );
}

void ScSimpleUndo::BeginRedo()
{
    pDocShell->SetInUndo( TRUE );   //! eigenes Flag fuer Redo?

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->HideAllCursors();       // z.B. wegen zusammengefassten Zellen
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

    pDocShell->SetInUndo( FALSE );
}

void ScSimpleUndo::ShowTable( SCTAB nTab )          // static
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nTab );
}

void ScSimpleUndo::ShowTable( const ScRange& rRange )           // static
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nStart = rRange.aStart.Tab();
        SCTAB nEnd   = rRange.aEnd.Tab();
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        if ( nTab < nStart || nTab > nEnd )                     // wenn nicht im Bereich:
            pViewShell->SetTabNo( nStart );                     // auf erste des Bereiches
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

__EXPORT ScBlockUndo::~ScBlockUndo()
{
    DeleteSdrUndoAction( pDrawUndo );
}

void ScBlockUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
    EnableDrawAdjust( pDocShell->GetDocument(), FALSE );
}

void ScBlockUndo::EndUndo()
{
    if (eMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    EnableDrawAdjust( pDocShell->GetDocument(), TRUE );
    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );

    ShowBlock();
    ScSimpleUndo::EndUndo();
}

/*
void ScBlockUndo::BeginRedo()
{
    ScSimpleUndo::BeginRedo();
}
*/

void ScBlockUndo::EndRedo()
{
    if (eMode == SC_UNDO_AUTOHEIGHT)
        AdjustHeight();

    ShowBlock();
    ScSimpleUndo::EndRedo();
}

BOOL ScBlockUndo::AdjustHeight()
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
        //  Zoom auf 100 lassen
        nPPTX = ScGlobal::nScreenPPTX;
        nPPTY = ScGlobal::nScreenPPTY;
    }

    BOOL bRet = pDoc->SetOptimalHeight( aBlockRange.aStart.Row(), aBlockRange.aEnd.Row(),
/*!*/                                   aBlockRange.aStart.Tab(), 0, &aVirtDev,
                                        nPPTX, nPPTY, aZoomX, aZoomY, FALSE );

    if (bRet)
        pDocShell->PostPaint( 0,      aBlockRange.aStart.Row(), aBlockRange.aStart.Tab(),
                              MAXCOL, MAXROW,                   aBlockRange.aEnd.Tab(),
                              PAINT_GRID | PAINT_LEFT );

    return bRet;
}

void ScBlockUndo::ShowBlock()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ShowTable( aBlockRange );       // bei mehreren Tabs im Range ist jede davon gut
        pViewShell->MoveCursorAbs( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                                   SC_FOLLOW_JUMP, FALSE, FALSE );
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        ScRange aRange = aBlockRange;
        aRange.aStart.SetTab( nTab );
        aRange.aEnd.SetTab( nTab );
        pViewShell->MarkRange( aRange );

        //  nicht per SetMarkArea an MarkData, wegen evtl. fehlendem Paint
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

__EXPORT ScMoveUndo::~ScMoveUndo()
{
    delete pRefUndoData;
    delete pRefUndoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}

void ScMoveUndo::UndoRef()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScRange aRange(0,0,0, MAXCOL,MAXROW,pRefUndoDoc->GetTableCount()-1);
    pRefUndoDoc->CopyToDocument( aRange, IDF_FORMULA, FALSE, pDoc, NULL, FALSE );
    if (pRefUndoData)
        pRefUndoData->DoUndo( pDoc, (eMode == SC_UNDO_REFFIRST) );
        // #65055# HACK: ScDragDropUndo ist der einzige mit REFFIRST.
        // Falls nicht, resultiert daraus evtl. ein zu haeufiges Anpassen
        // der ChartRefs, nicht schoen, aber auch nicht schlecht..
}

void ScMoveUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();

    EnableDrawAdjust( pDocShell->GetDocument(), FALSE );

    if (pRefUndoDoc && eMode == SC_UNDO_REFFIRST)
        UndoRef();
}

void ScMoveUndo::EndUndo()
{
    if (pDrawUndo)                   //@17.12.97 Reihenfolge der Fkt.s geaendert
        DoSdrUndoAction( pDrawUndo );

    if (pRefUndoDoc && eMode == SC_UNDO_REFLAST)
        UndoRef();

    EnableDrawAdjust( pDocShell->GetDocument(), TRUE );

    ScSimpleUndo::EndUndo();
}

/*
void ScMoveUndo::BeginRedo()
{
    ScSimpleUndo::BeginRedo();
}
*/

/*
void ScMoveUndo::EndRedo()
{
    ScSimpleUndo::EndRedo();
}
*/

// -----------------------------------------------------------------------

ScDBFuncUndo::ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal ) :
    ScSimpleUndo( pDocSh ),
    aOriginalRange( rOriginal )
{
    pAutoDBRange = pDocSh->GetOldAutoDBRange();
}

ScDBFuncUndo::~ScDBFuncUndo()
{
    delete pAutoDBRange;
}

void ScDBFuncUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();
}

void ScDBFuncUndo::EndUndo()
{
    ScSimpleUndo::EndUndo();

    if ( pAutoDBRange )
    {
        USHORT nNoNameIndex;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if ( pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            ScDBData* pNoNameData = (*pColl)[nNoNameIndex];

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
    if ( pAutoDBRange )
    {
        // move the database range to this function's position again (see ScDocShell::GetDBData)

        USHORT nNoNameIndex;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if ( pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            ScDBData* pNoNameData = (*pColl)[nNoNameIndex];

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

            pNoNameData->SetByRow( TRUE );
            pNoNameData->SetAutoFilter( FALSE );
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

String ScUndoWrapper::GetComment() const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetComment();
    else
        return String();
}

String ScUndoWrapper::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetRepeatComment(rTarget);
    else
        return String();
}

USHORT ScUndoWrapper::GetId() const
{
    if (pWrappedUndo)
        return pWrappedUndo->GetId();
    else
        return 0;
}

BOOL ScUndoWrapper::IsLinked()
{
    if (pWrappedUndo)
        return pWrappedUndo->IsLinked();
    else
        return FALSE;
}

void ScUndoWrapper::SetLinked( BOOL bIsLinked )
{
    if (pWrappedUndo)
        pWrappedUndo->SetLinked(bIsLinked);
}

BOOL ScUndoWrapper::Merge( SfxUndoAction* pNextAction )
{
    if (pWrappedUndo)
        return pWrappedUndo->Merge(pNextAction);
    else
        return FALSE;
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

BOOL ScUndoWrapper::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pWrappedUndo)
        return pWrappedUndo->CanRepeat(rTarget);
    else
        return FALSE;
}


