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
#include <svx/dbexch.hrc>
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
#include <svx/svdlegacy.hxx>

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

// #83525#
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

// #90129# restrict movement to WorkArea
void ImpCheckInsertPos(basegfx::B2DPoint& rPos, const basegfx::B2DVector& rSize, const basegfx::B2DRange& rWorkRange)
{
    if(!rWorkRange.isEmpty())
    {
        const basegfx::B2DRange aMarkRange(rPos - (rSize * 0.5), rPos + rSize);

        if(!aMarkRange.isInside(rWorkRange))
        {
            if(aMarkRange.getMinX() < rWorkRange.getMinX())
            {
                rPos.setX(rPos.getX() + rWorkRange.getMinX() - aMarkRange.getMinX());
            }

            if(aMarkRange.getMaxX() > rWorkRange.getMaxX())
            {
                rPos.setX(rPos.getX() - aMarkRange.getMaxX() - rWorkRange.getMaxX());
            }

            if(aMarkRange.getMinY() < rWorkRange.getMinY())
            {
                rPos.setY(rPos.getY() + rWorkRange.getMinY() - aMarkRange.getMinY());
            }

            if(aMarkRange.getMaxY() > rWorkRange.getMaxY())
            {
                rPos.setY(rPos.getY() - aMarkRange.getMaxY() - rWorkRange.getMaxY());
            }
        }
    }
}

bool View::InsertMetaFile( TransferableDataHelper& rDataHelper, const basegfx::B2DPoint& rPos, ImageMap* pImageMap, bool bOptimize )
{
    GDIMetaFile aMtf;

    if( !rDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
        return false;

/*
SvFileStream    aSvOutputStream( String( RTL_CONSTASCII_USTRINGPARAM( "/tmp/test.png" ) ), STREAM_WRITE | STREAM_TRUNC );
Graphic         aMtfGraphic( aMtf );
Size            aPreviewSizePixel( OutputDevice::LogicToLogic( aMtf.GetPrefSize(), aMtf.GetPrefMapMode(), MAP_PIXEL ) );

if( aPreviewSizePixel.Width() && aPreviewSizePixel.Height() )
{
    const double fWH = static_cast< double >( aPreviewSizePixel.Width() ) / static_cast< double >( aPreviewSizePixel.Height() );

    if( fWH <= 1.0 )
        aPreviewSizePixel.Width() = static_cast< long >( 128.0 * fWH ), aPreviewSizePixel.Height() = 128;
    else
        aPreviewSizePixel.Width() = 128, aPreviewSizePixel.Height() = static_cast< long >( 128.0 / fWH );

    if( GraphicConverter::Export( aSvOutputStream, aMtfGraphic.GetBitmapEx( &aPreviewSizePixel ), CVT_PNG ) )
    {
        // handle errror case here
    }
    else
    {
        // Success
    }
}
*/
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

    // #90129# restrict movement to WorkArea
    const basegfx::B2DVector aImageSize(bVector
        ? basegfx::B2DVector(aMtf.GetPrefSize().Width(), aMtf.GetPrefSize().Height())
        : basegfx::B2DVector(aGraphic.GetSizePixel().Width(), aGraphic.GetSizePixel().Height()));
    basegfx::B2DPoint aInsertPos(rPos);

    ImpCheckInsertPos(aInsertPos, aImageSize, GetWorkArea());

    if( bVector )
        aGraphic = Graphic( aMtf );

    aGraphic.SetPrefMapMode( aMtf.GetPrefMapMode() );
    aGraphic.SetPrefSize( aMtf.GetPrefSize() );
    InsertGraphic( aGraphic, mnAction, aInsertPos, NULL, pImageMap );

    return true;
}

bool View::InsertData( const TransferableDataHelper& rDataHelper,
                         const basegfx::B2DPoint& rPos, sal_Int8& rDnDAction, bool bDrag,
                         sal_uInt32 nFormat, sal_uInt32 nPage, sal_uInt32 nLayer )
{
    maDropPos = rPos;
    mnAction = rDnDAction;
    mbIsDropAllowed = false;

    TransferableDataHelper  aDataHelper( rDataHelper );
    SdrObject*              pPickObj = NULL;
    SdPage*                 pPage = NULL;
    ImageMap*               pImageMap = NULL;
    bool                    bReturn = false;
    bool                    bLink = ( ( mnAction & DND_ACTION_LINK ) != 0 );
    bool                    bCopy = ( ( ( mnAction & DND_ACTION_COPY ) != 0 ) || bLink );
    sal_uInt32                  nPasteOptions = SDRINSERT_SETDEFLAYER;

    if (mpViewSh != NULL)
    {
        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
        SfxInPlaceClient* pIpClient = mpViewSh->GetViewShell()->GetIPClient();

        if( dynamic_cast< ::sd::slidesorter::SlideSorterViewShell* >(mpViewSh) || (pIpClient && pIpClient->IsObjectInPlaceActive()))
        {
            nPasteOptions |= SDRINSERT_DONTMARK;
        }
    }

    if( bDrag )
    {
        PickObj( rPos, getHitTolLog(), pPickObj );
    }

    if( nPage != SDRPAGE_NOTFOUND )
        pPage = (SdPage*) mpDoc->GetPage( nPage );

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

                ByteString aLine;
                while( xStm->ReadLine(aLine) )
                {
                    xub_StrLen x = aLine.Search( "\\trowd" );
                    if( x != STRING_NOTFOUND )
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


        if( pOwnData->GetDocShell() && pOwnData->IsPageTransferable() && dynamic_cast< View* >(this) )
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
                    SdrLayerAdmin&  rLayerAdmin = mpDoc->GetModelLayerAdmin();
                    SdrLayer*       pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                    SdrPageView*    pPV = GetSdrPageView();
                    String          aLayer( pLayer->GetName() );

                    if( pPV && !pPV->IsLayerLocked( aLayer ) )
                    {
                        pOwnData->SetInternalMove( true );
                        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

                        for( sal_uInt32 nM(0); nM < aSelection.size(); nM++ )
                        {
                            SdrObject* pO = aSelection[nM];

                                // #i11702#
                                if( IsUndoEnabled() )
                                {
                                    BegUndo(String(SdResId(STR_MODIFYLAYER)));
                                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectLayerChange(*pO, pO->GetLayer(), (SdrLayerID)nLayer));
                                    EndUndo();
                                }

                                pO->SetLayer( (SdrLayerID) nLayer );
                            }

                        bReturn = true;
                    }
                }
                else
                {
                    SdrPageView*    pPV = GetSdrPageView();
                    bool            bDropOnTabBar = true;

                    if( !pPage && pPV && pPV->getSdrPageFromSdrPageView().GetPageNumber() != mnDragSrcPgNum )
                    {
                        pPage = (SdPage*) &pPV->getSdrPageFromSdrPageView();
                        bDropOnTabBar = false;
                    }

                    if( pPage )
                    {
                        // drop on other page
                        String aActiveLayer( GetActiveLayer() );

                        if( pPV && !pPV->IsLayerLocked( aActiveLayer ) )
                        {
                            if( !IsPresObjSelected() )
                            {
                                SdrObjectVector aMarkList;

                                if( (mnDragSrcPgNum != SDRPAGE_NOTFOUND) && (mnDragSrcPgNum != pPV->getSdrPageFromSdrPageView().GetPageNumber()) )
                                {
                                    aMarkList = maDragSrcMarkList;
                                }
                                else
                                {
                                    // actual mark list is used
                                    aMarkList = getSelectedSdrObjectVectorFromSdrMarkView();
                                }


                                // #83525# stuff to remember originals and clones
                                Container   aConnectorContainer(0);
                                sal_uInt32  a, nConnectorCount(0L);
                                basegfx::B2DPoint aCurPos;

                                // calculate real position of current
                                // source objects, if necessary (#103207)
                                if( pOwnData == SD_MOD()->pTransferSelection )
                                {
                                    const basegfx::B2DRange aCurBoundRange(sdr::legacy::GetAllObjBoundRange(aMarkList));

                                    if(!aCurBoundRange.isEmpty())
                                    {
                                        aCurPos = aCurBoundRange.getMinimum();
                                    }
                                    else
                                    {
                                        aCurPos = pOwnData->GetStartPos();
                                    }
                                }
                                else
                                {
                                    aCurPos = pOwnData->GetStartPos();
                                }

                                const basegfx::B2DVector aVector(maDropPos - aCurPos);
                                const basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(aVector));

                                for(a = 0; a < aMarkList.size(); a++)
                                {
                                    SdrObject* pObj = aMarkList[a]->CloneSdrObject(&pPage->getSdrModelFromSdrPage());

                                    if(!bDropOnTabBar)
                                    {
                                        // #83525# do a NbcMove(...) instead of setting SnapRects here
                                        sdr::legacy::transformSdrObject(*pObj, aTransform);
                                    }

                                    pPage->InsertObjectToSdrObjList(*pObj);

                                    if( IsUndoEnabled() )
                                    {
                                        BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pObj));
                                        EndUndo();
                                    }

                                    // #83525#
                                    ImpRememberOrigAndClone* pRem = new ImpRememberOrigAndClone;
                                    pRem->pOrig = aMarkList[a];
                                    pRem->pClone = pObj;
                                    aConnectorContainer.Insert(pRem, CONTAINER_APPEND);

                                    if(pObj->IsSdrEdgeObj())
                                        nConnectorCount++;
                                }

                                // #83525# try to re-establish connections at clones
                                if(nConnectorCount)
                                {
                                    for(a = 0; a < aConnectorContainer.Count(); a++)
                                    {
                                        ImpRememberOrigAndClone* pRem = (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);
                                        SdrEdgeObj* pOrigEdge = dynamic_cast< SdrEdgeObj* >(pRem->pOrig);
                                        SdrEdgeObj* pCloneEdge = dynamic_cast< SdrEdgeObj* >(pRem->pClone);

                                        if(pOrigEdge && pCloneEdge)
                                        {
                                            // test first connection
                                            SdrObjConnection& rConn0 = pOrigEdge->GetConnection(false);
                                            SdrObject* pConnObj = rConn0.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(false, pConnClone);
                                                    pCloneEdge->GetConnection(false).SetConnectorId(rConn0.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt32 nInd = pGlueList->FindGluePoint(rConn0.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            basegfx::B2DPoint aPosition = rGluePoint.GetAbsolutePos(sdr::legacy::GetSnapRange(*pConnObj));
                                                            aPosition += aVector;
                                                            pCloneEdge->SetTailPoint(false, aPosition);
                                                        }
                                                    }
                                                }
                                            }

                                            // test second connection
                                            SdrObjConnection& rConn1 = pOrigEdge->GetConnection(true);
                                            pConnObj = rConn1.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(true, pConnClone);
                                                    pCloneEdge->GetConnection(true).SetConnectorId(rConn1.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt32 nInd = pGlueList->FindGluePoint(rConn1.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            basegfx::B2DPoint aPosition = rGluePoint.GetAbsolutePos(sdr::legacy::GetSnapRange(*pConnObj));
                                                            aPosition += aVector;
                                                            pCloneEdge->SetTailPoint(true, aPosition);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // #83525# cleanup remember classes
                                for(a = 0; a < aConnectorContainer.Count(); a++)
                                    delete (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);

                                bReturn = true;
                            }
                            else
                            {
                                maDropErrorTimer.Start();
                                bReturn = false;
                            }
                        }
                    }
                    else
                    {
                        pOwnData->SetInternalMove( true );
                        MoveMarkedObj(maDropPos - pOwnData->GetStartPos(), bCopy );
                        bReturn = true;
                    }
                }
            }
            else
            {
                // different views
                if( !pSourceView->IsPresObjSelected() )
                {
                    // model is owned by from AllocModel() created DocShell
                    SdDrawDocument& rSourceDoc = dynamic_cast< SdDrawDocument& >(pSourceView->getSdrModelFromSdrView());
                    rSourceDoc.CreatingDataObj( pOwnData );
                    SdDrawDocument* pModel = (SdDrawDocument*) pSourceView->GetAllMarkedModel();
                    bReturn = Paste( *pModel, maDropPos, pPage, nPasteOptions );

                    if( !pPage && GetSdrPageView() )
                        pPage = (SdPage*) &GetSdrPageView()->getSdrPageFromSdrPageView();

                    if(pPage)
                    {
                        String aLayout( pPage->GetLayoutName() );
                        aLayout.Erase( aLayout.SearchAscii( SD_LT_SEPARATOR ) );
                        pPage->SetPresentationLayout( aLayout, false, false );
                        rSourceDoc.CreatingDataObj( NULL );
                    }
                }
                else
                {
                    maDropErrorTimer.Start();
                    bReturn = false;
                }
            }
        }
        else
        {
            SdDrawDocument* pWorkModel = (SdDrawDocument*) pOwnData->GetWorkDocument();
            SdPage*         pWorkPage = (SdPage*) pWorkModel->GetSdPage( 0, PK_STANDARD );

            // #120393# Clipboard data uses full object geometry range
            const basegfx::B2DVector aOffset(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());
            maDropPos = pOwnData->GetStartPos() + (aOffset * 0.5);

            // delete pages, that are not of any interest for us
            for( long i = ( pWorkModel->GetPageCount() - 1 ); i >= 0; i-- )
            {
                SdPage* pP = static_cast< SdPage* >( pWorkModel->GetPage( (sal_uInt32) i ) );

                if( pP->GetPageKind() != PK_STANDARD )
                    pWorkModel->DeletePage( i );
            }

            bReturn = Paste( *pWorkModel, maDropPos, pPage, nPasteOptions );

            if( !pPage && GetSdrPageView() )
                pPage = (SdPage*) &GetSdrPageView()->getSdrPageFromSdrPageView();

            if(pPage)
            {
                String aLayout(pPage->GetLayoutName());
                aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
                pPage->SetPresentationLayout( aLayout, false, false );
            }
       }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_DRAWING ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
        {
            bool bChanged = false;

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
                DBG_ERROR("empty or invalid drawing xml document on clipboard!" );
            }
            else
            {
                if( bReturn )
                {
                    if(1 == pModel->GetSdPage( 0, PK_STANDARD )->GetObjCount())
                    {
                        // only one object
                        SdrObject*      pObj = pModel->GetSdPage( 0, PK_STANDARD )->GetObj( 0 );
                        SdrObject*      pPickObj2 = NULL;
                        PickObj( rPos, getHitTolLog(), pPickObj2 );

                        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj2 && pObj && GetSdrPageView() )
                        {
                            // replace object
                            SdrPage& rWorkPage = GetSdrPageView()->getSdrPageFromSdrPageView();
                            SdrObject* pNewObj = pObj->CloneSdrObject(&rWorkPage.getSdrModelFromSdrPage());

                            // copy transformation and layer
                            pNewObj->setSdrObjectTransformation(pPickObj2->getSdrObjectTransformation());
                            pNewObj->SetLayer( pPickObj2->GetLayer() );

                            const bool bUndo = IsUndoEnabled();

                            if( bUndo )
                                BegUndo( String( SdResId(STR_UNDO_DRAGDROP ) ) );

                            rWorkPage.InsertObjectToSdrObjList(*pNewObj);

                            if( bUndo )
                            {
                                AddUndo( mpDoc->GetSdrUndoFactory().CreateUndoNewObject( *pNewObj ) );
                                AddUndo( mpDoc->GetSdrUndoFactory().CreateUndoDeleteObject( *pPickObj2 ) );
                            }

                            rWorkPage.RemoveObjectFromSdrObjList( pPickObj2->GetNavigationPosition() );

                            if( bUndo )
                            {
                                EndUndo();
                            }
                            else
                            {
                                deleteSdrObjectSafeAndClearPointer(pPickObj2);
                            }

                            bChanged = true;
                            mnAction = DND_ACTION_COPY;
                        }
                        else if( ( mnAction & DND_ACTION_LINK ) && pPickObj && pObj
                            && !dynamic_cast< SdrGrafObj* >(pPickObj)
                            && !dynamic_cast< SdrOle2Obj* >(pPickObj) )
                        {
                            SfxItemSet aSet( mpDoc->GetItemPool() );

                            // set new attributes to object
                            const bool bUndo = IsUndoEnabled();
                            if( bUndo )
                            {
                                BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                                AddUndo( mpDoc->GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
                            }
                            aSet.Put( pObj->GetMergedItemSet() );

                            // Eckenradius soll nicht uebernommen werden.
                            // In der Gallery stehen Farbverlauefe (Rechtecke)
                            // welche den Eckenradius == 0 haben. Dieser soll
                            // nicht auf das Objekt uebertragen werden.
                            aSet.ClearItem( SDRATTR_ECKENRADIUS );

                            pPickObj->SetMergedItemSetAndBroadcast( aSet );

                            if( dynamic_cast< E3dObject* >(pPickObj) && dynamic_cast< E3dObject* >(pObj) )
                            {
                                // Zusaetzlich 3D Attribute handeln
                                SfxItemSet aNewSet( pObj->GetObjectItemPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );
                                SfxItemSet aOldSet( pPickObj->GetObjectItemPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );

                                aOldSet.Put(pPickObj->GetMergedItemSet());
                                aNewSet.Put( pObj->GetMergedItemSet() );

                                if( bUndo )
                                    AddUndo( new E3dAttributesUndoAction( *mpDoc, this, (E3dObject*) pPickObj, aNewSet, aOldSet, false ) );
                                pPickObj->SetMergedItemSetAndBroadcast( aNewSet );
                            }

                            if( bUndo )
                                EndUndo();
                            bChanged = true;
                        }
                    }
                }

                if( !bChanged )
                {
                    SdrPage* pWorkPage = pModel->GetSdPage( 0, PK_STANDARD );

                    if( pOwnData )
                    {
                        // #120393# Clipboard data uses full object geometry range
                        const basegfx::B2DVector aOffset(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());
                        maDropPos = pOwnData->GetStartPos() + (aOffset * 0.5);
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
                const basegfx::B2DVector& rScale = pObj->getSdrObjectScale();

                maDropPos -= absolute(rScale) * 0.5;

                pObj->setSdrObjectTransformation(
                    basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        rScale,
                        pObj->getSdrObjectShearX(),
                        pObj->getSdrObjectRotate(),
                        maDropPos));

                InsertObjectAtView( *pObj, SDRINSERT_SETDEFLAYER );
                bReturn = true;
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
            if( mpDoc->GetDocSh() && ( mpDoc->GetDocSh()->GetClassName() == aObjDesc.maClassName ) )
            {
                uno::Reference < embed::XStorage > xStore( ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm ) );
                ::sd::DrawDocShellRef xDocShRef( new ::sd::DrawDocShell( SFX_CREATE_MODE_EMBEDDED, true, mpDoc->GetDocumentType() ) );

                // mba: BaseURL doesn't make sense for clipboard functionality
                SfxMedium *pMedium = new SfxMedium( xStore, String() );
                if( xDocShRef->DoLoad( pMedium ) )
                {
                    SdDrawDocument* pModel = (SdDrawDocument*) xDocShRef->GetDoc();
                    SdPage*         pWorkPage = (SdPage*) pModel->GetSdPage( 0, PK_STANDARD );

                    if( pOwnData )
                    {
                        // #120393# Clipboard data uses full object geometry range
                        const basegfx::B2DVector aOffset(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());
                        maDropPos = pOwnData->GetStartPos() + (aOffset * 0.5);
                    }

                    // delete pages, that are not of any interest for us
                    for( long i = ( pModel->GetPageCount() - 1 ); i >= 0; i-- )
                    {
                        SdPage* pP = static_cast< SdPage* >( pModel->GetPage( (sal_uInt32) i ) );

                        if( pP->GetPageKind() != PK_STANDARD )
                            pModel->DeletePage( i );
                    }

                    bReturn = Paste( *pModel, maDropPos, pPage, nPasteOptions );

                    if( !pPage && GetSdrPageView() )
                        pPage = (SdPage*) &GetSdrPageView()->getSdrPageFromSdrPageView();

                    if(pPage)
                    {
                        String aLayout(pPage->GetLayoutName());
                        aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
                        pPage->SetPresentationLayout( aLayout, false, false );
                    }
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
                    sal_uInt32 nGrFormat = 0;

// (wg. Selection Manager bei Trustet Solaris)
#ifndef SOLARIS
/*
                    if( aDataHelper.GetGraphic( SOT_FORMATSTR_ID_SVXB, aGraphic ) )
                        nGrFormat = SOT_FORMATSTR_ID_SVXB;
                    else if( aDataHelper.GetGraphic( FORMAT_GDIMETAFILE, aGraphic ) )
                        nGrFormat = SOT_FORMAT_GDIMETAFILE;
                    else if( aDataHelper.GetGraphic( FORMAT_BITMAP, aGraphic ) )
                        nGrFormat = SOT_FORMAT_BITMAP;
*/
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

                    const basegfx::B2DVector aObjectSize(aSize.Width(), aSize.Height());
                    maDropPos -= basegfx::minimum(aObjectSize, mpDoc->GetMaxObjectScale()) * 0.5;
//                    maDropPos.setX(maDropPos.getX() - (Min( aSize.Width(), basegfx::fround(mpDoc->GetMaxObjectScale().getX()) ) >> 1));
//                    maDropPos.setY(maDropPos.getY() - (Min( aSize.Height(), basegfx::fround(mpDoc->GetMaxObjectScale().getY()) ) >> 1));

                    SdrOle2Obj* pObj = new SdrOle2Obj(
                        getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        basegfx::tools::createScaleTranslateB2DHomMatrix(aObjectSize, maDropPos));

                    sal_uInt32 nOptions = SDRINSERT_SETDEFLAYER;

                    if (mpViewSh!=NULL)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObjectAtView( *pObj, nOptions );

                    if( pImageMap )
                        pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                    if ( pObj && pObj->IsChart() )
                    {
                        bool bDisableDataTableDialog = false;
                        svt::EmbeddedObjectRef::TryRunningState( xObj );
                        uno::Reference< beans::XPropertySet > xProps( xObj->getComponent(), uno::UNO_QUERY );
                        if ( xProps.is() &&
                             ( xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableDataTableDialog" ) ) ) >>= bDisableDataTableDialog ) &&
                             bDisableDataTableDialog )
                        {
                            xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableDataTableDialog" ) ),
                                uno::makeAny( sal_False ) );
                            xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableComplexChartTypes" ) ),
                                uno::makeAny( sal_False ) );
                            uno::Reference< util::XModifiable > xModifiable( xProps, uno::UNO_QUERY );
                            if ( xModifiable.is() )
                            {
                                xModifiable->setModified( sal_True );
                            }
                        }
                    }

                    bReturn = true;
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
                            ::comphelper::getProcessServiceFactory()->createInstance(
                                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.MSOLEObjectSystemCreator")) ),
                            uno::UNO_QUERY_THROW );

                        embed::InsertedObjectInfo aInfo = xClipboardCreator->createInstanceInitFromClipboard(
                                                                xTmpStor,
                                                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "DummyName" ) ),
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
                    sal_uInt32 nGrFormat = 0;

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

                    const basegfx::B2DVector aObjectSize(aSize.Width(), aSize.Height());
                    maDropPos -= basegfx::minimum(aObjectSize, mpDoc->GetMaxObjectScale()) * 0.5;
//                    maDropPos.setX(maDropPos.getX() - (Min( aSize.Width(), basegfx::fround(mpDoc->GetMaxObjectScale().getX()) ) >> 1));
//                    maDropPos.setY(maDropPos.getY() - (Min( aSize.Height(), basegfx::fround(mpDoc->GetMaxObjectScale().getY()) ) >> 1));

                    SdrOle2Obj* pObj = new SdrOle2Obj(
                        getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        basegfx::tools::createScaleTranslateB2DHomMatrix(aObjectSize, maDropPos));

                    sal_uInt32 nOptions = SDRINSERT_SETDEFLAYER;

                    if (mpViewSh!=NULL)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObjectAtView( *pObj, nOptions );

                    if( pImageMap )
                        pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                    // let the object stay in loaded state after insertion
                    pObj->Unload();
                    bReturn = true;
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
            basegfx::B2DPoint aInsertPos(rPos);
            Graphic aGraphic;

            *xStm >> aGraphic;

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                // #120393# Clipboard data uses full object geometry range
                const basegfx::B2DVector aRange(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());

                aInsertPos = pOwnData->GetStartPos() + (aRange * 0.5);
            }

            // #90129# restrict movement to WorkArea
            const Size aImageMapSize(OutputDevice::LogicToLogic(
                aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), MapMode(MAP_100TH_MM)));

            ImpCheckInsertPos(aInsertPos, basegfx::B2DVector(aImageMapSize.Width(), aImageMapSize.Height()), GetWorkArea());

            InsertGraphic( aGraphic, mnAction, aInsertPos, NULL, pImageMap );
            bReturn = true;
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_GDIMETAFILE ) )
    {
        basegfx::B2DPoint aInsertPos( rPos );

        if( pOwnData && pOwnData->GetWorkDocument() )

        {
            const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
            SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                pWorkModel->GetPage( 0 ) );

            // #120393# Clipboard data uses full object geometry range
            const basegfx::B2DVector aRange(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());

            aInsertPos = pOwnData->GetStartPos() + (aRange * 0.5);
        }

        bReturn = InsertMetaFile( aDataHelper, aInsertPos, pImageMap, nFormat == 0 ? true : false ) ? true : false;
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_BITMAP ) )
    {
        Bitmap aBmp;

        if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ) )
        {
            basegfx::B2DPoint aInsertPos(rPos);

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                // #120393# Clipboard data uses full object geometry range
                const basegfx::B2DVector aSize(sdr::legacy::GetAllObjBoundRange(pWorkPage->getSdrObjectVector()).getRange());

                aInsertPos = pOwnData->GetStartPos() + (aSize * 0.5);
            }

            // #90129# restrict movement to WorkArea
            const basegfx::B2DVector aImageMapSize(aBmp.GetPrefSize().Width(), aBmp.GetPrefSize().Height());
            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aBmp, mnAction, aInsertPos, NULL, pImageMap );
            bReturn = true;
        }
    }
    else if( pPickObj && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_XFA ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) )
        {
            XFillExchangeData aFillData( XFillAttrSetItem( &mpDoc->GetItemPool() ) );

            *xStm >> aFillData;

            if( IsUndoEnabled() )
            {
                BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
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
                SfxItemSet              aSet( mpDoc->GetItemPool() );
                const bool bClosed(pPickObj->IsClosedObj());
                ::sd::Window* pWin = mpViewSh->GetActiveWindow();
                const double fHitLog(basegfx::B2DVector(pWin->GetInverseViewTransformation() * basegfx::B2DVector(FuPoor::HITPIX, 0.0)).getLength());
                double f2HitLog(fHitLog * 2);

                const basegfx::B2DPoint aHitPosR(rPos.getX() + f2HitLog, rPos.getY());
                const basegfx::B2DPoint aHitPosL(rPos.getX() - f2HitLog, rPos.getY());
                const basegfx::B2DPoint aHitPosT(rPos.getX(), rPos.getY() + f2HitLog);
                const basegfx::B2DPoint aHitPosB(rPos.getX(), rPos.getY() - f2HitLog);

                if( bClosed &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosR, fHitLog, *this, false, 0) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosL, fHitLog, *this, false, 0) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosT, fHitLog, *this, false, 0) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosB, fHitLog, *this, false, 0) )
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
                const Rectangle aRect(pOLV->GetOutputArea());
                const basegfx::B2DRange aOutputRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
                   const basegfx::B2DPoint aPos(pOLV->GetWindow()->GetInverseViewTransformation() * maDropPos);

                if( aOutputRange.isInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                {
                    // mba: clipboard always must contain absolute URLs (could be from alien source)
                    pOLV->Read( *xStm, String(), EE_FORMAT_BIN, false, mpDocSh->GetHeaderAttributes() );
                    bReturn = true;
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
                    const Rectangle aRect(pOLV->GetOutputArea());
                    const basegfx::B2DRange aOutputRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
                       const basegfx::B2DPoint aPos(pOLV->GetWindow()->GetInverseViewTransformation() * maDropPos);

                    if( aOutputRange.isInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                    {
                        // mba: clipboard always must contain absolute URLs (could be from alien source)
                        pOLV->Read( *xStm, String(), EE_FORMAT_RTF, false, mpDocSh->GetHeaderAttributes() );
                        bReturn = true;
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

            for( sal_uInt32 i = 0, nCount = aDropFileList.Count(); i < nCount; i++ )
                maDropFileVector.push_back( aDropFileList.GetFile( i ) );

            maDropInsertFileTimer.Start();
        }

        bReturn = true;
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

        bReturn = true;
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
                    bReturn = true;
                }

                if( !bReturn )
                    bReturn = SdrView::Paste( aOUString, maDropPos, pPage, nPasteOptions );
            }
        }
    }

    mbIsDropAllowed = true;
    rDnDAction = mnAction;
    delete pImageMap;

    return bReturn;
}

extern void CreateTableFromRTF( SvStream& rStream, SdDrawDocument& rModel  );

bool View::PasteRTFTable( SotStorageStreamRef xStm, SdrPage* pPage, sal_uInt32 nPasteOptions )
{
    SdDrawDocument* pModel = new SdDrawDocument( DOCUMENT_TYPE_IMPRESS, mpDocSh );
    pModel->NewOrLoadCompleted(NEW_DOC);
    pModel->GetItemPool().SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pModel->InsertPage(pModel->AllocPage(false));

    Reference< XComponent > xComponent( new SdXImpressDocument( pModel, sal_True ) );
    pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );

    CreateTableFromRTF( *xStm, *pModel );
    bool bRet = Paste( *pModel, maDropPos, pPage, nPasteOptions );

    xComponent->dispose();
    xComponent.clear();

    delete pModel;

    return bRet;
}

} // end of namespace sd
