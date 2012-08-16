/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "View.hxx"
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <sot/filelist.hxx>
#include <unotools/pathoptions.hxx>
#include <editeng/editdata.hxx>
#include <svl/urlbmk.hxx>
#include <svx/xexch.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/app.hxx>
#include <svl/itempool.hxx>
#include <sot/clsids.hxx>
#include <svx/fmmodel.hxx>
#include <sot/formats.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <svx/obj3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/unomodel.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/metaact.hxx>
#include <svx/svxids.hrc>
#include <toolkit/helper/vclunohelper.hxx>

#include "DrawDocShell.hxx"
#include "fupoor.hxx"
#include "Window.hxx"
#include "sdxfer.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "SlideSorterViewShell.hxx"
#include "strmname.h"
#include "unomodel.hxx"
#include "ViewClipboard.hxx"

#include <sfx2/ipclient.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/stream.hxx>
#include <vcl/cvtgrf.hxx>
#include <svx/sdrhittesthelper.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

namespace sd {

#define CHECK_FORMAT_TRANS( _def_Type ) ( ( nFormat == (_def_Type) || !nFormat ) && aDataHelper.HasFormat( _def_Type ) )

/*************************************************************************
|*
|* Paste
|*
\************************************************************************/

struct ImpRememberOrigAndClone
{
    SdrObject*      pOrig;
    SdrObject*      pClone;
};

SdrObject* ImpGetClone(Container& aConnectorContainer, SdrObject* pConnObj)
{
    for(sal_uInt32 a(0); a < aConnectorContainer.Count(); a++)
    {
        if(pConnObj == ((ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a))->pOrig)
            return ((ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a))->pClone;
    }
    return 0L;
}

// restrict movement to WorkArea
void ImpCheckInsertPos(Point& rPos, const Size& rSize, const Rectangle& rWorkArea)
{
    if(!rWorkArea.IsEmpty())
    {
        Rectangle aMarkRect(Point(rPos.X() - (rSize.Width() / 2), rPos.Y() - (rSize.Height() / 2)), rSize);

        if(!aMarkRect.IsInside(rWorkArea))
        {
            if(aMarkRect.Left() < rWorkArea.Left())
            {
                rPos.X() += rWorkArea.Left() - aMarkRect.Left();
            }

            if(aMarkRect.Right() > rWorkArea.Right())
            {
                rPos.X() -= aMarkRect.Right() - rWorkArea.Right();
            }

            if(aMarkRect.Top() < rWorkArea.Top())
            {
                rPos.Y() += rWorkArea.Top() - aMarkRect.Top();
            }

            if(aMarkRect.Bottom() > rWorkArea.Bottom())
            {
                rPos.Y() -= aMarkRect.Bottom() - rWorkArea.Bottom();
            }
        }
    }
}

bool View::InsertMetaFile( TransferableDataHelper& rDataHelper, const Point& rPos, ImageMap* pImageMap, bool bOptimize )
{
    GDIMetaFile aMtf;

    if( !rDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
        return false;

    bool bVector = false;
    Graphic aGraphic;

    // check if metafile only contains a pixel image, if so insert a bitmap instead
    if( bOptimize )
    {
        MetaAction* pAction = aMtf.FirstAction();
        while( pAction && !bVector )
        {
            switch( pAction->GetType() )
            {
                case META_POINT_ACTION:
                case META_LINE_ACTION:
                case META_RECT_ACTION:
                case META_ROUNDRECT_ACTION:
                case META_ELLIPSE_ACTION:
                case META_ARC_ACTION:
                case META_PIE_ACTION:
                case META_CHORD_ACTION:
                case META_POLYLINE_ACTION:
                case META_POLYGON_ACTION:
                case META_POLYPOLYGON_ACTION:
                case META_TEXT_ACTION:
                case META_TEXTARRAY_ACTION:
                case META_STRETCHTEXT_ACTION:
                case META_TEXTRECT_ACTION:
                case META_GRADIENT_ACTION:
                case META_HATCH_ACTION:
                case META_WALLPAPER_ACTION:
                case META_EPS_ACTION:
                case META_TEXTLINE_ACTION:
                case META_FLOATTRANSPARENT_ACTION:
                case META_GRADIENTEX_ACTION:
                case META_BMPSCALEPART_ACTION:
                case META_BMPEXSCALEPART_ACTION:
                case META_RENDERGRAPHIC_ACTION:
                    bVector = true;
                    break;
                case META_BMP_ACTION:
                case META_BMPSCALE_ACTION:
                case META_BMPEX_ACTION:
                case META_BMPEXSCALE_ACTION:
                    if( aGraphic.GetType() != GRAPHIC_NONE )
                    {
                        bVector = true;
                    }
                    else switch( pAction->GetType() )
                    {
                        case META_BMP_ACTION:
                            {
                                MetaBmpAction* pBmpAction = dynamic_cast< MetaBmpAction* >( pAction );
                                if( pBmpAction )
                                    aGraphic = Graphic( pBmpAction->GetBitmap() );
                            }
                            break;
                        case META_BMPSCALE_ACTION:
                            {
                                MetaBmpScaleAction* pBmpScaleAction = dynamic_cast< MetaBmpScaleAction* >( pAction );
                                if( pBmpScaleAction )
                                    aGraphic = Graphic( pBmpScaleAction->GetBitmap() );
                            }
                            break;
                        case META_BMPEX_ACTION:
                            {
                                MetaBmpExAction* pBmpExAction = dynamic_cast< MetaBmpExAction* >( pAction );
                                if( pBmpExAction )
                                    aGraphic = Graphic( pBmpExAction->GetBitmapEx() );
                            }
                            break;
                        case META_BMPEXSCALE_ACTION:
                            {
                                MetaBmpExScaleAction* pBmpExScaleAction = dynamic_cast< MetaBmpExScaleAction* >( pAction );
                                if( pBmpExScaleAction )
                                    aGraphic = Graphic( pBmpExScaleAction->GetBitmapEx() );
                            }
                            break;
                    }
            }

            pAction = aMtf.NextAction();
        }
    }

    // it is not a vector metafile but it also has no graphic?
    if( !bVector && (aGraphic.GetType() == GRAPHIC_NONE) )
        bVector = true;

    // restrict movement to WorkArea
    Point aInsertPos( rPos );
    Size aImageSize;
    aImageSize = bVector ? aMtf.GetPrefSize() : aGraphic.GetSizePixel();
    ImpCheckInsertPos(aInsertPos, aImageSize, GetWorkArea());

    if( bVector )
        aGraphic = Graphic( aMtf );

    aGraphic.SetPrefMapMode( aMtf.GetPrefMapMode() );
    aGraphic.SetPrefSize( aMtf.GetPrefSize() );
    InsertGraphic( aGraphic, mnAction, aInsertPos, NULL, pImageMap );

    return true;
}

sal_Bool View::InsertData( const TransferableDataHelper& rDataHelper,
                         const Point& rPos, sal_Int8& rDnDAction, sal_Bool bDrag,
                         sal_uLong nFormat, sal_uInt16 nPage, sal_uInt16 nLayer )
{
    maDropPos = rPos;
    mnAction = rDnDAction;
    mbIsDropAllowed = sal_False;

    TransferableDataHelper  aDataHelper( rDataHelper );
    SdrObject*              pPickObj = NULL;
    SdPage*                 pPage = NULL;
    ImageMap*               pImageMap = NULL;
    sal_Bool                    bReturn = sal_False;
    sal_Bool                    bLink = ( ( mnAction & DND_ACTION_LINK ) != 0 );
    sal_Bool                    bCopy = ( ( ( mnAction & DND_ACTION_COPY ) != 0 ) || bLink );
    sal_uLong                   nPasteOptions = SDRINSERT_SETDEFLAYER;

    if (mpViewSh != NULL)
    {
        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
        SfxInPlaceClient* pIpClient = mpViewSh->GetViewShell()->GetIPClient();
        if( mpViewSh->ISA(::sd::slidesorter::SlideSorterViewShell)
            || (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive()))
        nPasteOptions |= SDRINSERT_DONTMARK;
    }

    if( bDrag )
    {
        SdrPageView* pPV = NULL;
        PickObj( rPos, getHitTolLog(), pPickObj, pPV );
    }

    if( nPage != SDRPAGE_NOTFOUND )
        pPage = (SdPage*) mrDoc.GetPage( nPage );

    SdTransferable* pOwnData = NULL;
    SdTransferable* pImplementation = SdTransferable::getImplementation( aDataHelper.GetTransferable() );

    // try to get own transfer data
    if( pImplementation )
    {
        if( SD_MOD()->pTransferClip == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferClip;
        else if( SD_MOD()->pTransferDrag == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferDrag;
        else if( SD_MOD()->pTransferSelection == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferSelection;
    }

    // ImageMap?
    if( !pOwnData && aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVIM ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVIM, xStm ) )
        {
            pImageMap = new ImageMap;
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            pImageMap->Read( *xStm, String() );
        }
    }

    bool bTable = false;
    // check special cases for pasting table formats as RTL
    if( !bLink && (!nFormat || (nFormat == SOT_FORMAT_RTF)) )
    {
        // if the objekt supports rtf and there is a table involved, default is to create a table
        if( aDataHelper.HasFormat( SOT_FORMAT_RTF ) && ! aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) )
        {
            SotStorageStreamRef xStm;

            if( aDataHelper.GetSotStorageStream( FORMAT_RTF, xStm ) )
            {
                xStm->Seek( 0 );

                rtl::OString aLine;
                while (xStm->ReadLine(aLine))
                {
                    sal_Int32 x = aLine.indexOf( "\\trowd" );
                    if (x != -1)
                    {
                        bTable = true;
                        nFormat = FORMAT_RTF;
                        break;
                    }
                }
            }
        }
    }

    if( pOwnData && !nFormat )
    {
        const View* pSourceView = pOwnData->GetView();


        if( pOwnData->GetDocShell() && pOwnData->IsPageTransferable() && ISA( View ) )
        {
            mpClipboard->HandlePageDrop (*pOwnData);
        }
        else if( pSourceView )
        {
            if( pSourceView == this )
            {
                // same view
                if( nLayer != SDRLAYER_NOTFOUND )
                {
                    // drop on layer tab bar
                    SdrLayerAdmin&  rLayerAdmin = mrDoc.GetLayerAdmin();
                    SdrLayer*       pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                    SdrPageView*    pPV = GetSdrPageView();
                    String          aLayer( pLayer->GetName() );

                    if( !pPV->IsLayerLocked( aLayer ) )
                    {
                        pOwnData->SetInternalMove( sal_True );
                        SortMarkedObjects();

                        for( sal_uLong nM = 0; nM < GetMarkedObjectCount(); nM++ )
                        {
                            SdrMark*    pM = GetSdrMarkByIndex( nM );
                            SdrObject*  pO = pM->GetMarkedSdrObj();

                            if( pO )
                            {
                                // #i11702#
                                if( IsUndoEnabled() )
                                {
                                    BegUndo(String(SdResId(STR_MODIFYLAYER)));
                                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectLayerChange(*pO, pO->GetLayer(), (SdrLayerID)nLayer));
                                    EndUndo();
                                }

                                pO->SetLayer( (SdrLayerID) nLayer );
                            }
                        }

                        bReturn = sal_True;
                    }
                }
                else
                {
                    SdrPageView*    pPV = GetSdrPageView();
                    sal_Bool            bDropOnTabBar = sal_True;

                    if( !pPage && pPV->GetPage()->GetPageNum() != mnDragSrcPgNum )
                    {
                        pPage = (SdPage*) pPV->GetPage();
                        bDropOnTabBar = sal_False;
                    }

                    if( pPage )
                    {
                        // drop on other page
                        String aActiveLayer( GetActiveLayer() );

                        if( !pPV->IsLayerLocked( aActiveLayer ) )
                        {
                            if( !IsPresObjSelected() )
                            {
                                SdrMarkList* pMarkList;

                                if( (mnDragSrcPgNum != SDRPAGE_NOTFOUND) && (mnDragSrcPgNum != pPV->GetPage()->GetPageNum()) )
                                {
                                    pMarkList = mpDragSrcMarkList;
                                }
                                else
                                {
                                    // actual mark list is used
                                    pMarkList = new SdrMarkList( GetMarkedObjectList());
                                }

                                pMarkList->ForceSort();

                                // stuff to remember originals and clones
                                Container   aConnectorContainer(0);
                                sal_uInt32  a, nConnectorCount(0L);
                                Point       aCurPos;

                                // calculate real position of current
                                // source objects, if necessary (#103207)
                                if( pOwnData == SD_MOD()->pTransferSelection )
                                {
                                    Rectangle aCurBoundRect;

                                    if( pMarkList->TakeBoundRect( pPV, aCurBoundRect ) )
                                        aCurPos = aCurBoundRect.TopLeft();
                                    else
                                        aCurPos = pOwnData->GetStartPos();
                                }
                                else
                                    aCurPos = pOwnData->GetStartPos();

                                const Size aVector( maDropPos.X() - aCurPos.X(), maDropPos.Y() - aCurPos.Y() );

                                for(a = 0; a < pMarkList->GetMarkCount(); a++)
                                {
                                    SdrMark* pM = pMarkList->GetMark(a);
                                    SdrObject* pObj = pM->GetMarkedSdrObj()->Clone();

                                    if(pObj)
                                    {
                                        if(!bDropOnTabBar)
                                        {
                                            // do a NbcMove(...) instead of setting SnapRects here
                                            pObj->NbcMove(aVector);
                                        }

                                        pPage->InsertObject(pObj);

                                        if( IsUndoEnabled() )
                                        {
                                            BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));
                                            EndUndo();
                                        }

                                        ImpRememberOrigAndClone* pRem = new ImpRememberOrigAndClone;
                                        pRem->pOrig = pM->GetMarkedSdrObj();
                                        pRem->pClone = pObj;
                                        aConnectorContainer.Insert(pRem, CONTAINER_APPEND);

                                        if(pObj->ISA(SdrEdgeObj))
                                            nConnectorCount++;
                                    }
                                }

                                // try to re-establish connections at clones
                                if(nConnectorCount)
                                {
                                    for(a = 0; a < aConnectorContainer.Count(); a++)
                                    {
                                        ImpRememberOrigAndClone* pRem = (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);

                                        if(pRem->pClone->ISA(SdrEdgeObj))
                                        {
                                            SdrEdgeObj* pOrigEdge = (SdrEdgeObj*)pRem->pOrig;
                                            SdrEdgeObj* pCloneEdge = (SdrEdgeObj*)pRem->pClone;

                                            // test first connection
                                            SdrObjConnection& rConn0 = pOrigEdge->GetConnection(sal_False);
                                            SdrObject* pConnObj = rConn0.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(sal_False, pConnClone);
                                                    pCloneEdge->GetConnection(sal_False).SetConnectorId(rConn0.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn0.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.X() += aVector.A();
                                                            aPosition.Y() += aVector.B();
                                                            pCloneEdge->SetTailPoint(sal_False, aPosition);
                                                        }
                                                    }
                                                }
                                            }

                                            // test second connection
                                            SdrObjConnection& rConn1 = pOrigEdge->GetConnection(sal_True);
                                            pConnObj = rConn1.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(sal_True, pConnClone);
                                                    pCloneEdge->GetConnection(sal_True).SetConnectorId(rConn1.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn1.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.X() += aVector.A();
                                                            aPosition.Y() += aVector.B();
                                                            pCloneEdge->SetTailPoint(sal_True, aPosition);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // cleanup remember classes
                                for(a = 0; a < aConnectorContainer.Count(); a++)
                                    delete (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);

                                if( pMarkList != mpDragSrcMarkList )
                                    delete pMarkList;

                                bReturn = sal_True;
                            }
                            else
                            {
                                maDropErrorTimer.Start();
                                bReturn = sal_False;
                            }
                        }
                    }
                    else
                    {
                        pOwnData->SetInternalMove( sal_True );
                        MoveAllMarked( Size( maDropPos.X() - pOwnData->GetStartPos().X(),
                                             maDropPos.Y() - pOwnData->GetStartPos().Y() ), bCopy );
                        bReturn = sal_True;
                    }
                }
            }
            else
            {
                // different views
                if( !pSourceView->IsPresObjSelected() )
                {
                    // model is owned by from AllocModel() created DocShell
                    SdDrawDocument* pSourceDoc = (SdDrawDocument*) pSourceView->GetModel();
                    pSourceDoc->CreatingDataObj( pOwnData );
                    SdDrawDocument* pModel = (SdDrawDocument*) pSourceView->GetAllMarkedModel();
                    bReturn = Paste( *pModel, maDropPos, pPage, nPasteOptions );

                    if( !pPage )
                        pPage = (SdPage*) GetSdrPageView()->GetPage();

                    String aLayout( pPage->GetLayoutName() );
                    aLayout.Erase( aLayout.SearchAscii( SD_LT_SEPARATOR ) );
                    pPage->SetPresentationLayout( aLayout, sal_False, sal_False );
                    pSourceDoc->CreatingDataObj( NULL );
                }
                else
                {
                    maDropErrorTimer.Start();
                    bReturn = sal_False;
                }
            }
        }
        else
        {
            SdDrawDocument* pWorkModel = (SdDrawDocument*) pOwnData->GetWorkDocument();
            SdPage*         pWorkPage = (SdPage*) pWorkModel->GetSdPage( 0, PK_STANDARD );

            pWorkPage->SetRectsDirty();

            // Use SnapRect, not BoundRect
            Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

            maDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
            maDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );

            // delete pages, that are not of any interest for us
            for( long i = ( pWorkModel->GetPageCount() - 1 ); i >= 0; i-- )
            {
                SdPage* pP = static_cast< SdPage* >( pWorkModel->GetPage( (sal_uInt16) i ) );

                if( pP->GetPageKind() != PK_STANDARD )
                    pWorkModel->DeletePage( (sal_uInt16) i );
            }

            bReturn = Paste( *pWorkModel, maDropPos, pPage, nPasteOptions );

            if( !pPage )
                pPage = (SdPage*) GetSdrPageView()->GetPage();

            String aLayout(pPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
            pPage->SetPresentationLayout( aLayout, sal_False, sal_False );
       }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_DRAWING ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
        {
            sal_Bool bChanged = sal_False;

            DrawDocShellRef xShell = new DrawDocShell(SFX_CREATE_MODE_INTERNAL);
            xShell->DoInitNew(0);

            SdDrawDocument* pModel = xShell->GetDoc();
            pModel->InsertPage(pModel->AllocPage(false));

            Reference< XComponent > xComponent( xShell->GetModel(), UNO_QUERY );
            xStm->Seek( 0 );

            com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ) );
            bReturn = SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Impress.XMLOasisImporter" );

            if( pModel->GetPageCount() == 0 )
            {
                OSL_FAIL("empty or invalid drawing xml document on clipboard!" );
            }
            else
            {
                if( bReturn )
                {
                    if( pModel->GetSdPage( 0, PK_STANDARD )->GetObjCount() == 1 )
                    {
                        // only one object
                        SdrObject*      pObj = pModel->GetSdPage( 0, PK_STANDARD )->GetObj( 0 );
                        SdrObject*      pPickObj2 = NULL;
                        SdrPageView*    pPV = NULL;
                        PickObj( rPos, getHitTolLog(), pPickObj2, pPV );

                        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj2 && pObj )
                        {
                            // replace object
                            SdrObject*  pNewObj = pObj->Clone();
                            Rectangle   aPickObjRect( pPickObj2->GetCurrentBoundRect() );
                            Size        aPickObjSize( aPickObjRect.GetSize() );
                            Point       aVec( aPickObjRect.TopLeft() );
                            Rectangle   aObjRect( pNewObj->GetCurrentBoundRect() );
                            Size        aObjSize( aObjRect.GetSize() );

                            Fraction aScaleWidth( aPickObjSize.Width(), aObjSize.Width() );
                            Fraction aScaleHeight( aPickObjSize.Height(), aObjSize.Height() );
                            pNewObj->NbcResize( aObjRect.TopLeft(), aScaleWidth, aScaleHeight );

                            aVec -= aObjRect.TopLeft();
                            pNewObj->NbcMove( Size( aVec.X(), aVec.Y() ) );

                            const bool bUndo = IsUndoEnabled();

                            if( bUndo )
                                BegUndo( String( SdResId(STR_UNDO_DRAGDROP ) ) );
                            pNewObj->NbcSetLayer( pPickObj->GetLayer() );
                            SdrPage* pWorkPage = GetSdrPageView()->GetPage();
                            pWorkPage->InsertObject( pNewObj );
                            if( bUndo )
                            {
                                AddUndo( mrDoc.GetSdrUndoFactory().CreateUndoNewObject( *pNewObj ) );
                                AddUndo( mrDoc.GetSdrUndoFactory().CreateUndoDeleteObject( *pPickObj2 ) );
                            }
                            pWorkPage->RemoveObject( pPickObj2->GetOrdNum() );

                            if( bUndo )
                            {
                                EndUndo();
                            }
                            else
                            {
                                SdrObject::Free(pPickObj2 );
                            }
                            bChanged = sal_True;
                            mnAction = DND_ACTION_COPY;
                        }
                        else if( ( mnAction & DND_ACTION_LINK ) && pPickObj && pObj && !pPickObj->ISA( SdrGrafObj ) && !pPickObj->ISA( SdrOle2Obj ) )
                        {
                            SfxItemSet aSet( mrDoc.GetPool() );

                            // set new attributes to object
                            const bool bUndo = IsUndoEnabled();
                            if( bUndo )
                            {
                                BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                                AddUndo( mrDoc.GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
                            }
                            aSet.Put( pObj->GetMergedItemSet() );

                            // Eckenradius soll nicht uebernommen werden.
                            // In der Gallery stehen Farbverlauefe (Rechtecke)
                            // welche den Eckenradius == 0 haben. Dieser soll
                            // nicht auf das Objekt uebertragen werden.
                            aSet.ClearItem( SDRATTR_ECKENRADIUS );

                            pPickObj->SetMergedItemSetAndBroadcast( aSet );

                            if( pPickObj->ISA( E3dObject ) && pObj->ISA( E3dObject ) )
                            {
                                // Zusaetzlich 3D Attribute handeln
                                SfxItemSet aNewSet( mrDoc.GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );
                                SfxItemSet aOldSet( mrDoc.GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );

                                aOldSet.Put(pPickObj->GetMergedItemSet());
                                aNewSet.Put( pObj->GetMergedItemSet() );

                                if( bUndo )
                                    AddUndo( new E3dAttributesUndoAction( mrDoc, (E3dObject*) pPickObj, aNewSet, aOldSet ) );
                                pPickObj->SetMergedItemSetAndBroadcast( aNewSet );
                            }

                            if( bUndo )
                                EndUndo();
                            bChanged = sal_True;
                        }
                    }
                }

                if( !bChanged )
                {
                    SdrPage* pWorkPage = pModel->GetSdPage( 0, PK_STANDARD );

                    pWorkPage->SetRectsDirty();

                    if( pOwnData )
                    {
                        // Use SnapRect, not BoundRect
                        Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                        maDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                        maDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                    }

                    bReturn = Paste( *pModel, maDropPos, pPage, nPasteOptions );
                }

                xShell->DoClose();
            }
        }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE ) )
    {
        ::rtl::OUString aOUString;

        if( aDataHelper.GetString( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE, aOUString ) )
        {
            SdrObject* pObj = CreateFieldControl( aOUString );

            if( pObj )
            {
                Rectangle   aRect( pObj->GetLogicRect() );
                Size        aSize( aRect.GetSize() );

                maDropPos.X() -= ( aSize.Width() >> 1 );
                maDropPos.Y() -= ( aSize.Height() >> 1 );

                aRect.SetPos( maDropPos );
                pObj->SetLogicRect( aRect );
                InsertObjectAtView( pObj, *GetSdrPageView(), SDRINSERT_SETDEFLAYER );
                bReturn = sal_True;
            }
        }
    }
    else if( !bLink &&
             ( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBED_SOURCE ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) )  &&
               aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
    {
        //TODO/LATER: is it possible that this format is binary?! (from old versions of SO)
        uno::Reference < io::XInputStream > xStm;
        TransferableObjectDescriptor    aObjDesc;

        if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) &&
            ( aDataHelper.GetInputStream( nFormat ? nFormat : SOT_FORMATSTR_ID_EMBED_SOURCE, xStm ) ||
              aDataHelper.GetInputStream( SOT_FORMATSTR_ID_EMBEDDED_OBJ, xStm ) ) )
        {
            if( mrDoc.GetDocSh() && ( mrDoc.GetDocSh()->GetClassName() == aObjDesc.maClassName ) )
            {
                uno::Reference < embed::XStorage > xStore( ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm ) );
                ::sd::DrawDocShellRef xDocShRef( new ::sd::DrawDocShell( SFX_CREATE_MODE_EMBEDDED, sal_True, mrDoc.GetDocumentType() ) );

                // mba: BaseURL doesn't make sense for clipboard functionality
                SfxMedium *pMedium = new SfxMedium( xStore, String() );
                if( xDocShRef->DoLoad( pMedium ) )
                {
                    SdDrawDocument* pModel = (SdDrawDocument*) xDocShRef->GetDoc();
                    SdPage*         pWorkPage = (SdPage*) pModel->GetSdPage( 0, PK_STANDARD );

                    pWorkPage->SetRectsDirty();

                    if( pOwnData )
                    {
                        // Use SnapRect, not BoundRect
                        Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                        maDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                        maDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                    }

                    // delete pages, that are not of any interest for us
                    for( long i = ( pModel->GetPageCount() - 1 ); i >= 0; i-- )
                    {
                        SdPage* pP = static_cast< SdPage* >( pModel->GetPage( (sal_uInt16) i ) );

                        if( pP->GetPageKind() != PK_STANDARD )
                            pModel->DeletePage( (sal_uInt16) i );
                    }

                    bReturn = Paste( *pModel, maDropPos, pPage, nPasteOptions );

                    if( !pPage )
                        pPage = (SdPage*) GetSdrPageView()->GetPage();

                    String aLayout(pPage->GetLayoutName());
                    aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
                    pPage->SetPresentationLayout( aLayout, sal_False, sal_False );
                }

                xDocShRef->DoClose();
                xDocShRef.Clear();

            }
            else
            {
                ::rtl::OUString aName;
                uno::Reference < embed::XEmbeddedObject > xObj = mpDocSh->GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                if ( xObj.is() )
                {
                    svt::EmbeddedObjectRef aObjRef( xObj, aObjDesc.mnViewAspect );

                    // try to get the replacement image from the clipboard
                    Graphic aGraphic;
                    sal_uLong nGrFormat = 0;


                    // insert replacement image ( if there is one ) into the object helper
                    if ( nGrFormat )
                    {
                        datatransfer::DataFlavor aDataFlavor;
                        SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                        aObjRef.SetGraphic( aGraphic, aDataFlavor.MimeType );
                    }

                    Size aSize;
                    if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
                    {
                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                            aSize = aObjDesc.maSize;
                        else
                        {
                            MapMode aMapMode( MAP_100TH_MM );
                            aSize = aObjRef.GetSize( &aMapMode );
                        }
                    }
                    else
                    {
                        awt::Size aSz;
                        MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( aObjDesc.mnViewAspect ) );
                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                        {
                            Size aTmp( OutputDevice::LogicToLogic( aObjDesc.maSize, MAP_100TH_MM, aMapUnit ) );
                            aSz.Width = aTmp.Width();
                            aSz.Height = aTmp.Height();
                            xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                        }

                        try
                        {
                            aSz = xObj->getVisualAreaSize( aObjDesc.mnViewAspect );
                        }
                        catch( embed::NoVisualAreaSizeException& )
                        {
                            // if the size still was not set the default size will be set later
                        }

                        aSize = Size( aSz.Width, aSz.Height );

                        if( !aSize.Width() || !aSize.Height() )
                        {
                            aSize.Width()  = 14100;
                            aSize.Height() = 10000;
                            aSize = OutputDevice::LogicToLogic( Size(14100, 10000), MAP_100TH_MM, aMapUnit );
                            aSz.Width = aSize.Width();
                            aSz.Height = aSize.Height();
                            xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                        }

                        aSize = OutputDevice::LogicToLogic( aSize, aMapUnit, MAP_100TH_MM );
                    }

                    Size aMaxSize( mrDoc.GetMaxObjSize() );

                    maDropPos.X() -= Min( aSize.Width(), aMaxSize.Width() ) >> 1;
                    maDropPos.Y() -= Min( aSize.Height(), aMaxSize.Height() ) >> 1;

                    Rectangle       aRect( maDropPos, aSize );
                    SdrOle2Obj*     pObj = new SdrOle2Obj( aObjRef, aName, aRect );
                    SdrPageView*    pPV = GetSdrPageView();
                    sal_uLong           nOptions = SDRINSERT_SETDEFLAYER;

                    if (mpViewSh!=NULL)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObjectAtView( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->AppendUserData( new SdIMapInfo( *pImageMap ) );

                    if ( pObj && pObj->IsChart() )
                    {
                        bool bDisableDataTableDialog = false;
                        svt::EmbeddedObjectRef::TryRunningState( xObj );
                        uno::Reference< beans::XPropertySet > xProps( xObj->getComponent(), uno::UNO_QUERY );
                        if ( xProps.is() &&
                             ( xProps->getPropertyValue( "DisableDataTableDialog" ) >>= bDisableDataTableDialog ) &&
                             bDisableDataTableDialog )
                        {
                            xProps->setPropertyValue( "DisableDataTableDialog" , uno::makeAny( sal_False ) );
                            xProps->setPropertyValue( "DisableComplexChartTypes" , uno::makeAny( sal_False ) );
                            uno::Reference< util::XModifiable > xModifiable( xProps, uno::UNO_QUERY );
                            if ( xModifiable.is() )
                            {
                                xModifiable->setModified( sal_True );
                            }
                        }
                    }

                    bReturn = sal_True;
                }
            }
        }
    }
    else if( !bLink &&
             ( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ) &&
               aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE ) )
    {
        // online insert ole if format is forced or no gdi metafile is available
        if( (nFormat != 0) || !aDataHelper.HasFormat( FORMAT_GDIMETAFILE ) )
        {
            uno::Reference < io::XInputStream > xStm;
            TransferableObjectDescriptor    aObjDesc;

            if ( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE, aObjDesc ) )
            {
                uno::Reference < embed::XEmbeddedObject > xObj;
                ::rtl::OUString aName;

                if ( aDataHelper.GetInputStream( nFormat ? nFormat : SOT_FORMATSTR_ID_EMBED_SOURCE_OLE, xStm ) ||
                    aDataHelper.GetInputStream( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE, xStm ) )
                {
                    xObj = mpDocSh->GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                }
                else
                {
                    try
                    {
                        uno::Reference< embed::XStorage > xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        uno::Reference < embed::XEmbedObjectClipboardCreator > xClipboardCreator(
                            ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.embed.MSOLEObjectSystemCreator"),
                            uno::UNO_QUERY_THROW );

                        embed::InsertedObjectInfo aInfo = xClipboardCreator->createInstanceInitFromClipboard(
                                                                xTmpStor,
                                                                "DummyName" ,
                                                                uno::Sequence< beans::PropertyValue >() );

                        // TODO/LATER: in future InsertedObjectInfo will be used to get container related information
                        // for example whether the object should be an iconified one
                        xObj = aInfo.Object;
                        if ( xObj.is() )
                            mpDocSh->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                    }
                    catch( uno::Exception& )
                    {}
                }

                if ( xObj.is() )
                {
                    svt::EmbeddedObjectRef aObjRef( xObj, aObjDesc.mnViewAspect );

                    // try to get the replacement image from the clipboard
                    Graphic aGraphic;
                    sal_uLong nGrFormat = 0;

// (wg. Selection Manager bei Trustet Solaris)
#ifndef SOLARIS
                    if( aDataHelper.GetGraphic( SOT_FORMATSTR_ID_SVXB, aGraphic ) )
                        nGrFormat = SOT_FORMATSTR_ID_SVXB;
                    else if( aDataHelper.GetGraphic( FORMAT_GDIMETAFILE, aGraphic ) )
                        nGrFormat = SOT_FORMAT_GDIMETAFILE;
                    else if( aDataHelper.GetGraphic( FORMAT_BITMAP, aGraphic ) )
                        nGrFormat = SOT_FORMAT_BITMAP;
#endif

                    // insert replacement image ( if there is one ) into the object helper
                    if ( nGrFormat )
                    {
                        datatransfer::DataFlavor aDataFlavor;
                        SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                        aObjRef.SetGraphic( aGraphic, aDataFlavor.MimeType );
                    }

                    Size aSize;
                    if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
                    {
                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                            aSize = aObjDesc.maSize;
                        else
                        {
                            MapMode aMapMode( MAP_100TH_MM );
                            aSize = aObjRef.GetSize( &aMapMode );
                        }
                    }
                    else
                    {
                        MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( aObjDesc.mnViewAspect ) );

                        awt::Size aSz;
                        try{
                            aSz = xObj->getVisualAreaSize( aObjDesc.mnViewAspect );
                        }
                        catch( embed::NoVisualAreaSizeException& )
                        {
                            // the default size will be set later
                        }

                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                        {
                            Size aTmp( OutputDevice::LogicToLogic( aObjDesc.maSize, MAP_100TH_MM, aMapUnit ) );
                            if ( aSz.Width != aTmp.Width() || aSz.Height != aTmp.Height() )
                            {
                                aSz.Width = aTmp.Width();
                                aSz.Height = aTmp.Height();
                                xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                            }
                        }

                        aSize = Size( aSz.Width, aSz.Height );

                        if( !aSize.Width() || !aSize.Height() )
                        {
                            aSize = OutputDevice::LogicToLogic( Size(14100, 10000), MAP_100TH_MM, aMapUnit );
                            aSz.Width = aSize.Width();
                            aSz.Height = aSize.Height();
                            xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                        }

                        aSize = OutputDevice::LogicToLogic( aSize, aMapUnit, MAP_100TH_MM );
                    }

                    Size aMaxSize( mrDoc.GetMaxObjSize() );

                    maDropPos.X() -= Min( aSize.Width(), aMaxSize.Width() ) >> 1;
                    maDropPos.Y() -= Min( aSize.Height(), aMaxSize.Height() ) >> 1;

                    Rectangle       aRect( maDropPos, aSize );
                    SdrOle2Obj*     pObj = new SdrOle2Obj( aObjRef, aName, aRect );
                    SdrPageView*    pPV = GetSdrPageView();
                    sal_uLong           nOptions = SDRINSERT_SETDEFLAYER;

                    if (mpViewSh!=NULL)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObjectAtView( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->AppendUserData( new SdIMapInfo( *pImageMap ) );

                    // let the object stay in loaded state after insertion
                    pObj->Unload();
                    bReturn = sal_True;
                }
            }
        }

        if( !bReturn && aDataHelper.HasFormat( FORMAT_GDIMETAFILE ) )
        {
            // if no object was inserted, insert a picture
            InsertMetaFile( aDataHelper, rPos, pImageMap, true );
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SVXB ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ) )
        {
            Point   aInsertPos( rPos );
            Graphic aGraphic;

            *xStm >> aGraphic;

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetRectsDirty();

                // Use SnapRect, not BoundRect
                Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            // restrict movement to WorkArea
            Size aImageMapSize = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(),
                aGraphic.GetPrefMapMode(), MapMode(MAP_100TH_MM));

            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aGraphic, mnAction, aInsertPos, NULL, pImageMap );
            bReturn = sal_True;
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_GDIMETAFILE ) )
    {
        Point aInsertPos( rPos );

        if( pOwnData && pOwnData->GetWorkDocument() )

        {
            const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
            SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                pWorkModel->GetPage( 0 ) );

            pWorkPage->SetRectsDirty();

            // Use SnapRect, not BoundRect
            Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

            aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
            aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
        }

        bReturn = InsertMetaFile( aDataHelper, aInsertPos, pImageMap, nFormat == 0 ? true : false ) ? sal_True : sal_False;
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_BITMAP ) )
    {
        Bitmap aBmp;

        if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ) )
        {
            Point aInsertPos( rPos );

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetRectsDirty();

                // Use SnapRect, not BoundRect
                Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            // restrict movement to WorkArea
            Size aImageMapSize(aBmp.GetPrefSize());
            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aBmp, mnAction, aInsertPos, NULL, pImageMap );
            bReturn = sal_True;
        }
    }
    else if( pPickObj && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_XFA ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) )
        {
            XFillExchangeData aFillData( XFillAttrSetItem( &mrDoc.GetPool() ) );

            *xStm >> aFillData;

            if( IsUndoEnabled() )
            {
                BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
                EndUndo();
            }

            XFillAttrSetItem*   pSetItem = aFillData.GetXFillAttrSetItem();
            SfxItemSet          rSet = pSetItem->GetItemSet();
            XFillStyle          eFill= ( (XFillStyleItem&) rSet.Get( XATTR_FILLSTYLE ) ).GetValue();

            if( eFill == XFILL_SOLID || eFill == XFILL_NONE )
            {
                const XFillColorItem&   rColItem = (XFillColorItem&) rSet.Get( XATTR_FILLCOLOR );
                Color                   aColor( rColItem.GetColorValue() );
                String                  aName( rColItem.GetName() );
                SfxItemSet              aSet( mrDoc.GetPool() );
                sal_Bool                    bClosed = pPickObj->IsClosedObj();
                ::sd::Window* pWin = mpViewSh->GetActiveWindow();
                sal_uInt16 nHitLog = (sal_uInt16) pWin->PixelToLogic(
                    Size(FuPoor::HITPIX, 0 ) ).Width();
                const long              n2HitLog = nHitLog << 1;
                Point                   aHitPosR( rPos );
                Point                   aHitPosL( rPos );
                Point                   aHitPosT( rPos );
                Point                   aHitPosB( rPos );
                const SetOfByte*        pVisiLayer = &GetSdrPageView()->GetVisibleLayers();

                aHitPosR.X() += n2HitLog;
                aHitPosL.X() -= n2HitLog;
                aHitPosT.Y() += n2HitLog;
                aHitPosB.Y() -= n2HitLog;

                if( bClosed &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosR, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosL, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosT, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosB, nHitLog, *GetSdrPageView(), pVisiLayer, false) )
                {
                    // area fill
                    if(eFill == XFILL_SOLID )
                        aSet.Put(XFillColorItem(aName, aColor));

                    aSet.Put( XFillStyleItem( eFill ) );
                }
                else
                    aSet.Put( XLineColorItem( aName, aColor ) );

                // Textfarbe hinzufuegen
                pPickObj->SetMergedItemSetAndBroadcast( aSet );
            }
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_HTML ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_HTML, xStm ) )
        {
            xStm->Seek( 0 );
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            bReturn = SdrView::Paste( *xStm, String(), EE_FORMAT_HTML, maDropPos, pPage, nPasteOptions );
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EDITENGINE ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_EDITENGINE, xStm ) )
        {
            OutlinerView* pOLV = GetTextEditOutlinerView();

            xStm->Seek( 0 );

            if( pOLV )
            {
                Rectangle   aRect( pOLV->GetOutputArea() );
                   Point       aPos( pOLV->GetWindow()->PixelToLogic( maDropPos ) );

                if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                {
                    // mba: clipboard always must contain absolute URLs (could be from alien source)
                    pOLV->Read( *xStm, String(), EE_FORMAT_BIN, sal_False, mpDocSh->GetHeaderAttributes() );
                    bReturn = sal_True;
                }
            }

            if( !bReturn )
                // mba: clipboard always must contain absolute URLs (could be from alien source)
                bReturn = SdrView::Paste( *xStm, String(), EE_FORMAT_BIN, maDropPos, pPage, nPasteOptions );
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( FORMAT_RTF ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( FORMAT_RTF, xStm ) )
        {
            xStm->Seek( 0 );

            if( bTable )
            {
                bReturn = PasteRTFTable( xStm, pPage, nPasteOptions );
            }
            else
            {
                OutlinerView* pOLV = GetTextEditOutlinerView();

                if( pOLV )
                {
                    Rectangle   aRect( pOLV->GetOutputArea() );
                       Point       aPos( pOLV->GetWindow()->PixelToLogic( maDropPos ) );

                    if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                    {
                        // mba: clipboard always must contain absolute URLs (could be from alien source)
                        pOLV->Read( *xStm, String(), EE_FORMAT_RTF, sal_False, mpDocSh->GetHeaderAttributes() );
                        bReturn = sal_True;
                    }
                }

                if( !bReturn )
                    // mba: clipboard always must contain absolute URLs (could be from alien source)
                    bReturn = SdrView::Paste( *xStm, String(), EE_FORMAT_RTF, maDropPos, pPage, nPasteOptions );
            }
        }
    }
    else if( CHECK_FORMAT_TRANS( FORMAT_FILE_LIST ) )
    {
        FileList aDropFileList;

        if( aDataHelper.GetFileList( FORMAT_FILE_LIST, aDropFileList ) )
        {
            maDropFileVector.clear();

            for( sal_uLong i = 0, nCount = aDropFileList.Count(); i < nCount; i++ )
                maDropFileVector.push_back( aDropFileList.GetFile( i ) );

            maDropInsertFileTimer.Start();
        }

        bReturn = sal_True;
    }
    else if( CHECK_FORMAT_TRANS( FORMAT_FILE ) )
    {
        String aDropFile;

        if( aDataHelper.GetString( FORMAT_FILE, aDropFile ) )
        {
            maDropFileVector.clear();
            maDropFileVector.push_back( aDropFile );
            maDropInsertFileTimer.Start();
        }

        bReturn = sal_True;
    }
    else if( !bLink && CHECK_FORMAT_TRANS( FORMAT_STRING ) )
    {
        if( ( FORMAT_STRING == nFormat ) ||
            ( !aDataHelper.HasFormat( SOT_FORMATSTR_ID_SOLK ) &&
              !aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
              !aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILENAME ) ) )
        {
            ::rtl::OUString aOUString;

            if( aDataHelper.GetString( FORMAT_STRING, aOUString ) )
            {
                OutlinerView* pOLV = GetTextEditOutlinerView();

                if( pOLV )
                {
                    pOLV->InsertText( aOUString );
                    bReturn = sal_True;
                }

                if( !bReturn )
                    bReturn = SdrView::Paste( aOUString, maDropPos, pPage, nPasteOptions );
            }
        }
    }

    MarkListHasChanged();
    mbIsDropAllowed = sal_True;
    rDnDAction = mnAction;
    delete pImageMap;

    return bReturn;
}

extern void CreateTableFromRTF( SvStream& rStream, SdDrawDocument* pModel  );

bool View::PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, sal_uLong nPasteOptions )
{
    SdDrawDocument* pModel = new SdDrawDocument( DOCUMENT_TYPE_IMPRESS, mpDocSh );
    pModel->NewOrLoadCompleted(NEW_DOC);
    pModel->GetItemPool().SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pModel->InsertPage(pModel->AllocPage(false));

    Reference< XComponent > xComponent( new SdXImpressDocument( pModel, sal_True ) );
    pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );

    CreateTableFromRTF( *xStm, pModel );
    bool bRet = Paste( *pModel, maDropPos, pPage, nPasteOptions );

    xComponent->dispose();
    xComponent.clear();

    delete pModel;

    return bRet;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
