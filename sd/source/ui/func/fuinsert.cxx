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

#include <fuinsert.hxx>
#include <comphelper/storagehelper.hxx>
#include <editeng/outlobj.hxx>
#include <officecfg/Office/Common.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxdlg.hxx>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/request.hxx>
#include <svl/globalnameitem.hxx>
#include <svtools/embedhlp.hxx>
#include <svx/linkwarn.hxx>
#include <avmedia/mediawindow.hxx>
#include <comphelper/classids.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/transfer.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sot/formats.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/charthelper.hxx>
#include <svx/svxids.hrc>

#include <sdresid.hxx>
#include <View.hxx>
#include <sdmod.hxx>
#include <Window.hxx>
#include <DrawViewShell.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <strings.hrc>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdgrffilter.hxx>
#include <vcl/svapp.hxx>
#include <memory>
#include <vcl/weld.hxx>
#include <vcl/errinf.hxx>
#include <vcl/graphicfilter.hxx>

#include <vcl/GraphicNativeTransform.hxx>
#include <vcl/GraphicNativeMetadata.hxx>

#include <comphelper/lok.hxx>

using namespace com::sun::star;

namespace sd {


FuInsertGraphic::FuInsertGraphic (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq,
    bool replaceExistingImage)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      mbReplaceExistingImage(replaceExistingImage)
{
}

rtl::Reference<FuPoor> FuInsertGraphic::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                                                SdDrawDocument* pDoc, SfxRequest& rReq, bool replaceExistingImage )
{
    rtl::Reference<FuPoor> xFunc( new FuInsertGraphic( pViewSh, pWin, pView, pDoc, rReq, replaceExistingImage ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertGraphic::DoExecute( SfxRequest& rReq )
{
    OUString aFileName;
    Graphic aGraphic;

    bool bAsLink = false;
    ErrCode nError = ERRCODE_GRFILTER_OPENERROR;

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;

    if ( pArgs &&
         pArgs->GetItemState( SID_INSERT_GRAPHIC, true, &pItem ) == SfxItemState::SET )
    {
        aFileName = static_cast<const SfxStringItem*>(pItem)->GetValue();

        OUString aFilterName;
        if ( pArgs->GetItemState( FN_PARAM_FILTER, true, &pItem ) == SfxItemState::SET )
            aFilterName = static_cast<const SfxStringItem*>(pItem)->GetValue();

        if ( pArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
            bAsLink = static_cast<const SfxBoolItem*>(pItem)->GetValue();

        nError = GraphicFilter::LoadGraphic( aFileName, aFilterName, aGraphic, &GraphicFilter::GetGraphicFilter() );
    }
    else
    {
        SvxOpenGraphicDialog aDlg(SdResId(STR_INSERTGRAPHIC), mpWindow ? mpWindow->GetFrameWeld() : nullptr);

        if( aDlg.Execute() != ERRCODE_NONE )
            return; // cancel dialog

        nError = aDlg.GetGraphic(aGraphic);
        bAsLink = aDlg.IsAsLink();
        aFileName = aDlg.GetPath();
    }

    if( nError == ERRCODE_NONE )
    {
        GraphicNativeMetadata aMetadata;
        if ( aMetadata.read(aGraphic) )
        {
            const Degree10 aRotation = aMetadata.getRotation();
            if (aRotation)
            {
                GraphicNativeTransform aTransform( aGraphic );
                aTransform.rotate( aRotation );
            }
        }
        if( dynamic_cast< DrawViewShell *>( mpViewShell ) )
        {
            sal_Int8    nAction = DND_ACTION_COPY;
            SdrObject* pPickObj = nullptr;
            if (mbReplaceExistingImage)
                pPickObj = mpView->GetSelectedSingleObject( mpView->GetPage() );
            if (pPickObj)
                nAction = DND_ACTION_LINK;
            else
            {
                pPickObj = mpView->GetEmptyPresentationObject( PresObjKind::Graphic );
                if (pPickObj)
                    nAction = DND_ACTION_LINK;
            }

            Point aPos = mpWindow->GetVisibleCenter();
            SdrGrafObj* pGrafObj = mpView->InsertGraphic(aGraphic, nAction, aPos, pPickObj, nullptr);

            if(pGrafObj && bAsLink )
            {
                // really store as link only?
                if( officecfg::Office::Common::Misc::ShowLinkWarningDialog::get() )
                {
                    SvxLinkWarningDialog aWarnDlg(mpWindow->GetFrameWeld(), aFileName);
                    if (aWarnDlg.run() != RET_OK)
                        return; // don't store as link
                }

                // store as link
                pGrafObj->SetGraphicLink(aFileName);
            }
        }
    }
    else
    {
        SdGRFFilter::HandleGraphicFilterError( nError, GraphicFilter::GetGraphicFilter().GetLastError() );
    }
}

FuInsertClipboard::FuInsertClipboard (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuInsertClipboard::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuInsertClipboard( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertClipboard::DoExecute( SfxRequest&  )
{
    TransferableDataHelper                      aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpWindow ) );
    SotClipboardFormatId                        nFormatId;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog(mpViewShell->GetFrameWeld()));
    pDlg->Insert( SotClipboardFormatId::EMBED_SOURCE, OUString() );
    pDlg->Insert( SotClipboardFormatId::LINK_SOURCE, OUString() );
    pDlg->Insert( SotClipboardFormatId::DRAWING, OUString() );
    pDlg->Insert( SotClipboardFormatId::SVXB, OUString() );
    pDlg->Insert( SotClipboardFormatId::GDIMETAFILE, OUString() );
    pDlg->Insert( SotClipboardFormatId::BITMAP, OUString() );
    pDlg->Insert( SotClipboardFormatId::NETSCAPE_BOOKMARK, OUString() );
    pDlg->Insert( SotClipboardFormatId::STRING, OUString() );
    pDlg->Insert( SotClipboardFormatId::HTML, OUString() );
    pDlg->Insert( SotClipboardFormatId::RTF, OUString() );
    pDlg->Insert( SotClipboardFormatId::RICHTEXT, OUString() );
    pDlg->Insert( SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT, OUString() );

    //TODO/MBA: testing
    nFormatId = pDlg->GetFormat( aDataHelper );
    if( nFormatId == SotClipboardFormatId::NONE || !aDataHelper.GetTransferable().is() )
        return;

    sal_Int8 nAction = DND_ACTION_COPY;
    DrawViewShell* pDrViewSh = nullptr;

    if (!mpView->InsertData( aDataHelper,
                            mpWindow->PixelToLogic( ::tools::Rectangle( Point(), mpWindow->GetOutputSizePixel() ).Center() ),
                            nAction, false, nFormatId ))
    {
        pDrViewSh = dynamic_cast<DrawViewShell*>(mpViewShell);
    }

    if (!pDrViewSh)
        return;

    INetBookmark        aINetBookmark( "", "" );

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

FuInsertOLE::FuInsertOLE (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuInsertOLE::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuInsertOLE( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertOLE::DoExecute( SfxRequest& rReq )
{
    if ( nSlotId == SID_ATTR_TABLE ||
         nSlotId == SID_INSERT_DIAGRAM ||
         nSlotId == SID_INSERT_MATH )
    {
        PresObjKind ePresObjKind = (nSlotId == SID_INSERT_DIAGRAM) ? PresObjKind::Chart : PresObjKind::Object;

        SdrObject* pPickObj = mpView->GetEmptyPresentationObject( ePresObjKind );

        // insert diagram or Calc table
        OUString aObjName;
        SvGlobalName aName;
        if (nSlotId == SID_INSERT_DIAGRAM)
            aName = SvGlobalName( SO3_SCH_CLASSID);
        else if (nSlotId == SID_ATTR_TABLE)
            aName = SvGlobalName(SO3_SC_CLASSID);
        else if (nSlotId == SID_INSERT_MATH)
            aName = SvGlobalName(SO3_SM_CLASSID);

        uno::Reference < embed::XEmbeddedObject > xObj = mpViewShell->GetViewFrame()->GetObjectShell()->
                GetEmbeddedObjectContainer().CreateEmbeddedObject( aName.GetByteSequence(), aObjName );
        if ( xObj.is() )
        {
            // Create default chart type.
            uno::Reference<chart2::XChartDocument> xChartDoc(xObj->getComponent(), uno::UNO_QUERY);
            if (xChartDoc.is())
                xChartDoc->createDefaultChart();

            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

            ::tools::Rectangle aRect;
            if( pPickObj )
            {
                aRect = pPickObj->GetLogicRect();

                awt::Size aSz;
                aSz.Width = aRect.GetWidth();
                aSz.Height = aRect.GetHeight();
                xObj->setVisualAreaSize( nAspect, aSz );
            }
            else
            {
                awt::Size aSz;
                try
                {
                    aSz = xObj->getVisualAreaSize( nAspect );
                }
                catch ( embed::NoVisualAreaSizeException& )
                {
                    // the default size will be set later
                }

                Size aSize( aSz.Width, aSz.Height );

                if (aSize.IsEmpty())
                {
                    // rectangle with balanced edge ratio
                    aSize.setWidth( 14100 );
                    aSize.setHeight( 10000 );
                    Size aTmp = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(aUnit));
                    aSz.Width = aTmp.Width();
                    aSz.Height = aTmp.Height();
                    xObj->setVisualAreaSize( nAspect, aSz );
                }
                else
                {
                    aSize = OutputDevice::LogicToLogic(aSize, MapMode(aUnit), MapMode(MapUnit::Map100thMM));
                }

                Point aPos = mpWindow->GetVisibleCenter();
                aPos.AdjustX( -(aSize.Width() / 2) );
                aPos.AdjustY( -(aSize.Height() / 2) );
                aRect = ::tools::Rectangle(aPos, aSize);
            }

            SdrOle2Obj* pOleObj = new SdrOle2Obj(
                mpView->getSdrModelFromSdrView(),
                svt::EmbeddedObjectRef( xObj, nAspect ),
                aObjName,
                aRect);
            SdrPageView* pPV = mpView->GetSdrPageView();

            // if we have a pick obj we need to make this new ole a pres obj replacing the current pick obj
            if( pPickObj )
            {
                SdPage* pPage = static_cast< SdPage* >(pPickObj->getSdrPageFromSdrObject());
                if(pPage && pPage->IsPresObj(pPickObj))
                {
                    pPage->InsertPresObj( pOleObj, ePresObjKind );
                    pOleObj->SetUserCall(pPickObj->GetUserCall());
                }

                // #i123468# we need to end text edit before replacing the object. There cannot yet
                // being text typed (else it would not be an EmptyPresObj anymore), but it may be
                // in text edit mode
                if (mpView->IsTextEdit())
                {
                    mpView->SdrEndTextEdit();
                }
            }

            bool bRet = true;
            if( pPickObj )
                mpView->ReplaceObjectAtView(pPickObj, *pPV, pOleObj );
            else
                bRet = mpView->InsertObjectAtView(pOleObj, *pPV, SdrInsertFlags::SETDEFLAYER);

            if (bRet && !comphelper::LibreOfficeKit::isActive())
            {
                // Let the chart be activated after the inserting (unless
                // via LibreOfficeKit)
                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    pOleObj->SetProgName( "StarChart");
                }
                else if (nSlotId == SID_ATTR_TABLE)
                {
                    pOleObj->SetProgName( "StarCalc" );
                }
                else if (nSlotId == SID_INSERT_MATH)
                {
                    pOleObj->SetProgName( "StarMath" );
                }

                pOleObj->SetLogicRect(aRect);
                Size aTmp( OutputDevice::LogicToLogic(aRect.GetSize(), MapMode(MapUnit::Map100thMM), MapMode(aUnit)) );
                awt::Size aVisualSize;
                aVisualSize.Width = aTmp.Width();
                aVisualSize.Height = aTmp.Height();
                xObj->setVisualAreaSize( nAspect, aVisualSize );
                mpViewShell->ActivateObject(pOleObj, embed::EmbedVerbs::MS_OLEVERB_SHOW);

                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    // note, that this call modified the chart model which
                    // results in a change notification.  So call this after
                    // everything else is finished.
                    ChartHelper::AdaptDefaultsForChart( xObj );
                }
            }
        }
        else
        {
            ErrorHandler::HandleError(* new StringErrorInfo(ERRCODE_SFX_OLEGENERAL,
                                        "" ) );
        }
    }
    else
    {
        // insert object
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        bool bCreateNew = false;
        uno::Reference < embed::XEmbeddedObject > xObj;
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
        SvObjectServerList aServerLst;
        OUString aName;

        OUString aIconMediaType;
        uno::Reference< io::XInputStream > xIconMetaFile;

        const SfxGlobalNameItem* pNameItem = rReq.GetArg<SfxGlobalNameItem>(SID_INSERT_OBJECT);
        if ( nSlotId == SID_INSERT_OBJECT && pNameItem )
        {
            const SvGlobalName& aClassName = pNameItem->GetValue();
            xObj =  mpViewShell->GetViewFrame()->GetObjectShell()->
                    GetEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aName );
        }
        else
        {
            switch ( nSlotId )
            {
                case SID_INSERT_OBJECT :
                {
                    aServerLst.FillInsertObjects();
                    if (mpDoc->GetDocumentType() == DocumentType::Draw)
                    {
                        aServerLst.Remove( GraphicDocShell::Factory().GetClassId() );
                    }
                    else
                    {
                        aServerLst.Remove( DrawDocShell::Factory().GetClassId() );
                    }

                    [[fallthrough]];
                }
                case SID_INSERT_FLOATINGFRAME :
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<SfxAbstractInsertObjectDialog> pDlg(
                            pFact->CreateInsertObjectDialog( mpViewShell->GetFrameWeld(), SD_MOD()->GetSlotPool()->GetSlot(nSlotId)->GetCommandString(),
                            xStorage, &aServerLst ));
                    pDlg->Execute();
                    bCreateNew = pDlg->IsCreateNew();
                    xObj = pDlg->GetObject();

                    xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                    if ( xIconMetaFile.is() )
                        nAspect = embed::Aspects::MSOLE_ICON;

                    if ( xObj.is() )
                        mpViewShell->GetObjectShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );

                    break;
                }
            }
        }

        try
        {
            if (xObj.is())
            {
                bool bInsertNewObject = true;

                Size aSize;
                MapUnit aMapUnit = MapUnit::Map100thMM;
                if ( nAspect != embed::Aspects::MSOLE_ICON )
                {
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( nAspect );
                    }
                    catch( embed::NoVisualAreaSizeException& )
                    {
                        // the default size will be set later
                    }

                    aSize =Size( aSz.Width, aSz.Height );

                    aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                    if (aSize.IsEmpty())
                    {
                        // rectangle with balanced edge ratio
                        aSize.setWidth( 14100 );
                        aSize.setHeight( 10000 );
                        Size aTmp = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                        aSz.Width = aTmp.Width();
                        aSz.Height = aTmp.Height();
                        xObj->setVisualAreaSize( nAspect, aSz );
                    }
                    else
                    {
                        aSize = OutputDevice::LogicToLogic(aSize, MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));
                    }
                }

                if ( mpView->AreObjectsMarked() )
                {
                    // as an empty OLE object available?
                    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetMarkedSdrObj();

                        if (pObj->GetObjInventor() == SdrInventor::Default &&
                        pObj->GetObjIdentifier() == OBJ_OLE2)
                        {
                            if ( !static_cast<SdrOle2Obj*>(pObj)->GetObjRef().is() )
                            {
                                // the empty OLE object gets a new IPObj
                                bInsertNewObject = false;
                                pObj->SetEmptyPresObj(false);
                                static_cast<SdrOle2Obj*>(pObj)->SetOutlinerParaObject(std::nullopt);
                                static_cast<SdrOle2Obj*>(pObj)->SetObjRef(xObj);
                                static_cast<SdrOle2Obj*>(pObj)->SetPersistName(aName);
                                static_cast<SdrOle2Obj*>(pObj)->SetName(aName);
                                static_cast<SdrOle2Obj*>(pObj)->SetAspect(nAspect);
                                ::tools::Rectangle aRect = static_cast<SdrOle2Obj*>(pObj)->GetLogicRect();

                                if ( nAspect == embed::Aspects::MSOLE_ICON )
                                {
                                    if( xIconMetaFile.is() )
                                        static_cast<SdrOle2Obj*>(pObj)->SetGraphicToObj( xIconMetaFile, aIconMediaType );
                                }
                                else
                                {
                                    Size aTmp = OutputDevice::LogicToLogic(aRect.GetSize(), MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                                    awt::Size aSz( aTmp.Width(), aTmp.Height() );
                                    xObj->setVisualAreaSize( nAspect, aSz );
                                }
                            }
                        }
                    }
                }

                if (bInsertNewObject)
                {
                    // we create a new OLE object
                    SdrPageView* pPV = mpView->GetSdrPageView();
                    Size aPageSize = pPV->GetPage()->GetSize();

                    // get the size from the iconified object
                    ::svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
                    if ( nAspect == embed::Aspects::MSOLE_ICON )
                    {
                        aObjRef.SetGraphicStream( xIconMetaFile, aIconMediaType );
                        MapMode aMapMode( MapUnit::Map100thMM );
                        aSize = aObjRef.GetSize( &aMapMode );
                    }

                    Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                        (aPageSize.Height() - aSize.Height()) / 2);
                    ::tools::Rectangle aRect (aPnt, aSize);
                    SdrOle2Obj* pObj = new SdrOle2Obj(
                        mpView->getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        aRect);

                    if( mpView->InsertObjectAtView(pObj, *pPV, SdrInsertFlags::SETDEFLAYER) )
                    {
                        //  Math objects change their object size during InsertObject.
                        //  New size must be set in SdrObject, or a wrong scale will be set at
                        //  ActivateObject.

                        if ( nAspect != embed::Aspects::MSOLE_ICON )
                        {
                            try
                            {
                                awt::Size aSz = xObj->getVisualAreaSize( nAspect );

                                Size aNewSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ),
                                    MapMode( aMapUnit ), MapMode( MapUnit::Map100thMM ) );
                                if ( aNewSize != aSize )
                                {
                                    aRect.SetSize( aNewSize );
                                    pObj->SetLogicRect( aRect );
                                }
                            }
                            catch( embed::NoVisualAreaSizeException& )
                            {}
                        }

                        if (bCreateNew)
                        {
                            pObj->SetLogicRect(aRect);

                            if ( nAspect != embed::Aspects::MSOLE_ICON )
                            {
                                Size aTmp = OutputDevice::LogicToLogic(aRect.GetSize(), MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                                awt::Size aSz( aTmp.Width(), aTmp.Height() );
                                xObj->setVisualAreaSize( nAspect, aSz );
                            }

                            mpViewShell->ActivateObject(pObj, embed::EmbedVerbs::MS_OLEVERB_SHOW);
                        }

                        Size aVisSizePixel = mpWindow->GetOutputSizePixel();
                        ::tools::Rectangle aVisAreaWin = mpWindow->PixelToLogic( ::tools::Rectangle( Point(0,0), aVisSizePixel) );
                        mpViewShell->VisAreaChanged(aVisAreaWin);
                        mpDocSh->SetVisArea(aVisAreaWin);
                    }
                }
            }
        }
        catch (uno::Exception&)
        {
            // For some reason the object can not be inserted.  For example
            // because it is password protected and is not properly unlocked.
        }
    }
}

FuInsertAVMedia::FuInsertAVMedia(
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuInsertAVMedia::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuInsertAVMedia( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertAVMedia::DoExecute( SfxRequest& rReq )
{
#if HAVE_FEATURE_AVMEDIA
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( &pReqArgs->Get( rReq.GetSlot() )  );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = !aURL.isEmpty();
        }
    }

    bool bLink(true);
    if (!(bAPI
        || ::avmedia::MediaWindow::executeMediaURLDialog(mpWindow ? mpWindow->GetFrameWeld() : nullptr, aURL, & bLink)
       ))
        return;

    Size aPrefSize;

    if( mpWindow )
        mpWindow->EnterWait();

    if( !::avmedia::MediaWindow::isMediaURL( aURL, "", true, &aPrefSize ) )
    {
        if( mpWindow )
            mpWindow->LeaveWait();

        if( !bAPI )
            ::avmedia::MediaWindow::executeFormatErrorBox(mpWindow->GetFrameWeld());
    }
    else
    {
        Point       aPos;
        Size        aSize;
        sal_Int8    nAction = DND_ACTION_COPY;

        if( aPrefSize.Width() && aPrefSize.Height() )
        {
            if( mpWindow )
                aSize = mpWindow->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
            else
                aSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
        }
        else
            aSize = Size( 5000, 5000 );

        if( mpWindow )
        {
            aPos = mpWindow->PixelToLogic( ::tools::Rectangle( aPos, mpWindow->GetOutputSizePixel() ).Center() );
            aPos.AdjustX( -(aSize.Width() >> 1) );
            aPos.AdjustY( -(aSize.Height() >> 1) );
        }

        mpView->InsertMediaURL( aURL, nAction, aPos, aSize, bLink ) ;

        if( mpWindow )
            mpWindow->LeaveWait();
    }
#else
    (void)rReq;
#endif
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
