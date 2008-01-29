/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawview.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:48:58 $
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
#include <svx/outlobj.hxx>
#include <svx/xoutx.hxx>
#include <svx/writingmodeitem.hxx>
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
    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    SetFrameDragSingles(TRUE);
//  SetSolidMarkHdl(TRUE);              // einstellbar -> UpdateUserViewOptions

    SetMinMoveDistancePixel( 2 );
    SetHitTolerancePixel( 2 );

    if (pViewData)
    {
        SCTAB nViewTab = pViewData->GetTabNo();
        ShowSdrPage(GetModel()->GetPage(nViewTab));

        BOOL bEx = pViewData->GetViewShell()->IsDrawSelMode();
        BOOL bProt = pDoc->IsTabProtected( nViewTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), TRUE );
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

        SetSwapAsynchron(TRUE);
    }
    else
    {
        ShowSdrPage(GetModel()->GetPage(nTab));
    }

    UpdateUserViewOptions();
    RecalcScale();
    UpdateWorkArea();

    bInConstruct = FALSE;
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

void ScDrawView::AddCustomHdl()
{
    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

    const SdrMarkList &rMrkList = GetMarkedObjectList();
    UINT32 nCount = rMrkList.GetMarkCount();
    for(UINT32 nPos=0; nPos<nCount; nPos++ )
    {
        const SdrObject* pObj = rMrkList.GetMark(nPos)->GetMarkedSdrObj();
        if(ScDrawLayer::GetAnchor(pObj) == SCA_CELL)
        {
            const INT32 nDelta = 1;

            Rectangle aBoundRect = pObj->GetCurrentBoundRect();
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
            INT32 nWidth = 0;

            for(nCol=0; nCol<=MAXCOL && nWidth<=nPosX; nCol++)
                nWidth += pDoc->GetColWidth(nCol,nTab);

            if(nCol > 0)
                --nCol;

            SCROW nRow = nPosY <= 0 ? 0 : pDoc->FastGetRowForHeight( nTab,
                    (ULONG) nPosY);
            if(nRow > 0)
                --nRow;

            ScTabView* pView = pViewData->GetView();
            ScAddress aScAddress(nCol, nRow, nTab);
            pView->CreateAnchorHandles(aHdl, aScAddress);
        }
    }
}

void ScDrawView::InvalidateAttribs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

        // echte Statuswerte:
    rBindings.InvalidateAll( TRUE );
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

void ScDrawView::SetMarkedToLayer( BYTE nLayerNo )
{
    if (AreObjectsMarked())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( ScGlobal::GetRscString( STR_UNDO_SELATTR ) );

        const SdrMarkList& rMark = GetMarkedObjectList();
        ULONG nCount = rMark.GetMarkCount();
        for (ULONG i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
            if ( !pObj->ISA(SdrUnoObj) )
            {
                AddUndo( new SdrUndoObjectLayerChange( *pObj, pObj->GetLayer(), (SdrLayerID)nLayerNo) );
                pObj->SetLayer( nLayerNo );
            }
        }

        EndUndo();

        //  repaint is done in SetLayer

        pViewData->GetDocShell()->SetDrawModified();

        //  #84073# check mark list now instead of later in a timer
        CheckMarked();
        MarkListHasChanged();
    }
}

BOOL ScDrawView::HasMarkedControl() const
{
    if (AreObjectsMarked())
    {
        const SdrMarkList& rMark = GetMarkedObjectList();
        ULONG nCount = rMark.GetMarkCount();
        for (ULONG i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
            if ( pObj->ISA(SdrUnoObj) )
                return TRUE;
            else if ( pObj->ISA(SdrObjGroup) )
            {
                SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );
                SdrObject* pSubObj = aIter.Next();
                while (pSubObj)
                {
                    if ( pSubObj->ISA(SdrUnoObj) )
                        return TRUE;
                    pSubObj = aIter.Next();
                }
            }

        }
    }
    return FALSE;       // war nix
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
        nEndRow = 20;

    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev,aZoomX,aZoomY,nPPTX,nPPTY,
                            aScaleX,aScaleY );
}

// #110094#-17 Not used
//void ScDrawView::PaintObject( SdrObject* pObject, OutputDevice* pDev ) const
//{
//  pXOut->SetOutDev( pDev );
//  SdrPaintInfoRec aInfoRec;
//  pObject->Paint( *pXOut, aInfoRec );
//}

void ScDrawView::DoConnect(SdrOle2Obj* pOleObj)
{
    // needed for plug-ins etc.
    // query for status embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE is in SdrOle2Obj::DoPaintObject

    if ( pViewData )
        pViewData->GetViewShell()->ConnectObject( pOleObj );
}

void __EXPORT ScDrawView::MarkListHasChanged()
{
    FmFormView::MarkListHasChanged();

    UpdateBrowser();

    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    if (!bInConstruct)          // nicht wenn die View gerade angelegt wird
    {
        pViewSh->Unmark();      // Selektion auff'm Doc entfernen

        //  #65379# end cell edit mode if drawing objects are selected
        if ( GetMarkedObjectList().GetMarkCount() )
            SC_MOD()->InputEnterHandler();
    }

    //  IP deaktivieren

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = (ScClient*) pViewSh->GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
    {
        //  #41730# beim ViewShell::Activate aus dem Reset2Open nicht die Handles anzeigen
        //HMHbDisableHdl = TRUE;
        pClient->DeactivateObject();
        //HMHbDisableHdl = FALSE;
        //  Image-Ole wieder durch Grafik ersetzen passiert jetzt in ScClient::UIActivate
    }

    //  Ole-Objekt selektiert?

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    SdrMediaObj* pMediaObj = NULL;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 0 && !pViewData->GetViewShell()->IsDrawSelMode() && !bInConstruct )
    {
        //  re-lock background layer if it was unlocked in SelectObject
        SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
        if ( pLayer && !IsLayerLocked( pLayer->GetName() ) )
            SetLayerLocked( pLayer->GetName(), TRUE );
        // re-lock this internal note object.
        pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
        if ( pLayer && !IsLayerLocked( pLayer->GetName() ) )
            SetLayerLocked( pLayer->GetName(), TRUE );
    }

    BOOL bSubShellSet = FALSE;
    if (nMarkCount == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
        {
            pOle2Obj = (SdrOle2Obj*) pObj;
            if (!pDoc->IsChart(pObj) )
                pViewSh->SetOleObjectShell(TRUE);
            else
                pViewSh->SetChartShell(TRUE);
            bSubShellSet = TRUE;
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
            pViewSh->SetGraphicShell(TRUE);
            bSubShellSet = TRUE;
        }
        else if (pObj->GetObjIdentifier() == OBJ_MEDIA)
        {
            pMediaObj = (SdrMediaObj*) pObj;
            pViewSh->SetMediaShell(TRUE);
            bSubShellSet = TRUE;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // Verhindern, das beim Anlegen
                    || !pViewSh->IsDrawTextShell())     // eines TextObjekts auf die
        {                                               // DrawShell umgeschaltet wird.
            pViewSh->SetDrawShell(TRUE);                //@#70206#
        }
    }

    if ( nMarkCount && !bSubShellSet )
    {
        BOOL bOnlyControls = TRUE;
        BOOL bOnlyGraf     = TRUE;
        for (ULONG i=0; i<nMarkCount; i++)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            if ( pObj->ISA( SdrObjGroup ) )
            {
                const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
                ULONG nListCount = pLst->GetObjCount();
                if ( nListCount == 0 )
                {
                    //  #104156# An empty group (may occur during Undo) is no control or graphics object.
                    //  Creating the form shell during undo would lead to problems with the undo manager.
                    bOnlyControls = FALSE;
                    bOnlyGraf = FALSE;
                }
                for ( USHORT j = 0; j < nListCount; ++j )
                {
                    SdrObject *pSubObj = pLst->GetObj( j );

                    if (!pSubObj->ISA(SdrUnoObj))
                        bOnlyControls = FALSE;
                    if (pSubObj->GetObjIdentifier() != OBJ_GRAF)
                        bOnlyGraf = FALSE;

                    if ( !bOnlyControls && !bOnlyGraf ) break;
                }
            }
            else
            {
                if (!pObj->ISA(SdrUnoObj))
                    bOnlyControls = FALSE;
                if (pObj->GetObjIdentifier() != OBJ_GRAF)
                    bOnlyGraf = FALSE;
            }

            if ( !bOnlyControls && !bOnlyGraf ) break;
        }

        if(bOnlyControls)
        {
            pViewSh->SetDrawFormShell(TRUE);            // jetzt UNO-Controls
        }
        else if(bOnlyGraf)
        {
            pViewSh->SetGraphicShell(TRUE);
        }
        else if(nMarkCount>1)
        {
            pViewSh->SetDrawShell(TRUE);
        }
    }



    //  Verben anpassen

    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();
    BOOL bOle = pViewSh->GetViewFrame()->GetFrame()->IsInPlace();
    if ( pOle2Obj && !bOle )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pOle2Obj->GetObjRef();
        if (xObj.is())
            pViewSh->SetVerbs( xObj->getSupportedVerbs() );
        else
        {
            DBG_ERROR("SdrOle2Obj ohne ObjRef");
            pViewSh->SetVerbs( 0 );
        }
    }
    else
        pViewSh->SetVerbs( 0 );

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
        SfxFrame* pFrame = pViewFrame->GetFrame();
        if (pFrame)
        {
            uno::Reference<frame::XController> xController = pFrame->GetController();
            if (xController.is())
            {
                ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
                if (pImp)
                    pImp->SelectionChanged();
            }
        }
    }

    //  update selection transfer object

    pViewSh->CheckSelectionTransfer();

}

void __EXPORT ScDrawView::ModelHasChanged()
{
    SdrObject* pEditObj = GetTextEditObject();
    if ( pEditObj && !pEditObj->IsInserted() && pViewData )
    {
        //  #111700# SdrObjEditView::ModelHasChanged will end text edit in this case,
        //  so make sure the EditEngine's undo manager is no longer used.
        pViewData->GetViewShell()->SetDrawTextUndo(NULL);
        SetCreateMode();    // don't leave FuText in a funny state
    }

    FmFormView::ModelHasChanged();
}

void __EXPORT ScDrawView::UpdateUserViewOptions()
{
    if (pViewData)
    {
        const ScViewOptions&    rOpt = pViewData->GetOptions();
        const ScGridOptions&    rGrid = rOpt.GetGridOptions();

        BOOL bBigHdl = rOpt.GetOption( VOPT_BIGHANDLES );

        SetDragStripes( rOpt.GetOption( VOPT_HELPLINES ) );
        SetSolidMarkHdl( rOpt.GetOption( VOPT_SOLIDHANDLES ) );
        SetMarkHdlSizePixel( bBigHdl ? SC_HANDLESIZE_BIG : SC_HANDLESIZE_SMALL );

        SetGridVisible( rGrid.GetGridVisible() );
        SetSnapEnabled( rGrid.GetUseGridSnap() );
        SetGridSnap( rGrid.GetUseGridSnap() );

        //  Snap from grid options is no longer used
//      SetSnapGrid( Size( rGrid.GetFldSnapX(), rGrid.GetFldSnapY() ) );

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

BOOL ScDrawView::SelectObject( const String& rName )
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

        //  #61585# to select an object on the background layer, the layer has to
        //  be unlocked even if exclusive drawing selection mode is not active
        //  (this is reversed in MarkListHasChanged when nothing is selected)

        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !pDoc->IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
            if (pLayer)
                SetLayerLocked( pLayer->GetName(), FALSE );
        }

        SdrPageView* pPV = GetSdrPageView();
        MarkObj( pFound, pPV );
    }

    return ( pFound != NULL );
}

String ScDrawView::GetSelectedChartName() const
{
    //  used for modifying a chart's data area - PersistName must always be used
    //  (as in ScDocument::FindChartData and UpdateChartArea)

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            if ( pDoc->IsChart(pObj) )
                return static_cast<SdrOle2Obj*>(pObj)->GetPersistName();
    }

    return EMPTY_STRING;        // nichts gefunden
}

FASTBOOL ScDrawView::InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, ULONG nOptions)
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

void __EXPORT ScDrawView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    //! rWin richtig auswerten
    //! ggf Zoom aendern

    if ( pViewData && pViewData->GetActiveWin() == &rWin )
        pViewData->GetView()->MakeVisible( rRect );
}

SdrEndTextEditKind ScDrawView::ScEndTextEdit()
{
    BOOL bIsTextEdit = IsTextEdit();
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

void ScDrawView::StoreCaptionAttribs()
{
    SdrObject* pObj = NULL;
    const SdrMarkList&  rMarkList   = GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if ( pObj && pObj->GetLayer() == SC_LAYER_INTERN && pObj->ISA(SdrCaptionObj) )
    {
        ScAddress aTabPos;
        ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
        if( pData )
            aTabPos = pData->aStt;
        ScPostIt aNote(pDoc);
        if(pDoc->GetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote ))
        {
            aNote.SetItemSet(pObj->GetMergedItemSet());
            pDoc->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );
        }
    }
}

void ScDrawView::StoreCaptionDimensions()
{
    SdrObject* pObj = NULL;
    const SdrMarkList&  rMarkList   = GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if ( pObj && pObj->GetLayer() == SC_LAYER_INTERN && pObj->ISA(SdrCaptionObj) )
    {
        ScAddress aTabPos;
        ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
        if( pData )
            aTabPos = pData->aStt;
        ScPostIt aNote(pDoc);
        if(pDoc->GetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote ))
        {
            Rectangle aOldRect = aNote.GetRectangle();
            Rectangle aNewRect = pObj->GetLogicRect();
            if(aOldRect != aNewRect)
            {
                aNote.SetRectangle(aNewRect);
                // The new height/width is honoured if property item is reset.
                SdrCaptionObj* pCaption = static_cast<SdrCaptionObj*>(pObj);
                OutlinerParaObject* pPObj = pCaption->GetOutlinerParaObject();
                bool bVertical = (pPObj && pPObj->IsVertical());
                if(!bVertical && aNewRect.Bottom() - aNewRect.Top() > aOldRect.Bottom() - aOldRect.Top())
                {
                    if(pCaption && pCaption->IsAutoGrowHeight())
                    {
                        pCaption->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
                        aNote.SetItemSet(pCaption->GetMergedItemSet());
                    }
                }
                else if(bVertical && aNewRect.Right() - aNewRect.Left() > aOldRect.Right() - aOldRect.Left())
                {
                    if(pCaption && pCaption->IsAutoGrowWidth())
                    {
                        pCaption->SetMergedItem( SdrTextAutoGrowWidthItem( false ) );
                        aNote.SetItemSet(pCaption->GetMergedItemSet());
                    }
                }
                pDoc->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );
            }
        }
    }
}

void ScDrawView::CaptionTextDirection( USHORT nSlot )
{
    if(nSlot != SID_TEXTDIRECTION_LEFT_TO_RIGHT && nSlot != SID_TEXTDIRECTION_TOP_TO_BOTTOM)
        return;

    SdrObject* pObject  = GetTextEditObject();

    if ( pObject && pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj) )
    {
        SdrCaptionObj* pCaption = static_cast<SdrCaptionObj*>(pObject);
        if(pCaption)
        {
            SfxItemSet aAttr(pCaption->GetMergedItemSet());
            aAttr.Put( SvxWritingModeItem(
                nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                    com::sun::star::text::WritingMode_LR_TB : com::sun::star::text::WritingMode_TB_RL,
                    SDRATTR_TEXTDIRECTION ) );
            pCaption->SetMergedItemSet(aAttr);
            FuPoor* pPoor = pViewData->GetView()->GetDrawFuncPtr();
            if ( pPoor )
            {
                FuText* pText = static_cast<FuText*>(pPoor);
                pText->StopEditMode(TRUE);
            }
        }
    }
}
