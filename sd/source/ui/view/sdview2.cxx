/*************************************************************************
 *
 *  $RCSfile: sdview2.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:22:21 $
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

#pragma hdrstop

#include "View.hxx"

#include <vector>

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDETC_HXX //autogen
#include <svx/svdetc.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_XEXCH_HXX //autogen
#include <svx/xexch.hxx>
#endif
//CHINA001 #ifndef _SVX_DLG_NAME_HXX
//CHINA001 #include <svx/dlgname.hxx>
//CHINA001 #endif
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#include <svx/dbexch.hrc>
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

#include "navigatr.hxx"
#include "anminfo.hxx"
#include "strings.hrc"
#include "sdxfer.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_FU_DRAW_HXX
#include "fudraw.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdpage.hxx"
#include "unoaprms.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_SLIDE_VIEW_HXX
#include "SlideView.hxx"
#endif
#include "helpids.h"

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
    USHORT                  mnPage;
    USHORT                  mnLayer;

    SdNavigatorDropEvent (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        USHORT nPage,
        USHORT nLayer )
        : ExecuteDropEvent( rEvt ),
          mrTargetHelper( rTargetHelper ),
          mpTargetWindow( pTargetWindow ),
          mnPage( nPage ),
          mnLayer( nLayer )
    {}
};

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateClipboardDataObject( View* pWorkView, ::Window& rWindow )
{
    // since SdTransferable::CopyToClipboard is called, this
    // dynamically created object ist destroyed automatically
    SdTransferable* pTransferable = new SdTransferable( pDoc, NULL, FALSE );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferClip = pTransferable;

    pDoc->CreatingDataObj( pTransferable );
    pTransferable->SetWorkDocument( (SdDrawDocument*) GetAllMarkedModel() );
    pDoc->CreatingDataObj( NULL );

    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = NULL;
    SdrPageView*                    pPgView = GetPageViewPvNum( 0 );
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

        if( pObj && pObj->ISA(SdrOle2Obj) && ((SdrOle2Obj*) pObj)->GetObjRef().Is() )
            pSdrOleObj = (SdrOle2Obj*) pObj;
    }

    if( pSdrOleObj )
        pSdrOleObj->GetObjRef()->FillTransferableObjectDescriptor( aObjDesc );
    else
        pTransferable->GetWorkDocument()->GetDocSh()->FillTransferableObjectDescriptor( aObjDesc );

    if( pDocSh )
        aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    aObjDesc.maSize = aMarkRect.GetSize();

    pTransferable->SetStartPos( aMarkRect.TopLeft() );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToClipboard( pViewSh->GetActiveWindow() );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateDragDataObject( View* pWorkView, ::Window& rWindow, const Point& rDragPos )
{
    SdTransferable* pTransferable = new SdTransferable( pDoc, pWorkView, FALSE );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferDrag = pTransferable;

    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = NULL;

    if( GetMarkedObjectCount() == 1 )
    {
        SdrObject* pObj = GetMarkedObjectByIndex( 0 );

        if( pObj && pObj->ISA( SdrOle2Obj ) && ( (SdrOle2Obj*) pObj )->GetObjRef().Is() )
            pSdrOleObj = (SdrOle2Obj*) pObj;
    }

    if( pDocSh )
        aDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    if( pSdrOleObj )
        pSdrOleObj->GetObjRef()->FillTransferableObjectDescriptor( aObjDesc );
    else
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );

    aObjDesc.maSize = GetAllMarkedRect().GetSize();
    aObjDesc.maDragStartPos = rDragPos;
    aObjDesc.maDisplayName = aDisplayName;
    aObjDesc.mbCanLink = FALSE;

    pTransferable->SetStartPos( rDragPos );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->StartDrag( &rWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );

    return xRet;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > View::CreateSelectionDataObject( View* pWorkView, ::Window& rWindow )
{
    SdTransferable*                 pTransferable = new SdTransferable( pDoc, pWorkView, TRUE );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );
    TransferableObjectDescriptor    aObjDesc;
    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    String                          aDisplayName;

    SD_MOD()->pTransferSelection = pTransferable;

    if( pDocSh )
    {
        aDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    }

    aObjDesc.maSize = aMarkRect.GetSize();

    pTransferable->SetStartPos( aMarkRect.TopLeft() );
    pTransferable->SetObjectDescriptor( aObjDesc );
    pTransferable->CopyToSelection( &rWindow );

    return xRet;
}

// -----------------------------------------------------------------------------

void View::UpdateSelectionClipboard( BOOL bForceDeselect )
{
    if( pViewSh && pViewSh->GetActiveWindow() )
    {
        if( !bForceDeselect && GetMarkedObjectList().GetMarkCount() )
            CreateSelectionDataObject( this, *pViewSh->GetActiveWindow() );
        else if( SD_MOD()->pTransferSelection && ( SD_MOD()->pTransferSelection->GetView() == this ) )
        {
            TransferableHelper::ClearSelection( pViewSh->GetActiveWindow() );
            SD_MOD()->pTransferSelection = NULL;
        }
    }
}

// -----------------------------------------------------------------------------

void View::DoCut (::Window* pWindow)
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
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
    {
        ( (OutlinerView*) pOLV)->PasteSpecial();

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
                    BOOL bOldUpdateMode = pOutliner->GetUpdateMode();

                    pOutliner->SetUpdateMode( FALSE );

                    const EditEngine& rEdit = pOutliner->GetEditEngine();
                    const int nParaCount = rEdit.GetParagraphCount();

                    for( int nPara = nParaCount - 2; nPara >= 0; nPara-- )
                    {
                        const int nParaLen = rEdit.GetTextLen( nPara );
                        pOutliner->QuickDelete( ESelection( nPara, nParaLen, nPara+1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( nPara, nParaLen, nPara, nParaLen ) );
                    }

                    DBG_ASSERT( rEdit.GetParagraphCount() <= 1, "Titelobjekt contains hard line breaks" );
                    pOutliner->SetUpdateMode(bOldUpdateMode);
                }
            }

            if( !pDoc->IsChanged() )
            {
                if( pOutliner && pOutliner->IsModified() )
                    pDoc->SetChanged( TRUE );
            }
        }
    }
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewSh->GetActiveWindow() ) );

        if( aDataHelper.GetTransferable().is() )
        {
            Point       aPos;
            sal_Int8    nDnDAction = DND_ACTION_COPY;

            if( pWindow )
                aPos = pWindow->PixelToLogic( Rectangle( aPos, pWindow->GetOutputSizePixel() ).Center() );

            if( !InsertData( aDataHelper, aPos, nDnDAction, FALSE ) )
            {
                DrawViewShell* pDrViewSh = (DrawViewShell*) pDocSh->GetViewShell();

                if( pDrViewSh )
                {
                    String          aEmptyStr;
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
}

// -----------------------------------------------------------------------------

void View::StartDrag( const Point& rStartPos, Window* pWindow )
{
    if( AreObjectsMarked() && IsAction() && pViewSh && pWindow && !pDragSrcMarkList )
    {
        BrkAction();

        if( IsTextEdit() )
            EndTextEdit();

        ViewShell* pViewShell= pDocSh->GetViewShell();

        if( pViewShell && pViewShell->ISA( DrawViewShell ) )
        {
            FuPoor* pFunc = ( (DrawViewShell*) pViewShell )->GetActualFunction();

            if( pFunc && pFunc->ISA( FuDraw ) )
                ( (FuDraw*) pFunc)->ForcePointer( NULL );
        }

        pDragSrcMarkList = new SdrMarkList(GetMarkedObjectList());
        nDragSrcPgNum = GetPageViewPvNum(0)->GetPage()->GetPageNum();

        String aStr( SdResId(STR_UNDO_DRAGDROP) );
        aStr += sal_Unicode(' ');
        aStr += pDragSrcMarkList->GetMarkDescription();
        BegUndo(aStr);
        CreateDragDataObject( this, *pWindow, rStartPos );
    }
}

// -----------------------------------------------------------------------------

void View::DragFinished( sal_Int8 nDropAction )
{
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if( pDragTransferable )
        pDragTransferable->SetView( NULL );

    if( ( nDropAction & DND_ACTION_MOVE ) &&
        pDragTransferable && !pDragTransferable->IsInternalMove() &&
        pDragSrcMarkList && pDragSrcMarkList->GetMarkCount() &&
        !IsPresObjSelected() )
    {
        pDragSrcMarkList->ForceSort();
        BegUndo();

        ULONG nm, nAnz = pDragSrcMarkList->GetMarkCount();

         for( nm = nAnz; nm>0; )
        {
            nm--;
            SdrMark* pM=pDragSrcMarkList->GetMark(nm);
            AddUndo(new SdrUndoDelObj(*pM->GetObj()));
        }

        pDragSrcMarkList->GetMark(0)->GetObj()->GetOrdNum();

        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=pDragSrcMarkList->GetMark(nm);
            SdrObject* pObj=pM->GetObj();
            UINT32 nOrdNum=pObj->GetOrdNumDirect();

            if( pObj && pObj->GetPage() )
            {
                SdrObject* pChkObj = pObj->GetPage()->RemoveObject(nOrdNum);
                DBG_ASSERT(pChkObj==pObj,"pChkObj!=pObj beim RemoveObject()");
            }
        }

        EndUndo();
    }

    if( pDragTransferable )
        pDragTransferable->SetInternalMove( FALSE );

    EndUndo();
    nDragSrcPgNum = SDRPAGE_NOTFOUND;
    delete pDragSrcMarkList, pDragSrcMarkList = NULL;
}

// -----------------------------------------------------------------------------

sal_Int8 View::AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                             ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer )
{
    String          aLayerName( GetActiveLayer() );
    SdrPageView*    pPV = GetPageViewPvNum(0);
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    if( nLayer != SDRLAYER_NOTFOUND )
    {
        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        aLayerName = rLayerAdmin.GetLayerPerID(nLayer)->GetName();
    }

    if( bIsDropAllowed && !pPV->IsLayerLocked( aLayerName ) && pPV->IsLayerVisible( aLayerName ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        BOOL                bIsInsideOutlinerView = FALSE;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if (GetMarkedObjectCount() == 1)
            {
                SdrMark* pMark = GetSdrMarkByIndex(0);
                SdrObject* pObj = pMark->GetObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            if( aRect.IsInside( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) ) )
            {
                bIsInsideOutlinerView = TRUE;
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
                const BOOL  bDrawing = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING );
                const BOOL  bGraphic = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB );
                const BOOL  bMtf = rTargetHelper.IsDropFormatSupported( FORMAT_GDIMETAFILE );
                const BOOL  bBitmap = rTargetHelper.IsDropFormatSupported( FORMAT_BITMAP );
                BOOL        bBookmark = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                BOOL        bXFillExchange = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_XFA );

                // check handle insert
                if( !nRet && ( bXFillExchange && ( SDRDRAG_GRADIENT == GetDragMode() ) || ( SDRDRAG_TRANSPARENCE == GetDragMode() ) ) )
                {
                    const SdrHdlList& rHdlList = GetHdlList();

                    for( sal_uInt32 n = 0; n < rHdlList.GetHdlCount(); n++ )
                    {
                        SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                        if( pIAOHandle && ( HDL_COLR == pIAOHandle->GetKind() ) )
                        {
                            const B2dIAOGroup& rIAOGroup = pIAOHandle->GetIAOGroup();

                            if( rIAOGroup.IsHit( rEvt.maPosPixel ) )
                            {
                                nRet = nDropAction;
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetSize( SDR_HANDLE_COLOR_SIZE_SELECTED );
                            }
                            else
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetSize( SDR_HANDLE_COLOR_SIZE_NORMAL );
                        }
                    }

                    RefreshAllIAOManagers();
                }

                // check object insert
                if( !nRet && ( bXFillExchange || ( ( bDrawing || bGraphic || bMtf || bBitmap || bBookmark ) && ( nDropAction & DND_ACTION_LINK ) ) ) )
                {
                    SdrObject*      pPickObj = NULL;
                    SdrPageView*    pPV = NULL;
                    ::sd::Window* pWindow = pViewSh->GetActiveWindow();
                    USHORT nHitLog = (USHORT) pWindow->PixelToLogic(
                        Size(FuPoor::HITPIX, 0 ) ).Width();
                    Point           aPos( pWindow->PixelToLogic( rEvt.maPosPixel ) );
                    const BOOL      bHasPickObj = PickObj( aPos, pPickObj, pPV );
                    BOOL            bIsPresTarget = FALSE;

                    if( bHasPickObj && pPickObj && ( pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall() ) )
                    {
                        SdPage* pPage = (SdPage*) pPickObj->GetPage();

                        if( pPage && pPage->IsMasterPage() )
                            bIsPresTarget = pPage->IsPresObj( pPickObj );
                    }

                    if( bHasPickObj && !bIsPresTarget &&
                        ( !pPickObj->ISA( SdrGrafObj ) || bGraphic || bMtf || bBitmap || ( bXFillExchange && !pPickObj->ISA( SdrGrafObj ) && !pPickObj->ISA( SdrOle2Obj ) ) ) )
                    {
                        if( !pDropMarker )
                            pDropMarker = new SdrViewUserMarker(this);

                        if( pDropMarkerObj != pPickObj )
                        {
                            pDropMarkerObj = pPickObj;
                            pDropMarker->SetXPolyPolygon( pDropMarkerObj, GetPageViewPvNum( 0 ) );
                            pDropMarker->Show();
                        }

                        nRet = nDropAction;
                    }
                    else
                        bXFillExchange = FALSE;
                }

                // check normal insert
                if( !nRet )
                {
                    const BOOL  bSBAFormat = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH );
                    const BOOL  bEditEngine = rTargetHelper.IsDropFormatSupported( SOT_FORMATSTR_ID_EDITENGINE );
                    const BOOL  bString = rTargetHelper.IsDropFormatSupported( FORMAT_STRING );
                    const BOOL  bRTF = rTargetHelper.IsDropFormatSupported( FORMAT_RTF );
                    const BOOL  bFile = rTargetHelper.IsDropFormatSupported( FORMAT_FILE );

                    if( pDropMarker )
                    {
                        pDropMarker->Hide();
                        pDropMarkerObj = NULL;
                    }

                    if( bBookmark && bFile && ( nDropAction & DND_ACTION_MOVE ) && ( !pViewSh || pViewSh->GetSlideShow() ) )
                        bBookmark = FALSE;

                    if( bDrawing || bGraphic || bMtf || bBitmap || bBookmark || bFile || bXFillExchange || bSBAFormat || bEditEngine || bString || bRTF )
                        nRet = nDropAction;
                }
            }
        }
    }

    // destroy drop marker if this is a leaving event
    if( rEvt.mbLeaving && pDropMarker )
    {
        pDropMarker->Hide();
        delete pDropMarker, pDropMarker = NULL;
        pDropMarkerObj = NULL;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

sal_Int8 View::ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                              ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer )
{
    SdrPageView*    pPV = GetPageViewPvNum( 0 );
    String          aActiveLayer = GetActiveLayer();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    // destroy drop marker if it is shown
    if( pDropMarker )
    {
        pDropMarker->Hide();
        delete pDropMarker, pDropMarker = NULL;
        pDropMarkerObj = NULL;
    }

    if( !pPV->IsLayerLocked( aActiveLayer ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        BOOL                bIsInsideOutlinerView = FALSE;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if( GetMarkedObjectCount() == 1 )
            {
                SdrMark* pMark = GetSdrMarkByIndex(0);
                SdrObject* pObj = pMark->GetObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            Point aPos( pOLV->GetWindow()->PixelToLogic( rEvt.maPosPixel ) );

            if( aRect.IsInside( aPos ) )
            {
                bIsInsideOutlinerView = TRUE;
            }
        }

        if( !bIsInsideOutlinerView )
        {
            Point                   aPos;
            DrawViewShell*      pDrViewSh = (DrawViewShell*) pDocSh->GetViewShell();
            SdrPage*                pPage = NULL;
            TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );

            if( pTargetWindow )
                aPos = pTargetWindow->PixelToLogic( rEvt.maPosPixel );

            // handle insert?
            if( !nRet && ( SDRDRAG_GRADIENT == GetDragMode() ) || ( SDRDRAG_TRANSPARENCE == GetDragMode() ) && aDataHelper.HasFormat( SOT_FORMATSTR_ID_XFA ) )
            {
                const SdrHdlList& rHdlList = GetHdlList();

                for( sal_uInt32 n = 0; !nRet && n < rHdlList.GetHdlCount(); n++ )
                {
                    SdrHdl* pIAOHandle = rHdlList.GetHdl( n );

                    if( pIAOHandle && ( HDL_COLR == pIAOHandle->GetKind() ) )
                    {
                        const B2dIAOGroup& rIAOGroup = pIAOHandle->GetIAOGroup();

                        if( rIAOGroup.IsHit( rEvt.maPosPixel ) )
                        {
                            SotStorageStreamRef xStm;

                            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) && xStm.Is() )
                            {
                                XFillExchangeData aFillData( XFillAttrSetItem( &pDoc->GetPool() ) );

                                *xStm >> aFillData;
                                const Color aColor( ( (XFillColorItem&) aFillData.GetXFillAttrSetItem()->GetItemSet().Get( XATTR_FILLCOLOR ) ).GetValue() );
                                static_cast< SdrHdlColor* >( pIAOHandle )->SetColor( aColor, TRUE );
                                nRet = nDropAction;
                            }
                        }
                    }
                }
            }

            // standard insert?
            if( !nRet && InsertData( aDataHelper, aPos, nDropAction, TRUE, 0, nPage, nLayer ) )
                nRet = nDropAction;

            // special insert?
            if( !nRet && pViewSh )
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
                        SdrPageView*    pPV = NULL;
                        ::sd::Window* pWindow = pViewSh->GetActiveWindow();
                        USHORT nHitLog = USHORT(pWindow->PixelToLogic(
                            Size(FuPoor::HITPIX,0)).Width());

                        if( PickObj( aPos, pPickObj, pPV ) )
                        {
                            // insert as clip action => jump
                            String              aBookmark( aINetBookmark.GetURL() );
                            SdAnimationInfo*    pInfo = pDoc->GetAnimationInfo( pPickObj );
                            BOOL                bCreated = FALSE;

                            if( aBookmark.Len() )
                            {
                                presentation::ClickAction eClickAction = presentation::ClickAction_DOCUMENT;
                                String aDocName( aBookmark.GetToken( 0, '#' ) );

                                if( pDocSh->GetMedium()->GetName() == aDocName || pDocSh->GetName() == aDocName )
                                {
                                    // Interner Sprung -> nur "#Bookmark" verwenden
                                    aBookmark = aBookmark.GetToken( 1, '#' );
                                    eClickAction = presentation::ClickAction_BOOKMARK;
                                }

                                if( !pInfo )
                                {
                                    pInfo = new SdAnimationInfo( pDoc );
                                    pPickObj->InsertUserData( pInfo );
                                    bCreated = TRUE;
                                }

                                // Undo-Action mit alten und neuen Groessen erzeugen
                                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction(pDoc, pPickObj, bCreated);
                                pAction->SetActive(pInfo->bActive, pInfo->bActive);
                                pAction->SetEffect(pInfo->eEffect, pInfo->eEffect);
                                pAction->SetTextEffect(pInfo->eTextEffect, pInfo->eTextEffect);
                                pAction->SetSpeed(pInfo->eSpeed, pInfo->eSpeed);
                                pAction->SetDim(pInfo->bDimPrevious, pInfo->bDimPrevious);
                                pAction->SetDimColor(pInfo->aDimColor, pInfo->aDimColor);
                                pAction->SetDimHide(pInfo->bDimHide, pInfo->bDimHide);
                                pAction->SetSoundOn(pInfo->bSoundOn, pInfo->bSoundOn);
                                pAction->SetSound(pInfo->aSoundFile, pInfo->aSoundFile);
                                pAction->SetPlayFull(pInfo->bPlayFull, pInfo->bPlayFull);
                                pAction->SetPathObj(pInfo->pPathObj, pInfo->pPathObj);
                                pAction->SetClickAction(pInfo->eClickAction, eClickAction);
                                pAction->SetBookmark(pInfo->aBookmark, aBookmark);
                                pAction->SetInvisibleInPres(pInfo->bInvisibleInPresentation, TRUE);
                                pAction->SetVerb(pInfo->nVerb, pInfo->nVerb);
                                pAction->SetSecondEffect(pInfo->eSecondEffect, pInfo->eSecondEffect);
                                pAction->SetSecondSpeed(pInfo->eSecondSpeed, pInfo->eSecondSpeed);
                                pAction->SetSecondSoundOn(pInfo->bSecondSoundOn, pInfo->bSecondSoundOn);
                                pAction->SetSecondPlayFull(pInfo->bSecondPlayFull, pInfo->bSecondPlayFull);

                                String aString(SdResId(STR_UNDO_ANIMATION));
                                pAction->SetComment(aString);
                                pDocSh->GetUndoManager()->AddUndoAction(pAction);
                                pInfo->eClickAction = eClickAction;
                                pInfo->aBookmark = aBookmark;
                                pDoc->SetChanged();

                                nRet = nDropAction;
                            }
                        }
                        else if( pViewSh->ISA( DrawViewShell ) )
                        {
                            // insert as normal URL button
                            ( (DrawViewShell*) pViewSh )->InsertURLButton( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), String(), &aPos );
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
        List    aBookmarkList;
        String  aBookmark;
        String  aFile;
        SdPage* pPage = (SdPage*) GetPageViewPvNum( 0 )->GetPage();
        USHORT  nPgPos = 0xFFFF;

        if( pSdNavigatorDropEvent->mpTargetWindow )
            aPos = pSdNavigatorDropEvent->mpTargetWindow->PixelToLogic( pSdNavigatorDropEvent->maPosPixel );

        aFile = aINetBookmark.GetURL().GetToken( 0, '#' );
        aBookmark = aINetBookmark.GetURL().GetToken( 1, '#' );
        aBookmarkList.Insert( &aBookmark );

        if( !pPage->IsMasterPage() )
        {
            if( pPage->GetPageKind() == PK_STANDARD )
                nPgPos = pPage->GetPageNum() + 2;
            else if( pPage->GetPageKind() == PK_NOTES )
                nPgPos = pPage->GetPageNum() + 1;
        }

        // Um zu gewaehrleisten, dass alle Seitennamen eindeutig sind, werden
        // die einzufuegenden geprueft und gegebenenfalls in einer Ersatzliste
        // aufgenommen (bNameOK == FALSE -> Benutzer hat abgebrochen)
        List*   pExchangeList = NULL;
        BOOL    bLink = ( NAVIGATOR_DRAGTYPE_LINK == pPageObjsTransferable->GetDragType()  ? TRUE : FALSE );
        BOOL    bNameOK = GetExchangeList( pExchangeList, &aBookmarkList, 2 );
        BOOL    bReplace = FALSE;

        // Da man hier nicht weiss, ob es sich um eine Seite oder ein Objekt handelt,
        // wird eine Liste sowohl mit Seiten, als auch mit Objekten gefuellt.
        // Sollten Seitennamen und Objektnamen identisch sein gibt es hier natuerlich Probleme !!!
        if( bNameOK )
        {
            pDoc->InsertBookmark( &aBookmarkList, pExchangeList,
                                  bLink, bReplace, nPgPos, FALSE,
                                  &pPageObjsTransferable->GetDocShell(),
                                  TRUE, &aPos );
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
|* bNameOK == FALSE -> Benutzer hat abgebrochen
|* nType == 0 -> Seiten
|* nType == 1 -> Objekte
|* nType == 2 -> Seiten + Objekte
|*
\************************************************************************/

BOOL View::GetExchangeList( List*& rpExchangeList, List* pBookmarkList, USHORT nType )
{
    DBG_ASSERT( !rpExchangeList, "ExchangeList muss NULL sein!");

    BOOL bListIdentical = TRUE; // BookmarkList und ExchangeList sind gleich
    BOOL bNameOK = TRUE;        // Name ist eindeutig

    rpExchangeList = new List();

    if( pBookmarkList )
    {
        String* pString = (String*) pBookmarkList->First();

        while( pString && bNameOK )
        {
            String* pNewName = new String( *pString );

            if( nType == 0  || nType == 2 )
                bNameOK = pDocSh->CheckPageName (
                    pViewSh->GetWindow(), *pNewName);

            if( bNameOK && ( nType == 1  || nType == 2 ) )
            {
                if( pDoc->GetObj( *pNewName ) )
                {
                    String          aTitle( SdResId( STR_TITLE_NAMEGROUP ) );
                    String          aDesc( SdResId( STR_DESC_NAMEGROUP ) );
                    //CHINA001 SvxNameDialog*  pDlg = new SvxNameDialog( pViewSh->GetWindow(), *pNewName, aDesc );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
                    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( pViewSh->GetWindow(), *pNewName, aDesc, ResId(RID_SVXDLG_NAME) );
                    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                    pDlg->SetEditHelpId( HID_SD_NAMEDIALOG_OBJECT );

                    bNameOK = FALSE;
                    pDlg->SetText( aTitle );

                    while( !bNameOK && pDlg->Execute() == RET_OK )
                    {
                        pDlg->GetName( *pNewName );

                        if( !pDoc->GetObj( *pNewName ) )
                            bNameOK = TRUE;
                    }

                    delete pDlg;
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

typedef std::vector< std::pair< sal_uInt32, sal_uInt32 > > PathSurrogateVector;
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

/** this method restores all connections between shapes with animations along a path and theire path */
SdrModel* View::GetMarkedObjModel() const
{
    // this vector holds the index of all shapes with animations along a path and the index
    // of theire corresponding path. The index is a flat index
    PathSurrogateVector aPathSurro;

    // this is a list with index's of all shapes with animation infos in the source list
    // it is used to later update the nPresOrder counter
    std::vector< std::pair< sal_uInt32, sal_uInt32 > > aAnimObjects;

    {
        // get a flat vector of all shapes inside the mark list first, including deep within groups
        SdrObjectVector aSdrVector;
        const sal_uInt32 nMarkCount = GetMarkedObjectCount();
        sal_uInt32 nMark;
        for(nMark = 0; nMark < nMarkCount; nMark++ )
        {
            ImplProcessObjectList( GetMarkedObjectByIndex( nMark ), aSdrVector );
        }

        // now see if we find any shape with an animation along a path
        sal_uInt32 nObj = 0;
        SdrObjectVector::iterator aIter( aSdrVector.begin() );
        const SdrObjectVector::iterator aEnd( aSdrVector.end() );
        while( aIter != aEnd )
        {
            const SdrObject* pObj = (*aIter++);

            if( pObj )
            {
                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(const_cast<SdrObject*>(pObj));
                if( pInfo)
                {
                    if( pInfo->eEffect == presentation::AnimationEffect_PATH && pInfo->pPathObj)
                    {
                        // we found one, now look for the path
                        SdrObjectVector::iterator aPathIter( aSdrVector.begin() );
                        sal_uInt32 nPath = 0;

                        while( aPathIter != aEnd )
                        {
                            SdrObject* pPathObj = (*aPathIter++);
                            if( pPathObj == pInfo->pPathObj )
                            {
                                // we found the path, so store the indices
                                aPathSurro.push_back( std::pair< sal_uInt32, sal_uInt32 >( nObj, nPath ) );
                                break;
                            }
                            nPath++;
                        }
                    }

                    if( pInfo->nPresOrder != LIST_APPEND )
                    {
                        aAnimObjects.push_back( std::pair< sal_uInt32, sal_uInt32 >( nObj, pInfo->nPresOrder ) );
                    }
                }
            }
            nObj++;
        }
    }

    // let the base class create the model
    SdrModel* pNewModel = FmFormView::GetMarkedObjModel();

    // if we had any shapes with animations along a path...
    if( !aPathSurro.empty() )
    {
        // restore theire connections to the they're path shapes
        SdrPage* pPage = pNewModel->GetPage(0);
        SdrObjListIter  aObjIter( *pPage, IM_DEEPWITHGROUPS );

        PathSurrogateVector::iterator aIter = aPathSurro.begin();
        const PathSurrogateVector::iterator aEnd = aPathSurro.end();

        while( aIter != aEnd )
        {

            sal_uInt32 nObject = (*aIter).first;
            SdrObject* pObject = NULL;
            aObjIter.Reset();
            while( (pObject = aObjIter.Next()) && nObject-- );

            SdrObject* pPath = NULL;
            nObject = (*aIter).second;
            aObjIter.Reset();
            while( (pPath = aObjIter.Next()) && nObject-- );

            if( pObject && pPath )
            {
                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObject);
                if( pInfo == NULL )
                {
                    pInfo = new SdAnimationInfo(pDoc);
                    pObject->InsertUserData( pInfo );
                }

                pInfo->eEffect = presentation::AnimationEffect_PATH;
                pInfo->pPathObj = PTR_CAST( SdrPathObj, pPath );
            }

            aIter++;
        }
    }

    // restore presentation order
    if( !aAnimObjects.empty() )
    {
        sal_uInt32 nCurrent = 0;
        SdrPage* pPage = pNewModel->GetPage(0);
        SdrObjListIter  aObjIter( *pPage, IM_DEEPWITHGROUPS );
        SdrObject* pObj = aObjIter.Next();

        std::vector< std::pair< sal_uInt32, sal_uInt32 > >::iterator aIter( aAnimObjects.begin() );
        std::vector< std::pair< sal_uInt32, sal_uInt32 > >::iterator aEnd( aAnimObjects.end() );
        while( aIter != aEnd )
        {
            sal_uInt32 nObject = (*aIter).first;

            while( (nCurrent < nObject) && aObjIter.IsMore() )
            {
                nCurrent++;
                pObj = aObjIter.Next();
            }

            DBG_ASSERT( nCurrent == nObject, "wrong shape count after paste!" );
            if( nCurrent != nObject )
                break;

            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
            DBG_ASSERT( pInfo, "shape that should have an animation info hasn't after paste!" );

            if( pInfo )
                pInfo->nPresOrder = (*aIter).second;

            aIter++;
        }
    }

    return pNewModel;
}

/** this method restores all connections between shapes with animations along a path and theire path */
BOOL View::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst /* =NULL */, UINT32 nOptions /* =0 */)
{
    // this vector holds the index of all shapes with animations along a path and the index
    // of theire corresponding path. The index is a flat index
    PathSurrogateVector aPathSurro;

    // this is a list with index's of all shapes with animation infos in the source list
    // it is used to later update the nPresOrder counter
    std::vector< std::pair< sal_uInt32, sal_uInt32 > > aAnimObjects;

    // get the destination object list or page
    SdrObjList* pDstList = pLst;
    if( NULL == pDstList )
    {
        SdrPageView* pPV=GetPageView(rPos);
        if (pPV!=NULL)
            pDstList=pPV->GetObjList();
    }

    sal_uInt32 nMaxPresOrder = 0;

    sal_uInt32 nDstObjCount = 0;
    if( pDstList )
    {
        // first we need the flat count of shapes already in the destination
        // object list, so we can have the absolut index of newly created shapes
        {
            SdrObjListIter  aIter( *pDstList, IM_DEEPWITHGROUPS );
            while( aIter.IsMore() )
            {
                SdrObject* pObj = aIter.Next();
                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(const_cast<SdrObject*>(pObj));
                if( pInfo && (pInfo->nPresOrder != LIST_APPEND) && (pInfo->nPresOrder > nMaxPresOrder) )
                    nMaxPresOrder = pInfo->nPresOrder;

                nDstObjCount++;
            }
        }

        // now find all shapes with an animation at path set and theire path shape
        sal_uInt32 nPageCount = rMod.GetPageCount();
        sal_uInt32 nPage;
        for( nPage=0; nPage < nPageCount; nPage++ )
        {
            const SdrPage* pPage = rMod.GetPage( 0 );
            if( pPage )
            {
                SdrObjListIter  aIter( *pPage, IM_DEEPWITHGROUPS );

                const sal_uInt32 nObjCount = pPage->GetObjCount();
                sal_uInt32 nObj;
                SdrObject* pObj;
                for( pObj = aIter.Next(), nObj = 0; pObj; pObj = aIter.Next(), nObj++ )
                {
                    SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(const_cast<SdrObject*>(pObj));
                    if( pInfo )
                    {
                        if( pInfo->eEffect == presentation::AnimationEffect_PATH && pInfo->pPathObj)
                        {
                            SdrObjListIter  aPathIter( *pPage, IM_DEEPWITHGROUPS );
                            sal_uInt32 nPath;
                            SdrObject* pPath;
                            for( pPath = aPathIter.Next(), nPath = 0; pPath; pPath = aPathIter.Next(), nPath++ )
                            {
                                if( pPath == pInfo->pPathObj )
                                {
                                    aPathSurro.push_back( std::pair< sal_uInt32, sal_uInt32 >( nDstObjCount + nObj, nDstObjCount + nPath ) );
                                    break;
                                }
                            }
                        }

                        if( pInfo->nPresOrder != LIST_APPEND )
                        {
                            const sal_uInt32 nNewPresOrder = pInfo->nPresOrder + 1 + nMaxPresOrder;
                            aAnimObjects.push_back( std::pair< sal_uInt32, sal_uInt32 >( nDstObjCount + nObj, nNewPresOrder ) );
                        }
                    }
                }
            }
        }
    }

    BOOL bRet = FmFormView::Paste( rMod, rPos, pLst,nOptions );

    if( bRet && !aPathSurro.empty() )
    {
        // restore all shapes with animation at path and theire path connections
        PathSurrogateVector::iterator aIter = aPathSurro.begin();
        const PathSurrogateVector::iterator aEnd = aPathSurro.end();

        SdrObjListIter  aObjIter( *pDstList, IM_DEEPWITHGROUPS );

        while( aIter != aEnd )
        {
            sal_uInt32 nObject = (*aIter).first;
            SdrObject* pObject = NULL;
            aObjIter.Reset();
            while( (pObject = aObjIter.Next()) && nObject-- );

            SdrObject* pPath = NULL;
            nObject = (*aIter).second;
            aObjIter.Reset();
            while( (pPath = aObjIter.Next()) && nObject-- );

            if( pObject && PTR_CAST( SdrPathObj, pPath ) )
            {
                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObject);
                if( pInfo == NULL )
                {
                    pInfo = new SdAnimationInfo(pDoc);
                    pObject->InsertUserData( pInfo );
                }

                pInfo->eEffect = presentation::AnimationEffect_PATH;
                pInfo->pPathObj = PTR_CAST(SdrPathObj, pPath );
            }

            aIter++;
        }
    }

    if( bRet && !aAnimObjects.empty() )
    {
        sal_uInt32 nCurrent = 0;
        SdrObjListIter  aObjIter( *pDstList, IM_DEEPWITHGROUPS );
        SdrObject* pObj = aObjIter.Next();

        std::vector< std::pair< sal_uInt32, sal_uInt32 > >::iterator aIter( aAnimObjects.begin() );
        std::vector< std::pair< sal_uInt32, sal_uInt32 > >::iterator aEnd( aAnimObjects.end() );
        while( aIter != aEnd )
        {
            sal_uInt32 nObject = (*aIter).first;

            while( (nCurrent < nObject) && aObjIter.IsMore() )
            {
                nCurrent++;
                pObj = aObjIter.Next();
            }

            DBG_ASSERT( nCurrent == nObject, "wrong shape count after paste!" );
            if( nCurrent != nObject )
                break;

            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
            DBG_ASSERT( pInfo, "shape that should have an animation info hasn't after paste!" );

            if( pInfo )
                pInfo->nPresOrder = (*aIter).second;

            aIter++;
        }
    }

    return bRet;
}

} // end of namespace sd
