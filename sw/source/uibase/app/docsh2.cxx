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

#include <config_features.h>

#include <com/sun/star/drawing/ModuleDispatcher.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <comphelper/processfactory.hxx>

#include <edtwin.hxx>
#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <svl/lckbitem.hxx>
#include <svl/eitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/dialogs.hrc>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/new.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/evntconf.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/classificationhelper.hxx>

#include <svtools/htmlcfg.hxx>
#include <svx/ofaitem.hxx>
#include <SwSmartTagMgr.hxx>
#include <sfx2/app.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <sot/storage.hxx>
#include <comphelper/classids.hxx>
#include <fmtcol.hxx>
#include <swevent.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <viewopt.hxx>
#include <globdoc.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <docstyle.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
#include <pagedesc.hxx>
#include <shellio.hxx>
#include <pview.hxx>
#include <srcview.hxx>
#include <poolfmt.hxx>
#include <usrpref.hxx>
#include <wdocsh.hxx>
#include <unotxdoc.hxx>
#include <acmplwrd.hxx>
#include <swmodule.hxx>
#include <unobaseclass.hxx>
#include <swwait.hxx>
#include <swcli.hxx>

#include <cmdid.h>
#include <globals.h>
#include <helpid.h>
#include <app.hrc>
#include <poolfmt.hrc>
#include <globals.hrc>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <editeng/acorrcfg.hxx>
#include <SwStyleNameMapper.hxx>
#include <officecfg/Office/Security.hxx>

#include <sfx2/fcontnr.hxx>

#include "dialog.hrc"
#include "swabstdlg.hxx"

#include <ndtxt.hxx>

#include <memory>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::sfx2;

// create DocInfo (virtual)
VclPtr<SfxDocumentInfoDialog> SwDocShell::CreateDocumentInfoDialog(const SfxItemSet &rSet)
{
    VclPtr<SfxDocumentInfoDialog> pDlg = VclPtr<SfxDocumentInfoDialog>::Create(nullptr, rSet);
    //only with statistics, when this document is being shown, not
    //from within the Doc-Manager
    SwDocShell* pDocSh = static_cast<SwDocShell*>( SfxObjectShell::Current());
    if( pDocSh == this )
    {
        //Not for SourceView.
        SfxViewShell *pVSh = SfxViewShell::Current();
        if ( pVSh && dynamic_cast< const SwSrcView *>( pVSh ) ==  nullptr )
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
            pDlg->AddFontTabPage();
            pDlg->AddTabPage(TP_DOC_STAT, SW_RESSTR(STR_DOC_STAT),pFact->GetTabPageCreatorFunc( TP_DOC_STAT ),nullptr);
        }
    }
    return pDlg;
}

void SwDocShell::ToggleLayoutMode(SwView* pView)
{
    OSL_ENSURE( pView, "SwDocShell::ToggleLayoutMode, pView is null." );

    const SwViewOption& rViewOptions = *pView->GetWrtShell().GetViewOptions();

    //TODO: Should HideWhitespace flag be saved in the document settings?
    GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, rViewOptions.getBrowseMode());
    UpdateFontList();  // Why is this necessary here?

    pView->GetViewFrame()->GetBindings().Invalidate(FN_SHADOWCURSOR);
    if( !GetDoc()->getIDocumentDeviceAccess().getPrinter( false ) )
        pView->SetPrinter( GetDoc()->getIDocumentDeviceAccess().getPrinter( false ), SfxPrinterChangeFlags::PRINTER | SfxPrinterChangeFlags::JOBSETUP );
    GetDoc()->CheckDefaultPageFormat();
    SfxViewFrame *pTmpFrame = SfxViewFrame::GetFirst(this, false);
    while (pTmpFrame)
    {
        if( pTmpFrame != pView->GetViewFrame() )
        {
            pTmpFrame->DoClose();
            pTmpFrame = SfxViewFrame::GetFirst(this, false);
        }
        else
            pTmpFrame = SfxViewFrame::GetNext(*pTmpFrame, this, false);
    }

    pView->GetWrtShell().InvalidateLayout(true);

    pView->RecheckBrowseMode();

    pView->SetNewWindowAllowed(!rViewOptions.getBrowseMode());
}

// update text fields on document properties changes
void SwDocShell::DoFlushDocInfo()
{
    if (!m_pDoc) return;

    bool bUnlockView(true);
    if (m_pWrtShell)
    {
        bUnlockView = !m_pWrtShell->IsViewLocked();
        m_pWrtShell->LockView( true );    // lock visible section
        m_pWrtShell->StartAllAction();
    }

    m_pDoc->getIDocumentStatistics().DocInfoChgd(IsEnableSetModified());

    if (m_pWrtShell)
    {
        m_pWrtShell->EndAllAction();
        if (bUnlockView)
        {
            m_pWrtShell->LockView( false );
        }
    }
}

static void lcl_processCompatibleSfxHint( const uno::Reference< script::vba::XVBAEventProcessor >& xVbaEvents, const SfxHint& rHint )
{
    using namespace com::sun::star::script::vba::VBAEventId;
    if ( const SfxEventHint* pSfxEventHint = dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        uno::Sequence< uno::Any > aArgs;
        switch( pSfxEventHint->GetEventId() )
        {
            case SfxEventHintId::CreateDoc:
                xVbaEvents->processVbaEvent( DOCUMENT_NEW, aArgs );
            break;
            case SfxEventHintId::OpenDoc:
                xVbaEvents->processVbaEvent( DOCUMENT_OPEN, aArgs );
            break;
            default: break;
        }
    }
}

// Notification on DocInfo changes
void SwDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (!m_pDoc)
    {
        return ;
    }

    uno::Reference< script::vba::XVBAEventProcessor > const xVbaEvents =
        m_pDoc->GetVbaEventProcessor();
    if( xVbaEvents.is() )
        lcl_processCompatibleSfxHint( xVbaEvents, rHint );

    sal_uInt16 nAction = 0;
    auto pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if( pEventHint && pEventHint->GetEventId() == SfxEventHintId::LoadFinished )
    {
        // #i38126# - own action id
        nAction = 3;
    }
    else
    {
        // switch for more actions
        if( rHint.GetId() == SfxHintId::TitleChanged)
        {
            if( GetMedium() )
                nAction = 2;
        }
    }

    if( nAction )
    {
        bool bUnlockView = true; //initializing prevents warning
        if (m_pWrtShell)
        {
            bUnlockView = !m_pWrtShell->IsViewLocked();
            m_pWrtShell->LockView( true );    //lock visible section
            m_pWrtShell->StartAllAction();
        }
        switch( nAction )
        {
        case 2:
            m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_FILENAMEFLD )->UpdateFields();
            break;
        // #i38126# - own action for event LOADFINISHED
        // in order to avoid a modified document.
        // #i41679# - Also for the instance of <SwDoc>
        // it has to be assured, that it's not modified.
        // Perform the same as for action id 1, but disable <SetModified>.
        case 3:
            {
                const bool bResetModified = IsEnableSetModified();
                if ( bResetModified )
                    EnableSetModified( false );
                // #i41679#
                const bool bIsDocModified = m_pDoc->getIDocumentState().IsModified();
                // TODO: is the ResetModified() below because of only the direct call from DocInfoChgd, or does UpdateFields() set it too?

                m_pDoc->getIDocumentStatistics().DocInfoChgd(false);

                // #i41679#
                if ( !bIsDocModified )
                    m_pDoc->getIDocumentState().ResetModified();
                if ( bResetModified )
                    EnableSetModified();
            }
            break;
        }

        if (m_pWrtShell)
        {
            m_pWrtShell->EndAllAction();
            if( bUnlockView )
                m_pWrtShell->LockView( false );
        }
    }
}

// Notification Close Doc
bool SwDocShell::PrepareClose( bool bUI )
{
    bool bRet = SfxObjectShell::PrepareClose( bUI );

    if( bRet )
        EndListening( *this );

    if (m_pDoc && IsInPrepareClose())
    {
        uno::Reference< script::vba::XVBAEventProcessor > const xVbaEvents =
            m_pDoc->GetVbaEventProcessor();
        if( xVbaEvents.is() )
        {
            using namespace com::sun::star::script::vba::VBAEventId;
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( DOCUMENT_CLOSE, aArgs );
        }
    }
    return bRet;
}

void SwDocShell::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    sal_uInt16 nWhich = rReq.GetSlot();
    bool bDone = false;
    switch ( nWhich )
    {
        case SID_AUTO_CORRECT_DLG:
        {
            SvxSwAutoFormatFlags* pAFlags = &SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
            SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

            bool bOldLocked = rACW.IsLockWordLstLocked(),
                 bOldAutoCmpltCollectWords = pAFlags->bAutoCmpltCollectWords;

            rACW.SetLockWordLstLocked( true );

            editeng::SortedAutoCompleteStrings aTmpLst( rACW.GetWordList() );
            pAFlags->m_pAutoCompleteList = &aTmpLst;

            SfxApplication* pApp = SfxGetpApp();
            SfxRequest aAppReq(SID_AUTO_CORRECT_DLG, SfxCallMode::SYNCHRON, pApp->GetPool());
            SfxBoolItem aSwOptions( SID_AUTO_CORRECT_DLG, true );
            aAppReq.AppendItem(aSwOptions);

            pAFlags->pSmartTagMgr = &SwSmartTagMgr::Get();

            SfxItemSet aSet( pApp->GetPool(), SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG, SID_OPEN_SMARTTAGOPTIONS, SID_OPEN_SMARTTAGOPTIONS, 0 );
            aSet.Put( aSwOptions );

            const SfxPoolItem* pOpenSmartTagOptionsItem = nullptr;
            if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_OPEN_SMARTTAGOPTIONS, false, &pOpenSmartTagOptionsItem ) )
                aSet.Put( *static_cast<const SfxBoolItem*>(pOpenSmartTagOptionsItem) );

            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            VclPtr<SfxAbstractTabDialog> pDlg = pFact->CreateAutoCorrTabDialog( &aSet );
            pDlg->Execute();
            pDlg.disposeAndClear();


            rACW.SetLockWordLstLocked( bOldLocked );

            SwEditShell::SetAutoFormatFlags( pAFlags );
            rACW.SetMinWordLen( pAFlags->nAutoCmpltWordLen );
            rACW.SetMaxCount( pAFlags->nAutoCmpltListLen );
            if (pAFlags->m_pAutoCompleteList)  // any changes?
            {
                rACW.CheckChangedList( aTmpLst );
                // clear the temp WordList pointer
                pAFlags->m_pAutoCompleteList = nullptr;
            }
            // remove all pointer we never delete the strings
            aTmpLst.clear();

            if( !bOldAutoCmpltCollectWords && bOldAutoCmpltCollectWords !=
                pAFlags->bAutoCmpltCollectWords )
            {
                // call on all Docs the idle formatter to start
                // the collection of Words
                for( SwDocShell *pDocSh = static_cast<SwDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<SwDocShell>));
                     pDocSh;
                     pDocSh = static_cast<SwDocShell*>(SfxObjectShell::GetNext( *pDocSh, checkSfxObjectShell<SwDocShell> )) )
                {
                    SwDoc* pTmp = pDocSh->GetDoc();
                    if ( pTmp->getIDocumentLayoutAccess().GetCurrentViewShell() )
                        pTmp->InvalidateAutoCompleteFlag();
                }
            }
        }
        break;

        case SID_PRINTPREVIEW:
            {
                bool bSet = false;
                bool bFound = false, bOnly = true;
                SfxViewFrame *pTmpFrame = SfxViewFrame::GetFirst(this);
                SfxViewShell* pViewShell = SfxViewShell::Current();
                SwView* pCurrView = dynamic_cast< SwView *> ( pViewShell );
                bool bCurrent = typeid(SwPagePreview) == typeid( pViewShell );

                while( pTmpFrame )    // search Preview
                {
                    if( typeid(SwView) == typeid( pTmpFrame->GetViewShell()) )
                        bOnly = false;
                    else if( typeid(SwPagePreview) == typeid( pTmpFrame->GetViewShell()))
                    {
                        pTmpFrame->GetFrame().Appear();
                        bFound = true;
                    }
                    if( bFound && !bOnly )
                        break;
                    pTmpFrame = SfxViewFrame::GetNext(*pTmpFrame, this);
                }

                if( pArgs && SfxItemState::SET ==
                    pArgs->GetItemState( SID_PRINTPREVIEW, false, &pItem ))
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                    bSet = !bCurrent;

                sal_uInt16 nSlotId = 0;
                if( bSet && !bFound )   // Nothing found, so create new Preview
                        nSlotId = SID_VIEWSHELL1;
                else if( bFound && !bSet )
                    nSlotId = bOnly ? SID_VIEWSHELL0 : SID_VIEWSHELL1;

                if( nSlotId )
                {
                    // PagePreview in the WebDocShell
                    // is found under Id VIEWSHELL2.
                    if( dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr && SID_VIEWSHELL1 == nSlotId )
                        nSlotId = SID_VIEWSHELL2;

                    if( pCurrView && pCurrView->GetDocShell() == this )
                        pTmpFrame = pCurrView->GetViewFrame();
                    else
                        pTmpFrame = SfxViewFrame::GetFirst( this );

                    if (pTmpFrame)
                        pTmpFrame->GetDispatcher()->Execute( nSlotId, SfxCallMode::ASYNCHRON );
                }

                rReq.SetReturnValue(SfxBoolItem(SID_PRINTPREVIEW, bSet ));
            }
            break;
        case SID_TEMPLATE_LOAD:
            {
                OUString aFileName;
                static bool bText = true;
                static bool bFrame = false;
                static bool bPage =  false;
                static bool bNum =   false;
                static bool bMerge = false;
                sal_uInt16 nRet = USHRT_MAX;

                SfxTemplateFlags nFlags = bFrame ? SfxTemplateFlags::LOAD_FRAME_STYLES : SfxTemplateFlags::NONE;
                if(bPage)
                    nFlags |= SfxTemplateFlags::LOAD_PAGE_STYLES;
                if(bNum)
                    nFlags |= SfxTemplateFlags::LOAD_NUM_STYLES;
                if(nFlags == SfxTemplateFlags::NONE || bText)
                    nFlags |= SfxTemplateFlags::LOAD_TEXT_STYLES;
                if(bMerge)
                    nFlags |= SfxTemplateFlags::MERGE_STYLES;

                if ( pArgs )
                {
                    const SfxStringItem* pTemplateItem = rReq.GetArg<SfxStringItem>(SID_TEMPLATE_NAME);
                    if ( pTemplateItem )
                    {
                        aFileName = pTemplateItem->GetValue();
                        const SfxInt32Item* pFlagsItem = rReq.GetArg<SfxInt32Item>(SID_TEMPLATE_LOAD);
                        if ( pFlagsItem )
                            nFlags = static_cast<SfxTemplateFlags>((sal_uInt16) pFlagsItem->GetValue());
                    }
                }

                if ( aFileName.isEmpty() )
                {
                    SvtPathOptions aPathOpt;
                    ScopedVclPtr<SfxNewFileDialog> pNewFileDlg(
                        VclPtr<SfxNewFileDialog>::Create(&GetView()->GetViewFrame()->GetWindow(), SfxNewFileDialogMode::LoadTemplate));
                    pNewFileDlg->SetTemplateFlags(nFlags);

                    nRet = pNewFileDlg->Execute();
                    if(RET_TEMPLATE_LOAD == nRet)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE );
                        uno::Reference < XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );

                        SfxObjectFactory &rFact = GetFactory();
                        SfxFilterMatcher aMatcher( OUString::createFromAscii(rFact.GetShortName()) );
                        SfxFilterMatcherIter aIter( aMatcher );
                        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
                        std::shared_ptr<const SfxFilter> pFlt = aIter.First();
                        while( pFlt )
                        {
                            // --> OD #i117339#
                            if( pFlt && pFlt->IsAllowedAsTemplate() &&
                                ( pFlt->GetUserData() == "CXML" ||
                                  pFlt->GetUserData() == "CXMLV" ) )
                            {
                                const OUString sWild = pFlt->GetWildcard().getGlob();
                                xFltMgr->appendFilter( pFlt->GetUIName(), sWild );
                            }
                            pFlt = aIter.Next();
                        }
                        bool bWeb = dynamic_cast< SwWebDocShell *>( this ) !=  nullptr;
                        std::shared_ptr<const SfxFilter> pOwnFlt =
                                SwDocShell::Factory().GetFilterContainer()->
                                GetFilter4FilterName("writer8");

                        // make sure the default file format is also available
                        if(bWeb)
                        {
                            const OUString sWild = pOwnFlt->GetWildcard().getGlob();
                            xFltMgr->appendFilter( pOwnFlt->GetUIName(), sWild );
                        }

                        bool bError = false;
                        // catch exception if wrong filter is selected - should not happen anymore
                        try
                        {
                            xFltMgr->setCurrentFilter( pOwnFlt->GetUIName() );
                        }
                        catch (const uno::Exception&)
                        {
                            bError = true;
                        }

                        if( !bError && ERRCODE_NONE == aDlgHelper.Execute() )
                        {
                            aFileName = xFP->getSelectedFiles().getConstArray()[0];
                        }
                    }
                    else if( RET_OK == nRet)
                    {
                        aFileName = pNewFileDlg->GetTemplateFileName();
                    }

                    nFlags = pNewFileDlg->GetTemplateFlags();
                    rReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, aFileName ) );
                    rReq.AppendItem( SfxInt32Item( SID_TEMPLATE_LOAD, (long) nFlags ) );
                }

                if( !aFileName.isEmpty() )
                {
                    SwgReaderOption aOpt;
                    aOpt.SetTextFormats(    bText  = bool(nFlags & SfxTemplateFlags::LOAD_TEXT_STYLES ));
                    aOpt.SetFrameFormats(    bFrame = bool(nFlags & SfxTemplateFlags::LOAD_FRAME_STYLES));
                    aOpt.SetPageDescs(  bPage  = bool(nFlags & SfxTemplateFlags::LOAD_PAGE_STYLES ));
                    aOpt.SetNumRules(   bNum   = bool(nFlags & SfxTemplateFlags::LOAD_NUM_STYLES  ));
                    //different meaning between SFX_MERGE_STYLES and aOpt.SetMerge!
                    bMerge = bool(nFlags & SfxTemplateFlags::MERGE_STYLES);
                    aOpt.SetMerge( !bMerge );

                    SetError( LoadStylesFromFile( aFileName, aOpt, false ), OSL_LOG_PREFIX);
                    if ( !GetError() )
                        rReq.Done();
                }
            }
            break;
            case SID_SOURCEVIEW:
            {
                SfxViewShell* pViewShell = GetView()
                                            ? static_cast<SfxViewShell*>(GetView())
                                            : SfxViewShell::Current();
                SfxViewFrame*  pViewFrame = pViewShell->GetViewFrame();
                SwSrcView* pSrcView = dynamic_cast< SwSrcView *>( pViewShell );
                if(!pSrcView)
                {
                    // 3 possible state:
                    // 1 - file unsaved -> save as HTML
                    // 2 - file modified and HTML filter active -> save
                    // 3 - file saved in non-HTML -> QueryBox to save as HTML
                    std::shared_ptr<const SfxFilter> pHtmlFlt =
                                    SwIoSystem::GetFilterOfFormat(
                                        "HTML",
                                        SwWebDocShell::Factory().GetFilterContainer() );
                    bool bLocalHasName = HasName();
                    if(bLocalHasName)
                    {
                        //check for filter type
                        std::shared_ptr<const SfxFilter> pFlt = GetMedium()->GetFilter();
                        if(!pFlt || pFlt->GetUserData() != pHtmlFlt->GetUserData())
                        {
                            ScopedVclPtrInstance<MessageDialog> aQuery(&pViewFrame->GetWindow(),
                                                                       "SaveAsHTMLDialog", "modules/swriter/ui/saveashtmldialog.ui");

                            if(RET_YES == aQuery->Execute())
                                bLocalHasName = false;
                            else
                                break;
                        }
                    }
                    if(!bLocalHasName)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION );
                        aDlgHelper.AddFilter( pHtmlFlt->GetFilterName(), pHtmlFlt->GetDefaultExtension() );
                        aDlgHelper.SetCurrentFilter( pHtmlFlt->GetFilterName() );
                        if( ERRCODE_NONE != aDlgHelper.Execute())
                        {
                            break;
                        }
                        OUString sPath = aDlgHelper.GetPath();
                        SfxStringItem aName(SID_FILE_NAME, sPath);
                        SfxStringItem aFilter(SID_FILTER_NAME, pHtmlFlt->GetName());
                        const SfxBoolItem* pBool = static_cast<const SfxBoolItem*>(
                                pViewFrame->GetDispatcher()->ExecuteList(
                                        SID_SAVEASDOC, SfxCallMode::SYNCHRON,
                                        { &aName, &aFilter }));
                        if(!pBool || !pBool->GetValue())
                            break;
                    }
                }

                OSL_ENSURE(dynamic_cast<SwWebDocShell*>(this),
                            "SourceView only in WebDocShell");

                // the SourceView is not the 1 for SwWebDocShell
                sal_uInt16 nSlot = SID_VIEWSHELL1;
                bool bSetModified = false;
                VclPtr<SfxPrinter> pSavePrinter;
                if( nullptr != pSrcView)
                {
                    SfxPrinter* pTemp = GetDoc()->getIDocumentDeviceAccess().getPrinter( false );
                    if(pTemp)
                        pSavePrinter = VclPtr<SfxPrinter>::Create(*pTemp);
                    bSetModified = IsModified() || pSrcView->IsModified();
                    if(pSrcView->IsModified()||pSrcView->HasSourceSaved())
                    {
                        utl::TempFile aTempFile;
                        aTempFile.EnableKillingFile();
                        pSrcView->SaveContent(aTempFile.GetURL());
                        bDone = true;
                        SvxMacro aMac(aEmptyOUStr, aEmptyOUStr, STARBASIC);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( GlobalEventId::OPENDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( GlobalEventId::PREPARECLOSEDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( GlobalEventId::ACTIVATEDOC ),     aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( GlobalEventId::DEACTIVATEDOC ), aMac, this);
                        ReloadFromHtml(aTempFile.GetURL(), pSrcView);
                        nSlot = 0;
                    }
                    else
                    {
                        nSlot = SID_VIEWSHELL0;
                    }
                }
                if(nSlot)
                    pViewFrame->GetDispatcher()->Execute(nSlot, SfxCallMode::SYNCHRON);
                if(bSetModified)
                    GetDoc()->getIDocumentState().SetModified();
                if(pSavePrinter)
                {
                    GetDoc()->getIDocumentDeviceAccess().setPrinter( pSavePrinter, true, true);
                    //pSavePrinter must not be deleted again
                }
                pViewFrame->GetBindings().SetState(SfxBoolItem(SID_SOURCEVIEW, nSlot == SID_VIEWSHELL2));
                pViewFrame->GetBindings().Invalidate( SID_NEWWINDOW );
                pViewFrame->GetBindings().Invalidate( SID_BROWSER_MODE );
                pViewFrame->GetBindings().Invalidate( FN_PRINT_LAYOUT );
            }
            break;
            case SID_GET_COLORLIST:
            {
                const SvxColorListItem* pColItem = static_cast<const SvxColorListItem*>(GetItem(SID_COLOR_TABLE));
                XColorListRef pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaRefItem<XColorList>(SID_GET_COLORLIST, pList));
            }
            break;
        case FN_ABSTRACT_STARIMPRESS:
        case FN_ABSTRACT_NEWDOC:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            ScopedVclPtr<AbstractSwInsertAbstractDlg> pDlg(pFact->CreateSwInsertAbstractDlg());
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            if(RET_OK == pDlg->Execute())
            {
                sal_uInt8 nLevel = pDlg->GetLevel();
                sal_uInt8 nPara = pDlg->GetPara();
                SwDoc* pSmryDoc = new SwDoc();
                SfxObjectShellLock xDocSh( new SwDocShell( pSmryDoc, SfxObjectCreateMode::STANDARD));
                xDocSh->DoInitNew();

                bool bImpress = FN_ABSTRACT_STARIMPRESS == nWhich;
                m_pDoc->Summary( pSmryDoc, nLevel, nPara, bImpress );
                if( bImpress )
                {
                    WriterRef xWrt;
                    // mba: looks as if relative URLs don't make sense here
                    ::GetRTFWriter( aEmptyOUStr, OUString(), xWrt );
                    SvMemoryStream *pStrm = new SvMemoryStream();
                    pStrm->SetBufferSize( 16348 );
                    SwWriter aWrt( *pStrm, *pSmryDoc );
                    ErrCode eErr = aWrt.Write( xWrt );
                    if( !IgnoreWarning( eErr ) )
                    {
                        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                        uno::Reference< frame::XDispatchProvider > xProv = drawing::ModuleDispatcher::create( xContext );

                        OUString aCmd("SendOutlineToImpress");
                        uno::Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
                        pStrm->Seek( STREAM_SEEK_TO_END );
                        pStrm->WriteChar( '\0' );
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );

                        // Transfer ownership of stream to a lockbytes object
                        SvLockBytes aLockBytes( pStrm, true );
                        SvLockBytesStat aStat;
                        if ( aLockBytes.Stat( &aStat, SVSTATFLAG_DEFAULT ) == ERRCODE_NONE )
                        {
                            sal_uInt32 nLen = aStat.nSize;
                            std::size_t nRead = 0;
                            uno::Sequence< sal_Int8 > aSeq( nLen );
                            aLockBytes.ReadAt( 0, aSeq.getArray(), nLen, &nRead );

                            uno::Sequence< beans::PropertyValue > aArgs(1);
                            aArgs[0].Name = "RtfOutline";
                            aArgs[0].Value <<= aSeq;
                            xHelper->executeDispatch( xProv, aCmd, OUString(), 0, aArgs );
                        }
                    }
                    else
                        ErrorHandler::HandleError(ErrCode( eErr ));
                }
                else
                {
                    // Create new document
                    SfxViewFrame *pFrame = SfxViewFrame::LoadDocument( *xDocSh, 0 );
                    SwView      *pCurrView = static_cast<SwView*>( pFrame->GetViewShell());

                    // Set document's title
                    OUString aTmp( SW_RES(STR_ABSTRACT_TITLE) );
                    aTmp += GetTitle();
                    xDocSh->SetTitle( aTmp );
                    pCurrView->GetWrtShell().SetNewDoc();
                    pFrame->Show();
                    pSmryDoc->getIDocumentState().SetModified();
                }

            }
        }
        break;
        case FN_OUTLINE_TO_CLIPBOARD:
        case FN_OUTLINE_TO_IMPRESS:
            {
                bool bEnable = IsEnableSetModified();
                EnableSetModified( false );
                WriterRef xWrt;
                // mba: looks as if relative URLs don't make sense here
                ::GetRTFWriter( OUString('O'), OUString(), xWrt );
                SvMemoryStream *pStrm = new SvMemoryStream();
                pStrm->SetBufferSize( 16348 );
                SwWriter aWrt( *pStrm, *GetDoc() );
                ErrCode eErr = aWrt.Write( xWrt );
                EnableSetModified( bEnable );
                if( !IgnoreWarning( eErr ) )
                {
                    pStrm->Seek( STREAM_SEEK_TO_END );
                    pStrm->WriteChar( '\0' );
                    pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                    if ( nWhich == FN_OUTLINE_TO_IMPRESS )
                    {
                        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                        uno::Reference< frame::XDispatchProvider > xProv = drawing::ModuleDispatcher::create( xContext );

                        OUString aCmd("SendOutlineToImpress");
                        uno::Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
                        pStrm->Seek( STREAM_SEEK_TO_END );
                        pStrm->WriteChar( '\0' );
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );

                        // Transfer ownership of stream to a lockbytes object
                        SvLockBytes aLockBytes( pStrm, true );
                        SvLockBytesStat aStat;
                        if ( aLockBytes.Stat( &aStat, SVSTATFLAG_DEFAULT ) == ERRCODE_NONE )
                        {
                            sal_uInt32 nLen = aStat.nSize;
                            std::size_t nRead = 0;
                            uno::Sequence< sal_Int8 > aSeq( nLen );
                            aLockBytes.ReadAt( 0, aSeq.getArray(), nLen, &nRead );

                            uno::Sequence< beans::PropertyValue > aArgs(1);
                            aArgs[0].Name = "RtfOutline";
                            aArgs[0].Value <<= aSeq;
                            xHelper->executeDispatch( xProv, aCmd, OUString(), 0, aArgs );
                        }
                    }
                    else
                    {
                        TransferDataContainer* pClipCntnr = new TransferDataContainer;
                        css::uno::Reference< css::datatransfer::XTransferable > xRef( pClipCntnr );

                        pClipCntnr->CopyAnyData( SotClipboardFormatId::RTF, static_cast<sal_Char const *>(
                                    pStrm->GetData()), pStrm->GetEndOfData() );
                        pClipCntnr->CopyToClipboard(
                            GetView()? &GetView()->GetEditWin() : nullptr );
                        delete pStrm;
                    }
                }
                else
                    ErrorHandler::HandleError(ErrCode( eErr ));
            }
            break;
            case SID_SPELLCHECKER_CHANGED:
                //! false, true, true is on the save side but a probably overdone
                SwModule::CheckSpellChanges(false, true, true, false );
            break;

        case SID_MAIL_PREPAREEXPORT:
        {
            //pWrtShell is not set in page preview
            if (m_pWrtShell)
                m_pWrtShell->StartAllAction();
            m_pDoc->getIDocumentFieldsAccess().UpdateFields( false );
            m_pDoc->getIDocumentLinksAdministration().EmbedAllLinks();
            m_IsRemovedInvisibleContent
                = officecfg::Office::Security::HiddenContent::RemoveHiddenContent::get();
            if (m_IsRemovedInvisibleContent)
                m_pDoc->RemoveInvisibleContent();
            if (m_pWrtShell)
                m_pWrtShell->EndAllAction();
        }
        break;

        case SID_MAIL_EXPORT_FINISHED:
        {
                if (m_pWrtShell)
                    m_pWrtShell->StartAllAction();
                //try to undo the removal of invisible content
                if (m_IsRemovedInvisibleContent)
                    m_pDoc->RestoreInvisibleContent();
                if (m_pWrtShell)
                    m_pWrtShell->EndAllAction();
        }
        break;
        case FN_NEW_HTML_DOC:
        case FN_NEW_GLOBAL_DOC:
            {
                bDone = false;
                bool bCreateHtml = FN_NEW_HTML_DOC == nWhich;

                bool bCreateByOutlineLevel = false;
                sal_Int32  nTemplateOutlineLevel = 0;

                OUString aFileName, aTemplateName;
                if( pArgs && SfxItemState::SET == pArgs->GetItemState( nWhich, false, &pItem ) )
                {
                    aFileName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    const SfxStringItem* pTemplItem = SfxItemSet::GetItem<SfxStringItem>(pArgs, SID_TEMPLATE_NAME, false);
                    if ( pTemplItem )
                        aTemplateName = pTemplItem->GetValue();
                }
                if ( aFileName.isEmpty() )
                {
                    bool bError = false;

                    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE );

                    const sal_Int16 nControlIds[] = {
                        CommonFilePickerElementIds::PUSHBUTTON_OK,
                        CommonFilePickerElementIds::PUSHBUTTON_CANCEL,
                        CommonFilePickerElementIds::LISTBOX_FILTER,
                        CommonFilePickerElementIds::CONTROL_FILEVIEW,
                        CommonFilePickerElementIds::EDIT_FILEURL,
                        ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION,
                        ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                        0
                    };

                    const char* aHTMLHelpIds[] =
                    {
                         HID_SEND_HTML_CTRL_PUSHBUTTON_OK,
                         HID_SEND_HTML_CTRL_PUSHBUTTON_CANCEL,
                         HID_SEND_HTML_CTRL_LISTBOX_FILTER,
                         HID_SEND_HTML_CTRL_CONTROL_FILEVIEW,
                         HID_SEND_HTML_CTRL_EDIT_FILEURL,
                         HID_SEND_HTML_CTRL_CHECKBOX_AUTOEXTENSION,
                         HID_SEND_HTML_CTRL_LISTBOX_TEMPLATE,
                         ""
                    };

                    const char* aMasterHelpIds[] =
                    {
                         HID_SEND_MASTER_CTRL_PUSHBUTTON_OK,
                         HID_SEND_MASTER_CTRL_PUSHBUTTON_CANCEL,
                         HID_SEND_MASTER_CTRL_LISTBOX_FILTER,
                         HID_SEND_MASTER_CTRL_CONTROL_FILEVIEW,
                         HID_SEND_MASTER_CTRL_EDIT_FILEURL,
                         HID_SEND_MASTER_CTRL_CHECKBOX_AUTOEXTENSION,
                         HID_SEND_MASTER_CTRL_LISTBOX_TEMPLATE,
                         ""
                    };

                    const char** pHelpIds = bCreateHtml ? aHTMLHelpIds : aMasterHelpIds;
                    aDlgHelper.SetControlHelpIds( nControlIds, pHelpIds );
                    uno::Reference < XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

                    std::shared_ptr<const SfxFilter> pFlt;
                    sal_uInt16 nStrId;

                    if( bCreateHtml )
                    {
                        // for HTML there is only one filter!!
                        pFlt = SwIoSystem::GetFilterOfFormat(
                                "HTML",
                                SwWebDocShell::Factory().GetFilterContainer() );
                        nStrId = STR_LOAD_HTML_DOC;
                    }
                    else
                    {
                        // for Global-documents we now only offer the current one.
                        pFlt = SwGlobalDocShell::Factory().GetFilterContainer()->
                                    GetFilter4Extension( "odm"  );
                        nStrId = STR_LOAD_GLOBAL_DOC;
                    }

                    if( pFlt )
                    {
                        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
                        const OUString sWild = pFlt->GetWildcard().getGlob();
                        xFltMgr->appendFilter( pFlt->GetUIName(), sWild );
                        try
                        {
                            xFltMgr->setCurrentFilter( pFlt->GetUIName() ) ;
                        }
                        catch (const uno::Exception&)
                        {
                            bError = true;
                        }
                    }
                    if(!bError)
                    {
                        uno::Reference<XFilePickerControlAccess> xCtrlAcc(xFP, UNO_QUERY);

                        bool    bOutline[MAXLEVEL] = {false};
                        const SwOutlineNodes& rOutlNds = m_pDoc->GetNodes().GetOutLineNds();
                        if( !rOutlNds.empty() )
                        {
                            for( size_t n = 0; n < rOutlNds.size(); ++n )
                            {
                                const int nLevel = rOutlNds[n]->GetTextNode()->GetAttrOutlineLevel();
                                if( nLevel > 0 && ! bOutline[nLevel-1] )
                                {
                                    bOutline[nLevel-1] = true;
                                }
                            }
                        }

                        const sal_uInt16 nStyleCount = m_pDoc->GetTextFormatColls()->size();
                        Sequence<OUString> aListBoxEntries( MAXLEVEL + nStyleCount);
                        OUString* pEntries = aListBoxEntries.getArray();
                        sal_Int32   nIdx = 0 ;

                        OUString    sOutline( SW_RESSTR(STR_FDLG_OUTLINE_LEVEL) );
                        for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
                        {
                            if( bOutline[i] )
                                pEntries[nIdx++] = sOutline + OUString::number( i+1 );
                        }

                        OUString    sStyle( SW_RESSTR(STR_FDLG_STYLE) );
                        for(sal_uInt16 i = 0; i < nStyleCount; ++i)
                        {
                            SwTextFormatColl &rTextColl = *(*m_pDoc->GetTextFormatColls())[ i ];
                            if( !rTextColl.IsDefault() && rTextColl.IsAtDocNodeSet() )
                            {
                                pEntries[nIdx++] = sStyle + rTextColl.GetName();
                            }
                        }

                        aListBoxEntries.realloc(nIdx);
                        sal_Int16 nSelect = 0;

                        try
                        {
                            Any aTemplates(&aListBoxEntries, cppu::UnoType<decltype(aListBoxEntries)>::get());

                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::ADD_ITEMS , aTemplates );
                            Any aSelectPos(&nSelect, cppu::UnoType<decltype(nSelect)>::get());
                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::SET_SELECT_ITEM, aSelectPos );
                            xCtrlAcc->setLabel( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                                    SW_RES( STR_FDLG_TEMPLATE_NAME ));
                        }
                        catch (const Exception&)
                        {
                            OSL_FAIL("control access failed");
                        }

                        xFP->setTitle( SW_RESSTR( nStrId ));
                        SvtPathOptions aPathOpt;
                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );
                        if( ERRCODE_NONE == aDlgHelper.Execute())
                        {
                            aFileName = xFP->getSelectedFiles().getConstArray()[0];
                            Any aTemplateValue = xCtrlAcc->getValue(
                                ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::GET_SELECTED_ITEM );
                            OUString sTmpl;
                            aTemplateValue >>= sTmpl;

                            sal_Int32 nColonPos = sTmpl.indexOf( ':' );
                            OUString sPrefix = sTmpl.copy( 0L, nColonPos );
                            if ( sPrefix == "Style" )
                            {
                                aTemplateName = sTmpl.copy( 7L );   //get string behind "Style: "
                            }
                            else if ( sPrefix == "Outline" )
                            {
                                nTemplateOutlineLevel = ( sTmpl.copy( 15L )).toInt32(); //get string behind "Outline: Leve  ";
                                bCreateByOutlineLevel = true;
                            }

                            if ( !aFileName.isEmpty() )
                            {
                                rReq.AppendItem( SfxStringItem( nWhich, aFileName ) );
                                if( !aTemplateName.isEmpty() )
                                    rReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, aTemplateName ) );
                            }
                        }
                    }
                }

                if( !aFileName.isEmpty() )
                {
                    if( PrepareClose( false ) )
                    {
                        SwWait aWait( *this, true );

                        if ( bCreateByOutlineLevel )
                        {
                            bDone = bCreateHtml
                                ? m_pDoc->GenerateHTMLDoc( aFileName, nTemplateOutlineLevel )
                                : m_pDoc->GenerateGlobalDoc( aFileName, nTemplateOutlineLevel );
                        }
                        else
                        {
                            const SwTextFormatColl* pSplitColl = nullptr;
                            if ( !aTemplateName.isEmpty() )
                                pSplitColl = m_pDoc->FindTextFormatCollByName(aTemplateName);
                            bDone = bCreateHtml
                                ? m_pDoc->GenerateHTMLDoc( aFileName, pSplitColl )
                                : m_pDoc->GenerateGlobalDoc( aFileName, pSplitColl );
                        }
                        if( bDone )
                        {
                            SfxStringItem aName( SID_FILE_NAME, aFileName );
                            SfxStringItem aReferer( SID_REFERER, aEmptyOUStr );
                            SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                            while(pViewShell)
                            {
                                //search for the view that created the call
                                if(pViewShell->GetObjectShell() == this && pViewShell->GetDispatcher())
                                {
                                    std::unique_ptr<SfxFrameItem> pFrameItem(new SfxFrameItem( SID_DOCFRAME,
                                                        pViewShell->GetViewFrame() ));
                                    SfxDispatcher* pDispatch = pViewShell->GetDispatcher();
                                    pDispatch->ExecuteList(SID_OPENDOC,
                                        SfxCallMode::ASYNCHRON,
                                        { &aName, &aReferer, pFrameItem.get() });
                                    break;
                                }
                                pViewShell = SfxViewShell::GetNext(*pViewShell);
                            }
                        }
                    }
                    if( !bDone && !rReq.IsAPI() )
                    {
                        ScopedVclPtrInstance<InfoBox>(nullptr, SW_RESSTR( STR_CANTCREATE))->Execute();
                    }
                }
            }
            rReq.SetReturnValue(SfxBoolItem( nWhich, bDone ));
            if (bDone)
                rReq.Done();
            else
                rReq.Ignore();
            break;

        case SID_ATTR_YEAR2000:
            if ( pArgs && SfxItemState::SET == pArgs->GetItemState( nWhich , false, &pItem ))
            {
                OSL_ENSURE(dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "wrong Item");
                sal_uInt16 nYear2K = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                // iterate over Views and put the State to FormShells

                SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( this );
                SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : nullptr;
                SwView* pCurrView = dynamic_cast< SwView* >( pViewShell );
                while(pCurrView)
                {
                    FmFormShell* pFormShell = pCurrView->GetFormShell();
                    if(pFormShell)
                        pFormShell->SetY2KState(nYear2K);
                    pVFrame = SfxViewFrame::GetNext( *pVFrame, this );
                    pViewShell = pVFrame ? pVFrame->GetViewShell() : nullptr;
                    pCurrView = dynamic_cast<SwView*>( pViewShell );
                }
                m_pDoc->GetNumberFormatter()->SetYear2000(nYear2K);
            }
        break;
        case FN_OPEN_FILE:
        {
            SfxViewShell* pViewShell = GetView();
            if (!pViewShell)
                pViewShell = SfxViewShell::Current();

            if (!pViewShell)
                // Ok.  I did my best.
                break;

            SfxStringItem aApp(SID_DOC_SERVICE, OUString("com.sun.star.text.TextDocument"));
            SfxStringItem aTarget(SID_TARGETNAME, OUString("_blank"));
            pViewShell->GetDispatcher()->ExecuteList(SID_OPENDOC,
                SfxCallMode::API|SfxCallMode::SYNCHRON,
                { &aApp, &aTarget });
        }
        break;
        case SID_CLASSIFICATION_APPLY:
        {
            if (pArgs && pArgs->GetItemState(nWhich, false, &pItem) == SfxItemState::SET)
            {
                SwWrtShell* pSh = GetWrtShell();
                const OUString& rValue = static_cast<const SfxStringItem*>(pItem)->GetValue();
                auto eType = SfxClassificationPolicyType::IntellectualProperty;
                if (pArgs->GetItemState(SID_TYPE_NAME, false, &pItem) == SfxItemState::SET)
                {
                    const OUString& rType = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    eType = SfxClassificationHelper::stringToPolicyType(rType);
                }
                pSh->SetClassification(rValue, eType);
            }
            else
                SAL_WARN("sw.ui", "missing parameter for SID_CLASSIFICATION_APPLY");
        }
        break;
        case SID_NOTEBOOKBAR:
        {
            const SfxStringItem* pFile = rReq.GetArg<SfxStringItem>( SID_NOTEBOOKBAR );
            SfxViewShell* pViewShell = GetView()? GetView(): SfxViewShell::Current();
            SfxBindings& rBindings( pViewShell->GetViewFrame()->GetBindings() );

            if ( SfxNotebookBar::IsActive() )
                sfx2::SfxNotebookBar::ExecMethod( rBindings, pFile ? pFile->GetValue() : "" );
            else
            {
                sfx2::SfxNotebookBar::CloseMethod( rBindings );
            }
        }
        break;

        default: OSL_FAIL("wrong Dispatcher");
    }
}

#if defined(_WIN32)
bool SwDocShell::DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             uno::Any & rValue )
{
    return m_pDoc->getIDocumentLinksAdministration().GetData( rItem, rMimeType, rValue );
}

bool SwDocShell::DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const uno::Any & rValue )
{
    return m_pDoc->getIDocumentLinksAdministration().SetData( rItem, rMimeType, rValue );
}

#endif

::sfx2::SvLinkSource* SwDocShell::DdeCreateLinkSource( const OUString& rItem )
{
    return m_pDoc->getIDocumentLinksAdministration().CreateLinkSource( rItem );
}

void SwDocShell::ReconnectDdeLink(SfxObjectShell& rServer)
{
    if (m_pDoc)
    {
        ::sfx2::LinkManager& rLinkManager = m_pDoc->getIDocumentLinksAdministration().GetLinkManager();
        rLinkManager.ReconnectDdeLink(rServer);
    }
}

void SwDocShell::FillClass( SvGlobalName * pClassName,
                                   SotClipboardFormatId * pClipFormat,
                                   OUString * /*pAppName*/,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nVersion,
                                   bool bTemplate /* = false */) const
{
    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = SotClipboardFormatId::STARWRITER_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = bTemplate ? SotClipboardFormatId::STARWRITER_8_TEMPLATE : SotClipboardFormatId::STARWRITER_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
// #FIXME check with new Event handling
#if 0
    uno::Reference< document::XVbaEventsHelper > xVbaEventsHelper = m_pDoc->GetVbaEventsHelper();
    if( xVbaEventsHelper.is() )
        lcl_processCompatibleSfxHint( xVbaEventsHelper, rHint );
#endif

    *pUserName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
}

void SwDocShell::SetModified( bool bSet )
{
    if (utl::ConfigManager::IsAvoidConfig())
        return;
    SfxObjectShell::SetModified( bSet );
    if( IsEnableSetModified())
    {
         if (!m_pDoc->getIDocumentState().IsInCallModified())
         {
            EnableSetModified( false );
            if( bSet )
            {
                bool const bOld = m_pDoc->getIDocumentState().IsModified();
                m_pDoc->getIDocumentState().SetModified();
                if( !bOld )
                {
                    m_pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
                }
            }
            else
                m_pDoc->getIDocumentState().ResetModified();

            EnableSetModified();
         }

        UpdateChildWindows();
        Broadcast(SfxHint(SfxHintId::DocChanged));
    }
}

void SwDocShell::UpdateChildWindows()
{
    // if necessary newly initialize Fielddlg (i.e. for TYP_SETVAR)
    if(!GetView())
        return;
    SfxViewFrame* pVFrame = GetView()->GetViewFrame();
    SwFieldDlgWrapper *pWrp = static_cast<SwFieldDlgWrapper*>(pVFrame->
            GetChildWindow( SwFieldDlgWrapper::GetChildWindowId() ));
    if( pWrp )
        pWrp->ReInitDlg( this );

    // if necessary newly initialize RedlineDlg
    SwRedlineAcceptChild *pRed = static_cast<SwRedlineAcceptChild*>(pVFrame->
            GetChildWindow( SwRedlineAcceptChild::GetChildWindowId() ));
    if( pRed )
        pRed->ReInitDlg( this );
}

// #i48748#
class SwReloadFromHtmlReader : public SwReader
{
    public:
        SwReloadFromHtmlReader( SfxMedium& _rTmpMedium,
                                const OUString& _rFilename,
                                SwDoc* _pDoc )
            : SwReader( _rTmpMedium, _rFilename, _pDoc )
        {
            SetBaseURL( _rFilename );
        }
};

void SwDocShell::ReloadFromHtml( const OUString& rStreamName, SwSrcView* pSrcView )
{
    bool bModified = IsModified();

    // The HTTP-Header fields have to be removed, otherwise
    // there are some from Meta-Tags dublicated or triplicated afterwards.
    ClearHeaderAttributesForSourceViewHack();

#if HAVE_FEATURE_SCRIPTING
    // The Document-Basic also bites the dust ...
    // A EnterBasicCall is not needed here, because nothing is called and
    // there can't be any Dok-Basic, that has not yet been loaded inside
    // of an HTML document.
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    //#59620# HasBasic() shows, that there already is a BasicManager at the DocShell.
    //          That was always generated in HTML-Import, when there are
    //          Macros in the source code.
    if( rHtmlOptions.IsStarBasic() && HasBasic())
    {
        BasicManager *pBasicMan = GetBasicManager();
        if( pBasicMan && (pBasicMan != SfxApplication::GetBasicManager()) )
        {
            sal_uInt16 nLibCount = pBasicMan->GetLibCount();
            while( nLibCount )
            {
                StarBASIC *pBasic = pBasicMan->GetLib( --nLibCount );
                if( pBasic )
                {
                    // Notify the IDE
                    SfxUsrAnyItem aShellItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( GetModel() ) );
                    OUString aLibName( pBasic->GetName() );
                    SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
                    pSrcView->GetViewFrame()->GetDispatcher()->ExecuteList(
                                            SID_BASICIDE_LIBREMOVED,
                                            SfxCallMode::SYNCHRON,
                                            { &aShellItem, &aLibNameItem });

                    // Only the modules are deleted from the standard-lib
                    if( nLibCount )
                        pBasicMan->RemoveLib( nLibCount, true );
                    else
                        pBasic->Clear();
                }
            }

            OSL_ENSURE( pBasicMan->GetLibCount() <= 1,
                    "Deleting Basics didn't work" );
        }
    }
#endif
    bool bWasBrowseMode = m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE);
    RemoveLink();

    // now also the UNO-Model has to be informed about the new Doc #51535#
    uno::Reference<text::XTextDocument> xDoc(GetBaseModel(), uno::UNO_QUERY);
    text::XTextDocument* pxDoc = xDoc.get();
    static_cast<SwXTextDocument*>(pxDoc)->InitNewDoc();

    AddLink();
    //#116402# update font list when new document is created
    UpdateFontList();
    m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, bWasBrowseMode);
    pSrcView->SetPool(&GetPool());

    const OUString& rMedname = GetMedium()->GetName();

    // The HTML template still has to be set
    SetHTMLTemplate( *GetDoc() );   //Styles from HTML.vor

    SfxViewShell* pViewShell = GetView() ? static_cast<SfxViewShell*>(GetView())
                                         : SfxViewShell::Current();
    SfxViewFrame*  pViewFrame = pViewShell->GetViewFrame();
    pViewFrame->GetDispatcher()->Execute( SID_VIEWSHELL0, SfxCallMode::SYNCHRON );

    SubInitNew();

    SfxMedium aMed( rStreamName, StreamMode::READ );
    // #i48748# - use class <SwReloadFromHtmlReader>, because
    // the base URL has to be set to the filename of the document <rMedname>
    // and not to the base URL of the temporary file <aMed> in order to get
    // the URLs of the linked graphics correctly resolved.
    SwReloadFromHtmlReader aReader( aMed, rMedname, m_pDoc );

    aReader.Read( *ReadHTML );

    const SwView* pCurrView = GetView();
    //in print layout the first page(s) may have been formatted as a mix of browse
    //and print layout
    if(!bWasBrowseMode && pCurrView)
    {
        SwWrtShell& rWrtSh = pCurrView->GetWrtShell();
        if( rWrtSh.GetLayout())
            rWrtSh.InvalidateLayout( true );
    }

    // Take HTTP-Header-Attibutes over into the DokInfo again.
    // The Base-URL doesn't matter here because TLX uses the one from the document
    // for absolutization.
    SetHeaderAttributesForSourceViewHack();

    if(bModified && !IsReadOnly())
        SetModified();
    else
        m_pDoc->getIDocumentState().ResetModified();
}

sal_uLong SwDocShell::LoadStylesFromFile( const OUString& rURL,
                    SwgReaderOption& rOpt, bool bUnoCall )
{
    sal_uLong nErr = 0;

    // Set filter:
    OUString sFactory(OUString::createFromAscii(SwDocShell::Factory().GetShortName()));
    SfxFilterMatcher aMatcher( sFactory );

    // search for filter in WebDocShell, too
    SfxMedium aMed( rURL, StreamMode::STD_READ );
    std::shared_ptr<const SfxFilter> pFlt;
    aMatcher.DetectFilter( aMed, pFlt );
    if(!pFlt)
    {
        OUString sWebFactory(OUString::createFromAscii(SwWebDocShell::Factory().GetShortName()));
        SfxFilterMatcher aWebMatcher( sWebFactory );
        aWebMatcher.DetectFilter( aMed, pFlt );
    }
    // --> OD #i117339# - trigger import only for own formats
    bool bImport( false );
    {
        if ( aMed.IsStorage() )
        {
            // As <SfxMedium.GetFilter().IsOwnFormat() resp. IsOwnTemplateFormat()
            // does not work correct (e.g., MS Word 2007 XML Template),
            // use workaround provided by MAV.
            uno::Reference< embed::XStorage > xStorage = aMed.GetStorage();
            if ( xStorage.is() )
            {
                // use <try-catch> on retrieving <MediaType> in order to check,
                // if the storage is one of our own ones.
                try
                {
                    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY_THROW );
                    const OUString aMediaTypePropName( "MediaType" );
                    xProps->getPropertyValue( aMediaTypePropName );
                    bImport = true;
                }
                catch (const uno::Exception&)
                {
                    bImport = false;
                }
            }
        }
    }
    if ( bImport )
    {
        SwRead pRead =  ReadXML;
        std::unique_ptr<SwReader> pReader;
        std::unique_ptr<SwPaM> pPam;
        // the SW3IO - Reader need the pam/wrtshell, because only then he
        // insert the styles!
        if( bUnoCall )
        {
            SwNodeIndex aIdx( m_pDoc->GetNodes().GetEndOfContent(), -1 );
            pPam.reset(new SwPaM( aIdx ));
            pReader.reset(new SwReader( aMed, rURL, *pPam ));
        }
        else
        {
            pReader.reset(new SwReader( aMed, rURL, *m_pWrtShell->GetCursor() ));
        }

        pRead->GetReaderOpt().SetTextFormats( rOpt.IsTextFormats() );
        pRead->GetReaderOpt().SetFrameFormats( rOpt.IsFrameFormats() );
        pRead->GetReaderOpt().SetPageDescs( rOpt.IsPageDescs() );
        pRead->GetReaderOpt().SetNumRules( rOpt.IsNumRules() );
        pRead->GetReaderOpt().SetMerge( rOpt.IsMerge() );

        if( bUnoCall )
        {
            UnoActionContext aAction( m_pDoc );
            nErr = pReader->Read( *pRead );
        }
        else
        {
            m_pWrtShell->StartAllAction();
            nErr = pReader->Read( *pRead );
            m_pWrtShell->EndAllAction();
        }
    }

    return nErr;
}

// Get a client for an embedded object if possible.
SfxInPlaceClient* SwDocShell::GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef )
{
    SfxInPlaceClient* pResult = nullptr;

    SwWrtShell* pShell = GetWrtShell();
    if ( pShell )
    {
        pResult = pShell->GetView().FindIPClient( xObjRef.GetObject(), &pShell->GetView().GetEditWin() );
        if ( !pResult )
            pResult = new SwOleClient( &pShell->GetView(), &pShell->GetView().GetEditWin(), xObjRef );
    }

    return pResult;
}

static bool lcl_MergePortions(SwNode *const& pNode, void *)
{
    if (pNode->IsTextNode())
    {
        pNode->GetTextNode()->FileLoadedInitHints();
    }
    return true;
}

int SwFindDocShell( SfxObjectShellRef& xDocSh,
                    SfxObjectShellLock& xLockRef,
                    const OUString& rFileName,
                    const OUString& rPasswd,
                    const OUString& rFilter,
                    sal_Int16 nVersion,
                    SwDocShell* pDestSh )
{
    if ( rFileName.isEmpty() )
        return 0;

    // 1. Does the file already exist in the list of all Documents?
    INetURLObject aTmpObj( rFileName );
    aTmpObj.SetMark( OUString() );

    // Iterate over the DocShell and get the ones with the name

    SfxObjectShell* pShell = pDestSh;
    bool bFirst = nullptr != pShell;

    if( !bFirst )
        // No DocShell passed, starting with the first from the DocShell list
        pShell = SfxObjectShell::GetFirst( checkSfxObjectShell<SwDocShell> );

    while( pShell )
    {
        // We want this one
        SfxMedium* pMed = pShell->GetMedium();
        if( pMed && pMed->GetURLObject() == aTmpObj )
        {
            const SfxPoolItem* pItem;
            if( ( SfxItemState::SET == pMed->GetItemSet()->GetItemState(
                                            SID_VERSION, false, &pItem ) )
                    ? (nVersion == static_cast<const SfxInt16Item*>(pItem)->GetValue())
                    : !nVersion )
            {
                // Found, thus return
                xDocSh = pShell;
                return 1;
            }
        }

        if( bFirst )
        {
            bFirst = false;
            pShell = SfxObjectShell::GetFirst( checkSfxObjectShell<SwDocShell> );
        }
        else
            pShell = SfxObjectShell::GetNext( *pShell, checkSfxObjectShell<SwDocShell> );
    }

    // 2. Open the file ourselves
    std::unique_ptr<SfxMedium> xMed(new SfxMedium( aTmpObj.GetMainURL(
                             INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));
    if( INetProtocol::File == aTmpObj.GetProtocol() )
        xMed->Download(); // Touch the medium (download it)

    std::shared_ptr<const SfxFilter> pSfxFlt;
    if (!xMed->GetError())
    {
        SfxFilterMatcher aMatcher( rFilter == "writerglobal8"
            ? OUString::createFromAscii(SwGlobalDocShell::Factory().GetShortName())
            : OUString::createFromAscii(SwDocShell::Factory().GetShortName()) );

        // No Filter, so search for it. Else test if the one passed is a valid one
        if( !rFilter.isEmpty() )
        {
            pSfxFlt = aMatcher.GetFilter4FilterName( rFilter );
        }

        if( nVersion )
            xMed->GetItemSet()->Put( SfxInt16Item( SID_VERSION, nVersion ));

        if( !rPasswd.isEmpty() )
            xMed->GetItemSet()->Put( SfxStringItem( SID_PASSWORD, rPasswd ));

        if( !pSfxFlt )
            aMatcher.DetectFilter( *xMed, pSfxFlt );

        if( pSfxFlt )
        {
            // We cannot do anything without a Filter
            xMed->SetFilter( pSfxFlt );

            // If the new shell is created, SfxObjectShellLock should be used to let it be closed later for sure
            SwDocShell *const pNew(new SwDocShell(SfxObjectCreateMode::INTERNAL));
            xLockRef = pNew;
            xDocSh = static_cast<SfxObjectShell*>(xLockRef);
            if (xDocSh->DoLoad(xMed.release()))
            {
                SwDoc const& rDoc(*pNew->GetDoc());
                const_cast<SwDoc&>(rDoc).GetNodes().ForEach(&lcl_MergePortions);
                return 2;
            }
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
