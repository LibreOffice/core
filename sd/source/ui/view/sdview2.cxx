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
#include <memory>

namespace sd {

using namespace ::com::sun::star;


struct SdNavigatorDropEvent : public ExecuteDropEvent
{
    VclPtr< ::sd::Window>    mpTargetWindow;

    SdNavigatorDropEvent (
        const ExecuteDropEvent& rEvt,
        ::sd::Window* pTargetWindow )
        : ExecuteDropEvent( rEvt ),
          mpTargetWindow( pTargetWindow )
    {}
};

css::uno::Reference< css::datatransfer::XTransferable > View::CreateClipboardDataObject( View*, vcl::Window& )
{
    // since SdTransferable::CopyToClipboard is called, this
    // dynamically created object ist destroyed automatically
    SdTransferable* pTransferable = new SdTransferable( &mrDoc, nullptr, false );
    css::uno::Reference< css::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferClip = pTransferable;

    mrDoc.CreatingDataObj( pTransferable );
    pTransferable->SetWorkDocument( static_cast<SdDrawDocument*>(GetMarkedObjModel()) );
    mrDoc.CreatingDataObj( nullptr );

    // #112978# need to use GetAllMarkedBoundRect instead of GetAllMarkedRect to get
    // fat lines correctly
    const Rectangle                 aMarkRect( GetAllMarkedBoundRect() );
    TransferableObjectDescriptor    aObjDesc;
    SdrOle2Obj*                     pSdrOleObj = nullptr;
    SdrPageView*                    pPgView = GetSdrPageView();
    SdPage*                         pOldPage = pPgView ? static_cast<SdPage*>( pPgView->GetPage() ) : nullptr;
    SdPage*                         pNewPage = const_cast<SdPage*>(static_cast<const SdPage*>( pTransferable->GetWorkDocument()->GetPage( 0 ) ));

    if( pOldPage )
    {
        pNewPage->SetSize( pOldPage->GetSize() );
        pNewPage->SetLayoutName( pOldPage->GetLayoutName() );
    }

    if( GetMarkedObjectCount() == 1 )
    {
        SdrObject* pObj = GetMarkedObjectByIndex(0);

        if( pObj && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr && static_cast<SdrOle2Obj*>(pObj)->GetObjRef().is() )
        {
            // If object has no persistence it must be copied as part of the document
            try
            {
                uno::Reference< embed::XEmbedPersist > xPersObj( static_cast<SdrOle2Obj*>(pObj)->GetObjRef(), uno::UNO_QUERY );
                if ( xPersObj.is() && xPersObj->hasEntry() )
                     pSdrOleObj = static_cast<SdrOle2Obj*>(pObj);
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

css::uno::Reference< css::datatransfer::XTransferable > View::CreateDragDataObject( View* pWorkView, vcl::Window& rWindow, const Point& rDragPos )
{
    SdTransferable* pTransferable = new SdTransferable( &mrDoc, pWorkView, false );
    css::uno::Reference< css::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferDrag = pTransferable;

    TransferableObjectDescriptor    aObjDesc;
    OUString                        aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = nullptr;

    if( GetMarkedObjectCount() == 1 )
    {
        SdrObject* pObj = GetMarkedObjectByIndex( 0 );

        if( pObj && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr && static_cast<SdrOle2Obj*>(pObj)->GetObjRef().is() )
        {
            // If object has no persistence it must be copied as part of the document
            try
            {
                uno::Reference< embed::XEmbedPersist > xPersObj( static_cast<SdrOle2Obj*>(pObj)->GetObjRef(), uno::UNO_QUERY );
                if ( xPersObj.is() && xPersObj->hasEntry() )
                     pSdrOleObj = static_cast<SdrOle2Obj*>(pObj);
            }
            catch( uno::Exception& )
            {}
        }
    }

    if( mpDocSh )
        aDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    if( pSdrOleObj )
        SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pSdrOleObj->GetObjRef(), pSdrOleObj->GetGraphic(), pSdrOleObj->GetAspect() );
    else if (mpDocSh)
        mpDocSh->FillTransferableObjectDescriptor( aObjDesc );

    aObjDesc.maSize = GetAllMarkedRect().GetSize();
    aObjDesc.maDragStartPos = rDragPos;
    aObjDesc.maDisplayName = aDisplayName;

    pTransferable->SetStartPos( rDragPos );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->StartDrag( &rWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );

    return xRet;
}

css::uno::Reference< css::datatransfer::XTransferable > View::CreateSelectionDataObject( View* pWorkView, vcl::Window& rWindow )
{
    SdTransferable*                 pTransferable = new SdTransferable( &mrDoc, pWorkView, true );
    css::uno::Reference< css::datatransfer::XTransferable > xRet( pTransferable );
    TransferableObjectDescriptor    aObjDesc;
    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    OUString                        aDisplayName;

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

void View::UpdateSelectionClipboard( bool bForceDeselect )
{
    if( mpViewSh && mpViewSh->GetActiveWindow() )
    {
        if( !bForceDeselect && GetMarkedObjectList().GetMarkCount() )
            CreateSelectionDataObject( this, *mpViewSh->GetActiveWindow() );
        else if( SD_MOD()->pTransferSelection && ( SD_MOD()->pTransferSelection->GetView() == this ) )
        {
            TransferableHelper::ClearSelection( mpViewSh->GetActiveWindow() );
            SD_MOD()->pTransferSelection = nullptr;
        }
    }
}

void View::DoCut(vcl::Window* )
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        const_cast<OutlinerView*>(pOLV)->Cut();
    else if( AreObjectsMarked() )
    {
        OUString aStr(SD_RESSTR(STR_UNDO_CUT));

        DoCopy();
        BegUndo(aStr + " " + GetDescriptionOfMarkedObjects());
        DeleteMarked();
        EndUndo();
    }
}

void View::DoCopy (vcl::Window* pWindow)
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        const_cast<OutlinerView*>(pOLV)->Copy();
    else if( AreObjectsMarked() )
    {
        BrkAction();
        CreateClipboardDataObject( this, *pWindow );
    }
}

void View::DoPaste (vcl::Window* pWindow)
{
    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpViewSh->GetActiveWindow() ) );
    if( !aDataHelper.GetTransferable().is() )
        return; // empty clipboard?

    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV && EditEngine::HasValidData( aDataHelper.GetTransferable() ) )
    {
        const_cast< OutlinerView* >(pOLV)->PasteSpecial();

        SdrObject*  pObj = GetTextEditObject();
        SdPage*     pPage = static_cast<SdPage*>( pObj ? pObj->GetPage() : nullptr );
        ::Outliner* pOutliner = pOLV->GetOutliner();

        if( pOutliner)
        {
            if( pObj && pPage && pPage->GetPresObjKind(pObj) == PRESOBJ_TITLE )
            {
                // remove all hard linebreaks from the title
                if( pOutliner && pOutliner->GetParagraphCount() > 1 )
                {
                    bool bOldUpdateMode = pOutliner->GetUpdateMode();

                    pOutliner->SetUpdateMode( false );

                    const EditEngine& rEdit = pOutliner->GetEditEngine();
                    const sal_Int32 nParaCount = rEdit.GetParagraphCount();

                    for( sal_Int32 nPara = nParaCount - 2; nPara >= 0; nPara-- )
                    {
                        const sal_Int32 nParaLen = rEdit.GetTextLen( nPara );
                        pOutliner->QuickDelete( ESelection( nPara, nParaLen, nPara+1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( nPara, nParaLen, nPara, nParaLen ) );
                    }

                    DBG_ASSERT( rEdit.GetParagraphCount() <= 1, "Titleobject contains hard line breaks" );
                    pOutliner->SetUpdateMode(bOldUpdateMode);
                }
            }

            if( !mrDoc.IsChanged() )
            {
                if( pOutliner && pOutliner->IsModified() )
                    mrDoc.SetChanged();
            }
        }
    }
    else
    {
        Point       aPos;
        sal_Int8    nDnDAction = DND_ACTION_COPY;

        if( pWindow )
            aPos = pWindow->PixelToLogic( Rectangle( aPos, pWindow->GetOutputSizePixel() ).Center() );

        DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>( mpDocSh->GetViewShell() );

        if (pDrViewSh != nullptr)
        {
            if( !InsertData( aDataHelper, aPos, nDnDAction, false ) )
            {
                INetBookmark    aINetBookmark( "", "" );

                if( ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) &&
                      aDataHelper.GetINetBookmark( SotClipboardFormatId::UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                {
                    pDrViewSh->InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), "" );
                }
            }
        }
    }
}

void View::StartDrag( const Point& rStartPos, vcl::Window* pWindow )
{
    if( AreObjectsMarked() && IsAction() && mpViewSh && pWindow && !mpDragSrcMarkList )
    {
        BrkAction();

        if( IsTextEdit() )
            SdrEndTextEdit();

        DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpDocSh ? mpDocSh->GetViewShell() : nullptr );

        if( pDrawViewShell )
        {
            rtl::Reference<FuPoor> xFunction( pDrawViewShell->GetCurrentFunction() );

            if( xFunction.is() && nullptr != dynamic_cast< const FuDraw *>( xFunction.get() ) )
                static_cast<FuDraw*>(xFunction.get())->ForcePointer();
        }

        mpDragSrcMarkList = new SdrMarkList(GetMarkedObjectList());
        mnDragSrcPgNum = GetSdrPageView()->GetPage()->GetPageNum();

        if( IsUndoEnabled() )
        {
            OUString aStr(SD_RESSTR(STR_UNDO_DRAGDROP));
            BegUndo(aStr + " " + mpDragSrcMarkList->GetMarkDescription());
        }
        CreateDragDataObject( this, *pWindow, rStartPos );
    }
}

void View::DragFinished( sal_Int8 nDropAction )
{
    const bool bUndo = IsUndoEnabled();

    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if( pDragTransferable )
        pDragTransferable->SetView( nullptr );

    if( ( nDropAction & DND_ACTION_MOVE ) &&
        pDragTransferable && !pDragTransferable->IsInternalMove() &&
        mpDragSrcMarkList && mpDragSrcMarkList->GetMarkCount() &&
        !IsPresObjSelected() )
    {
        mpDragSrcMarkList->ForceSort();

        if( bUndo )
            BegUndo();

        const size_t nAnz = mpDragSrcMarkList->GetMarkCount();

        for( size_t nm = nAnz; nm>0; )
        {
            --nm;
            SdrMark* pM=mpDragSrcMarkList->GetMark(nm);
            if( bUndo )
                AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeleteObject(*pM->GetMarkedSdrObj()));
        }

        mpDragSrcMarkList->GetMark(0)->GetMarkedSdrObj()->GetOrdNum();

        for (size_t nm = nAnz; nm>0;)
        {
            --nm;
            SdrMark* pM=mpDragSrcMarkList->GetMark(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();

            if( pObj && pObj->GetPage() )
            {
                const size_t nOrdNum = pObj->GetOrdNumDirect();
                SdrObject* pChkObj = pObj->GetPage()->RemoveObject(nOrdNum);
                DBG_ASSERT(pChkObj==pObj,"pChkObj!=pObj in RemoveObject()");
            }
        }

        if( bUndo )
            EndUndo();
    }

    if( pDragTransferable )
        pDragTransferable->SetInternalMove( false );

    //This Undo appears to matches with the STR_UNDO_DRAGDROP Undo Start of
    //View::StartDrag But this DragFinished can be called without a matching
    //StartDrag. So use the existence of mpDragSrcMarkList as a flag that
    //this EndUndo has a matching BegUndo
    if (bUndo && mpDragSrcMarkList)
        EndUndo();
    mnDragSrcPgNum = SDRPAGE_NOTFOUND;
    delete mpDragSrcMarkList;
    mpDragSrcMarkList = nullptr;
}

sal_Int8 View::AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                             ::sd::Window*, sal_uInt16, sal_uInt16 nLayer )
{
    OUString        aLayerName = GetActiveLayer();
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
        bool                bIsInsideOutlinerView = false;

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
                bIsInsideOutlinerView = true;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

            if(pDragTransferable && (nDropAction & DND_ACTION_LINK))
            {
                // suppress own data when it's intention is to use it as fill information
                pDragTransferable = nullptr;
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
                const bool  bDrawing = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::DRAWING );
                const bool  bGraphic = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::SVXB );
                const bool  bMtf = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::GDIMETAFILE );
                const bool  bBitmap = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::BITMAP );
                bool        bBookmark = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK );
                bool        bXFillExchange = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::XFA );

                // check handle insert
                if( !nRet && ( (bXFillExchange && ( SdrDragMode::Gradient == GetDragMode() )) || ( SdrDragMode::Transparence == GetDragMode() ) ) )
                {
                    const SdrHdlList& rHdlList = GetHdlList();

                    for( size_t n = 0; n < rHdlList.GetHdlCount(); ++n )
                    {
                        SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                        if( pIAOHandle && ( SdrHdlKind::Color == pIAOHandle->GetKind() ) )
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
                    SdrPageView*    pPageView = nullptr;
                    ::sd::Window* pWindow = mpViewSh->GetActiveWindow();
                    Point           aPos( pWindow->PixelToLogic( rEvt.maPosPixel ) );
                    SdrObject* pPickObj = PickObj(aPos, getHitTolLog(), pPageView);
                    bool            bIsPresTarget = false;

                    if (pPickObj && (pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall()))
                    {
                        SdPage* pPage = static_cast<SdPage*>( pPickObj->GetPage() );

                        if( pPage && pPage->IsMasterPage() )
                            bIsPresTarget = pPage->IsPresObj( pPickObj );
                    }

                    if (pPickObj && !bIsPresTarget && (bGraphic || bMtf || bBitmap || bXFillExchange))
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
                        bXFillExchange = false;
                }

                // check normal insert
                if( !nRet )
                {
                    const bool  bSBAFormat = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::SVX_FORMFIELDEXCH );
                    const bool  bEditEngine = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::EDITENGINE );
                    const bool  bString = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::STRING );
                    const bool  bRTF = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::RTF );
                    const bool  bFile = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE );
                    const bool  bFileList = rTargetHelper.IsDropFormatSupported( SotClipboardFormatId::FILE_LIST );

                    if( mpDropMarker )
                    {
                        ImplClearDrawDropMarker();
                        mpDropMarkerObj = nullptr;
                    }

                    if( bBookmark && bFile && ( nDropAction & DND_ACTION_MOVE ) && mpViewSh && SlideShow::IsRunning(mpViewSh->GetViewShellBase()) )
                        bBookmark = false;

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
        mpDropMarkerObj = nullptr;
    }

    return nRet;
}

sal_Int8 View::ExecuteDrop( const ExecuteDropEvent& rEvt,
                              ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer )
{
    SdrPageView*    pPV = GetSdrPageView();
    OUString        aActiveLayer = GetActiveLayer();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    // destroy drop marker if it is shown
    if( mpDropMarker )
    {
        ImplClearDrawDropMarker();
        mpDropMarkerObj = nullptr;
    }

    if( !pPV->IsLayerLocked( aActiveLayer ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        bool                bIsInsideOutlinerView = false;

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
                bIsInsideOutlinerView = true;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            Point                   aPos;
            TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );

            if( pTargetWindow )
                aPos = pTargetWindow->PixelToLogic( rEvt.maPosPixel );

            // handle insert?
            if( (!nRet && ( SdrDragMode::Gradient == GetDragMode() )) || (( SdrDragMode::Transparence == GetDragMode() ) && aDataHelper.HasFormat( SotClipboardFormatId::XFA )) )
            {
                const SdrHdlList& rHdlList = GetHdlList();

                for( size_t n = 0; !nRet && n < rHdlList.GetHdlCount(); ++n )
                {
                    SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                    if( pIAOHandle && ( SdrHdlKind::Color == pIAOHandle->GetKind() ) )
                    {
                        if(pIAOHandle->getOverlayObjectList().isHitPixel(rEvt.maPosPixel))
                        {
                            ::tools::SvRef<SotStorageStream> xStm;

                            if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::XFA, xStm ) && xStm.is() )
                            {
                                XFillExchangeData aFillData( XFillAttrSetItem( &mrDoc.GetPool() ) );

                                ReadXFillExchangeData( *xStm, aFillData );
                                const Color aColor( static_cast<const XFillColorItem&>( aFillData.GetXFillAttrSetItem()->GetItemSet().Get( XATTR_FILLCOLOR ) ).GetColorValue() );
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetColor( aColor, true );
                                nRet = nDropAction;
                            }
                        }
                    }
                }
            }

            // standard insert?
            if( !nRet && InsertData( aDataHelper, aPos, nDropAction, true, SotClipboardFormatId::NONE, nPage, nLayer ) )
                nRet = nDropAction;

            // special insert?
            if( !nRet && mpViewSh )
            {
                OUString        aTmpString1, aTmpString2;
                INetBookmark    aINetBookmark( aTmpString1, aTmpString2 );

                // insert bookmark
                if( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
                    aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) )
                {
                    SdPageObjsTLB::SdPageObjsTransferable* pPageObjsTransferable = SdPageObjsTLB::SdPageObjsTransferable::getImplementation( aDataHelper.GetXTransferable() );

                    if( pPageObjsTransferable &&
                        ( NAVIGATOR_DRAGTYPE_LINK == pPageObjsTransferable->GetDragType() ||
                          NAVIGATOR_DRAGTYPE_EMBEDDED == pPageObjsTransferable->GetDragType() ) )
                    {
                        // insert bookmark from own navigator (handled async. due to possible message box )
                        Application::PostUserEvent( LINK( this, View, ExecuteNavigatorDrop ),
                                                    new SdNavigatorDropEvent( rEvt, pTargetWindow ) );
                        nRet = nDropAction;
                    }
                    else
                    {
                        SdrPageView*    pPageView = nullptr;

                        SdrObject* pPickObj = PickObj(aPos, getHitTolLog(), pPageView);
                        if (pPickObj)
                        {
                            // insert as clip action => jump
                            OUString       aBookmark( aINetBookmark.GetURL() );
                            SdAnimationInfo*    pInfo = SdDrawDocument::GetAnimationInfo( pPickObj );

                            if( !aBookmark.isEmpty() )
                            {
                                bool bCreated = false;

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
                                    bCreated = true;
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
                                pAction->SetPathObj(nullptr, nullptr);
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
                        else if( dynamic_cast< const DrawViewShell *>( mpViewSh ) !=  nullptr )
                        {
                            // insert as normal URL button
                            static_cast<DrawViewShell*>( mpViewSh )->InsertURLButton( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), OUString(), &aPos );
                            nRet = nDropAction;
                        }
                    }
                }
            }
        }
    }

    return nRet;
}

IMPL_LINK( View, ExecuteNavigatorDrop, void*, p, void )
{
    SdNavigatorDropEvent*                   pSdNavigatorDropEvent = static_cast<SdNavigatorDropEvent*>(p);
    TransferableDataHelper                  aDataHelper( pSdNavigatorDropEvent->maDropEvent.Transferable );
    SdPageObjsTLB::SdPageObjsTransferable*  pPageObjsTransferable = SdPageObjsTLB::SdPageObjsTransferable::getImplementation( aDataHelper.GetXTransferable() );
    INetBookmark                            aINetBookmark;

    if( pPageObjsTransferable && aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) )
    {
        Point   aPos;
        OUString  aBookmark;
        SdPage* pPage = static_cast<SdPage*>( GetSdrPageView()->GetPage() );
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
            if( pPage->GetPageKind() == PageKind::Standard )
                nPgPos = pPage->GetPageNum() + 2;
            else if( pPage->GetPageKind() == PageKind::Notes )
                nPgPos = pPage->GetPageNum() + 1;
        }

        /* In order t ensure unique page names, we test the ones we want to
           insert. If necessary. we put them into and replacement list (bNameOK
           == sal_False -> User canceled).  */
        bool    bLink = pPageObjsTransferable->GetDragType() == NAVIGATOR_DRAGTYPE_LINK;
        bool    bNameOK = GetExchangeList( aExchangeList, aBookmarkList, 2 );
        bool    bReplace = false;

        /* Since we don't know the type (page or object), we fill a list with
           pages and objects.
           Of course we have problems if there are pages and objects with the
           same name!!! */
        if( bNameOK )
        {
            mrDoc.InsertBookmark( aBookmarkList, aExchangeList,
                                  bLink, bReplace, nPgPos,
                                  &pPageObjsTransferable->GetDocShell(),
                                  &aPos );
        }
    }

    delete pSdNavigatorDropEvent;
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
        OUString aNewName = *pIter;

        if( nType == 0  || nType == 2 )
            bNameOK = mpDocSh->CheckPageName(mpViewSh->GetActiveWindow(), aNewName);

        if( bNameOK && ( nType == 1  || nType == 2 ) )
        {
            if( mrDoc.GetObj( aNewName ) )
            {
                OUString aTitle(SD_RESSTR(STR_TITLE_NAMEGROUP));
                OUString aDesc(SD_RESSTR(STR_DESC_NAMEGROUP));

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact ? pFact->CreateSvxNameDialog(mpViewSh->GetActiveWindow(), aNewName, aDesc) : nullptr);

                if (pDlg)
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
    bool bIsGroup(pObj->IsGroupObject());
    if(bIsGroup && dynamic_cast< const E3dObject *>( pObj ) != nullptr && dynamic_cast< const E3dScene *>( pObj ) ==  nullptr)
        bIsGroup = false;

    rVector.push_back( pObj );

    if(bIsGroup)
    {
        SdrObjList* pObjList = pObj->GetSubList();
        for( size_t a = 0; a < pObjList->GetObjCount(); ++a)
            ImplProcessObjectList(pObjList->GetObj(a), rVector);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
