/*************************************************************************
 *
 *  $RCSfile: fuinsert.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-16 17:36:36 $
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

#include <tools/urlobj.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include <so3/plugin.hxx>
#include <svx/pfiledlg.hxx>
#include <svx/impgrf.hxx>
#include <svx/dialogs.hrc>
#include <svx/linkmgr.hxx>
#include <svx/svdetc.hxx>
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _PASTEDLG_HXX //autogen
#include <so3/pastedlg.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
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

#include "app.hrc"
#include "misc.hxx"
#include "sdresid.hxx"
#include "sdview.hxx"
#include "app.hxx"
#include "sdwindow.hxx"
#include "drawview.hxx"
#include "drviewsh.hxx"
#include "fuinsert.hxx"
#include "docshell.hxx"
#include "grdocsh.hxx"
#include "strings.hrc"
#include "graphpro.hxx"
#include "drawdoc.hxx"
#include "sdgrffilter.hxx"
#include "sdxfer.hxx"

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
TYPEINIT1( FuInsertTwain, FuPoor );


/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

#ifdef WNT
#pragma optimize ( "", off )
#endif

FuInsertGraphic::FuInsertGraphic(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SvxImportGraphicDialog* pDlg = new SvxImportGraphicDialog( NULL,
                                       String( SdResId(STR_INSERTGRAPHIC) ),
                                       ENABLE_STD_AND_LINK );

    USHORT nResult = pDlg->Execute();

    switch (nResult)
    {
        case RET_OK:
        {
            USHORT      nError = 0;
            Graphic     aGraphic;
            Graphic*    pGraphic = pDlg->GetGraphic();
            String      aPath( pDlg->GetPath() );

            if( pGraphic )
            {
                aGraphic = *pGraphic;
            }
            else
            {
                GraphicFilter&  rFilter = pDlg->GetFilter();
                FilterProgress  aFilterProgress( &rFilter, pViewSh->GetDocSh() );
                SvStream*       pIStm = ::utl::UcbStreamHelper::CreateStream( aPath, STREAM_READ | STREAM_SHARE_DENYNONE );

                if( pIStm )
                {
                    nError = rFilter.ImportGraphic( aGraphic, aPath, *pIStm );
                    delete pIStm;
                }
            }

            if ( nError == 0 && pViewSh->ISA(SdDrawViewShell) )
            {
                sal_Int8    nAction = DND_ACTION_COPY;
                SdrGrafObj* pEmptyGrafObj = NULL;

                if ( pView->HasMarkedObj() )
                {
                    /**********************************************************
                    * Ist ein leeres Graphik-Objekt vorhanden?
                    **********************************************************/
                    const SdrMarkList& rMarkList = pView->GetMarkList();

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

                if (pGrafObj && pDlg->AsLink())
                {
                    // Soll nur ein Link benutzt werden?
                    String aFltName = pDlg->GetCurFilter();
                    pGrafObj->SetGraphicLink(aPath, aFltName);
                }
            }
            else if ( nError )
                SdGRFFilter::HandleGraphicFilterError( nError, GetGrfFilter()->GetLastError().nStreamError );
        }
        break;

        default:
        break;
    }

    delete pDlg;
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

FuInsertClipboard::FuInsertClipboard(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    TransferableDataHelper                      aDataHelper( TransferableDataHelper::CreateFromSystemClipboard() );
    SvPasteObjectDialog*                        pDlg = new SvPasteObjectDialog();
    ::com::sun::star::datatransfer::DataFlavor  aFlavor;

    pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, String() );
    pDlg->Insert( SOT_FORMATSTR_ID_LINK_SOURCE, String() );
    pDlg->Insert( SOT_FORMATSTR_ID_DRAWING, ( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_DRAWING, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( SOT_FORMATSTR_ID_SVXB, ( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_SVXB, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( FORMAT_GDIMETAFILE, ( SotExchange::GetFormatDataFlavor( FORMAT_GDIMETAFILE, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( FORMAT_BITMAP, ( SotExchange::GetFormatDataFlavor( FORMAT_BITMAP, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( FORMAT_STRING, ( SotExchange::GetFormatDataFlavor( FORMAT_STRING, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( SOT_FORMATSTR_ID_HTML, ( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_HTML, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( FORMAT_RTF, ( SotExchange::GetFormatDataFlavor( FORMAT_RTF, aFlavor ), aFlavor.HumanPresentableName ) );
    pDlg->Insert( EditEngine::RegisterClipboardFormatName(), String() );

    const ULONG nFormatId = pDlg->Execute( pWindow, aDataHelper.GetTransferable() );

    if( nFormatId && aDataHelper.GetTransferable().is() )
    {
        sal_Int8 nAction = DND_ACTION_COPY;

        pView->InsertData( aDataHelper.GetTransferable(),
                           pWindow->PixelToLogic( Rectangle( Point(), pWindow->GetOutputSizePixel() ).Center() ),
                           nAction, FALSE, nFormatId );
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

FuInsertOLE::FuInsertOLE(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                         SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    String aEmptyStr;

#ifdef STARIMAGE_AVAILABLE
    if (nSlotId == SID_INSERT_IMAGE &&
        (pView->HasMarkedObj() || pView->IsTextEdit()))
    {
        /**********************************************************************
        * Ist ein Graphik-Objekt selektiert?
        **********************************************************************/
        if (pView->IsTextEdit())
        {
           pView->EndTextEdit();
        }

        const SdrMarkList& rMarkList = pView->GetMarkList();

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

#ifndef SO3
        if (nSlotId == SID_INSERT_DIAGRAM)
        {
            aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit(
                          SvGlobalName(SO3_SCH_CLASSID_50), aStor);
        }
        else if (nSlotId == SID_ATTR_TABLE)
        {
            aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit(
                          SvGlobalName(SO3_SC_CLASSID_50), aStor);
        }
#ifdef STARIMAGE_AVAILABLE
        else if (nSlotId == SID_INSERT_IMAGE)
        {
            aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit(
                          SvGlobalName(SO3_SIM_CLASSID_50), aStor);
        }
#endif
        else if (nSlotId == SID_INSERT_MATH)
        {
            aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit(
                          SvGlobalName(SO3_SM_CLASSID_50), aStor);
        }
#else
        if (nSlotId == SID_INSERT_DIAGRAM)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SCH_CLASSID_50), aStor);
        }
        else if (nSlotId == SID_ATTR_TABLE)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SC_CLASSID_50), aStor);
        }
#ifdef STARIMAGE_AVAILABLE
        else if (nSlotId == SID_INSERT_IMAGE)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SIM_CLASSID_50), aStor);
        }
#endif
        else if (nSlotId == SID_INSERT_MATH)
        {
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                          SvGlobalName(SO3_SM_CLASSID_50), aStor);
        }
#endif

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
                aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic(
                                        aSize, MAP_100TH_MM, aIPObj->GetMapUnit() ) );
            }
            else
            {
                aSize = OutputDevice::LogicToLogic(aSize, aIPObj->GetMapUnit(),
                                                   MAP_100TH_MM);
            }

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
                aIPObj->SetVisAreaSize( aRect.GetSize() );
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
            pOleDlg = new SvInsertOleObjectDialog();
            pOleDlg->SetHelpId(SID_INSERT_OBJECT);

            // Die Liste ist eine Memberliste, und loescht selbst ihre Eintraege
            SvObjectServerList aServerLst;

            // Liste mit Servern fuellen
            pOleDlg->FillObjectServerList(&aServerLst);

            // Eigenen Eintrag loeschen
            if (pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW)
            {
                aServerLst.Remove( *SdGraphicDocShell::ClassFactory() );
            }
            else
            {
                aServerLst.Remove( *SdDrawDocShell::ClassFactory() );
            }

            SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
            aIPObj = pOleDlg->Execute(pWin, aStor, &aServerLst );
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
            if (RET_OK == aPluginFileDialog.Execute ())
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
                    SvFactory * pPlugIn = SvFactory::GetDefaultPlugInFactory();
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
                    aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic(
                                      aSize, MAP_100TH_MM, aIPObj->GetMapUnit() ) );
                }
                else
                {
                    aSize = OutputDevice::LogicToLogic(aSize, aIPObj->GetMapUnit(),
                                                       MAP_100TH_MM);
                }

                if ( pView->HasMarkedObj() )
                {
                    /**********************************************************
                    * Ist ein leeres OLE-Objekt vorhanden?
                    **********************************************************/
                    const SdrMarkList& rMarkList = pView->GetMarkList();

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
                                ( (SdrOle2Obj*) pObj)->SetName(aName);
                                Rectangle aRect = ( (SdrOle2Obj*) pObj)->GetLogicRect();

                                pView->HideMarkHdl(NULL);
                                aIPObj->SetVisAreaSize( aRect.GetSize() );

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
                    if (pOleDlg && pOleDlg->IsCreateNew())
                    {
                        pView->HideMarkHdl(NULL);
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


/*************************************************************************
|*
|* FuInsertTwain::Ctor
|*
\************************************************************************/

FuInsertTwain::FuInsertTwain(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                             SdDrawDocument* pDoc, SfxRequest& rReq,
                             const Bitmap& rBmp )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{

    if ( pViewSh->ISA(SdDrawViewShell) )
    {
        Size aSize;
        Size aBmpSize( rBmp.GetPrefSize() );

        if ( !aBmpSize.Width() || !aBmpSize.Height() )
            aBmpSize = rBmp.GetSizePixel();

        if ( rBmp.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        {
            const MapMode&   rWinMap = pWin->GetMapMode();
            const Fraction&  rWinScaleX = rWinMap.GetScaleX();
            const Fraction&  rWinScaleY = rWinMap.GetScaleY();

            aSize = pWin->PixelToLogic( aBmpSize );
            aSize.Width() = ( aSize.Width() * rWinScaleX.GetNumerator() /
                              rWinScaleX.GetDenominator() );
            aSize.Height() = ( aSize.Height() * rWinScaleY.GetNumerator() /
                               rWinScaleY.GetDenominator() );
        }
        else
            aSize = OutputDevice::LogicToLogic( aBmpSize, rBmp.GetPrefMapMode(),
                                                MapMode( MAP_100TH_MM ) );

        // Groesse ggf. auf Seitengroesse begrenzen
        SdrPage* pPage = pView->GetPageViewPvNum(0)->GetPage();
        Size aPageSize = pPage->GetSize();
        aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ((aSize.Height() > aPageSize.Height()) ||
            (aSize.Width()  > aPageSize.Width()) &&
            aSize.Height() && aPageSize.Height())
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aPageSize.Width() /
                            (float)aPageSize.Height();

            // Grafik an Pagesize anpassen (skaliert)
            if ( fGrfWH < fWinWH )
            {
                aSize.Width() = (long)(aPageSize.Height() * fGrfWH);
                aSize.Height()= aPageSize.Height();
            }
            else if ( fGrfWH > 0.F )
            {
                aSize.Width() = aPageSize.Width();
                aSize.Height()= (long)(aPageSize.Width() / fGrfWH);
            }
        }

        // Ausgaberechteck fuer Grafik setzen
        Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                    (aPageSize.Height() - aSize.Height()) / 2);
        aPnt += Point(pPage->GetLftBorder(), pPage->GetUppBorder());
        Rectangle aRect (aPnt, aSize);

        SdrGrafObj* pGrafObj = NULL;

        BOOL bInsertNewObject = TRUE;

        if ( pView->HasMarkedObj() )
        {
            /**********************************************************
            * Ist ein leeres Graphik-Objekt vorhanden?
            **********************************************************/
            const SdrMarkList& rMarkList = pView->GetMarkList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetObj();

                if (pObj->GetObjInventor() == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_GRAF)
                {
                    pGrafObj = (SdrGrafObj*) pObj;

                    if ( pGrafObj->IsEmptyPresObj() )
                    {
                        /**********************************************
                        * Das leere Graphik-Objekt bekommt eine neue
                        * Graphik
                        **********************************************/
                        bInsertNewObject = FALSE;
                        pGrafObj->SetEmptyPresObj(FALSE);
                        pGrafObj->SetOutlinerParaObject(NULL);
                        pGrafObj->SetGraphic( Graphic( rBmp ) );
                    }
                }
            }
        }

        if (bInsertNewObject)
        {
            pGrafObj = new SdrGrafObj( Graphic( rBmp ), aRect );
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            pView->InsertObject(pGrafObj, *pPV, SDRINSERT_SETDEFLAYER);
        }
    }
}



