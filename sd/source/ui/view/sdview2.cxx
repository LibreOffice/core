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

#include "View.hxx"

#include <vector>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <vcl/graph.hxx>
#include <svx/xexch.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/childwin.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpagv.hxx>
#include <svl/urlbmk.hxx>
#include <editeng/outliner.hxx>
#include <svx/xflclit.hxx>
#include <sot/formats.hxx>
#include <editeng/editeng.hxx>
#include <svx/svditer.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>

#include <sfx2/objsh.hxx>
#include <svtools/embedtransfer.hxx>

#include "navigatr.hxx"
#include "anminfo.hxx"
#include "strings.hrc"
#include "glob.hrc"
#include "sdxfer.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "DrawViewShell.hxx"
#include "DrawDocShell.hxx"
#include "fudraw.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "unoaprms.hxx"
#include "drawview.hxx"
#include "helpids.h"
#include <vcl/svapp.hxx>

#include "slideshow.hxx"

namespace sd {

using namespace ::com::sun::star;

// ------------------------
// - SdNavigatorDropEvent -
// ------------------------

struct SdNavigatorDropEvent : public ExecuteDropEvent
{
    DropTargetHelper&       mrTargetHelper;
    ::sd::Window* mpTargetWindow;
    sal_uInt16                  mnPage;
    sal_uInt16                  mnLayer;

    SdNavigatorDropEvent (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        sal_uInt16 nLayer )
        : ExecuteDropEvent( rEvt ),
          mrTargetHelper( rTargetHelper ),
          mpTargetWindow( pTargetWindow ),
          mnPage( nPage ),
          mnLayer( nLayer )
    {}
};

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateClipboardDataObject( View*, ::Window& )
{
    // since SdTransferable::CopyToClipboard is called, this
    // dynamically created object ist destroyed automatically
    SdTransferable* pTransferable = new SdTransferable( &mrDoc, NULL, sal_False );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferClip = pTransferable;

    mrDoc.CreatingDataObj( pTransferable );
    pTransferable->SetWorkDocument( (SdDrawDocument*) GetMarkedObjModel() );
    mrDoc.CreatingDataObj( NULL );

    // #112978# need to use GetAllMarkedBoundRect instead of GetAllMarkedRect to get
    // fat lines correctly
    // This seems to be broken on LibreOffice. So reverted this temporarily
    // to fix fdo#45260
    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    TransferableObjectDescriptor    aObjDesc;
    SdrOle2Obj*                     pSdrOleObj = NULL;
    SdrPageView*                    pPgView = GetSdrPageView();
    SdPage*                         pOldPage = pPgView ? ( (SdPage*) pPgView->GetPage() ) : NULL;
    SdPage*                         pNewPage = (SdPage*) pTransferable->GetWorkDocument()->GetPage( 0 );

    if( pOldPage )
    {
        pNewPage->SetSize( pOldPage->GetSize() );
        pNewPage->SetLayoutName( pOldPage->GetLayoutName() );
    }

    if( GetMarkedObjectCount() == 1 )
    {
        SdrObject* pObj = GetMarkedObjectByIndex(0);

        if( pObj && pObj->ISA(SdrOle2Obj) && ((SdrOle2Obj*) pObj)->GetObjRef().is() )
        {
            // If object has no persistence it must be copied as part of the document
            try
            {
                uno::Reference< embed::XEmbedPersist > xPersObj( ((SdrOle2Obj*)pObj)->GetObjRef(), uno::UNO_QUERY );
                if ( xPersObj.is() && xPersObj->hasEntry() )
                     pSdrOleObj = (SdrOle2Obj*) pObj;
            }
            catch( uno::Exception& )
            {}
        }
    }

    if( pSdrOleObj )
        SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pSdrOleObj->GetObjRef(), pSdrOleObj->GetGraphic(), pSdrOleObj->GetAspect() );
    else
        pTransferable->GetWorkDocument()->GetDocSh()->FillTransferableObjectDescriptor( aObjDesc );

    if( mpDocSh )
        aObjDesc.maDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    aObjDesc.maSize = aMarkRect.GetSize();

    pTransferable->SetStartPos( aMarkRect.TopLeft() );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToClipboard( mpViewSh->GetActiveWindow() );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateDragDataObject( View* pWorkView, ::Window& rWindow, const Point& rDragPos )
{
    SdTransferable* pTransferable = new SdTransferable( &mrDoc, pWorkView, sal_False );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferDrag = pTransferable;

    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = NULL;

    if( GetMarkedObjectCount() == 1 )
    {
        SdrObject* pObj = GetMarkedObjectByIndex( 0 );

        if( pObj && pObj->ISA( SdrOle2Obj ) && ( (SdrOle2Obj*) pObj )->GetObjRef().is() )
        {
            // If object has no persistence it must be copied as part of the document
            try
            {
                uno::Reference< embed::XEmbedPersist > xPersObj( ((SdrOle2Obj*)pObj)->GetObjRef(), uno::UNO_QUERY );
                if ( xPersObj.is() && xPersObj->hasEntry() )
                     pSdrOleObj = (SdrOle2Obj*) pObj;
            }
            catch( uno::Exception& )
            {}
        }
    }

    if( mpDocSh )
        aDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    if( pSdrOleObj )
        SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pSdrOleObj->GetObjRef(), pSdrOleObj->GetGraphic(), pSdrOleObj->GetAspect() );
    else
        mpDocSh->FillTransferableObjectDescriptor( aObjDesc );

    aObjDesc.maSize = GetAllMarkedRect().GetSize();
    aObjDesc.maDragStartPos = rDragPos;
    aObjDesc.maDisplayName = aDisplayName;
    aObjDesc.mbCanLink = sal_False;

    pTransferable->SetStartPos( rDragPos );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->StartDrag( &rWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateSelectionDataObject( View* pWorkView, ::Window& rWindow )
{
    SdTransferable*                 pTransferable = new SdTransferable( &mrDoc, pWorkView, sal_True );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );
    TransferableObjectDescriptor    aObjDesc;
    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    String                          aDisplayName;

    SD_MOD()->pTransferSelection = pTransferable;

    if( mpDocSh )
    {
        aDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        mpDocSh->FillTransferableObjectDescriptor( aObjDesc );
    }

    aObjDesc.maSize = aMarkRect.GetSize();

    pTransferable->SetStartPos( aMarkRect.TopLeft() );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToSelection( &rWindow );

    return xRet;
}

// -----------------------------------------------------------------------------

void View::UpdateSelectionClipboard( sal_Bool bForceDeselect )
{
    if( mpViewSh && mpViewSh->GetActiveWindow() )
    {
        if( !bForceDeselect && GetMarkedObjectList().GetMarkCount() )
            CreateSelectionDataObject( this, *mpViewSh->GetActiveWindow() );
        else if( SD_MOD()->pTransferSelection && ( SD_MOD()->pTransferSelection->GetView() == this ) )
        {
            TransferableHelper::ClearSelection( mpViewSh->GetActiveWindow() );
            SD_MOD()->pTransferSelection = NULL;
        }
    }
}

// -----------------------------------------------------------------------------

void View::DoCut(::Window* )
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        ( (OutlinerView*) pOLV)->Cut();
    else if( AreObjectsMarked() )
    {
        String aStr( SdResId(STR_UNDO_CUT) );

        DoCopy();
        BegUndo( ( aStr += sal_Unicode(' ') ) += GetDescriptionOfMarkedObjects() );
        DeleteMarked();
        EndUndo();
    }
}

// -----------------------------------------------------------------------------

void View::DoCopy (::Window* pWindow)
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        ( (OutlinerView*) pOLV)->Copy();
    else if( AreObjectsMarked() )
    {
        BrkAction();
        CreateClipboardDataObject( this, *pWindow );
    }
}

// -----------------------------------------------------------------------------

void View::DoPaste (::Window* pWindow)
{
    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpViewSh->GetActiveWindow() ) );
    if( !aDataHelper.GetTransferable().is() )
        return; // empty clipboard?

    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV && EditEngine::HasValidData( aDataHelper.GetTransferable() ) )
    {
        const_cast< OutlinerView* >(pOLV)->PasteSpecial();

        SdrObject*  pObj = GetTextEditObject();
        SdPage*     pPage = (SdPage*)( pObj ? pObj->GetPage() : NULL );
        ::Outliner* pOutliner = pOLV->GetOutliner();

        if( pOutliner)
        {
            if( pObj && pPage && pPage->GetPresObjKind(pObj) == PRESOBJ_TITLE )
            {
                // remove all hard linebreaks from the title
                if( pOutliner && pOutliner->GetParagraphCount() > 1 )
                {
                    sal_Bool bOldUpdateMode = pOutliner->GetUpdateMode();

                    pOutliner->SetUpdateMode( sal_False );

                    const EditEngine& rEdit = pOutliner->GetEditEngine();
                    const sal_Int32 nParaCount = rEdit.GetParagraphCount();

                    for( sal_Int32 nPara = nParaCount - 2; nPara >= 0; nPara-- )
                    {
                        const sal_uInt16 nParaLen = (sal_uInt16)rEdit.GetTextLen( nPara );
                        pOutliner->QuickDelete( ESelection( nPara, nParaLen, nPara+1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( nPara, nParaLen, nPara, nParaLen ) );
                    }

                    DBG_ASSERT( rEdit.GetParagraphCount() <= 1, "Titelobjekt contains hard line breaks" );
                    pOutliner->SetUpdateMode(bOldUpdateMode);
                }
            }

            if( !mrDoc.IsChanged() )
            {
                if( pOutliner && pOutliner->IsModified() )
                    mrDoc.SetChanged( sal_True );
            }
        }
    }
    else
    {
        Point       aPos;
        sal_Int8    nDnDAction = DND_ACTION_COPY;

        if( pWindow )
            aPos = pWindow->PixelToLogic( Rectangle( aPos, pWindow->GetOutputSizePixel() ).Center() );

        DrawViewShell* pDrViewSh = (DrawViewShell*) mpDocSh->GetViewShell();

        if (pDrViewSh != NULL)
        {
            if( !InsertData( aDataHelper, aPos, nDnDAction, sal_False ) )
            {
                INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                {
                    pDrViewSh->InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, NULL );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void View::StartDrag( const Point& rStartPos, ::Window* pWindow )
{
    if( AreObjectsMarked() && IsAction() && mpViewSh && pWindow && !mpDragSrcMarkList )
    {
        BrkAction();

        if( IsTextEdit() )
            SdrEndTextEdit();

        DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpDocSh ? mpDocSh->GetViewShell() : 0 );

        if( pDrawViewShell )
        {
            FunctionReference xFunction( pDrawViewShell->GetCurrentFunction() );

            if( xFunction.is() && xFunction->ISA( FuDraw ) )
                static_cast<FuDraw*>(xFunction.get())->ForcePointer( NULL );
        }

        mpDragSrcMarkList = new SdrMarkList(GetMarkedObjectList());
        mnDragSrcPgNum = GetSdrPageView()->GetPage()->GetPageNum();

        if( IsUndoEnabled() )
        {
            String aStr( SdResId(STR_UNDO_DRAGDROP) );
            aStr += sal_Unicode(' ');
            aStr += mpDragSrcMarkList->GetMarkDescription();
            BegUndo(aStr);
        }
        CreateDragDataObject( this, *pWindow, rStartPos );
    }
}

// -----------------------------------------------------------------------------

void View::DragFinished( sal_Int8 nDropAction )
{
    const bool bUndo = IsUndoEnabled();

    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if( pDragTransferable )
        pDragTransferable->SetView( NULL );

    if( ( nDropAction & DND_ACTION_MOVE ) &&
        pDragTransferable && !pDragTransferable->IsInternalMove() &&
        mpDragSrcMarkList && mpDragSrcMarkList->GetMarkCount() &&
        !IsPresObjSelected() )
    {
        mpDragSrcMarkList->ForceSort();

        if( bUndo )
            BegUndo();

        sal_uLong nm, nAnz = mpDragSrcMarkList->GetMarkCount();

         for( nm = nAnz; nm>0; )
        {
            nm--;
            SdrMark* pM=mpDragSrcMarkList->GetMark(nm);
            if( bUndo )
                AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeleteObject(*pM->GetMarkedSdrObj()));
        }

        mpDragSrcMarkList->GetMark(0)->GetMarkedSdrObj()->GetOrdNum();

        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=mpDragSrcMarkList->GetMark(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();

            if( pObj && pObj->GetPage() )
            {
                sal_uInt32 nOrdNum=pObj->GetOrdNumDirect();
#ifdef DBG_UTIL
                SdrObject* pChkObj =
#endif
                    pObj->GetPage()->RemoveObject(nOrdNum);
                DBG_ASSERT(pChkObj==pObj,"pChkObj!=pObj in RemoveObject()");
            }
        }

        if( bUndo )
            EndUndo();
    }

    if( pDragTransferable )
        pDragTransferable->SetInternalMove( sal_False );

    if( bUndo )
        EndUndo();
    mnDragSrcPgNum = SDRPAGE_NOTFOUND;
    delete mpDragSrcMarkList;
    mpDragSrcMarkList = NULL;
}

// -----------------------------------------------------------------------------

sal_Int8 View::AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                             ::sd::Window*, sal_uInt16, sal_uInt16 nLayer )
{
    String          aLayerName( GetActiveLayer() );
    SdrPageView*    pPV = GetSdrPageView();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    if( nLayer != SDRLAYER_NOTFOUND )
    {
        SdrLayerAdmin& rLayerAdmin = mrDoc.GetLayerAdmin();
        aLayerName = rLayerAdmin.GetLayerPerID(nLayer)->GetName();
    }

    if( mbIsDropAllowed && !pPV->IsLayerLocked( aLayerName ) && pPV->IsLayerVisible( aLayerName ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        sal_Bool                bIsInsideOutlinerView = sal_False;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if (GetMarkedObjectCount() == 1)
            {
                SdrMark* pMark = GetSdrMarkByIndex(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            if( aRect.IsInside( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) ) )
            {
                bIsInsideOutlinerView = sal_True;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

            if(pDragTransferable && (nDropAction & DND_ACTION_LINK))
            {
                // suppress own data when it's intention is to use it as fill information
                pDragTransferable = 0;
            }

            if( pDragTransferable )
            {
                const View* pSourceView = pDragTransferable->GetView();

                if( pDragTransferable->IsPageTransferable() )
                {
                    nRet = DND_ACTION_COPY;
                }
                else if( pSourceView )
                {
                    if( !( nDropAction & DND_ACTION_LINK ) ||
                        !pSourceView->GetDocSh()->GetMedium()->GetName().isEmpty() )
                    {
                        nRet = nDropAction;
                    }
                }
            }
            else
            {
                const sal_Bool  bDrawing = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING );
                const sal_Bool  bGraphic = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB );
                const sal_Bool  bMtf = rTargetHelper.IsDropFormatSupported( FORMAT_GDIMETAFILE );
                const sal_Bool  bBitmap = rTargetHelper.IsDropFormatSupported( FORMAT_BITMAP );
                sal_Bool        bBookmark = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                sal_Bool        bXFillExchange = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_XFA );

                // check handle insert
                if( !nRet && ( (bXFillExchange && ( SDRDRAG_GRADIENT == GetDragMode() )) || ( SDRDRAG_TRANSPARENCE == GetDragMode() ) ) )
                {
                    const SdrHdlList& rHdlList = GetHdlList();

                    for( sal_uInt32 n = 0; n < rHdlList.GetHdlCount(); n++ )
                    {
                        SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                        if( pIAOHandle && ( HDL_COLR == pIAOHandle->GetKind() ) )
                        {
                            if(pIAOHandle->getOverlayObjectList().isHitPixel(rEvt.maPosPixel))
                            {
                                nRet = nDropAction;
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetSize( SDR_HANDLE_COLOR_SIZE_SELECTED );
                            }
                            else
                            {
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetSize( SDR_HANDLE_COLOR_SIZE_NORMAL );
                            }
                        }
                    }
                }

                // check object insert
                if( !nRet && ( bXFillExchange || ( ( bDrawing || bGraphic || bMtf || bBitmap || bBookmark ) && ( nDropAction & DND_ACTION_LINK ) ) ) )
                {
                    SdrObject*      pPickObj = NULL;
                    SdrPageView*    pPageView = NULL;
                    ::sd::Window* pWindow = mpViewSh->GetActiveWindow();
                    Point           aPos( pWindow->PixelToLogic( rEvt.maPosPixel ) );
                    const sal_Bool      bHasPickObj = PickObj( aPos, getHitTolLog(), pPickObj, pPageView );
                    sal_Bool            bIsPresTarget = sal_False;

                    if( bHasPickObj && pPickObj && ( pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall() ) )
                    {
                        SdPage* pPage = (SdPage*) pPickObj->GetPage();

                        if( pPage && pPage->IsMasterPage() )
                            bIsPresTarget = pPage->IsPresObj( pPickObj );
                    }

                    if( bHasPickObj && !bIsPresTarget &&
                        ( bGraphic || bMtf || bBitmap || bXFillExchange ) )
                    {
                        if( mpDropMarkerObj != pPickObj )
                        {
                            mpDropMarkerObj = pPickObj;
                            ImplClearDrawDropMarker();

                            if(mpDropMarkerObj)
                            {
                                mpDropMarker = new SdrDropMarkerOverlay(*this, *mpDropMarkerObj);
                            }
                        }

                        nRet = nDropAction;
                    }
                    else
                        bXFillExchange = sal_False;
                }

                // check normal insert
                if( !nRet )
                {
                    const sal_Bool  bSBAFormat = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH );
                    const sal_Bool  bEditEngine = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_EDITENGINE );
                    const sal_Bool  bString = rTargetHelper.IsDropFormatSupported( FORMAT_STRING );
                    const sal_Bool  bRTF = rTargetHelper.IsDropFormatSupported( FORMAT_RTF );
                    const sal_Bool  bFile = rTargetHelper.IsDropFormatSupported( FORMAT_FILE );
                    const sal_Bool  bFileList = rTargetHelper.IsDropFormatSupported( FORMAT_FILE_LIST );

                    if( mpDropMarker )
                    {
                        ImplClearDrawDropMarker();
                        mpDropMarkerObj = NULL;
                    }

                    if( bBookmark && bFile && ( nDropAction & DND_ACTION_MOVE ) && mpViewSh && SlideShow::IsRunning(mpViewSh->GetViewShellBase()) )
                        bBookmark = sal_False;

                    if( bDrawing || bGraphic || bMtf || bBitmap || bBookmark || bFile || bFileList || bXFillExchange || bSBAFormat || bEditEngine || bString || bRTF )
                        nRet = nDropAction;

                    // For entries from the navigator, change action copy.
                    if (bBookmark
                        && rTargetHelper.IsDropFormatSupported(
                            SdPageObjsTLB::SdPageObjsTransferable::GetListBoxDropFormatId())
                        && (nDropAction & DND_ACTION_MOVE)!=0)
                    {
                        nRet = DND_ACTION_COPY;
                    }
                }
            }
        }
    }

    // destroy drop marker if this is a leaving event
    if( rEvt.mbLeaving && mpDropMarker )
    {
        ImplClearDrawDropMarker();
        mpDropMarkerObj = NULL;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int8 View::ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                              ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer )
{
    SdrPageView*    pPV = GetSdrPageView();
    String          aActiveLayer = GetActiveLayer();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    // destroy drop marker if it is shown
    if( mpDropMarker )
    {
        ImplClearDrawDropMarker();
        mpDropMarkerObj = NULL;
    }

    if( !pPV->IsLayerLocked( aActiveLayer ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        sal_Bool                bIsInsideOutlinerView = sal_False;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if( GetMarkedObjectCount() == 1 )
            {
                SdrMark* pMark = GetSdrMarkByIndex(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            Point aPos( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) );

            if( aRect.IsInside( aPos ) )
            {
                bIsInsideOutlinerView = sal_True;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            Point                   aPos;
            TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );

            if( pTargetWindow )
                aPos = pTargetWindow->PixelToLogic( rEvt.maPosPixel );

            // handle insert?
            if( (!nRet && ( SDRDRAG_GRADIENT == GetDragMode() )) || (( SDRDRAG_TRANSPARENCE == GetDragMode() ) && aDataHelper.HasFormat( SOT_FORMATSTR_ID_XFA )) )
            {
                const SdrHdlList& rHdlList = GetHdlList();

                for( sal_uInt32 n = 0; !nRet && n < rHdlList.GetHdlCount(); n++ )
                {
                    SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                    if( pIAOHandle && ( HDL_COLR == pIAOHandle->GetKind() ) )
                    {
                        if(pIAOHandle->getOverlayObjectList().isHitPixel(rEvt.maPosPixel))
                        {
                            SotStorageStreamRef xStm;

                            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) && xStm.Is() )
                            {
                                XFillExchangeData aFillData( XFillAttrSetItem( &mrDoc.GetPool() ) );

                                *xStm >> aFillData;
                                const Color aColor( ( (XFillColorItem&) aFillData.GetXFillAttrSetItem()->GetItemSet().Get( XATTR_FILLCOLOR ) ).GetColorValue() );
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetColor( aColor, sal_True );
                                nRet = nDropAction;
                            }
                        }
                    }
                }
            }

            // standard insert?
            if( !nRet && InsertData( aDataHelper, aPos, nDropAction, sal_True, 0, nPage, nLayer ) )
                nRet = nDropAction;

            // special insert?
            if( !nRet && mpViewSh )
            {
                String          aTmpString1, aTmpString2;
                INetBookmark    aINetBookmark( aTmpString1, aTmpString2 );

                // insert bookmark
                if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                    aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) )
                {
                    SdPageObjsTLB::SdPageObjsTransferable* pPageObjsTransferable = SdPageObjsTLB::SdPageObjsTransferable::getImplementation( aDataHelper.GetXTransferable() );

                    if( pPageObjsTransferable &&
                        ( NAVIGATOR_DRAGTYPE_LINK == pPageObjsTransferable->GetDragType() ||
                          NAVIGATOR_DRAGTYPE_EMBEDDED == pPageObjsTransferable->GetDragType() ) )
                    {
                        // insert bookmark from own navigator (handled async. due to possible message box )
                        Application::PostUserEvent( LINK( this, View, ExecuteNavigatorDrop ),
                                                    new SdNavigatorDropEvent( rEvt, rTargetHelper, pTargetWindow,
                                                                              nPage, nLayer ) );
                        nRet = nDropAction;
                    }
                    else
                    {
                        SdrObject*      pPickObj = NULL;
                        SdrPageView*    pPageView = NULL;

                        if( PickObj( aPos, getHitTolLog(), pPickObj, pPageView ) )
                        {
                            // insert as clip action => jump
                            OUString       aBookmark( aINetBookmark.GetURL() );
                            SdAnimationInfo*    pInfo = mrDoc.GetAnimationInfo( pPickObj );
                            sal_Bool                bCreated = sal_False;

                            if( !aBookmark.isEmpty() )
                            {
                                presentation::ClickAction eClickAction = presentation::ClickAction_DOCUMENT;

                                sal_Int32 nIndex = aBookmark.indexOf( (sal_Unicode)'#' );
                                if( nIndex != -1 )
                                {
                                    const OUString aDocName( aBookmark.copy( 0, nIndex ) );

                                    if (mpDocSh->GetMedium()->GetName() == aDocName || aDocName.equals(mpDocSh->GetName()))
                                    {
                                        // internal jump, only use the part after and including '#'
                                        eClickAction = presentation::ClickAction_BOOKMARK;
                                        aBookmark = aBookmark.copy( nIndex+1 );
                                    }
                                }

                                if( !pInfo )
                                {
                                    pInfo = SdDrawDocument::GetShapeUserData( *pPickObj, true );
                                    bCreated = sal_True;
                                }

                                // create undo action with old and new sizes
                                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction(&mrDoc, pPickObj, bCreated);
                                pAction->SetActive(pInfo->mbActive, pInfo->mbActive);
                                pAction->SetEffect(pInfo->meEffect, pInfo->meEffect);
                                pAction->SetTextEffect(pInfo->meTextEffect, pInfo->meTextEffect);
                                pAction->SetSpeed(pInfo->meSpeed, pInfo->meSpeed);
                                pAction->SetDim(pInfo->mbDimPrevious, pInfo->mbDimPrevious);
                                pAction->SetDimColor(pInfo->maDimColor, pInfo->maDimColor);
                                pAction->SetDimHide(pInfo->mbDimHide, pInfo->mbDimHide);
                                pAction->SetSoundOn(pInfo->mbSoundOn, pInfo->mbSoundOn);
                                pAction->SetSound(pInfo->maSoundFile, pInfo->maSoundFile);
                                pAction->SetPlayFull(pInfo->mbPlayFull, pInfo->mbPlayFull);
                                pAction->SetPathObj(pInfo->mpPathObj, pInfo->mpPathObj);
                                pAction->SetClickAction(pInfo->meClickAction, eClickAction);
                                pAction->SetBookmark(pInfo->GetBookmark(), aBookmark);
                                pAction->SetVerb(pInfo->mnVerb, pInfo->mnVerb);
                                pAction->SetSecondEffect(pInfo->meSecondEffect, pInfo->meSecondEffect);
                                pAction->SetSecondSpeed(pInfo->meSecondSpeed, pInfo->meSecondSpeed);
                                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, pInfo->mbSecondSoundOn);
                                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull, pInfo->mbSecondPlayFull);

                                OUString aString(SdResId(STR_UNDO_ANIMATION));
                                pAction->SetComment(aString);
                                mpDocSh->GetUndoManager()->AddUndoAction(pAction);
                                pInfo->meClickAction = eClickAction;
                                pInfo->SetBookmark( aBookmark );
                                mrDoc.SetChanged();

                                nRet = nDropAction;
                            }
                        }
                        else if( mpViewSh->ISA( DrawViewShell ) )
                        {
                            // insert as normal URL button
                            ( (DrawViewShell*) mpViewSh )->InsertURLButton( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), String(), &aPos );
                            nRet = nDropAction;
                        }
                    }
                }
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( View, ExecuteNavigatorDrop, SdNavigatorDropEvent*, pSdNavigatorDropEvent )
{
    TransferableDataHelper                  aDataHelper( pSdNavigatorDropEvent->maDropEvent.Transferable );
    SdPageObjsTLB::SdPageObjsTransferable*  pPageObjsTransferable = SdPageObjsTLB::SdPageObjsTransferable::getImplementation( aDataHelper.GetXTransferable() );
    INetBookmark                            aINetBookmark;

    if( pPageObjsTransferable && aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) )
    {
        Point   aPos;
        OUString  aBookmark;
        SdPage* pPage = (SdPage*) GetSdrPageView()->GetPage();
        sal_uInt16  nPgPos = 0xFFFF;

        if( pSdNavigatorDropEvent->mpTargetWindow )
            aPos = pSdNavigatorDropEvent->mpTargetWindow->PixelToLogic( pSdNavigatorDropEvent->maPosPixel );

        const OUString aURL( aINetBookmark.GetURL() );
        sal_Int32 nIndex = aURL.indexOf( (sal_Unicode)'#' );
        if( nIndex != -1 )
            aBookmark = aURL.copy( nIndex+1 );

        std::vector<OUString> aExchangeList;
        std::vector<OUString> aBookmarkList(1,aBookmark);

        if( !pPage->IsMasterPage() )
        {
            if( pPage->GetPageKind() == PK_STANDARD )
                nPgPos = pPage->GetPageNum() + 2;
            else if( pPage->GetPageKind() == PK_NOTES )
                nPgPos = pPage->GetPageNum() + 1;
        }

        /* In order t ensure unique page names, we test the ones we want to
           insert. If necessary. we put them into and replacement list (bNameOK
           == sal_False -> User canceled).  */
        sal_Bool    bLink = ( NAVIGATOR_DRAGTYPE_LINK == pPageObjsTransferable->GetDragType()  ? sal_True : sal_False );
        sal_Bool    bNameOK = GetExchangeList( aExchangeList, aBookmarkList, 2 );
        sal_Bool    bReplace = sal_False;

        /* Since we don't know the type (page or object), we fill a list with
           pages and objects.
           Of course we have problems if there are pages and objects with the
           same name!!! */
        if( bNameOK )
        {
            mrDoc.InsertBookmark( aBookmarkList, aExchangeList,
                                  bLink, bReplace, nPgPos, sal_False,
                                  &pPageObjsTransferable->GetDocShell(),
                                  sal_True, &aPos );
        }
    }

    delete pSdNavigatorDropEvent;

    return 0;
}

bool View::GetExchangeList (std::vector<OUString> &rExchangeList,
                            std::vector<OUString> &rBookmarkList,
                            const sal_uInt16 nType)
{
    assert(rExchangeList.empty());

    bool bListIdentical = true; ///< Bookmark list and exchange list are identical
    bool bNameOK = true;        ///< name is unique

    std::vector<OUString>::const_iterator pIter;
    for ( pIter = rBookmarkList.begin(); bNameOK && pIter != rBookmarkList.end(); ++pIter )
    {
        String aNewName = *pIter;

        if( nType == 0  || nType == 2 )
            bNameOK = mpDocSh->CheckPageName(mpViewSh->GetActiveWindow(), aNewName);

        if( bNameOK && ( nType == 1  || nType == 2 ) )
        {
            if( mrDoc.GetObj( aNewName ) )
            {
                OUString aTitle(SD_RESSTR(STR_TITLE_NAMEGROUP));
                OUString aDesc(SD_RESSTR(STR_DESC_NAMEGROUP));

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                AbstractSvxNameDialog* pDlg = 0;

                if (pFact)
                    pDlg = pFact->CreateSvxNameDialog( mpViewSh->GetActiveWindow(), aNewName, aDesc );

                if( pDlg )
                {
                    pDlg->SetEditHelpId( HID_SD_NAMEDIALOG_OBJECT );

                    bNameOK = false;
                    pDlg->SetText( aTitle );

                    while( !bNameOK && pDlg->Execute() == RET_OK )
                    {
                        pDlg->GetName( aNewName );

                        if( !mrDoc.GetObj( aNewName ) )
                            bNameOK = true;
                    }

                    delete pDlg;
                }
            }
        }

        bListIdentical = pIter->equals(aNewName);

        rExchangeList.push_back(aNewName);
    }

    // Exchange list is identical to bookmark list
    if( !rExchangeList.empty() && bListIdentical )
        rExchangeList.clear();

    return bNameOK;
}

typedef std::vector< SdrObject* > SdrObjectVector;

void ImplProcessObjectList(SdrObject* pObj, SdrObjectVector& rVector )
{
    sal_Bool bIsGroup(pObj->IsGroupObject());
    if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
        bIsGroup = sal_False;

    rVector.push_back( pObj );

    if(bIsGroup)
    {
        SdrObjList* pObjList = pObj->GetSubList();
        sal_uInt32 a;
        for( a = 0; a < pObjList->GetObjCount(); a++)
            ImplProcessObjectList(pObjList->GetObj(a), rVector);
    }
}

SdrModel* View::GetMarkedObjModel() const
{
    return FmFormView::GetMarkedObjModel();;
}

sal_Bool View::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst /* =NULL */, sal_uInt32 nOptions /* =0 */)
{
    return FmFormView::Paste( rMod, rPos, pLst,nOptions );;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
