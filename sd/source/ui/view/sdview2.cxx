/*************************************************************************
 *
 *  $RCSfile: sdview2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-01-19 19:11:24 $
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

#include "strings.hrc"
#include "sdview.hxx"
#include "dragserv.hxx"
#include "sdxfer.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "drviewsh.hxx"
#include "docshell.hxx"
#include "fudraw.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "sdpage.hxx"
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

/*************************************************************************
|*
|* DataObject fuer Drag&Drop erzeugen
|*
\************************************************************************/

SvDataObjectRef SdView::CreateDataObject( SdView* pWorkView, const Point& rDragPos )
{
    SdDataObjectRef     xDataObject( new SdDataObject( pDoc, pWorkView ) );
    SvObjectDescriptor* pObjDesc;
    String              aDisplayName;
    SdrOle2Obj*         pSdrOleObj = NULL;

    SD_MOD()->pDragData = xDataObject;

    if( aMark.GetMarkCount() == 1 )
    {
        SdrObject* pObj = aMark.GetMark(0)->GetObj();

        if( pObj && pObj->ISA(SdrOle2Obj) && ((SdrOle2Obj*) pObj)->GetObjRef().Is() )
            pSdrOleObj = (SdrOle2Obj*) pObj;
    }

    if( pDocSh )
        aDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();

    if( pSdrOleObj )
        pObjDesc = new SvObjectDescriptor( pSdrOleObj->GetObjRef(), rDragPos );
    else
        pObjDesc = new SvObjectDescriptor( pDocSh, rDragPos );

    pObjDesc->SetSize( GetAllMarkedRect().GetSize() );
    pObjDesc->SetDisplayName(aDisplayName);
    pObjDesc->SetCanLink(FALSE);

    xDataObject->SetStartPos( rDragPos );
    xDataObject->SetObjectDescriptor( *pObjDesc );

    delete pObjDesc;

    return xDataObject;
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
                DropAction eAction = DROP_COPY;

                if( !InsertData( aDataHelper.GetTransferable(), aPos, eAction, FALSE ) )
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
    BOOL bReturn = FALSE;

    if (HasMarkedObj() && IsAction() && pViewSh)
    {
        BrkAction();

        SdViewShell* pViewShell= pDocSh->GetViewShell();

        if ( pViewShell && pViewShell->ISA(SdDrawViewShell) )
        {
            FuPoor* pFunc = ( (SdDrawViewShell*) pViewShell)->GetActualFunction();

            if ( pFunc && pFunc->ISA(FuDraw) )
            {
                ( (FuDraw*) pFunc)->ForcePointer(NULL);
            }
        }

        BOOL bPresObjSelected = IsPresObjSelected();

        SdDataObjectRef pDataObject = CreateDataObject(this, aStartPos);

        pDragSrcMarkList = new SdrMarkList(aMark);
        nDragSrcPgNum = GetPageViewPvNum(0)->GetPage()->GetPageNum();

        String aStr( SdResId(STR_UNDO_DRAGDROP) );
        aStr += sal_Unicode(' ');
        aStr += pDragSrcMarkList->GetMarkDescription();
        BegUndo(aStr);

        DropAction eDropAction;

        Region aRegion ( GetAllMarkedRect() ) ;

        bDragActive = TRUE;
        USHORT nDragMode = DRAG_ALL;

        eDropAction = pDataObject->ExecuteDrag(pWindow, POINTER_MOVEDATA,
                                               POINTER_COPYDATA, POINTER_LINKDATA,
                                               nDragMode, &aRegion);

        bDragActive = FALSE;
        pDataObject->SetView( NULL );

        switch (eDropAction)
        {
            case DROP_MOVE:
            case DROP_DISCARD:
            {
                if (!pDataObject->IsInternalMove() && !bPresObjSelected)
                {
                    // Objekte loeschen

                    if (pDragSrcMarkList->GetMarkCount()!=0)
                    {
                        pDragSrcMarkList->ForceSort();
                        BegUndo();
                        ULONG nAnz=pDragSrcMarkList->GetMarkCount();
                        ULONG nm;
                        for (nm=nAnz; nm>0;)
                        {
                            nm--;
                            SdrMark* pM=pDragSrcMarkList->GetMark(nm);
                            AddUndo(new SdrUndoDelObj(*pM->GetObj()));
                        }
                        // Sicherstellen, dass die OrderNums stimmen:
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
                }
            }
            break;

            case DROP_NONE:
            case DROP_COPY:
            case DROP_LINK:
            case DROP_PRINT:
                break;

            default:
                break;
        }

        if (eDropAction != DROP_NONE)
        {
            bReturn = TRUE;
        }

        pDataObject->SetInternalMove( FALSE );

        EndUndo();

        nDragSrcPgNum = SDRPAGE_NOTFOUND;
        delete pDragSrcMarkList;
    }

    return (bReturn);
}


/*************************************************************************
|*
|* QueryDrop-Event
|*
\************************************************************************/

BOOL SdView::QueryDrop(DropEvent& rDEvt, SdWindow* pWin,
                                USHORT nPage, USHORT nLayer)
{
    BOOL bReturn = FALSE;
    String aLayerName( GetActiveLayer() );
    SdrPageView* pPV = GetPageViewPvNum(0);

    if (nLayer != SDRLAYER_NOTFOUND)
    {
        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        aLayerName = rLayerAdmin.GetLayerPerID(nLayer)->GetName();
    }

    bReturn = bIsDropAllowed && !pPV->IsLayerLocked( aLayerName ) && pPV->IsLayerVisible( aLayerName );

    if( !bReturn )
        return bReturn;                 // Layer gesperrt oder unsichtbar

    BOOL bIsInsideOutlinerView = FALSE;
    const OutlinerView* pOLV = GetTextEditOutlinerView();

    if (pOLV)
    {
        Rectangle aRect( pOLV->GetOutputArea() );

        if (aMark.GetMarkCount() == 1)
        {
            SdrMark* pMark = aMark.GetMark(0);
            SdrObject* pObj = pMark->GetObj();
            aRect.Union( pObj->GetLogicRect() );
        }

        Point aPos = pOLV->GetWindow()->PixelToLogic( rDEvt.GetPosPixel() );

        if ( aRect.IsInside(aPos) )
        {
            bIsInsideOutlinerView = TRUE;
            bReturn = ( (OutlinerView*) pOLV)->QueryDrop(rDEvt);
        }
    }

    if( !bIsInsideOutlinerView )
    {
        SdDataObjectRef pDataObject = SD_MOD()->pDragData;

        if( pDataObject )
        {
            const SdView* pSourceView = pDataObject->GetView();

            if( pSourceView )
            {
                /**********************************************************
                * Eigenes Format innerhalb einer View immer
                **********************************************************/
                bReturn = TRUE;

                // Dokument hat keinen Namen: Link nicht zulassen
                if (rDEvt.GetAction() == DROP_LINK && pSourceView->GetDocSh()->GetMedium()->GetName().Len() == 0)
                    bReturn = FALSE;
            }
        }
        else
        {
            eAction = rDEvt.GetAction();

            if (rDEvt.IsDefaultAction() && eAction == DROP_MOVE)
            {
                eAction = DROP_COPY;
                rDEvt.SetAction(eAction);
            }

            BOOL bBookmark = INetBookmark::DragServerHasFormat(0);
            BOOL bGraphic = DragServer::HasFormat( 0, SOT_FORMATSTR_ID_SVXB );
            BOOL bXFillExchange = DragServer::HasFormat(0, SOT_FORMATSTR_ID_XFA);
            BOOL bSBAFormat = DragServer::HasFormat(0, Exchange::RegisterFormatName(
                String( RTL_CONSTASCII_USTRINGPARAM( SBA_FIELDEXCHANGE_FORMAT ))));
            BOOL bIsPresTarget = FALSE;
            BOOL bDefaultDrop = FmFormView::QueryDrop(rDEvt, pWin);

            if ( !bDefaultDrop &&
                (((DragServer::HasFormat(0, FORMAT_BITMAP)                 ||
                   DragServer::HasFormat(0, FORMAT_GDIMETAFILE)            ||
                   DragServer::HasFormat(0, SOT_FORMATSTR_ID_DRAWING)      ||
                   bGraphic                                                ||
                   bBookmark)                                              &&
                   (eAction==DROP_MOVE || eAction==DROP_LINK))             ||
                   bXFillExchange) )
            {
                // Drop auf Objekt kann erfolgen
                SdrObject* pPickObj = NULL;
                SdrPageView* pPV = NULL;
                SdWindow* pWindow = pViewSh->GetActiveWindow();
                USHORT nHitLog = USHORT(pWindow->PixelToLogic(Size(HITPIX,0)).Width());
                Point aPos = pWindow->PixelToLogic( rDEvt.GetPosPixel() );

                BOOL bHasPickObj = PickObj(aPos, pPickObj, pPV);

                if ( bHasPickObj && pPickObj &&
                        ( pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall() ) )
                {
                    SdPage* pPage = (SdPage*) pPickObj->GetPage();
                    if ( pPage && pPage->IsMasterPage() )
                        bIsPresTarget = pPage->GetPresObjList()->GetPos(pPickObj) != LIST_ENTRY_NOTFOUND;
                }

                if ( bHasPickObj &&
                    (!pPickObj->ISA(SdrGrafObj)                     ||
                     DragServer::HasFormat(0, FORMAT_BITMAP)        ||
                     DragServer::HasFormat(0, FORMAT_GDIMETAFILE)   ||
                     (bXFillExchange && !pPickObj->ISA(SdrGrafObj)
                                     && !pPickObj->ISA(SdrOle2Obj)) ||
                     bGraphic                                       ||
                     ((eAction == DROP_MOVE) && !bIsPresTarget)))
                {

                    if (!pDropMarker)
                    {
                        pDropMarker = new SdrViewUserMarker(this);
                    }

                    if (pDropMarkerObj != pPickObj)
                    {
                        pDropMarkerObj = pPickObj;
                        pDropMarker->SetXPolyPolygon(pDropMarkerObj, GetPageViewPvNum(0));
                        pDropMarker->Show();
                    }
                }
                else
                {
                    bXFillExchange = FALSE;

                    if (pDropMarker)
                    {
                        pDropMarker->Hide();
                        pDropMarkerObj = NULL;
                    }
                }
            }
            else
            {
                if (pDropMarker)
                {
                    pDropMarker->Hide();
                    pDropMarkerObj = NULL;
                }
            }

            BOOL bFile = ((DragServer::HasFormat(0, FORMAT_FILE) &&
                          pViewSh && !pViewSh->GetSlideShow()   &&
                         (rDEvt.IsDefaultAction() || eAction != DROP_MOVE)));

            if (bBookmark && !bFile && DragServer::HasFormat(0, FORMAT_FILE))
            {
                // Link auf File: kein DROP_MOVE zulassen
                bBookmark = FALSE;
            }

            bReturn = IsDragDropFormatSupported()                         ||
                      bFile                                               ||
                      bBookmark                                           ||
                      bGraphic                                            ||
                      bXFillExchange                                      ||
                      bSBAFormat                                          ||
                      bDefaultDrop;
        }
    }

    return bReturn;
}


