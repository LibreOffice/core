/*************************************************************************
 *
 *  $RCSfile: fuinsfil.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-04 13:13:22 $
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

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _EDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFXFILEDLG_HXX //autogen
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif


#pragma hdrstop

#include <svx/dialogs.hrc>

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#include "sdresid.hxx"
#include "fuinsfil.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "sdview.hxx"
#include "strings.hrc"
#include "stlpool.hxx"
#include "glob.hrc"
#include "sdpage.hxx"
#include "strmname.h"
#include "strings.hrc"
#include "drviewsh.hxx"
#include "outlview.hxx"
#include "docshell.hxx"
#include "app.hrc"
#include "unmovss.hxx"
#include "inspagob.hxx"

#include <tools/urlobj.hxx>

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif


TYPEINIT1( FuInsertFile, FuPoor );

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuInsertFile::FuInsertFile(SdViewShell*    pViewSh,
                           SdWindow*       pWin,
                           SdView*         pView,
                           SdDrawDocument* pDoc,
                           SfxRequest&     rReq)
           : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    String           aSDDSpec (SdResId (STR_IMPRESS));
    String           aTemplateSpec(SdResId(STR_TEMPLATE));
    String           aPlainTextSpec (SdResId (STR_FORMAT_STRING));
    String           aRTFSpec (SdResId (STR_FORMAT_RTF));
    String           aHTMLSpec (SdResId(STR_FORMAT_HTML));
    String           aAllSpec (SdResId (STR_ALL_FILES));
    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (!pArgs)
    {
        SfxFileDialog aFileDialog( pWin, SFXWB_INSERT | WB_3DLOOK | WB_STDMODAL );

        SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        SfxFilterContainer* pCont = NULL;
        const SfxFilter* pFilter = NULL;

        if( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
        {
            pCont = rMatcher.GetContainer( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "simpress" ) ) );
            pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50 );
        }
        else
        {
            pCont = rMatcher.GetContainer( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sdraw" ) ) );
            pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50 );
        }

        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

        // set default-filter
        aFileDialog.SetCurFilter( pFilter->GetUIName() );

        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50, SFX_FILTER_TEMPLATEPATH );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50 );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );
        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50, SFX_FILTER_TEMPLATEPATH  );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40 );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );
        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40, SFX_FILTER_TEMPLATEPATH  );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );
        pFilter = pCont->GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW, SFX_FILTER_TEMPLATEPATH  );
        if( pFilter )
            aFileDialog.AddFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

        aFileDialog.SetText(String(SdResId(STR_DLG_INSERT_PAGES_FROM_FILE)));

#ifdef MAC
        aFileDialog.AddFilter( aPlainTextSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.TXT" ) ),
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "TEXT0" ) ) );
        aFileDialog.AddFilter( aRTFSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.RTF" ) ),
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "RTF 0" ) ) );

        // kein MAC-Dateityp, sonst werden *alle* TEXT-Dateien angezeigt
        aFileDialog.AddFilter( aHTMLSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.htm;*.html" ) ) );

        aFileDialog.AddFilter( aAllSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*" ) ) );
#else
        aFileDialog.AddFilter( aPlainTextSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.txt" ) ) );
        aFileDialog.AddFilter( aRTFSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.rtf" ) ));
        aFileDialog.AddFilter( aHTMLSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.htm;*.html" ) ));
        aFileDialog.AddFilter( aAllSpec,
                               UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.*" ) ) );
#endif

        aFileDialog.SetText(String(SdResId(STR_DLG_INSERT_PAGES_FROM_FILE)));

        if (!aFileDialog.Execute ()) return;
        else
        {
            aFilterName = aFileDialog.GetCurFilter();
            aFile       = aFileDialog.GetPath();
            INetURLObject::SetBaseURL(aFile);
            aFile       = ::URIHelper::SmartRelToAbs( aFile, FALSE,
                                                      INetURLObject::WAS_ENCODED,
                                                      INetURLObject::DECODE_UNAMBIGUOUS );
        }
    }
    else
    {
        SFX_REQUEST_ARG (rReq, pFileName, SfxStringItem, ID_VAL_DUMMY0, FALSE);
        SFX_REQUEST_ARG (rReq, pFilterName, SfxStringItem, ID_VAL_DUMMY1, FALSE);

        aFile = pFileName->GetValue ();
        if (pFilterName)
            aFilterName = pFilterName->GetValue ();
        else
        {

            aFilterName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.sdd" ) );
        }
    }

    BOOL bDrawMode  = pViewSh->ISA(SdDrawViewShell);

    pDocSh->SetWaitCursor( TRUE );

    // Das Medium muss ggf. mit READ/WRITE geoeffnet werden, daher wird
    // ersteinmal nachgeschaut, ob es einen Storage enthaelt

    SfxMedium* pMedium = new SfxMedium( aFile,
                                        STREAM_READ | STREAM_NOCREATE,
                                        FALSE,                // direkt
                                        TRUE);                // Download

    const SfxFilter* pFilter = NULL;
    ErrCode nErr = SFX_APP()->GetFilterMatcher().
                              GuessFilter(*pMedium, &pFilter, SFX_FILTER_IMPORT,
                                          SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );
    if(pFilter)
        pMedium->SetFilter(pFilter);

    if (pMedium->IsStorage())
    {
        // Storage
        SvStorage* pStorage = pMedium->GetStorage();

        // Erkennung ueber contained streams (StarDraw 3.0)
        if( pStorage &&
          ( ( pStorage->IsContained(pStarDrawDoc)  &&
              pStorage->IsStream(pStarDrawDoc) )         ||
            ( pStorage->IsContained(pStarDrawDoc3) &&
              pStorage->IsStream(pStarDrawDoc3) ) ) )
        {
            pMedium->Close();

            if (bDrawMode)
                InsSDDinDrMode(pMedium);
            else
                InsSDDinOlMode(pMedium);
        }
        else
        {
            ErrorBox aErrorBox( pWindow, (WinBits)WB_OK,
                      String(SdResId(STR_READ_DATA_ERROR)));
            aErrorBox.Execute();
        }
    }
    else    // kein Storage-Format
    {
        if (pFilter /* && !nErr */)
        {
            aFilterName = pFilter->GetFilterName();

            if (aFilterName.SearchAscii( "Text" ) != STRING_NOTFOUND ||
                aFilterName.EqualsAscii("Rich Text Format")          ||
                aFilterName.EqualsAscii("HTML"))
            {
                if (bDrawMode)
                    InsTextOrRTFinDrMode(pMedium);
                else
                    InsTextOrRTFinOlMode(pMedium);
            }
            else
            {
                ErrorBox aErrorBox( pWindow, (WinBits)WB_OK,
                          String(SdResId(STR_READ_DATA_ERROR)));
                aErrorBox.Execute();
            }
        }
    }

    delete pMedium;

    pDocSh->SetWaitCursor( FALSE );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuInsertFile::~FuInsertFile()
{
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuInsertFile::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuInsertFile::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* SDD im Draw-Mode einlesen
|*
\************************************************************************/

BOOL FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    BOOL bOK = FALSE;

    // Liste mit Seitennamen (wenn NULL, dann alle Seiten)
    List* pBookmarkList = NULL;

    pDocSh->SetWaitCursor( FALSE );
    SdInsertPagesObjsDlg* pDlg = new SdInsertPagesObjsDlg( NULL, pDoc,
                                                    pMedium, aFile );

    // Ev. wird eine QueryBox geoeffnet ("Links aktualisieren?"),
    // daher wird der Dialog der aktuelle DefModalDialogParent
    Window* pDefParent = GetpApp()->GetDefDialogParent();
    GetpApp()->SetDefDialogParent(pDlg);

    USHORT nRet = pDlg->Execute();

    GetpApp()->SetDefDialogParent(pDefParent);

    pDocSh->SetWaitCursor( TRUE );

    if( nRet == RET_OK )
    {
        // Zuerst Seiten einfuegen
        pBookmarkList = pDlg->GetList( 1 ); // Seiten
        BOOL bLink = pDlg->IsLink();
        BOOL bReplace = FALSE;
        SdPage* pPage = NULL;
        SdView* pView = pViewShell->GetView();

        if (pView->ISA(SdOutlineView))
        {
            pPage = ((SdOutlineView*) pView)->GetActualPage();
        }
        else
        {
            pPage = (SdPage*) pView->GetPageViewPvNum(0)->GetPage();
        }

        USHORT nPos = 0xFFFF;

        if (pPage && !pPage->IsMasterPage())
        {
            if (pPage->GetPageKind() == PK_STANDARD)
            {
                nPos = pPage->GetPageNum() + 2;
            }
            else if (pPage->GetPageKind() == PK_NOTES)
            {
                nPos = pPage->GetPageNum() + 1;
            }
        }

        BOOL  bNameOK;
        List* pObjectBookmarkList = pDlg->GetList( 2 ); // Objekte
        List* pExchangeList = NULL;

        // Es werden ausgewaehlte Seiten und/oder ausgewaehlte Objekte oder
        // alles eingefuegt, wenn pBookmarkList NULL ist!
        if( pBookmarkList || !pObjectBookmarkList )
        {
            // Um zu gewaehrleisten, dass alle Seitennamen eindeutig sind, werden
            // die einzufuegenden geprueft und gegebenenfalls in einer Ersatzliste
            // aufgenommen
            // bNameOK == FALSE -> Benutzer hat abgebrochen
            bNameOK = pView->GetExchangeList( pExchangeList, pBookmarkList, 0 );

            if( bNameOK )
                bOK = pDoc->InsertBookmarkAsPage( pBookmarkList, pExchangeList,
                                    bLink, bReplace, nPos,
                                    FALSE, NULL, TRUE, TRUE );

            // Loeschen der BookmarkList
            if( pBookmarkList )
            {
                String* pString = (String*) pBookmarkList->First();
                while( pString )
                {
                    delete pString;
                    pString = (String*) pBookmarkList->Next();
                }
                delete pBookmarkList;
                pBookmarkList = NULL;
            }
            // Loeschen der ExchangeList
            if( pExchangeList )
            {
                String* pString = (String*) pExchangeList->First();
                while( pString )
                {
                    delete pString;
                    pString = (String*) pExchangeList->Next();
                }
                delete pExchangeList;
                pExchangeList = NULL;
            }
        }
        // Dann Objekte einfuegen
        //pBookmarkList = pDlg->GetList( 2 ); // Objekte
        pBookmarkList = pObjectBookmarkList;

        // Um zu gewaehrleisten... (s.o.)
        bNameOK = pView->GetExchangeList( pExchangeList, pBookmarkList, 1 );

        if( bNameOK )
            bOK = pDoc->InsertBookmarkAsObject( pBookmarkList, pExchangeList,
                                bLink, NULL, NULL);

        // Loeschen der BookmarkList
        if( pBookmarkList )
        {
            String* pString = (String*) pBookmarkList->First();
            while( pString )
            {
                delete pString;
                pString = (String*) pBookmarkList->Next();
            }
            delete pBookmarkList;
            pBookmarkList = NULL;
        }
        // Loeschen der ExchangeList
        if( pExchangeList )
        {
            String* pString = (String*) pExchangeList->First();
            while( pString )
            {
                delete pString;
                pString = (String*) pExchangeList->Next();
            }
            delete pExchangeList;
            pExchangeList = NULL;
        }

        if( pDlg->IsRemoveUnnessesaryMasterPages() )
            pDoc->RemoveUnnessesaryMasterPages();
    }

    delete pDlg;

    return (bOK);
}

/*************************************************************************
|*
|* Text oder RTF im Draw-Mode einlesen
|*
\************************************************************************/

void FuInsertFile::InsTextOrRTFinDrMode(SfxMedium* pMedium)
{
    pDocSh->SetWaitCursor( FALSE );
    SdInsertPagesObjsDlg* pDlg = new SdInsertPagesObjsDlg( NULL, pDoc,
                                                        NULL, aFile );

    USHORT nRet = pDlg->Execute();
    pDocSh->SetWaitCursor( TRUE );

    if( nRet == RET_OK )
    {
        BOOL bLink = pDlg->IsLink();

        // gewaehltes Dateiformat: Text oder RTF oder HTML (Default ist Text)
        USHORT nFormat = EE_FORMAT_TEXT;
        if (aFilterName.EqualsAscii("Rich Text Format"))
            nFormat = EE_FORMAT_RTF;
        else if (aFilterName.EqualsAscii("HTML"))
            nFormat = EE_FORMAT_HTML;

        // einen eigenen Outliner erzeugen, denn:
        // der Dokument-Outliner koennte gerade vom Gliederungsmodus
        // benutzt werden;
        // der Draw-Outliner der Drawing Engine koennte zwischendurch
        // was zeichnen muessen;
        // der globale Outliner koennte in SdPage::CreatePresObj
        // benutzt werden
        SfxItemPool* pPool = pDoc->GetDrawOutliner().GetEmptyItemSet().GetPool();
        SdrOutliner* pOutliner = new SdrOutliner( pPool, OUTLINERMODE_TEXTOBJECT );
        pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pDoc->GetStyleSheetPool());
        pOutliner->SetEditTextObjectPool(pPool);

        // Referenz-Device setzen
        SfxPrinter* pPrinter = pDocSh->GetPrinter(TRUE);
        pOutliner->SetRefDevice(pPrinter);

        SdPage* pPage = ((SdDrawViewShell*)pViewShell)->GetActualPage();
        aLayoutName = pPage->GetLayoutName();
        aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

        pOutliner->SetPaperSize(pPage->GetSize());
        pOutliner->SetMinDepth(0);

        SvStream* pStream = pMedium->GetInStream();
        DBG_ASSERT( pStream, "Kein InStream!" );

        ULONG nErr = pOutliner->Read( *pStream, nFormat, pDocSh->GetHeaderAttributes() );

        if (nErr || !pOutliner->GetEditEngine().GetText().Len())
        {
            ErrorBox aErrorBox( pWindow, (WinBits)WB_OK,
                      String(SdResId(STR_READ_DATA_ERROR)));
            aErrorBox.Execute();
        }
        else
        {
            // ist es eine Masterpage?
            if (((SdDrawViewShell*)pViewShell)->GetEditMode() == EM_MASTERPAGE &&
                !pPage->IsMasterPage())
            {
                pPage = (SdPage*)pPage->GetMasterPage(0);
            }

            DBG_ASSERT(pPage, "Seite nicht gefunden");

            // wenn gerade editiert wird, in dieses Textobjekt einfliessen lassen
            OutlinerView* pOutlinerView = pView->GetTextEditOutlinerView();
            if( pOutlinerView )
            {
                SdrObject* pObj = pView->GetTextEditObject();
                if( pObj &&
                    pObj->GetObjInventor()   == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_TITLETEXT &&
                    pOutliner->GetParagraphCount() > 1 )
                {
                    // In Titelobjekten darf nur ein Absatz vorhanden sein
                    while ( pOutliner->GetParagraphCount() > 1 )
                    {
                        Paragraph* pPara = pOutliner->GetParagraph( 0 );
                        ULONG nLen = pOutliner->GetText( pPara, 1 ).Len();
                        pOutliner->QuickDelete( ESelection( 0, nLen, 1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( 0, nLen, 0, nLen ) );
                    }
                }
            }

            OutlinerParaObject* pOPO = pOutliner->CreateParaObject();

            if (pOutlinerView)
            {
                pOutlinerView->InsertText(*pOPO);
            }
            else
            {
                SdrRectObj* pTO = new SdrRectObj(OBJ_TEXT);
                pTO->SetOutlinerParaObject(pOPO);
                Size aSize(pOutliner->CalcTextSize());

                // koennte groesser sein als die max. erlaubte Groesse:
                // falls noetig, die Objektgroesse begrenzen
                Size aMaxSize = pDoc->GetMaxObjSize();
                aSize.Height() = Min(aSize.Height(), aMaxSize.Height());
                aSize.Width()  = Min(aSize.Width(), aMaxSize.Width());

                aSize = pWindow->LogicToPixel(aSize);

                // in der Mitte des Fensters absetzen
                Size aTemp(pWindow->GetOutputSizePixel());
                Point aPos(aTemp.Width() / 2, aTemp.Height() / 2);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;
                aSize = pWindow->PixelToLogic(aSize);
                aPos = pWindow->PixelToLogic(aPos);
                pTO->SetLogicRect(Rectangle(aPos, aSize));

                if (pDlg->IsLink())
                {
                    pTO->SetTextLink(aFile, aFilterName, gsl_getSystemTextEncoding() );
                }

                pView->BegUndo(String(SdResId(STR_UNDO_INSERT_TEXTFRAME)));
                pPage->InsertObject(pTO);
                pView->AddUndo(new SdrUndoInsertObj(*pTO));
                pView->EndUndo();
            }
        }
        delete pOutliner;
    }

    delete pDlg;
}

/*************************************************************************
|*
|* Text oder RTF im Outline-Mode einlesen
|*
\************************************************************************/

void FuInsertFile::InsTextOrRTFinOlMode(SfxMedium* pMedium)
{
    // gewaehltes Dateiformat: Text oder RTF oder HTML (Default ist Text)
    USHORT nFormat = EE_FORMAT_TEXT;
    if( aFilterName.EqualsAscii("Rich Text Format"))
        nFormat = EE_FORMAT_RTF;
    else if (aFilterName.EqualsAscii("HTML"))
        nFormat = EE_FORMAT_HTML;

    Outliner*      pDocliner = ((SdOutlineView*)pView)->GetOutliner();
    List*          pList     = pDocliner->GetView(0)->CreateSelectionList();
    Paragraph*     pPara     = (Paragraph*)pList->First();

    // wo soll eingefuegt werden?
    while ( pDocliner->GetDepth( pDocliner->GetAbsPos( pPara ) ) != 0)
    {
        pPara = pDocliner->GetParent(pPara);
    }
    ULONG nTargetPos = pDocliner->GetAbsPos(pPara) + 1;

    // Layout der Vorgaengerseite uebernehmen
    USHORT nPage = 0;
    pPara = pDocliner->GetParagraph( pDocliner->GetAbsPos( pPara ) - 1 );
    while (pPara)
    {
        ULONG nPos = pDocliner->GetAbsPos( pPara );
        if ( pDocliner->GetDepth( nPos ) == 0 )
            nPage++;
        pPara = pDocliner->GetParagraph( nPos - 1 );
    }
    SdPage* pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
    aLayoutName = pPage->GetLayoutName();
    aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

    // einen eigenen Outliner erzeugen, denn:
    // der Dokument-Outliner koennte gerade vom Gliederungsmodus
    // benutzt werden;
    // der Draw-Outliner der Drawing Engine koennte zwischendurch
    // was zeichnen muessen;
    // der globale Outliner koennte in SdPage::CreatePresObj
    // benutzt werden
    Outliner* pOutliner = new Outliner( &pDoc->GetItemPool(), OUTLINERMODE_OUTLINEOBJECT );
    pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pDoc->GetStyleSheetPool());

    // Referenz-Device setzen
    SfxPrinter* pPrinter = pDocSh->GetPrinter(TRUE);
    pOutliner->SetRefDevice(pPrinter);
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "Kein InStream!" );

    ULONG nErr = pOutliner->Read(*pStream, nFormat, pDocSh->GetHeaderAttributes());

    if (nErr || !pOutliner->GetEditEngine().GetText().Len())
    {
        ErrorBox aErrorBox( pWindow, (WinBits)WB_OK,
                  String(SdResId(STR_READ_DATA_ERROR)));
        aErrorBox.Execute();
    }
    else
    {
        ULONG nParaCount = pOutliner->GetParagraphCount();

        // fuer Fortschrittsanzeige: Anzahl der Ebene-0-Absaetze
        USHORT nNewPages = 0;
        pPara = pOutliner->GetParagraph( 0 );
        while (pPara)
        {
            ULONG nPos = pOutliner->GetAbsPos( pPara );
            if( pOutliner->GetDepth( nPos ) == 0 )
                nNewPages++;
            pPara = pOutliner->GetParagraph( ++nPos );
        }

        pDocSh->SetWaitCursor( FALSE );

        SfxProgress* pProgress = new SfxProgress( pDocSh, String( SdResId(STR_CREATE_PAGES)), nNewPages);
        if( pProgress )
            pProgress->SetState( 0, 100 );

        nNewPages = 0;

        pDocliner->GetUndoManager().EnterListAction(
                                    String(SdResId(STR_UNDO_INSERT_FILE)), String() );

        ULONG nSourcePos = 0;
        SfxStyleSheet* pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
        Paragraph* pSourcePara = pOutliner->GetParagraph( 0 );
        while (pSourcePara)
        {
            ULONG nPos = pOutliner->GetAbsPos( pSourcePara );
            USHORT nDepth = pOutliner->GetDepth( nPos );

            // den letzte Absatz nur uebernehmen, wenn er gefuellt ist
            if (nSourcePos < nParaCount - 1 ||
                pOutliner->GetText(pSourcePara).Len() > 0)
            {
                pDocliner->Insert( pOutliner->GetText(pSourcePara), nTargetPos, nDepth );
                String aStyleSheetName( pStyleSheet->GetName() );
                aStyleSheetName.Erase( aStyleSheetName.Len()-1, 1 );
                aStyleSheetName += String::CreateFromInt32( nDepth );
                SfxStyleSheetBasePool* pStylePool = pDoc->GetStyleSheetPool();
                SfxStyleSheet* pOutlStyle = (SfxStyleSheet*) pStylePool->Find( aStyleSheetName, pStyleSheet->GetFamily() );
                pDocliner->SetStyleSheet( nTargetPos, pOutlStyle );
            }

            if ( nDepth == 0 )
            {
                nNewPages++;
                if( pProgress )
                    pProgress->SetState( nNewPages );
            }

            pSourcePara = pOutliner->GetParagraph( ++nPos );
            nTargetPos++;
            nSourcePos++;
        }

        pDocliner->GetUndoManager().LeaveListAction();

        if( pProgress )
            delete pProgress;

        pDocSh->SetWaitCursor( TRUE );
    }

    delete pOutliner;
}

/*************************************************************************
|*
|* SDD im Outline-Mode einlesen
|*
\************************************************************************/

void FuInsertFile::InsSDDinOlMode(SfxMedium* pMedium)
{
    SdOutlineView* pOlView = (SdOutlineView*)pView;

    // Outliner-Inhalte ins SdDrawDocument uebertragen
    pOlView->PrepareClose();

    // einlesen wie im Zeichenmodus
    if (InsSDDinDrMode(pMedium))
    {
        Outliner* pOutliner = pOlView->GetViewByWindow(pWindow)->GetOutliner();

        // Benachrichtigungs-Links temporaer trennen
        Link aOldParagraphInsertedHdl = pOutliner->GetParaInsertedHdl();
        pOutliner->SetParaInsertedHdl( Link(NULL, NULL));
        Link aOldParagraphRemovingHdl = pOutliner->GetParaRemovingHdl();
        pOutliner->SetParaRemovingHdl( Link(NULL, NULL));
        Link aOldDepthChangedHdl      = pOutliner->GetDepthChangedHdl();
        pOutliner->SetDepthChangedHdl( Link(NULL, NULL));
        Link aOldBeginMovingHdl       = pOutliner->GetBeginMovingHdl();
        pOutliner->SetBeginMovingHdl( Link(NULL, NULL));
        Link aOldEndMovingHdl         = pOutliner->GetEndMovingHdl();
        pOutliner->SetEndMovingHdl( Link(NULL, NULL));

        Link aOldStatusEventHdl       = pOutliner->GetStatusEventHdl();
        pOutliner->SetStatusEventHdl(Link(NULL, NULL));

        pOutliner->Clear();
        pOlView->FillOutliner();

        // Links wieder setzen
        pOutliner->SetParaInsertedHdl(aOldParagraphInsertedHdl);
        pOutliner->SetParaRemovingHdl(aOldParagraphRemovingHdl);
        pOutliner->SetDepthChangedHdl(aOldDepthChangedHdl);
        pOutliner->SetBeginMovingHdl(aOldBeginMovingHdl);
        pOutliner->SetEndMovingHdl(aOldEndMovingHdl);
        pOutliner->SetStatusEventHdl(aOldStatusEventHdl);
    }
}


