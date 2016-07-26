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
#include <osl/file.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdundo.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediawindow.hxx>
#include <avmedia/modeltools.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/embedhlp.hxx>
#include <vcl/graphicfilter.hxx>
#include "app.hrc"
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "fuinsfil.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "sdpage.hxx"
#include "view/SlideSorterView.hxx"
#include "undo/undoobjects.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <svtools/soerr.hxx>
#include <sfx2/ipclient.hxx>
#include <svx/svdoashp.hxx>
#include "glob.hrc"

#include <config_features.h>

using namespace com::sun::star;

namespace sd {

/**
 * If an empty graphic object is provided, we fill it. Otherwise we fill an
 * existing object at the specified position. If there is no object at the
 * position, we create a new object and return a pointer to it.
 */
SdrGrafObj* View::InsertGraphic( const Graphic& rGraphic, sal_Int8& rAction,
                                   const Point& rPos, SdrObject* pObj, ImageMap* pImageMap )
{
    SdrEndTextEdit();
    mnAction = rAction;

    // Is there a object at the position rPos?
    SdrGrafObj*     pNewGrafObj = nullptr;
    SdrPageView*    pPV = GetSdrPageView();
    SdrObject*      pPickObj = pObj;
    const bool bOnMaster = pPV && pPV->GetPage() && pPV->GetPage()->IsMasterPage();

    if(pPV && dynamic_cast< const ::sd::slidesorter::view::SlideSorterView* >(this) !=  nullptr)
    {
        if(!pPV->GetPageRect().IsInside(rPos))
            pPV = nullptr;
    }

    if( !pPickObj && pPV )
    {
        SdrPageView* pPageView = pPV;
        PickObj(rPos, getHitTolLog(), pPickObj, pPageView);
    }

    const bool bIsGraphic(dynamic_cast< const SdrGrafObj* >(pPickObj) !=  nullptr);

    if (DND_ACTION_LINK == mnAction
        && pPickObj
        && pPV
        && (bIsGraphic || (pPickObj->IsEmptyPresObj() && !bOnMaster))) // #121603# Do not use pObj, it may be NULL
    {
        // hit on SdrGrafObj with wanted new linked graphic (or PresObj placeholder hit)
        if( IsUndoEnabled() )
            BegUndo(OUString(SdResId(STR_INSERTGRAPHIC)));

        SdPage* pPage = static_cast<SdPage*>( pPickObj->GetPage() );

        if( bIsGraphic )
        {
            // We fill the object with the Bitmap
            pNewGrafObj = static_cast<SdrGrafObj*>( pPickObj->Clone() );
            pNewGrafObj->SetGraphic(rGraphic);
        }
        else
        {
            pNewGrafObj = new SdrGrafObj( rGraphic, pPickObj->GetLogicRect() );
            pNewGrafObj->SetEmptyPresObj(true);
        }

        if ( pNewGrafObj->IsEmptyPresObj() )
        {
            Rectangle aRect( pNewGrafObj->GetLogicRect() );
            pNewGrafObj->AdjustToMaxRect( aRect );
            pNewGrafObj->SetOutlinerParaObject(nullptr);
            pNewGrafObj->SetEmptyPresObj(false);
        }

        if (pPage && pPage->IsPresObj(pPickObj))
        {
            // Insert new PresObj into the list
            pPage->InsertPresObj( pNewGrafObj, PRESOBJ_GRAPHIC );
            pNewGrafObj->SetUserCall(pPickObj->GetUserCall());
        }

        if (pImageMap)
            pNewGrafObj->AppendUserData(new SdIMapInfo(*pImageMap));

        ReplaceObjectAtView(pPickObj, *pPV, pNewGrafObj); // maybe ReplaceObjectAtView

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
            BegUndo(OUString(SdResId(STR_UNDO_DRAGDROP)));
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pPickObj));
            EndUndo();
        }

        SfxItemSet aSet(mpDocSh->GetPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);

        aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        aSet.Put(XFillBitmapItem(&mpDocSh->GetPool(), rGraphic));
        pPickObj->SetMergedItemSetAndBroadcast(aSet);
    }

    else if ( pPV )
    {
        // create  new object
        Size aSize;

        if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        {
            ::OutputDevice* pOutDev = nullptr;
            if( mpViewSh )
                pOutDev = mpViewSh->GetActiveWindow();

            if( !pOutDev )
                pOutDev = Application::GetDefaultDevice();

            if( pOutDev )
                aSize = pOutDev->PixelToLogic( rGraphic.GetPrefSize(), MAP_100TH_MM );
        }
        else
        {
            aSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                rGraphic.GetPrefMapMode(),
                                                MapMode( MAP_100TH_MM ) );
        }

        pNewGrafObj = new SdrGrafObj( rGraphic, Rectangle( rPos, aSize ) );
        SdrPage* pPage = pPV->GetPage();
        Size aPageSize( pPage->GetSize() );
        aPageSize.Width()  -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();
        pNewGrafObj->AdjustToMaxRect( Rectangle( Point(), aPageSize ), true );

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
            SdPage* pP = static_cast< SdPage* >( pPickObj->GetPage() );

            if ( pP && pP->IsMasterPage() )
                bIsPresTarget = pP->IsPresObj(pPickObj);
        }

        if( ( mnAction & DND_ACTION_MOVE ) && pPickObj && !bIsPresTarget )
        {
            // replace object
            if (pImageMap)
                pNewGrafObj->AppendUserData(new SdIMapInfo(*pImageMap));

            Rectangle aPickObjRect(pPickObj->GetCurrentBoundRect());
            Size aPickObjSize(aPickObjRect.GetSize());
            Rectangle aObjRect(pNewGrafObj->GetCurrentBoundRect());
            Size aObjSize(aObjRect.GetSize());

            Fraction aScaleWidth(aPickObjSize.Width(), aObjSize.Width());
            Fraction aScaleHeight(aPickObjSize.Height(), aObjSize.Height());
            pNewGrafObj->NbcResize(aObjRect.TopLeft(), aScaleWidth, aScaleHeight);

            Point aVec = aPickObjRect.TopLeft() - aObjRect.TopLeft();
            pNewGrafObj->NbcMove(Size(aVec.X(), aVec.Y()));

            const bool bUndo = IsUndoEnabled();

            if( bUndo )
                BegUndo(SD_RESSTR(STR_UNDO_DRAGDROP));
            pNewGrafObj->NbcSetLayer(pPickObj->GetLayer());
            SdrPage* pP = pPV->GetPage();
            pP->InsertObject(pNewGrafObj);
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
            else
            {
                SdrObject::Free(pPickObj);
            }
            mnAction = DND_ACTION_COPY;
        }
        else
        {
            InsertObjectAtView(pNewGrafObj, *pPV, nOptions);

            if( pImageMap )
                pNewGrafObj->AppendUserData(new SdIMapInfo(*pImageMap));
        }
    }

    rAction = mnAction;

    return pNewGrafObj;
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
        bool const bRet = ::avmedia::EmbedMedia(xModel, rMediaURL, realURL);
        if (!bRet) { return; }
    }

    InsertMediaObj( realURL, "application/vnd.sun.star.media", rAction, rPos, rSize );
}
#if HAVE_FEATURE_OPENGL
#if HAVE_FEATURE_GLTF
void View::Insert3DModelURL(
    const OUString& rModelURL, sal_Int8& rAction,
    const Point& rPos, const Size& rSize )
{
    OUString sRealURL;
    uno::Reference<frame::XModel> const xModel(
                GetDoc().GetObjectShell()->GetModel());
    bool const bRet = ::avmedia::Embed3DModel(xModel, rModelURL, sRealURL);
    if (!bRet)
        return;

    SdrMediaObj* pRetObject = InsertMediaObj( sRealURL, "model/vnd.gltf+json", rAction, rPos, rSize );
    avmedia::MediaItem aItem = pRetObject->getMediaProperties();
    aItem.setLoop(true);
    pRetObject->setMediaProperties(aItem);
}
#endif
#endif

SdrMediaObj* View::InsertMediaObj( const OUString& rMediaURL, const OUString& rMimeType, sal_Int8& rAction,
                                   const Point& rPos, const Size& rSize )
{
    SdrEndTextEdit();
    mnAction = rAction;

    SdrMediaObj*    pNewMediaObj = nullptr;
    SdrPageView*    pPV = GetSdrPageView();
    SdrObject*      pPickObj = GetEmptyPresentationObject( PRESOBJ_MEDIA );

    if(pPV && dynamic_cast<const ::sd::slidesorter::view::SlideSorterView* >(this) )
    {
        if(!pPV->GetPageRect().IsInside(rPos))
            pPV = nullptr;
    }

    if( !pPickObj && pPV )
    {
        SdrPageView* pPageView = pPV;
        PickObj(rPos, getHitTolLog(), pPickObj, pPageView);
    }

    if( mnAction == DND_ACTION_LINK && pPickObj && pPV && dynamic_cast< SdrMediaObj *>( pPickObj ) !=  nullptr )
    {
        pNewMediaObj = static_cast< SdrMediaObj* >( pPickObj->Clone() );
        pNewMediaObj->setURL( rMediaURL, ""/*TODO?*/, rMimeType );

        BegUndo(SD_RESSTR(STR_UNDO_DRAGDROP));
        ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj);
        EndUndo();
    }
    else if( pPV )
    {
        Rectangle aRect( rPos, rSize );
        if( pPickObj )
            aRect = pPickObj->GetLogicRect();

        pNewMediaObj = new SdrMediaObj( aRect );

        bool bIsPres = false;
        if( pPickObj )
        {
            SdPage* pPage = static_cast< SdPage* >(pPickObj->GetPage());
            bIsPres = pPage && pPage->IsPresObj(pPickObj);
            if( bIsPres )
            {
                pPage->InsertPresObj( pNewMediaObj, PRESOBJ_MEDIA );
            }
        }

        if( pPickObj )
            ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj);
        else
            InsertObjectAtView( pNewMediaObj, *pPV, SdrInsertFlags::SETDEFLAYER );

        OUString referer;
        DrawDocShell * sh = GetDocSh();
        if (sh != nullptr && sh->HasName()) {
            referer = sh->GetMedium()->GetName();
        }
        pNewMediaObj->setURL( rMediaURL, referer, rMimeType );

        if( pPickObj )
        {
            pNewMediaObj->AdjustToMaxRect( pPickObj->GetLogicRect() );
            if( bIsPres )
                pNewMediaObj->SetUserCall(pPickObj->GetUserCall());
        }
    }

    rAction = mnAction;

    return pNewMediaObj;
}

/**
 * Timer handler for InsertFile at Drop()
 */
IMPL_LINK_NOARG_TYPED(View, DropInsertFileHdl, Idle *, void)
{
    DBG_ASSERT( mpViewSh, "sd::View::DropInsertFileHdl(), I need a view shell to work!" );
    if( !mpViewSh )
        return;

    SfxErrorContext aEc( ERRCTX_ERROR, mpViewSh->GetActiveWindow(), RID_SO_ERRCTX );
    ErrCode nError = 0;

    ::std::vector< OUString >::const_iterator aIter( maDropFileVector.begin() );

    while( (aIter != maDropFileVector.end()) && !nError )
    {
        OUString aCurrentDropFile( *aIter );
        INetURLObject   aURL( aCurrentDropFile );
        bool            bOK = false;

        if( aURL.GetProtocol() == INetProtocol::NotValid )
        {
            OUString aURLStr;
            osl::FileBase::getFileURLFromSystemPath( aCurrentDropFile, aURLStr );
            aURL = INetURLObject( aURLStr );
        }

        GraphicFilter&  rGraphicFilter = GraphicFilter::GetGraphicFilter();
        Graphic         aGraphic;

        aCurrentDropFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

        if( !::avmedia::MediaWindow::isMediaURL( aCurrentDropFile, ""/*TODO?*/ ) )
        {
            if( !rGraphicFilter.ImportGraphic( aGraphic, aURL ) )
            {
                sal_Int8    nTempAction = ( aIter == maDropFileVector.begin() ) ? mnAction : 0;
                const bool bLink = ( ( nTempAction & DND_ACTION_LINK ) != 0 );
                SdrGrafObj* pGrafObj = InsertGraphic( aGraphic, nTempAction, maDropPos, nullptr, nullptr );
                if(pGrafObj && bLink)
                {
                    pGrafObj->SetGraphicLink( aCurrentDropFile, ""/*TODO?*/, OUString() );
                }

                // return action from first inserted graphic
                if( aIter == maDropFileVector.begin() )
                    mnAction = nTempAction;

                bOK = true;
            }
            if( !bOK )
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
                        bOK = true;
                    }
                }
            }
        }

        if( !bOK )
        {
            Size aPrefSize;

            if( ::avmedia::MediaWindow::isMediaURL( aCurrentDropFile, ""/*TODO?*/ ) &&
                ::avmedia::MediaWindow::isMediaURL( aCurrentDropFile, ""/*TODO?*/, true, &aPrefSize ) )
            {
                if( aPrefSize.Width() && aPrefSize.Height() )
                {
                    ::sd::Window* pWin = mpViewSh->GetActiveWindow();

                    if( pWin )
                        aPrefSize = pWin->PixelToLogic( aPrefSize, MAP_100TH_MM );
                    else
                        aPrefSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
                }
                else
                    aPrefSize  = Size( 5000, 5000 );

                InsertMediaURL( aCurrentDropFile, mnAction, maDropPos, aPrefSize, true ) ;
            }
            else if( mnAction & DND_ACTION_LINK )
                static_cast< DrawViewShell* >( mpViewSh )->InsertURLButton( aCurrentDropFile, aCurrentDropFile, OUString(), &maDropPos );
            else
            {
                if( mpViewSh )
                {
                    try
                    {
                        //TODO/MBA: testing
                        OUString aName;
                        uno::Sequence < beans::PropertyValue > aMedium(1);
                        aMedium[0].Name = "URL" ;
                        aMedium[0].Value <<= aCurrentDropFile ;

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
                            Rectangle   aRect;

                            if (!aSize.Width() || !aSize.Height())
                            {
                                aSize.Width()   = 1410;
                                aSize.Height()  = 1000;
                            }

                            aRect = Rectangle( maDropPos, aSize );

                            SdrOle2Obj* pOleObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aName, aRect );
                            SdrInsertFlags nOptions = SdrInsertFlags::SETDEFLAYER;

                            if (mpViewSh != nullptr)
                            {
                                OSL_ASSERT (mpViewSh->GetViewShell()!=nullptr);
                                SfxInPlaceClient* pIpClient =
                                    mpViewSh->GetViewShell()->GetIPClient();
                                if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
                                    nOptions |= SdrInsertFlags::DONTMARK;
                            }

                            InsertObjectAtView( pOleObj, *GetSdrPageView(), nOptions );
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
IMPL_LINK_NOARG_TYPED(View, DropErrorHdl, Idle *, void)
{
    ScopedVclPtrInstance<InfoBox>( mpViewSh ? mpViewSh->GetActiveWindow() : nullptr, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
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
