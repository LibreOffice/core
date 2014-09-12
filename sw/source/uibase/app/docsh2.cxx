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

#include <com/sun/star/drawing/ModuleDispatcher.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <comphelper/processfactory.hxx>

#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
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
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
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

#include <sfx2/fcontnr.hxx>

#include "dialog.hrc"
#include "swabstdlg.hxx"

#include <ndtxt.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::sfx2;

// create DocInfo (virtual)
SfxDocumentInfoDialog* SwDocShell::CreateDocumentInfoDialog(
                                Window *pParent, const SfxItemSet &rSet)
{
    SfxDocumentInfoDialog* pDlg = new SfxDocumentInfoDialog(pParent, rSet);
    //only with statistics, when this document is being shown, not
    //from within the Doc-Manager
    SwDocShell* pDocSh = (SwDocShell*) SfxObjectShell::Current();
    if( pDocSh == this )
    {
        //Not for SourceView.
        SfxViewShell *pVSh = SfxViewShell::Current();
        if ( pVSh && !pVSh->ISA(SwSrcView) )
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
            pDlg->AddFontTabPage();
            pDlg->AddTabPage(TP_DOC_STAT, SW_RESSTR(STR_DOC_STAT),pFact->GetTabPageCreatorFunc( TP_DOC_STAT ),0);
        }
    }
    return pDlg;
}

// Disable "multiple layout"

void SwDocShell::ToggleBrowserMode(bool bSet, SwView* _pView )
{
    GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bSet );
    UpdateFontList();
    SwView* pTempView = _pView ? _pView : (SwView*)GetView();
    if( pTempView )
    {
        pTempView->GetViewFrame()->GetBindings().Invalidate(FN_SHADOWCURSOR);
        if( !GetDoc()->getIDocumentDeviceAccess().getPrinter( false ) )
            pTempView->SetPrinter( GetDoc()->getIDocumentDeviceAccess().getPrinter( false ), SFX_PRINTER_PRINTER | SFX_PRINTER_JOBSETUP );
        GetDoc()->CheckDefaultPageFmt();
        SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this, false);
        while (pTmpFrm)
        {
            if( pTmpFrm != pTempView->GetViewFrame() )
            {
                pTmpFrm->DoClose();
                pTmpFrm = SfxViewFrame::GetFirst(this, false);
            }
            else
                pTmpFrm = SfxViewFrame::GetNext(*pTmpFrm, this, false);
        }
        const SwViewOption& rViewOptions = *pTempView->GetWrtShell().GetViewOptions();
        pTempView->GetWrtShell().CheckBrowseView( true );
        pTempView->CheckVisArea();
        if( bSet )
        {
            const SvxZoomType eType = (SvxZoomType)rViewOptions.GetZoomType();
            if ( SVX_ZOOM_PERCENT != eType)
                ((SwView*)GetView())->SetZoom( eType );
        }
        pTempView->InvalidateBorder();
        pTempView->SetNewWindowAllowed(!bSet);
    }
}
// End of disabled "multiple layout"

// update text fields on document properties changes
void SwDocShell::DoFlushDocInfo()
{
    if ( !mpDoc ) return;

    bool bUnlockView(true);
    if ( mpWrtShell ) {
        bUnlockView = !mpWrtShell->IsViewLocked();
        mpWrtShell->LockView( true );    // lock visible section
        mpWrtShell->StartAllAction();
    }

    mpDoc->getIDocumentStatistics().DocInfoChgd();

    if ( mpWrtShell ) {
        mpWrtShell->EndAllAction();
        if ( bUnlockView ) {
            mpWrtShell->LockView( false );
        }
    }
}

static void lcl_processCompatibleSfxHint( const uno::Reference< script::vba::XVBAEventProcessor >& xVbaEvents, const SfxHint& rHint )
{
    using namespace com::sun::star::script::vba::VBAEventId;
    if ( dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        uno::Sequence< uno::Any > aArgs;
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch( nEventId )
        {
            case SFX_EVENT_CREATEDOC:
                xVbaEvents->processVbaEvent( DOCUMENT_NEW, aArgs );
            break;
            case SFX_EVENT_OPENDOC:
                xVbaEvents->processVbaEvent( DOCUMENT_OPEN, aArgs );
            break;
        }
    }
}

// Notification on DocInfo changes
void SwDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( !mpDoc )
    {
        return ;
    }

    uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = mpDoc->GetVbaEventProcessor();
    if( xVbaEvents.is() )
        lcl_processCompatibleSfxHint( xVbaEvents, rHint );

    sal_uInt16 nAction = 0;
    if( dynamic_cast<const SfxSimpleHint*>(&rHint) )
    {
        // switch for more actions
        switch( ((SfxSimpleHint&) rHint).GetId() )
        {
            case SFX_HINT_TITLECHANGED:
                if( GetMedium() )
                    nAction = 2;
            break;
        }
    }
    else if( dynamic_cast<const SfxEventHint*>(&rHint) &&
        ((SfxEventHint&) rHint).GetEventId() == SFX_EVENT_LOADFINISHED )
    {
        // #i38126# - own action id
        nAction = 3;
    }

    if( nAction )
    {
        bool bUnlockView = true; //initializing prevents warning
        if( mpWrtShell )
        {
            bUnlockView = !mpWrtShell->IsViewLocked();
            mpWrtShell->LockView( true );    //lock visible section
            mpWrtShell->StartAllAction();
        }
        switch( nAction )
        {
        case 2:
            mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_FILENAMEFLD )->UpdateFlds();
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
                const bool bIsDocModified = mpDoc->getIDocumentState().IsModified();

                mpDoc->getIDocumentStatistics().DocInfoChgd( );

                // #i41679#
                if ( !bIsDocModified )
                    mpDoc->getIDocumentState().ResetModified();
                if ( bResetModified )
                    EnableSetModified( true );
            }
            break;
        }

        if( mpWrtShell )
        {
            mpWrtShell->EndAllAction();
            if( bUnlockView )
                mpWrtShell->LockView( false );
        }
    }
}

// Notification Close Doc
bool SwDocShell::PrepareClose( bool bUI )
{
    bool nRet = SfxObjectShell::PrepareClose( bUI );

    if( nRet )
        EndListening( *this );

    if( mpDoc && IsInPrepareClose() )
    {
        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = mpDoc->GetVbaEventProcessor();
        if( xVbaEvents.is() )
        {
            using namespace com::sun::star::script::vba::VBAEventId;
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( DOCUMENT_CLOSE, aArgs );
        }
    }
    return nRet;
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
            SvxSwAutoFmtFlags* pAFlags = &SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
            SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

            bool bOldLocked = rACW.IsLockWordLstLocked(),
                 bOldAutoCmpltCollectWords = pAFlags->bAutoCmpltCollectWords;

            rACW.SetLockWordLstLocked( true );

            editeng::SortedAutoCompleteStrings aTmpLst( rACW.GetWordList() );
            pAFlags->m_pAutoCompleteList = &aTmpLst;

            SfxApplication* pApp = SfxGetpApp();
            SfxRequest aAppReq(SID_AUTO_CORRECT_DLG, SFX_CALLMODE_SYNCHRON, pApp->GetPool());
            SfxBoolItem aSwOptions( SID_AUTO_CORRECT_DLG, true );
            aAppReq.AppendItem(aSwOptions);

            pAFlags->pSmartTagMgr = &SwSmartTagMgr::Get();

            SfxItemSet aSet( pApp->GetPool(), SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG, SID_OPEN_SMARTTAGOPTIONS, SID_OPEN_SMARTTAGOPTIONS, 0 );
            aSet.Put( aSwOptions );

            const SfxPoolItem* pOpenSmartTagOptionsItem = 0;
            if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_OPEN_SMARTTAGOPTIONS, false, &pOpenSmartTagOptionsItem ) )
                aSet.Put( *static_cast<const SfxBoolItem*>(pOpenSmartTagOptionsItem) );

            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
              SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog( RID_OFA_AUTOCORR_DLG, NULL, &aSet, NULL );
              pDlg->Execute();
              delete pDlg;

            rACW.SetLockWordLstLocked( bOldLocked );

            SwEditShell::SetAutoFmtFlags( pAFlags );
            rACW.SetMinWordLen( pAFlags->nAutoCmpltWordLen );
            rACW.SetMaxCount( pAFlags->nAutoCmpltListLen );
            if (pAFlags->m_pAutoCompleteList)  // any changes?
            {
                rACW.CheckChangedList( aTmpLst );
                // clear the temp WordList pointer
                pAFlags->m_pAutoCompleteList = 0;
            }
            // remove all pointer we never delete the strings
            aTmpLst.clear();

            if( !bOldAutoCmpltCollectWords && bOldAutoCmpltCollectWords !=
                pAFlags->bAutoCmpltCollectWords )
            {
                // call on all Docs the idle formatter to start
                // the collection of Words
                TypeId aType = TYPE(SwDocShell);
                for( SwDocShell *pDocSh = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
                     pDocSh;
                     pDocSh = (SwDocShell*)SfxObjectShell::GetNext( *pDocSh, &aType ) )
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
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                SfxViewShell* pViewShell = SfxViewShell::Current();
                SwView* pCurrView = dynamic_cast< SwView *> ( pViewShell );
                bool bCurrent = IS_TYPE( SwPagePreview, pViewShell );

                while( pTmpFrm )    // search Preview
                {
                    if( IS_TYPE( SwView, pTmpFrm->GetViewShell()) )
                        bOnly = false;
                    else if( IS_TYPE( SwPagePreview, pTmpFrm->GetViewShell()))
                    {
                        pTmpFrm->GetFrame().Appear();
                        bFound = true;
                    }
                    if( bFound && !bOnly )
                        break;
                    pTmpFrm = SfxViewFrame::GetNext(*pTmpFrm, this);
                }

                if( pArgs && SfxItemState::SET ==
                    pArgs->GetItemState( SID_PRINTPREVIEW, false, &pItem ))
                    bSet = ((SfxBoolItem*)pItem)->GetValue();
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
                    if( ISA(SwWebDocShell) && SID_VIEWSHELL1 == nSlotId )
                        nSlotId = SID_VIEWSHELL2;

                    if( pCurrView && pCurrView->GetDocShell() == this )
                        pTmpFrm = pCurrView->GetViewFrame();
                    else
                        pTmpFrm = SfxViewFrame::GetFirst( this );

                    if (pTmpFrm)
                        pTmpFrm->GetDispatcher()->Execute( nSlotId, 0, 0, SFX_CALLMODE_ASYNCHRON );
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

                sal_uInt16 nFlags = bFrame ? SFX_LOAD_FRAME_STYLES : 0;
                if(bPage)
                    nFlags|= SFX_LOAD_PAGE_STYLES;
                if(bNum)
                    nFlags|= SFX_LOAD_NUM_STYLES;
                if(!nFlags || bText)
                    nFlags|= SFX_LOAD_TEXT_STYLES;
                if(bMerge)
                    nFlags|= SFX_MERGE_STYLES;

                if ( pArgs )
                {
                    SFX_REQUEST_ARG( rReq, pTemplateItem, SfxStringItem, SID_TEMPLATE_NAME, false );
                    if ( pTemplateItem )
                    {
                        aFileName = pTemplateItem->GetValue();
                        SFX_REQUEST_ARG( rReq, pFlagsItem, SfxInt32Item, SID_TEMPLATE_LOAD, false );
                        if ( pFlagsItem )
                            nFlags = (sal_uInt16) pFlagsItem->GetValue();
                    }
                }

                if ( aFileName.isEmpty() )
                {
                    SvtPathOptions aPathOpt;
                    boost::scoped_ptr<SfxNewFileDialog> pNewFileDlg(
                        new SfxNewFileDialog(&GetView()->GetViewFrame()->GetWindow(), SFXWB_LOAD_TEMPLATE));
                    pNewFileDlg->SetTemplateFlags(nFlags);

                    nRet = pNewFileDlg->Execute();
                    if(RET_TEMPLATE_LOAD == nRet)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
                        uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );

                        SfxObjectFactory &rFact = GetFactory();
                        SfxFilterMatcher aMatcher( OUString::createFromAscii(rFact.GetShortName()) );
                        SfxFilterMatcherIter aIter( aMatcher );
                        uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
                        const SfxFilter* pFlt = aIter.First();
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
                        bool bWeb = 0 != dynamic_cast< SwWebDocShell *>( this );
                        const SfxFilter *pOwnFlt =
                                SwDocShell::Factory().GetFilterContainer()->
                                GetFilter4FilterName(OUString("writer8"));

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
                            aFileName = xFP->getFiles().getConstArray()[0];
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
                    aOpt.SetTxtFmts(    bText = (0 != (nFlags&SFX_LOAD_TEXT_STYLES) ));
                    aOpt.SetFrmFmts(    bFrame = (0 != (nFlags&SFX_LOAD_FRAME_STYLES)));
                    aOpt.SetPageDescs(  bPage = (0 != (nFlags&SFX_LOAD_PAGE_STYLES )));
                    aOpt.SetNumRules(   bNum = (0 != (nFlags&SFX_LOAD_NUM_STYLES  )));
                    //different meaning between SFX_MERGE_STYLES and aOpt.SetMerge!
                    bMerge = 0 != (nFlags&SFX_MERGE_STYLES);
                    aOpt.SetMerge( !bMerge );

                    SetError( LoadStylesFromFile( aFileName, aOpt, false ), OUString( OSL_LOG_PREFIX ));
                    if ( !GetError() )
                        rReq.Done();
                }
            }
            break;
            case SID_SOURCEVIEW:
            {
                SfxViewShell* pViewShell = GetView()
                                            ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
                SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
                SwSrcView* pSrcView = dynamic_cast< SwSrcView *>( pViewShell );
                if(!pSrcView)
                {
                    // 3 possible state:
                    // 1 - file unsaved -> save as HTML
                    // 2 - file modified and HTML filter active -> save
                    // 3 - file saved in non-HTML -> QueryBox to save as HTML
                    const SfxFilter* pHtmlFlt =
                                    SwIoSystem::GetFilterOfFormat(
                                        OUString("HTML"),
                                        SwWebDocShell::Factory().GetFilterContainer() );
                    bool bLocalHasName = HasName();
                    if(bLocalHasName)
                    {
                        //check for filter type
                        const SfxFilter* pFlt = GetMedium()->GetFilter();
                        if(!pFlt || pFlt->GetUserData() != pHtmlFlt->GetUserData())
                        {
                            MessageDialog aQuery(&pViewFrm->GetWindow(),
                                "SaveAsHTMLDialog", "modules/swriter/ui/saveashtmldialog.ui");

                            if(RET_YES == aQuery.Execute())
                                bLocalHasName = false;
                            else
                                break;
                        }
                    }
                    if(!bLocalHasName)
                    {
                        FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
                        aDlgHelper.AddFilter( pHtmlFlt->GetFilterName(), pHtmlFlt->GetDefaultExtension() );
                        aDlgHelper.SetCurrentFilter( pHtmlFlt->GetFilterName() );
                        if( ERRCODE_NONE != aDlgHelper.Execute())
                        {
                            break;
                        }
                        OUString sPath = aDlgHelper.GetPath();
                        SfxStringItem aName(SID_FILE_NAME, sPath);
                        SfxStringItem aFilter(SID_FILTER_NAME, pHtmlFlt->GetName());
                        const SfxBoolItem* pBool = (const SfxBoolItem*)
                                pViewFrm->GetDispatcher()->Execute(
                                        SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, &aName, &aFilter, 0L );
                        if(!pBool || !pBool->GetValue())
                            break;
                    }
                }

                OSL_ENSURE(dynamic_cast<SwWebDocShell*>(this),
                            "SourceView only in WebDocShell");

                // the SourceView is not the 1 for SwWebDocShell
                sal_uInt16 nSlot = SID_VIEWSHELL1;
                bool bSetModified = false;
                SfxPrinter* pSavePrinter = 0;
                if( 0 != pSrcView)
                {
                    SfxPrinter* pTemp = GetDoc()->getIDocumentDeviceAccess().getPrinter( false );
                    if(pTemp)
                        pSavePrinter = new SfxPrinter(*pTemp);
                    bSetModified = IsModified() || pSrcView->IsModified();
                    if(pSrcView->IsModified()||pSrcView->HasSourceSaved())
                    {
                        utl::TempFile aTempFile;
                        aTempFile.EnableKillingFile();
                        pSrcView->SaveContent(aTempFile.GetURL());
                        bDone = true;
                        SvxMacro aMac(aEmptyOUStr, aEmptyOUStr, STARBASIC);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_OPENDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_PREPARECLOSEDOC ), aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_ACTIVATEDOC ),     aMac, this);
                        SfxEventConfiguration::ConfigureEvent(GlobalEventConfig::GetEventName( STR_EVENT_DEACTIVATEDOC ), aMac, this);
                        ReloadFromHtml(aTempFile.GetURL(), pSrcView);
                        nSlot = 0;
                    }
                    else
                    {
                        nSlot = SID_VIEWSHELL0;
                    }
                }
                if(nSlot)
                    pViewFrm->GetDispatcher()->Execute(nSlot, SFX_CALLMODE_SYNCHRON);
                if(bSetModified)
                    GetDoc()->getIDocumentState().SetModified();
                if(pSavePrinter)
                {
                    GetDoc()->getIDocumentDeviceAccess().setPrinter( pSavePrinter, true, true);
                    //pSavePrinter must not be deleted again
                }
                pViewFrm->GetBindings().SetState(SfxBoolItem(SID_SOURCEVIEW, nSlot == SID_VIEWSHELL2));
                pViewFrm->GetBindings().Invalidate( SID_NEWWINDOW );
                pViewFrm->GetBindings().Invalidate( SID_BROWSER_MODE );
                pViewFrm->GetBindings().Invalidate( FN_PRINT_LAYOUT );
            }
            break;
            case SID_GET_COLORLIST:
            {
                SvxColorListItem* pColItem = (SvxColorListItem*)GetItem(SID_COLOR_TABLE);
                XColorListRef pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaRefItem<XColorList>(SID_GET_COLORLIST, pList));
            }
            break;
        case FN_ABSTRACT_STARIMPRESS:
        case FN_ABSTRACT_NEWDOC:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            boost::scoped_ptr<AbstractSwInsertAbstractDlg> pDlg(pFact->CreateSwInsertAbstractDlg(0));
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            if(RET_OK == pDlg->Execute())
            {
                sal_uInt8 nLevel = pDlg->GetLevel();
                sal_uInt8 nPara = pDlg->GetPara();
                SwDoc* pSmryDoc = new SwDoc();
                SfxObjectShellLock xDocSh( new SwDocShell( pSmryDoc, SFX_CREATE_MODE_STANDARD));
                xDocSh->DoInitNew( 0 );

                bool bImpress = FN_ABSTRACT_STARIMPRESS == nWhich;
                mpDoc->Summary( pSmryDoc, nLevel, nPara, bImpress );
                if( bImpress )
                {
                    WriterRef xWrt;
                    // mba: looks as if relative URLs don't make sense here
                    ::GetRTFWriter( aEmptyOUStr, OUString(), xWrt );
                    SvMemoryStream *pStrm = new SvMemoryStream();
                    pStrm->SetBufferSize( 16348 );
                    SwWriter aWrt( *pStrm, *pSmryDoc );
                    ErrCode eErr = aWrt.Write( xWrt );
                    if( !ERRCODE_TOERROR( eErr ) )
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
                            sal_uLong nRead = 0;
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
                    SwView      *pCurrView = (SwView*) pFrame->GetViewShell();

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
                if( !ERRCODE_TOERROR( eErr ) )
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
                            sal_uLong nRead = 0;
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
                        TransferDataContainer* pClipCntnr =
                                                    new TransferDataContainer;
                        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                                                        xRef( pClipCntnr );

                        pClipCntnr->CopyAnyData( FORMAT_RTF, (sal_Char*)
                                    pStrm->GetData(), pStrm->GetEndOfData() );
                        pClipCntnr->CopyToClipboard(
                            GetView()? (Window*)&GetView()->GetEditWin() : 0 );
                        delete pStrm;
                    }
                }
                else
                    ErrorHandler::HandleError(ErrCode( eErr ));
            }
            break;
            case SID_SPELLCHECKER_CHANGED:
                //! false, true, true is on the save side but a probably overdone
                SW_MOD()->CheckSpellChanges(false, true, true, false );
            break;

        case SID_MAIL_PREPAREEXPORT:
        {
            //pWrtShell is not set in page preview
            if(mpWrtShell)
                mpWrtShell->StartAllAction();
            mpDoc->getIDocumentFieldsAccess().UpdateFlds( NULL, false );
            mpDoc->getIDocumentLinksAdministration().EmbedAllLinks();
            mpDoc->RemoveInvisibleContent();
            if(mpWrtShell)
                mpWrtShell->EndAllAction();
        }
        break;

        case SID_MAIL_EXPORT_FINISHED:
        {
                if(mpWrtShell)
                    mpWrtShell->StartAllAction();
                //try to undo the removal of invisible content
                mpDoc->RestoreInvisibleContent();
                if(mpWrtShell)
                    mpWrtShell->EndAllAction();
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
                    aFileName = ((const SfxStringItem*)pItem)->GetValue();
                    SFX_ITEMSET_ARG( pArgs, pTemplItem, SfxStringItem, SID_TEMPLATE_NAME, false );
                    if ( pTemplItem )
                        aTemplateName = pTemplItem->GetValue();
                }
                bool bError = false;
                if ( aFileName.isEmpty() )
                {
                    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE, 0 );

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
                    uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

                    const SfxFilter* pFlt;
                    sal_uInt16 nStrId;

                    if( bCreateHtml )
                    {
                        // for HTML there is only one filter!!
                        pFlt = SwIoSystem::GetFilterOfFormat(
                                OUString("HTML"),
                                SwWebDocShell::Factory().GetFilterContainer() );
                        nStrId = STR_LOAD_HTML_DOC;
                    }
                    else
                    {
                        // for Global-documents we now only offer the current one.
                        pFlt = SwGlobalDocShell::Factory().GetFilterContainer()->
                                    GetFilter4Extension( OUString("odm")  );
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
                        const SwOutlineNodes& rOutlNds = mpDoc->GetNodes().GetOutLineNds();
                        if( !rOutlNds.empty() )
                        {
                            int nLevel;
                            for(sal_uInt16 n = 0; n < rOutlNds.size(); ++n )
                                if( ( nLevel = rOutlNds[n]->GetTxtNode()->GetAttrOutlineLevel()) > 0 &&
                                    ! bOutline[nLevel-1] )
                                {
                                    bOutline[nLevel-1] = true;
                                }
                        }

                        const sal_uInt16 nStyleCount = mpDoc->GetTxtFmtColls()->size();
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
                            SwTxtFmtColl &rTxtColl = *(*mpDoc->GetTxtFmtColls())[ i ];
                            if( !rTxtColl.IsDefault() && rTxtColl.IsAtDocNodeSet() )
                            {
                                pEntries[nIdx++] = sStyle + rTxtColl.GetName();
                            }
                        }

                        aListBoxEntries.realloc(nIdx);
                        sal_Int16 nSelect = 0;

                        try
                        {
                            Any aTemplates(&aListBoxEntries, ::getCppuType(&aListBoxEntries));

                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::ADD_ITEMS , aTemplates );
                            Any aSelectPos(&nSelect, ::getCppuType(&nSelect));
                            xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                ListboxControlActions::SET_SELECT_ITEM, aSelectPos );
                            xCtrlAcc->setLabel( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,
                                                    OUString(SW_RES( STR_FDLG_TEMPLATE_NAME )));
                        }
                        catch (const Exception&)
                        {
                            OSL_FAIL("control acces failed");
                        }

                        xFP->setTitle( SW_RESSTR( nStrId ));
                        SvtPathOptions aPathOpt;
                        xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );
                        if( ERRCODE_NONE == aDlgHelper.Execute())
                        {
                            aFileName = xFP->getFiles().getConstArray()[0];
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
                                ? mpDoc->GenerateHTMLDoc( aFileName, nTemplateOutlineLevel )
                                : mpDoc->GenerateGlobalDoc( aFileName, nTemplateOutlineLevel );
                        }
                        else
                        {
                            const SwTxtFmtColl* pSplitColl = 0;
                            if ( !aTemplateName.isEmpty() )
                                pSplitColl = mpDoc->FindTxtFmtCollByName(aTemplateName);
                            bDone = bCreateHtml
                                ? mpDoc->GenerateHTMLDoc( aFileName, pSplitColl )
                                : mpDoc->GenerateGlobalDoc( aFileName, pSplitColl );
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
                                    boost::scoped_ptr<SfxFrameItem> pFrameItem(new SfxFrameItem( SID_DOCFRAME,
                                                        pViewShell->GetViewFrame() ));
                                    SfxDispatcher* pDispatch = pViewShell->GetDispatcher();
                                    pDispatch->Execute(
                                            SID_OPENDOC,
                                            SFX_CALLMODE_ASYNCHRON,
                                            &aName,
                                            &aReferer,
                                            pFrameItem.get(), 0L );
                                    break;
                                }
                                pViewShell = SfxViewShell::GetNext(*pViewShell);
                            }
                        }
                    }
                    if( !bDone && !rReq.IsAPI() )
                    {
                        InfoBox( 0, SW_RESSTR( STR_CANTCREATE )).Execute();
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
                OSL_ENSURE(pItem->ISA(SfxUInt16Item), "wrong Item");
                sal_uInt16 nYear2K = ((SfxUInt16Item*)pItem)->GetValue();
                // iterate over Views and put the State to FormShells

                SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( this );
                SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                SwView* pCurrView = dynamic_cast< SwView* >( pViewShell );
                while(pCurrView)
                {
                    FmFormShell* pFormShell = pCurrView->GetFormShell();
                    if(pFormShell)
                        pFormShell->SetY2KState(nYear2K);
                    pVFrame = SfxViewFrame::GetNext( *pVFrame, this );
                    pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                    pCurrView = dynamic_cast<SwView*>( pViewShell );
                }
                mpDoc->GetNumberFormatter(true)->SetYear2000(nYear2K);
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
            pViewShell->GetDispatcher()->Execute(
                SID_OPENDOC, SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON, &aApp, &aTarget, 0L);
        }
        break;

        default: OSL_FAIL("wrong Dispatcher");
    }
}

 // #FIXME - align with NEW event stuff ( if possible )
#if 0
void lcl_processCompatibleSfxHint( const uno::Reference< document::XVbaEventsHelper >& xVbaEventsHelper, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxEventHint ) )
    {
        uno::Sequence< uno::Any > aArgs;
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch( nEventId )
        {
            case SFX_EVENT_CREATEDOC:
            {
                xVbaEventsHelper->ProcessCompatibleVbaEvent( VBAEVENT_DOCUMENT_NEW, aArgs );
                break;
            }
            case SFX_EVENT_OPENDOC:
            {
                xVbaEventsHelper->ProcessCompatibleVbaEvent( VBAEVENT_DOCUMENT_OPEN, aArgs );
                break;
            }
            default:
            {
                //do nothing
            }
        }
    }
}
#endif

bool SwDocShell::DdeGetData( const OUString& rItem, const OUString& rMimeType,
                             uno::Any & rValue )
{
    return mpDoc->getIDocumentLinksAdministration().GetData( rItem, rMimeType, rValue );
}

bool SwDocShell::DdeSetData( const OUString& rItem, const OUString& rMimeType,
                             const uno::Any & rValue )
{
    return mpDoc->getIDocumentLinksAdministration().SetData( rItem, rMimeType, rValue );
}

::sfx2::SvLinkSource* SwDocShell::DdeCreateLinkSource( const OUString& rItem )
{
    return mpDoc->getIDocumentLinksAdministration().CreateLinkSource( rItem );
}

void SwDocShell::ReconnectDdeLink(SfxObjectShell& rServer)
{
    if ( mpDoc ) {
        ::sfx2::LinkManager& rLinkManager = mpDoc->getIDocumentLinksAdministration().GetLinkManager();
        rLinkManager.ReconnectDdeLink(rServer);
    }
}

void SwDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   OUString * /*pAppName*/,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nVersion,
                                   bool bTemplate /* = false */) const
{
    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITER_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_60 );
        *pClipFormat    = bTemplate ? SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE : SOT_FORMATSTR_ID_STARWRITER_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
// #FIXME check with new Event handling
#if 0
    uno::Reference< document::XVbaEventsHelper > xVbaEventsHelper = mpDoc->GetVbaEventsHelper();
    if( xVbaEventsHelper.is() )
        lcl_processCompatibleSfxHint( xVbaEventsHelper, rHint );
#endif

    *pUserName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
}

void SwDocShell::SetModified( bool bSet )
{
    SfxObjectShell::SetModified( bSet );
    if( IsEnableSetModified())
    {
         if (!mpDoc->getIDocumentState().IsInCallModified() )
         {
            EnableSetModified( false );
            if( bSet )
            {
                bool bOld = mpDoc->getIDocumentState().IsModified();
                mpDoc->getIDocumentState().SetModified();
                if( !bOld )
                {
                    mpDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
                }
            }
            else
                mpDoc->getIDocumentState().ResetModified();

            EnableSetModified( true );
         }

        UpdateChildWindows();
        Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
    }
}

void SwDocShell::UpdateChildWindows()
{
    // if necessary newly initialize Flddlg (i.e. for TYP_SETVAR)
    if(!GetView())
        return;
    SfxViewFrame* pVFrame = GetView()->GetViewFrame();
    SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->
            GetChildWindow( SwFldDlgWrapper::GetChildWindowId() );
    if( pWrp )
        pWrp->ReInitDlg( this );

    // if necessary newly initialize RedlineDlg
    SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->
            GetChildWindow( SwRedlineAcceptChild::GetChildWindowId() );
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

#ifndef DISABLE_SCRIPTING
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
        if( pBasicMan && (pBasicMan != SfxGetpApp()->GetBasicManager()) )
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
                    pSrcView->GetViewFrame()->GetDispatcher()->Execute(
                                            SID_BASICIDE_LIBREMOVED,
                                            SFX_CALLMODE_SYNCHRON,
                                            &aShellItem, &aLibNameItem, 0L );

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
    bool bWasBrowseMode = mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::BROWSE_MODE);
    RemoveLink();

    // now also the UNO-Model has to be informed about the new Doc #51535#
    uno::Reference<text::XTextDocument> xDoc(GetBaseModel(), uno::UNO_QUERY);
    text::XTextDocument* pxDoc = xDoc.get();
    ((SwXTextDocument*)pxDoc)->InitNewDoc();

    AddLink();
    //#116402# update font list when new document is created
    UpdateFontList();
    mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bWasBrowseMode);
    pSrcView->SetPool(&GetPool());

    const OUString& rMedname = GetMedium()->GetName();

    // The HTML template still has to be set
    SetHTMLTemplate( *GetDoc() );   //Styles from HTML.vor

    SfxViewShell* pViewShell = GetView() ? (SfxViewShell*)GetView()
                                         : SfxViewShell::Current();
    SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
    pViewFrm->GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON );

    SubInitNew();

    SfxMedium aMed( rStreamName, STREAM_READ );
    // #i48748# - use class <SwReloadFromHtmlReader>, because
    // the base URL has to be set to the filename of the document <rMedname>
    // and not to the base URL of the temporary file <aMed> in order to get
    // the URLs of the linked graphics correctly resolved.
    SwReloadFromHtmlReader aReader( aMed, rMedname, mpDoc );

    aReader.Read( *ReadHTML );

    const SwView* pCurrView = GetView();
    //in print layout the first page(s) may have been formatted as a mix of browse
    //and print layout
    if(!bWasBrowseMode && pCurrView)
    {
        SwWrtShell& rWrtSh = pCurrView->GetWrtShell();
        if( rWrtSh.GetLayout())
            rWrtSh.CheckBrowseView( true );
    }

    // Take HTTP-Header-Attibutes over into the DokInfo again.
    // The Base-URL doesn't matter here because TLX uses the one from the document
    // for absolutization.
    SetHeaderAttributesForSourceViewHack();

    if(bModified && !IsReadOnly())
        SetModified();
    else
        mpDoc->getIDocumentState().ResetModified();
}

sal_uLong SwDocShell::LoadStylesFromFile( const OUString& rURL,
                    SwgReaderOption& rOpt, bool bUnoCall )
{
    sal_uLong nErr = 0;

    // Set filter:
    OUString sFactory(OUString::createFromAscii(SwDocShell::Factory().GetShortName()));
    SfxFilterMatcher aMatcher( sFactory );

    // search for filter in WebDocShell, too
    SfxMedium aMed( rURL, STREAM_STD_READ );
    const SfxFilter* pFlt = 0;
    aMatcher.DetectFilter( aMed, &pFlt, false, false );
    if(!pFlt)
    {
        OUString sWebFactory(OUString::createFromAscii(SwWebDocShell::Factory().GetShortName()));
        SfxFilterMatcher aWebMatcher( sWebFactory );
        aWebMatcher.DetectFilter( aMed, &pFlt, false, false );
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
        boost::scoped_ptr<SwReader> pReader;
        boost::scoped_ptr<SwPaM> pPam;
        // the SW3IO - Reader need the pam/wrtshell, because only then he
        // insert the styles!
        if( bUnoCall )
        {
            SwNodeIndex aIdx( mpDoc->GetNodes().GetEndOfContent(), -1 );
            pPam.reset(new SwPaM( aIdx ));
            pReader.reset(new SwReader( aMed, rURL, *pPam ));
        }
        else
        {
            pReader.reset(new SwReader( aMed, rURL, *mpWrtShell->GetCrsr() ));
        }

        pRead->GetReaderOpt().SetTxtFmts( rOpt.IsTxtFmts() );
        pRead->GetReaderOpt().SetFrmFmts( rOpt.IsFrmFmts() );
        pRead->GetReaderOpt().SetPageDescs( rOpt.IsPageDescs() );
        pRead->GetReaderOpt().SetNumRules( rOpt.IsNumRules() );
        pRead->GetReaderOpt().SetMerge( rOpt.IsMerge() );

        if( bUnoCall )
        {
            UnoActionContext aAction( mpDoc );
            nErr = pReader->Read( *pRead );
        }
        else
        {
            mpWrtShell->StartAllAction();
            nErr = pReader->Read( *pRead );
            mpWrtShell->EndAllAction();
        }
    }

    return nErr;
}

// Get a client for an embedded object if possible.
SfxInPlaceClient* SwDocShell::GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef )
{
    SfxInPlaceClient* pResult = NULL;

    SwWrtShell* pShell = GetWrtShell();
    if ( pShell )
    {
        pResult = pShell->GetView().FindIPClient( xObjRef.GetObject(), (Window*)&pShell->GetView().GetEditWin() );
        if ( !pResult )
            pResult = new SwOleClient( &pShell->GetView(), &pShell->GetView().GetEditWin(), xObjRef );
    }

    return pResult;
}

static bool lcl_MergePortions(SwNode *const& pNode, void *)
{
    if (pNode->IsTxtNode())
    {
        pNode->GetTxtNode()->FileLoadedInitHints();
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
    TypeId aType( TYPE(SwDocShell) );

    SfxObjectShell* pShell = pDestSh;
    bool bFirst = 0 != pShell;

    if( !bFirst )
        // No DocShell passed, starting with the first from the DocShell list
        pShell = SfxObjectShell::GetFirst( &aType );

    while( pShell )
    {
        // We want this one
        SfxMedium* pMed = pShell->GetMedium();
        if( pMed && pMed->GetURLObject() == aTmpObj )
        {
            const SfxPoolItem* pItem;
            if( ( SfxItemState::SET == pMed->GetItemSet()->GetItemState(
                                            SID_VERSION, false, &pItem ) )
                    ? (nVersion == ((SfxInt16Item*)pItem)->GetValue())
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
            pShell = SfxObjectShell::GetFirst( &aType );
        }
        else
            pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }

    // 2. Open the file ourselves
    SfxMedium* pMed = new SfxMedium( aTmpObj.GetMainURL(
                             INetURLObject::NO_DECODE ), STREAM_READ );
    if( INET_PROT_FILE == aTmpObj.GetProtocol() )
        pMed->Download(); // Touch the medium (download it)

    const SfxFilter* pSfxFlt = 0;
    if( !pMed->GetError() )
    {
        SfxFilterMatcher aMatcher( OUString::createFromAscii(SwDocShell::Factory().GetShortName()) );

        // No Filter, so search for it. Else test if the one passed is a valid one
        if( !rFilter.isEmpty() )
        {
            pSfxFlt = aMatcher.GetFilter4FilterName( rFilter );
        }

        if( nVersion )
            pMed->GetItemSet()->Put( SfxInt16Item( SID_VERSION, nVersion ));

        if( !rPasswd.isEmpty() )
            pMed->GetItemSet()->Put( SfxStringItem( SID_PASSWORD, rPasswd ));

        if( !pSfxFlt )
            aMatcher.DetectFilter( *pMed, &pSfxFlt, false, false );

        if( pSfxFlt )
        {
            // We cannot do anything without a Filter
            pMed->SetFilter( pSfxFlt );

            // If the new shell is created, SfxObjectShellLock should be used to let it be closed later for sure
            SwDocShell *const pNew(new SwDocShell(SFX_CREATE_MODE_INTERNAL));
            xLockRef = pNew;
            xDocSh = (SfxObjectShell*)xLockRef;
            if( xDocSh->DoLoad( pMed ) )
            {
                SwDoc const& rDoc(*pNew->GetDoc());
                const_cast<SwDoc&>(rDoc).GetNodes().ForEach(&lcl_MergePortions);
                return 2;
            }
        }
    }

    if( !xDocSh.Is() ) // Medium still needs to be deleted
        delete pMed;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
