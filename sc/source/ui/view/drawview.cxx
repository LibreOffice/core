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

#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/sdrundomanager.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xbtmpit.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdrpagewindow.hxx>

#include <drawview.hxx>
#include <global.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <drawutil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <tabvwsh.hxx>
#include <client.hxx>
#include <scmod.hxx>
#include <drwlayer.hxx>
#include <docsh.hxx>
#include <viewuno.hxx>
#include <userdat.hxx>
#include <postit.hxx>
#include <undocell.hxx>
#include <gridwin.hxx>

#include <sc.hrc>

using namespace com::sun::star;

#define SC_HANDLESIZE_BIG       9

void ScDrawView::Construct()
{
    EnableExtendedKeyInputDispatcher(false);
    EnableExtendedMouseEventDispatcher(false);

    SetFrameDragSingles();

    SetMinMoveDistancePixel( 2 );
    SetHitTolerancePixel( 2 );

    if (pViewData)
    {
        SCTAB nViewTab = pViewData->GetTabNo();
        ShowSdrPage(GetModel()->GetPage(nViewTab));

        bool bEx = pViewData->GetViewShell()->IsDrawSelMode();
        bool bProt = rDoc.IsTabProtected( nViewTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            SetLayerLocked( pLayer->GetName() );
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

        SetSwapAsynchron();
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
    pDropMarker.reset();
}

ScDrawView::~ScDrawView()
{
    ImplClearCalcDropMarker();
}

void ScDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    const size_t nCount = rMrkList.GetMarkCount();
    for(size_t nPos=0; nPos<nCount; ++nPos )
    {
        SdrObject* pObj = rMrkList.GetMark(nPos)->GetMarkedSdrObj();
        if (ScDrawObjData *pAnchor = ScDrawLayer::GetObjDataTab(pObj, nTab))
        {
            if (ScTabView* pView = pViewData->GetView())
                pView->CreateAnchorHandles(maHdlList, pAnchor->maStart);
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
    rBindings.Invalidate( SID_ATTR_CHAR_BACK_COLOR );
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

void ScDrawView::SetMarkedToLayer( SdrLayerID nLayerNo )
{
    if (!AreObjectsMarked())
        return;

    //  #i11702# use SdrUndoObjectLayerChange for undo
    //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
    BegUndo( ScResId( STR_UNDO_SELATTR ) );

    const SdrMarkList& rMark = GetMarkedObjectList();
    const size_t nCount = rMark.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
        if ( dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr && (pObj->GetLayer() != SC_LAYER_INTERN) )
        {
            AddUndo( std::make_unique<SdrUndoObjectLayerChange>( *pObj, pObj->GetLayer(), nLayerNo) );
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

bool ScDrawView::HasMarkedControl() const
{
    SdrObjListIter aIter( GetMarkedObjectList() );
    for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        if( dynamic_cast<const SdrUnoObj*>( pObj) !=  nullptr )
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
        Size aPageSize( pPage->GetSize() );
        tools::Rectangle aNewArea( Point(), aPageSize );
        if ( aPageSize.Width() < 0 )
        {
            //  RTL: from max.negative (left) to zero (right)
            aNewArea.SetRight( 0 );
            aNewArea.SetLeft( aPageSize.Width() + 1 );
        }
        SetWorkArea( aNewArea );
    }
    else
    {
        OSL_FAIL("Page not found");
    }
}

void ScDrawView::DoCut()
{
    DoCopy();
    BegUndo( ScResId( STR_UNDO_CUT ) );
    DeleteMarked();     // In this View - not affected by 505f change
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
        Point aLogic = pDev->LogicToPixel(Point(1000,1000), MapMode(MapUnit::MapTwip));
        nPPTX = aLogic.X() / 1000.0;
        nPPTY = aLogic.Y() / 1000.0;
                                            //! Zoom, handed over ???
    }

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    rDoc.GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 20;

    ScDrawUtil::CalcScale(
        rDoc, nTab, 0, 0, nEndCol, nEndRow, pDev, aZoomX, aZoomY, nPPTX, nPPTY,
        aScaleX, aScaleY);

    // clear all evtl existing GridOffset vectors
    resetGridOffsetsForAllSdrPageViews();

    SdrPageView* pPV = GetSdrPageView();
    if ( !(pViewData && pPV) )
        return;

    if ( SdrPage* pPage = pPV->GetPage() )
    {
        const size_t nCount = pPage->GetObjCount();
        for ( size_t i = 0; i < nCount; ++i )
        {
            SdrObject* pObj = pPage->GetObj( i );
            // Align objects to nearest grid position
            SyncForGrid( pObj );
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

    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    // #i110829# remove the cell selection only if drawing objects are selected
    if ( !bInConstruct && GetMarkedObjectList().GetMarkCount() )
    {
        pViewSh->Unmark();      // remove cell selection

        //  end cell edit mode if drawing objects are selected
        SC_MOD()->InputEnterHandler();
    }

    // deactivate IP

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = static_cast<ScClient*>( pViewSh->GetIPClient() );
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
    {
        // do not display the handles for ViewShell::Activate from the Reset2Open
        pClient->DeactivateObject();
        // replacing image ole graphics is now done in ScClient::UIActivate
    }

    //  Select Ole object?

    SdrOle2Obj* pOle2Obj = nullptr;
    SdrGrafObj* pGrafObj = nullptr;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 0 && !pViewData->GetViewShell()->IsDrawSelMode() && !bInConstruct )
    {
        //  relock layers that may have been unlocked before
        LockBackgroundLayer(true);
        LockInternalLayer();
    }

    bool bSubShellSet = false;
    if (nMarkCount == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
        {
            pOle2Obj = static_cast<SdrOle2Obj*>(pObj);
            if (!ScDocument::IsChart(pObj) )
                pViewSh->SetOleObjectShell(true);
            else
                pViewSh->SetChartShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = static_cast<SdrGrafObj*>(pObj);
            pViewSh->SetGraphicShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() == OBJ_MEDIA)
        {
            pViewSh->SetMediaShell(true);
            bSubShellSet = true;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // prevent switching to the drawing shell
                    || !pViewSh->IsDrawTextShell())     // when creating a text object @#70206#
        {
            pViewSh->SetDrawShell(true);
        }
    }

    if ( nMarkCount && !bSubShellSet )
    {
        bool bOnlyControls = true;
        bool bOnlyGraf     = true;
        for (size_t i=0; i<nMarkCount; ++i)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            if ( auto pObjGroup = dynamic_cast<const SdrObjGroup*>( pObj) )
            {
                const SdrObjList *pLst = pObjGroup->GetSubList();
                const size_t nListCount = pLst->GetObjCount();
                if ( nListCount == 0 )
                {
                    //  An empty group (may occur during Undo) is no control or graphics object.
                    //  Creating the form shell during undo would lead to problems with the undo manager.
                    bOnlyControls = false;
                    bOnlyGraf = false;
                }
                for ( size_t j = 0; j < nListCount; ++j )
                {
                    SdrObject *pSubObj = pLst->GetObj( j );

                    if (dynamic_cast<const SdrUnoObj*>( pSubObj) ==  nullptr)
                        bOnlyControls = false;
                    if (pSubObj->GetObjIdentifier() != OBJ_GRAF)
                        bOnlyGraf = false;

                    if ( !bOnlyControls && !bOnlyGraf ) break;
                }
            }
            else
            {
                if (dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr)
                    bOnlyControls = false;
                if (pObj->GetObjIdentifier() != OBJ_GRAF)
                    bOnlyGraf = false;
            }

            if ( !bOnlyControls && !bOnlyGraf ) break;
        }

        if(bOnlyControls)
        {
            pViewSh->SetDrawFormShell(true);            // now UNO controls
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

    // adjust verbs

    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();
    bool bOle = pViewSh->GetViewFrame()->GetFrame().IsInPlace();
    uno::Sequence< embed::VerbDescriptor > aVerbs;
    if ( pOle2Obj && !bOle )
    {
        const uno::Reference < embed::XEmbeddedObject >& xObj = pOle2Obj->GetObjRef();
        OSL_ENSURE( xObj.is(), "SdrOle2Obj without ObjRef" );
        if (xObj.is())
            aVerbs = xObj->getSupportedVerbs();
    }
    pViewSh->SetVerbs( aVerbs );

    // image map editor

    if ( pOle2Obj )
        UpdateIMap( pOle2Obj );
    else if ( pGrafObj )
        UpdateIMap( pGrafObj );

    InvalidateAttribs();                // after the image map editor update
    InvalidateDrawTextAttrs();

    for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

        if(OUTDEV_WINDOW == rOutDev.GetOutDevType())
        {
            static_cast<vcl::Window&>(rOutDev).PaintImmediately();
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
            ScTabViewObj* pImp = comphelper::getUnoTunnelImplementation<ScTabViewObj>( xController );
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
    vcl::Window* pWinL,
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

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (OutlinerView* pView = GetTextEditOutlinerView())
        {
            tools::Rectangle aRectangle = pView->GetOutputArea();
            if (pWinL && pWinL->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
            OString sRectangle = aRectangle.toString();
            SfxLokHelper::notifyOtherViews(pViewSh, LOK_CALLBACK_VIEW_LOCK, "rectangle", sRectangle);
        }
    }

    if ( pViewSh->GetViewFrame() )
    {
        SfxFrame& rFrame = pViewSh->GetViewFrame()->GetFrame();
        uno::Reference< frame::XController > xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = comphelper::getUnoTunnelImplementation<ScTabViewObj>( xController );
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

    if (comphelper::LibreOfficeKit::isActive())
        SfxLokHelper::notifyOtherViews(pViewSh, LOK_CALLBACK_VIEW_LOCK, "rectangle", "EMPTY");

    if ( pViewSh->GetViewFrame() )
    {
        SfxFrame& rFrame = pViewSh->GetViewFrame()->GetFrame();
        uno::Reference< frame::XController > xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = comphelper::getUnoTunnelImplementation<ScTabViewObj>( xController );
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
        pViewData->GetViewShell()->SetDrawTextUndo(nullptr);
        SetCreateMode();    // don't leave FuText in a funny state
    }

    FmFormView::ModelHasChanged();
}

void ScDrawView::UpdateUserViewOptions()
{
    if (!pViewData)
        return;

    const ScViewOptions&    rOpt = pViewData->GetOptions();
    const ScGridOptions&    rGrid = rOpt.GetGridOptions();

    SetDragStripes( rOpt.GetOption( VOPT_HELPLINES ) );
    SetMarkHdlSizePixel( SC_HANDLESIZE_BIG );

    SetGridVisible( rGrid.GetGridVisible() );
    SetSnapEnabled( rGrid.GetUseGridSnap() );
    SetGridSnap( rGrid.GetUseGridSnap() );

    Fraction aFractX( rGrid.GetFieldDrawX(), rGrid.GetFieldDivisionX() + 1 );
    Fraction aFractY( rGrid.GetFieldDrawY(), rGrid.GetFieldDivisionY() + 1 );
    SetSnapGridWidth( aFractX, aFractY );

    SetGridCoarse( Size( rGrid.GetFieldDrawX(), rGrid.GetFieldDrawY() ) );
    SetGridFine( Size( rGrid.GetFieldDrawX() / (rGrid.GetFieldDivisionX() + 1),
                       rGrid.GetFieldDrawY() / (rGrid.GetFieldDivisionY() + 1) ) );
}

SdrObject* ScDrawView::GetObjectByName(const OUString& rName)
{
    SfxObjectShell* pShell = rDoc.GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        sal_uInt16 nTabCount = rDoc.GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(i);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
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
    return nullptr;
}

//realize multi-selection of objects

void ScDrawView::SelectCurrentViewObject( const OUString& rName )
{
    sal_uInt16 nObjectTab = 0;
    SdrObject* pFound = nullptr;
    SfxObjectShell* pShell = rDoc.GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        sal_uInt16 nTabCount = rDoc.GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(i);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
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
    if ( !pFound )
        return;

    ScTabView* pView = pViewData->GetView();
    if ( nObjectTab != nTab )                               // switch sheet
        pView->SetTabNo( nObjectTab );
    DBG_ASSERT( nTab == nObjectTab, "Switching sheets did not work" );
    pView->ScrollToObject( pFound );
    if ( pFound->GetLayer() == SC_LAYER_BACK &&
            !pViewData->GetViewShell()->IsDrawSelMode() &&
            !rDoc.IsTabProtected( nTab ) &&
            !pViewData->GetSfxDocShell()->IsReadOnly() )
    {
        SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), false );
    }
    SdrPageView* pPV = GetSdrPageView();
    const bool bUnMark = IsObjMarked(pFound);
    MarkObj( pFound, pPV, bUnMark);
}

bool ScDrawView::SelectObject( const OUString& rName )
{
    UnmarkAll();

    SCTAB nObjectTab = 0;
    SdrObject* pFound = nullptr;

    SfxObjectShell* pShell = rDoc.GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        SCTAB nTabCount = rDoc.GetTableCount();
        for (SCTAB i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(i));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
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
        if ( nObjectTab != nTab )                               // switch sheet
            pView->SetTabNo( nObjectTab );

        OSL_ENSURE( nTab == nObjectTab, "Switching sheets did not work" );

        pView->ScrollToObject( pFound );

        /*  To select an object on the background layer, the layer has to
            be unlocked even if exclusive drawing selection mode is not active
            (this is reversed in MarkListHasChanged when nothing is selected) */
        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !rDoc.IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            LockBackgroundLayer(false);
        }

        SdrPageView* pPV = GetSdrPageView();
        MarkObj( pFound, pPV );
    }

    return ( pFound != nullptr );
}

//If  object  is marked , return true , else return false .
bool ScDrawView::GetObjectIsMarked(  const SdrObject* pObject  )
{
    bool bisMarked = false;
    if (pObject )
    {
        bisMarked = IsObjMarked(pObject);
    }
    return  bisMarked;
}

bool ScDrawView::InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV)
{
    SdrInsertFlags nOptions=SdrInsertFlags::NONE;
    // Do not change marks when the ole object is active
    // (for Drop from ole object would otherwise be deactivated in the middle of ExecuteDrag!)

    if (pViewData)
    {
        SfxInPlaceClient* pClient = pViewData->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SdrInsertFlags::DONTMARK;
    }

    return InsertObjectAtView(pObj, rPV, nOptions);
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
    return nullptr;
}

void ScDrawView::LockCalcLayer( SdrLayerID nLayer, bool bLock )
{
    SdrLayer* pLockLayer = GetModel()->GetLayerAdmin().GetLayerPerID( nLayer );
    if( pLockLayer && (IsLayerLocked( pLockLayer->GetName() ) != bLock) )
        SetLayerLocked( pLockLayer->GetName(), bLock );
}

void ScDrawView::MakeVisible( const tools::Rectangle& rRect, vcl::Window& rWin )
{
    //! Evaluate rWin properly
    //! change zoom if necessary

    if ( pViewData && pViewData->GetActiveWin() == &rWin )
        pViewData->GetView()->MakeVisible( rRect );
}

SfxViewShell* ScDrawView::GetSfxViewShell() const
{
    return pViewData->GetViewShell();
}

void ScDrawView::DeleteMarked()
{
    // try to delete a note caption object with its cell note in the Calc document
    ScDrawObjData* pCaptData = nullptr;
    if( SdrObject* pCaptObj = GetMarkedNoteCaption( &pCaptData ) )
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        ScDocShell* pDocShell = pViewData ? pViewData->GetDocShell() : nullptr;
        SfxUndoManager* pUndoMgr = pDocShell ? pDocShell->GetUndoManager() : nullptr;
        bool bUndo = pDrawLayer && pDocShell && pUndoMgr && rDoc.IsUndoEnabled();

        // remove the cell note from document, we are its owner now
        std::unique_ptr<ScPostIt> pNote = rDoc.ReleaseNote( pCaptData->maStart );
        OSL_ENSURE( pNote, "ScDrawView::DeleteMarked - cell note missing in document" );
        if( pNote )
        {
            // rescue note data for undo (with pointer to caption object)
            ScNoteData aNoteData = pNote->GetNoteData();
            OSL_ENSURE( aNoteData.mxCaption.get() == pCaptObj, "ScDrawView::DeleteMarked - caption object does not match" );
            // collect the drawing undo action created while deleting the note
            if( bUndo )
                pDrawLayer->BeginCalcUndo(false);
            // delete the note (already removed from document above)
            pNote.reset();
            // add the undo action for the note
            if( bUndo )
                pUndoMgr->AddUndoAction( std::make_unique<ScUndoReplaceNote>( *pDocShell, pCaptData->maStart, aNoteData, false, pDrawLayer->GetCalcUndo() ) );
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

    if (bIsTextEdit)
        pViewData->GetViewShell()->SetDrawTextUndo(nullptr);   // the "normal" undo manager

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
            pDropMarker.reset( new SdrDropMarkerOverlay(*this, *pDropMarkObj) );
        }
    }
}

// In order to counteract the effects of rounding due to the nature of how the
// grid positions are calculated and drawn we calculate the offset needed at the
// current zoom to be applied to an SrdObject when it is drawn in order to make
// sure that it's position relative to the nearest cell anchor doesn't change.
// Of course not all shape(s)/control(s) are cell anchored, if the
// object doesn't have a cell anchor we synthesise a temporary anchor.
void ScDrawView::SyncForGrid( SdrObject* pObj )
{
    // process members of a group shape separately
    if ( auto pObjGroup = dynamic_cast<const SdrObjGroup*>( pObj) )
    {
        SdrObjList *pLst = pObjGroup->GetSubList();
        for ( size_t i = 0, nCount = pLst->GetObjCount(); i < nCount; ++i )
            SyncForGrid( pLst->GetObj( i ) );
    }

    ScSplitPos eWhich = pViewData->GetActivePart();
    ScGridWindow* pGridWin = pViewData->GetActiveWin();
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    if ( !pGridWin )
        return;

    ScAddress aOldStt;
    if( pData && pData->maStart.IsValid())
    {
        aOldStt = pData->maStart;
    }
    else
    {
        // Page anchored object so...
        // synthesise an anchor ( but don't attach it to
        // the object as we want to maintain page anchoring )
        ScDrawObjData aAnchor;
        const tools::Rectangle aObjRect(pObj->GetLogicRect());
        ScDrawLayer::GetCellAnchorFromPosition(
            aObjRect,
            aAnchor,
            rDoc,
            GetTab());
        aOldStt = aAnchor.maStart;
    }
    MapMode aDrawMode = pGridWin->GetDrawMapMode();
    // find pos anchor position
    Point aOldPos( rDoc.GetColOffset( aOldStt.Col(), aOldStt.Tab()  ), rDoc.GetRowOffset( aOldStt.Row(), aOldStt.Tab() ) );
    aOldPos.setX( sc::TwipsToHMM( aOldPos.X() ) );
    aOldPos.setY( sc::TwipsToHMM( aOldPos.Y() ) );
    // find position of same point on the screen ( e.g. grid )
    Point aCurPos =  pViewData->GetScrPos(  aOldStt.Col(), aOldStt.Row(), eWhich, true );
    Point aCurPosHmm = pGridWin->PixelToLogic(aCurPos, aDrawMode );
    Point aGridOff = aCurPosHmm - aOldPos;
    // fdo#63878 Fix the X position for RTL Sheet
    if( rDoc.IsNegativePage( GetTab() ) )
        aGridOff.setX( aCurPosHmm.getX() + aOldPos.getX() );
}

void ScDrawView::resetGridOffsetsForAllSdrPageViews()
{
    SdrPageView* pPageView(GetSdrPageView());

    if(nullptr == pPageView)
        return;

    for(sal_uInt32 a(0); a < pPageView->PageWindowCount(); a++)
    {
        SdrPageWindow* pPageWindow(pPageView->GetPageWindow(a));
        assert(pPageWindow && "SdrView::SetMasterPagePaintCaching: Corrupt SdrPageWindow list (!)");

        if(nullptr != pPageWindow)
        {
            sdr::contact::ObjectContact& rObjectContact(pPageWindow->GetObjectContact());

            if(rObjectContact.supportsGridOffsets())
            {
                rObjectContact.resetAllGridOffsets();
            }
        }
    }
}

bool ScDrawView::calculateGridOffsetForSdrObject(
    SdrObject& rSdrObject,
    basegfx::B2DVector& rTarget) const
{
    if (comphelper::LibreOfficeKit::isActive() &&
            !comphelper::LibreOfficeKit::isCompatFlagSet(
                    comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        return false;

    ScGridWindow* pGridWin(pViewData->GetActiveWin());

    if(nullptr == pGridWin)
    {
        return false;
    }

    ScDrawObjData* pData(ScDrawLayer::GetObjData(&rSdrObject));
    ScAddress aOldStt;

    if(nullptr != pData && pData->maStart.IsValid())
    {
        aOldStt = pData->maStart;
    }
    else
    {
        // Page anchored object so...
        // synthesise an anchor ( but don't attach it to
        // the object as we want to maintain page anchoring )
        ScDrawObjData aAnchor;
        const tools::Rectangle aObjRect(rSdrObject.GetLogicRect());
        ScDrawLayer::GetCellAnchorFromPosition(
            aObjRect,
            aAnchor,
            rDoc,
            GetTab());
        aOldStt = aAnchor.maStart;
    }

    MapMode aDrawMode = pGridWin->GetDrawMapMode();

    // find pos anchor position
    Point aOldPos(rDoc.GetColOffset(aOldStt.Col(), aOldStt.Tab()), rDoc.GetRowOffset(aOldStt.Row(), aOldStt.Tab()));
    aOldPos.setX(sc::TwipsToHMM(aOldPos.X()));
    aOldPos.setY(sc::TwipsToHMM(aOldPos.Y()));

    // find position of same point on the screen ( e.g. grid )
    ScSplitPos eWhich(pViewData->GetActivePart());
    Point aCurPos(pViewData->GetScrPos(aOldStt.Col(), aOldStt.Row(), eWhich, true));
    Point aCurPosHmm(pGridWin->PixelToLogic(aCurPos, aDrawMode));
    Point aGridOff(aCurPosHmm - aOldPos);

    // fdo#63878 Fix the X position for RTL Sheet
    if(rDoc.IsNegativePage(GetTab()))
    {
        aGridOff.setX(aCurPosHmm.getX() + aOldPos.getX());
    }

    rTarget.setX(aGridOff.X());
    rTarget.setY(aGridOff.Y());
    return true;
}

bool ScDrawView::calculateGridOffsetForB2DRange(
    const basegfx::B2DRange& rB2DRange,
    basegfx::B2DVector& rTarget) const
{
    ScGridWindow* pGridWin(pViewData->GetActiveWin());

    if(nullptr == pGridWin || rB2DRange.isEmpty())
    {
        return false;
    }

    // No SdrObject, so synthesise an anchor ( but don't attach it to
    // the object as we want to maintain page anchoring )
    ScDrawObjData aAnchor;
    const tools::Rectangle aRectangle(
        basegfx::fround(rB2DRange.getMinX()), basegfx::fround(rB2DRange.getMinY()),
        basegfx::fround(rB2DRange.getMaxX()), basegfx::fround(rB2DRange.getMaxY()));
    ScDrawLayer::GetCellAnchorFromPosition(
        aRectangle,
        aAnchor,
        rDoc,
        GetTab());
    ScAddress aOldStt(aAnchor.maStart);

    MapMode aDrawMode = pGridWin->GetDrawMapMode();

    // find pos anchor position
    Point aOldPos(rDoc.GetColOffset(aOldStt.Col(), aOldStt.Tab()), rDoc.GetRowOffset(aOldStt.Row(), aOldStt.Tab()));
    aOldPos.setX(sc::TwipsToHMM(aOldPos.X()));
    aOldPos.setY(sc::TwipsToHMM(aOldPos.Y()));

    // find position of same point on the screen ( e.g. grid )
    ScSplitPos eWhich(pViewData->GetActivePart());
    Point aCurPos(pViewData->GetScrPos(aOldStt.Col(), aOldStt.Row(), eWhich, true));
    Point aCurPosHmm(pGridWin->PixelToLogic(aCurPos, aDrawMode));
    Point aGridOff(aCurPosHmm - aOldPos);

    // fdo#63878 Fix the X position for RTL Sheet
    if(rDoc.IsNegativePage(GetTab()))
    {
        aGridOff.setX(aCurPosHmm.getX() + aOldPos.getX());
    }

    rTarget.setX(aGridOff.X());
    rTarget.setY(aGridOff.Y());
    return true;
}

// support enhanced text edit for draw objects
SdrUndoManager* ScDrawView::getSdrUndoManagerForEnhancedTextEdit() const
{
    return dynamic_cast<SdrUndoManager*>(rDoc.GetUndoManager());
}

// #i123922# helper to apply a Graphic to an existing SdrObject
SdrObject* ScDrawView::ApplyGraphicToObject(
    SdrObject& rHitObject,
    const Graphic& rGraphic,
    const OUString& rBeginUndoText,
    const OUString& rFile)
{
    if(dynamic_cast< SdrGrafObj* >(&rHitObject))
    {
        SdrGrafObj* pNewGrafObj(static_cast<SdrGrafObj*>(rHitObject.CloneSdrObject(rHitObject.getSdrModelFromSdrObject())));

        pNewGrafObj->SetGraphic(rGraphic);
        BegUndo(rBeginUndoText);
        ReplaceObjectAtView(&rHitObject, *GetSdrPageView(), pNewGrafObj);

        // set in all cases - the Clone() will have copied an existing link (!)
        pNewGrafObj->SetGraphicLink( rFile );

        EndUndo();
        return pNewGrafObj;
    }
    else if(rHitObject.IsClosedObj() && !dynamic_cast< SdrOle2Obj* >(&rHitObject))
    {
        AddUndo(std::make_unique<SdrUndoAttrObj>(rHitObject));

        SfxItemSet aSet(GetModel()->GetItemPool(), svl::Items<XATTR_FILLSTYLE, XATTR_FILLBITMAP>{});

        aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        aSet.Put(XFillBitmapItem(OUString(), rGraphic));
        rHitObject.SetMergedItemSetAndBroadcast(aSet);
        return &rHitObject;
    }

    return nullptr;
}

// Own derivation of ObjectContact to allow on-demand calculation of
// GridOffset for non-linear ViewToDevice transformation (calc)
namespace sdr::contact
{
        namespace {

        class ObjectContactOfScDrawView final : public ObjectContactOfPageView
        {
        private:
            // The ScDrawView to work on
            const ScDrawView&       mrScDrawView;

        public:
            explicit ObjectContactOfScDrawView(
                const ScDrawView& rScDrawView,
                SdrPageWindow& rPageWindow,
                const char* pDebugName);

            virtual bool supportsGridOffsets() const override;
            virtual void calculateGridOffsetForViewOjectContact(
                basegfx::B2DVector& rTarget,
                const ViewObjectContact& rClient) const override;
            virtual void calculateGridOffsetForB2DRange(
                basegfx::B2DVector& rTarget,
                const basegfx::B2DRange& rB2DRange) const override;
        };

        }

        ObjectContactOfScDrawView::ObjectContactOfScDrawView(
            const ScDrawView& rScDrawView,
            SdrPageWindow& rPageWindow,
            const char* pDebugName)
        :   ObjectContactOfPageView(rPageWindow, pDebugName),
            mrScDrawView(rScDrawView)
        {
        }

        bool ObjectContactOfScDrawView::supportsGridOffsets() const
        {
            // Except when scPrintTwipsMsgs flag is active,
            // Calc in LOK mode directly sets pixel-aligned logical coordinates for draw-objects.
            if (comphelper::LibreOfficeKit::isActive() &&
                !comphelper::LibreOfficeKit::isCompatFlagSet(
                    comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
                return false;

            // no GridOffset support for printer
            if(isOutputToPrinter())
            {
                return false;
            }

            // no GridOffset support for PDF export
            if(isOutputToPDFFile())
            {
                return false;
            }

            // yes - we support it
            return true;
        }

        void ObjectContactOfScDrawView::calculateGridOffsetForViewOjectContact(
            basegfx::B2DVector& rTarget,
            const ViewObjectContact& rClient) const
        {
            // Here the on-demand calculation happens. Try to access the SdrObject involved
            SdrObject* pTargetSdrObject(rClient.GetViewContact().TryToGetSdrObject());

            if(nullptr != pTargetSdrObject)
            {
                mrScDrawView.calculateGridOffsetForSdrObject(
                    *pTargetSdrObject,
                    rTarget);
            }
        }

        void ObjectContactOfScDrawView::calculateGridOffsetForB2DRange(
            basegfx::B2DVector& rTarget,
            const basegfx::B2DRange& rB2DRange) const
        {
            // Here the on-demand calculation happens. Try to access the SdrObject involved
            if(!rB2DRange.isEmpty())
            {
                mrScDrawView.calculateGridOffsetForB2DRange(
                    rB2DRange,
                    rTarget);
            }
        }
}

// Create own derivation of ObjectContact for calc
sdr::contact::ObjectContact* ScDrawView::createViewSpecificObjectContact(
    SdrPageWindow& rPageWindow,
    const char* pDebugName) const
{
    return new sdr::contact::ObjectContactOfScDrawView(
        *this,
        rPageWindow,
        pDebugName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
