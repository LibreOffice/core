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
#include <svx/svdlegacy.hxx>
#include <svx/sdrundomanager.hxx>

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
#include "document.hxx"

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
    EnableExtendedKeyInputDispatcher(sal_False);
    EnableExtendedMouseEventDispatcher(sal_False);
    EnableExtendedCommandEventDispatcher(sal_False);

    SetFrameHandles(true);
//  SetSolidMarkHdl(true);              // einstellbar -> UpdateUserViewOptions

    SetMinMoveDistancePixel( 2 );
    SetHitTolerancePixel( 2 );

    if (pViewData)
    {
        SCTAB nViewTab = pViewData->GetTabNo();
        ShowSdrPage(*getSdrModelFromSdrView().GetPage(nViewTab));

        sal_Bool bEx = pViewData->GetViewShell()->IsDrawSelMode();
        sal_Bool bProt = pDoc->IsTabProtected( nViewTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = getSdrModelFromSdrView().GetModelLayerAdmin();
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
            SetActiveLayer( pLayer->GetName() );        // FRONT als aktiven Layer setzen
        }
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_HIDDEN);
        if (pLayer)
        {
            SetLayerLocked( pLayer->GetName(), bProt );
            SetLayerVisible( pLayer->GetName(), sal_False);
        }

        SetSwapAsynchron(true);
    }
    else
    {
        ShowSdrPage(*getSdrModelFromSdrView().GetPage(nTab));
    }

    UpdateUserViewOptions();
    RecalcScale();
    UpdateWorkArea();

    bInConstruct = sal_False;
}

void ScDrawView::ImplClearCalcDropMarker()
{
    if(pDropMarker)
    {
        delete pDropMarker;
        pDropMarker = 0L;
    }
}

__EXPORT ScDrawView::~ScDrawView()
{
    ImplClearCalcDropMarker();
}

void ScDrawView::AddCustomHdl(SdrHdlList& rTarget)
{
    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for(sal_uInt32 nPos(0); nPos < aSelection.size(); nPos++ )
    {
        const SdrObject* pObj = aSelection[nPos];

        if(ScDrawLayer::GetAnchor(pObj) == SCA_CELL)
        {
            const sal_Int32 nDelta = 1;

            Rectangle aBoundRect = sdr::legacy::GetBoundRect(*pObj);
            Point aPos;
            if (bNegativePage)
            {
                aPos = aBoundRect.TopRight();
                aPos.X() = -aPos.X();           // so the loop below is the same
            }
            else
                aPos = aBoundRect.TopLeft();
            long nPosX = (long) (aPos.X() / HMM_PER_TWIPS) + nDelta;
            long nPosY = (long) (aPos.Y() / HMM_PER_TWIPS) + nDelta;

            SCCOL nCol;
            sal_Int32 nWidth = 0;

            for(nCol=0; nCol<=MAXCOL && nWidth<=nPosX; nCol++)
                nWidth += pDoc->GetColWidth(nCol,nTab);

            if(nCol > 0)
                --nCol;

            SCROW nRow = nPosY <= 0 ? 0 : pDoc->GetRowForHeight( nTab,
                    (sal_uLong) nPosY);
            if(nRow > 0)
                --nRow;

            ScTabView* pView = pViewData->GetView();
            ScAddress aScAddress(nCol, nRow, nTab);
            pView->CreateAnchorHandles(rTarget, aScAddress);
        }
    }
}

void ScDrawView::InvalidateAttribs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

        // echte Statuswerte:
    rBindings.InvalidateAll( sal_True );
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
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
}

//void ScDrawView::DrawMarks( OutputDevice* pOut ) const
//{
//  DBG_ASSERT(pOut, "ScDrawView::DrawMarks: No OutputDevice (!)");
//  SdrPaintWindow* pPaintWindow = FindPaintWindow(*pOut);
//
//  if(pPaintWindow)
//  {
//      if(pPaintWindow->isXorVisible())
//      {
//          ToggleShownXor(pOut, 0L);
//      }
//  }
//}

void ScDrawView::SetMarkedToLayer( sal_uInt8 nLayerNo )
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if (aSelection.size())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( ScGlobal::GetRscString( STR_UNDO_SELATTR ) );

        for (sal_uInt32 i(0); i < aSelection.size(); i++)
        {
            SdrObject* pObj = aSelection[i];

            if ( !dynamic_cast< SdrUnoObj* >(pObj) && (pObj->GetLayer() != SC_LAYER_INTERN) )
            {
                AddUndo( new SdrUndoObjectLayerChange( *pObj, pObj->GetLayer(), (SdrLayerID)nLayerNo) );
                pObj->SetLayer( nLayerNo );
            }
        }

        EndUndo();

        //  repaint is done in SetLayer

        pViewData->GetDocShell()->SetDrawModified();
    }
}

bool ScDrawView::HasMarkedControl() const
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            SdrObjListIter aIter( *aSelection[a] );

            for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
            {
                if( dynamic_cast< SdrUnoObj* >(pObj) )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool ScDrawView::HasMarkedInternal() const
{
    // internal objects should not be inside a group, but who knows...
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            SdrObjListIter aIter( *aSelection[a] );

            for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
            {
                if( pObj->GetLayer() == SC_LAYER_INTERN )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void ScDrawView::UpdateWorkArea()
{
    SdrPage* pPage = getSdrModelFromSdrView().GetPage(static_cast< sal_uInt32 >(nTab));
    if (pPage)
    {
        const basegfx::B2DRange aNewArea(basegfx::B2DPoint(0.0, 0.0), pPage->GetPageScale());

        SetWorkArea( aNewArea );
    }
    else
    {
        DBG_ERROR("Page nicht gefunden");
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
        nEndRow = 20;   // #i116848# instead of a large row number for an empty sheet, heights are multiplied in CalcScale

    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev,aZoomX,aZoomY,nPPTX,nPPTY,
                            aScaleX,aScaleY );
}

void ScDrawView::DoConnect(SdrOle2Obj* pOleObj)
{
    if ( pViewData )
        pViewData->GetViewShell()->ConnectObject( pOleObj );
}

namespace
{
    void triggerSelectionChanged(ScTabViewShell* pViewSh)
    {
        if(pViewSh && pViewSh->GetViewFrame())
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
    }
} // end of anonymous namespace

void ScDrawView::handleSelectionChange()
{
    // call parent
    FmFormView::handleSelectionChange();

    // local reactions
    UpdateBrowser();

    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    // #i110829# remove the cell selection only if drawing objects are selected
    if ( !bInConstruct && areSdrObjectsSelected() )
    {
        pViewSh->Unmark();      // remove cell selection

        //  #65379# end cell edit mode if drawing objects are selected
        SC_MOD()->InputEnterHandler();
    }

    //  IP deaktivieren

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = (ScClient*) pViewSh->GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
    {
        //  #41730# beim ViewShell::Activate aus dem Reset2Open nicht die Handles anzeigen
        pClient->DeactivateObject();
    }

    //  Ole-Objekt selektiert?

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    SdrMediaObj* pMediaObj = NULL;

    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if ( !aSelection.size() == 0 && !pViewData->GetViewShell()->IsDrawSelMode() && !bInConstruct )
    {
        //  relock layers that may have been unlocked before
        LockBackgroundLayer();
        LockInternalLayer();
    }

    sal_Bool bSubShellSet = sal_False;
    if (1 == aSelection.size())
    {
        SdrObject* pObj = aSelection[0];

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
            pMediaObj = (SdrMediaObj*) pObj;
            pViewSh->SetMediaShell(sal_True);
            bSubShellSet = sal_True;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // Verhindern, das beim Anlegen
                    || !pViewSh->IsDrawTextShell())     // eines TextObjekts auf die
        {                                               // DrawShell umgeschaltet wird.
            pViewSh->SetDrawShell(sal_True);                //@#70206#
        }
    }

    if ( aSelection.size() && !bSubShellSet )
    {
        sal_Bool bOnlyControls = sal_True;
        sal_Bool bOnlyGraf     = sal_True;

        for (sal_uInt32 i(0); i < aSelection.size(); i++)
        {
            SdrObject* pObj = aSelection[i];

            if ( dynamic_cast< SdrObjGroup* >(pObj) )
            {
                const SdrObjList* pLst = pObj->getChildrenOfSdrObject();
                sal_uInt32 nListCount(pLst ? pLst->GetObjCount() : 0);

                if ( !nListCount )
                {
                    //  #104156# An empty group (may occur during Undo) is no control or graphics object.
                    //  Creating the form shell during undo would lead to problems with the undo manager.
                    bOnlyControls = sal_False;
                    bOnlyGraf = sal_False;
                }

                for ( sal_uInt32 j(0); j < nListCount; ++j )
                {
                    SdrObject *pSubObj = pLst->GetObj( j );

                    if (!dynamic_cast< SdrUnoObj* >(pSubObj))
                        bOnlyControls = sal_False;
                    if (pSubObj->GetObjIdentifier() != OBJ_GRAF)
                        bOnlyGraf = sal_False;

                    if ( !bOnlyControls && !bOnlyGraf ) break;
                }
            }
            else
            {
                if (!dynamic_cast< SdrUnoObj* >(pObj))
                    bOnlyControls = sal_False;
                if (pObj->GetObjIdentifier() != OBJ_GRAF)
                    bOnlyGraf = sal_False;
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
        else if(aSelection.size() > 1)
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

    // uno object for view returns drawing objects as selection,
    // so it must notify its SelectionChangeListeners
    triggerSelectionChanged(pViewSh);

    //  update selection transfer object

    pViewSh->CheckSelectionTransfer();

}

bool ScDrawView::SdrBeginTextEdit(
    SdrObject* pObj,
    ::Window* pWin,
    bool bIsNewObj,
    SdrOutliner* pGivenOutliner,
    OutlinerView* pGivenOutlinerView,
    bool bDontDeleteOutliner,
    bool bOnlyOneView,
    bool bGrabFocus)
{
    const bool bRetval(
        FmFormView::SdrBeginTextEdit(
            pObj, pWin, bIsNewObj, pGivenOutliner,  pGivenOutlinerView,
            bDontDeleteOutliner, bOnlyOneView, bGrabFocus));

    // uno object for view returns drawing objects as selection,
    // so it must notify its SelectionChangeListeners

    // TTTT: Not sure if this is the right place to do this; a
    // selection change *should* call ScDrawView::handleSelectionChange() above
    // which also uses triggerSelectionChanged. Thus, SdrBeginTextEdit and
    // SdrEndTextEdit overloads should be obsolete

    triggerSelectionChanged(pViewData->GetViewShell());

    return bRetval;
}

SdrEndTextEditKind ScDrawView::SdrEndTextEdit(bool bDontDeleteReally)
{
    const SdrEndTextEditKind aRetval(FmFormView::SdrEndTextEdit(bDontDeleteReally));

    // uno object for view returns drawing objects as selection,
    // so it must notify its SelectionChangeListeners
    triggerSelectionChanged(pViewData->GetViewShell());

    return aRetval;
}

void __EXPORT ScDrawView::LazyReactOnObjectChanges()
{
    SdrObject* pEditObj = GetTextEditObject();
    if ( pEditObj && !pEditObj->IsObjectInserted() && pViewData )
    {
        //  #111700# SdrObjEditView::LazyReactOnObjectChanges will end text edit in this case,
        //  so make sure the EditEngine's undo manager is no longer used.
        pViewData->GetViewShell()->SetDrawTextUndo(NULL);
        SetViewEditMode(SDREDITMODE_CREATE);    // don't leave FuText in a funny state
    }

    FmFormView::LazyReactOnObjectChanges();
}

void __EXPORT ScDrawView::UpdateUserViewOptions()
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

        //  Snap from grid options is no longer used
//      SetSnapGrid( Size( rGrid.GetFldSnapX(), rGrid.GetFldSnapY() ) );

        SetSnapGridWidth(
            (double)rGrid.GetFldDrawX() / (rGrid.GetFldDivisionX() + 1.0),
            (double)rGrid.GetFldDrawY() / (rGrid.GetFldDivisionY() + 1.0));

        SetGridCoarse( Size( rGrid.GetFldDrawX(), rGrid.GetFldDrawY() ) );
        SetGridFine( Size( rGrid.GetFldDrawX() / (rGrid.GetFldDivisionX() + 1),
                           rGrid.GetFldDrawY() / (rGrid.GetFldDivisionY() + 1) ) );
    }
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

//IAccessibility2 Implementation 2009-----
SdrObject* ScDrawView::GetObjectByName(const String& rName)
{
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel& rDrawLayer = getSdrModelFromSdrView();
        sal_uInt16 nTabCount = pDoc->GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount; i++)
        {
            SdrPage* pPage = rDrawLayer.GetPage(i);
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
//Solution: realize multi-selection of objects
//==================================================
sal_Bool ScDrawView::SelectCurrentViewObject( const String& rName )
{
    sal_uInt16 nObjectTab = 0;
    SdrObject* pFound = NULL;
       sal_Bool bUnMark=sal_False;
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel& rDrawLayer = getSdrModelFromSdrView();
        sal_uInt16 nTabCount = pDoc->GetTableCount();
        for (sal_uInt16 i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = rDrawLayer.GetPage(i);
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
            SdrLayer* pLayer = getSdrModelFromSdrView().GetModelLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
            if (pLayer)
                SetLayerLocked( pLayer->GetName(), sal_False );
        }
        bUnMark = isSdrObjectSelected(*pFound);
        MarkObj(*pFound, bUnMark);
    }
    return ( bUnMark );
}
//-----IAccessibility2 Implementation 2009
sal_Bool ScDrawView::SelectObject( const String& rName )
{
    UnmarkAll();

    SCTAB nObjectTab = 0;
    SdrObject* pFound = NULL;

    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = getSdrModelFromSdrView().GetPage(static_cast<sal_uInt16>(i));
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

        /*  #61585# To select an object on the background layer, the layer has to
            be unlocked even if exclusive drawing selection mode is not active */
        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !pDoc->IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            UnlockBackgroundLayer();
        }

        MarkObj( *pFound );
    }

    return ( pFound != NULL );
}

//UNUSED2008-05  String ScDrawView::GetSelectedChartName() const
//UNUSED2008-05  {
//UNUSED2008-05      //  used for modifying a chart's data area - PersistName must always be used
//UNUSED2008-05      //  (as in ScDocument::FindChartData and UpdateChartArea)
//UNUSED2008-05
//UNUSED2008-05      const SdrMarkList& rMarkList = GetMarkedObjectList();
//UNUSED2008-05      if (rMarkList.GetMarkCount() == 1)
//UNUSED2008-05      {
//UNUSED2008-05          SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
//UNUSED2008-05          if (pObj->GetObjIdentifier() == OBJ_OLE2)
//UNUSED2008-05              if ( pDoc->IsChart(pObj) )
//UNUSED2008-05                  return static_cast<SdrOle2Obj*>(pObj)->GetPersistName();
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05      return EMPTY_STRING;        // nichts gefunden
//UNUSED2008-05  }

bool ScDrawView::InsertObjectSafe(SdrObject& rObj, sal_uLong nOptions)
{
    //  Markierung nicht aendern, wenn Ole-Objekt aktiv
    //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

    if (pViewData)
    {
        SfxInPlaceClient* pClient = pViewData->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;
    }

    return InsertObjectAtView( rObj, nOptions );
}

SdrObject* ScDrawView::GetMarkedNoteCaption( ScDrawObjData** ppCaptData )
{
    SdrObject* pSelected = getSelectedIfSingle();

    if( pViewData && pSelected )
    {
        if( ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( *pSelected, pViewData->GetTabNo() ) )
        {
            if( ppCaptData ) *ppCaptData = pCaptData;
            return pSelected;
        }
    }

    return 0;
}

void ScDrawView::LockCalcLayer( SdrLayerID nLayer, bool bLock )
{
    SdrLayer* pLockLayer = getSdrModelFromSdrView().GetModelLayerAdmin().GetLayerPerID( nLayer );
    if( pLockLayer && (IsLayerLocked( pLockLayer->GetName() ) != bLock) )
        SetLayerLocked( pLockLayer->GetName(), bLock );
}

void __EXPORT ScDrawView::MakeVisibleAtView( const basegfx::B2DRange& rRange, Window& rWin )
{
    //! rWin richtig auswerten
    //! ggf Zoom aendern

    if ( pViewData && pViewData->GetActiveWin() == &rWin )
    {
        pViewData->GetView()->MakeVisibleAtView( rRange );
    }
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
        bool bUndo = pDocShell && pUndoMgr && pDoc->IsUndoEnabled();

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

// support enhanced text edit for draw objects
SdrUndoManager* ScDrawView::getSdrUndoManagerForEnhancedTextEdit() const
{
    return pDoc ? dynamic_cast< SdrUndoManager* >(pDoc->GetUndoManager()) : 0;
}

// eof
