/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <vcl/virdev.hxx>

#include "undobase.hxx"
#include "refundo.hxx"
#include "docsh.hxx"
#include "dbdocfun.hxx"
#include "tabvwsh.hxx"
#include "undoolk.hxx"
#include "undodraw.hxx"
#include "dbcolect.hxx"
#include "attrib.hxx"
#include "queryparam.hxx"
#include "globstr.hrc"

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

sal_Bool __EXPORT ScSimpleUndo::Merge( SfxUndoAction *pNextAction )
{
    //  Zu jeder Undo-Action kann eine SdrUndoGroup fuer das Aktualisieren
    //  der Detektiv-Pfeile gehoeren.
    //  DetectiveRefresh kommt immer hinterher, die SdrUndoGroup ist in
    //  eine ScUndoDraw Action verpackt.
    //  Nur beim automatischen Aktualisieren wird AddUndoAction mit
    //  bTryMerg=TRUE gerufen.
    ScUndoDraw* pCalcUndo = dynamic_cast< ScUndoDraw* >(pNextAction);

    if ( !pDetectiveUndo && pCalcUndo )
    {
        //  SdrUndoAction aus der ScUndoDraw Action uebernehmen,
        //  ScUndoDraw wird dann vom UndoManager geloescht

        pDetectiveUndo = pCalcUndo->GetDrawUndo();
        pCalcUndo->ForgetDrawUndo();
        return sal_True;
    }

    return sal_False;
}

void ScSimpleUndo::BeginUndo()
{
    pDocShell->SetInUndo( sal_True );

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

    pDocShell->SetInUndo( sal_False );
}

void ScSimpleUndo::BeginRedo()
{
    pDocShell->SetInUndo( sal_True );   //! eigenes Flag fuer Redo?

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

    pDocShell->SetInUndo( sal_False );
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
    EnableDrawAdjust( pDocShell->GetDocument(), sal_False );
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
        //  Zoom auf 100 lassen
        nPPTX = ScGlobal::nScreenPPTX;
        nPPTY = ScGlobal::nScreenPPTY;
    }

    sal_Bool bRet = pDoc->SetOptimalHeight( aBlockRange.aStart.Row(), aBlockRange.aEnd.Row(),
/*!*/                                   aBlockRange.aStart.Tab(), 0, &aVirtDev,
                                        nPPTX, nPPTY, aZoomX, aZoomY, sal_False );

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
        ShowTable( aBlockRange );       // bei mehreren Tabs im Range ist jede davon gut
        pViewShell->MoveCursorAbs( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                                   SC_FOLLOW_JUMP, sal_False, sal_False );
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
    pRefUndoDoc->CopyToDocument( aRange, IDF_FORMULA, sal_False, pDoc, NULL, sal_False );
    if (pRefUndoData)
        pRefUndoData->DoUndo( pDoc, (eMode == SC_UNDO_REFFIRST) );
        // #65055# HACK: ScDragDropUndo ist der einzige mit REFFIRST.
        // Falls nicht, resultiert daraus evtl. ein zu haeufiges Anpassen
        // der ChartRefs, nicht schoen, aber auch nicht schlecht..
}

void ScMoveUndo::BeginUndo()
{
    ScSimpleUndo::BeginUndo();

    EnableDrawAdjust( pDocShell->GetDocument(), sal_False );

    if (pRefUndoDoc && eMode == SC_UNDO_REFFIRST)
        UndoRef();
}

void ScMoveUndo::EndUndo()
{
    //@17.12.97 Reihenfolge der Fkt.s geaendert
    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );     // #125875# must also be called when pointer is null

    if (pRefUndoDoc && eMode == SC_UNDO_REFLAST)
        UndoRef();

    EnableDrawAdjust( pDocShell->GetDocument(), sal_True );

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
        sal_uInt16 nNoNameIndex;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if ( pColl->SearchName( pAutoDBRange->GetName(), nNoNameIndex ) )
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
            /*if (pAutoDBRange->HasQueryParam())   //maybe conflict with AOO
            {
                ScQueryParam    aParam;
                pAutoDBRange->GetQueryParam(aParam);
                ScDBDocFunc aDBDocFunc( *pDocShell );
                aDBDocFunc.Query( nRangeTab, aParam, NULL, sal_False, sal_False );
            }*/

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

        sal_uInt16 nNoNameIndex;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if ( pColl->SearchName( pAutoDBRange->GetName(), nNoNameIndex ) )
        {
            ScDBData* pNoNameData = (*pColl)[nNoNameIndex];

            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            /*if (pAutoDBRange->HasQueryParam())
            {
                ScQueryParam    aParam;
                pAutoDBRange->GetQueryParam(aParam);
                SCSIZE nEC = aParam.GetEntryCount();
                for (SCSIZE i=0; i<nEC; i++)
                    aParam.GetEntry(i).bDoQuery = sal_False;
                aParam.bDuplicate = sal_True;
                ScDBDocFunc aDBDocFunc( *pDocShell );
                aDBDocFunc.Query( nRangeTab, aParam, NULL, sal_False, sal_False );
            }*/
            pDocShell->DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

            pNoNameData->SetSortParam( ScSortParam() );
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( aOriginalRange.aStart.Tab(),
                                  aOriginalRange.aStart.Col(), aOriginalRange.aStart.Row(),
                                  aOriginalRange.aEnd.Col(), aOriginalRange.aEnd.Row() );

            pNoNameData->SetByRow( sal_True );
            pNoNameData->SetAutoFilter( sal_False );
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
        return sal_False;
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
        return sal_False;
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
        return sal_False;
}


