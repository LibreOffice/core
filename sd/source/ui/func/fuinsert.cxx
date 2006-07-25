/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuinsert.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 11:34:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "fuinsert.hxx"

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxdlg.hxx>

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <tools/urlobj.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include <svtools/sores.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/request.hxx>
#include <svtools/globalnameitem.hxx>
#include <svtools/pathoptions.hxx>
#include <svx/pfiledlg.hxx>
#include <svx/impgrf.hxx>
#include <svx/dialogs.hrc>
#include <svx/linkmgr.hxx>
#include <svx/svdetc.hxx>
#include <avmedia/mediawindow.hxx>
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#include <sot/clsids.hxx>
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOMEDIA_HXX //autogen
#include <svx/svdomedia.hxx>
#endif
#ifndef _EDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SCH_DLL_HXX //autogen
#include <sch/schdll.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_OPENGRF_HXX
#include <svx/opengrf.hxx>
#endif

#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "misc.hxx"
#include "sdresid.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "app.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#include "strings.hrc"
#include "graphpro.hxx"
#include "drawdoc.hxx"
#include "sdgrffilter.hxx"
#include "sdxfer.hxx"

using namespace com::sun::star;

namespace sd {

TYPEINIT1( FuInsertGraphic, FuPoor );
TYPEINIT1( FuInsertClipboard, FuPoor );
TYPEINIT1( FuInsertOLE, FuPoor );
TYPEINIT1( FuInsertAVMedia, FuPoor );

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

#ifdef WNT
#pragma optimize ( "", off )
#endif

void FuInsertGraphic::DoExecute( SfxRequest& rReq )
{
    SvxOpenGraphicDialog    aDlg(SdResId(STR_INSERTGRAPHIC));

    if( aDlg.Execute() == GRFILTER_OK )
    {
        USHORT      nError = 0;
        Graphic     aGraphic;

        if( (nError=aDlg.GetGraphic(aGraphic)) == GRFILTER_OK )
        {
            if( pViewShell && pViewShell->ISA(DrawViewShell))
            {
                sal_Int8    nAction = DND_ACTION_COPY;
                SdrGrafObj* pEmptyGrafObj = NULL;

                if ( pView->AreObjectsMarked() )
                {
                    /**********************************************************
                    * Is an empty graphic object available?
                    **********************************************************/
                    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetMarkedSdrObj();

                        if (pObj->GetObjInventor() == SdrInventor &&
                            pObj->GetObjIdentifier() == OBJ_GRAF)
                        {
                            nAction = DND_ACTION_LINK;
                            pEmptyGrafObj = (SdrGrafObj*) pObj;
                        }
                    }
                }

                Point aPos;
                Rectangle aRect(aPos, pWindow->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = pWindow->PixelToLogic(aPos);
                SdrGrafObj* pGrafObj = pView->InsertGraphic(aGraphic, nAction, aPos, pEmptyGrafObj, NULL);

                if(pGrafObj && aDlg.IsAsLink())
                {
                    // store link only?
                    String aFltName(aDlg.GetCurrentFilter());
                    String aPath(aDlg.GetPath());
                    pGrafObj->SetGraphicLink(aPath, aFltName);
                }
            }
        }
        else
        {
            SdGRFFilter::HandleGraphicFilterError( nError, GetGrfFilter()->GetLastError().nStreamError );
        }
    }
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

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

void FuInsertClipboard::DoExecute( SfxRequest& rReq )
{
    TransferableDataHelper                      aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWindow ) );
    ULONG                                       nFormatId;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pViewShell->GetActiveWindow() );
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

            if( !pView->InsertData( aDataHelper,
                                    pWindow->PixelToLogic( Rectangle( Point(), pWindow->GetOutputSizePixel() ).Center() ),
                                    nAction, FALSE, nFormatId ) &&
                ( pViewShell && pViewShell->ISA( DrawViewShell ) ) )
            {
                DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(pViewShell);
                String              aEmptyStr;
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
    String aEmptyStr;

    if ( nSlotId == SID_ATTR_TABLE ||
         nSlotId == SID_INSERT_DIAGRAM ||
         nSlotId == SID_INSERT_MATH )
    {
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

        uno::Reference < embed::XEmbeddedObject > xObj = pViewShell->GetViewFrame()->GetObjectShell()->
                GetEmbeddedObjectContainer().CreateEmbeddedObject( aName.GetByteSequence(), aObjName );
        if ( xObj.is() )
        {
            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
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

            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            if (aSize.Height() == 0 || aSize.Width() == 0)
            {
                // Rechteck mit ausgewogenem Kantenverhaeltnis
                aSize.Width()  = 14100;
                aSize.Height() = 10000;
                Size aTmp = OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aUnit );
                aSz.Width = aTmp.Width();
                aSz.Height = aTmp.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }
            else
                aSize = OutputDevice::LogicToLogic(aSize, aUnit, MAP_100TH_MM);

            Point aPos;
            Rectangle aWinRect(aPos, pWindow->GetOutputSizePixel() );
            aPos = aWinRect.Center();
            aPos = pWindow->PixelToLogic(aPos);
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
            Rectangle aRect (aPos, aSize);
            SdrOle2Obj* pOleObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aObjName, aRect );
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            if( pView->InsertObject(pOleObj, *pPV, SDRINSERT_SETDEFLAYER) )
            {
                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarChart" ) ) );
                    SchDLL::SetTransparentBackground( xObj, TRUE );
                }
                else if (nSlotId == SID_ATTR_TABLE)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarCalc" ) ) );
                }
                else if (nSlotId == SID_INSERT_MATH)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarMath" ) ) );
                }

                pView->HideMarkHdl(NULL);
                pOleObj->SetLogicRect(aRect);
                Size aTmp = OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aUnit );
                aSz.Width = aTmp.Width();
                aSz.Height = aTmp.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
                pViewShell->ActivateObject(pOleObj, SVVERB_SHOW);
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
        BOOL bCreateNew = FALSE;
        uno::Reference < embed::XEmbeddedObject > xObj;
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
        SvObjectServerList aServerLst;
        ::rtl::OUString aName;

        SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT, sal_False );
        if ( nSlotId == SID_INSERT_OBJECT && pNameItem )
        {
            SvGlobalName aClassName = pNameItem->GetValue();
            xObj =  pViewShell->GetViewFrame()->GetObjectShell()->
                    GetEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aName );
        }
        else
        {
            switch ( nSlotId )
            {
                case SID_INSERT_OBJECT :
                {
                    aServerLst.FillInsertObjects();
                    if (pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW)
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
                case SID_INSERT_APPLET :
                case SID_INSERT_FLOATINGFRAME :
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    SfxAbstractInsertObjectDialog* pDlg =
                            pFact->CreateInsertObjectDialog( pViewShell->GetActiveWindow(), nSlotId,
                            xStorage, &aServerLst );
                    if ( pDlg )
                    {
                        pDlg->Execute();
                        bCreateNew = pDlg->IsCreateNew();
                        xObj = pDlg->GetObject();
                        if ( xObj.is() )
                            pViewShell->GetObjectShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                        DELETEZ( pDlg );
                    }

                    break;
                }
                case SID_INSERT_SOUND :
                case SID_INSERT_VIDEO :
                {
                    // create special filedialog for plugins
                    SvxPluginFileDlg aPluginFileDialog (pWindow, nSlotId);
                    if( ERRCODE_NONE == aPluginFileDialog.Execute () )
                    {
                        // get URL
                        String aStrURL(aPluginFileDialog.GetPath());
                        INetURLObject aURL( aStrURL, INET_PROT_FILE );
                        if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                        {
                            // create a plugin object
                            xObj = pViewShell->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), aName );
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
                            String aStrErr( ResId( STR_ERROR_OBJNOCREATE_PLUGIN ) );
                            String aMask;
                            aMask += sal_Unicode('%');
                            aStrErr.SearchAndReplace( aMask, aStrURL );
                            ErrorBox( pWindow, WB_3DLOOK | WB_OK, aStrErr ).Execute();
                        }
                    }
                }
            }
        }

        if (xObj.is())
        {
            //TODO/LATER: needs status for RESIZEONPRINTERCHANGE
            //if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->getStatus( nAspect ) )
            //    aIPObj->OnDocumentPrinterChanged( pDocSh->GetPrinter(FALSE) );

            BOOL bInsertNewObject = TRUE;
            awt::Size aSz;
            try
            {
                aSz = xObj->getVisualAreaSize( nAspect );
            }
            catch( embed::NoVisualAreaSizeException& )
            {
                // the default size will be set later
            }

            Size aSize( aSz.Width, aSz.Height );

            MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            if (aSize.Height() == 0 || aSize.Width() == 0)
            {
                // Rechteck mit ausgewogenem Kantenverhaeltnis
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

            if ( pView->AreObjectsMarked() )
            {
                /**********************************************************
                * Ist ein leeres OLE-Objekt vorhanden?
                **********************************************************/
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

                if (rMarkList.GetMarkCount() == 1)
                {
                    SdrMark* pMark = rMarkList.GetMark(0);
                    SdrObject* pObj = pMark->GetMarkedSdrObj();

                    if (pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_OLE2)
                    {
                        if ( !( (SdrOle2Obj*) pObj)->GetObjRef().is() )
                        {
                            /**************************************************
                            * Das leere OLE-Objekt bekommt ein neues IPObj
                            **************************************************/
                            bInsertNewObject = FALSE;
                            pObj->SetEmptyPresObj(FALSE);
                            ( (SdrOle2Obj*) pObj)->SetOutlinerParaObject(NULL);
                            ( (SdrOle2Obj*) pObj)->SetObjRef(xObj);
                            ( (SdrOle2Obj*) pObj)->SetPersistName(aName);
                            ( (SdrOle2Obj*) pObj)->SetName(aName);
                            Rectangle aRect = ( (SdrOle2Obj*) pObj)->GetLogicRect();

                            pView->HideMarkHdl(NULL);
                            Size aTmp = OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aMapUnit );
                            aSz.Width = aTmp.Width();
                            aSz.Height = aTmp.Height();
                            xObj->setVisualAreaSize( nAspect, aSz );
                        }
                    }
                }
            }

            if (bInsertNewObject)
            {
                /**************************************************************
                * Ein neues OLE-Objekt wird erzeugt
                **************************************************************/
                SdrPageView* pPV = pView->GetPageViewPvNum(0);
                Size aPageSize = pPV->GetPage()->GetSize();
                Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                            (aPageSize.Height() - aSize.Height()) / 2);
                Rectangle aRect (aPnt, aSize);

                SdrOle2Obj* pObj = new SdrOle2Obj(svt::EmbeddedObjectRef( xObj, nAspect ), aName, aRect);

                if( pView->InsertObject(pObj, *pPV, SDRINSERT_SETDEFLAYER) )
                {
                    //  #73279# Math objects change their object size during InsertObject.
                    //  New size must be set in SdrObject, or a wrong scale will be set at
                    //  ActivateObject.

                    try
                    {
                        aSz = xObj->getVisualAreaSize( nAspect );

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

                    if (bCreateNew)
                    {
                        pView->HideMarkHdl(NULL);
                        pObj->SetLogicRect(aRect);
                        Size aTmp = OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aMapUnit );
                        aSz.Width = aTmp.Width();
                        aSz.Height = aTmp.Height();
                        xObj->setVisualAreaSize( nAspect, aSz );
                        pViewShell->ActivateObject(pObj, SVVERB_SHOW);
                    }

                    Size aVisSizePixel = pWindow->GetOutputSizePixel();
                    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
                    pViewShell->VisAreaChanged(aVisAreaWin);
                    pDocSh->SetVisArea(aVisAreaWin);
                }
            }
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
        const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    if( bAPI || ::avmedia::MediaWindow::executeMediaURLDialog( pWindow, aURL ) )
    {
        Size aPrefSize;

        if( pWindow )
            pWindow->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
        {
            if( pWindow )
                pWindow->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( pWindow );
        }
        else
        {
            Point       aPos;
            Size        aSize;
            sal_Int8    nAction = DND_ACTION_COPY;

            if( aPrefSize.Width() && aPrefSize.Height() )
            {
                if( pWindow )
                    aSize = pWindow->PixelToLogic( aPrefSize, MAP_100TH_MM );
                else
                    aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
            }
            else
                aSize = Size( 5000, 5000 );

            if( pWindow )
            {
                aPos = pWindow->PixelToLogic( Rectangle( aPos, pWindow->GetOutputSizePixel() ).Center() );
                aPos.X() -= aSize.Width() >> 1;
                aPos.Y() -= aSize.Height() >> 1;
            }

            pView->InsertMediaURL( aURL, nAction, aPos, aSize ) ;

            if( pWindow )
                pWindow->LeaveWait();
        }
    }
}

} // end of namespace sd
