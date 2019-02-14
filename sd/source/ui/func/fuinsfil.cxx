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

#include <fuinsfil.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/progress.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editeng.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <vcl/weld.hxx>
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

#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <Window.hxx>
#include <View.hxx>
#include <strings.hrc>
#include <stlpool.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <strmname.h>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <OutlineView.hxx>
#include <OutlineViewShell.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <app.hrc>
#include <unmovss.hxx>
#include <Outliner.hxx>
#include <sdabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;

typedef ::std::pair< OUString, OUString > FilterDesc;

namespace
{

OUString lcl_GetExtensionsList ( ::std::vector< FilterDesc > const& rFilterDescList )
{
    OUStringBuffer aExtensions;

    for (const auto& rFilterDesc : rFilterDescList)
    {
        OUString sWildcard = rFilterDesc.second;

        if ( aExtensions.indexOf( sWildcard ) == -1 )
        {
            if ( !aExtensions.isEmpty() )
                aExtensions.append(";");
            aExtensions.append(sWildcard);
        }

    }

    return aExtensions.makeStringAndClear();
}

void lcl_AddFilter ( ::std::vector< FilterDesc >& rFilterDescList,
                     const std::shared_ptr<const SfxFilter>& pFilter )
{
    if (pFilter)
        rFilterDescList.emplace_back( pFilter->GetUIName(), pFilter->GetDefaultExtension() );
}

}

namespace sd {


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
                FileDialogFlags::Insert, mpWindow ? mpWindow->GetFrameWeld() : nullptr);
        Reference< XFilePicker >    xFilePicker( aFileDialog.GetFilePicker(), UNO_QUERY );
        Reference< XFilterManager > xFilterManager( xFilePicker, UNO_QUERY );
        OUString aOwnCont;
        OUString aOtherCont;

        aFileDialog.SetTitle( SdResId(STR_DLG_INSERT_PAGES_FROM_FILE) );

        if( mpDoc->GetDocumentType() == DocumentType::Impress )
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
                std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetDefaultFilterFromFactory( aOwnCont );
                lcl_AddFilter( aFilterVector, pFilter );

                // get template filter
                if( mpDoc->GetDocumentType() == DocumentType::Impress )
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
                for( const auto& rOtherFilter : aOtherFilterVector )
                {
                    if( ( pFilter = rMatcher.GetFilter4Mime( rOtherFilter ) ) != nullptr )
                        lcl_AddFilter( aFilterVector, pFilter );
                }

                // set "All supported formats" as the default filter
                OUString aAllSpec( SdResId( STR_ALL_SUPPORTED_FORMATS ) );
                OUString aExtensions = lcl_GetExtensionsList( aFilterVector );
                OUString aGUIName = aAllSpec + " (" + aExtensions + ")";

                xFilterManager->appendFilter( aGUIName, aExtensions );
                xFilterManager->setCurrentFilter( aAllSpec );

                // append individual filters
                for( const auto& rFilter : aFilterVector )
                {
                    xFilterManager->appendFilter( rFilter.first, rFilter.second );
                }

                // end with "All files" as fallback
                xFilterManager->appendFilter( SdResId( STR_ALL_FILES ), "*.*" );
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
        const SfxStringItem* pFileName = rReq.GetArg<SfxStringItem>(ID_VAL_DUMMY0);
        const SfxStringItem* pFilterName = rReq.GetArg<SfxStringItem>(ID_VAL_DUMMY1);

        aFile = pFileName->GetValue ();

        if( pFilterName )
            aFilterName = pFilterName->GetValue ();
    }

    mpDocSh->SetWaitCursor( true );

    std::unique_ptr<SfxMedium>  xMedium(new SfxMedium(aFile, StreamMode::READ | StreamMode::NOCREATE));
    std::shared_ptr<const SfxFilter> pFilter;

    SfxGetpApp()->GetFilterMatcher().GuessFilter(*xMedium, pFilter);

    bool                bDrawMode = dynamic_cast< const DrawViewShell *>( mpViewShell ) != nullptr;
    bool                bInserted = false;

    if( pFilter )
    {
        xMedium->SetFilter( pFilter );
        aFilterName = pFilter->GetFilterName();

        if( xMedium->IsStorage() || ( xMedium->GetInStream() && SotStorage::IsStorageFile( xMedium->GetInStream() ) ) )
        {
            if ( pFilter->GetServiceName() == "com.sun.star.presentation.PresentationDocument" ||
                 pFilter->GetServiceName() == "com.sun.star.drawing.DrawingDocument" )
            {
                // Draw, Impress or PowerPoint document
                // the ownership of the Medium is transferred
                if( bDrawMode )
                    InsSDDinDrMode(xMedium.release());
                else
                    InsSDDinOlMode(xMedium.release());

                // ownership of pMedium has changed in this case
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
                    InsTextOrRTFinDrMode(xMedium.get());
                else
                    InsTextOrRTFinOlMode(xMedium.get());

                bInserted = true;
                xMedium.reset();
            }
        }
    }

    mpDocSh->SetWaitCursor( false );

    if( !bInserted )
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(mpWindow->GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
        xErrorBox->run();
    }
}

bool FuInsertFile::InsSDDinDrMode(SfxMedium* pMedium)
{
    bool bOK = false;

    mpDocSh->SetWaitCursor( false );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    weld::Window* pParent = mpViewShell ? mpViewShell->GetFrameWeld() : nullptr;
    ScopedVclPtr<AbstractSdInsertPagesObjsDlg> pDlg( pFact->CreateSdInsertPagesObjsDlg(pParent, mpDoc, pMedium, aFile) );

    sal_uInt16 nRet = pDlg->Execute();

    mpDocSh->SetWaitCursor( true );

    if( nRet == RET_OK )
    {
        /* list with page names (if NULL, then all pages)
           First, insert pages */
        std::vector<OUString> aBookmarkList = pDlg->GetList( 1 ); // pages
        bool bLink = pDlg->IsLink();
        SdPage* pPage = nullptr;
        ::sd::View* pView = mpViewShell ? mpViewShell->GetView() : nullptr;

        if (pView)
        {
            if( dynamic_cast< const OutlineView *>( pView ) !=  nullptr)
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
            if (pPage->GetPageKind() == PageKind::Standard)
            {
                nPos = pPage->GetPageNum() + 2;
            }
            else if (pPage->GetPageKind() == PageKind::Notes)
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
                                    bLink, false/*bReplace*/, nPos,
                                    false, nullptr, true, true, false );

            aBookmarkList.clear();
            aExchangeList.clear();
        }

        // to ensure ... (see above)
        bNameOK = mpView->GetExchangeList( aExchangeList, aObjectBookmarkList, 1 );

        if( bNameOK )
            bOK = mpDoc->InsertBookmarkAsObject( aObjectBookmarkList, aExchangeList,
                                nullptr, nullptr, false );

        if( pDlg->IsRemoveUnnessesaryMasterPages() )
            mpDoc->RemoveUnnecessaryMasterPages();
    }

    return bOK;
}

void FuInsertFile::InsTextOrRTFinDrMode(SfxMedium* pMedium)
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSdInsertPagesObjsDlg> pDlg( pFact->CreateSdInsertPagesObjsDlg(mpViewShell->GetFrameWeld(), mpDoc, nullptr, aFile) );

    mpDocSh->SetWaitCursor( false );

    sal_uInt16 nRet = pDlg->Execute();
    mpDocSh->SetWaitCursor( true );

    if( nRet != RET_OK )
        return;

    // selected file format: text, RTF or HTML (default is text)
    EETextFormat nFormat = EETextFormat::Text;

    if( aFilterName.indexOf( "Rich") != -1 )
        nFormat = EETextFormat::Rtf;
    else if( aFilterName.indexOf( "HTML" ) != -1 )
        nFormat = EETextFormat::Html;

    /* create our own outline since:
       - it is possible that the document outliner is actually used in the
         structuring mode
       - the draw outliner of the drawing engine has to draw something in
         between
       - the global outliner could be used in SdPage::CreatePresObj */
    std::unique_ptr<SdrOutliner> pOutliner(new SdOutliner( mpDoc, OutlinerMode::TextObject ));

    // set reference device
    pOutliner->SetRefDevice( SD_MOD()->GetVirtualRefDevice() );

    SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
    aLayoutName = pPage->GetLayoutName();
    sal_Int32 nIndex = aLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        aLayoutName = aLayoutName.copy(0, nIndex);

    pOutliner->SetPaperSize(pPage->GetSize());

    SvStream* pStream = pMedium->GetInStream();
    assert(pStream && "No InStream!");
    pStream->Seek( 0 );

    ErrCode nErr = pOutliner->Read( *pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes() );

    if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(mpWindow->GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
        xErrorBox->run();
    }
    else
    {
        // is it a master page?
        if (static_cast<DrawViewShell*>(mpViewShell)->GetEditMode() == EditMode::MasterPage &&
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
                pObj->GetObjInventor()   == SdrInventor::Default &&
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

        std::unique_ptr<OutlinerParaObject> pOPO = pOutliner->CreateParaObject();

        if (pOutlinerView)
        {
            pOutlinerView->InsertText(*pOPO);
        }
        else
        {
            SdrRectObj* pTO = new SdrRectObj(
                mpView->getSdrModelFromSdrView(),
                OBJ_TEXT);
            pTO->SetOutlinerParaObject(std::move(pOPO));

            const bool bUndo = mpView->IsUndoEnabled();
            if( bUndo )
                mpView->BegUndo(SdResId(STR_UNDO_INSERT_TEXTFRAME));
            pPage->InsertObject(pTO);

            /* can be bigger as the maximal allowed size:
               limit object size if necessary */
            Size aSize(pOutliner->CalcTextSize());
            Size aMaxSize = mpDoc->GetMaxObjSize();
            aSize.setHeight( std::min(aSize.Height(), aMaxSize.Height()) );
            aSize.setWidth( std::min(aSize.Width(), aMaxSize.Width()) );
            aSize = mpWindow->LogicToPixel(aSize);

            // put it at the center of the window
            Size aTemp(mpWindow->GetOutputSizePixel());
            Point aPos(aTemp.Width() / 2, aTemp.Height() / 2);
            aPos.AdjustX( -(aSize.Width() / 2) );
            aPos.AdjustY( -(aSize.Height() / 2) );
            aSize = mpWindow->PixelToLogic(aSize);
            aPos = mpWindow->PixelToLogic(aPos);
            pTO->SetLogicRect(::tools::Rectangle(aPos, aSize));

            if (pDlg->IsLink())
            {
                pTO->SetTextLink(aFile, aFilterName );
            }

            if( bUndo )
            {
                mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoInsertObject(*pTO));
                mpView->EndUndo();
            }
        }
    }
}

void FuInsertFile::InsTextOrRTFinOlMode(SfxMedium* pMedium)
{
    // selected file format: text, RTF or HTML (default is text)
    EETextFormat nFormat = EETextFormat::Text;

    if( aFilterName.indexOf( "Rich") != -1 )
        nFormat = EETextFormat::Rtf;
    else if( aFilterName.indexOf( "HTML" ) != -1 )
        nFormat = EETextFormat::Html;

    ::Outliner&    rDocliner = static_cast<OutlineView*>(mpView)->GetOutliner();

    std::vector<Paragraph*> aSelList;
    rDocliner.GetView(0)->CreateSelectionList(aSelList);

    Paragraph* pPara = aSelList.empty() ? nullptr : *(aSelList.begin());

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
    SdPage* pPage = mpDoc->GetSdPage(nPage, PageKind::Standard);
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
    std::unique_ptr< ::Outliner> pOutliner(new ::Outliner( &mpDoc->GetItemPool(), OutlinerMode::OutlineObject ));
    pOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));

    // set reference device
    pOutliner->SetRefDevice(SD_MOD()->GetVirtualRefDevice());
    pOutliner->SetPaperSize(Size(0x7fffffff, 0x7fffffff));

    SvStream* pStream = pMedium->GetInStream();
    DBG_ASSERT( pStream, "No InStream!" );
    pStream->Seek( 0 );

    ErrCode nErr = pOutliner->Read(*pStream, pMedium->GetBaseURL(), nFormat, mpDocSh->GetHeaderAttributes());

    if (nErr || pOutliner->GetEditEngine().GetText().isEmpty())
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(mpWindow->GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
        xErrorBox->run();
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

        std::unique_ptr<SfxProgress> pProgress(new SfxProgress( mpDocSh, SdResId(STR_CREATE_PAGES), nNewPages));
        pProgress->SetState( 0, 100 );

        nNewPages = 0;

        ViewShellId nViewShellId = mpViewShell ? mpViewShell->GetViewShellBase().GetViewShellId() : ViewShellId(-1);
        rDocliner.GetUndoManager().EnterListAction(
                                    SdResId(STR_UNDO_INSERT_FILE), OUString(), 0, nViewShellId );

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

    // read in like in the character mode
    if (InsSDDinDrMode(pMedium))
    {
        ::Outliner* pOutliner = pOlView->GetViewByWindow(mpWindow)->GetOutliner();

        // cut notification links temporarily
        Link<Outliner::ParagraphHdlParam,void> aOldParagraphInsertedHdl = pOutliner->GetParaInsertedHdl();
        pOutliner->SetParaInsertedHdl( Link<Outliner::ParagraphHdlParam,void>());
        Link<Outliner::ParagraphHdlParam,void> aOldParagraphRemovingHdl = pOutliner->GetParaRemovingHdl();
        pOutliner->SetParaRemovingHdl( Link<Outliner::ParagraphHdlParam,void>());
        Link<Outliner::DepthChangeHdlParam,void> aOldDepthChangedHdl = pOutliner->GetDepthChangedHdl();
        pOutliner->SetDepthChangedHdl( Link<::Outliner::DepthChangeHdlParam,void>());
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
    std::shared_ptr<const SfxFilter> pSearchFilter;

    rFilterVector.clear();

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "text/plain" )) != nullptr )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "application/rtf" ) ) != nullptr )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );

    if( ( pSearchFilter = rMatcher.GetFilter4Mime( "text/html" ) ) != nullptr )
        rFilterVector.push_back( pSearchFilter->GetMimeType() );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
