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
#include <memory>
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;

typedef ::std::pair< OUString, OUString > FilterDesc;

namespace
{

OUString lcl_GetExtensionsList ( ::std::vector< FilterDesc > const& rFilterDescList )
{
    OUString aExtensions;
    ::std::vector< FilterDesc >::const_iterator aIter( rFilterDescList.begin() );

    while (aIter != rFilterDescList.end())
    {
        OUString sWildcard = (*aIter).second;

        if ( aExtensions.indexOf( sWildcard ) == -1 )
        {
            if ( !aExtensions.isEmpty() )
                aExtensions += ";";
            aExtensions += sWildcard;
        }

        ++aIter;
    }

    return aExtensions;
}

void lcl_AddFilter ( ::std::vector< FilterDesc >& rFilterDescList,
                     const SfxFilter *pFilter )
{
    if (pFilter)
        rFilterDescList.push_back( ::std::make_pair( pFilter->GetUIName(), pFilter->GetDefaultExtension() ) );
}

}

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

rtl::Reference<FuPoor> FuInsertFile::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuInsertFile( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuInsertFile::DoExecute( SfxRequest& rReq )
{
    SfxFilterMatcher&       rMatcher = SfxGetpApp()->GetFilterMatcher();
    ::std::vector< FilterDesc > aFilterVector;
    ::std::vector< OUString > aOtherFilterVector;
    const SfxItemSet*       pArgs = rReq.GetArgs ();

    FuInsertFile::GetSupportedFilterVector( aOtherFilterVector );

    if (!pArgs)
    {
        sfx2::FileDialogHelper      aFileDialog(
                ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                SFXWB_INSERT );
        Reference< XFilePicker >    xFilePicker( aFileDialog.GetFilePicker(), UNO_QUERY );
        Reference< XFilterManager > xFilterManager( xFilePicker, UNO_QUERY );
        OUString aOwnCont;
        OUString aOtherCont;

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
                // Get main filter
                const SfxFilter* pFilter = SfxFilter::GetDefaultFilterFromFactory( aOwnCont );
                lcl_AddFilter( aFilterVector, pFilter );

                // get template filter
                if( mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
                    pFilter = DrawDocShell::Factory().GetTemplateFilter();
                else
                    pFilter = GraphicDocShell::Factory().GetTemplateFilter();
                lcl_AddFilter( aFilterVector, pFilter );

                // get cross filter
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aOtherCont );
                lcl_AddFilter( aFilterVector, pFilter );

                // get Powerpoint filter
                OUString aExt = ".ppt";
                pFilter = aMatch.GetFilter4Extension( aExt );
                lcl_AddFilter( aFilterVector, pFilter );

                // Get other draw/impress filters
                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARIMPRESS_60, SfxFilterFlags::IMPORT, SfxFilterFlags::TEMPLATEPATH );
                lcl_AddFilter( aFilterVector, pFilter );

                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARIMPRESS_60, SfxFilterFlags::TEMPLATEPATH );
                lcl_AddFilter( aFilterVector, pFilter );

                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARDRAW_60, SfxFilterFlags::IMPORT, SfxFilterFlags::TEMPLATEPATH  );
                lcl_AddFilter( aFilterVector, pFilter );

                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARDRAW_60, SfxFilterFlags::TEMPLATEPATH  );
                lcl_AddFilter( aFilterVector, pFilter );

                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARDRAW, SfxFilterFlags::IMPORT, SfxFilterFlags::TEMPLATEPATH  );
                lcl_AddFilter( aFilterVector, pFilter );

                pFilter = aMatch.GetFilter4ClipBoardId( SotClipboardFormatId::STARDRAW, SfxFilterFlags::TEMPLATEPATH  );
                lcl_AddFilter( aFilterVector, pFilter );

                // add additional supported filters
                ::std::vector< OUString >::const_iterator aOtherIter( aOtherFilterVector.begin() );

                while( aOtherIter != aOtherFilterVector.end() )
                {
                    if( ( pFilter = rMatcher.GetFilter4Mime( *aOtherIter ) ) != NULL )
                        lcl_AddFilter( aFilterVector, pFilter );

                    ++aOtherIter;
                }

                // set default-filter (<All>)
                OUString aAllSpec( SD_RESSTR( STR_ALL_FILES ) );
                OUString aExtensions = lcl_GetExtensionsList( aFilterVector );
                OUString aGUIName = aAllSpec + " (" + aExtensions + ")";

                xFilterManager->appendFilter( aGUIName, aExtensions );
                xFilterManager->setCurrentFilter( aAllSpec );

                // add filters to filter manager finally
                ::std::vector< ::std::pair < OUString, OUString > >::const_iterator aIter( aFilterVector.begin() );

                while( aIter != aFilterVector.end() )
                {
                    xFilterManager->appendFilter( (*aIter).first, (*aIter).second );
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
        SFX_REQUEST_ARG (rReq, pFileName, SfxStringItem, ID_VAL_DUMMY0, false);
        SFX_REQUEST_ARG (rReq, pFilterName, SfxStringItem, ID_VAL_DUMMY1, false);

        aFile = pFileName->GetValue ();

        if( pFilterName )
            aFilterName = pFilterName->GetValue ();
    }

    mpDocSh->SetWaitCursor( true );

    SfxMedium*          pMedium = new SfxMedium( aFile, StreamMode::READ | StreamMode::NOCREATE );
    const SfxFilter*    pFilter = NULL;

    SfxGetpApp()->GetFilterMatcher().GuessFilter( *pMedium, &pFilter, SfxFilterFlags::IMPORT, SFX_FILTER_NOTINSTALLED );

    bool                bDrawMode = mpViewShell && mpViewShell->ISA(DrawViewShell);
    bool                bInserted = false;

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
                bInserted = true;
            }
        }
        else
        {
            bool bFound = ( ::std::find( aOtherFilterVector.begin(), aOtherFilterVector.end(), pFilter->GetMimeType() ) != aOtherFilterVector.end() );
            if( !bFound &&
                ( aFilterName.indexOf( "Text" ) != -1 ||
                aFilterName.indexOf( "Rich" ) != -1 ||
                aFilterName.indexOf( "RTF" )  != -1 ||
                aFilterName.indexOf( "HTML" ) != -1 ) )
            {
                bFound = true;
            }

            if( bFound )
            {
                if( bDrawMode )
                    InsTextOrRTFinDrMode(pMedium);
                else
                    InsTextOrRTFinOlMode(pMedium);

                bInserted = true;
                delete pMedium;
            }
        }
    }

    mpDocSh->SetWaitCursor( false );

    if( !bInserted )
    {
        ScopedVclPtrInstance< MessageDialog > aErrorBox(mpWindow, SD_RESSTR( STR_READ_DATA_ERROR));
        aErrorBox->Execute();
        delete pMedium;
    }
}

bool FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    bool bOK = false;

    mpDocSh->SetWaitCursor( false );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    boost::scoped_ptr<AbstractSdInsertPagesObjsDlg> pDlg(pFact ? pFact->CreateSdInsertPagesObjsDlg( NULL, mpDoc, pMedium, aFile ) : 0);

    if( !pDlg )
        return false;

    /* Maybe a QueryBox is opened ("update links?"), therefore the dialog
       becomes the current DefModalDialogParent */
    vcl::Window* pDefParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent(pDlg->GetWindow());

    sal_uInt16 nRet = pDlg->Execute();

    Application::SetDefDialogParent(pDefParent);

    mpDocSh->SetWaitCursor( true );

    if( nRet == RET_OK )
    {
        /* list with page names (if NULL, then all pages)
           First, insert pages */
        std::vector<OUString> aBookmarkList = pDlg->GetList( 1 ); // pages
        bool bLink = pDlg->IsLink();
        bool bReplace = false;
        SdPage* pPage = NULL;
        ::sd::View* pView = mpViewShell ? mpViewShell->GetView() : NULL;

        if (pView)
        {
            if (pView->ISA(OutlineView))
            {
                pPage = static_cast<OutlineView*>(pView)->GetActualPage();
            }
            else
            {
                pPage = static_cast<SdPage*>(pView->GetSdrPageView()->GetPage());
            }
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

        bool  bNameOK;
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
                                    false, NULL, true, true, false );

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

    return bOK;
}

void FuInsertFile::InsTextOrRTFinDrMode(SfxMedium* pMedium)
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    boost::scoped_ptr<AbstractSdInsertPagesObjsDlg> pDlg(pFact ? pFact->CreateSdInsertPagesObjsDlg(NULL, mpDoc, NULL, aFile ) : 0);
    if( !pDlg )
        return;

    mpDocSh->SetWaitCursor( false );

    sal_uInt16 nRet = pDlg->Execute();
    mpDocSh->SetWaitCursor( true );

    if( nRet == RET_OK )
    {
        // selected file format: text, RTF or HTML (default is text)
        sal_uInt16 nFormat = EE_FORMAT_TEXT;

        if( aFilterName.indexOf( "Rich") != -1 )
            nFormat = EE_FORMAT_RTF;
        else if( aFilterName.indexOf( "HTML" ) != -1 )
            nFormat = EE_FORMAT_HTML;

        /* create our own outline since:
           - it is possible that the document outliner is actually used in the
             structuring mode
           - the draw outliner of the drawing engine has to draw something in
             between
           - the global outliner could be used in SdPage::CreatePresObj */
        boost::scoped_ptr<SdrOutliner> pOutliner(new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT ));

        // set reference device
        pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
        aLayoutName = pPage->GetLayoutName();
        sal_Int32 nIndex = aLayoutName.indexOf(SD_LT_SEPARATOR);
        if( nIndex != -1 )
            aLayoutName = aLayoutName.copy(0, nIndex);

        pOutliner->SetPaperSize(pPage->GetSize());

        SvStream* pStream = pMedium->GetInStream();
        assert(pStream && "No InStream!");
        pStream->Seek( 0 );

        sal_uLong nErr = pOutliner->Read( *pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes() );

        if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
        {
            ScopedVclPtrInstance< MessageDialog > aErrorBox(mpWindow, SD_RESSTR(STR_READ_DATA_ERROR));
            aErrorBox->Execute();
        }
        else
        {
            // is it a master page?
            if (static_cast<DrawViewShell*>(mpViewShell)->GetEditMode() == EM_MASTERPAGE &&
                !pPage->IsMasterPage())
            {
                pPage = static_cast<SdPage*>(&(pPage->TRG_GetMasterPage()));
            }

            assert(pPage && "page not found");

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
                        sal_uLong nLen = pOutliner->GetText( pPara ).getLength();
                        pOutliner->QuickDelete( ESelection( 0, nLen, 1, 0 ) );
                        pOutliner->QuickInsertLineBreak( ESelection( 0, nLen, 0, nLen ) );
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
    }
}

void FuInsertFile::InsTextOrRTFinOlMode(SfxMedium* pMedium)
{
    // selected file format: text, RTF or HTML (default is text)
    sal_uInt16 nFormat = EE_FORMAT_TEXT;

    if( aFilterName.indexOf( "Rich") != -1 )
        nFormat = EE_FORMAT_RTF;
    else if( aFilterName.indexOf( "HTML" ) != -1 )
        nFormat = EE_FORMAT_HTML;

    ::Outliner&    rDocliner = static_cast<OutlineView*>(mpView)->GetOutliner();

    std::vector<Paragraph*> aSelList;
    rDocliner.GetView(0)->CreateSelectionList(aSelList);

    Paragraph* pPara = aSelList.empty() ? NULL : *(aSelList.begin());

    // what should we insert?
    while (pPara && !Outliner::HasParaFlag(pPara, ParaFlag::ISPAGE))
        pPara = rDocliner.GetParent(pPara);

    sal_Int32 nTargetPos = rDocliner.GetAbsPos(pPara) + 1;

    // apply layout of predecessor page
    sal_uInt16 nPage = 0;
    pPara = rDocliner.GetParagraph( rDocliner.GetAbsPos( pPara ) - 1 );
    while (pPara)
    {
        sal_Int32 nPos = rDocliner.GetAbsPos( pPara );
        if ( Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE ) )
            nPage++;
        pPara = rDocliner.GetParagraph( nPos - 1 );
    }
    SdPage* pPage = mpDoc->GetSdPage(nPage, PK_STANDARD);
    aLayoutName = pPage->GetLayoutName();
    sal_Int32 nIndex = aLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        aLayoutName = aLayoutName.copy(0, nIndex);

    /* create our own outline since:
       - it is possible that the document outliner is actually used in the
         structuring mode
       - the draw outliner of the drawing engine has to draw something in
         between
       - the global outliner could be used in SdPage::CreatePresObj */
    boost::scoped_ptr< ::Outliner> pOutliner(new ::Outliner( &mpDoc->GetItemPool(), OUTLINERMODE_OUTLINEOBJECT ));
    pOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));

    // set reference device
    pOutliner->SetRefDevice(SD_MOD()->GetRefDevice( *mpDocSh ));
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "No InStream!" );
    pStream->Seek( 0 );

    sal_uLong nErr = pOutliner->Read(*pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes());

    if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
    {
        ScopedVclPtrInstance< MessageDialog > aErrorBox(mpWindow, SD_RESSTR(STR_READ_DATA_ERROR));
        aErrorBox->Execute();
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
            if( Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE ) )
                nNewPages++;
            pPara = pOutliner->GetParagraph( ++nPos );
        }

        mpDocSh->SetWaitCursor( false );

        boost::scoped_ptr<SfxProgress> pProgress(new SfxProgress( mpDocSh, SD_RESSTR(STR_CREATE_PAGES), nNewPages));
        if( pProgress )
            pProgress->SetState( 0, 100 );

        nNewPages = 0;

        rDocliner.GetUndoManager().EnterListAction(
                                    SD_RESSTR(STR_UNDO_INSERT_FILE), OUString() );

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
                rDocliner.Insert( pOutliner->GetText(pSourcePara), nTargetPos, nDepth );
                OUString aStyleSheetName( pStyleSheet->GetName() );
                aStyleSheetName = aStyleSheetName.copy( 0, aStyleSheetName.getLength()-1 );
                aStyleSheetName += OUString::number( nDepth <= 0 ? 1 : nDepth+1 );
                SfxStyleSheetBasePool* pStylePool = mpDoc->GetStyleSheetPool();
                SfxStyleSheet* pOutlStyle = static_cast<SfxStyleSheet*>( pStylePool->Find( aStyleSheetName, pStyleSheet->GetFamily() ) );
                rDocliner.SetStyleSheet( nTargetPos, pOutlStyle );
            }

            if( Outliner::HasParaFlag( pSourcePara, ParaFlag::ISPAGE ) )
            {
                nNewPages++;
                if( pProgress )
                    pProgress->SetState( nNewPages );
            }

            pSourcePara = pOutliner->GetParagraph( ++nPos );
            nTargetPos++;
            nSourcePos++;
        }

        rDocliner.GetUndoManager().LeaveListAction();

        pProgress.reset();

        mpDocSh->SetWaitCursor( true );
    }
}

bool FuInsertFile::InsSDDinOlMode(SfxMedium* pMedium)
{
    OutlineView* pOlView = static_cast<OutlineView*>(mpView);

    // transfer Outliner content to SdDrawDocument
    pOlView->PrepareClose();

    // einlesen wie im Zeichenmodus
    if (InsSDDinDrMode(pMedium))
    {
        ::Outliner* pOutliner = pOlView->GetViewByWindow(mpWindow)->GetOutliner();

        // cut notification links temporarily
        Link<::Outliner*,void> aOldParagraphInsertedHdl = pOutliner->GetParaInsertedHdl();
        pOutliner->SetParaInsertedHdl( Link<::Outliner*,void>());
        Link<::Outliner*,void> aOldParagraphRemovingHdl = pOutliner->GetParaRemovingHdl();
        pOutliner->SetParaRemovingHdl( Link<::Outliner*,void>());
        Link<::Outliner*,void> aOldDepthChangedHdl = pOutliner->GetDepthChangedHdl();
        pOutliner->SetDepthChangedHdl( Link<::Outliner*,void>());
        Link<::Outliner*,void> aOldBeginMovingHdl = pOutliner->GetBeginMovingHdl();
        pOutliner->SetBeginMovingHdl( Link<::Outliner*,void>());
        Link<::Outliner*,void> aOldEndMovingHdl = pOutliner->GetEndMovingHdl();
        pOutliner->SetEndMovingHdl( Link<::Outliner*,void>());

        Link<EditStatus&,void> aOldStatusEventHdl = pOutliner->GetStatusEventHdl();
        pOutliner->SetStatusEventHdl(Link<EditStatus&,void>());

        pOutliner->Clear();
        pOlView->FillOutliner();

        // set links again
        pOutliner->SetParaInsertedHdl(aOldParagraphInsertedHdl);
        pOutliner->SetParaRemovingHdl(aOldParagraphRemovingHdl);
        pOutliner->SetDepthChangedHdl(aOldDepthChangedHdl);
        pOutliner->SetBeginMovingHdl(aOldBeginMovingHdl);
        pOutliner->SetEndMovingHdl(aOldEndMovingHdl);
        pOutliner->SetStatusEventHdl(aOldStatusEventHdl);

        return true;
    }
    else
        return false;
}

void FuInsertFile::GetSupportedFilterVector( ::std::vector< OUString >& rFilterVector )
{
    SfxFilterMatcher&   rMatcher = SfxGetpApp()->GetFilterMatcher();
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
