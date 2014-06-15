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
#include <svx/sdrundomanager.hxx>
#include <svx/xbtmpit.hxx>

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
#include "gridwin.hxx"

#include "sc.hrc"

using namespace com::sun::star;

#define SC_HANDLESIZE_BIG       9

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

        bool bEx = pViewData->GetViewShell()->IsDrawSelMode();
        bool bProt = pDoc->IsTabProtected( nViewTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), true );
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

        SetSwapAsynchron(true);
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
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );
    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    rBindings.Invalidate( SID_TABLE_VERT_NONE );
    rBindings.Invalidate( SID_TABLE_VERT_CENTER );
    rBindings.Invalidate( SID_TABLE_VERT_BOTTOM );
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
        nEndRow = 20;

    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev,aZoomX,aZoomY,nPPTX,nPPTY,
                            aScaleX,aScaleY );
    SdrPageView* pPV = GetSdrPageView();
    if ( pViewData && pPV )
    {
        if ( SdrPage* pPage = pPV->GetPage() )
        {
            sal_uLong nCount = pPage->GetObjCount();
            for ( sal_uLong i = 0; i < nCount; i++ )
            {
                SdrObject* pObj = pPage->GetObj( i );
                // Align objects to nearset grid position
                SyncForGrid( pObj );
            }
        }
    }
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

    bool bSubShellSet = false;
    if (nMarkCount == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
        {
            pOle2Obj = (SdrOle2Obj*) pObj;
            if (!pDoc->IsChart(pObj) )
                pViewSh->SetOleObjectShell(true);
            else
                pViewSh->SetChartShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
            pViewSh->SetGraphicShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() == OBJ_MEDIA)
        {
            pViewSh->SetMediaShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // Verhindern, das beim Anlegen
                    || !pViewSh->IsDrawTextShell())     // eines TextObjekts auf die
        {                                               // DrawShell umgeschaltet wird.
            pViewSh->SetDrawShell(true);                //@#70206#
        }
    }

    if ( nMarkCount && !bSubShellSet )
    {
        bool bOnlyControls = true;
        bool bOnlyGraf     = true;
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
            pViewSh->SetDrawFormShell(true);            // jetzt UNO-Controls
        }
        else if(bOnlyGraf)
        {
            pViewSh->SetGraphicShell(true);
        }
        else if(nMarkCount>1)
        {
            pViewSh->SetDrawShell(true);
        }
    }

    //  Verben anpassen

    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();
    bool bOle = pViewSh->GetViewFrame()->GetFrame().IsInPlace();
    uno::Sequence< embed::VerbDescriptor > aVerbs;
    if ( pOle2Obj && !bOle )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pOle2Obj->GetObjRef();
        OSL_ENSURE( xObj.is(), "SdrOle2Obj ohne ObjRef" );
        if (xObj.is())
            aVerbs = xObj->getSupportedVerbs();
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

bool ScDrawView::SdrBeginTextEdit(
    SdrObject* pObj,
    SdrPageView* pPV,
    ::Window* pWinL,
    bool bIsNewObj,
    SdrOutliner* pGivenOutliner,
    OutlinerView* pGivenOutlinerView,
    bool bDontDeleteOutliner,
    bool bOnlyOneView,
    bool bGrabFocus )
{
    const bool bRet = FmFormView::SdrBeginTextEdit(
        pObj, pPV, pWinL, bIsNewObj,
        pGivenOutliner, pGivenOutlinerView, bDontDeleteOutliner,
        bOnlyOneView, bGrabFocus );

    ScTabViewShell* pViewSh = pViewData->GetViewShell();
    if ( pViewSh->GetViewFrame() )
    {
        SfxFrame& rFrame = pViewSh->GetViewFrame()->GetFrame();
        uno::Reference< frame::XController > xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
            if (pImp)
                pImp->SelectionChanged();
        }
    }

    return bRet;
}


SdrEndTextEditKind ScDrawView::SdrEndTextEdit( bool bDontDeleteReally )
{
    const SdrEndTextEditKind eRet = FmFormView::SdrEndTextEdit( bDontDeleteReally );

    ScTabViewShell* pViewSh = pViewData->GetViewShell();
    if ( pViewSh->GetViewFrame() )
    {
        SfxFrame& rFrame = pViewSh->GetViewFrame()->GetFrame();
        uno::Reference< frame::XController > xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
            if (pImp)
                pImp->SelectionChanged();
        }
    }

    return eRet;
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

        SetDragStripes( rOpt.GetOption( VOPT_HELPLINES ) );
        SetMarkHdlSizePixel( SC_HANDLESIZE_BIG );

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

SdrObject* ScDrawView::GetObjectByName(const OUString& rName)
{
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        sal_uInt16 nTabCount = pDoc->GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(i);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( ScDrawLayer::GetVisibleName( pObject ) == rName )
                    {
                        return pObject;
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }
    return 0;
}

//realize multi-selection of objects

bool ScDrawView::SelectCurrentViewObject( const OUString& rName )
{
    sal_uInt16 nObjectTab = 0;
    SdrObject* pFound = NULL;
    bool bUnMark = false;
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        sal_uInt16 nTabCount = pDoc->GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(i);
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
        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !pDoc->IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
            if (pLayer)
                SetLayerLocked( pLayer->GetName(), false );
        }
        SdrPageView* pPV = GetSdrPageView();
              bUnMark = IsObjMarked(pFound);
           MarkObj( pFound, pPV, bUnMark);
    }
    return bUnMark;
}

bool ScDrawView::SelectObject( const OUString& rName )
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
            OSL_ENSURE(pPage,"Page ?");
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

        OSL_ENSURE( nTab == nObjectTab, "Tabellen umschalten hat nicht geklappt" );

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

//If  object  is marked , return true , else return false .
bool ScDrawView::GetObjectIsMarked(  SdrObject* pObject  )
{
    bool bisMarked = false;
    if (pObject )
    {
        bisMarked = IsObjMarked(pObject);
    }
    return  bisMarked;
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
        OSL_ENSURE( pNote, "ScDrawView::DeleteMarked - cell note missing in document" );
        if( pNote )
        {
            // rescue note data for undo (with pointer to caption object)
            ScNoteData aNoteData = pNote->GetNoteData();
            OSL_ENSURE( aNoteData.mpCaption == pCaptObj, "ScDrawView::DeleteMarked - caption object does not match" );
            // collect the drawing undo action created while deleting the note
            if( bUndo )
                pDrawLayer->BeginCalcUndo(false);
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
    bool bIsTextEdit = IsTextEdit();
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

// In order to counteract the effects of rounding due to the nature of how the
// grid positions are calcuated and drawn we calculate the offset needed at the
// current zoom to be applied to an SrdObject when it is drawn in order to make
// sure that it's position relative to the nearest cell anchor doesn't change.
// Of course not all shape(s)/control(s) are cell anchored, if the
// object doesn't have a cell anchor we synthesise a temporary anchor.
void ScDrawView::SyncForGrid( SdrObject* pObj )
{
    // process members of a group shape separately
    if ( pObj->ISA( SdrObjGroup ) )
    {
        SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( sal_uLong i = 0, nCount = pLst->GetObjCount(); i < nCount; ++i )
            SyncForGrid( pLst->GetObj( i ) );
    }

    ScSplitPos eWhich = pViewData->GetActivePart();
    ScGridWindow* pGridWin = (ScGridWindow*)pViewData->GetActiveWin();
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    if ( pGridWin )
    {
        ScAddress aOldStt;
        if( pData )
        {
            aOldStt = pData->maStart;
        }
        else
        {
            // Page anchored object so...
            // synthesise an anchor ( but don't attach it to
            // the object as we want to maintain page anchoring )
            ScDrawObjData aAnchor;
            ScDrawLayer::GetCellAnchorFromPosition( *pObj, aAnchor, *pDoc, GetTab() );
            aOldStt = aAnchor.maStart;
        }
        MapMode aDrawMode = pGridWin->GetDrawMapMode();
        // find pos anchor position
        Point aOldPos( pDoc->GetColOffset( aOldStt.Col(), aOldStt.Tab()  ), pDoc->GetRowOffset( aOldStt.Row(), aOldStt.Tab() ) );
        aOldPos.X() = sc::TwipsToHMM( aOldPos.X() );
        aOldPos.Y() = sc::TwipsToHMM( aOldPos.Y() );
        // find position of same point on the screen ( e.g. grid )
        Point aCurPos =  pViewData->GetScrPos(  aOldStt.Col(), aOldStt.Row(), eWhich, true );
        Point aCurPosHmm = pGridWin->PixelToLogic(aCurPos, aDrawMode );
        Point aGridOff = ( aCurPosHmm - aOldPos );
        // fdo#63878 Fix the X position for RTL Sheet
        if( pDoc->IsNegativePage( GetTab() ) )
            aGridOff.setX( aCurPosHmm.getX() + aOldPos.getX() );
        pObj->SetGridOffset( aGridOff );
    }
}

// support enhanced text edit for draw objects
SdrUndoManager* ScDrawView::getSdrUndoManagerForEnhancedTextEdit() const
{
    return pDoc ? dynamic_cast< SdrUndoManager* >(pDoc->GetUndoManager()) : 0;
}

// #i123922# helper to apply a Graphic to an existing SdrObject
SdrObject* ScDrawView::ApplyGraphicToObject(
    SdrObject& rHitObject,
    const Graphic& rGraphic,
    const OUString& rBeginUndoText,
    const OUString& rFile,
    const OUString& rFilter)
{
    if(dynamic_cast< SdrGrafObj* >(&rHitObject))
    {
        SdrGrafObj* pNewGrafObj = (SdrGrafObj*)rHitObject.Clone();

        pNewGrafObj->SetGraphic(rGraphic);
        BegUndo(rBeginUndoText);
        ReplaceObjectAtView(&rHitObject, *GetSdrPageView(), pNewGrafObj);

        // set in all cases - the Clone() will have copied an existing link (!)
        pNewGrafObj->SetGraphicLink( rFile, ""/*TODO?*/, rFilter );

        EndUndo();
        return pNewGrafObj;
    }
    else if(rHitObject.IsClosedObj() && !dynamic_cast< SdrOle2Obj* >(&rHitObject))
    {
        AddUndo(new SdrUndoAttrObj(rHitObject));

        SfxItemSet aSet(GetModel()->GetItemPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);

        aSet.Put(XFillStyleItem(XFILL_BITMAP));
        aSet.Put(XFillBitmapItem(OUString(), rGraphic));
        rHitObject.SetMergedItemSetAndBroadcast(aSet);
        return &rHitObject;
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
