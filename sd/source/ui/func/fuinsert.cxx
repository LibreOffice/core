/*************************************************************************
 *
 *  $RCSfile: fuinsert.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:02:36 $
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

#include "fuinsert.hxx"

#include <tools/urlobj.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include <sfx2/request.hxx>
#include <so3/outplace.hxx>
#include <svtools/globalnameitem.hxx>
#include <so3/plugin.hxx>
#include <so3/pastedlg.hxx>
#include <svx/pfiledlg.hxx>
#include <svx/impgrf.hxx>
#include <svx/dialogs.hrc>
#include <svx/linkmgr.hxx>
#include <svx/svdetc.hxx>
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _INSDLG_HXX //autogen
#include <so3/insdlg.hxx>
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
#ifndef _EDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SCH_DLL_HXX //autogen
#include <sch/schdll.hxx>
#endif
#ifndef _FRAMEOBJ_HXX //autogen
#include <sfx2/frameobj.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_OPENGRF_HXX
#include <svx/opengrf.hxx>
#endif

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

namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

TYPEINIT1( FuInsertGraphic, FuPoor );
TYPEINIT1( FuInsertClipboard, FuPoor );
TYPEINIT1( FuInsertOLE, FuPoor );

/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

#ifdef WNT
#pragma optimize ( "", off )
#endif

FuInsertGraphic::FuInsertGraphic (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SvxOpenGraphicDialog    aDlg(SdResId(STR_INSERTGRAPHIC));

    if( aDlg.Execute() == GRFILTER_OK )
    {
        USHORT      nError = 0;
        Graphic     aGraphic;

        if( (nError=aDlg.GetGraphic(aGraphic)) == GRFILTER_OK )
        {
            if( pViewSh->ISA(DrawViewShell))
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
                        SdrObject* pObj = pMark->GetObj();

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
|* FuInsertGraphic::Destruktor
|*
\************************************************************************/

FuInsertGraphic::~FuInsertGraphic()
{
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
    TransferableDataHelper                      aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
    SvPasteObjectDialog*                        pDlg = new SvPasteObjectDialog();
    const String                                aEmptyString;
    ::com::sun::star::datatransfer::DataFlavor  aFlavor;
    ULONG                                       nFormatId;

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

    try
    {
        nFormatId = pDlg->Execute( pWindow, aDataHelper.GetTransferable() );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        nFormatId = 0;
    }

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

/*************************************************************************
|*
|* FuInsertClipboard::Destruktor
|*
\************************************************************************/

FuInsertClipboard::~FuInsertClipboard()
{
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
    String aEmptyStr;

#ifdef STARIMAGE_AVAILABLE
    if (nSlotId == SID_INSERT_IMAGE &&
        (pView->AreObjectsMarked() || pView->IsTextEdit()))
    {
        /**********************************************************************
        * Ist ein Graphik-Objekt selektiert?
        **********************************************************************/
        if (pView->IsTextEdit())
        {
           pView->EndTextEdit();
        }

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();

            if( pObj->GetObjInventor() == SdrInventor &&
                pObj->GetObjIdentifier() == OBJ_GRAF &&
                ((SdrGrafObj*) pObj)->GetGraphicType() == GRAPHIC_BITMAP )
            {
                // Graphik-Objekt wird bearbeitet
                pViewSh->DoVerb(0);
                return;
            }
        }
    }
#endif

    if ( nSlotId == SID_ATTR_TABLE ||
         nSlotId == SID_INSERT_DIAGRAM ||
#ifdef STARIMAGE_AVAILABLE
         nSlotId == SID_INSERT_IMAGE ||
#endif
         nSlotId == SID_INSERT_MATH )
    {
        /**********************************************************************
        * Diagramm oder StarCalc-Tabelle einfuegen
        **********************************************************************/
        SvInPlaceObjectRef aIPObj;
        SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );

        if (nSlotId == SID_INSERT_DIAGRAM)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SCH_CLASSID), aStor);
        }
        else if (nSlotId == SID_ATTR_TABLE)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SC_CLASSID), aStor);
        }
#ifdef STARIMAGE_AVAILABLE
        else if (nSlotId == SID_INSERT_IMAGE)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SIM_CLASSID), aStor);
        }
#endif
        else if (nSlotId == SID_INSERT_MATH)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SM_CLASSID), aStor);
        }

        if ( aIPObj.Is() )
        {
            String aName = pViewShell->GetViewFrame()->GetObjectShell()->
                                InsertObject( aIPObj, aEmptyStr )->GetObjName();

            Size aSize = aIPObj->GetVisArea().GetSize();

            if (aSize.Height() == 0 || aSize.Width() == 0)
            {
                // Rechteck mit ausgewogenem Kantenverhaeltnis
                aSize.Width()  = 14100;
                aSize.Height() = 10000;
                aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aIPObj->GetMapUnit() ) );
            }
            else
                aSize = OutputDevice::LogicToLogic(aSize, aIPObj->GetMapUnit(), MAP_100TH_MM);

            Point aPos;
            Rectangle aWinRect(aPos, pWin->GetOutputSizePixel() );
            aPos = aWinRect.Center();
            aPos = pWindow->PixelToLogic(aPos);
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
            Rectangle aRect (aPos, aSize);
            SdrOle2Obj* pOleObj = new SdrOle2Obj(aIPObj, aName, aRect);
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            if( pView->InsertObject(pOleObj, *pPV, SDRINSERT_SETDEFLAYER) )
            {
                if (nSlotId == SID_INSERT_DIAGRAM)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarChart" ) ) );
                    SchDLL::SetTransparentBackground( aIPObj, TRUE );
                }
                else if (nSlotId == SID_ATTR_TABLE)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarCalc" ) ) );
                }
#ifdef STARIMAGE_AVAILABLE
                else if (nSlotId == SID_INSERT_IMAGE)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarImage" ) ) );
                }
#endif
                else if (nSlotId == SID_INSERT_MATH)
                {
                    pOleObj->SetProgName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarMath" ) ) );
                }

                pView->HideMarkHdl(NULL);
                pOleObj->SetLogicRect(aRect);
                aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aIPObj->GetMapUnit() ) );
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
        SvInPlaceObjectRef aIPObj;
        SvInsertOleObjectDialog* pOleDlg = NULL;
        SvInsertPlugInDialog* pPlugInDlg = NULL;
        SvInsertAppletDialog* pAppletDlg = NULL;
        SfxInsertFloatingFrameDialog* pFrameDlg = NULL;

        if (nSlotId == SID_INSERT_OBJECT)
        {
            SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT, sal_False );
            if ( pNameItem )
            {
                SvGlobalName aName = pNameItem->GetValue();
                aIPObj = SvInPlaceObject::CreateObject( aName );
            }
            else
            {
                pOleDlg = new SvInsertOleObjectDialog();
                pOleDlg->SetHelpId(SID_INSERT_OBJECT);

                // Die Liste ist eine Memberliste, und loescht selbst ihre Eintraege
                SvObjectServerList aServerLst;

                // Liste mit Servern fuellen
                pOleDlg->FillObjectServerList(&aServerLst);

                // Eigenen Eintrag loeschen
                if (pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW)
                {
                    aServerLst.Remove( *GraphicDocShell::ClassFactory() );
                }
                else
                {
                    aServerLst.Remove( *DrawDocShell::ClassFactory() );
                }

                SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
                aIPObj = pOleDlg->Execute(pWin, aStor, &aServerLst );
            }
        }
        else if (nSlotId == SID_INSERT_PLUGIN)
        {
            pPlugInDlg = new SvInsertPlugInDialog();
            pPlugInDlg->SetHelpId(SID_INSERT_PLUGIN);

            SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
            aIPObj = pPlugInDlg->Execute(pWin, aStor);
        }
        else if (nSlotId == SID_INSERT_SOUND || nSlotId == SID_INSERT_VIDEO)
        { // load sound- or video- plugin

            // create special filedialog for plugins
            SvxPluginFileDlg aPluginFileDialog (pWin, nSlotId);

            // open filedlg
            if( ERRCODE_NONE == aPluginFileDialog.Execute () )
            {
                // get URL
                String aStrURL(aPluginFileDialog.GetPath());
                aStrURL = ::URIHelper::SmartRelToAbs( aStrURL, FALSE,
                                                      INetURLObject::WAS_ENCODED,
                                                      INetURLObject::DECODE_UNAMBIGUOUS );

                INetURLObject aURL( aStrURL );

                if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                {
                    // create plugin, initialize, etc.
                    SvFactory *pPlugIn = (SvFactory*) SvPlugInObject::ClassFactory();
                    SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
                    SvPlugInObjectRef xObj = &pPlugIn->CreateAndInit( *pPlugIn, aStor );
                    xObj->SetPlugInMode( (USHORT)PLUGIN_EMBEDED );
                    xObj->SetURL( aURL );
                    aIPObj = (SvInPlaceObject*)&xObj;
                }
                else
                {
                    // PlugIn konnte nicht erzeugt werden
                    String aStrErr( ResId( STR_ERROR_OBJNOCREATE_PLUGIN , SOAPP->GetResMgr() ) );
                    String aMask;
                    aMask += sal_Unicode('%');
                    aStrErr.SearchAndReplace( aMask, aStrURL );
                    ErrorBox( pWin, WB_3DLOOK | WB_OK, aStrErr ).Execute();
                }
            }
        }
        else if (nSlotId == SID_INSERT_APPLET)
        {
            pAppletDlg = new SvInsertAppletDialog();
            pAppletDlg->SetHelpId(SID_INSERT_APPLET);

            SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
            aIPObj = pAppletDlg->Execute(pWin, aStor);
        }
        else if (nSlotId == SID_INSERT_FLOATINGFRAME)
        {
            pFrameDlg = new SfxInsertFloatingFrameDialog( pWin );
//            pFrameDlg->SetHelpId(SID_INSERT_FLOATINGFRAME);

            SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
            aIPObj = pFrameDlg->Execute(aStor);
        }

        if (aIPObj.Is())
        {
            if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & aIPObj->GetMiscStatus() )
                aIPObj->OnDocumentPrinterChanged( pDocSh->GetPrinter(FALSE) );

            BOOL bInsertNewObject = FALSE;
            Size aSize;
            String aName;

            SvEmbeddedInfoObject* pInfoObj = pViewShell->GetViewFrame()
                          ->GetObjectShell()->InsertObject( aIPObj, aEmptyStr );

            if (pInfoObj)
            {
                bInsertNewObject = TRUE;
                aName = pInfoObj->GetObjName();

                aSize = aIPObj->GetVisArea().GetSize();

                if (aSize.Height() == 0 || aSize.Width() == 0)
                {
                    // Rechteck mit ausgewogenem Kantenverhaeltnis
                    aSize.Width()  = 14100;
                    aSize.Height() = 10000;
                    aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aIPObj->GetMapUnit() ) );
                }
                else
                {
                    aSize = OutputDevice::LogicToLogic(aSize, aIPObj->GetMapUnit(), MAP_100TH_MM);
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
                        SdrObject* pObj = pMark->GetObj();

                        if (pObj->GetObjInventor() == SdrInventor &&
                            pObj->GetObjIdentifier() == OBJ_OLE2)
                        {
                            SvInPlaceObjectRef aOldIPObj = ( (SdrOle2Obj*) pObj)->GetObjRef();

                            if ( !aOldIPObj.Is() )
                            {
                                /**************************************************
                                * Das leere OLE-Objekt bekommt ein neues IPObj
                                **************************************************/
                                bInsertNewObject = FALSE;
                                pObj->SetEmptyPresObj(FALSE);
                                ( (SdrOle2Obj*) pObj)->SetOutlinerParaObject(NULL);
                                ( (SdrOle2Obj*) pObj)->SetObjRef(aIPObj);
                                ( (SdrOle2Obj*) pObj)->SetPersistName(aName);
                                ( (SdrOle2Obj*) pObj)->SetName(aName);
                                Rectangle aRect = ( (SdrOle2Obj*) pObj)->GetLogicRect();

                                pView->HideMarkHdl(NULL);
                                aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aIPObj->GetMapUnit() ) );

                                if ( nSlotId == SID_ATTR_TABLE ||
                                     nSlotId == SID_INSERT_DIAGRAM ||
#ifdef STARIMAGE_AVAILABLE
                                     nSlotId == SID_INSERT_IMAGE ||
#endif
                                     nSlotId == SID_INSERT_MATH )
                                {
                                    if (nSlotId == SID_INSERT_DIAGRAM)
                                        SchDLL::SetTransparentBackground( aIPObj, TRUE );

                                    pViewShell->ActivateObject( (SdrOle2Obj*) pObj, SVVERB_SHOW);
                                }
                            }
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

                SdrOle2Obj* pObj = new SdrOle2Obj(aIPObj, aName, aRect);

                if( pView->InsertObject(pObj, *pPV, SDRINSERT_SETDEFLAYER) )
                {
                    //  #73279# Math objects change their object size during InsertObject.
                    //  New size must be set in SdrObject, or a wrong scale will be set at
                    //  ActivateObject.

                    Size aNewSize = Window::LogicToLogic( aIPObj->GetVisArea().GetSize(),
                                    MapMode( aIPObj->GetMapUnit() ), MapMode( MAP_100TH_MM ) );
                    if ( aNewSize != aSize )
                    {
                        aRect.SetSize( aNewSize );
                        pObj->SetLogicRect( aRect );
                    }

                    if (pOleDlg && pOleDlg->IsCreateNew())
                    {
                        pView->HideMarkHdl(NULL);
                        pObj->SetLogicRect(aRect);
                        aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aRect.GetSize(), MAP_100TH_MM, aIPObj->GetMapUnit() ) );
                        pViewShell->ActivateObject(pObj, SVVERB_SHOW);
                    }

                    Size aVisSizePixel = pWin->GetOutputSizePixel();
                    Rectangle aVisAreaWin = pWin->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
                    pViewShell->VisAreaChanged(aVisAreaWin);
                    pDocSh->SetVisArea(aVisAreaWin);
                }
            }
        }

        delete pOleDlg;
        delete pPlugInDlg;
        delete pAppletDlg;
        delete pFrameDlg;
    }
}

/*************************************************************************
|*
|* FuInsertOLE::Destruktor
|*
\************************************************************************/

FuInsertOLE::~FuInsertOLE()
{
}

} // end of namespace sd
