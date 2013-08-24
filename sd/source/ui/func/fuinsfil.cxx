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


#include "fuinsfil.hxx"
#include <vcl/wrkwin.hxx>
#include <sfx2/progress.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
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
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialogs.hrc>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "View.hxx"
#include "strings.hrc"
#include "stlpool.hxx"
#include "glob.hrc"
#include "sdpage.hxx"
#include "strmname.h"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "app.hrc"
#include "unmovss.hxx"
#include "Outliner.hxx"
#include "sdabstdlg.hxx"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuInsertFile, FuPoor );


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
    ::std::vector< OUString > aFilterVector;
    const SfxItemSet*       pArgs = rReq.GetArgs ();

    FuInsertFile::GetSupportedFilterVector( aFilterVector );

    if (!pArgs)
    {
        sfx2::FileDialogHelper      aFileDialog(
                ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                SFXWB_INSERT );
        Reference< XFilePicker >    xFilePicker( aFileDialog.GetFilePicker(), UNO_QUERY );
        Reference< XFilterManager > xFilterManager( xFilePicker, UNO_QUERY );
        OUString aOwnCont;
        OUString aOtherCont;
        const SfxFilter*            pFilter = NULL;

        aFileDialog.SetTitle( SD_RESSTR(STR_DLG_INSERT_PAGES_FROM_FILE) );

        if( mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
        {
            aOwnCont = "simpress";
            aOtherCont = "sdraw";
        }
        else
        {
            aOtherCont = "simpress";
            aOwnCont = "sdraw" ;
        }

        SfxFilterMatcher aMatch( aOwnCont );

        if( xFilterManager.is() )
        {
            // Get filter for current format
            try
            {
                String  aExt;
                String  aAllSpec( SdResId( STR_ALL_FILES ) );

                xFilterManager->appendFilter( aAllSpec, OUString("*.*") );
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
                aExt = OUString(".ppt");
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
                ::std::vector< OUString >::const_iterator aIter( aFilterVector.begin() );

                while( aIter != aFilterVector.end() )
                {
                    if( ( pFilter = rMatcher.GetFilter4Mime( *aIter ) ) != NULL )
                        xFilterManager->appendFilter( pFilter->GetUIName(), pFilter->GetDefaultExtension() );

                    ++aIter;
                }
            }
            catch (const IllegalArgumentException&)
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

    SfxMedium*          pMedium = new SfxMedium( aFile, STREAM_READ | STREAM_NOCREATE );
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
            if ( pFilter->GetServiceName() == "com.sun.star.presentation.PresentationDocument" ||
                 pFilter->GetServiceName() == "com.sun.star.drawing.DrawingDocument" )
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
        ErrorBox aErrorBox( mpWindow, WB_OK, SD_RESSTR( STR_READ_DATA_ERROR ) );
        aErrorBox.Execute();
        delete pMedium;
    }
}

// -----------------------------------------------------------------------------

sal_Bool FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    sal_Bool bOK = sal_False;

    mpDocSh->SetWaitCursor( sal_False );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdInsertPagesObjsDlg* pDlg = pFact ? pFact->CreateSdInsertPagesObjsDlg( NULL, mpDoc, pMedium, aFile ) : 0;

    if( !pDlg )
        return sal_False;

    /* Maybe a QueryBox is opened ("update links?"), therefore the dialog
       becomes the current DefModalDialogParent */
    ::Window* pDefParent = GetpApp()->GetDefDialogParent();
    GetpApp()->SetDefDialogParent(pDlg->GetWindow());

    sal_uInt16 nRet = pDlg->Execute();

    GetpApp()->SetDefDialogParent(pDefParent);

    mpDocSh->SetWaitCursor( sal_True );

    if( nRet == RET_OK )
    {
        /* list with page names (if NULL, then all pages)
           First, insert pages */
        std::vector<OUString> aBookmarkList = pDlg->GetList( 1 ); // pages
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
        std::vector<OUString> aExchangeList;
        std::vector<OUString> aObjectBookmarkList = pDlg->GetList( 2 ); // objects

        /* if pBookmarkList is NULL, we insert selected pages, and/or selected
           objects or everything. */
        if( !aBookmarkList.empty() || aObjectBookmarkList.empty() )
        {
            /* To ensure that all page names are unique, we check the ones we
               want to insert and insert them into a substitution list if
               necessary.
               bNameOK is sal_False if the user has canceled. */
            bNameOK = mpView->GetExchangeList( aExchangeList, aBookmarkList, 0 );

            if( bNameOK )
                bOK = mpDoc->InsertBookmarkAsPage( aBookmarkList, &aExchangeList,
                                    bLink, bReplace, nPos,
                                    sal_False, NULL, sal_True, sal_True, sal_False );

            aBookmarkList.clear();
            aExchangeList.clear();
        }

        // to ensure ... (see above)
        bNameOK = mpView->GetExchangeList( aExchangeList, aObjectBookmarkList, 1 );

        if( bNameOK )
            bOK = mpDoc->InsertBookmarkAsObject( aObjectBookmarkList, aExchangeList,
                                bLink, NULL, NULL);

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
        // selected file format: text, RTF or HTML (default is text)
        sal_uInt16 nFormat = EE_FORMAT_TEXT;

        if( aFilterName.SearchAscii( "Rich") != STRING_NOTFOUND )
            nFormat = EE_FORMAT_RTF;
        else if( aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND )
            nFormat = EE_FORMAT_HTML;

        /* create our own outline since:
           - it is possible that the document outliner is actually used in the
             structuring mode
           - the draw outliner of the drawing engine has to draw something in
             between
           - the global outliner could be used in SdPage::CreatePresObj */
        SdrOutliner* pOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );

        // set reference device
        pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
        aLayoutName = pPage->GetLayoutName();
        aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

        pOutliner->SetPaperSize(pPage->GetSize());

        SvStream* pStream = pMedium->GetInStream();
        DBG_ASSERT( pStream, "No InStream!" );
        pStream->Seek( 0 );

        sal_uLong nErr = pOutliner->Read( *pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes() );

        if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
        {
            ErrorBox aErrorBox( mpWindow, (WinBits)WB_OK,
                      SD_RESSTR(STR_READ_DATA_ERROR));
            aErrorBox.Execute();
        }
        else
        {
            // is it a master page?
            if (static_cast<DrawViewShell*>(mpViewShell)->GetEditMode() == EM_MASTERPAGE &&
                !pPage->IsMasterPage())
            {
                pPage = (SdPage*)(&(pPage->TRG_GetMasterPage()));
            }

            DBG_ASSERT(pPage, "page not found");

            // if editing is going on right now, let it flow into this text object
            OutlinerView* pOutlinerView = mpView->GetTextEditOutlinerView();
            if( pOutlinerView )
            {
                SdrObject* pObj = mpView->GetTextEditObject();
                if( pObj &&
                    pObj->GetObjInventor()   == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_TITLETEXT &&
                    pOutliner->GetParagraphCount() > 1 )
                {
                    // in title objects, only one paragraph is allowed
                    while ( pOutliner->GetParagraphCount() > 1 )
                    {
                        Paragraph* pPara = pOutliner->GetParagraph( 0 );
                        sal_uLong nLen = pOutliner->GetText( pPara, 1 ).getLength();
                        pOutliner->QuickDelete( ESelection( 0, (sal_uInt16) nLen, 1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( 0, (sal_uInt16) nLen, 0, (sal_uInt16) nLen ) );
                    }
                }
            }

            OutlinerParaObject* pOPO = pOutliner->CreateParaObject();

            if (pOutlinerView)
            {
                pOutlinerView->InsertText(*pOPO);
                delete pOPO;
            }
            else
            {
                SdrRectObj* pTO = new SdrRectObj(OBJ_TEXT);
                pTO->SetOutlinerParaObject(pOPO);

                const bool bUndo = mpView->IsUndoEnabled();
                if( bUndo )
                    mpView->BegUndo(SD_RESSTR(STR_UNDO_INSERT_TEXTFRAME));
                pPage->InsertObject(pTO);

                /* can be bigger as the maximal allowed size:
                   limit object size if necessary */
                Size aSize(pOutliner->CalcTextSize());
                Size aMaxSize = mpDoc->GetMaxObjSize();
                aSize.Height() = std::min(aSize.Height(), aMaxSize.Height());
                aSize.Width()  = std::min(aSize.Width(), aMaxSize.Width());
                aSize = mpWindow->LogicToPixel(aSize);

                // put it at the center of the window
                Size aTemp(mpWindow->GetOutputSizePixel());
                Point aPos(aTemp.Width() / 2, aTemp.Height() / 2);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;
                aSize = mpWindow->PixelToLogic(aSize);
                aPos = mpWindow->PixelToLogic(aPos);
                pTO->SetLogicRect(Rectangle(aPos, aSize));

                if (pDlg->IsLink())
                {
                    pTO->SetTextLink(aFile, aFilterName, osl_getThreadTextEncoding() );
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
    // selected file format: text, RTF or HTML (default is text)
    sal_uInt16 nFormat = EE_FORMAT_TEXT;

    if( aFilterName.SearchAscii( "Rich") != STRING_NOTFOUND )
        nFormat = EE_FORMAT_RTF;
    else if( aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND )
        nFormat = EE_FORMAT_HTML;

    ::Outliner*    pDocliner = static_cast<OutlineView*>(mpView)->GetOutliner();

    std::vector<Paragraph*> aSelList;
    pDocliner->GetView(0)->CreateSelectionList(aSelList);

    Paragraph* pPara = aSelList.empty() ? NULL : *(aSelList.begin());

    // what should we insert?
    while( !pDocliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
        pPara = pDocliner->GetParent(pPara);

    sal_Int32 nTargetPos = pDocliner->GetAbsPos(pPara) + 1;

    // apply layout of predecessor page
    sal_uInt16 nPage = 0;
    pPara = pDocliner->GetParagraph( pDocliner->GetAbsPos( pPara ) - 1 );
    while (pPara)
    {
        sal_Int32 nPos = pDocliner->GetAbsPos( pPara );
        if ( pDocliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
            nPage++;
        pPara = pDocliner->GetParagraph( nPos - 1 );
    }
    SdPage* pPage = mpDoc->GetSdPage(nPage, PK_STANDARD);
    aLayoutName = pPage->GetLayoutName();
    aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

    /* create our own outline since:
       - it is possible that the document outliner is actually used in the
         structuring mode
       - the draw outliner of the drawing engine has to draw something in
         between
       - the global outliner could be used in SdPage::CreatePresObj */
    ::Outliner* pOutliner = new ::Outliner( &mpDoc->GetItemPool(), OUTLINERMODE_OUTLINEOBJECT );
    pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)mpDoc->GetStyleSheetPool());

    // set reference device
    pOutliner->SetRefDevice(SD_MOD()->GetRefDevice( *mpDocSh ));
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "No InStream!" );
    pStream->Seek( 0 );

    sal_uLong nErr = pOutliner->Read(*pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes());

    if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
    {
        ErrorBox aErrorBox( mpWindow, (WinBits)WB_OK,
                  SD_RESSTR(STR_READ_DATA_ERROR));
        aErrorBox.Execute();
    }
    else
    {
        sal_Int32 nParaCount = pOutliner->GetParagraphCount();

        // for progress bar: number of level-0-paragraphs
        sal_uInt16 nNewPages = 0;
        pPara = pOutliner->GetParagraph( 0 );
        while (pPara)
        {
            sal_Int32 nPos = pOutliner->GetAbsPos( pPara );
            if( pOutliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
                nNewPages++;
            pPara = pOutliner->GetParagraph( ++nPos );
        }

        mpDocSh->SetWaitCursor( sal_False );

        SfxProgress* pProgress = new SfxProgress( mpDocSh, SD_RESSTR(STR_CREATE_PAGES), nNewPages);
        if( pProgress )
            pProgress->SetState( 0, 100 );

        nNewPages = 0;

        pDocliner->GetUndoManager().EnterListAction(
                                    SD_RESSTR(STR_UNDO_INSERT_FILE), String() );

        sal_Int32 nSourcePos = 0;
        SfxStyleSheet* pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
        Paragraph* pSourcePara = pOutliner->GetParagraph( 0 );
        while (pSourcePara)
        {
            sal_Int32 nPos = pOutliner->GetAbsPos( pSourcePara );
            sal_Int16 nDepth = pOutliner->GetDepth( nPos );

            // only take the last paragraph if it is filled
            if (nSourcePos < nParaCount - 1 ||
                !pOutliner->GetText(pSourcePara).isEmpty())
            {
                pDocliner->Insert( pOutliner->GetText(pSourcePara), nTargetPos, nDepth );
                String aStyleSheetName( pStyleSheet->GetName() );
                aStyleSheetName.Erase( aStyleSheetName.Len()-1, 1 );
                aStyleSheetName += OUString::number( nDepth <= 0 ? 1 : nDepth+1 );
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

    // transfer Outliner content to SdDrawDocument
    pOlView->PrepareClose();

    // einlesen wie im Zeichenmodus
    if (InsSDDinDrMode(pMedium))
    {
        ::Outliner* pOutliner = pOlView->GetViewByWindow(mpWindow)->GetOutliner();

        // cut notification links temporarily
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

        // set links again
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

void FuInsertFile::GetSupportedFilterVector( ::std::vector< OUString >& rFilterVector )
{
    SfxFilterMatcher&   rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter*    pSearchFilter = NULL;

    rFilterVector.clear();

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "text/plain" )) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "application/rtf" ) ) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "text/html" ) ) != NULL )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
