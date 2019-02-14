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

#include <View.hxx>
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/MSOLEObjectSystemCreator.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <sot/filelist.hxx>
#include <unotools/pathoptions.hxx>
#include <editeng/editdata.hxx>
#include <svl/urlbmk.hxx>
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
#include <comphelper/classids.hxx>
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
#include <svtools/embedhlp.hxx>
#include <DrawDocShell.hxx>
#include <fupoor.hxx>
#include <tablefunction.hxx>
#include <Window.hxx>
#include <sdxfer.hxx>
#include <sdpage.hxx>
#include <DrawViewShell.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <imapinfo.hxx>
#include <SlideSorterViewShell.hxx>
#include <strmname.h>
#include <unomodel.hxx>
#include <ViewClipboard.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/classificationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/stream.hxx>
#include <vcl/cvtgrf.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/xbtmpit.hxx>
#include <memory>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

namespace sd {

#define CHECK_FORMAT_TRANS( _def_Type ) ( ( nFormat == (_def_Type) || nFormat == SotClipboardFormatId::NONE ) && aDataHelper.HasFormat( _def_Type ) )

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

static SdrObject* ImpGetClone(std::vector<ImpRememberOrigAndClone>& aConnectorContainer, SdrObject const * pConnObj)
{
    for(ImpRememberOrigAndClone& rImp : aConnectorContainer)
    {
        if(pConnObj == rImp.pOrig)
            return rImp.pClone;
    }
    return nullptr;
}

// restrict movement to WorkArea
static void ImpCheckInsertPos(Point& rPos, const Size& rSize, const ::tools::Rectangle& rWorkArea)
{
    if(rWorkArea.IsEmpty())
        return;

    ::tools::Rectangle aMarkRect(Point(rPos.X() - (rSize.Width() / 2), rPos.Y() - (rSize.Height() / 2)), rSize);

    if(aMarkRect.IsInside(rWorkArea))
        return;

    if(aMarkRect.Left() < rWorkArea.Left())
    {
        rPos.AdjustX(rWorkArea.Left() - aMarkRect.Left() );
    }

    if(aMarkRect.Right() > rWorkArea.Right())
    {
        rPos.AdjustX( -(aMarkRect.Right() - rWorkArea.Right()) );
    }

    if(aMarkRect.Top() < rWorkArea.Top())
    {
        rPos.AdjustY(rWorkArea.Top() - aMarkRect.Top() );
    }

    if(aMarkRect.Bottom() > rWorkArea.Bottom())
    {
        rPos.AdjustY( -(aMarkRect.Bottom() - rWorkArea.Bottom()) );
    }
}

bool View::InsertMetaFile( TransferableDataHelper& rDataHelper, const Point& rPos, ImageMap const * pImageMap, bool bOptimize )
{
    GDIMetaFile aMtf;

    if( !rDataHelper.GetGDIMetaFile( SotClipboardFormatId::GDIMETAFILE, aMtf ) )
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
                case MetaActionType::POINT:
                case MetaActionType::LINE:
                case MetaActionType::RECT:
                case MetaActionType::ROUNDRECT:
                case MetaActionType::ELLIPSE:
                case MetaActionType::ARC:
                case MetaActionType::PIE:
                case MetaActionType::CHORD:
                case MetaActionType::POLYLINE:
                case MetaActionType::POLYGON:
                case MetaActionType::POLYPOLYGON:
                case MetaActionType::TEXT:
                case MetaActionType::TEXTARRAY:
                case MetaActionType::STRETCHTEXT:
                case MetaActionType::TEXTRECT:
                case MetaActionType::GRADIENT:
                case MetaActionType::HATCH:
                case MetaActionType::WALLPAPER:
                case MetaActionType::EPS:
                case MetaActionType::TEXTLINE:
                case MetaActionType::FLOATTRANSPARENT:
                case MetaActionType::GRADIENTEX:
                case MetaActionType::BMPSCALEPART:
                case MetaActionType::BMPEXSCALEPART:
                    bVector = true;
                    break;
                case MetaActionType::BMP:
                case MetaActionType::BMPSCALE:
                case MetaActionType::BMPEX:
                case MetaActionType::BMPEXSCALE:
                    if( aGraphic.GetType() != GraphicType::NONE )
                    {
                        bVector = true;
                    }
                    else switch( pAction->GetType() )
                    {
                        case MetaActionType::BMP:
                            {
                                MetaBmpAction* pBmpAction = dynamic_cast< MetaBmpAction* >( pAction );
                                if( pBmpAction )
                                    aGraphic = Graphic( pBmpAction->GetBitmap() );
                            }
                            break;
                        case MetaActionType::BMPSCALE:
                            {
                                MetaBmpScaleAction* pBmpScaleAction = dynamic_cast< MetaBmpScaleAction* >( pAction );
                                if( pBmpScaleAction )
                                    aGraphic = Graphic( pBmpScaleAction->GetBitmap() );
                            }
                            break;
                        case MetaActionType::BMPEX:
                            {
                                MetaBmpExAction* pBmpExAction = dynamic_cast< MetaBmpExAction* >( pAction );
                                if( pBmpExAction )
                                    aGraphic = Graphic( pBmpExAction->GetBitmapEx() );
                            }
                            break;
                        case MetaActionType::BMPEXSCALE:
                            {
                                MetaBmpExScaleAction* pBmpExScaleAction = dynamic_cast< MetaBmpExScaleAction* >( pAction );
                                if( pBmpExScaleAction )
                                    aGraphic = Graphic( pBmpExScaleAction->GetBitmapEx() );
                            }
                            break;
                        default: break;
                    }
                    break;
                default: break;
            }

            pAction = aMtf.NextAction();
        }
    }

    // it is not a vector metafile but it also has no graphic?
    if( !bVector && (aGraphic.GetType() == GraphicType::NONE) )
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
    InsertGraphic( aGraphic, mnAction, aInsertPos, nullptr, pImageMap );

    return true;
}

bool View::InsertData( const TransferableDataHelper& rDataHelper,
                         const Point& rPos, sal_Int8& rDnDAction, bool bDrag,
                         SotClipboardFormatId nFormat, sal_uInt16 nPage, SdrLayerID nLayer )
{
    maDropPos = rPos;
    mnAction = rDnDAction;
    mbIsDropAllowed = false;

    TransferableDataHelper  aDataHelper( rDataHelper );
    SdrObject*              pPickObj = nullptr;
    SdPage*                 pPage = nullptr;
    std::unique_ptr<ImageMap> pImageMap;
    bool bReturn = false;
    bool                    bLink = ( ( mnAction & DND_ACTION_LINK ) != 0 );
    bool                    bCopy = ( ( ( mnAction & DND_ACTION_COPY ) != 0 ) || bLink );
    SdrInsertFlags          nPasteOptions = SdrInsertFlags::SETDEFLAYER;

    if (mpViewSh != nullptr)
    {
        OSL_ASSERT (mpViewSh->GetViewShell()!=nullptr);
        SfxInPlaceClient* pIpClient = mpViewSh->GetViewShell()->GetIPClient();
        if( dynamic_cast< ::sd::slidesorter::SlideSorterViewShell *>( mpViewSh ) !=  nullptr
            || (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive()))
        nPasteOptions |= SdrInsertFlags::DONTMARK;
    }

    if( bDrag )
    {
        SdrPageView* pPV = nullptr;
        pPickObj = PickObj(rPos, getHitTolLog(), pPV);
    }

    if( nPage != SDRPAGE_NOTFOUND )
        pPage = static_cast<SdPage*>( mrDoc.GetPage( nPage ) );

    SdTransferable* pOwnData = nullptr;
    SdTransferable* pImplementation = SdTransferable::getImplementation( aDataHelper.GetTransferable() );

    if(pImplementation && (rDnDAction & DND_ACTION_LINK))
    {
        // suppress own data when it's intention is to use it as fill information
        pImplementation = nullptr;
    }

    // try to get own transfer data
    if( pImplementation )
    {
        if( SD_MOD()->pTransferClip == pImplementation )
            pOwnData = SD_MOD()->pTransferClip;
        else if( SD_MOD()->pTransferDrag == pImplementation )
            pOwnData = SD_MOD()->pTransferDrag;
        else if( SD_MOD()->pTransferSelection == pImplementation )
            pOwnData = SD_MOD()->pTransferSelection;
    }

    // ImageMap?
    if( !pOwnData && aDataHelper.HasFormat( SotClipboardFormatId::SVIM ) )
    {
        ::tools::SvRef<SotStorageStream> xStm;

        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::SVIM, xStm ) )
        {
            pImageMap.reset(new ImageMap);
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            pImageMap->Read( *xStm );
        }
    }

    bool bTable = false;
    // check special cases for pasting table formats as RTL
    if( !bLink && (nFormat == SotClipboardFormatId::NONE || (nFormat == SotClipboardFormatId::RTF) || (nFormat == SotClipboardFormatId::RICHTEXT)) )
    {
        // if the object supports rtf and there is a table involved, default is to create a table
        bool bIsRTF = aDataHelper.HasFormat( SotClipboardFormatId::RTF );
        if( ( bIsRTF || aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) )
            && ! aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ) )
        {
            ::tools::SvRef<SotStorageStream> xStm;

            if( aDataHelper.GetSotStorageStream( bIsRTF ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT, xStm ) )
            {
                xStm->Seek( 0 );

                OString aLine;
                while (xStm->ReadLine(aLine))
                {
                    sal_Int32 x = aLine.indexOf( "\\trowd" );
                    if (x != -1)
                    {
                        bTable = true;
                        nFormat = bIsRTF ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT;
                        break;
                    }
                }
            }
        }
    }

    // Changed the whole decision tree to be dependent of bReturn as a flag that
    // the work was done; this allows to check multiple formats and not just fail
    // when a CHECK_FORMAT_TRANS(*format*) detected format does not work. This is
    // e.g. necessary for SotClipboardFormatId::BITMAP

    if (!bReturn && pOwnData)
    {
        // Paste only if SfxClassificationHelper recommends so.
        const SfxObjectShellRef& pSource = pOwnData->GetDocShell();
        SfxObjectShell* pDestination = mrDoc.GetDocSh();
        if (pSource.is() && pDestination)
        {
            SfxClassificationCheckPasteResult eResult = SfxClassificationHelper::CheckPaste(pSource->getDocProperties(), pDestination->getDocProperties());
            if (!SfxClassificationHelper::ShowPasteInfo(eResult))
                bReturn = true;
        }
    }

    if( !bReturn && pOwnData && nFormat == SotClipboardFormatId::NONE )
    {
        const View* pSourceView = pOwnData->GetView();

        if( pOwnData->GetDocShell().is() && pOwnData->IsPageTransferable() )
        {
            mpClipboard->HandlePageDrop (*pOwnData);
            bReturn = true;
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
                    OUString        aLayer = pLayer->GetName();

                    if( !pPV->IsLayerLocked( aLayer ) )
                    {
                        pOwnData->SetInternalMove( true );
                        SortMarkedObjects();

                        for( size_t nM = 0; nM < GetMarkedObjectCount(); ++nM )
                        {
                            SdrMark*    pM = GetSdrMarkByIndex( nM );
                            SdrObject*  pO = pM->GetMarkedSdrObj();

                            if( pO )
                            {
                                // #i11702#
                                if( IsUndoEnabled() )
                                {
                                    BegUndo(SdResId(STR_MODIFYLAYER));
                                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectLayerChange(*pO, pO->GetLayer(), nLayer));
                                    EndUndo();
                                }

                                pO->SetLayer( nLayer );
                            }
                        }

                        bReturn = true;
                    }
                }
                else
                {
                    SdrPageView*    pPV = GetSdrPageView();
                    bool            bDropOnTabBar = true;

                    if( !pPage && pPV->GetPage()->GetPageNum() != mnDragSrcPgNum )
                    {
                        pPage = static_cast<SdPage*>( pPV->GetPage() );
                        bDropOnTabBar = false;
                    }

                    if( pPage )
                    {
                        // drop on other page
                        OUString aActiveLayer = GetActiveLayer();

                        if( !pPV->IsLayerLocked( aActiveLayer ) )
                        {
                            if( !IsPresObjSelected() )
                            {
                                SdrMarkList* pMarkList;

                                if( (mnDragSrcPgNum != SDRPAGE_NOTFOUND) && (mnDragSrcPgNum != pPV->GetPage()->GetPageNum()) )
                                {
                                    pMarkList = mpDragSrcMarkList.get();
                                }
                                else
                                {
                                    // actual mark list is used
                                    pMarkList = new SdrMarkList( GetMarkedObjectList());
                                }

                                pMarkList->ForceSort();

                                // stuff to remember originals and clones
                                std::vector<ImpRememberOrigAndClone> aConnectorContainer;
                                size_t nConnectorCount = 0;
                                Point       aCurPos;

                                // calculate real position of current
                                // source objects, if necessary (#103207)
                                if( pOwnData == SD_MOD()->pTransferSelection )
                                {
                                    ::tools::Rectangle aCurBoundRect;

                                    if( pMarkList->TakeBoundRect( pPV, aCurBoundRect ) )
                                        aCurPos = aCurBoundRect.TopLeft();
                                    else
                                        aCurPos = pOwnData->GetStartPos();
                                }
                                else
                                    aCurPos = pOwnData->GetStartPos();

                                const Size aVector( maDropPos.X() - aCurPos.X(), maDropPos.Y() - aCurPos.Y() );

                                for(size_t a = 0; a < pMarkList->GetMarkCount(); ++a)
                                {
                                    SdrMark* pM = pMarkList->GetMark(a);
                                    SdrObject* pObj(pM->GetMarkedSdrObj()->CloneSdrObject(pPage->getSdrModelFromSdrPage()));

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
                                            BegUndo(SdResId(STR_UNDO_DRAGDROP));
                                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));
                                            EndUndo();
                                        }

                                        ImpRememberOrigAndClone aRem;
                                        aRem.pOrig = pM->GetMarkedSdrObj();
                                        aRem.pClone = pObj;
                                        aConnectorContainer.push_back(aRem);

                                        if(dynamic_cast< SdrEdgeObj *>( pObj ) !=  nullptr)
                                            nConnectorCount++;
                                    }
                                }

                                // try to re-establish connections at clones
                                if(nConnectorCount)
                                {
                                    for(size_t a = 0; a < aConnectorContainer.size(); ++a)
                                    {
                                        ImpRememberOrigAndClone* pRem = &aConnectorContainer[a];

                                        if(dynamic_cast< const SdrEdgeObj *>( pRem->pClone ) !=  nullptr)
                                        {
                                            SdrEdgeObj* pOrigEdge = static_cast<SdrEdgeObj*>(pRem->pOrig);
                                            SdrEdgeObj* pCloneEdge = static_cast<SdrEdgeObj*>(pRem->pClone);

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
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn0.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.AdjustX(aVector.Width() );
                                                            aPosition.AdjustY(aVector.Height() );
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
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn1.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.AdjustX(aVector.Width() );
                                                            aPosition.AdjustY(aVector.Height() );
                                                            pCloneEdge->SetTailPoint(true, aPosition);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if( pMarkList != mpDragSrcMarkList.get() )
                                    delete pMarkList;

                                bReturn = true;
                            }
                            else
                            {
                                maDropErrorIdle.Start();
                                bReturn = false;
                            }
                        }
                    }
                    else
                    {
                        pOwnData->SetInternalMove( true );
                        MoveAllMarked( Size( maDropPos.X() - pOwnData->GetStartPos().X(),
                                             maDropPos.Y() - pOwnData->GetStartPos().Y() ), bCopy );
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
                    SdDrawDocument* pSourceDoc = static_cast<SdDrawDocument*>( pSourceView->GetModel() );
                    pSourceDoc->CreatingDataObj( pOwnData );
                    SdDrawDocument* pModel = static_cast<SdDrawDocument*>( pSourceView->CreateMarkedObjModel().release() );
                    bReturn = Paste(*pModel, maDropPos, pPage, nPasteOptions);

                    if( !pPage )
                        pPage = static_cast<SdPage*>( GetSdrPageView()->GetPage() );

                    OUString aLayout = pPage->GetLayoutName();
                    sal_Int32 nPos = aLayout.indexOf(SD_LT_SEPARATOR);
                    if (nPos != -1)
                        aLayout = aLayout.copy(0, nPos);
                    pPage->SetPresentationLayout( aLayout, false, false );
                    pSourceDoc->CreatingDataObj( nullptr );
                }
                else
                {
                    maDropErrorIdle.Start();
                    bReturn = false;
                }
            }
        }
        else
        {
            SdDrawDocument* pWorkModel = const_cast<SdDrawDocument*>(pOwnData->GetWorkDocument());
            SdPage*         pWorkPage = pWorkModel->GetSdPage( 0, PageKind::Standard );

            pWorkPage->SetSdrObjListRectsDirty();

            // #i120393# Clipboard data uses full object geometry range
            const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

            maDropPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
            maDropPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );

            // delete pages, that are not of any interest for us
            for( long i = pWorkModel->GetPageCount() - 1; i >= 0; i-- )
            {
                SdPage* pP = static_cast< SdPage* >( pWorkModel->GetPage( static_cast<sal_uInt16>(i) ) );

                if( pP->GetPageKind() != PageKind::Standard )
                    pWorkModel->DeletePage( static_cast<sal_uInt16>(i) );
            }

            bReturn = Paste(*pWorkModel, maDropPos, pPage, nPasteOptions);

            if( !pPage )
                pPage = static_cast<SdPage*>( GetSdrPageView()->GetPage() );

            OUString aLayout = pPage->GetLayoutName();
            sal_Int32 nPos = aLayout.indexOf(SD_LT_SEPARATOR);
            if (nPos != -1)
                aLayout = aLayout.copy(0, nPos);
            pPage->SetPresentationLayout( aLayout, false, false );
       }
    }

    if(!bReturn && CHECK_FORMAT_TRANS( SotClipboardFormatId::DRAWING ))
    {
        ::tools::SvRef<SotStorageStream> xStm;

        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::DRAWING, xStm ) )
        {
            DrawDocShellRef xShell = new DrawDocShell(SfxObjectCreateMode::INTERNAL, false, DocumentType::Impress);
            xShell->DoInitNew();

            SdDrawDocument* pModel = xShell->GetDoc();
            pModel->InsertPage(pModel->AllocPage(false));

            Reference< XComponent > xComponent( xShell->GetModel(), UNO_QUERY );
            xStm->Seek( 0 );

            css::uno::Reference< css::io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ) );
            bReturn = SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Impress.XMLOasisImporter" );

            if( pModel->GetPageCount() == 0 )
            {
                OSL_FAIL("empty or invalid drawing xml document on clipboard!" );
            }
            else
            {
                bool bChanged = false;

                if( bReturn )
                {
                    if( pModel->GetSdPage( 0, PageKind::Standard )->GetObjCount() == 1 )
                    {
                        // only one object
                        SdrObject*      pObj = pModel->GetSdPage( 0, PageKind::Standard )->GetObj( 0 );
                        SdrPageView*    pPV = nullptr;
                        SdrObject* pPickObj2 = PickObj(rPos, getHitTolLog(), pPV);

                        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj2 && pObj )
                        {
                            // replace object
                            SdrPage* pWorkPage = GetSdrPageView()->GetPage();
                            SdrObject* pNewObj(pObj->CloneSdrObject(pWorkPage->getSdrModelFromSdrPage()));
                            ::tools::Rectangle   aPickObjRect( pPickObj2->GetCurrentBoundRect() );
                            Size        aPickObjSize( aPickObjRect.GetSize() );
                            Point       aVec( aPickObjRect.TopLeft() );
                            ::tools::Rectangle   aObjRect( pNewObj->GetCurrentBoundRect() );
                            Size        aObjSize( aObjRect.GetSize() );

                            Fraction aScaleWidth( aPickObjSize.Width(), aObjSize.Width() );
                            Fraction aScaleHeight( aPickObjSize.Height(), aObjSize.Height() );
                            pNewObj->NbcResize( aObjRect.TopLeft(), aScaleWidth, aScaleHeight );

                            aVec -= aObjRect.TopLeft();
                            pNewObj->NbcMove( Size( aVec.X(), aVec.Y() ) );

                            const bool bUndo = IsUndoEnabled();

                            if( bUndo )
                                BegUndo(SdResId(STR_UNDO_DRAGDROP));
                            pNewObj->NbcSetLayer( pPickObj->GetLayer() );
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
                            bChanged = true;
                            mnAction = DND_ACTION_COPY;
                        }
                        else if( ( mnAction & DND_ACTION_LINK ) && pPickObj && pObj &&
                            dynamic_cast< const SdrGrafObj *>( pPickObj ) ==  nullptr &&
                                dynamic_cast< const SdrOle2Obj *>( pPickObj ) ==  nullptr )
                        {
                            SfxItemSet aSet( mrDoc.GetPool() );

                            // set new attributes to object
                            const bool bUndo = IsUndoEnabled();
                            if( bUndo )
                            {
                                BegUndo( SdResId(STR_UNDO_DRAGDROP) );
                                AddUndo( mrDoc.GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
                            }

                            aSet.Put( pObj->GetMergedItemSet() );

                            /* Do not take over corner radius. There are
                               gradients (rectangles) in the gallery with corner
                               radius of 0. We should not use that on the
                               object. */
                            aSet.ClearItem( SDRATTR_ECKENRADIUS );

                            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);

                            if(pSdrGrafObj)
                            {
                                // If we have a graphic as source object, use its graphic
                                // content as fill style
                                aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                                aSet.Put(XFillBitmapItem(pSdrGrafObj->GetGraphic()));
                            }

                            pPickObj->SetMergedItemSetAndBroadcast( aSet );

                            if( dynamic_cast< E3dObject *>( pPickObj ) !=  nullptr && dynamic_cast< E3dObject *>( pObj ) !=  nullptr )
                            {
                                // handle 3D attribute in addition
                                SfxItemSet aNewSet( mrDoc.GetPool(), svl::Items<SID_ATTR_3D_START, SID_ATTR_3D_END>{} );
                                SfxItemSet aOldSet( mrDoc.GetPool(), svl::Items<SID_ATTR_3D_START, SID_ATTR_3D_END>{} );

                                aOldSet.Put(pPickObj->GetMergedItemSet());
                                aNewSet.Put( pObj->GetMergedItemSet() );

                                if( bUndo )
                                    AddUndo(
                                        std::make_unique<E3dAttributesUndoAction>(
                                            *static_cast< E3dObject* >(pPickObj),
                                            aNewSet,
                                            aOldSet));
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
                    SdrPage* pWorkPage = pModel->GetSdPage( 0, PageKind::Standard );

                    pWorkPage->SetSdrObjListRectsDirty();

                    if( pOwnData )
                    {
                        // #i120393# Clipboard data uses full object geometry range
                        const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

                        maDropPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
                        maDropPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );
                    }

                    bReturn = Paste(*pModel, maDropPos, pPage, nPasteOptions);
                }

                xShell->DoClose();
            }
        }
    }

    if(!bReturn && CHECK_FORMAT_TRANS(SotClipboardFormatId::SBA_FIELDDATAEXCHANGE))
    {
        OUString aOUString;

        if( aDataHelper.GetString( SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, aOUString ) )
        {
            SdrObjectUniquePtr pObj = CreateFieldControl( aOUString );

            if( pObj )
            {
                ::tools::Rectangle   aRect( pObj->GetLogicRect() );
                Size        aSize( aRect.GetSize() );

                maDropPos.AdjustX( -( aSize.Width() >> 1 ) );
                maDropPos.AdjustY( -( aSize.Height() >> 1 ) );

                aRect.SetPos( maDropPos );
                pObj->SetLogicRect( aRect );
                InsertObjectAtView( pObj.release(), *GetSdrPageView(), SdrInsertFlags::SETDEFLAYER );
                bReturn = true;
            }
        }
    }

    if(!bReturn &&
        !bLink &&
        (CHECK_FORMAT_TRANS(SotClipboardFormatId::EMBED_SOURCE) || CHECK_FORMAT_TRANS(SotClipboardFormatId::EMBEDDED_OBJ))  &&
        aDataHelper.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        //TODO/LATER: is it possible that this format is binary?! (from old versions of SO)
        uno::Reference < io::XInputStream > xStm;
        TransferableObjectDescriptor    aObjDesc;

        if (aDataHelper.GetTransferableObjectDescriptor(SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc))
        {
            OUString aDocShellID = SfxObjectShell::CreateShellID(mrDoc.GetDocSh());
            xStm = aDataHelper.GetInputStream(nFormat != SotClipboardFormatId::NONE ? nFormat : SotClipboardFormatId::EMBED_SOURCE, aDocShellID);
            if (!xStm.is())
                xStm = aDataHelper.GetInputStream(SotClipboardFormatId::EMBEDDED_OBJ, aDocShellID);
        }

        if (xStm.is())
        {
            if( mrDoc.GetDocSh() && ( mrDoc.GetDocSh()->GetClassName() == aObjDesc.maClassName ) )
            {
                uno::Reference < embed::XStorage > xStore( ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm ) );
                ::sd::DrawDocShellRef xDocShRef( new ::sd::DrawDocShell( SfxObjectCreateMode::EMBEDDED, true, mrDoc.GetDocumentType() ) );

                // mba: BaseURL doesn't make sense for clipboard functionality
                SfxMedium *pMedium = new SfxMedium( xStore, OUString() );
                if( xDocShRef->DoLoad( pMedium ) )
                {
                    SdDrawDocument* pModel = xDocShRef->GetDoc();
                    SdPage*         pWorkPage = pModel->GetSdPage( 0, PageKind::Standard );

                    pWorkPage->SetSdrObjListRectsDirty();

                    if( pOwnData )
                    {
                        // #i120393# Clipboard data uses full object geometry range
                        const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

                        maDropPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
                        maDropPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );
                    }

                    // delete pages, that are not of any interest for us
                    for( long i = pModel->GetPageCount() - 1; i >= 0; i-- )
                    {
                        SdPage* pP = static_cast< SdPage* >( pModel->GetPage( static_cast<sal_uInt16>(i) ) );

                        if( pP->GetPageKind() != PageKind::Standard )
                            pModel->DeletePage( static_cast<sal_uInt16>(i) );
                    }

                    bReturn = Paste(*pModel, maDropPos, pPage, nPasteOptions);

                    if( !pPage )
                        pPage = static_cast<SdPage*>(GetSdrPageView()->GetPage());

                    OUString aLayout = pPage->GetLayoutName();
                    sal_Int32 nPos = aLayout.indexOf(SD_LT_SEPARATOR);
                    if (nPos != -1)
                        aLayout = aLayout.copy(0, nPos);
                    pPage->SetPresentationLayout( aLayout, false, false );
                }

                xDocShRef->DoClose();
                xDocShRef.clear();

            }
            else
            {
                OUString aName;
                uno::Reference < embed::XEmbeddedObject > xObj = mpDocSh->GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                if ( xObj.is() )
                {
                    svt::EmbeddedObjectRef aObjRef( xObj, aObjDesc.mnViewAspect );

                    Size aSize;
                    if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
                    {
                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                            aSize = aObjDesc.maSize;
                        else
                        {
                            MapMode aMapMode( MapUnit::Map100thMM );
                            aSize = aObjRef.GetSize( &aMapMode );
                        }
                    }
                    else
                    {
                        awt::Size aSz;
                        MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( aObjDesc.mnViewAspect ) );
                        if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                        {
                            Size aTmp(OutputDevice::LogicToLogic(aObjDesc.maSize, MapMode(MapUnit::Map100thMM), MapMode(aMapUnit)));
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
                            aSize.setWidth( 14100 );
                            aSize.setHeight( 10000 );
                            aSize = OutputDevice::LogicToLogic(Size(14100, 10000), MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                            aSz.Width = aSize.Width();
                            aSz.Height = aSize.Height();
                            xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                        }

                        aSize = OutputDevice::LogicToLogic(aSize, MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));
                    }

                    Size aMaxSize( mrDoc.GetMaxObjSize() );

                    maDropPos.AdjustX( -(std::min( aSize.Width(), aMaxSize.Width() ) >> 1) );
                    maDropPos.AdjustY( -(std::min( aSize.Height(), aMaxSize.Height() ) >> 1) );

                    ::tools::Rectangle       aRect( maDropPos, aSize );
                    SdrOle2Obj*     pObj = new SdrOle2Obj(
                        getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        aRect);
                    SdrPageView*    pPV = GetSdrPageView();
                    SdrInsertFlags  nOptions = SdrInsertFlags::SETDEFLAYER;

                    if (mpViewSh!=nullptr)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=nullptr);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SdrInsertFlags::DONTMARK;
                    }

                    InsertObjectAtView( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new SdIMapInfo( *pImageMap )) );

                    if (pObj->IsChart())
                    {
                        bool bDisableDataTableDialog = false;
                        svt::EmbeddedObjectRef::TryRunningState( xObj );
                        uno::Reference< beans::XPropertySet > xProps( xObj->getComponent(), uno::UNO_QUERY );
                        if ( xProps.is() &&
                             ( xProps->getPropertyValue( "DisableDataTableDialog" ) >>= bDisableDataTableDialog ) &&
                             bDisableDataTableDialog )
                        {
                            xProps->setPropertyValue( "DisableDataTableDialog" , uno::makeAny( false ) );
                            xProps->setPropertyValue( "DisableComplexChartTypes" , uno::makeAny( false ) );
                            uno::Reference< util::XModifiable > xModifiable( xProps, uno::UNO_QUERY );
                            if ( xModifiable.is() )
                            {
                                xModifiable->setModified( true );
                            }
                        }
                    }

                    bReturn = true;
                }
            }
        }
    }

    if(!bReturn &&
        !bLink &&
        (CHECK_FORMAT_TRANS(SotClipboardFormatId::EMBEDDED_OBJ_OLE) || CHECK_FORMAT_TRANS(SotClipboardFormatId::EMBED_SOURCE_OLE)) &&
        aDataHelper.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR_OLE))
    {
        // online insert ole if format is forced or no gdi metafile is available
        if( (nFormat != SotClipboardFormatId::NONE) || !aDataHelper.HasFormat( SotClipboardFormatId::GDIMETAFILE ) )
        {
            uno::Reference < io::XInputStream > xStm;
            TransferableObjectDescriptor    aObjDesc;

            if ( aDataHelper.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR_OLE, aObjDesc ) )
            {
                uno::Reference < embed::XEmbeddedObject > xObj;
                OUString aName;

                xStm = aDataHelper.GetInputStream(nFormat != SotClipboardFormatId::NONE ? nFormat : SotClipboardFormatId::EMBED_SOURCE_OLE, OUString());
                if (!xStm.is())
                    xStm = aDataHelper.GetInputStream(SotClipboardFormatId::EMBEDDED_OBJ_OLE, OUString());

                if (xStm.is())
                {
                    xObj = mpDocSh->GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                }
                else
                {
                    try
                    {
                        uno::Reference< embed::XStorage > xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        uno::Reference < embed::XEmbedObjectClipboardCreator > xClipboardCreator =
                            embed::MSOLEObjectSystemCreator::create( ::comphelper::getProcessComponentContext() );

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
                    SotClipboardFormatId nGrFormat = SotClipboardFormatId::NONE;

// (for Selection Manager in Trusted Solaris)
#ifndef __sun
                    if( aDataHelper.GetGraphic( SotClipboardFormatId::SVXB, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::SVXB;
                    else if( aDataHelper.GetGraphic( SotClipboardFormatId::GDIMETAFILE, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::GDIMETAFILE;
                    else if( aDataHelper.GetGraphic( SotClipboardFormatId::BITMAP, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::BITMAP;
#endif

                    // insert replacement image ( if there is one ) into the object helper
                    if ( nGrFormat != SotClipboardFormatId::NONE )
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
                            MapMode aMapMode( MapUnit::Map100thMM );
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
                            Size aTmp(OutputDevice::LogicToLogic(aObjDesc.maSize, MapMode(MapUnit::Map100thMM), MapMode(aMapUnit)));
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
                            aSize = OutputDevice::LogicToLogic(Size(14100, 10000), MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                            aSz.Width = aSize.Width();
                            aSz.Height = aSize.Height();
                            xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                        }

                        aSize = OutputDevice::LogicToLogic(aSize, MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));
                    }

                    Size aMaxSize( mrDoc.GetMaxObjSize() );

                    maDropPos.AdjustX( -(std::min( aSize.Width(), aMaxSize.Width() ) >> 1) );
                    maDropPos.AdjustY( -(std::min( aSize.Height(), aMaxSize.Height() ) >> 1) );

                    ::tools::Rectangle       aRect( maDropPos, aSize );
                    SdrOle2Obj*     pObj = new SdrOle2Obj(
                        getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        aRect);
                    SdrPageView*    pPV = GetSdrPageView();
                    SdrInsertFlags  nOptions = SdrInsertFlags::SETDEFLAYER;

                    if (mpViewSh!=nullptr)
                    {
                        OSL_ASSERT (mpViewSh->GetViewShell()!=nullptr);
                        SfxInPlaceClient* pIpClient
                            = mpViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
                            nOptions |= SdrInsertFlags::DONTMARK;
                    }

                    InsertObjectAtView( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new SdIMapInfo( *pImageMap )) );

                    // let the object stay in loaded state after insertion
                    pObj->Unload();
                    bReturn = true;
                }
            }
        }

        if( !bReturn && aDataHelper.HasFormat( SotClipboardFormatId::GDIMETAFILE ) )
        {
            // if no object was inserted, insert a picture
            InsertMetaFile( aDataHelper, rPos, pImageMap.get(), true );
            bReturn = true;
        }
    }

    if(!bReturn && (!bLink || pPickObj) && CHECK_FORMAT_TRANS(SotClipboardFormatId::SVXB))
    {
        ::tools::SvRef<SotStorageStream> xStm;

        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::SVXB, xStm ) )
        {
            Point   aInsertPos( rPos );
            Graphic aGraphic;

            ReadGraphic( *xStm, aGraphic );

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = const_cast<SdrPage*>( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PageKind::Standard ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetSdrObjListRectsDirty();

                // #i120393# Clipboard data uses full object geometry range
                const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

                aInsertPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
                aInsertPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );
            }

            // restrict movement to WorkArea
            Size aImageMapSize = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(),
                aGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM));

            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aGraphic, mnAction, aInsertPos, nullptr, pImageMap.get() );
            bReturn = true;
        }
    }

    if(!bReturn && (!bLink || pPickObj) && CHECK_FORMAT_TRANS(SotClipboardFormatId::GDIMETAFILE))
    {
        Point aInsertPos( rPos );

        if( pOwnData && pOwnData->GetWorkDocument() )

        {
            const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
            SdrPage*                pWorkPage = const_cast<SdrPage*>( ( pWorkModel->GetPageCount() > 1 ) ?
                                                pWorkModel->GetSdPage( 0, PageKind::Standard ) :
                                                pWorkModel->GetPage( 0 ) );

            pWorkPage->SetSdrObjListRectsDirty();

            // #i120393# Clipboard data uses full object geometry range
            const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

            aInsertPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
            aInsertPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );
        }

        bReturn = InsertMetaFile( aDataHelper, aInsertPos, pImageMap.get(), nFormat == SotClipboardFormatId::NONE );
    }

    if(!bReturn && (!bLink || pPickObj) && CHECK_FORMAT_TRANS(SotClipboardFormatId::BITMAP))
    {
        BitmapEx aBmpEx;

        // get basic Bitmap data
        aDataHelper.GetBitmapEx(SotClipboardFormatId::BITMAP, aBmpEx);

        if(aBmpEx.IsEmpty())
        {
            // if this did not work, try to get graphic formats and convert these to bitmap
            Graphic aGraphic;

            if(aDataHelper.GetGraphic(SotClipboardFormatId::GDIMETAFILE, aGraphic))
            {
                aBmpEx = aGraphic.GetBitmapEx();
            }
            else if(aDataHelper.GetGraphic(SotClipboardFormatId::SVXB, aGraphic))
            {
                aBmpEx = aGraphic.GetBitmapEx();
            }
            else if(aDataHelper.GetGraphic(SotClipboardFormatId::BITMAP, aGraphic))
            {
                aBmpEx = aGraphic.GetBitmapEx();
            }
        }

        if(!aBmpEx.IsEmpty())
        {
            Point aInsertPos( rPos );

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = const_cast<SdrPage*>( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PageKind::Standard ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetSdrObjListRectsDirty();

                // #i120393# Clipboard data uses full object geometry range
                const Size aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

                aInsertPos.setX( pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 ) );
                aInsertPos.setY( pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 ) );
            }

            // restrict movement to WorkArea
            Size aImageMapSize(aBmpEx.GetPrefSize());
            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aBmpEx, mnAction, aInsertPos, nullptr, pImageMap.get() );
            bReturn = true;
        }
    }

    if(!bReturn && pPickObj && CHECK_FORMAT_TRANS( SotClipboardFormatId::XFA ) )
    {
        uno::Any const data(aDataHelper.GetAny(SotClipboardFormatId::XFA, ""));
        uno::Sequence<beans::NamedValue> props;
        if (data >>= props)
        {
            if( IsUndoEnabled() )
            {
                BegUndo( SdResId(STR_UNDO_DRAGDROP) );
                AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pPickObj ) );
                EndUndo();
            }

            ::comphelper::SequenceAsHashMap const map(props);
            drawing::FillStyle eFill(drawing::FillStyle_BITMAP); // default to something that's ignored
            Color aColor(COL_BLACK);
            auto it = map.find("FillStyle");
            if (it != map.end())
            {
                XFillStyleItem style;
                style.PutValue(it->second, 0);
                eFill = style.GetValue();
            }
            it = map.find("FillColor");
            if (it != map.end())
            {
                XFillColorItem color;
                color.PutValue(it->second, 0);
                aColor = color.GetColorValue();
            }

            if( eFill == drawing::FillStyle_SOLID || eFill == drawing::FillStyle_NONE )
            {
                OUString                aName;
                SfxItemSet              aSet( mrDoc.GetPool() );
                bool                    bClosed = pPickObj->IsClosedObj();
                ::sd::Window* pWin = mpViewSh->GetActiveWindow();
                sal_uInt16 nHitLog = static_cast<sal_uInt16>(pWin->PixelToLogic(
                    Size(FuPoor::HITPIX, 0 ) ).Width());
                const long              n2HitLog = nHitLog << 1;
                Point                   aHitPosR( rPos );
                Point                   aHitPosL( rPos );
                Point                   aHitPosT( rPos );
                Point                   aHitPosB( rPos );
                const SdrLayerIDSet*        pVisiLayer = &GetSdrPageView()->GetVisibleLayers();

                aHitPosR.AdjustX(n2HitLog );
                aHitPosL.AdjustX( -n2HitLog );
                aHitPosT.AdjustY(n2HitLog );
                aHitPosB.AdjustY( -n2HitLog );

                if( bClosed &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosR, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosL, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosT, nHitLog, *GetSdrPageView(), pVisiLayer, false) &&
                    SdrObjectPrimitiveHit(*pPickObj, aHitPosB, nHitLog, *GetSdrPageView(), pVisiLayer, false) )
                {
                    // area fill
                    if(eFill == drawing::FillStyle_SOLID )
                        aSet.Put(XFillColorItem(aName, aColor));

                    aSet.Put( XFillStyleItem( eFill ) );
                }
                else
                    aSet.Put( XLineColorItem( aName, aColor ) );

                // add text color
                pPickObj->SetMergedItemSetAndBroadcast( aSet );
            }
            bReturn = true;
        }
    }

    if(!bReturn && !bLink && CHECK_FORMAT_TRANS(SotClipboardFormatId::HTML))
    {
        ::tools::SvRef<SotStorageStream> xStm;

        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::HTML, xStm ) )
        {
            xStm->Seek( 0 );
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            bReturn = SdrView::Paste( *xStm, EETextFormat::Html, maDropPos, pPage, nPasteOptions );
        }
    }

    if(!bReturn && !bLink && CHECK_FORMAT_TRANS(SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT))
    {
        ::tools::SvRef<SotStorageStream> xStm;
        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT, xStm ) )
        {
            OutlinerView* pOLV = GetTextEditOutlinerView();

            xStm->Seek( 0 );

            if( pOLV )
            {
                ::tools::Rectangle   aRect( pOLV->GetOutputArea() );
                Point       aPos( pOLV->GetWindow()->PixelToLogic( maDropPos ) );

                if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                {
                    // mba: clipboard always must contain absolute URLs (could be from alien source)
                    pOLV->Read( *xStm, EETextFormat::Xml, mpDocSh->GetHeaderAttributes() );
                    bReturn = true;
                }
            }

            if( !bReturn )
                // mba: clipboard always must contain absolute URLs (could be from alien source)
                bReturn = SdrView::Paste( *xStm, EETextFormat::Xml, maDropPos, pPage, nPasteOptions );
        }
    }

    bool bIsRTF = false;
    if(!bReturn && !bLink && (( bIsRTF = CHECK_FORMAT_TRANS(SotClipboardFormatId::RTF) ) || CHECK_FORMAT_TRANS(SotClipboardFormatId::RICHTEXT) ))
    {
        ::tools::SvRef<SotStorageStream> xStm;

        if( aDataHelper.GetSotStorageStream( bIsRTF ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT, xStm ) )
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
                    ::tools::Rectangle   aRect( pOLV->GetOutputArea() );
                    Point       aPos( pOLV->GetWindow()->PixelToLogic( maDropPos ) );

                    if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                    {
                        // mba: clipboard always must contain absolute URLs (could be from alien source)
                        pOLV->Read( *xStm, EETextFormat::Rtf, mpDocSh->GetHeaderAttributes() );
                        bReturn = true;
                    }
                }

                if( !bReturn )
                    // mba: clipboard always must contain absolute URLs (could be from alien source)
                    bReturn = SdrView::Paste( *xStm, EETextFormat::Rtf, maDropPos, pPage, nPasteOptions );
            }
        }
    }

    if(!bReturn && CHECK_FORMAT_TRANS(SotClipboardFormatId::FILE_LIST))
    {
        FileList aDropFileList;

        if( aDataHelper.GetFileList( SotClipboardFormatId::FILE_LIST, aDropFileList ) )
        {
            maDropFileVector.clear();

            for( sal_uLong i = 0, nCount = aDropFileList.Count(); i < nCount; i++ )
                maDropFileVector.push_back( aDropFileList.GetFile( i ) );

            maDropInsertFileIdle.Start();
        }

        bReturn = true;
    }

    if(!bReturn && CHECK_FORMAT_TRANS(SotClipboardFormatId::SIMPLE_FILE))
    {
        OUString aDropFile;

        if( aDataHelper.GetString( SotClipboardFormatId::SIMPLE_FILE, aDropFile ) )
        {
            maDropFileVector.clear();
            maDropFileVector.push_back( aDropFile );
            maDropInsertFileIdle.Start();
        }

        bReturn = true;
    }

    if(!bReturn && !bLink && CHECK_FORMAT_TRANS(SotClipboardFormatId::STRING))
    {
        if( ( SotClipboardFormatId::STRING == nFormat ) ||
            ( !aDataHelper.HasFormat( SotClipboardFormatId::SOLK ) &&
              !aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
              !aDataHelper.HasFormat( SotClipboardFormatId::FILENAME ) ) )
        {
            OUString aOUString;

            if( aDataHelper.GetString( SotClipboardFormatId::STRING, aOUString ) )
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

    MarkListHasChanged();
    mbIsDropAllowed = true;
    rDnDAction = mnAction;

    return bReturn;
}

bool View::PasteRTFTable( const ::tools::SvRef<SotStorageStream>& xStm, SdrPage* pPage, SdrInsertFlags nPasteOptions )
{
    std::unique_ptr<SdDrawDocument> pModel(new SdDrawDocument( DocumentType::Impress, mpDocSh ));
    pModel->NewOrLoadCompleted(NEW_DOC);
    pModel->GetItemPool().SetDefaultMetric(MapUnit::Map100thMM);
    pModel->InsertPage(pModel->AllocPage(false));

    Reference< XComponent > xComponent( new SdXImpressDocument( pModel.get(), true ) );
    pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );

    CreateTableFromRTF( *xStm, pModel.get() );
    bool bRet = Paste(*pModel, maDropPos, pPage, nPasteOptions);

    xComponent->dispose();
    xComponent.clear();

    return bRet;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
