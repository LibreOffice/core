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
#include <svx/pfiledlg.hxx>
#include <svx/dialogs.hrc>
#include <sfx2/linkmgr.hxx>
#include <svx/svdetc.hxx>
#include <avmedia/mediawindow.hxx>
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#include <sfx2/printer.hxx>
#include <sot/clsids.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/transfer.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdomedia.hxx>
#ifndef _EDITENG_HXX //autogen
#include <editeng/editeng.hxx>
#endif
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <svx/svdpagv.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/opengrf.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdlegacy.hxx>
#include <vcl/svapp.hxx>
#include <svx/charthelper.hxx>

#include "app.hrc"
#include "misc.hxx"
#include "sdresid.hxx"
#include "View.hxx"
#include "app.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "sdgrffilter.hxx"
#include "sdxfer.hxx"
#include "undo/undoobjects.hxx"
#include "glob.hrc"

using namespace com::sun::star;

namespace sd {

/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

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
        Graphic aGraphic;
        int nError(aDlg.GetGraphic(aGraphic));

        if(GRFILTER_OK == nError)
        {
            if(mpViewShell && dynamic_cast< DrawViewShell* >(mpViewShell))
            {
                sal_Int8 nAction = DND_ACTION_COPY;
                SdrObject* pPickObj = mpView->GetEmptyPresentationObject( PRESOBJ_GRAPHIC );
                SdrObject* pSingleSelected = mpView->getSelectedIfSingle();
                bool bSelectionReplaced(false);

                if( pPickObj )
                {
                    nAction = DND_ACTION_LINK;
                }
                else if(pSingleSelected)
                {
                    pPickObj = pSingleSelected;
                    nAction = DND_ACTION_MOVE;
                    bSelectionReplaced = true;
                }

                Point aPos;
                Rectangle aRect(aPos, mpWindow->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = mpWindow->PixelToLogic(aPos);
                SdrGrafObj* pGrafObj = mpView->InsertGraphic(aGraphic, nAction, basegfx::B2DPoint(aPos.X(), aPos.Y()), pPickObj, NULL);

                if(pGrafObj && aDlg.IsAsLink())
                {
                    // store link only?
                    String aFltName(aDlg.GetCurrentFilter());
                    String aPath(aDlg.GetPath());

                    pGrafObj->SetGraphicLink(aPath, aFltName);
                }

                if(bSelectionReplaced && pGrafObj)
                {
                    mpView->MarkObj(*pGrafObj);
                }
            }
        }
        else
        {
            SdGRFFilter::HandleGraphicFilterError( (sal_uInt16)nError, GraphicFilter::GetGraphicFilter()->GetLastError().nStreamError );
        }
    }
}

/*************************************************************************
|*
|* FuInsertClipboard::Konstruktor
|*
\************************************************************************/

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
        const String                                aEmptyString;
        ::com::sun::star::datatransfer::DataFlavor  aFlavor;

        pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_LINK_SOURCE, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_DRAWING, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_SVXB, aEmptyString );
        pDlg->Insert( FORMAT_GDIMETAFILE, aEmptyString );
        pDlg->Insert( FORMAT_BITMAP, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aEmptyString );
        pDlg->Insert( FORMAT_STRING, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_HTML, aEmptyString );
        pDlg->Insert( FORMAT_RTF, aEmptyString );
        pDlg->Insert( SOT_FORMATSTR_ID_EDITENGINE, aEmptyString );

        //TODO/MBA: testing
        nFormatId = pDlg->GetFormat( aDataHelper );
        if( nFormatId && aDataHelper.GetTransferable().is() )
        {
            sal_Int8 nAction = DND_ACTION_COPY;

            if( !mpView->InsertData( aDataHelper,
                                    mpWindow->GetLogicRange().getCenter(),
                                    nAction, false, nFormatId ) &&
                ( mpViewShell && dynamic_cast< DrawViewShell* >(mpViewShell) ) )
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
                    pDrViewSh->InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, 0);
                }
            }
        }

        delete pDlg;
    }
}


/*************************************************************************
|*
|* FuInsertOLE::Konstruktor
|*
\************************************************************************/

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

        /**********************************************************************
        * Diagramm oder StarCalc-Tabelle einfuegen
        **********************************************************************/

        ::rtl::OUString aObjName;
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
            basegfx::B2DHomMatrix aObjTrans;

            if( pPickObj )
            {
                const basegfx::B2DVector& rScale(pPickObj->getSdrObjectScale());
                awt::Size aSz;

                aObjTrans = pPickObj->getSdrObjectTransformation();
                aSz.Width = basegfx::fround(fabs(rScale.getX()));
                aSz.Height = basegfx::fround(fabs(rScale.getY()));
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

                basegfx::B2DVector aScale(aSz.Width, aSz.Height);

                if(basegfx::fTools::equalZero(aScale.getX()) || basegfx::fTools::equalZero(aScale.getY()))
                {
                    // Rechteck mit ausgewogenem Kantenverhaeltnis
                    const double fFactor(OutputDevice::GetFactorLogicToLogic(MAP_100TH_MM, aUnit));

                    aScale = basegfx::B2DVector(14100.0, 10000.0);
                    aSz.Width = basegfx::fround(fabs(aScale.getX() * fFactor));
                    aSz.Height = basegfx::fround(fabs(aScale.getY() * fFactor));
                    xObj->setVisualAreaSize(nAspect, aSz);
                }
                else
                {
                    aScale *= OutputDevice::GetFactorLogicToLogic(aUnit, MAP_100TH_MM);
                }

                aObjTrans = basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aScale,
                    mpWindow->GetLogicRange().getCenter() - (aScale * 0.5));
            }

            SdrOle2Obj* pOleObj = new SdrOle2Obj(
                mpView->getSdrModelFromSdrView(),
                svt::EmbeddedObjectRef( xObj, nAspect ),
                aObjName,
                aObjTrans);

            // if we have a pick obj we need to make this new ole a pres obj replacing the current pick obj
            if( pPickObj )
            {
                SdPage* pPage = static_cast< SdPage* >(pPickObj->getSdrPageFromSdrObject());
                if(pPage && pPage->IsPresObj(pPickObj))
                {
                    pPage->InsertPresObj( pOleObj, ePresObjKind );

                    // replace formally used 'pOleObj->SetUserCall(pPickObj->GetUserCall())' by
                    // new notify/listener mechanism
                    SdPage* pCurrentlyConnectedSdPage = findConnectionToSdrObject(pPickObj);
                    establishConnectionToSdrObject(pOleObj, pCurrentlyConnectedSdPage);
                }

                // #123468# we need to end text edit before replacing the object. There cannot yet
                // being text typed (else it would not be an EmptyPresObj anymore), but it may be
                // in text edit mode
                if(mpView->IsTextEdit())
                {
                    mpView->SdrEndTextEdit();
                }
            }

            bool bRet = true;
            if( pPickObj )
            {
                mpView->ReplaceObjectAtView(*pPickObj, *pOleObj, true );
            }
            else
            {
                bRet = mpView->InsertObjectAtView(*pOleObj, SDRINSERT_SETDEFLAYER);
            }

            if( bRet )
            {
                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarChart" ) ) );
                }
                else if (nSlotId == SID_ATTR_TABLE)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarCalc" ) ) );
                }
                else if (nSlotId == SID_INSERT_MATH)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarMath" ) ) );
                }

                pOleObj->setSdrObjectTransformation(aObjTrans);
                const basegfx::B2DVector& rScale = pOleObj->getSdrObjectScale();
                const sal_Int32 nWidth(basegfx::fround(fabs(rScale.getX())));
                const sal_Int32 nHeight(basegfx::fround(fabs(rScale.getY())));
                Size aTmp(OutputDevice::LogicToLogic(Size(nWidth, nHeight), MAP_100TH_MM, aUnit));
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
        /**********************************************************************
        * Objekt einfuegen
        **********************************************************************/
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        bool bCreateNew = false;
        uno::Reference < embed::XEmbeddedObject > xObj;
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
        SvObjectServerList aServerLst;
        ::rtl::OUString aName;

        ::rtl::OUString aIconMediaType;
        uno::Reference< io::XInputStream > xIconMetaFile;

        SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT );
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
                        String aStrURL(aPluginFileDialog.GetPath());
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
                                    xSet->setPropertyValue( ::rtl::OUString::createFromAscii("PluginURL"),
                                            uno::makeAny( ::rtl::OUString( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                                }
                            }
                        }
                        else
                        {
                            // PlugIn konnte nicht erzeugt werden
                            String aStrErr( SdResId( STR_ERROR_OBJNOCREATE_PLUGIN ) );
                            String aMask;
                            aMask += sal_Unicode('%');
                            aStrErr.SearchAndReplace( aMask, aStrURL );
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
                //    aIPObj->OnDocumentPrinterChanged( mpDocSh->GetPrinter(false) );

                bool bInsertNewObject = true;
                basegfx::B2DVector aSize(0.0, 0.0);
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

                    aSize = basegfx::B2DVector( aSz.Width, aSz.Height );

                    aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                    if(basegfx::fTools::equalZero(aSize.getX()) || basegfx::fTools::equalZero(aSize.getY()))
                    {
                        // Rechteck mit ausgewogenem Kantenverhaeltnis
                        aSize.setX(14100.0);
                        aSize.setY(10000.0);
                        const basegfx::B2DVector aTemp(aSize * OutputDevice::GetFactorLogicToLogic(MAP_100TH_MM, aMapUnit));
                        aSz.Width = basegfx::fround(aTemp.getX());
                        aSz.Height = basegfx::fround(aTemp.getY());
                        xObj->setVisualAreaSize( nAspect, aSz );
                    }
                    else
                    {
                        aSize *= OutputDevice::GetFactorLogicToLogic(aMapUnit, MAP_100TH_MM);
                    }
                }

                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(mpView->getSelectedIfSingle());

                if ( pSdrOle2Obj )
                {
                    /**********************************************************
                        * Ist ein leeres OLE-Objekt vorhanden?
                        **********************************************************/
                    if ( !pSdrOle2Obj->GetObjRef().is() )
                            {
                                /**************************************************
                                    * Das leere OLE-Objekt bekommt ein neues IPObj
                                    **************************************************/
                        bInsertNewObject = false;
                        pSdrOle2Obj->SetEmptyPresObj(false);
                        pSdrOle2Obj->SetOutlinerParaObject(NULL);
                        pSdrOle2Obj->SetObjRef(xObj);
                        pSdrOle2Obj->SetPersistName(aName);
                        pSdrOle2Obj->SetName(aName);
                        pSdrOle2Obj->SetAspect(nAspect);

                                if ( nAspect == embed::Aspects::MSOLE_ICON )
                                {
                                    if( xIconMetaFile.is() )
                                pSdrOle2Obj->SetGraphicToObj( xIconMetaFile, aIconMediaType );
                                }
                                else
                                {
                            const basegfx::B2DVector aTemp(sdr::legacy::GetLogicRange(*pSdrOle2Obj).getRange() *
                                OutputDevice::GetFactorLogicToLogic(MAP_100TH_MM, aMapUnit));
                            const awt::Size aSz(basegfx::fround(aTemp.getX()), basegfx::fround(aTemp.getY()));

                                    xObj->setVisualAreaSize( nAspect, aSz );
                                }
                            }
                        }

                if (bInsertNewObject)
                {
                    /**************************************************************
                        * Ein neues OLE-Objekt wird erzeugt
                        **************************************************************/
                    // get the size from the iconified object
                    ::svt::EmbeddedObjectRef aObjRef( xObj, nAspect );

                    if ( nAspect == embed::Aspects::MSOLE_ICON )
                    {
                        aObjRef.SetGraphicStream( xIconMetaFile, aIconMediaType );
                        MapMode aMapMode( MAP_100TH_MM );
                        const Size aOldSize(aObjRef.GetSize( &aMapMode ));
                        aSize = basegfx::B2DVector(aOldSize.Width(), aOldSize.Height());
                    }

                    SdrPageView* pPV = mpView->GetSdrPageView();
                    const basegfx::B2DVector aPageScale(pPV ? pPV->getSdrPageFromSdrPageView().GetPageScale() : basegfx::B2DVector(0.0, 0.0));
                    const basegfx::B2DPoint aPnt((aPageScale - aSize) * 0.5);

                    SdrOle2Obj* pObj = new SdrOle2Obj(
                        mpView->getSdrModelFromSdrView(),
                        aObjRef,
                        aName,
                        basegfx::tools::createScaleTranslateB2DHomMatrix(aSize, aPnt));

                    if( mpView->InsertObjectAtView(*pObj, SDRINSERT_SETDEFLAYER) )
                    {
                        //  #73279# Math objects change their object size during InsertObjectToSdrObjList.
                        //  New size must be set in SdrObject, or a wrong scale will be set at
                        //  ActivateObject.

                        if ( nAspect != embed::Aspects::MSOLE_ICON )
                        {
                            try
                            {
                                const awt::Size aSz = xObj->getVisualAreaSize( nAspect );
                                const basegfx::B2DVector aNewSize(basegfx::B2DVector(aSz.Width, aSz.Height) *
                                    Window::GetFactorLogicToLogic(aMapUnit, MAP_100TH_MM));

                                if(!aNewSize.equal(aSize))
                                {
                                    sdr::legacy::SetLogicRange(*pObj, basegfx::B2DRange(aPnt, aPnt + aNewSize));
                                }
                            }
                            catch( embed::NoVisualAreaSizeException& )
                            {}
                        }

                        if (bCreateNew)
                        {
                            sdr::legacy::SetLogicRange(*pObj, basegfx::B2DRange(aPnt, aPnt + aSize));

                            if ( nAspect != embed::Aspects::MSOLE_ICON )
                            {
                                const basegfx::B2DVector aTemp(aSize * OutputDevice::GetFactorLogicToLogic(MAP_100TH_MM, aMapUnit));
                                const awt::Size aSz(basegfx::fround(aTemp.getX()), basegfx::fround(aTemp.getY()));
                                xObj->setVisualAreaSize( nAspect, aSz );
                            }

                            mpViewShell->ActivateObject(pObj, SVVERB_SHOW);
                        }

                        const basegfx::B2DRange aVisAreaWin(mpWindow->GetLogicRange());
                        const Rectangle aOldArea(
                            basegfx::fround(aVisAreaWin.getMinX()), basegfx::fround(aVisAreaWin.getMinY()),
                            basegfx::fround(aVisAreaWin.getMaxX()), basegfx::fround(aVisAreaWin.getMaxY()));
                        mpViewShell->VisAreaChanged(aOldArea);
                        mpDocSh->SetVisArea(aOldArea);
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


/*************************************************************************
|*
|* FuInsertAVMedia::Konstruktor
|*
\************************************************************************/

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
    ::rtl::OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast< const SfxStringItem* >(&pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    if( bAPI || ::avmedia::MediaWindow::executeMediaURLDialog( mpWindow, aURL ) )
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
            basegfx::B2DPoint aPosition(0.0, 0.0);
            basegfx::B2DVector aScale(aPrefSize.Width(), aPrefSize.Height());

            if(basegfx::fTools::equalZero(aScale.getX()) || basegfx::fTools::equalZero(aScale.getY()))
            {
                aScale = basegfx::B2DVector(5000.0, 5000.0);
            }
            else
            {
                aScale *= Application::GetDefaultDevice()->GetFactorLogicToLogic(MAP_PIXEL, MAP_100TH_MM);
            }

            if(mpWindow)
            {
                aPosition = mpWindow->GetLogicRange().getCenter() - (aScale * 0.5);
            }

            sal_Int8 nAction(DND_ACTION_COPY);

            mpView->InsertMediaURL(
                aURL,
                nAction,
                aPosition,
                aScale);

            if(mpWindow)
            {
                mpWindow->LeaveWait();
            }
        }
    }
}

} // end of namespace sd
