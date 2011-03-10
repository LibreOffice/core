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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <com/sun/star/embed/EmbedStates.hpp>

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/writingmodeitem.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

#include "drawview.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "drawutil.hxx"
#include "futext.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "client.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "docsh.hxx"
#include "viewuno.hxx"
#include "userdat.hxx"
#include "postit.hxx"
#include "undocell.hxx"

#include "sc.hrc"

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define SC_HANDLESIZE_BIG       9
#define SC_HANDLESIZE_SMALL     7

// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif


void ScDrawView::Construct()
{
    EnableExtendedKeyInputDispatcher(false);
    EnableExtendedMouseEventDispatcher(false);
    EnableExtendedCommandEventDispatcher(false);

    SetFrameDragSingles(true);

    SetMinMoveDistancePixel( 2 );
    SetHitTolerancePixel( 2 );

    if (pViewData)
    {
        SCTAB nViewTab = pViewData->GetTabNo();
        ShowSdrPage(GetModel()->GetPage(nViewTab));

        sal_Bool bEx = pViewData->GetViewShell()->IsDrawSelMode();
        sal_Bool bProt = pDoc->IsTabProtected( nViewTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), sal_True );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_FRONT);
        if (pLayer)
        {
            SetLayerLocked( pLayer->GetName(), bProt );
            SetActiveLayer( pLayer->GetName() );        // set active layer to FRONT
        }
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_HIDDEN);
        if (pLayer)
        {
            SetLayerLocked( pLayer->GetName(), bProt );
            SetLayerVisible( pLayer->GetName(), false);
        }

        SetSwapAsynchron(sal_True);
    }
    else
    {
        ShowSdrPage(GetModel()->GetPage(nTab));
    }

    UpdateUserViewOptions();
    RecalcScale();
    UpdateWorkArea();

    bInConstruct = false;
}

void ScDrawView::ImplClearCalcDropMarker()
{
    if(pDropMarker)
    {
        delete pDropMarker;
        pDropMarker = 0L;
    }
}

ScDrawView::~ScDrawView()
{
    ImplClearCalcDropMarker();
}

void ScDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    sal_uInt32 nCount = rMrkList.GetMarkCount();
    for(sal_uInt32 nPos=0; nPos<nCount; nPos++ )
    {
        SdrObject* pObj = rMrkList.GetMark(nPos)->GetMarkedSdrObj();
        if (ScDrawObjData *pAnchor = ScDrawLayer::GetObjDataTab(pObj, nTab))
        {
            if (ScTabView* pView = pViewData->GetView())
                pView->CreateAnchorHandles(aHdl, pAnchor->maStart);
        }
    }
}

void ScDrawView::InvalidateAttribs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

        // true status values:
    rBindings.InvalidateAll( true );
}

void ScDrawView::InvalidateDrawTextAttrs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

    //  cjk/ctl font items have no configured slots,
    //  need no invalidate

    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
}

void ScDrawView::SetMarkedToLayer( sal_uInt8 nLayerNo )
{
    if (AreObjectsMarked())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( ScGlobal::GetRscString( STR_UNDO_SELATTR ) );

        const SdrMarkList& rMark = GetMarkedObjectList();
        sal_uLong nCount = rMark.GetMarkCount();
        for (sal_uLong i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
            if ( !pObj->ISA(SdrUnoObj) && (pObj->GetLayer() != SC_LAYER_INTERN) )
            {
                AddUndo( new SdrUndoObjectLayerChange( *pObj, pObj->GetLayer(), (SdrLayerID)nLayerNo) );
                pObj->SetLayer( nLayerNo );
            }
        }

        EndUndo();

        //  repaint is done in SetLayer

        pViewData->GetDocShell()->SetDrawModified();

        //  check mark list now instead of later in a timer
        CheckMarked();
        MarkListHasChanged();
    }
}

bool ScDrawView::HasMarkedControl() const
{
    SdrObjListIter aIter( GetMarkedObjectList() );
    for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        if( pObj->ISA( SdrUnoObj ) )
            return true;
    return false;
}

bool ScDrawView::HasMarkedInternal() const
{
    // internal objects should not be inside a group, but who knows...
    SdrObjListIter aIter( GetMarkedObjectList() );
    for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        if( pObj->GetLayer() == SC_LAYER_INTERN )
            return true;
    return false;
}

void ScDrawView::UpdateWorkArea()
{
    SdrPage* pPage = GetModel()->GetPage(static_cast<sal_uInt16>(nTab));
    if (pPage)
    {
        Point aPos;
        Size aPageSize( pPage->GetSize() );
        Rectangle aNewArea( aPos, aPageSize );
        if ( aPageSize.Width() < 0 )
        {
            //  RTL: from max.negative (left) to zero (right)
            aNewArea.Right() = 0;
            aNewArea.Left() = aPageSize.Width() + 1;
        }
        SetWorkArea( aNewArea );
    }
    else
    {
        OSL_FAIL("Page nicht gefunden");
    }
}

void ScDrawView::DoCut()
{
    DoCopy();
    BegUndo( ScGlobal::GetRscString( STR_UNDO_CUT ) );
    DeleteMarked();     // auf dieser View - von der 505f Umstellung nicht betroffen
    EndUndo();
}

void ScDrawView::GetScale( Fraction& rFractX, Fraction& rFractY ) const
{
    rFractX = aScaleX;
    rFractY = aScaleY;
}

void ScDrawView::RecalcScale()
{
    double nPPTX;
    double nPPTY;
    Fraction aZoomX(1,1);
    Fraction aZoomY(1,1);

    if (pViewData)
    {
        nTab = pViewData->GetTabNo();
        nPPTX = pViewData->GetPPTX();
        nPPTY = pViewData->GetPPTY();
        aZoomX = pViewData->GetZoomX();
        aZoomY = pViewData->GetZoomY();
    }
    else
    {
        Point aLogic = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
        nPPTX = aLogic.X() / 1000.0;
        nPPTY = aLogic.Y() / 1000.0;
                                            //! Zoom uebergeben ???
    }

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 1000;

    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev,aZoomX,aZoomY,nPPTX,nPPTY,
                            aScaleX,aScaleY );
}

void ScDrawView::DoConnect(SdrOle2Obj* pOleObj)
{
    if ( pViewData )
        pViewData->GetViewShell()->ConnectObject( pOleObj );
}

void ScDrawView::MarkListHasChanged()
{
    FmFormView::MarkListHasChanged();

    UpdateBrowser();

    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    // #i110829# remove the cell selection only if drawing objects are selected
    if ( !bInConstruct && GetMarkedObjectList().GetMarkCount() )
    {
        pViewSh->Unmark();      // remove cell selection

        //  end cell edit mode if drawing objects are selected
        SC_MOD()->InputEnterHandler();
    }

    //  IP deaktivieren

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = (ScClient*) pViewSh->GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
    {
        //  beim ViewShell::Activate aus dem Reset2Open nicht die Handles anzeigen
        pClient->DeactivateObject();
        //  Image-Ole wieder durch Grafik ersetzen passiert jetzt in ScClient::UIActivate
    }

    //  Ole-Objekt selektiert?

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    sal_uLong nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 0 && !pViewData->GetViewShell()->IsDrawSelMode() && !bInConstruct )
    {
        //  relock layers that may have been unlocked before
        LockBackgroundLayer();
        LockInternalLayer();
    }

    sal_Bool bSubShellSet = false;
    if (nMarkCount == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
        {
            pOle2Obj = (SdrOle2Obj*) pObj;
            if (!pDoc->IsChart(pObj) )
                pViewSh->SetOleObjectShell(sal_True);
            else
                pViewSh->SetChartShell(sal_True);
            bSubShellSet = sal_True;
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
            pViewSh->SetGraphicShell(sal_True);
            bSubShellSet = sal_True;
        }
        else if (pObj->GetObjIdentifier() == OBJ_MEDIA)
        {
            pViewSh->SetMediaShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // Verhindern, das beim Anlegen
                    || !pViewSh->IsDrawTextShell())     // eines TextObjekts auf die
        {                                               // DrawShell umgeschaltet wird.
            pViewSh->SetDrawShell(sal_True);                //@#70206#
        }
    }

    if ( nMarkCount && !bSubShellSet )
    {
        sal_Bool bOnlyControls = sal_True;
        sal_Bool bOnlyGraf     = sal_True;
        for (sal_uLong i=0; i<nMarkCount; i++)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            if ( pObj->ISA( SdrObjGroup ) )
            {
                const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
                sal_uLong nListCount = pLst->GetObjCount();
                if ( nListCount == 0 )
                {
                    //  An empty group (may occur during Undo) is no control or graphics object.
                    //  Creating the form shell during undo would lead to problems with the undo manager.
                    bOnlyControls = false;
                    bOnlyGraf = false;
                }
                for ( sal_uInt16 j = 0; j < nListCount; ++j )
                {
                    SdrObject *pSubObj = pLst->GetObj( j );

                    if (!pSubObj->ISA(SdrUnoObj))
                        bOnlyControls = false;
                    if (pSubObj->GetObjIdentifier() != OBJ_GRAF)
                        bOnlyGraf = false;

                    if ( !bOnlyControls && !bOnlyGraf ) break;
                }
            }
            else
            {
                if (!pObj->ISA(SdrUnoObj))
                    bOnlyControls = false;
                if (pObj->GetObjIdentifier() != OBJ_GRAF)
                    bOnlyGraf = false;
            }

            if ( !bOnlyControls && !bOnlyGraf ) break;
        }

        if(bOnlyControls)
        {
            pViewSh->SetDrawFormShell(sal_True);            // jetzt UNO-Controls
        }
        else if(bOnlyGraf)
        {
            pViewSh->SetGraphicShell(sal_True);
        }
        else if(nMarkCount>1)
        {
            pViewSh->SetDrawShell(sal_True);
        }
    }



    //  Verben anpassen

    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();
    sal_Bool bOle = pViewSh->GetViewFrame()->GetFrame().IsInPlace();
    uno::Sequence< embed::VerbDescriptor > aVerbs;
    if ( pOle2Obj && !bOle )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pOle2Obj->GetObjRef();
        DBG_ASSERT( xObj.is(), "SdrOle2Obj ohne ObjRef" );
        if (xObj.is())
            pViewSh->SetVerbs( xObj->getSupportedVerbs() );
        else
        {
            OSL_FAIL("SdrOle2Obj ohne ObjRef");
            pViewSh->SetVerbs( 0 );
        }
    }
    pViewSh->SetVerbs( aVerbs );

    //  Image-Map Editor

    if ( pOle2Obj )
        UpdateIMap( pOle2Obj );
    else if ( pGrafObj )
        UpdateIMap( pGrafObj );

    InvalidateAttribs();                // nach dem IMap-Editor Update
    InvalidateDrawTextAttrs();

    for(sal_uInt32 a(0L); a < PaintWindowCount(); a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

        if(OUTDEV_WINDOW == rOutDev.GetOutDevType())
        {
            ((Window&)rOutDev).Update();
        }
    }

    //  uno object for view returns drawing objects as selection,
    //  so it must notify its SelectionChangeListeners

    if (pViewFrame)
    {
        SfxFrame& rFrame = pViewFrame->GetFrame();
        uno::Reference<frame::XController> xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
            if (pImp)
                pImp->SelectionChanged();
        }
    }

    //  update selection transfer object

    pViewSh->CheckSelectionTransfer();

}

void ScDrawView::ModelHasChanged()
{
    SdrObject* pEditObj = GetTextEditObject();
    if ( pEditObj && !pEditObj->IsInserted() && pViewData )
    {
        //  SdrObjEditView::ModelHasChanged will end text edit in this case,
        //  so make sure the EditEngine's undo manager is no longer used.
        pViewData->GetViewShell()->SetDrawTextUndo(NULL);
        SetCreateMode();    // don't leave FuText in a funny state
    }

    FmFormView::ModelHasChanged();
}

void ScDrawView::UpdateUserViewOptions()
{
    if (pViewData)
    {
        const ScViewOptions&    rOpt = pViewData->GetOptions();
        const ScGridOptions&    rGrid = rOpt.GetGridOptions();

        sal_Bool bBigHdl = rOpt.GetOption( VOPT_BIGHANDLES );

        SetDragStripes( rOpt.GetOption( VOPT_HELPLINES ) );
        SetSolidMarkHdl( rOpt.GetOption( VOPT_SOLIDHANDLES ) );
        SetMarkHdlSizePixel( bBigHdl ? SC_HANDLESIZE_BIG : SC_HANDLESIZE_SMALL );

        SetGridVisible( rGrid.GetGridVisible() );
        SetSnapEnabled( rGrid.GetUseGridSnap() );
        SetGridSnap( rGrid.GetUseGridSnap() );

        Fraction aFractX( rGrid.GetFldDrawX(), rGrid.GetFldDivisionX() + 1 );
        Fraction aFractY( rGrid.GetFldDrawY(), rGrid.GetFldDivisionY() + 1 );
        SetSnapGridWidth( aFractX, aFractY );

        SetGridCoarse( Size( rGrid.GetFldDrawX(), rGrid.GetFldDrawY() ) );
        SetGridFine( Size( rGrid.GetFldDrawX() / (rGrid.GetFldDivisionX() + 1),
                           rGrid.GetFldDrawY() / (rGrid.GetFldDivisionY() + 1) ) );
    }
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

sal_Bool ScDrawView::SelectObject( const String& rName )
{
    UnmarkAll();

    SCTAB nObjectTab = 0;
    SdrObject* pFound = NULL;

    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(i));
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject && !pFound)
                {
                    if ( ScDrawLayer::GetVisibleName( pObject ) == rName )
                    {
                        pFound = pObject;
                        nObjectTab = i;
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( pFound )
    {
        ScTabView* pView = pViewData->GetView();
        if ( nObjectTab != nTab )                               // Tabelle umschalten
            pView->SetTabNo( nObjectTab );

        DBG_ASSERT( nTab == nObjectTab, "Tabellen umschalten hat nicht geklappt" );

        pView->ScrollToObject( pFound );

        /*  To select an object on the background layer, the layer has to
            be unlocked even if exclusive drawing selection mode is not active
            (this is reversed in MarkListHasChanged when nothing is selected) */
        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !pDoc->IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            UnlockBackgroundLayer();
        }

        SdrPageView* pPV = GetSdrPageView();
        MarkObj( pFound, pPV );
    }

    return ( pFound != NULL );
}


bool ScDrawView::InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, sal_uLong nOptions)
{
    //  Markierung nicht aendern, wenn Ole-Objekt aktiv
    //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

    if (pViewData)
    {
        SfxInPlaceClient* pClient = pViewData->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;
    }

    return InsertObjectAtView( pObj, rPV, nOptions );
}

SdrObject* ScDrawView::GetMarkedNoteCaption( ScDrawObjData** ppCaptData )
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( pViewData && (rMarkList.GetMarkCount() == 1) )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        if( ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObj, pViewData->GetTabNo() ) )
        {
            if( ppCaptData ) *ppCaptData = pCaptData;
            return pObj;
        }
    }
    return 0;
}

void ScDrawView::LockCalcLayer( SdrLayerID nLayer, bool bLock )
{
    SdrLayer* pLockLayer = GetModel()->GetLayerAdmin().GetLayerPerID( nLayer );
    if( pLockLayer && (IsLayerLocked( pLockLayer->GetName() ) != bLock) )
        SetLayerLocked( pLockLayer->GetName(), bLock );
}

void ScDrawView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    //! rWin richtig auswerten
    //! ggf Zoom aendern

    if ( pViewData && pViewData->GetActiveWin() == &rWin )
        pViewData->GetView()->MakeVisible( rRect );
}

void ScDrawView::DeleteMarked()
{
    // try to delete a note caption object with its cell note in the Calc document
    ScDrawObjData* pCaptData = 0;
    if( SdrObject* pCaptObj = GetMarkedNoteCaption( &pCaptData ) )
    {
        (void)pCaptObj; // prevent 'unused variable' compiler warning in pro builds
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        ScDocShell* pDocShell = pViewData ? pViewData->GetDocShell() : 0;
        ::svl::IUndoManager* pUndoMgr = pDocShell ? pDocShell->GetUndoManager() : 0;
        bool bUndo = pDrawLayer && pDocShell && pUndoMgr && pDoc->IsUndoEnabled();

        // remove the cell note from document, we are its owner now
        ScPostIt* pNote = pDoc->ReleaseNote( pCaptData->maStart );
        DBG_ASSERT( pNote, "ScDrawView::DeleteMarked - cell note missing in document" );
        if( pNote )
        {
            // rescue note data for undo (with pointer to caption object)
            ScNoteData aNoteData = pNote->GetNoteData();
            DBG_ASSERT( aNoteData.mpCaption == pCaptObj, "ScDrawView::DeleteMarked - caption object does not match" );
            // collect the drawing undo action created while deleting the note
            if( bUndo )
                pDrawLayer->BeginCalcUndo();
            // delete the note (already removed from document above)
            delete pNote;
            // add the undo action for the note
            if( bUndo )
                pUndoMgr->AddUndoAction( new ScUndoReplaceNote( *pDocShell, pCaptData->maStart, aNoteData, false, pDrawLayer->GetCalcUndo() ) );
            // repaint the cell to get rid of the note marker
            if( pDocShell )
                pDocShell->PostPaintCell( pCaptData->maStart );
            // done, return now to skip call of FmFormView::DeleteMarked()
            return;
        }
    }

    FmFormView::DeleteMarked();
}

SdrEndTextEditKind ScDrawView::ScEndTextEdit()
{
    sal_Bool bIsTextEdit = IsTextEdit();
    SdrEndTextEditKind eKind = SdrEndTextEdit();

    if ( bIsTextEdit && pViewData )
        pViewData->GetViewShell()->SetDrawTextUndo(NULL);   // "normaler" Undo-Manager

    return eKind;
}

void ScDrawView::MarkDropObj( SdrObject* pObj )
{
    if ( pDropMarkObj != pObj )
    {
        pDropMarkObj = pObj;
        ImplClearCalcDropMarker();

        if(pDropMarkObj)
        {
            pDropMarker = new SdrDropMarkerOverlay(*this, *pDropMarkObj);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
