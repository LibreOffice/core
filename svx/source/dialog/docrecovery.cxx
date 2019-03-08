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

#include <config_folders.h>

#include <sal/macros.h>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <bitmaps.hlst>
#include <docrecovery.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/scrbar.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/theAutoRecovery.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/treelistentry.hxx>
#include <officecfg/Office/Recovery.hxx>

namespace svx{
    namespace DocRecovery{

using namespace ::osl;

RecoveryCore::RecoveryCore(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                 bool                                            bUsedForSaving)
    : m_xContext        ( rxContext    )
    , m_pListener       ( nullptr            )
    , m_bListenForSaving(bUsedForSaving)
{
    impl_startListening();
}


RecoveryCore::~RecoveryCore()
{
    impl_stopListening();
}


const css::uno::Reference< css::uno::XComponentContext >& RecoveryCore::getComponentContext()
{
    return m_xContext;
}


TURLList& RecoveryCore::getURLListAccess()
{
    return m_lURLs;
}


bool RecoveryCore::isBrokenTempEntry(const TURLInfo& rInfo)
{
    if (rInfo.TempURL.isEmpty())
        return false;

    // Note: If the original files was recovery ... but a temp file
    // exists ... an error inside the temp file exists!
    if (
        (rInfo.RecoveryState != E_RECOVERY_FAILED            ) &&
        (rInfo.RecoveryState != E_ORIGINAL_DOCUMENT_RECOVERED)
       )
       return false;

    return true;
}


void RecoveryCore::saveBrokenTempEntries(const OUString& rPath)
{
    if (rPath.isEmpty())
        return;

    if (!m_xRealCore.is())
        return;

    // prepare all needed parameters for the following dispatch() request.
    css::util::URL aCopyURL = impl_getParsedURL(RECOVERY_CMD_DO_ENTRY_BACKUP);
    css::uno::Sequence< css::beans::PropertyValue > lCopyArgs(3);
    lCopyArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lCopyArgs[0].Value <<= false;
    lCopyArgs[1].Name    = PROP_SAVEPATH;
    lCopyArgs[1].Value <<= rPath;
    lCopyArgs[2].Name    = PROP_ENTRYID;
    // lCopyArgs[2].Value will be changed during next loop...

    // work on a copied list only...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    for (const TURLInfo& rInfo : lURLs)
    {
        if (!RecoveryCore::isBrokenTempEntry(rInfo))
            continue;

        lCopyArgs[2].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aCopyURL, lCopyArgs);
    }
}


void RecoveryCore::saveAllTempEntries(const OUString& rPath)
{
    if (rPath.isEmpty())
        return;

    if (!m_xRealCore.is())
        return;

    // prepare all needed parameters for the following dispatch() request.
    css::util::URL aCopyURL = impl_getParsedURL(RECOVERY_CMD_DO_ENTRY_BACKUP);
    css::uno::Sequence< css::beans::PropertyValue > lCopyArgs(3);
    lCopyArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lCopyArgs[0].Value <<= false;
    lCopyArgs[1].Name    = PROP_SAVEPATH;
    lCopyArgs[1].Value <<= rPath;
    lCopyArgs[2].Name    = PROP_ENTRYID;
    // lCopyArgs[2].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    for (const TURLInfo& rInfo : lURLs)
    {
        if (rInfo.TempURL.isEmpty())
            continue;

        lCopyArgs[2].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aCopyURL, lCopyArgs);
    }
}


void RecoveryCore::forgetBrokenTempEntries()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aRemoveURL = impl_getParsedURL(RECOVERY_CMD_DO_ENTRY_CLEANUP);
    css::uno::Sequence< css::beans::PropertyValue > lRemoveArgs(2);
    lRemoveArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lRemoveArgs[0].Value <<= false;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    for (const TURLInfo& rInfo : lURLs)
    {
        if (!RecoveryCore::isBrokenTempEntry(rInfo))
            continue;

        lRemoveArgs[1].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aRemoveURL, lRemoveArgs);
    }
}


void RecoveryCore::forgetAllRecoveryEntries()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aRemoveURL = impl_getParsedURL(RECOVERY_CMD_DO_ENTRY_CLEANUP);
    css::uno::Sequence< css::beans::PropertyValue > lRemoveArgs(2);
    lRemoveArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lRemoveArgs[0].Value <<= false;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    for (const TURLInfo& rInfo : lURLs)
    {
        lRemoveArgs[1].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aRemoveURL, lRemoveArgs);
    }
}


void RecoveryCore::forgetBrokenRecoveryEntries()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aRemoveURL = impl_getParsedURL(RECOVERY_CMD_DO_ENTRY_CLEANUP);
    css::uno::Sequence< css::beans::PropertyValue > lRemoveArgs(2);
    lRemoveArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lRemoveArgs[0].Value <<= false;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    for (const TURLInfo& rInfo : lURLs)
    {
        if (!RecoveryCore::isBrokenTempEntry(rInfo))
            continue;

        lRemoveArgs[1].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aRemoveURL, lRemoveArgs);
    }
}


void RecoveryCore::setProgressHandler(const css::uno::Reference< css::task::XStatusIndicator >& xProgress)
{
    m_xProgress = xProgress;
}


void RecoveryCore::setUpdateListener(IRecoveryUpdateListener* pListener)
{
    m_pListener = pListener;
}


void RecoveryCore::doEmergencySavePrepare()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aURL = impl_getParsedURL(RECOVERY_CMD_DO_PREPARE_EMERGENCY_SAVE);

    css::uno::Sequence< css::beans::PropertyValue > lArgs(1);
    lArgs[0].Name    = PROP_DISPATCHASYNCHRON;
    lArgs[0].Value <<= false;

    m_xRealCore->dispatch(aURL, lArgs);
}


void RecoveryCore::doEmergencySave()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aURL = impl_getParsedURL(RECOVERY_CMD_DO_EMERGENCY_SAVE);

    css::uno::Sequence< css::beans::PropertyValue > lArgs(2);
    lArgs[0].Name    = PROP_STATUSINDICATOR;
    lArgs[0].Value <<= m_xProgress;
    lArgs[1].Name    = PROP_DISPATCHASYNCHRON;
    lArgs[1].Value <<= true;

    m_xRealCore->dispatch(aURL, lArgs);
}


void RecoveryCore::doRecovery()
{
    if (!m_xRealCore.is())
        return;

    css::util::URL aURL = impl_getParsedURL(RECOVERY_CMD_DO_RECOVERY);

    css::uno::Sequence< css::beans::PropertyValue > lArgs(2);
    lArgs[0].Name    = PROP_STATUSINDICATOR;
    lArgs[0].Value <<= m_xProgress;
    lArgs[1].Name    = PROP_DISPATCHASYNCHRON;
    lArgs[1].Value <<= true;

    m_xRealCore->dispatch(aURL, lArgs);
}


ERecoveryState RecoveryCore::mapDocState2RecoverState(EDocStates eDocState)
{
    // ???
    ERecoveryState eRecState = E_NOT_RECOVERED_YET;

    /* Attention:
        Some of the following states can occur at the
        same time. So we have to check for the "worst case" first!

        DAMAGED -> INCOMPLETE -> HANDLED
     */

    // running ...
    if (
        (eDocState & EDocStates::TryLoadBackup  ) ||
        (eDocState & EDocStates::TryLoadOriginal)
       )
        eRecState = E_RECOVERY_IS_IN_PROGRESS;
    // red
    else if (eDocState & EDocStates::Damaged)
        eRecState = E_RECOVERY_FAILED;
    // yellow
    else if (eDocState & EDocStates::Incomplete)
        eRecState = E_ORIGINAL_DOCUMENT_RECOVERED;
    // green
    else if (eDocState & EDocStates::Succeeded)
        eRecState = E_SUCCESSFULLY_RECOVERED;

    return eRecState;
}


void SAL_CALL RecoveryCore::statusChanged(const css::frame::FeatureStateEvent& aEvent)
{
    // a) special notification about start/stop async dispatch!
    //    FeatureDescriptor = "start" || "stop"
    if (aEvent.FeatureDescriptor == RECOVERY_OPERATIONSTATE_START)
    {
        if (m_pListener)
            m_pListener->start();
        return;
    }

    if (aEvent.FeatureDescriptor == RECOVERY_OPERATIONSTATE_STOP)
    {
        if (m_pListener)
            m_pListener->end();
        return;
    }

    // b) normal notification about changed items
    //    FeatureDescriptor = "Update"
    //    State             = Lits of information [seq< NamedValue >]
    if (aEvent.FeatureDescriptor != RECOVERY_OPERATIONSTATE_UPDATE)
        return;

    ::comphelper::SequenceAsHashMap lInfo(aEvent.State);
    TURLInfo                        aNew;

    aNew.ID          = lInfo.getUnpackedValueOrDefault(STATEPROP_ID         , sal_Int32(0)     );
    aNew.DocState    = static_cast<EDocStates>(lInfo.getUnpackedValueOrDefault(STATEPROP_STATE      , sal_Int32(0)     ));
    aNew.OrgURL      = lInfo.getUnpackedValueOrDefault(STATEPROP_ORGURL     , OUString());
    aNew.TempURL     = lInfo.getUnpackedValueOrDefault(STATEPROP_TEMPURL    , OUString());
    aNew.FactoryURL  = lInfo.getUnpackedValueOrDefault(STATEPROP_FACTORYURL , OUString());
    aNew.TemplateURL = lInfo.getUnpackedValueOrDefault(STATEPROP_TEMPLATEURL, OUString());
    aNew.DisplayName = lInfo.getUnpackedValueOrDefault(STATEPROP_TITLE      , OUString());
    aNew.Module      = lInfo.getUnpackedValueOrDefault(STATEPROP_MODULE     , OUString());

    if (aNew.OrgURL.isEmpty()) {
        // If there is no file URL, the window title is used for the display name.
        // Remove any unwanted elements such as " - LibreOffice Writer".
        sal_Int32 i = aNew.DisplayName.indexOf(" - ");
        if (i > 0)
            aNew.DisplayName = aNew.DisplayName.copy(0, i);
    } else {
        // If there is a file URL, parse out the filename part as the display name.
        INetURLObject aOrgURL(aNew.OrgURL);
        aNew.DisplayName = aOrgURL.getName(INetURLObject::LAST_SEGMENT, true,
                                           INetURLObject::DecodeMechanism::WithCharset);
    }

    // search for already existing items and update her nState value ...
    for (TURLInfo& aOld : m_lURLs)
    {
        if (aOld.ID == aNew.ID)
        {
            // change existing
            aOld.DocState      = aNew.DocState;
            aOld.RecoveryState = RecoveryCore::mapDocState2RecoverState(aOld.DocState);
            if (m_pListener)
            {
                m_pListener->updateItems();
                m_pListener->stepNext(&aOld);
            }
            return;
        }
    }

    // append as new one
    // TODO think about matching Module name to a corresponding icon
    OUString sURL = aNew.OrgURL;
    if (sURL.isEmpty())
        sURL = aNew.FactoryURL;
    if (sURL.isEmpty())
        sURL = aNew.TempURL;
    if (sURL.isEmpty())
        sURL = aNew.TemplateURL;
    INetURLObject aURL(sURL);
    aNew.StandardImageId = SvFileInformationManager::GetFileImageId(aURL);

    /* set the right UI state for this item to NOT_RECOVERED_YET... because nDocState shows the state of
       the last emergency save operation before and is interesting for the used recovery core service only...
       for now! But if there is a further notification for this item (see lines above!) we must
       map the doc state to an UI state. */
    aNew.RecoveryState = E_NOT_RECOVERED_YET;

    m_lURLs.push_back(aNew);

    if (m_pListener)
        m_pListener->updateItems();
}


void SAL_CALL RecoveryCore::disposing(const css::lang::EventObject& /*aEvent*/)
{
    m_xRealCore.clear();
}


void RecoveryCore::impl_startListening()
{
    // listening already initialized ?
    if (m_xRealCore.is())
        return;
    m_xRealCore = css::frame::theAutoRecovery::get(m_xContext);

    css::util::URL aURL;
    if (m_bListenForSaving)
        aURL.Complete = RECOVERY_CMD_DO_EMERGENCY_SAVE;
    else
        aURL.Complete = RECOVERY_CMD_DO_RECOVERY;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
    xParser->parseStrict(aURL);

    /* Note: addStatusListener() call us synchronous back ... so we
             will get the complete list of currently open documents! */
    m_xRealCore->addStatusListener(static_cast< css::frame::XStatusListener* >(this), aURL);
}


void RecoveryCore::impl_stopListening()
{
    // Ignore it, if this instance doesn't listen currently
    if (!m_xRealCore.is())
        return;

    css::util::URL aURL;
    if (m_bListenForSaving)
        aURL.Complete = RECOVERY_CMD_DO_EMERGENCY_SAVE;
    else
        aURL.Complete = RECOVERY_CMD_DO_RECOVERY;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
    xParser->parseStrict(aURL);

    m_xRealCore->removeStatusListener(static_cast< css::frame::XStatusListener* >(this), aURL);
    m_xRealCore.clear();
}


css::util::URL RecoveryCore::impl_getParsedURL(const OUString& sURL)
{
    css::util::URL aURL;
    aURL.Complete = sURL;

    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
    xParser->parseStrict(aURL);

    return aURL;
}

PluginProgress::PluginProgress(weld::ProgressBar* pProgressBar)
    : m_pProgressBar(pProgressBar)
    , m_nRange(100)
{
}

PluginProgress::~PluginProgress()
{
}

void SAL_CALL PluginProgress::dispose()
{
    m_pProgressBar = nullptr;
}

void SAL_CALL PluginProgress::addEventListener(const css::uno::Reference< css::lang::XEventListener >& )
{
}

void SAL_CALL PluginProgress::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
{
}

void SAL_CALL PluginProgress::start(const OUString&, sal_Int32 nRange)
{
    m_nRange = nRange;
    if (m_pProgressBar)
        m_pProgressBar->set_percentage(0);
}

void SAL_CALL PluginProgress::end()
{
    if (m_pProgressBar)
        m_pProgressBar->set_percentage(m_nRange);
}

void SAL_CALL PluginProgress::setText(const OUString& rText)
{
    if (m_pProgressBar)
        m_pProgressBar->set_text(rText);
}

void SAL_CALL PluginProgress::setValue(sal_Int32 nValue)
{
    if (m_pProgressBar)
        m_pProgressBar->set_percentage((nValue * 100) / m_nRange);
}

void SAL_CALL PluginProgress::reset()
{
    if (m_pProgressBar)
        m_pProgressBar->set_percentage(0);
}

SaveDialog::SaveDialog(weld::Window* pParent, RecoveryCore* pCore)
    : GenericDialogController(pParent, "svx/ui/docrecoverysavedialog.ui", "DocRecoverySaveDialog")
    , m_pCore(pCore)
    , m_xFileListLB(m_xBuilder->weld_tree_view("filelist"))
    , m_xOkBtn(m_xBuilder->weld_button("ok"))
{
    m_xFileListLB->set_size_request(-1, m_xFileListLB->get_height_rows(10));

    // Prepare the office for the following crash save step.
    // E.g. hide all open windows so the user can't influence our
    // operation .-)
    m_pCore->doEmergencySavePrepare();

    m_xOkBtn->connect_clicked(LINK(this, SaveDialog, OKButtonHdl));

    // fill listbox with current open documents

    TURLList&                rURLs = m_pCore->getURLListAccess();

    for (const TURLInfo& rInfo : rURLs)
    {
        m_xFileListLB->append("", rInfo.DisplayName, rInfo.StandardImageId);
    }
}

SaveDialog::~SaveDialog()
{
}

IMPL_LINK_NOARG(SaveDialog, OKButtonHdl, weld::Button&, void)
{
    // start crash-save with progress
    std::unique_ptr<SaveProgressDialog> xProgress(new SaveProgressDialog(m_xDialog.get(), m_pCore));
    short nResult = xProgress->run();
    xProgress.reset();

    // if "CANCEL" => return "CANCEL"
    // if "OK"     => "AUTOLUNCH" always !
    if (nResult == DLG_RET_OK)
        nResult = DLG_RET_OK_AUTOLUNCH;

    m_xDialog->response(nResult);
}

SaveProgressDialog::SaveProgressDialog(weld::Window* pParent, RecoveryCore* pCore)
    : GenericDialogController(pParent, "svx/ui/docrecoveryprogressdialog.ui", "DocRecoveryProgressDialog")
    , m_pCore(pCore)
    , m_xProgressBar(m_xBuilder->weld_progress_bar("progress"))
{
    m_xProgressBar->set_size_request(m_xProgressBar->get_approximate_digit_width() * 50, -1);
    PluginProgress* pProgress = new PluginProgress(m_xProgressBar.get());
    m_xProgress.set(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);
}

SaveProgressDialog::~SaveProgressDialog()
{
}

short SaveProgressDialog::run()
{
    ::SolarMutexGuard aLock;

    m_pCore->setProgressHandler(m_xProgress);
    m_pCore->setUpdateListener(this);
    m_pCore->doEmergencySave();
    short nRet = DialogController::run();
    m_pCore->setUpdateListener(nullptr);
    return nRet;
}

void SaveProgressDialog::updateItems()
{
}

void SaveProgressDialog::stepNext(TURLInfo* )
{
    /* TODO

       if m_pCore would have a member m_mCurrentItem, you could see,
       who is current, who is next ... You can show this information
       in progress report FixText
    */
}

void SaveProgressDialog::start()
{
}

void SaveProgressDialog::end()
{
    m_xDialog->response(DLG_RET_OK);
}

static short impl_askUserForWizardCancel(weld::Widget* pParent, const char* pRes)
{
    std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(pParent,
                                                VclMessageType::Question, VclButtonsType::YesNo, SvxResId(pRes)));
    if (xQuery->run() == RET_YES)
        return DLG_RET_OK;
    else
        return DLG_RET_CANCEL;
}

RecoveryDialog::RecoveryDialog(weld::Window* pParent, RecoveryCore* pCore)
    : GenericDialogController(pParent, "svx/ui/docrecoveryrecoverdialog.ui", "DocRecoveryRecoverDialog")
    , m_aTitleRecoveryInProgress(SvxResId(RID_SVXSTR_RECOVERY_INPROGRESS))
    , m_aRecoveryOnlyFinish (SvxResId(RID_SVXSTR_RECOVERYONLY_FINISH))
    , m_aRecoveryOnlyFinishDescr(SvxResId(RID_SVXSTR_RECOVERYONLY_FINISH_DESCR))
    , m_pCore(pCore)
    , m_eRecoveryState(RecoveryDialog::E_RECOVERY_PREPARED)
    , m_bWaitForCore(false)
    , m_bWasRecoveryStarted(false)
    , m_aSuccessRecovStr(SvxResId(RID_SVXSTR_SUCCESSRECOV))
    , m_aOrigDocRecovStr(SvxResId(RID_SVXSTR_ORIGDOCRECOV))
    , m_aRecovFailedStr(SvxResId(RID_SVXSTR_RECOVFAILED))
    , m_aRecovInProgrStr(SvxResId(RID_SVXSTR_RECOVINPROGR))
    , m_aNotRecovYetStr(SvxResId(RID_SVXSTR_NOTRECOVYET))
    , m_xDescrFT(m_xBuilder->weld_label("desc"))
    , m_xProgressBar(m_xBuilder->weld_progress_bar("progress"))
    , m_xFileListLB(m_xBuilder->weld_tree_view("filelist"))
    , m_xNextBtn(m_xBuilder->weld_button("next"))
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
{
    const auto nWidth = m_xFileListLB->get_approximate_digit_width() * 70;
    m_xFileListLB->set_size_request(nWidth, m_xFileListLB->get_height_rows(10));
    m_xProgressBar->set_size_request(m_xProgressBar->get_approximate_digit_width() * 50, -1);
    PluginProgress* pProgress = new PluginProgress(m_xProgressBar.get());
    m_xProgress.set(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);

    std::vector<int> aWidths;
    aWidths.push_back(m_xFileListLB->get_checkbox_column_width());
    aWidths.push_back(60 * nWidth / 100);
    aWidths.push_back(m_xFileListLB->get_checkbox_column_width());
    m_xFileListLB->set_column_fixed_widths(aWidths);

    m_xNextBtn->set_sensitive(true);
    m_xNextBtn->connect_clicked( LINK( this, RecoveryDialog, NextButtonHdl ) );
    m_xCancelBtn->connect_clicked( LINK( this, RecoveryDialog, CancelButtonHdl ) );

    // fill list box first time
    TURLList& rURLList = m_pCore->getURLListAccess();
    for (size_t i = 0, nCount = rURLList.size(); i < nCount; ++i)
    {
        const TURLInfo& rInfo = rURLList[i];
        m_xFileListLB->append();
        m_xFileListLB->set_id(i, OUString::number(reinterpret_cast<sal_IntPtr>(&rInfo)));
        m_xFileListLB->set_image(i, rInfo.StandardImageId, 0);
        m_xFileListLB->set_text(i, rInfo.DisplayName, 1);
        m_xFileListLB->set_image(i, impl_getStatusImage(rInfo), 2);
        m_xFileListLB->set_text(i, impl_getStatusString(rInfo), 3);
    }

    // mark first item
    if (m_xFileListLB->n_children())
        m_xFileListLB->set_cursor(0);
}

RecoveryDialog::~RecoveryDialog()
{
}

short RecoveryDialog::execute()
{
    ::SolarMutexGuard aSolarLock;

    switch (m_eRecoveryState)
    {
        case RecoveryDialog::E_RECOVERY_IN_PROGRESS :
             {
                // user decided to start recovery ...
                m_bWasRecoveryStarted = true;
                // do it asynchronous (to allow repaints)
                // and wait for this asynchronous operation.
                m_xDescrFT->set_label( m_aTitleRecoveryInProgress );
                m_xNextBtn->set_sensitive(false);
                m_xCancelBtn->set_sensitive(false);
                m_pCore->setProgressHandler(m_xProgress);
                m_pCore->setUpdateListener(this);
                m_pCore->doRecovery();

                m_bWaitForCore = true;
                while(m_bWaitForCore)
                    Application::Yield();

                m_pCore->setUpdateListener(nullptr);
                m_eRecoveryState = RecoveryDialog::E_RECOVERY_CORE_DONE;
                return execute();
             }

        case RecoveryDialog::E_RECOVERY_CORE_DONE :
             {
                 // the core finished it's task.
                 // let the user decide the next step.
                 m_xDescrFT->set_label(m_aRecoveryOnlyFinishDescr);
                 m_xNextBtn->set_label(m_aRecoveryOnlyFinish);
                 m_xNextBtn->set_sensitive(true);
                 m_xCancelBtn->set_sensitive(false);
                 return 0;
             }

        case RecoveryDialog::E_RECOVERY_DONE :
             {
                 // All documents were recovered.
                 // User decided to step to the "next" wizard page.
                 // Do it ... but check first, if there exist some
                 // failed recovery documents. They must be saved to
                 // a user selected directory.
                 short                 nRet                  = DLG_RET_UNKNOWN;
                 BrokenRecoveryDialog aBrokenRecoveryDialog(m_xDialog.get(), m_pCore, !m_bWasRecoveryStarted);
                 OUString sSaveDir = aBrokenRecoveryDialog.getSaveDirURL(); // get the default dir
                 if (aBrokenRecoveryDialog.isExecutionNeeded())
                 {
                     nRet = aBrokenRecoveryDialog.run();
                     sSaveDir = aBrokenRecoveryDialog.getSaveDirURL();
                 }

                 switch(nRet)
                 {
                     // no broken temp files exists
                     // step to the next wizard page
                     case DLG_RET_UNKNOWN :
                          {
                              m_eRecoveryState = RecoveryDialog::E_RECOVERY_HANDLED;
                              return DLG_RET_OK;
                          }

                     // user decided to save the broken temp files
                     // do and forget it
                     // step to the next wizard page
                     case DLG_RET_OK :
                          {
                              m_pCore->saveBrokenTempEntries(sSaveDir);
                              m_pCore->forgetBrokenTempEntries();
                              m_eRecoveryState = RecoveryDialog::E_RECOVERY_HANDLED;
                              return DLG_RET_OK;
                          }

                     // user decided to ignore broken temp files.
                     // Ask it again ... may be this decision was wrong.
                     // Results:
                     //     IGNORE => remove broken temp files
                     //            => step to the next wizard page
                     //     CANCEL => step back to the recovery page
                     case DLG_RET_CANCEL :
                          {
                              // TODO ask user ...
                              m_pCore->forgetBrokenTempEntries();
                              m_eRecoveryState = RecoveryDialog::E_RECOVERY_HANDLED;
                              return DLG_RET_OK;
                          }
                 }

                 m_eRecoveryState = RecoveryDialog::E_RECOVERY_HANDLED;
                 return DLG_RET_OK;
             }

        case RecoveryDialog::E_RECOVERY_CANCELED :
             {
                 // "YES" => break recovery
                 // But there exist different states, where "cancel" can be called.
                 // Handle it different.
                 if (m_bWasRecoveryStarted)
                     m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED_AFTERWARDS;
                 else
                     m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED_BEFORE;
                 return execute();
             }

        case RecoveryDialog::E_RECOVERY_CANCELED_BEFORE :
        case RecoveryDialog::E_RECOVERY_CANCELED_AFTERWARDS :
             {
                 // We have to check if there exists some temp. files.
                 // They should be saved to a user defined location.
                 // If no temp files exists or user decided to ignore it ...
                 // we have to remove all recovery/session data anyway!
                 short nRet = DLG_RET_UNKNOWN;
                 BrokenRecoveryDialog aBrokenRecoveryDialog(m_xDialog.get(), m_pCore, !m_bWasRecoveryStarted);
                 OUString sSaveDir = aBrokenRecoveryDialog.getSaveDirURL(); // get the default save location

                 // dialog itself checks if there is a need to copy files for this mode.
                 // It uses the information m_bWasRecoveryStarted doing so.
                 if (aBrokenRecoveryDialog.isExecutionNeeded())
                 {
                     nRet     = aBrokenRecoveryDialog.run();
                     sSaveDir = aBrokenRecoveryDialog.getSaveDirURL();
                 }

                 // Possible states:
                 // a) nRet == DLG_RET_UNKNOWN
                 //         dialog was not shown ...
                 //         because there exists no temp file for copy.
                 //         => remove all recovery data
                 // b) nRet == DLG_RET_OK
                 //         dialog was shown ...
                 //         user decided to save temp files
                 //         => save all OR broken temp files (depends from the time, where cancel was called)
                 //         => remove all recovery data
                 // c) nRet == DLG_RET_CANCEL
                 //         dialog was shown ...
                 //         user decided to ignore temp files
                 //         => remove all recovery data
                 // => a)/c) are the same ... b) has one additional operation

                 // b)
                 if (nRet == DLG_RET_OK)
                 {
                     if (m_bWasRecoveryStarted)
                         m_pCore->saveBrokenTempEntries(sSaveDir);
                     else
                         m_pCore->saveAllTempEntries(sSaveDir);
                 }

                 // a,b,c)
                 if (m_bWasRecoveryStarted)
                    m_pCore->forgetBrokenRecoveryEntries();
                 else
                    m_pCore->forgetAllRecoveryEntries();
                 m_eRecoveryState = RecoveryDialog::E_RECOVERY_HANDLED;

                 // THERE IS NO WAY BACK. see impl_askUserForWizardCancel()!
                 return DLG_RET_CANCEL;
             }
    }

    // should never be reached .-)
    OSL_FAIL("Should never be reached!");
    return DLG_RET_OK;
}

void RecoveryDialog::start()
{
}

void RecoveryDialog::updateItems()
{
    int c = m_xFileListLB->n_children();
    for (int i = 0; i < c; ++i)
    {
        TURLInfo* pInfo = reinterpret_cast<TURLInfo*>(m_xFileListLB->get_id(i).toInt64());
        if ( !pInfo )
            continue;

        m_xFileListLB->set_image(i, impl_getStatusImage(*pInfo), 2);
        OUString sStatus = impl_getStatusString( *pInfo );
        if (!sStatus.isEmpty())
            m_xFileListLB->set_text(i, sStatus, 3);
    }
}

void RecoveryDialog::stepNext(TURLInfo* pItem)
{
    int c = m_xFileListLB->n_children();
    for (int i=0; i < c; ++i)
    {
        TURLInfo* pInfo = reinterpret_cast<TURLInfo*>(m_xFileListLB->get_id(i).toInt64());
        if (pInfo->ID != pItem->ID)
            continue;

        m_xFileListLB->set_cursor(i);
        m_xFileListLB->scroll_to_row(i);
        break;
    }
}

void RecoveryDialog::end()
{
    m_bWaitForCore = false;
}

IMPL_LINK_NOARG(RecoveryDialog, NextButtonHdl, weld::Button&, void)
{
    switch (m_eRecoveryState)
    {
        case RecoveryDialog::E_RECOVERY_PREPARED:
            m_eRecoveryState = RecoveryDialog::E_RECOVERY_IN_PROGRESS;
            execute();
        break;
        case RecoveryDialog::E_RECOVERY_CORE_DONE:
            m_eRecoveryState = RecoveryDialog::E_RECOVERY_DONE;
            execute();
        break;
    }

    if (m_eRecoveryState == RecoveryDialog::E_RECOVERY_HANDLED)
    {
        m_xDialog->response(DLG_RET_OK);
    }
}

IMPL_LINK_NOARG(RecoveryDialog, CancelButtonHdl, weld::Button&, void)
{
    switch (m_eRecoveryState)
    {
        case RecoveryDialog::E_RECOVERY_PREPARED:
            if (impl_askUserForWizardCancel(m_xDialog.get(), RID_SVXSTR_QUERY_EXIT_RECOVERY) != DLG_RET_CANCEL)
            {
                m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED;
                execute();
            }
            break;
        case RecoveryDialog::E_RECOVERY_CORE_DONE:
            m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED;
            execute();
            break;
    }

    if (m_eRecoveryState == RecoveryDialog::E_RECOVERY_HANDLED)
    {
        m_xDialog->response(RET_CANCEL);
    }
}

OUString RecoveryDialog::impl_getStatusString( const TURLInfo& rInfo ) const
{
    OUString sStatus;
    switch ( rInfo.RecoveryState )
    {
        case E_SUCCESSFULLY_RECOVERED :
            sStatus = m_aSuccessRecovStr;
            break;
        case E_ORIGINAL_DOCUMENT_RECOVERED :
            sStatus = m_aOrigDocRecovStr;
            break;
        case E_RECOVERY_FAILED :
            sStatus = m_aRecovFailedStr;
            break;
        case E_RECOVERY_IS_IN_PROGRESS :
            sStatus = m_aRecovInProgrStr;
            break;
        case E_NOT_RECOVERED_YET :
            sStatus = m_aNotRecovYetStr;
            break;
        default:
            break;
    }
    return sStatus;
}

OUString RecoveryDialog::impl_getStatusImage( const TURLInfo& rInfo )
{
    OUString sStatus;
    switch ( rInfo.RecoveryState )
    {
        case E_SUCCESSFULLY_RECOVERED :
            sStatus = RID_SVXBMP_GREENCHECK;
            break;
        case E_ORIGINAL_DOCUMENT_RECOVERED :
            sStatus = RID_SVXBMP_YELLOWCHECK;
            break;
        case E_RECOVERY_FAILED :
            sStatus = RID_SVXBMP_REDCROSS;
            break;
        default:
            break;
    }
    return sStatus;
}

BrokenRecoveryDialog::BrokenRecoveryDialog(weld::Window* pParent,
                                           RecoveryCore* pCore,
                                           bool bBeforeRecovery)
    : GenericDialogController(pParent, "svx/ui/docrecoverybrokendialog.ui", "DocRecoveryBrokenDialog")
    , m_pCore(pCore)
    , m_bBeforeRecovery(bBeforeRecovery)
    , m_bExecutionNeeded(false)
    , m_xFileListLB(m_xBuilder->weld_tree_view("filelist"))
    , m_xSaveDirED(m_xBuilder->weld_entry("savedir"))
    , m_xSaveDirBtn(m_xBuilder->weld_button("change"))
    , m_xOkBtn(m_xBuilder->weld_button("ok"))
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
{
    m_xSaveDirBtn->connect_clicked( LINK( this, BrokenRecoveryDialog, SaveButtonHdl ) );
    m_xOkBtn->connect_clicked( LINK( this, BrokenRecoveryDialog, OkButtonHdl ) );
    m_xCancelBtn->connect_clicked( LINK( this, BrokenRecoveryDialog, CancelButtonHdl ) );

    m_sSavePath = SvtPathOptions().GetWorkPath();
    INetURLObject aObj( m_sSavePath );
    OUString sPath;
    osl::FileBase::getSystemPathFromFileURL(aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), sPath);
    m_xSaveDirED->set_text(sPath);

    impl_refresh();
}

BrokenRecoveryDialog::~BrokenRecoveryDialog()
{
}

void BrokenRecoveryDialog::impl_refresh()
{
    m_bExecutionNeeded = false;
    TURLList& rURLList = m_pCore->getURLListAccess();
    for (const TURLInfo& rInfo : rURLList)
    {
        if (m_bBeforeRecovery)
        {
            // "Cancel" before recovery ->
            // search for any temp files!
            if (rInfo.TempURL.isEmpty())
                continue;
        }
        else
        {
            // "Cancel" after recovery ->
            // search for broken temp files
            if (!RecoveryCore::isBrokenTempEntry(rInfo))
                continue;
        }

        m_bExecutionNeeded = true;

        m_xFileListLB->append(OUString::number(reinterpret_cast<sal_IntPtr>(&rInfo)), rInfo.DisplayName, rInfo.StandardImageId);
    }
    m_sSavePath.clear();
    m_xOkBtn->grab_focus();
}

bool BrokenRecoveryDialog::isExecutionNeeded()
{
    return m_bExecutionNeeded;
}

const OUString& BrokenRecoveryDialog::getSaveDirURL()
{
    return m_sSavePath;
}

IMPL_LINK_NOARG(BrokenRecoveryDialog, OkButtonHdl, weld::Button&, void)
{
    OUString sPhysicalPath = comphelper::string::strip(m_xSaveDirED->get_text(), ' ');
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath( sPhysicalPath, sURL );
    m_sSavePath = sURL;
    while (m_sSavePath.isEmpty())
        impl_askForSavePath();

    m_xDialog->response(DLG_RET_OK);
}

IMPL_LINK_NOARG(BrokenRecoveryDialog, CancelButtonHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(BrokenRecoveryDialog, SaveButtonHdl, weld::Button&, void)
{
    impl_askForSavePath();
}

void BrokenRecoveryDialog::impl_askForSavePath()
{
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker =
        css::ui::dialogs::FolderPicker::create( m_pCore->getComponentContext() );

    INetURLObject aURL(m_sSavePath, INetProtocol::File);
    xFolderPicker->setDisplayDirectory(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
    short nRet = xFolderPicker->execute();
    if (nRet == css::ui::dialogs::ExecutableDialogResults::OK)
    {
        m_sSavePath = xFolderPicker->getDirectory();
        OUString sPath;
        osl::FileBase::getSystemPathFromFileURL(m_sSavePath, sPath);
        m_xSaveDirED->set_text(sPath);
    }
}

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
