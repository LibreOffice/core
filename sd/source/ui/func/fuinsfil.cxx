/*************************************************************************
 *
 *  $RCSfile: fuinsfil.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:02:59 $
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

#include "fuinsfil.hxx"

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
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#include <svx/dialogs.hrc>

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif

#include "sdresid.hxx"
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "strings.hrc"
#include "stlpool.hxx"
#include "glob.hrc"
#include "sdpage.hxx"
#include "strmname.h"
#include "strings.hrc"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "DrawDocShell.hxx"
#include "app.hrc"
#include "unmovss.hxx"
//CHINA001 #include "inspagob.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#include "sdabstdlg.hxx" //CHINA001
#include "inspagob.hrc" //CHINA001
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace sd {

TYPEINIT1( FuInsertFile, FuPoor );

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuInsertFile::FuInsertFile (
    ViewShell*    pViewSh,
    ::sd::Window*      pWin,
    ::sd::View*        pView,
    SdDrawDocument* pDoc,
    SfxRequest&    rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SfxFilterMatcher&       rMatcher = SFX_APP()->GetFilterMatcher();
    ::std::vector< String > aFilterVector;
    const SfxItemSet*       pArgs = rReq.GetArgs ();

    FuInsertFile::GetSupportedFilterVector( aFilterVector );

    if (!pArgs)
    {
        sfx2::FileDialogHelper      aFileDialog( WB_OPEN | SFXWB_INSERT | WB_STDMODAL );
        Reference< XFilePicker >    xFilePicker( aFileDialog.GetFilePicker(), UNO_QUERY );
        Reference< XFilterManager > xFilterManager( xFilePicker, UNO_QUERY );
        String aCont;
        const SfxFilter*            pFilter = NULL;

        aFileDialog.SetTitle( String( SdResId(STR_DLG_INSERT_PAGES_FROM_FILE ) ) );

        if( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
            aCont = String( RTL_CONSTASCII_USTRINGPARAM( "simpress" ) );
        else
            aCont = String( RTL_CONSTASCII_USTRINGPARAM( "sdraw" ) ) ;
        SfxFilterMatcher aMatch( aCont );

        if( xFilterManager.is() )
        {
            // Get filter for current format
            try
            {
                String  aExt;
                String  aAllSpec( SdResId( STR_ALL_FILES ) );

                xFilterManager->appendFilter( aAllSpec, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.*" ) ) );
                xFilterManager->setCurrentFilter( aAllSpec ); // set default-filter (<All>)

                // Get main filter
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aCont );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // Get Draw filter for Impress and Impress filter for Draw as secondary filter
                if( pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    aExt = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".sxd" ) );
                else
                    aExt = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".sxi" ) );

                pFilter = aMatch.GetFilter4Extension( aExt );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // Get other filters
                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50 );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50, SFX_FILTER_TEMPLATEPATH );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50 );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40 );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // add additional supported filters
                ::std::vector< String >::const_iterator aIter( aFilterVector.begin() );

                while( aIter != aFilterVector.end() )
                {
                    if( ( pFilter = rMatcher.GetFilter4Mime( *aIter ) ) != NULL )
                        xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                    ++aIter;
                }
            }
            catch(IllegalArgumentException)
            {
            }
        }

        if( aFileDialog.Execute() != ERRCODE_NONE )
            return;
        else
        {
            aFilterName = aFileDialog.GetCurrentFilter();
            aFile = aFileDialog.GetPath();
            INetURLObject::SetBaseURL( aFile );
            aFile = ::URIHelper::SmartRelToAbs( aFile, FALSE,
                                                INetURLObject::WAS_ENCODED,
                                                INetURLObject::DECODE_UNAMBIGUOUS );
        }
    }
    else
    {
        SFX_REQUEST_ARG (rReq, pFileName, SfxStringItem, ID_VAL_DUMMY0, FALSE);
        SFX_REQUEST_ARG (rReq, pFilterName, SfxStringItem, ID_VAL_DUMMY1, FALSE);

        aFile = pFileName->GetValue ();

        if( pFilterName )
            aFilterName = pFilterName->GetValue ();
        else
            aFilterName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.sdd" ) );
    }

    pDocSh->SetWaitCursor( TRUE );

    SfxMedium*          pMedium = new SfxMedium( aFile, STREAM_READ | STREAM_NOCREATE, FALSE );
    const SfxFilter*    pFilter = NULL;
    ErrCode             nErr = SFX_APP()->GetFilterMatcher().GuessFilter( *pMedium, &pFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );
    BOOL                bDrawMode = pViewSh->ISA(DrawViewShell);
    BOOL                bInserted = FALSE;

    if( pFilter )
    {
        pMedium->SetFilter( pFilter );
        aFilterName = pFilter->GetFilterName();
    }

    if( pMedium->IsStorage() )
    {
        // Storage
        SvStorage* pStorage = pMedium->GetStorage();

        if( pStorage )
        {
            BOOL bOwnDocument = FALSE;

            if( pStorage->IsStream( pStarDrawDoc ) || pStorage->IsStream( pStarDrawDoc3 ) )
            {
                // binary document
                bOwnDocument = TRUE;
            }
            else if( ( pStorage->IsStream( pStarDrawXMLContent ) || pStorage->IsStream( pStarDrawOldXMLContent ) ) &&
                     ( !pFilter ||
                       ( aFilterName.SearchAscii( "StarOffice XML (Draw)" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "StarOffice XML (Impress)" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "draw_StarOffice_XML_Impress" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "impress_StarOffice_XML_Draw" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "impress_StarOffice_XML_Impress_Template" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "draw_StarOffice_XML_Draw_Template" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "draw_StarOffice_XML_Draw" ) != STRING_NOTFOUND ||
                         aFilterName.SearchAscii( "impress_StarOffice_XML_Impress" ) != STRING_NOTFOUND ) ) )
            {
                // XML document
                bOwnDocument = TRUE;
            }

            if( bOwnDocument )
            {
                pMedium->Close();

                if( bDrawMode )
                    InsSDDinDrMode( pMedium );
                else
                    InsSDDinOlMode( pMedium );

                bInserted = TRUE;
            }
        }
    }
    else if( pFilter )
    {
        BOOL bFound = ( ::std::find( aFilterVector.begin(), aFilterVector.end(), pFilter->GetMimeType() ) != aFilterVector.end() );

        if( !bFound &&
            ( aFilterName.SearchAscii( "Text" ) != STRING_NOTFOUND ||
              aFilterName.SearchAscii( "Rich" ) != STRING_NOTFOUND ||
              aFilterName.SearchAscii( "RTF" )  != STRING_NOTFOUND ||
              aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND ) )
        {
            bFound = TRUE;
        }

        if( bFound )
        {
            if( bDrawMode )
                InsTextOrRTFinDrMode(pMedium);
            else
                InsTextOrRTFinOlMode(pMedium);

            bInserted = TRUE;
        }
    }

    delete pMedium;
    pDocSh->SetWaitCursor( FALSE );

    if( !bInserted )
    {
        ErrorBox aErrorBox( pWindow, WB_OK, String( SdResId( STR_READ_DATA_ERROR ) ) );
        aErrorBox.Execute();
    }
}

// -----------------------------------------------------------------------------

FuInsertFile::~FuInsertFile()
{
}

// -----------------------------------------------------------------------------

void FuInsertFile::Activate()
{
    FuPoor::Activate();
}

// -----------------------------------------------------------------------------

void FuInsertFile::Deactivate()
{
    FuPoor::Deactivate();
}

// -----------------------------------------------------------------------------

BOOL FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    BOOL bOK = FALSE;

    // Liste mit Seitennamen (wenn NULL, dann alle Seiten)
    List* pBookmarkList = NULL;

    pDocSh->SetWaitCursor( FALSE );
//CHINA001  SdInsertPagesObjsDlg* pDlg = new SdInsertPagesObjsDlg( NULL, pDoc,
//CHINA001  pMedium, aFile );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
    AbstractSdInsertPagesObjsDlg* pDlg = pFact->CreateSdInsertPagesObjsDlg(ResId( DLG_INSERT_PAGES_OBJS ), NULL, pDoc,
                                                    pMedium, aFile );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    // Ev. wird eine QueryBox geoeffnet ("Links aktualisieren?"),
    // daher wird der Dialog der aktuelle DefModalDialogParent
    ::Window* pDefParent = GetpApp()->GetDefDialogParent();
    GetpApp()->SetDefDialogParent(pDlg->GetWindow()); //CHINA001 GetpApp()->SetDefDialogParent(pDlg);

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
        ::sd::View* pView = pViewShell->GetView();

        if (pView->ISA(OutlineView))
        {
            pPage = static_cast<OutlineView*>(pView)->GetActualPage();
        }
        else
        {
            pPage = static_cast<SdPage*>(pView->GetPageViewPvNum(0)->GetPage());
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
                                    FALSE, NULL, TRUE, TRUE, FALSE );

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

// -----------------------------------------------------------------------------

void FuInsertFile::InsTextOrRTFinDrMode(SfxMedium* pMedium)
{
    pDocSh->SetWaitCursor( FALSE );
//CHINA001  SdInsertPagesObjsDlg* pDlg = new SdInsertPagesObjsDlg( NULL, pDoc,
//CHINA001  NULL, aFile );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
    AbstractSdInsertPagesObjsDlg* pDlg = pFact->CreateSdInsertPagesObjsDlg(ResId( DLG_INSERT_PAGES_OBJS ), NULL, pDoc,
                                                        NULL, aFile );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    USHORT nRet = pDlg->Execute();
    pDocSh->SetWaitCursor( TRUE );

    if( nRet == RET_OK )
    {
        BOOL bLink = pDlg->IsLink();

        // gewaehltes Dateiformat: Text oder RTF oder HTML (Default ist Text)
        USHORT nFormat = EE_FORMAT_TEXT;

        if( aFilterName.SearchAscii( "Rich") != STRING_NOTFOUND )
            nFormat = EE_FORMAT_RTF;
        else if( aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND )
            nFormat = EE_FORMAT_HTML;

        // einen eigenen Outliner erzeugen, denn:
        // der Dokument-Outliner koennte gerade vom Gliederungsmodus
        // benutzt werden;
        // der Draw-Outliner der Drawing Engine koennte zwischendurch
        // was zeichnen muessen;
        // der globale Outliner koennte in SdPage::CreatePresObj
        // benutzt werden
//      SfxItemPool* pPool = pDoc->GetDrawOutliner().GetEmptyItemSet().GetPool();
        SdrOutliner* pOutliner = new ::sd::Outliner( pDoc, OUTLINERMODE_TEXTOBJECT );
//      pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pDoc->GetStyleSheetPool());
//      pOutliner->SetEditTextObjectPool(pPool);
//      pOutliner->SetForbiddenCharsTable( pDoc->GetForbiddenCharsTable() );

        // Referenz-Device setzen
        pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *pDocSh ) );

        SdPage* pPage = static_cast<DrawViewShell*>(pViewShell)->GetActualPage();
        aLayoutName = pPage->GetLayoutName();
        aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

        pOutliner->SetPaperSize(pPage->GetSize());
        pOutliner->SetMinDepth(0);

        SvStream* pStream = pMedium->GetInStream();
        DBG_ASSERT( pStream, "Kein InStream!" );
        pStream->Seek( 0 );

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
            if (static_cast<DrawViewShell*>(pViewShell)->GetEditMode() == EM_MASTERPAGE &&
                !pPage->IsMasterPage())
            {
                pPage = (SdPage*)(&(pPage->TRG_GetMasterPage()));
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
                        pOutliner->QuickDelete( ESelection( 0, (USHORT) nLen, 1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( 0, (USHORT) nLen, 0, (USHORT) nLen ) );
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

                pView->BegUndo(String(SdResId(STR_UNDO_INSERT_TEXTFRAME)));
                pPage->InsertObject(pTO);

                // koennte groesser sein als die max. erlaubte Groesse:
                // falls noetig, die Objektgroesse begrenzen
                Size aSize(pOutliner->CalcTextSize());
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

                pView->AddUndo(new SdrUndoInsertObj(*pTO));
                pView->EndUndo();
            }
        }
        delete pOutliner;
    }

    delete pDlg;
}

// -----------------------------------------------------------------------------

void FuInsertFile::InsTextOrRTFinOlMode(SfxMedium* pMedium)
{
    // gewaehltes Dateiformat: Text oder RTF oder HTML (Default ist Text)
    USHORT nFormat = EE_FORMAT_TEXT;

    if( aFilterName.SearchAscii( "Rich") != STRING_NOTFOUND )
        nFormat = EE_FORMAT_RTF;
    else if( aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND )
        nFormat = EE_FORMAT_HTML;

    ::Outliner*    pDocliner = static_cast<OutlineView*>(pView)->GetOutliner();
    List*          pList     = pDocliner->GetView(0)->CreateSelectionList();
    Paragraph*     pPara     = (Paragraph*)pList->First();

    // wo soll eingefuegt werden?
    while ( pDocliner->GetDepth( (USHORT) pDocliner->GetAbsPos( pPara ) ) != 0)
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
        if ( pDocliner->GetDepth( (USHORT) nPos ) == 0 )
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
    ::Outliner* pOutliner = new ::Outliner( &pDoc->GetItemPool(), OUTLINERMODE_OUTLINEOBJECT );
    pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pDoc->GetStyleSheetPool());

    // Referenz-Device setzen
    pOutliner->SetRefDevice(SD_MOD()->GetRefDevice( *pDocSh ));
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "Kein InStream!" );
    pStream->Seek( 0 );

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
            if( pOutliner->GetDepth( (USHORT) nPos ) == 0 )
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
            USHORT nDepth = pOutliner->GetDepth( (USHORT) nPos );

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

// -----------------------------------------------------------------------------

void FuInsertFile::InsSDDinOlMode(SfxMedium* pMedium)
{
    OutlineView* pOlView = static_cast<OutlineView*>(pView);

    // Outliner-Inhalte ins SdDrawDocument uebertragen
    pOlView->PrepareClose();

    // einlesen wie im Zeichenmodus
    if (InsSDDinDrMode(pMedium))
    {
        ::Outliner* pOutliner = pOlView->GetViewByWindow(pWindow)->GetOutliner();

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

// -----------------------------------------------------------------------------

void FuInsertFile::GetSupportedFilterVector( ::std::vector< String >& rFilterVector )
{
    SfxFilterMatcher&   rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter*    pSearchFilter = NULL;

    rFilterVector.clear();

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "text/plain" ) ) ) ) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "application/rtf" ) ) ) ) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "text/html" ) ) ) ) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );
}

} // end of namespace sd
