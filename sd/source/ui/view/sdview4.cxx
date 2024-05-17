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

#include <config_features.h>

#include <View.hxx>

#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/outdev.hxx>
#include <vcl/pdfread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/svdundo.hxx>
#include <svx/xfillit0.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/ImageMapInfo.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediawindow.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/embedhlp.hxx>
#include <vcl/graphicfilter.hxx>
#include <app.hrc>
#include <Window.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <fuinsfil.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <sdpage.hxx>
#include <view/SlideSorterView.hxx>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <svtools/soerr.hxx>
#include <sfx2/ipclient.hxx>
#include <tools/debug.hxx>

using namespace com::sun::star;

namespace sd {

/**
 * If an empty graphic object is provided, we fill it. Otherwise we fill an
 * existing object at the specified position. If there is no object at the
 * position, we create a new object and return a pointer to it.
 */
SdrGrafObj* View::InsertGraphic( const Graphic& rGraphic, sal_Int8& rAction,
                                   const Point& rPos, SdrObject* pObj, ImageMap const * pImageMap )
{
    SdrEndTextEdit();
    mnAction = rAction;

    // Is there an object at the position rPos?
    rtl::Reference<SdrGrafObj> pNewGrafObj;
    SdrPageView*    pPV = GetSdrPageView();
    SdrObject*      pPickObj = pObj;
    const bool bOnMaster = pPV && pPV->GetPage() && pPV->GetPage()->IsMasterPage();

    if(pPV && dynamic_cast< const ::sd::slidesorter::view::SlideSorterView* >(this) !=  nullptr)
    {
        if(!pPV->GetPageRect().Contains(rPos))
            pPV = nullptr;
    }

    if( !pPickObj && pPV )
    {
        SdrPageView* pPageView = pPV;
        pPickObj = PickObj(rPos, getHitTolLog(), pPageView);
    }

    const bool bIsGraphic(dynamic_cast< const SdrGrafObj* >(pPickObj) !=  nullptr);

    if (DND_ACTION_LINK == mnAction
        && pPickObj
        && pPV
        && (bIsGraphic || (pPickObj->IsEmptyPresObj() && !bOnMaster))) // #121603# Do not use pObj, it may be NULL
    {
        // hit on SdrGrafObj with wanted new linked graphic (or PresObj placeholder hit)
        if( IsUndoEnabled() )
            BegUndo(SdResId(STR_INSERTGRAPHIC));

        SdPage* pPage = static_cast<SdPage*>( pPickObj->getSdrPageFromSdrObject() );

        if( bIsGraphic )
        {
            // We fill the object with the Bitmap
            pNewGrafObj = SdrObject::Clone(static_cast<SdrGrafObj&>(*pPickObj), pPickObj->getSdrModelFromSdrObject());
            pNewGrafObj->SetGraphic(rGraphic);
        }
        else
        {
            pNewGrafObj = new SdrGrafObj(
                getSdrModelFromSdrView(),
                rGraphic,
                pPickObj->GetLogicRect());
            pNewGrafObj->SetEmptyPresObj(true);
        }

        if ( pNewGrafObj->IsEmptyPresObj() )
        {
            ::tools::Rectangle aRect( pNewGrafObj->GetLogicRect() );
            pNewGrafObj->AdjustToMaxRect( aRect );
            pNewGrafObj->SetOutlinerParaObject(std::nullopt);
            pNewGrafObj->SetEmptyPresObj(false);
        }

        if (pPage && pPage->IsPresObj(pPickObj))
        {
            // Insert new PresObj into the list
            pPage->InsertPresObj( pNewGrafObj.get(), PresObjKind::Graphic );
            pNewGrafObj->SetUserCall(pPickObj->GetUserCall());
        }

        if (pImageMap)
            pNewGrafObj->AppendUserData(std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(*pImageMap)));

        ReplaceObjectAtView(pPickObj, *pPV, pNewGrafObj.get()); // maybe ReplaceObjectAtView

        if( IsUndoEnabled() )
            EndUndo();
    }
    else if (DND_ACTION_LINK == mnAction
        && pPickObj
        && !bIsGraphic
        && pPickObj->IsClosedObj()
        && !dynamic_cast< const SdrOle2Obj* >(pPickObj))
    {
        // fill style change (fill object with graphic), independent of mnAction
        // and thus of DND_ACTION_LINK or DND_ACTION_MOVE
        if( IsUndoEnabled() )
        {
            BegUndo(SdResId(STR_UNDO_DRAGDROP));
            AddUndo(GetModel().GetSdrUndoFactory().CreateUndoAttrObject(*pPickObj));
            EndUndo();
        }

        SfxItemSetFixed<XATTR_FILLSTYLE, XATTR_FILLBITMAP> aSet(mpDocSh->GetPool());

        aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        aSet.Put(XFillBitmapItem(rGraphic));
        pPickObj->SetMergedItemSetAndBroadcast(aSet);
    }

    else if ( pPV )
    {
        Size aSizePixel = rGraphic.GetSizePixel();

        // create  new object
        Size aSize;

        if ( rGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
        {
            ::OutputDevice* pOutDev = nullptr;
            if( mpViewSh )
                pOutDev = mpViewSh->GetActiveWindow()->GetOutDev();

            if( !pOutDev )
                pOutDev = Application::GetDefaultDevice();

            if( pOutDev )
                aSize = pOutDev->PixelToLogic(rGraphic.GetPrefSize(), MapMode(MapUnit::Map100thMM));
        }
        else
        {
            aSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                rGraphic.GetPrefMapMode(),
                                                MapMode( MapUnit::Map100thMM ) );
        }

        sal_Int32 nPreferredDPI = mrDoc.getImagePreferredDPI();

        if (rGraphic.GetGfxLink().GetType() == GfxLinkType::NativePdf && nPreferredDPI == 0 && vcl::PDF_INSERT_MAGIC_SCALE_FACTOR > 1)
            nPreferredDPI = Application::GetDefaultDevice()->GetDPIX() * vcl::PDF_INSERT_MAGIC_SCALE_FACTOR;

        if (nPreferredDPI > 0)
        {
            auto nWidth = o3tl::convert(aSizePixel.Width() / double(nPreferredDPI), o3tl::Length::in, o3tl::Length::mm100);
            auto nHeight = o3tl::convert(aSizePixel.Height() / double(nPreferredDPI), o3tl::Length::in, o3tl::Length::mm100);
            if (nWidth > 0 && nHeight > 0)
                aSize = Size(nWidth, nHeight);
        }

        pNewGrafObj = new SdrGrafObj(getSdrModelFromSdrView(), rGraphic, ::tools::Rectangle(rPos, aSize));

        if (nPreferredDPI > 0)
        {
            // move to the center of insertion point
            pNewGrafObj->NbcMove(Size(-aSize.Width() / 2, -aSize.Height() / 2));
        }
        else
        {
            SdrPage* pPage = pPV->GetPage();
            Size aPageSize( pPage->GetSize() );
            aPageSize.AdjustWidth( -(pPage->GetLeftBorder() + pPage->GetRightBorder()) );
            aPageSize.AdjustHeight( -(pPage->GetUpperBorder() + pPage->GetLowerBorder()) );
            pNewGrafObj->AdjustToMaxRect( ::tools::Rectangle( Point(), aPageSize ), true );
        }

        SdrInsertFlags nOptions = SdrInsertFlags::SETDEFLAYER;
        bool    bIsPresTarget = false;

        if ((mpViewSh
                && mpViewSh->GetViewShell()!=nullptr
                && mpViewSh->GetViewShell()->GetIPClient()
                && mpViewSh->GetViewShell()->GetIPClient()->IsObjectInPlaceActive())
            || dynamic_cast<const ::sd::slidesorter::view::SlideSorterView* >(this))
            nOptions |= SdrInsertFlags::DONTMARK;

        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj && (pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall()) )
        {
            SdPage* pP = static_cast< SdPage* >( pPickObj->getSdrPageFromSdrObject() );

            if ( pP && pP->IsMasterPage() )
                bIsPresTarget = pP->IsPresObj(pPickObj);
        }

        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj && !bIsPresTarget )
        {
            // replace object
            if (pImageMap)
                pNewGrafObj->AppendUserData(std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(*pImageMap)));

            ::tools::Rectangle aPickObjRect(pPickObj->GetCurrentBoundRect());
            Size aPickObjSize(aPickObjRect.GetSize());
            ::tools::Rectangle aObjRect(pNewGrafObj->GetCurrentBoundRect());
            Size aObjSize(aObjRect.GetSize());

            Fraction aScaleWidth(aPickObjSize.Width(), aObjSize.Width());
            Fraction aScaleHeight(aPickObjSize.Height(), aObjSize.Height());
            pNewGrafObj->NbcResize(aObjRect.TopLeft(), aScaleWidth, aScaleHeight);

            Point aVec = aPickObjRect.TopLeft() - aObjRect.TopLeft();
            pNewGrafObj->NbcMove(Size(aVec.X(), aVec.Y()));

            const bool bUndo = IsUndoEnabled();

            if( bUndo )
                BegUndo(SdResId(STR_UNDO_DRAGDROP));
            pNewGrafObj->NbcSetLayer(pPickObj->GetLayer());
            SdrPage* pP = pPV->GetPage();
            pP->InsertObject(pNewGrafObj.get());
            if( bUndo )
            {
                AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoNewObject(*pNewGrafObj));
                AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeleteObject(*pPickObj));
            }
            pP->RemoveObject(pPickObj->GetOrdNum());

            if( bUndo )
            {
                EndUndo();
            }
            mnAction = DND_ACTION_COPY;
        }
        else
        {
            bool bSuccess = InsertObjectAtView(pNewGrafObj.get(), *pPV, nOptions);
            if (!bSuccess)
                pNewGrafObj = nullptr;
            else if (pImageMap)
                pNewGrafObj->AppendUserData(std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(*pImageMap)));
        }
    }

    rAction = mnAction;

    return pNewGrafObj.get();
}

void View::InsertMediaURL( const OUString& rMediaURL, sal_Int8& rAction,
                                   const Point& rPos, const Size& rSize,
                                   bool const bLink )
{
    OUString realURL;
    if (bLink)
    {
        realURL = rMediaURL;
    }
    else
    {
        uno::Reference<frame::XModel> const xModel(
                GetDoc().GetObjectShell()->GetModel());
#if HAVE_FEATURE_AVMEDIA
        bool const bRet = ::avmedia::EmbedMedia(xModel, rMediaURL, realURL);
        if (!bRet) { return; }
#else
        return;
#endif
    }

    InsertMediaObj(realURL, rAction, rPos, rSize);
}

SdrMediaObj* View::InsertMediaObj( const OUString& rMediaURL, sal_Int8& rAction,
                                   const Point& rPos, const Size& rSize )
{
    SdrEndTextEdit();
    mnAction = rAction;

    rtl::Reference<SdrMediaObj> pNewMediaObj;
    SdrPageView*    pPV = GetSdrPageView();
    SdrObject*      pPickObj = GetEmptyPresentationObject( PresObjKind::Media );

    if(pPV && dynamic_cast<const ::sd::slidesorter::view::SlideSorterView* >(this) )
    {
        if(!pPV->GetPageRect().Contains(rPos))
            pPV = nullptr;
    }

    if( mnAction == DND_ACTION_LINK && pPV && dynamic_cast< SdrMediaObj *>( pPickObj ) )
    {
        pNewMediaObj = SdrObject::Clone(static_cast<SdrMediaObj&>(*pPickObj), pPickObj->getSdrModelFromSdrObject());
        pNewMediaObj->setURL(rMediaURL, u""_ustr/*TODO?*/);

        BegUndo(SdResId(STR_UNDO_DRAGDROP));
        ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj.get());
        EndUndo();
    }
    else if( pPV )
    {
        ::tools::Rectangle aRect( rPos, rSize );
        SdrObjUserCall* pUserCall = nullptr;
        if( pPickObj )
        {
            aRect = pPickObj->GetLogicRect();
            pUserCall = pPickObj->GetUserCall(); // ReplaceObjectAtView can free pPickObj
        }

        pNewMediaObj = new SdrMediaObj(
            getSdrModelFromSdrView(),
            aRect);

        bool bIsPres = false;
        if( pPickObj )
        {
            SdPage* pPage = static_cast< SdPage* >(pPickObj->getSdrPageFromSdrObject());
            bIsPres = pPage && pPage->IsPresObj(pPickObj);
            if( bIsPres )
            {
                pPage->InsertPresObj( pNewMediaObj.get(), PresObjKind::Media );
            }
        }

        if( pPickObj )
            ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj.get());
        else
        {
            if (!InsertObjectAtView(pNewMediaObj.get(), *pPV, SdrInsertFlags::SETDEFLAYER))
                pNewMediaObj = nullptr;
        }

        OUString referer;
        DrawDocShell * sh = GetDocSh();
        if (sh != nullptr && sh->HasName()) {
            referer = sh->GetMedium()->GetName();
        }

        if (pNewMediaObj)
        {
            pNewMediaObj->setURL(rMediaURL, referer);

            if( pPickObj )
            {
                pNewMediaObj->AdjustToMaxRect( aRect );
                if( bIsPres )
                    pNewMediaObj->SetUserCall( pUserCall );
            }
        }
    }

    rAction = mnAction;

    return pNewMediaObj.get();
}

/**
 * Timer handler for InsertFile at Drop()
 */
IMPL_LINK_NOARG(View, DropInsertFileHdl, Timer *, void)
{
    DBG_ASSERT( mpViewSh, "sd::View::DropInsertFileHdl(), I need a view shell to work!" );
    if( !mpViewSh )
        return;

    SfxErrorContext aEc( ERRCTX_ERROR, mpViewSh->GetFrameWeld(), RID_SO_ERRCTX );
    ErrCode nError = ERRCODE_NONE;

    ::std::vector< OUString >::const_iterator aIter( maDropFileVector.begin() );

    while( (aIter != maDropFileVector.end()) && !nError )
    {
        OUString aCurrentDropFile( *aIter );
        INetURLObject   aURL( aCurrentDropFile );
        bool bHandled = false;

        if( aURL.GetProtocol() == INetProtocol::NotValid )
        {
            OUString aURLStr;
            osl::FileBase::getFileURLFromSystemPath( aCurrentDropFile, aURLStr );
            aURL = INetURLObject( aURLStr );
        }

        GraphicFilter&  rGraphicFilter = GraphicFilter::GetGraphicFilter();
        Graphic         aGraphic;

        aCurrentDropFile = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

#if HAVE_FEATURE_AVMEDIA
        if( !::avmedia::MediaWindow::isMediaURL( aCurrentDropFile, u""_ustr/*TODO?*/ ) )
#else
#endif
        {
            if( !rGraphicFilter.ImportGraphic( aGraphic, aURL ) )
            {
                sal_Int8    nTempAction = ( aIter == maDropFileVector.begin() ) ? mnAction : 0;
                const bool bLink = ( ( nTempAction & DND_ACTION_LINK ) != 0 );
                SdrGrafObj* pGrafObj = InsertGraphic( aGraphic, nTempAction, maDropPos, nullptr, nullptr );
                if(pGrafObj && bLink)
                {
                    pGrafObj->SetGraphicLink( aCurrentDropFile );
                }

                // return action from first inserted graphic
                if( aIter == maDropFileVector.begin() )
                    mnAction = nTempAction;

                bHandled = true;
            }
            if (!bHandled)
            {
                std::shared_ptr<const SfxFilter> pFoundFilter;
                SfxMedium               aSfxMedium( aCurrentDropFile, StreamMode::READ | StreamMode::SHARE_DENYNONE );
                ErrCode                 nErr = SfxGetpApp()->GetFilterMatcher().GuessFilter(  aSfxMedium, pFoundFilter );

                if( pFoundFilter && !nErr )
                {
                    ::std::vector< OUString > aFilterVector;
                    OUString aFilterName = pFoundFilter->GetFilterName();
                    OUString aLowerAsciiFileName = aCurrentDropFile.toAsciiLowerCase();

                    FuInsertFile::GetSupportedFilterVector( aFilterVector );

                    if( ( ::std::find( aFilterVector.begin(), aFilterVector.end(), pFoundFilter->GetMimeType() ) != aFilterVector.end() ) ||
                        aFilterName.indexOf( "Text" ) != -1 ||
                        aFilterName.indexOf( "Rich" ) != -1 ||
                        aFilterName.indexOf( "RTF" ) != -1 ||
                        aFilterName.indexOf( "HTML" ) != -1 ||
                        aLowerAsciiFileName.indexOf(".sdd") != -1 ||
                        aLowerAsciiFileName.indexOf(".sda") != -1 ||
                        aLowerAsciiFileName.indexOf(".sxd") != -1 ||
                        aLowerAsciiFileName.indexOf(".sxi") != -1 ||
                        aLowerAsciiFileName.indexOf(".std") != -1 ||
                        aLowerAsciiFileName.indexOf(".sti") != -1 )
                    {
                        ::sd::Window* pWin = mpViewSh->GetActiveWindow();
                        SfxRequest      aReq(SID_INSERTFILE, ::SfxCallMode::SLOT, mrDoc.GetItemPool());
                        SfxStringItem   aItem1( ID_VAL_DUMMY0, aCurrentDropFile ), aItem2( ID_VAL_DUMMY1, pFoundFilter->GetFilterName() );

                        aReq.AppendItem( aItem1 );
                        aReq.AppendItem( aItem2 );
                        FuInsertFile::Create( mpViewSh, pWin, this, &mrDoc, aReq );
                        bHandled = true;
                    }
                }
            }
        }

#if HAVE_FEATURE_AVMEDIA
        if (!bHandled)
        {
            bool bShallowDetect = ::avmedia::MediaWindow::isMediaURL(aCurrentDropFile, u""_ustr/*TODO?*/);
            if (bShallowDetect)
            {
                mxDropMediaSizeListener.set(new avmedia::PlayerListener(
                    [this, aCurrentDropFile](const css::uno::Reference<css::media::XPlayer>& rPlayer){
                        SolarMutexGuard g;

                        css::awt::Size aSize = rPlayer->getPreferredPlayerWindowSize();
                        Size aPrefSize(aSize.Width, aSize.Height);

                        if (aPrefSize.Width() && aPrefSize.Height())
                        {
                            ::sd::Window* pWin = mpViewSh->GetActiveWindow();

                            if( pWin )
                                aPrefSize = pWin->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
                            else
                                aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
                        }
                        else
                            aPrefSize  = Size( 5000, 5000 );

                        InsertMediaURL(aCurrentDropFile, mnAction, maDropPos, aPrefSize, true);

                        mxDropMediaSizeListener.clear();
                    }));
            }
            bHandled = bShallowDetect && ::avmedia::MediaWindow::isMediaURL(aCurrentDropFile, u""_ustr/*TODO?*/, true, mxDropMediaSizeListener);
        }
#endif

        if (!bHandled)
        {
            if( mnAction & DND_ACTION_LINK )
                static_cast< DrawViewShell* >( mpViewSh )->InsertURLButton( aCurrentDropFile, aCurrentDropFile, OUString(), &maDropPos );
            else
            {
                if( mpViewSh )
                {
                    try
                    {
                        //TODO/MBA: testing
                        OUString aName;
                        uno::Sequence < beans::PropertyValue > aMedium{ comphelper::makePropertyValue(
                            u"URL"_ustr, aCurrentDropFile) };

                        uno::Reference < embed::XEmbeddedObject > xObj = mpDocSh->GetEmbeddedObjectContainer().
                                InsertEmbeddedObject( aMedium, aName );

                        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                        if ( xPersist.is())
                        {
                            // TODO/LEAN: VisualArea access can switch the object to running state
                            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

                            xPersist->storeOwn();

                            awt::Size aSz;
                            try
                            {
                                aSz = xObj->getVisualAreaSize( nAspect );
                            }
                            catch( embed::NoVisualAreaSizeException& )
                            {
                                // the default size will be set later
                            }

                            Size        aSize( aSz.Width, aSz.Height );
                            ::tools::Rectangle   aRect;

                            if (!aSize.Width() || !aSize.Height())
                            {
                                aSize.setWidth( 1410 );
                                aSize.setHeight( 1000 );
                            }

                            aRect = ::tools::Rectangle( maDropPos, aSize );

                            rtl::Reference<SdrOle2Obj> pOleObj = new SdrOle2Obj(
                                getSdrModelFromSdrView(),
                                svt::EmbeddedObjectRef(xObj, nAspect),
                                aName,
                                aRect);
                            SdrInsertFlags nOptions = SdrInsertFlags::SETDEFLAYER;

                            if (mpViewSh != nullptr)
                            {
                                OSL_ASSERT (mpViewSh->GetViewShell()!=nullptr);
                                SfxInPlaceClient* pIpClient =
                                    mpViewSh->GetViewShell()->GetIPClient();
                                if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
                                    nOptions |= SdrInsertFlags::DONTMARK;
                            }

                            if (InsertObjectAtView( pOleObj.get(), *GetSdrPageView(), nOptions ))
                                pOleObj->SetLogicRect( aRect );
                            aSz.Width = aRect.GetWidth();
                            aSz.Height = aRect.GetHeight();
                            xObj->setVisualAreaSize( nAspect,aSz );
                        }
                    }
                    catch( uno::Exception& )
                    {
                        nError = ERRCODE_IO_GENERAL;
                        // TODO/LATER: better error handling
                    }
                }
            }
        }

        ++aIter;
    }

    if( nError )
        ErrorHandler::HandleError( nError );
}

/**
 * Timer handler for Errorhandling at Drop()
 */
IMPL_LINK_NOARG(View, DropErrorHdl, Timer *, void)
{
    vcl::Window* pWin = mpViewSh ? mpViewSh->GetActiveWindow() : nullptr;
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  SdResId(STR_ACTION_NOTPOSSIBLE)));
    xInfoBox->run();
}

/**
 * @returns StyleSheet from selection
 */
SfxStyleSheet* View::GetStyleSheet() const
{
    return SdrView::GetStyleSheet();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
