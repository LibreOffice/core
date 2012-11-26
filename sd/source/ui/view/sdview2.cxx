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
#include "precompiled_sd.hxx"


#include "View.hxx"

#include <vector>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <tools/ref.hxx>
#include <tools/urlobj.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
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
#include <svl/urlbmk.hxx>
#include <editeng/outliner.hxx>
#include <svx/xflclit.hxx>
#include <svx/dbexch.hrc>
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
#include <svx/svdlegacy.hxx>

#include "slideshow.hxx"

namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

using namespace ::com::sun::star;

// ------------------------
// - SdNavigatorDropEvent -
// ------------------------

struct SdNavigatorDropEvent : public ExecuteDropEvent
{
    DropTargetHelper&       mrTargetHelper;
    ::sd::Window* mpTargetWindow;
    sal_uInt32                  mnPage;
    sal_uInt32                  mnLayer;

    SdNavigatorDropEvent (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt32 nPage,
        SdrLayerID aLayer )
        : ExecuteDropEvent( rEvt ),
          mrTargetHelper( rTargetHelper ),
          mpTargetWindow( pTargetWindow ),
          mnPage( nPage ),
          mnLayer( aLayer )
    {}
};

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateClipboardDataObject( View*, ::Window& )
{
    // since SdTransferable::CopyToClipboard is called, this
    // dynamically created object ist destroyed automatically
    SdTransferable* pTransferable = new SdTransferable( mpDoc, NULL, false );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferClip = pTransferable;

    mpDoc->CreatingDataObj( pTransferable );
    pTransferable->SetWorkDocument( (SdDrawDocument*) GetAllMarkedModel() );
    mpDoc->CreatingDataObj( NULL );

    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrPageView*                    pPgView = GetSdrPageView();
    SdPage*                         pOldPage = pPgView ? ( (SdPage*) &pPgView->getSdrPageFromSdrPageView() ) : NULL;
    SdPage*                         pNewPage = (SdPage*) pTransferable->GetWorkDocument()->GetPage( 0 );

    if( pOldPage )
    {
        pNewPage->SetPageScale( pOldPage->GetPageScale() );
        pNewPage->SetLayoutName( pOldPage->GetLayoutName() );
    }

    SdrOle2Obj* pSdrOleObj = dynamic_cast< SdrOle2Obj* >(getSelectedIfSingle());

    if( pSdrOleObj && pSdrOleObj->GetObjRef().is() )
    {
        // If object has no persistence it must be copied as part of the document
        try
        {
            uno::Reference< embed::XEmbedPersist > xPersObj( pSdrOleObj->GetObjRef(), uno::UNO_QUERY );

            if ( !xPersObj.is() || !xPersObj->hasEntry() )
                 pSdrOleObj = 0;
        }
        catch( uno::Exception& )
        {}
    }

    if( pSdrOleObj )
    {
        SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pSdrOleObj->GetObjRef(), pSdrOleObj->GetGraphic(), pSdrOleObj->GetAspect() );
    }
    else
    {
        pTransferable->GetWorkDocument()->GetDocSh()->FillTransferableObjectDescriptor( aObjDesc );
    }

    if( mpDocSh )
        aObjDesc.maDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    const basegfx::B2DRange aAllRange(getMarkedObjectSnapRange());
    aObjDesc.maSize = Size(basegfx::fround(aAllRange.getWidth()), basegfx::fround(aAllRange.getHeight()));

    pTransferable->SetStartPos(aAllRange.getMinimum());
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToClipboard( mpViewSh->GetActiveWindow() );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateDragDataObject( View* pWorkView, ::Window& rWindow, const Point& rDragPos )
{
    SdTransferable* pTransferable = new SdTransferable( mpDoc, pWorkView, false );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferDrag = pTransferable;

    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj* pSdrOleObj = dynamic_cast< SdrOle2Obj* >(getSelectedIfSingle());

    if(pSdrOleObj && pSdrOleObj->GetObjRef().is())
    {
        // If object has no persistence it must be copied as part of the document
        try
        {
            uno::Reference< embed::XEmbedPersist > xPersObj( pSdrOleObj->GetObjRef(), uno::UNO_QUERY );

            if ( !xPersObj.is() || !xPersObj->hasEntry() )
                 pSdrOleObj = 0;
        }
        catch( uno::Exception& )
        {}
    }

    if( mpDocSh )
    {
        aDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    }

    if( pSdrOleObj )
    {
        SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pSdrOleObj->GetObjRef(), pSdrOleObj->GetGraphic(), pSdrOleObj->GetAspect() );
    }
    else
    {
        mpDocSh->FillTransferableObjectDescriptor( aObjDesc );
    }

    const basegfx::B2DRange aAllRange(getMarkedObjectSnapRange());
    aObjDesc.maSize = Size(basegfx::fround(aAllRange.getWidth()), basegfx::fround(aAllRange.getHeight()));
    aObjDesc.maDragStartPos = rDragPos;
    aObjDesc.maDisplayName = aDisplayName;
    aObjDesc.mbCanLink = false;

    pTransferable->SetStartPos( basegfx::B2DPoint(rDragPos.X(), rDragPos.Y()) );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->StartDrag( &rWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateSelectionDataObject( View* pWorkView, ::Window& rWindow )
{
    SdTransferable*                 pTransferable = new SdTransferable( mpDoc, pWorkView, true );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );
    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;

    SD_MOD()->pTransferSelection = pTransferable;

    if( mpDocSh )
    {
        aDisplayName = mpDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        mpDocSh->FillTransferableObjectDescriptor( aObjDesc );
    }

    const basegfx::B2DRange aAllRange(getMarkedObjectSnapRange());
    aObjDesc.maSize = Size(basegfx::fround(aAllRange.getWidth()), basegfx::fround(aAllRange.getHeight()));

    pTransferable->SetStartPos(aAllRange.getMinimum());
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToSelection( &rWindow );

    return xRet;
}

// -----------------------------------------------------------------------------

void View::UpdateSelectionClipboard( bool bForceDeselect )
{
    if( mpViewSh && mpViewSh->GetActiveWindow() )
    {
        if( !bForceDeselect && areSdrObjectsSelected() )
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
    {
        ( (OutlinerView*) pOLV)->Cut();
    }
    else if( areSdrObjectsSelected() )
    {
        String aStr( SdResId(STR_UNDO_CUT) );
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        DoCopy();
        BegUndo( ( aStr += sal_Unicode(' ') ) += getSelectionDescription(aSelection) );
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
    else if( areSdrObjectsSelected() )
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
        SdPage*     pPage = (SdPage*)( pObj ? pObj->getSdrPageFromSdrObject() : NULL );
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
                    const int nParaCount = rEdit.GetParagraphCount();

                    for( int nPara = nParaCount - 2; nPara >= 0; nPara-- )
                    {
                        const sal_uInt16 nParaLen = (sal_uInt16)rEdit.GetTextLen( (sal_uInt16)nPara );
                        pOutliner->QuickDelete( ESelection( (sal_uInt16)nPara, nParaLen, (sal_uInt16)nPara+1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( (sal_uInt16)nPara, nParaLen, (sal_uInt16)nPara, nParaLen ) );
                    }

                    DBG_ASSERT( rEdit.GetParagraphCount() <= 1, "Titelobjekt contains hard line breaks" );
                    pOutliner->SetUpdateMode(bOldUpdateMode);
                }
            }

            if( !mpDoc->IsChanged() )
            {
                if( pOutliner && pOutliner->IsModified() )
                    mpDoc->SetChanged( true );
            }
        }
    }
    else
    {
        DrawViewShell* pDrViewSh = dynamic_cast< DrawViewShell* >(mpDocSh->GetViewShell());

        if(pDrViewSh)
        {
            basegfx::B2DPoint aPos(0.0, 0.0);
            sal_Int8 nDnDAction(DND_ACTION_COPY);

            if(pWindow)
            {
                aPos = pWindow->GetLogicRange().getCenter();
            }

            if(!InsertData(aDataHelper, aPos, nDnDAction, false))
            {
                INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                    ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                      aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                {
                    pDrViewSh->InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, 0);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void View::StartDrag( const Point& rStartPos, ::Window* pWindow )
{
    if( areSdrObjectsSelected() && IsAction() && mpViewSh && pWindow && !maDragSrcMarkList.size() && GetSdrPageView() )
    {
        BrkAction();

        if( IsTextEdit() )
            SdrEndTextEdit();

        DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpDocSh ? mpDocSh->GetViewShell() : 0 );

        if( pDrawViewShell )
        {
            FunctionReference xFunction( pDrawViewShell->GetCurrentFunction() );
            FuDraw* pFuDraw = dynamic_cast< FuDraw* >(xFunction.get());

            if( pFuDraw )
                pFuDraw->ForcePointer( NULL );
        }

        maDragSrcMarkList = getSelectedSdrObjectVectorFromSdrMarkView();
        mnDragSrcPgNum = GetSdrPageView()->getSdrPageFromSdrPageView().GetPageNumber();

        if( IsUndoEnabled() )
        {
            String aStr( SdResId(STR_UNDO_DRAGDROP) );
            aStr += sal_Unicode(' ');
            aStr += getSelectionDescription(maDragSrcMarkList);
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
        maDragSrcMarkList.size() &&
        !IsPresObjSelected() )
    {
        if( bUndo )
            BegUndo();

        sal_uInt32 nm;

         for( nm = maDragSrcMarkList.size(); nm > 0; )
        {
            nm--;
            if( bUndo )
            {
                AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoDeleteObject(*maDragSrcMarkList[nm]));
            }
        }

        for (nm = maDragSrcMarkList.size(); nm > 0; )
        {
            nm--;
            SdrObject* pObj = maDragSrcMarkList[nm];

            if( pObj && pObj->getParentOfSdrObject() )
            {
#ifdef DBG_UTIL
                SdrObject* pChkObj =
#endif
                pObj->getParentOfSdrObject()->RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());
                DBG_ASSERT(pChkObj==pObj,"pChkObj!=pObj beim RemoveObject()");
            }
        }

        if( bUndo )
            EndUndo();
    }

    if( pDragTransferable )
        pDragTransferable->SetInternalMove( false );

    if( bUndo )
        EndUndo();

    mnDragSrcPgNum = SDRPAGE_NOTFOUND;
    maDragSrcMarkList.clear();
}

// -----------------------------------------------------------------------------

sal_Int8 View::AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                             ::sd::Window*, sal_uInt32, SdrLayerID aLayer )
{
    String          aLayerName( GetActiveLayer() );
    SdrPageView*    pPV = GetSdrPageView();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    if( aLayer != SDRLAYER_NOTFOUND )
    {
        SdrLayerAdmin& rLayerAdmin = mpDoc->GetModelLayerAdmin();
        aLayerName = rLayerAdmin.GetLayerPerID(aLayer)->GetName();
    }

    if( mbIsDropAllowed && pPV && !pPV->IsLayerLocked( aLayerName ) && pPV->IsLayerVisible( aLayerName ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        bool                bIsInsideOutlinerView = false;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );
            SdrObject* pSelected = getSelectedIfSingle();

            if (pSelected)
            {
                aRect.Union( sdr::legacy::GetLogicRect(*pSelected) );
            }

            if( aRect.IsInside( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) ) )
            {
                bIsInsideOutlinerView = true;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

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
                        pSourceView->GetDocSh()->GetMedium()->GetName().Len() )
                    {
                        nRet = nDropAction;
                    }
                }
            }
            else
            {
                const bool  bDrawing = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING );
                const bool  bGraphic = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB );
                const bool  bMtf = rTargetHelper.IsDropFormatSupported( FORMAT_GDIMETAFILE );
                const bool  bBitmap = rTargetHelper.IsDropFormatSupported( FORMAT_BITMAP );
                bool        bBookmark = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                bool        bXFillExchange = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_XFA );

                // check handle insert
                if( !nRet && ( (bXFillExchange && ( SDRDRAG_GRADIENT == GetDragMode() )) || ( SDRDRAG_TRANSPARENCE == GetDragMode() ) ) )
                {
                    const SdrHdlList& rHdlList = GetHdlList();

                    for( sal_uInt32 n(0); n < rHdlList.GetHdlCount(); n++ )
                    {
                        SdrHdlColor* pIAOHandle = dynamic_cast< SdrHdlColor* >(rHdlList.GetHdlByIndex( n ));

                        if( pIAOHandle && HDL_COLR == pIAOHandle->GetKind() )
                        {
                            if(pIAOHandle->getOverlayObjectList().isHitPixel(rEvt.maPosPixel))
                            {
                                nRet = nDropAction;
                                pIAOHandle->SetSize( SDR_HANDLE_COLOR_SIZE_SELECTED );
                            }
                            else
                            {
                                pIAOHandle->SetSize( SDR_HANDLE_COLOR_SIZE_NORMAL );
                            }
                        }
                    }
                }

                // check object insert
                if( !nRet && ( bXFillExchange || ( ( bDrawing || bGraphic || bMtf || bBitmap || bBookmark ) && ( nDropAction & DND_ACTION_LINK ) ) ) )
                {
                    SdrObject*      pPickObj = NULL;
                    ::sd::Window* pWindow = mpViewSh->GetActiveWindow();
                    const basegfx::B2DPoint aPos(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rEvt.maPosPixel.X(), rEvt.maPosPixel.Y()));
                    const bool      bHasPickObj = PickObj( aPos, getHitTolLog(), pPickObj );
                    bool            bIsPresTarget = false;

                    if( bHasPickObj && pPickObj && ( pPickObj->IsEmptyPresObj() || findConnectionToSdrObject(pPickObj) ) )
                    {
                        SdPage* pPage = (SdPage*) pPickObj->getSdrPageFromSdrObject();

                        if( pPage && pPage->IsMasterPage() )
                            bIsPresTarget = pPage->IsPresObj( pPickObj );
                    }

                    SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pPickObj);

                    if( bHasPickObj && !bIsPresTarget &&
                        ( !pSdrGrafObj || bGraphic || bMtf || bBitmap || ( bXFillExchange && !pSdrGrafObj && !dynamic_cast< SdrOle2Obj* >(pPickObj) ) ) )
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
                    const bool  bSBAFormat = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH );
                    const bool  bEditEngine = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_EDITENGINE );
                    const bool  bString = rTargetHelper.IsDropFormatSupported( FORMAT_STRING );
                    const bool  bRTF = rTargetHelper.IsDropFormatSupported( FORMAT_RTF );
                    const bool  bFile = rTargetHelper.IsDropFormatSupported( FORMAT_FILE );
                    const bool  bFileList = rTargetHelper.IsDropFormatSupported( FORMAT_FILE_LIST );

                    if( mpDropMarker )
                    {
                        ImplClearDrawDropMarker();
                        mpDropMarkerObj = NULL;
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
        mpDropMarkerObj = NULL;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int8 View::ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                              ::sd::Window* pTargetWindow, sal_uInt32 nPage, SdrLayerID aLayer )
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

    if( pPV && !pPV->IsLayerLocked( aActiveLayer ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        bool                bIsInsideOutlinerView = false;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );
            SdrObject* pSelected = getSelectedIfSingle();

            if( pSelected )
            {
                aRect.Union( sdr::legacy::GetLogicRect(*pSelected) );
            }

            Point aPos( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) );

            if( aRect.IsInside( aPos ) )
            {
                bIsInsideOutlinerView = true;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            basegfx::B2DPoint aPos;
            TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );

            if( pTargetWindow )
            {
                aPos = pTargetWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rEvt.maPosPixel.X(), rEvt.maPosPixel.Y());
            }

            // handle insert?
            if( (!nRet && ( SDRDRAG_GRADIENT == GetDragMode() )) || (( SDRDRAG_TRANSPARENCE == GetDragMode() ) && aDataHelper.HasFormat( SOT_FORMATSTR_ID_XFA )) )
            {
                const SdrHdlList& rHdlList = GetHdlList();

                for( sal_uInt32 n = 0; !nRet && n < rHdlList.GetHdlCount(); n++ )
                {
                    SdrHdl* pIAOHandle = rHdlList.GetHdlByIndex( n );

                    if( pIAOHandle && ( HDL_COLR == pIAOHandle->GetKind() ) )
                    {
                        if(pIAOHandle->getOverlayObjectList().isHitPixel(rEvt.maPosPixel))
                        {
                            SotStorageStreamRef xStm;

                            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) && xStm.Is() )
                            {
                                XFillExchangeData aFillData( XFillAttrSetItem( &mpDoc->GetItemPool() ) );

                                *xStm >> aFillData;
                                const Color aColor( ( (XFillColorItem&) aFillData.GetXFillAttrSetItem()->GetItemSet().Get( XATTR_FILLCOLOR ) ).GetColorValue() );
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetColor( aColor, true );
                                nRet = nDropAction;
                            }
                        }
                    }
                }
            }

            // standard insert?
            if( !nRet && InsertData( aDataHelper, aPos, nDropAction, true, 0, nPage, aLayer ) )
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
                                                                              nPage, aLayer ) );
                        nRet = nDropAction;
                    }
                    else
                    {
                        SdrObject*      pPickObj = NULL;

                        if( PickObj( aPos, getHitTolLog(), pPickObj ) )
                        {
                            // insert as clip action => jump
                            rtl::OUString       aBookmark( aINetBookmark.GetURL() );
                            SdAnimationInfo*    pInfo = mpDoc->GetAnimationInfo( pPickObj );
                            bool                bCreated = false;

                            if( aBookmark.getLength() )
                            {
                                presentation::ClickAction eClickAction = presentation::ClickAction_DOCUMENT;

                                sal_Int32 nIndex = aBookmark.indexOf( (sal_Unicode)'#' );
                                if( nIndex != -1 )
                                {
                                    const String aDocName( aBookmark.copy( 0, nIndex ) );

                                    if( mpDocSh->GetMedium()->GetName() == aDocName || mpDocSh->GetName() == aDocName )
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

                                // Undo-Action mit alten und neuen Groessen erzeugen
                                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction(mpDoc, pPickObj, bCreated);
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
//                              pAction->SetInvisibleInPres(pInfo->mbInvisibleInPresentation, true);
                                pAction->SetVerb(pInfo->mnVerb, pInfo->mnVerb);
                                pAction->SetSecondEffect(pInfo->meSecondEffect, pInfo->meSecondEffect);
                                pAction->SetSecondSpeed(pInfo->meSecondSpeed, pInfo->meSecondSpeed);
                                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, pInfo->mbSecondSoundOn);
                                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull, pInfo->mbSecondPlayFull);

                                String aString(SdResId(STR_UNDO_ANIMATION));
                                pAction->SetComment(aString);
                                mpDocSh->GetUndoManager()->AddUndoAction(pAction);
                                pInfo->meClickAction = eClickAction;
                                pInfo->SetBookmark( aBookmark );
                                mpDoc->SetChanged();

                                nRet = nDropAction;
                            }
                        }
                        else
                        {
                            DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewSh);

                            if( pDrawViewShell )
                            {
                                // insert as normal URL button
                                pDrawViewShell->InsertURLButton( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), String(), &aPos );
                                nRet = nDropAction;
                            }
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

    if( pPageObjsTransferable
        && aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark )
        && GetSdrPageView() )
    {
        basegfx::B2DPoint aPos;
        List    aBookmarkList;
        String  aBookmark;
        SdPage& rPage = (SdPage&) GetSdrPageView()->getSdrPageFromSdrPageView();
        sal_uInt32 nPgPos = 0xffffffff;

        if( pSdNavigatorDropEvent->mpTargetWindow )
        {
            aPos = basegfx::B2DPoint(pSdNavigatorDropEvent->maPosPixel.X(), pSdNavigatorDropEvent->maPosPixel.Y());
            aPos = pSdNavigatorDropEvent->mpTargetWindow->GetInverseViewTransformation() * aPos;
        }

        const rtl::OUString aURL( aINetBookmark.GetURL() );
        sal_Int32 nIndex = aURL.indexOf( (sal_Unicode)'#' );
        if( nIndex != -1 )
            aBookmark = aURL.copy( nIndex+1 );
        aBookmarkList.Insert( &aBookmark );

        if( !rPage.IsMasterPage() )
        {
            if( rPage.GetPageKind() == PK_STANDARD )
                nPgPos = rPage.GetPageNumber() + 2;
            else if( rPage.GetPageKind() == PK_NOTES )
                nPgPos = rPage.GetPageNumber() + 1;
        }

        // Um zu gewaehrleisten, dass alle Seitennamen eindeutig sind, werden
        // die einzufuegenden geprueft und gegebenenfalls in einer Ersatzliste
        // aufgenommen (bNameOK == false -> Benutzer hat abgebrochen)
        List*   pExchangeList = NULL;
        bool    bLink = ( NAVIGATOR_DRAGTYPE_LINK == pPageObjsTransferable->GetDragType()  ? true : false );
        bool    bNameOK = GetExchangeList( pExchangeList, &aBookmarkList, 2 );
        bool    bReplace = false;

        // Da man hier nicht weiss, ob es sich um eine Seite oder ein Objekt handelt,
        // wird eine Liste sowohl mit Seiten, als auch mit Objekten gefuellt.
        // Sollten Seitennamen und Objektnamen identisch sein gibt es hier natuerlich Probleme !!!
        if( bNameOK )
        {
            mpDoc->InsertBookmark( &aBookmarkList, pExchangeList,
                                  bLink, bReplace, nPgPos, false,
                                  &pPageObjsTransferable->GetDocShell(),
                                  true, &aPos );
        }

        // Loeschen der ExchangeList
        if( pExchangeList )
        {
            for( void* p = pExchangeList->First(); p; p = pExchangeList->Next() )
                delete (String*) p;

            delete pExchangeList;
        }
    }

    delete pSdNavigatorDropEvent;

    return 0;
}

/*************************************************************************
|*
|* Rueckgabeparameter:
|* pExchangeList == NULL -> Namen sind alle eindeutig
|* bNameOK == false -> Benutzer hat abgebrochen
|* nType == 0 -> Seiten
|* nType == 1 -> Objekte
|* nType == 2 -> Seiten + Objekte
|*
\************************************************************************/

bool View::GetExchangeList( List*& rpExchangeList, List* pBookmarkList, sal_uInt16 nType )
{
    DBG_ASSERT( !rpExchangeList, "ExchangeList muss NULL sein!");

    bool bListIdentical = true; // BookmarkList und ExchangeList sind gleich
    bool bNameOK = true;        // Name ist eindeutig

    rpExchangeList = new List();

    if( pBookmarkList )
    {
        String* pString = (String*) pBookmarkList->First();

        while( pString && bNameOK )
        {
            String* pNewName = new String( *pString );

            if( nType == 0  || nType == 2 )
                bNameOK = mpDocSh->CheckPageName (
                    mpViewSh->GetActiveWindow(), *pNewName);

            if( bNameOK && ( nType == 1  || nType == 2 ) )
            {
                if( mpDoc->GetObj( *pNewName ) )
                {
                    String          aTitle( SdResId( STR_TITLE_NAMEGROUP ) );
                    String          aDesc( SdResId( STR_DESC_NAMEGROUP ) );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    AbstractSvxNameDialog* pDlg = pFact ? pFact->CreateSvxNameDialog( mpViewSh->GetActiveWindow(), *pNewName, aDesc ) : 0;
                    if( pDlg )
                    {
                        pDlg->SetEditHelpId( HID_SD_NAMEDIALOG_OBJECT );

                        bNameOK = false;
                        pDlg->SetText( aTitle );

                        while( !bNameOK && pDlg->Execute() == RET_OK )
                        {
                            pDlg->GetName( *pNewName );

                            if( !mpDoc->GetObj( *pNewName ) )
                                bNameOK = true;
                        }

                        delete pDlg;
                    }
                }
            }

            if( bListIdentical )
                bListIdentical = ( *pString == *pNewName );

            rpExchangeList->Insert( pNewName, LIST_APPEND );
            pString = (String*) pBookmarkList->Next();
        }
    }

    // ExchangeList ist mit BookmarkList identisch
    if( rpExchangeList && bListIdentical )
    {
        String* pString = (String*) rpExchangeList->First();
        while( pString )
        {
            delete pString;
            pString = (String*) rpExchangeList->Next();
        }
        delete rpExchangeList;
        rpExchangeList = NULL;
    }

    return( bNameOK );
}

void ImplProcessObjectList(SdrObject* pObj, SdrObjectVector& rVector )
{
    const bool bIsGroup(pObj->getChildrenOfSdrObject());
    rVector.push_back( pObj );

    if(bIsGroup)
    {
        SdrObjList* pObjList = pObj->getChildrenOfSdrObject();
        sal_uInt32 a;
        for( a = 0; a < pObjList->GetObjCount(); a++)
            ImplProcessObjectList(pObjList->GetObj(a), rVector);
    }
}

SdrModel* View::GetMarkedObjModel() const
{
    return FmFormView::GetMarkedObjModel();
}

bool View::Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pLst /* =NULL */, sal_uInt32 nOptions /* =0 */)
{
    return FmFormView::Paste( rMod, rPos, pLst,nOptions );
}

} // end of namespace sd
