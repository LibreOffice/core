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


#include "fuinsert.hxx"

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxdlg.hxx>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/msgpool.hxx>
#include <svtools/sores.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/request.hxx>
#include <svl/globalnameitem.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svx/pfiledlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/linkmgr.hxx>
#include <svx/linkwarn.hxx>
#include <svx/svdetc.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/printer.hxx>
#include <comphelper/classids.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/transfer.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdomedia.hxx>
#include <editeng/editeng.hxx>
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/charthelper.hxx>

#include "app.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "sdmod.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "sdgrffilter.hxx"
#include "sdxfer.hxx"
#include <vcl/svapp.hxx>
#include "undo/undoobjects.hxx"
#include "glob.hrc"

using namespace com::sun::star;

namespace sd {

TYPEINIT1( FuInsertGraphic, FuPoor );
TYPEINIT1( FuInsertClipboard, FuPoor );
TYPEINIT1( FuInsertOLE, FuPoor );
TYPEINIT1( FuInsertAVMedia, FuPoor );

FuInsertGraphic::FuInsertGraphic (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuInsertGraphic::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuInsertGraphic( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertGraphic::DoExecute( SfxRequest&  )
{
    SvxOpenGraphicDialog    aDlg(SdResId(STR_INSERTGRAPHIC));

    if( aDlg.Execute() == GRFILTER_OK )
    {
        Graphic     aGraphic;
        int nError = aDlg.GetGraphic(aGraphic);
        if( nError == GRFILTER_OK )
        {
            if( mpViewShell && mpViewShell->ISA(DrawViewShell))
            {
                sal_Int8    nAction = DND_ACTION_COPY;
                SdrObject* pPickObj;

                if( ( pPickObj = mpView->GetSelectedSingleObject( mpView->GetPage() ) ) || ( pPickObj = mpView->GetEmptyPresentationObject( PRESOBJ_GRAPHIC ) ) )
                    nAction = DND_ACTION_LINK;

                Point aPos;
                Rectangle aRect(aPos, mpWindow->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = mpWindow->PixelToLogic(aPos);
                SdrGrafObj* pGrafObj = mpView->InsertGraphic(aGraphic, nAction, aPos, pPickObj, NULL);

                if(pGrafObj && aDlg.IsAsLink())
                {
                    // really store as link only?
                    if( SvtMiscOptions().ShowLinkWarningDialog() )
                    {
                        SvxLinkWarningDialog aWarnDlg(mpWindow,aDlg.GetPath());
                        if( aWarnDlg.Execute() != RET_OK )
                            return; // don't store as link
                    }

                    // store as link
                    OUString aFltName(aDlg.GetCurrentFilter());
                    OUString aPath(aDlg.GetPath());
                    pGrafObj->SetGraphicLink(aPath, aFltName);
                }
            }
        }
        else
        {
            SdGRFFilter::HandleGraphicFilterError( (sal_uInt16)nError, GraphicFilter::GetGraphicFilter().GetLastError().nStreamError );
        }
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

FunctionReference FuInsertClipboard::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuInsertClipboard( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertClipboard::DoExecute( SfxRequest&  )
{
    TransferableDataHelper                      aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpWindow ) );
    sal_uLong                                       nFormatId;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( mpViewShell->GetActiveWindow() );
    if ( pDlg )
    {
        ::com::sun::star::datatransfer::DataFlavor  aFlavor;

        pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_LINK_SOURCE, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_DRAWING, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_SVXB, OUString() );
        pDlg->Insert( FORMAT_GDIMETAFILE, OUString() );
        pDlg->Insert( FORMAT_BITMAP, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, OUString() );
        pDlg->Insert( FORMAT_STRING, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_HTML, OUString() );
        pDlg->Insert( FORMAT_RTF, OUString() );
        pDlg->Insert( SOT_FORMATSTR_ID_EDITENGINE, OUString() );

        //TODO/MBA: testing
        nFormatId = pDlg->GetFormat( aDataHelper );
        if( nFormatId && aDataHelper.GetTransferable().is() )
        {
            sal_Int8 nAction = DND_ACTION_COPY;

            if( !mpView->InsertData( aDataHelper,
                                    mpWindow->PixelToLogic( Rectangle( Point(), mpWindow->GetOutputSizePixel() ).Center() ),
                                    nAction, sal_False, nFormatId ) &&
                ( mpViewShell && mpViewShell->ISA( DrawViewShell ) ) )
            {
                DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(mpViewShell);
                INetBookmark        aINetBookmark( aEmptyStr, aEmptyStr );

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

        delete pDlg;
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

FunctionReference FuInsertOLE::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuInsertOLE( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertOLE::DoExecute( SfxRequest& rReq )
{
    if ( nSlotId == SID_ATTR_TABLE ||
         nSlotId == SID_INSERT_DIAGRAM ||
         nSlotId == SID_INSERT_MATH )
    {
        PresObjKind ePresObjKind = (nSlotId == SID_INSERT_DIAGRAM) ? PRESOBJ_CHART : PRESOBJ_OBJECT;

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
            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

            Rectangle aRect;
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

                if (aSize.Height() == 0 || aSize.Width() == 0)
                {
                    // rectangle with balanced edge ratio
                    aSize.Width()  = 14100;
                    aSize.Height() = 10000;
                    Size aTmp = OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aUnit );
                    aSz.Width = aTmp.Width();
                    aSz.Height = aTmp.Height();
                    xObj->setVisualAreaSize( nAspect, aSz );
                }
                else
                {
                    aSize = OutputDevice::LogicToLogic(aSize, aUnit, MAP_100TH_MM);
                }

                Point aPos;
                Rectangle aWinRect(aPos, mpWindow->GetOutputSizePixel() );
                aPos = aWinRect.Center();
                aPos = mpWindow->PixelToLogic(aPos);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;
                aRect = Rectangle(aPos, aSize);
            }

            SdrOle2Obj* pOleObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aObjName, aRect );
            SdrPageView* pPV = mpView->GetSdrPageView();

            // if we have a pick obj we need to make this new ole a pres obj replacing the current pick obj
            if( pPickObj )
            {
                SdPage* pPage = static_cast< SdPage* >(pPickObj->GetPage());
                if(pPage && pPage->IsPresObj(pPickObj))
                {
                    pPage->InsertPresObj( pOleObj, ePresObjKind );
                    pOleObj->SetUserCall(pPickObj->GetUserCall());
                }
            }

            bool bRet = true;
            if( pPickObj )
                mpView->ReplaceObjectAtView(pPickObj, *pPV, pOleObj, sal_True );
            else
                bRet = mpView->InsertObjectAtView(pOleObj, *pPV, SDRINSERT_SETDEFLAYER);

            if( bRet )
            {
                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    pOleObj->SetProgName( OUString( "StarChart" ));
                }
                else if (nSlotId == SID_ATTR_TABLE)
                {
                    pOleObj->SetProgName( OUString( "StarCalc" ) );
                }
                else if (nSlotId == SID_INSERT_MATH)
                {
                    pOleObj->SetProgName( OUString( "StarMath" ) );
                }

                pOleObj->SetLogicRect(aRect);
                Size aTmp( OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aUnit ) );
                awt::Size aVisualSize;
                aVisualSize.Width = aTmp.Width();
                aVisualSize.Height = aTmp.Height();
                xObj->setVisualAreaSize( nAspect, aVisualSize );
                mpViewShell->ActivateObject(pOleObj, SVVERB_SHOW);

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
                                        aEmptyStr ) );
        }
    }
    else
    {
        // insert object
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        sal_Bool bCreateNew = sal_False;
        uno::Reference < embed::XEmbeddedObject > xObj;
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
        SvObjectServerList aServerLst;
        OUString aName;

        OUString aIconMediaType;
        uno::Reference< io::XInputStream > xIconMetaFile;

        SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT, sal_False );
        if ( nSlotId == SID_INSERT_OBJECT && pNameItem )
        {
            SvGlobalName aClassName = pNameItem->GetValue();
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
                    if (mpDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW)
                    {
                        aServerLst.Remove( GraphicDocShell::Factory().GetClassId() );
                    }
                    else
                    {
                        aServerLst.Remove( DrawDocShell::Factory().GetClassId() );
                    }

                    // intentionally no break!
                }
                case SID_INSERT_PLUGIN :
                case SID_INSERT_FLOATINGFRAME :
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    SfxAbstractInsertObjectDialog* pDlg =
                            pFact->CreateInsertObjectDialog( mpViewShell->GetActiveWindow(), SD_MOD()->GetSlotPool()->GetSlot(nSlotId)->GetCommandString(),
                            xStorage, &aServerLst );
                    if ( pDlg )
                    {
                        pDlg->Execute();
                        bCreateNew = pDlg->IsCreateNew();
                        xObj = pDlg->GetObject();

                        xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                        if ( xIconMetaFile.is() )
                            nAspect = embed::Aspects::MSOLE_ICON;

                        if ( xObj.is() )
                            mpViewShell->GetObjectShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                        DELETEZ( pDlg );
                    }

                    break;
                }
                case SID_INSERT_SOUND :
                case SID_INSERT_VIDEO :
                {
                    // create special filedialog for plugins
                    SvxPluginFileDlg aPluginFileDialog (mpWindow, nSlotId);
                    if( ERRCODE_NONE == aPluginFileDialog.Execute () )
                    {
                        // get URL
                        OUString aStrURL(aPluginFileDialog.GetPath());
                        INetURLObject aURL( aStrURL, INET_PROT_FILE );
                        if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                        {
                            // create a plugin object
                            xObj = mpViewShell->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), aName );
                        }

                        if ( xObj.is() && svt::EmbeddedObjectRef::TryRunningState( xObj ) )
                        {
                            // set properties from dialog
                            uno::Reference < embed::XComponentSupplier > xSup( xObj, uno::UNO_QUERY );
                            if ( xSup.is() )
                            {
                                uno::Reference < beans::XPropertySet > xSet( xSup->getComponent(), uno::UNO_QUERY );
                                if ( xSet.is() )
                                {
                                    xSet->setPropertyValue("PluginURL",
                                            uno::makeAny( OUString( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                                }
                            }
                        }
                        else
                        {
                            // unable to create PlugIn
                            OUString aStrErr( SdResId( STR_ERROR_OBJNOCREATE_PLUGIN ) );
                            aStrErr = aStrErr.replaceFirst( "%", aStrURL );
                            ErrorBox( mpWindow, WB_3DLOOK | WB_OK, aStrErr ).Execute();
                        }
                    }
                }
            }
        }

        try
        {
            if (xObj.is())
            {
                //TODO/LATER: needs status for RESIZEONPRINTERCHANGE
                //if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->getStatus( nAspect ) )
                //    aIPObj->OnDocumentPrinterChanged( mpDocSh->GetPrinter(sal_False) );

                sal_Bool bInsertNewObject = sal_True;

                Size aSize;
                MapUnit aMapUnit = MAP_100TH_MM;
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
                    if (aSize.Height() == 0 || aSize.Width() == 0)
                    {
                        // rectangle with balanced edge ratio
                        aSize.Width()  = 14100;
                        aSize.Height() = 10000;
                        Size aTmp = OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aMapUnit );
                        aSz.Width = aTmp.Width();
                        aSz.Height = aTmp.Height();
                        xObj->setVisualAreaSize( nAspect, aSz );
                    }
                    else
                    {
                        aSize = OutputDevice::LogicToLogic(aSize, aMapUnit, MAP_100TH_MM);
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

                        if (pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_OLE2)
                        {
                            if ( !( (SdrOle2Obj*) pObj)->GetObjRef().is() )
                            {
                                // the empty OLE object gets a new IPObj
                                bInsertNewObject = sal_False;
                                pObj->SetEmptyPresObj(sal_False);
                                ( (SdrOle2Obj*) pObj)->SetOutlinerParaObject(NULL);
                                ( (SdrOle2Obj*) pObj)->SetObjRef(xObj);
                                ( (SdrOle2Obj*) pObj)->SetPersistName(aName);
                                ( (SdrOle2Obj*) pObj)->SetName(aName);
                                ( (SdrOle2Obj*) pObj)->SetAspect(nAspect);
                                Rectangle aRect = ( (SdrOle2Obj*) pObj)->GetLogicRect();

                                if ( nAspect == embed::Aspects::MSOLE_ICON )
                                {
                                    if( xIconMetaFile.is() )
                                        ( (SdrOle2Obj*) pObj)->SetGraphicToObj( xIconMetaFile, aIconMediaType );
                                }
                                else
                                {
                                    Size aTmp = OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aMapUnit );
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
                        MapMode aMapMode( MAP_100TH_MM );
                        aSize = aObjRef.GetSize( &aMapMode );
                    }

                    Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                        (aPageSize.Height() - aSize.Height()) / 2);
                    Rectangle aRect (aPnt, aSize);

                    SdrOle2Obj* pObj = new SdrOle2Obj( aObjRef, aName, aRect);

                    if( mpView->InsertObjectAtView(pObj, *pPV, SDRINSERT_SETDEFLAYER) )
                    {
                        //  Math objects change their object size during InsertObject.
                        //  New size must be set in SdrObject, or a wrong scale will be set at
                        //  ActivateObject.

                        if ( nAspect != embed::Aspects::MSOLE_ICON )
                        {
                            try
                            {
                                awt::Size aSz = xObj->getVisualAreaSize( nAspect );

                                Size aNewSize = Window::LogicToLogic( Size( aSz.Width, aSz.Height ),
                                    MapMode( aMapUnit ), MapMode( MAP_100TH_MM ) );
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
                                Size aTmp = OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aMapUnit );
                                awt::Size aSz( aTmp.Width(), aTmp.Height() );
                                xObj->setVisualAreaSize( nAspect, aSz );
                            }

                            mpViewShell->ActivateObject(pObj, SVVERB_SHOW);
                        }

                        Size aVisSizePixel = mpWindow->GetOutputSizePixel();
                        Rectangle aVisAreaWin = mpWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
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

FunctionReference FuInsertAVMedia::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuInsertAVMedia( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertAVMedia::DoExecute( SfxRequest& rReq )
{
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = !aURL.isEmpty();
        }
    }

    bool bLink(true);
    if (bAPI ||
        ::avmedia::MediaWindow::executeMediaURLDialog(mpWindow, aURL, & bLink))
    {
        Size aPrefSize;

        if( mpWindow )
            mpWindow->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
        {
            if( mpWindow )
                mpWindow->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( mpWindow );
        }
        else
        {
            Point       aPos;
            Size        aSize;
            sal_Int8    nAction = DND_ACTION_COPY;

            if( aPrefSize.Width() && aPrefSize.Height() )
            {
                if( mpWindow )
                    aSize = mpWindow->PixelToLogic( aPrefSize, MAP_100TH_MM );
                else
                    aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
            }
            else
                aSize = Size( 5000, 5000 );

            if( mpWindow )
            {
                aPos = mpWindow->PixelToLogic( Rectangle( aPos, mpWindow->GetOutputSizePixel() ).Center() );
                aPos.X() -= aSize.Width() >> 1;
                aPos.Y() -= aSize.Height() >> 1;
            }

            mpView->InsertMediaURL( aURL, nAction, aPos, aSize, bLink ) ;

            if( mpWindow )
                mpWindow->LeaveWait();
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
