/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "fuinsfil.hxx"
#include <vcl/wrkwin.hxx>
#include <sfx2/progress.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sot/formats.hxx>
#include <svl/urihelper.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialogs.hrc>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "View.hxx"
#include "strings.hrc"
#include "stlpool.hxx"
#include "glob.hrc"
#include "sdpage.hxx"
#include "strmname.h"
#include "strings.hrc"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "app.hrc"
#include "unmovss.hxx"
#include "Outliner.hxx"
#include "sdabstdlg.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuInsertFile, FuPoor );

#define POOL_BUFFER_SIZE        (sal_uInt16)32768
#define BASIC_BUFFER_SIZE       (sal_uInt16)8192
#define DOCUMENT_BUFFER_SIZE    (sal_uInt16)32768

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
}

FunctionReference FuInsertFile::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuInsertFile( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertFile::DoExecute( SfxRequest& rReq )
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
        String aOwnCont;
        String aOtherCont;
        const SfxFilter*            pFilter = NULL;

        aFileDialog.SetTitle( String( SdResId(STR_DLG_INSERT_PAGES_FROM_FILE ) ) );

        if( mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
        {
            aOwnCont = String( RTL_CONSTASCII_USTRINGPARAM( "simpress" ) );
            aOtherCont = String( RTL_CONSTASCII_USTRINGPARAM( "sdraw" ) ) ;
        }
        else
        {
            aOtherCont = String( RTL_CONSTASCII_USTRINGPARAM( "simpress" ) );
            aOwnCont = String( RTL_CONSTASCII_USTRINGPARAM( "sdraw" ) ) ;
        }

        SfxFilterMatcher aMatch( aOwnCont );

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
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aOwnCont );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // get cross filter
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aOtherCont );
                if( pFilter )
                {
                    pFilter = aMatch.GetFilter4Extension( pFilter->GetDefaultExtension() );
                    if ( pFilter )
                        xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );
                }

                // get femplate filter
                if( mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    pFilter = DrawDocShell::Factory().GetTemplateFilter();
                else
                    pFilter = GraphicDocShell::Factory().GetTemplateFilter();
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // get Powerpoint filter
                aExt = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".ppt" ) );
                pFilter = aMatch.GetFilter4Extension( aExt );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                // Get other draw/impress filters
                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_60, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_60, SFX_FILTER_TEMPLATEPATH );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_60, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_60, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARIMPRESS_50, SFX_FILTER_TEMPLATEPATH );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_50, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW_40, SFX_FILTER_TEMPLATEPATH  );
                if( pFilter )
                    xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                pFilter = aMatch.GetFilter4ClipBoardId( SOT_FORMATSTR_ID_STARDRAW, SFX_FILTER_IMPORT, SFX_FILTER_TEMPLATEPATH  );
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
        }
    }
    else
    {
        SFX_REQUEST_ARG (rReq, pFileName, SfxStringItem, ID_VAL_DUMMY0, sal_False);
        SFX_REQUEST_ARG (rReq, pFilterName, SfxStringItem, ID_VAL_DUMMY1, sal_False);

        aFile = pFileName->GetValue ();

        if( pFilterName )
            aFilterName = pFilterName->GetValue ();
    }

    mpDocSh->SetWaitCursor( sal_True );

    SfxMedium*          pMedium = new SfxMedium( aFile, STREAM_READ | STREAM_NOCREATE, sal_False );
    const SfxFilter*    pFilter = NULL;

    SFX_APP()->GetFilterMatcher().GuessFilter( *pMedium, &pFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

    sal_Bool                bDrawMode = mpViewShell && mpViewShell->ISA(DrawViewShell);
    sal_Bool                bInserted = sal_False;

    if( pFilter )
    {
        pMedium->SetFilter( pFilter );
        aFilterName = pFilter->GetFilterName();

        if( pMedium->IsStorage() || ( pMedium->GetInStream() && SotStorage::IsStorageFile( pMedium->GetInStream() ) ) )
        {
            if ( pFilter->GetServiceName().EqualsAscii( "com.sun.star.presentation.PresentationDocument" ) ||
                 pFilter->GetServiceName().EqualsAscii( "com.sun.star.drawing.DrawingDocument" ) )
            {
                // Draw, Impress or PowerPoint document
                // the ownership of the Medium is transferred
                if( bDrawMode )
                    InsSDDinDrMode( pMedium );
                else
                    InsSDDinOlMode( pMedium );

                // don't delete Medium here, ownership of pMedium has changed in this case
                bInserted = sal_True;
            }
        }
        else
        {
            sal_Bool bFound = ( ::std::find( aFilterVector.begin(), aFilterVector.end(), pFilter->GetMimeType() ) != aFilterVector.end() );
            if( !bFound &&
                ( aFilterName.SearchAscii( "Text" ) != STRING_NOTFOUND ||
                aFilterName.SearchAscii( "Rich" ) != STRING_NOTFOUND ||
                aFilterName.SearchAscii( "RTF" )  != STRING_NOTFOUND ||
                aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND ) )
            {
                bFound = sal_True;
            }

            if( bFound )
            {
                if( bDrawMode )
                    InsTextOrRTFinDrMode(pMedium);
                else
                    InsTextOrRTFinOlMode(pMedium);

                bInserted = sal_True;
                delete pMedium;
            }
        }
    }

    mpDocSh->SetWaitCursor( sal_False );

    if( !bInserted )
    {
        ErrorBox aErrorBox( mpWindow, WB_OK, String( SdResId( STR_READ_DATA_ERROR ) ) );
        aErrorBox.Execute();
        delete pMedium;
    }
}

// -----------------------------------------------------------------------------

sal_Bool FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    sal_Bool bOK = sal_False;

    // Liste mit Seitennamen (wenn NULL, dann alle Seiten)
    List* pBookmarkList = NULL;

    mpDocSh->SetWaitCursor( sal_False );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdInsertPagesObjsDlg* pDlg = pFact ? pFact->CreateSdInsertPagesObjsDlg( NULL, mpDoc, pMedium, aFile ) : 0;

    if( !pDlg )
        return sal_False;

    // Ev. wird eine QueryBox geoeffnet ("Links aktualisieren?"),
    // daher wird der Dialog der aktuelle DefModalDialogParent
    ::Window* pDefParent = GetpApp()->GetDefDialogParent();
    GetpApp()->SetDefDialogParent(pDlg->GetWindow());

    sal_uInt16 nRet = pDlg->Execute();

    GetpApp()->SetDefDialogParent(pDefParent);

    mpDocSh->SetWaitCursor( sal_True );

    if( nRet == RET_OK )
    {
        // Zuerst Seiten einfuegen
        pBookmarkList = pDlg->GetList( 1 ); // Seiten
        sal_Bool bLink = pDlg->IsLink();
        sal_Bool bReplace = sal_False;
        SdPage* pPage = NULL;
        ::sd::View* pView = mpViewShell->GetView();

        if (pView->ISA(OutlineView))
        {
            pPage = static_cast<OutlineView*>(pView)->GetActualPage();
        }
        else
        {
            pPage = static_cast<SdPage*>(pView->GetSdrPageView()->GetPage());
        }

        sal_uInt16 nPos = 0xFFFF;

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

        sal_Bool  bNameOK;
        List* pObjectBookmarkList = pDlg->GetList( 2 ); // Objekte
        List* pExchangeList = NULL;

        // Es werden ausgewaehlte Seiten und/oder ausgewaehlte Objekte oder
        // alles eingefuegt, wenn pBookmarkList NULL ist!
        if( pBookmarkList || !pObjectBookmarkList )
        {
            // Um zu gewaehrleisten, dass alle Seitennamen eindeutig sind, werden
            // die einzufuegenden geprueft und gegebenenfalls in einer Ersatzliste
            // aufgenommen
            // bNameOK == sal_False -> Benutzer hat abgebrochen
            bNameOK = mpView->GetExchangeList( pExchangeList, pBookmarkList, 0 );

            if( bNameOK )
                bOK = mpDoc->InsertBookmarkAsPage( pBookmarkList, pExchangeList,
                                    bLink, bReplace, nPos,
                                    sal_False, NULL, sal_True, sal_True, sal_False );

            // delete the BookmarkList
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
            // delete the ExchangeList
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
        pBookmarkList = pObjectBookmarkList;

        // Um zu gewaehrleisten... (s.o.)
        bNameOK = mpView->GetExchangeList( pExchangeList, pBookmarkList, 1 );

        if( bNameOK )
            bOK = mpDoc->InsertBookmarkAsObject( pBookmarkList, pExchangeList,
                                bLink, NULL, NULL);

        // delete the BookmarkList
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
        // delete the ExchangeList
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
            mpDoc->RemoveUnnecessaryMasterPages();
    }

    delete pDlg;

    return (bOK);
}

// -----------------------------------------------------------------------------

void FuInsertFile::InsTextOrRTFinDrMode(SfxMedium* pMedium)
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdInsertPagesObjsDlg* pDlg = pFact ? pFact->CreateSdInsertPagesObjsDlg(NULL, mpDoc, NULL, aFile ) : 0;
    if( !pDlg )
        return;

    mpDocSh->SetWaitCursor( sal_False );

    sal_uInt16 nRet = pDlg->Execute();
    mpDocSh->SetWaitCursor( sal_True );

    if( nRet == RET_OK )
    {
        // gewaehltes Dateiformat: Text oder RTF oder HTML (Default ist Text)
        sal_uInt16 nFormat = EE_FORMAT_TEXT;

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
        SdrOutliner* pOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );

        // Referenz-Device setzen
        pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
        aLayoutName = pPage->GetLayoutName();
        aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

        pOutliner->SetPaperSize(pPage->GetSize());

        SvStream* pStream = pMedium->GetInStream();
        DBG_ASSERT( pStream, "Kein InStream!" );
        pStream->Seek( 0 );

        sal_uLong nErr = pOutliner->Read( *pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes() );

        if (nErr || !pOutliner->GetEditEngine().GetText().Len())
        {
            ErrorBox aErrorBox( mpWindow, (WinBits)WB_OK,
                      String(SdResId(STR_READ_DATA_ERROR)));
            aErrorBox.Execute();
        }
        else
        {
            // ist es eine Masterpage?
            if (static_cast<DrawViewShell*>(mpViewShell)->GetEditMode() == EM_MASTERPAGE &&
                !pPage->IsMasterPage())
            {
                pPage = (SdPage*)(&(pPage->TRG_GetMasterPage()));
            }

            DBG_ASSERT(pPage, "Seite nicht gefunden");

            // wenn gerade editiert wird, in dieses Textobjekt einfliessen lassen
            OutlinerView* pOutlinerView = mpView->GetTextEditOutlinerView();
            if( pOutlinerView )
            {
                SdrObject* pObj = mpView->GetTextEditObject();
                if( pObj &&
                    pObj->GetObjInventor()   == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_TITLETEXT &&
                    pOutliner->GetParagraphCount() > 1 )
                {
                    // In Titelobjekten darf nur ein Absatz vorhanden sein
                    while ( pOutliner->GetParagraphCount() > 1 )
                    {
                        Paragraph* pPara = pOutliner->GetParagraph( 0 );
                        sal_uLong nLen = pOutliner->GetText( pPara, 1 ).Len();
                        pOutliner->QuickDelete( ESelection( 0, (sal_uInt16) nLen, 1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( 0, (sal_uInt16) nLen, 0, (sal_uInt16) nLen ) );
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

                const bool bUndo = mpView->IsUndoEnabled();
                if( bUndo )
                    mpView->BegUndo(String(SdResId(STR_UNDO_INSERT_TEXTFRAME)));
                pPage->InsertObject(pTO);

                // koennte groesser sein als die max. erlaubte Groesse:
                // falls noetig, die Objektgroesse begrenzen
                Size aSize(pOutliner->CalcTextSize());
                Size aMaxSize = mpDoc->GetMaxObjSize();
                aSize.Height() = Min(aSize.Height(), aMaxSize.Height());
                aSize.Width()  = Min(aSize.Width(), aMaxSize.Width());
                aSize = mpWindow->LogicToPixel(aSize);

                // in der Mitte des Fensters absetzen
                Size aTemp(mpWindow->GetOutputSizePixel());
                Point aPos(aTemp.Width() / 2, aTemp.Height() / 2);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;
                aSize = mpWindow->PixelToLogic(aSize);
                aPos = mpWindow->PixelToLogic(aPos);
                pTO->SetLogicRect(Rectangle(aPos, aSize));

                if (pDlg->IsLink())
                {
                    pTO->SetTextLink(aFile, aFilterName, gsl_getSystemTextEncoding() );
                }

                if( bUndo )
                {
                    mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoInsertObject(*pTO));
                    mpView->EndUndo();
                }
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
    sal_uInt16 nFormat = EE_FORMAT_TEXT;

    if( aFilterName.SearchAscii( "Rich") != STRING_NOTFOUND )
        nFormat = EE_FORMAT_RTF;
    else if( aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND )
        nFormat = EE_FORMAT_HTML;

    ::Outliner*    pDocliner = static_cast<OutlineView*>(mpView)->GetOutliner();

    std::vector<Paragraph*> aSelList;
    pDocliner->GetView(0)->CreateSelectionList(aSelList);

    Paragraph* pPara = aSelList.empty() ? NULL : *(aSelList.begin());

    // wo soll eingefuegt werden?
    while( !pDocliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
        pPara = pDocliner->GetParent(pPara);

    sal_uLong nTargetPos = pDocliner->GetAbsPos(pPara) + 1;

    // Layout der Vorgaengerseite uebernehmen
    sal_uInt16 nPage = 0;
    pPara = pDocliner->GetParagraph( pDocliner->GetAbsPos( pPara ) - 1 );
    while (pPara)
    {
        sal_uLong nPos = pDocliner->GetAbsPos( pPara );
        if ( pDocliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
            nPage++;
        pPara = pDocliner->GetParagraph( nPos - 1 );
    }
    SdPage* pPage = mpDoc->GetSdPage(nPage, PK_STANDARD);
    aLayoutName = pPage->GetLayoutName();
    aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

    // einen eigenen Outliner erzeugen, denn:
    // der Dokument-Outliner koennte gerade vom Gliederungsmodus
    // benutzt werden;
    // der Draw-Outliner der Drawing Engine koennte zwischendurch
    // was zeichnen muessen;
    // der globale Outliner koennte in SdPage::CreatePresObj
    // benutzt werden
    ::Outliner* pOutliner = new ::Outliner( &mpDoc->GetItemPool(), OUTLINERMODE_OUTLINEOBJECT );
    pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)mpDoc->GetStyleSheetPool());

    // Referenz-Device setzen
    pOutliner->SetRefDevice(SD_MOD()->GetRefDevice( *mpDocSh ));
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "Kein InStream!" );
    pStream->Seek( 0 );

    sal_uLong nErr = pOutliner->Read(*pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes());

    if (nErr || !pOutliner->GetEditEngine().GetText().Len())
    {
        ErrorBox aErrorBox( mpWindow, (WinBits)WB_OK,
                  String(SdResId(STR_READ_DATA_ERROR)));
        aErrorBox.Execute();
    }
    else
    {
        sal_uLong nParaCount = pOutliner->GetParagraphCount();

        // fuer Fortschrittsanzeige: Anzahl der Ebene-0-Absaetze
        sal_uInt16 nNewPages = 0;
        pPara = pOutliner->GetParagraph( 0 );
        while (pPara)
        {
            sal_uLong nPos = pOutliner->GetAbsPos( pPara );
            if( pOutliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
                nNewPages++;
            pPara = pOutliner->GetParagraph( ++nPos );
        }

        mpDocSh->SetWaitCursor( sal_False );

        SfxProgress* pProgress = new SfxProgress( mpDocSh, String( SdResId(STR_CREATE_PAGES)), nNewPages);
        if( pProgress )
            pProgress->SetState( 0, 100 );

        nNewPages = 0;

        pDocliner->GetUndoManager().EnterListAction(
                                    String(SdResId(STR_UNDO_INSERT_FILE)), String() );

        sal_uLong nSourcePos = 0;
        SfxStyleSheet* pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
        Paragraph* pSourcePara = pOutliner->GetParagraph( 0 );
        while (pSourcePara)
        {
            sal_uLong nPos = pOutliner->GetAbsPos( pSourcePara );
            sal_Int16 nDepth = pOutliner->GetDepth( (sal_uInt16) nPos );

            // den letzte Absatz nur uebernehmen, wenn er gefuellt ist
            if (nSourcePos < nParaCount - 1 ||
                pOutliner->GetText(pSourcePara).Len() > 0)
            {
                pDocliner->Insert( pOutliner->GetText(pSourcePara), nTargetPos, nDepth );
                String aStyleSheetName( pStyleSheet->GetName() );
                aStyleSheetName.Erase( aStyleSheetName.Len()-1, 1 );
                aStyleSheetName += String::CreateFromInt32( nDepth <= 0 ? 1 : nDepth+1 );
                SfxStyleSheetBasePool* pStylePool = mpDoc->GetStyleSheetPool();
                SfxStyleSheet* pOutlStyle = (SfxStyleSheet*) pStylePool->Find( aStyleSheetName, pStyleSheet->GetFamily() );
                pDocliner->SetStyleSheet( nTargetPos, pOutlStyle );
            }

            if( pDocliner->HasParaFlag( pSourcePara, PARAFLAG_ISPAGE ) )
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

        mpDocSh->SetWaitCursor( sal_True );
    }

    delete pOutliner;
}

// -----------------------------------------------------------------------------

sal_Bool FuInsertFile::InsSDDinOlMode(SfxMedium* pMedium)
{
    OutlineView* pOlView = static_cast<OutlineView*>(mpView);

    // Outliner-Inhalte ins SdDrawDocument uebertragen
    pOlView->PrepareClose();

    // einlesen wie im Zeichenmodus
    if (InsSDDinDrMode(pMedium))
    {
        ::Outliner* pOutliner = pOlView->GetViewByWindow(mpWindow)->GetOutliner();

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

        return sal_True;
    }
    else
        return sal_False;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
