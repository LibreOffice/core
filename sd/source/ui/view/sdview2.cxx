/*************************************************************************
 *
 *  $RCSfile: sdview2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-08 11:24:32 $
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


#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
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
#include <svx/dbexch.hrc>
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif

#include "navigatr.hxx"
#include "anminfo.hxx"
#include "strings.hrc"
#include "sdview.hxx"
#include "sdxfer.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "drviewsh.hxx"
#include "docshell.hxx"
#include "fudraw.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "sdpage.hxx"
#include "unoaprms.hxx"
#include "drawview.hxx"
#include "slidview.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

using namespace ::com::sun::star;

/*************************************************************************
|*
|* DataObject fuer Drag&Drop erzeugen
|*
\************************************************************************/

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SdView::CreateDataObject( SdView* pWorkView, Window& rWindow,
                                                                                                            const Point& rDragPos )
{
    SdTransferable* pTransferable = new SdTransferable( pDoc, pWorkView );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferDrag = pTransferable;

    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = NULL;

    if( aMark.GetMarkCount() == 1 )
    {
        SdrObject* pObj = aMark.GetMark( 0 )->GetObj();

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


/*************************************************************************
|*
|* DataObject fuers Clipboard erzeugen
|*
\************************************************************************/

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SdView::CreateDataObject()
{
    // since SdTransferable::CopyToClipboard is called, this
    // dynamically created object ist destroyed automatically
    SdTransferable* pTransferable = new SdTransferable( pDoc );
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRet( pTransferable );

    SD_MOD()->pTransferClip = pTransferable;

    pDoc->CreatingDataObj( TRUE );
    pTransferable->SetWorkDocument( (SdDrawDocument*) GetAllMarkedModel() );
    pDoc->CreatingDataObj( FALSE );

    const Rectangle                 aMarkRect( GetAllMarkedRect() );
    TransferableObjectDescriptor    aObjDesc;
    String                          aDisplayName;
    SdrOle2Obj*                     pSdrOleObj = NULL;
    SdrPageView*                    pPgView = GetPageViewPvNum( 0 );
    SdPage*                         pPage = (SdPage*) pPgView->GetPage();
    SdPage*                         pNewPage = (SdPage*) pTransferable->GetWorkDocument()->GetPage(0);

    pNewPage->SetSize( pPage->GetSize() );
    pNewPage->SetLayoutName( pPage->GetLayoutName() );

    if( aMark.GetMarkCount() == 1 )
    {
        SdrObject* pObj = aMark.GetMark(0)->GetObj();

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
    pTransferable->CopyToClipboard();

    return xRet;
}


/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void __EXPORT SdView::DoCut(Window* pWindow)
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        ( (OutlinerView*) pOLV)->Cut();
    else if( HasMarkedObj() )
    {
        String aStr( SdResId(STR_UNDO_CUT) );

        DoCopy();
        BegUndo( ( aStr += sal_Unicode(' ') ) += aMark.GetMarkDescription() );
        DeleteMarked();
        EndUndo();
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void __EXPORT SdView::DoCopy(Window* pWindow)
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
        ( (OutlinerView*) pOLV)->Copy();
    else if( HasMarkedObj() )
    {
        BrkAction();
        CreateDataObject();
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void __EXPORT SdView::DoPaste( Window* pWindow )
{
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if( pOLV )
    {
        ( (OutlinerView*) pOLV)->PasteSpecial();

        SdrObject*  pObj = GetTextEditObject();
        SdPage*     pPage = (SdPage*)( pObj ? pObj->GetPage() : NULL );
        Outliner*   pOutliner = pOLV->GetOutliner();

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
        Point           aPos;
        BOOL            bPagesInserted = FALSE;
        SdTransferable* pTransferClip = SD_MOD()->pTransferClip;

        if( pWindow )
            aPos = pWindow->PixelToLogic( Rectangle( aPos, pWindow->GetOutputSizePixel() ).Center() );

        if( pTransferClip && pTransferClip->GetDocShell() )
        {
            // Eigenes Format: Ganze Seiten einfuegen?
            SvEmbeddedObject*   pObj = pTransferClip->GetDocShell();
            SdDrawDocShell*     pDataDocSh = (SdDrawDocShell*) pObj;
            SdDrawDocument*     pDataDoc = pDataDocSh->GetDoc();

            if( pDataDoc && pDataDoc->GetPageCount() > 1 )
            {
                // Dokument hat mehrere Seiten -> Seiten einfuegen
                bPagesInserted = TRUE;

                USHORT  nInsertPgCnt = pDataDoc->GetSdPageCount(PK_STANDARD);
                USHORT  nInsertPos = pDoc->GetSdPageCount(PK_STANDARD) * 2 + 1;
                USHORT  nPgCnt = pDoc->GetSdPageCount(PK_STANDARD);
                BOOL    bMergeMasterPages = TRUE;

                for( USHORT nPage = 0; nPage < nPgCnt; nPage++ )
                {
                    SdPage* pPage = pDoc->GetSdPage( nPage, PK_STANDARD );

                    if( pPage->IsSelected() )
                        nInsertPos = nPage * 2 + 3;
                }

                if( pTransferClip->HasSourceDoc( pDoc ) )
                    bMergeMasterPages = FALSE;

                pDoc->InsertBookmarkAsPage( NULL, NULL, FALSE, FALSE, nInsertPos,
                                            FALSE, pDataDocSh, TRUE, bMergeMasterPages );

                if( this->ISA( SdSlideView ) )
                {
                    // Alle Seiten deselektieren
                    for( USHORT nPage = 0, nPgCnt = pDoc->GetSdPageCount( PK_STANDARD ); nPage < nPgCnt; nPage++ )
                        pDoc->GetSdPage( nPage, PK_STANDARD )->SetSelected( FALSE );

                    // Die letzte eingefuegte Seite selektieren
                    SdPage* pPage = pDoc->GetSdPage( nInsertPos / 2 + nInsertPgCnt - 1, PK_STANDARD );

                    if( pPage )
                        pPage->SetSelected( TRUE );
                }
            }
        }

        if( !bPagesInserted && this->ISA( SdDrawView ) )
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard() );

            if( aDataHelper.GetTransferable().is() )
            {
                sal_Int8 nDnDAction = DND_ACTION_COPY;

                if( !InsertData( aDataHelper.GetTransferable(), aPos, nDnDAction, FALSE ) )
                {
                    DBG_ERROR( "INetBookmark not supported" );
/* !!!
                    SdDrawViewShell* pDrViewSh = (SdDrawViewShell*) pDocSh->GetViewShell();

                    if( pDrViewSh )
                    {
                        String          aEmptyStr;
                        INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );
                        BOOL            bPasted = FALSE;

                        if( aINetBookmark.Paste( *aDataObj, SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) )
                            bPasted = TRUE;
                        else if ( aINetBookmark.Paste( *aDataObj, SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) )
                            bPasted = TRUE;
                        else if( aINetBookmark.Paste( *aDataObj, SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) )
                            bPasted = TRUE;

                        if( bPasted )
                            pDrViewSh->InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, NULL );
                    }
*/
                }
            }
        }
    }
}

/*************************************************************************
|*
|* DragServer starten
|*
\************************************************************************/

BOOL SdView::BeginDrag(Window* pWindow, Point aStartPos)
{
    BOOL bRet = HasMarkedObj() && IsAction() && pViewSh && pWindow;

    if( bRet )
    {
        BrkAction();

        SdViewShell* pViewShell= pDocSh->GetViewShell();

        if( pViewShell && pViewShell->ISA( SdDrawViewShell ) )
        {
            FuPoor* pFunc = ( (SdDrawViewShell*) pViewShell )->GetActualFunction();

            if( pFunc && pFunc->ISA( FuDraw ) )
                ( (FuDraw*) pFunc)->ForcePointer( NULL );
        }

        pDragSrcMarkList = new SdrMarkList( aMark );
        nDragSrcPgNum = GetPageViewPvNum(0)->GetPage()->GetPageNum();

        String aStr( SdResId(STR_UNDO_DRAGDROP) );
        aStr += sal_Unicode(' ');
        aStr += pDragSrcMarkList->GetMarkDescription();
        BegUndo(aStr);
        CreateDataObject( this, *pWindow, aStartPos );
    }

    return bRet;
}

/*************************************************************************
|*
|* DragFinished
|*
\************************************************************************/

void SdView::DragFinished( sal_Int8 nDropAction )
{
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if( pDragTransferable )
        pDragTransferable->SetView( NULL );

    if( ( nDropAction & DND_ACTION_MOVE ) && !pDragTransferable->IsInternalMove() &&
        !IsPresObjSelected() && pDragSrcMarkList && pDragSrcMarkList->GetMarkCount() )
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
            SdrObject* pChkObj = pObj->GetPage()->RemoveObject(nOrdNum);
            DBG_ASSERT(pChkObj==pObj,"DeleteMarked(MarkList): pChkObj!=pObj beim RemoveObject()");
        }

        EndUndo();
    }

    if( pDragTransferable )
        pDragTransferable->SetInternalMove( FALSE );

    EndUndo();
    nDragSrcPgNum = SDRPAGE_NOTFOUND;
    delete pDragSrcMarkList, pDragSrcMarkList = NULL;
}

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 SdView::AcceptDrop( const AcceptDropEvent& rEvt, SdWindow* pWin, USHORT nPage, USHORT nLayer )
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

            if (aMark.GetMarkCount() == 1)
            {
                SdrMark* pMark = aMark.GetMark(0);
                SdrObject* pObj = pMark->GetObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            if( aRect.IsInside( pOLV->GetWindow()->PixelToLogic( rEvt.maPos ) ) )
            {
                bIsInsideOutlinerView = TRUE;
                //!!!DND nRet = ( (OutlinerView*) pOLV )->AcceptDrop( rEvt, pWin, nPage, nLayer );
            }
        }

        if( !bIsInsideOutlinerView )
        {
            SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

            if( pDragTransferable )
            {
                const SdView* pSourceView = pDragTransferable->GetView();

                if( pSourceView )
                {
                    if( !( nDropAction & DND_ACTION_LINK ) || pSourceView->GetDocSh()->GetMedium()->GetName().Len() )
                        nRet = nDropAction;
                }
            }
            else
            {
                BOOL            bDrawing = pWin->IsDropFormatSupported( SOT_FORMATSTR_ID_DRAWING );
                BOOL            bGraphic = pWin->IsDropFormatSupported( SOT_FORMATSTR_ID_SVXB );
                BOOL            bMtf = pWin->IsDropFormatSupported( FORMAT_GDIMETAFILE );
                BOOL            bBitmap = pWin->IsDropFormatSupported( FORMAT_BITMAP );
                BOOL            bBookmark = pWin->IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                BOOL            bXFillExchange = pWin->IsDropFormatSupported( SOT_FORMATSTR_ID_XFA );
                BOOL            bSBAFormat = pWin->IsDropFormatSupported( SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH );
                BOOL            bIsPresTarget = FALSE;
                BOOL            nDefaultDrop = DND_ACTION_NONE;
                //!!!DND BOOL nDefaultDrop = nRet = FmFormView::AcceptDrop( rEvt, pWin, nPage, nLayer );

                nDropAction = ( ( nDropAction & DND_ACTION_MOVE ) ? DND_ACTION_COPY : nDropAction );

                if( !nDefaultDrop && ( ( ( bDrawing || bGraphic || bMtf || bBitmap || bBookmark ) && ( ( nDropAction & DND_ACTION_MOVE ) || ( nDropAction & DND_ACTION_LINK ) ) ) || bXFillExchange ) )
                {
                    SdrObject*      pPickObj = NULL;
                    SdrPageView*    pPV = NULL;
                    SdWindow*       pWindow = pViewSh->GetActiveWindow();
                    USHORT          nHitLog = USHORT( pWindow->PixelToLogic( Size( HITPIX, 0 ) ).Width() );
                    Point           aPos( pWindow->PixelToLogic( rEvt.maPos ) );
                    BOOL            bHasPickObj = PickObj( aPos, pPickObj, pPV );

                    if( bHasPickObj && pPickObj && ( pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall() ) )
                    {
                        SdPage* pPage = (SdPage*) pPickObj->GetPage();

                        if( pPage && pPage->IsMasterPage() )
                            bIsPresTarget = pPage->GetPresObjList()->GetPos( pPickObj ) != LIST_ENTRY_NOTFOUND;
                    }

                    if( bHasPickObj && ( !pPickObj->ISA( SdrGrafObj ) || bGraphic || bMtf || bBitmap ||
                          ( bXFillExchange && !pPickObj->ISA( SdrGrafObj ) && !pPickObj->ISA( SdrOle2Obj ) ) ||
                          ( ( nAction == DND_ACTION_MOVE ) && !bIsPresTarget ) ) )
                    {

                        if( !pDropMarker )
                            pDropMarker = new SdrViewUserMarker(this);

                        if( pDropMarkerObj != pPickObj )
                        {
                            pDropMarkerObj = pPickObj;
                            pDropMarker->SetXPolyPolygon( pDropMarkerObj, GetPageViewPvNum( 0 ) );
                            pDropMarker->Show();
                        }
                    }
                    else
                    {
                        bXFillExchange = FALSE;

                        if( pDropMarker )
                        {
                            pDropMarker->Hide();
                            pDropMarkerObj = NULL;
                        }
                    }
                }
                else
                {
                    if( pDropMarker )
                    {
                        pDropMarker->Hide();
                        pDropMarkerObj = NULL;
                    }
                }


                BOOL bFile = ( pWin->IsDropFormatSupported( FORMAT_FILE ) && pViewSh && !pViewSh->GetSlideShow() && !( nDropAction & DND_ACTION_MOVE ) );

                if( bBookmark && !bFile && pWin->IsDropFormatSupported( FORMAT_FILE ) )
                    bBookmark = FALSE;

                if( IsDragDropFormatSupported() || bFile || bBookmark || bGraphic || bXFillExchange || bSBAFormat || nDefaultDrop )
                    nRet = nDropAction;
            }
        }
    }

    return nRet;
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 SdView::ExecuteDrop( const ExecuteDropEvent& rEvt, SdWindow* pWin, USHORT nPage, USHORT nLayer )
{
    if( pDropMarker )
    {
        pDropMarker->Hide();
        delete pDropMarker, pDropMarker = NULL;
        pDropMarkerObj = NULL;
    }

    SdrPageView*    pPV = GetPageViewPvNum( 0 );
    String          aActiveLayer = GetActiveLayer();
    sal_Int8        nDropAction = rEvt.mnAction;
    sal_Int8        nRet = DND_ACTION_NONE;

    if( !pPV->IsLayerLocked( aActiveLayer ) )
    {
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        BOOL                bIsInsideOutlinerView = FALSE;

        if( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if( aMark.GetMarkCount() == 1 )
            {
                SdrMark* pMark = aMark.GetMark(0);
                SdrObject* pObj = pMark->GetObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            Point aPos = pOLV->GetWindow()->PixelToLogic( rEvt.maPos );

            if( aRect.IsInside( aPos ) )
            {
                bIsInsideOutlinerView = TRUE;
                //!!!DND nRet = ( (OutlinerView*) pOLV)->ExecuteDrop( nDropAction, rPointerPos, nDnDSourceActions, rxTransferable, pWin, nPage, nLayer );
            }
        }

        if( !bIsInsideOutlinerView )
        {
            Point               aPos;
            SdDrawViewShell*    pDrViewSh = (SdDrawViewShell*) pDocSh->GetViewShell();
            SdrPage*            pPage = NULL;
            SdTransferable*     pDragTransferable = SD_MOD()->pTransferDrag;

            if( pWin )
                aPos = pWin->PixelToLogic( rEvt.maPos );

            //!!!DND if( !( bReturn = FmFormView::Drop(rMEvt, pWin) ) ) )
            if( !InsertData( rEvt.maDropEvent.Transferable, aPos, nDropAction, TRUE, 0, nPage, nLayer ) && pViewSh )
            {
                String              aTmpString1, aTmpString2;
                INetBookmark        aINetBookmark( aTmpString1, aTmpString2 );
                const USHORT        nCount = DragServer::GetItemCount();
                SdNavigatorWin*     pNavWin = NULL;
                NavigatorDragType   eDragType = NAVIGATOR_DRAGTYPE_NONE;

                if( pViewSh->GetViewFrame()->HasChildWindow( SID_NAVIGATOR ) )
                    pNavWin = (SdNavigatorWin*)(pViewSh->GetViewFrame()->GetChildWindow( SID_NAVIGATOR )->GetContextWindow( SD_MOD() ) );

                if( pNavWin && pNavWin->GetDropDocSh() )
                    eDragType = pNavWin->GetNavigatorDragType();

                if( eDragType == NAVIGATOR_DRAGTYPE_LINK || eDragType == NAVIGATOR_DRAGTYPE_EMBEDDED )
                {
/*!!!DND for Navigator
                    List    aBookmarkList;
                    String  aBookmark;
                    String  aFile;

                    for( USHORT i = 0; i < nCount; i++ )
                    {
                        // Bookmark-Liste fuellen
                        if( aINetBookmark.PasteDragServer( i ) )
                        {
                            if( i==0 )
                            {
                                aFile = aINetBookmark.GetURL().GetToken(0, '#');
                            }

                            aBookmark = aINetBookmark.GetURL().GetToken(1, '#');
                            aBookmarkList.Insert(&aBookmark);
                        }
                    }

                    SdPage* pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
                    USHORT  nPgPos = 0xFFFF;

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
                    BOOL    bNameOK = GetExchangeList( pExchangeList, &aBookmarkList, 2 );
                    BOOL    bLink = eDragType == NAVIGATOR_DRAGTYPE_LINK ? TRUE : FALSE;
                    BOOL    bReplace = FALSE;

                    // Da man hier nicht weiss, ob es sich um eine Seite oder ein Objekt
                    // handelt, wird eine Liste sowohl mit Seiten, als auch mit Objekten
                    // gefuellt.
                    // Sollten Seitennamen und Objektnamen identisch sein gibt es hier
                    // natuerlich Probleme !!!

                    if( bNameOK )
                        bReturn = pDoc->InsertBookmark(&aBookmarkList, pExchangeList, bLink, bReplace, nPgPos, FALSE, pNavWin->GetDropDocSh(), TRUE, &aPos);

                    // Loeschen der ExchangeList
                    if( pExchangeList )
                    {
                        for( void* p = pExchangeList->First(); p; p = pExchangeList->Next() )
                            delete (String*) p;

                        delete pExchangeList;
                    }
*/
                }
                else
                {
                    SdrObject*      pPickObj = NULL;
                    SdrPageView*    pPV = NULL;
                    SdWindow*       pWindow = pViewSh->GetActiveWindow();
                    USHORT          nHitLog = USHORT(pWindow->PixelToLogic(Size(HITPIX,0)).Width());

                    if( PickObj( aPos, pPickObj, pPV ) )
                    {
                        aINetBookmark.PasteDragServer( 0 );

                        String aBookmark( aINetBookmark.GetURL() );

                        if( aBookmark.Len() )
                        {
                            presentation::ClickAction eClickAction = presentation::ClickAction_DOCUMENT;
                            String aDocName(aBookmark.GetToken(0, '#'));

                            if (pDocSh->GetMedium()->GetName() == aDocName || pDocSh->GetName() == aDocName)
                            {
                                // Interner Sprung -> nur "#Bookmark" verwenden
                                aBookmark = aBookmark.GetToken(1, '#');
                                eClickAction = presentation::ClickAction_BOOKMARK;
                            }

                            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pPickObj);

                            BOOL bCreated = FALSE;

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
                            pAction->SetBlueScreen(pInfo->aBlueScreen, pInfo->aBlueScreen);
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
                        }
                    }
                    else if( pViewSh->ISA( SdDrawViewShell ) )
                    {
                        /******************************************************
                        * URLs als Buttons einfuegen
                        ******************************************************/
                        for (USHORT i = 0; i < nCount; i++)
                        {
                            if( aINetBookmark.PasteDragServer(i) )
                            {
                                ((SdDrawViewShell*)pViewSh)->InsertURLButton( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), String(), &aPos);
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

BOOL SdView::GetExchangeList( List*& rpExchangeList, List* pBookmarkList, USHORT nType )
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
                bNameOK = pDocSh->CheckPageName( pViewSh->GetWindow(), *pNewName );
            if( bNameOK && (nType == 1  || nType == 2) )
                bNameOK = pDocSh->CheckObjectName( pViewSh->GetWindow(), *pNewName );

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
