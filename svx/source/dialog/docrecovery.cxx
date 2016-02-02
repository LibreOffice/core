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
#include <svx/dialogs.hrc>
#include "docrecovery.hxx"
#include "docrecovery.hrc"

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
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
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/pathoptions.hxx>
#include "svtools/treelistentry.hxx"
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


css::uno::Reference< css::uno::XComponentContext > RecoveryCore::getComponentContext()
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
        !(rInfo.RecoveryState == E_RECOVERY_FAILED            ) &&
        !(rInfo.RecoveryState == E_ORIGINAL_DOCUMENT_RECOVERED)
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
    lCopyArgs[0].Value <<= sal_False;
    lCopyArgs[1].Name    = PROP_SAVEPATH;
    lCopyArgs[1].Value <<= rPath;
    lCopyArgs[2].Name    = PROP_ENTRYID;
    // lCopyArgs[2].Value will be changed during next loop...

    // work on a copied list only...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    TURLList::const_iterator pIt;
    for (  pIt  = lURLs.begin();
           pIt != lURLs.end()  ;
         ++pIt                 )
    {
        const TURLInfo& rInfo = *pIt;
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
    lCopyArgs[0].Value <<= sal_False;
    lCopyArgs[1].Name    = PROP_SAVEPATH;
    lCopyArgs[1].Value <<= rPath;
    lCopyArgs[2].Name    = PROP_ENTRYID;
    // lCopyArgs[2].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    TURLList::const_iterator pIt;
    for (  pIt  = lURLs.begin();
           pIt != lURLs.end()  ;
         ++pIt                 )
    {
        const TURLInfo& rInfo = *pIt;
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
    lRemoveArgs[0].Value <<= sal_False;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    TURLList::const_iterator pIt;
    for (  pIt  = lURLs.begin();
           pIt != lURLs.end()  ;
         ++pIt                 )
    {
        const TURLInfo& rInfo = *pIt;
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
    lRemoveArgs[0].Value <<= sal_False;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    TURLList::const_iterator pIt;
    for (  pIt  = lURLs.begin();
           pIt != lURLs.end()  ;
         ++pIt                 )
    {
        const TURLInfo& rInfo = *pIt;
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
    lRemoveArgs[0].Value <<= sal_False;
    lRemoveArgs[1].Name    = PROP_ENTRYID;
    // lRemoveArgs[1].Value will be changed during next loop ...

    // work on a copied list only ...
    // Reason: We will get notifications from the core for every
    // changed or removed element. And that will change our m_lURLs list.
    // That's not a good idea, if we use a stl iterator inbetween .-)
    TURLList lURLs = m_lURLs;
    TURLList::const_iterator pIt;
    for (  pIt  = lURLs.begin();
           pIt != lURLs.end()  ;
         ++pIt                 )
    {
        const TURLInfo& rInfo = *pIt;
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
    lArgs[0].Value <<= sal_False;

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
    lArgs[1].Value <<= sal_True;

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
    lArgs[1].Value <<= sal_True;

    m_xRealCore->dispatch(aURL, lArgs);
}


ERecoveryState RecoveryCore::mapDocState2RecoverState(sal_Int32 eDocState)
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
        ((eDocState & E_TRY_LOAD_BACKUP  ) == E_TRY_LOAD_BACKUP  ) ||
        ((eDocState & E_TRY_LOAD_ORIGINAL) == E_TRY_LOAD_ORIGINAL)
       )
        eRecState = E_RECOVERY_IS_IN_PROGRESS;
    // red
    else if ((eDocState & E_DAMAGED) == E_DAMAGED)
        eRecState = E_RECOVERY_FAILED;
    // yellow
    else if ((eDocState & E_INCOMPLETE) == E_INCOMPLETE)
        eRecState = E_ORIGINAL_DOCUMENT_RECOVERED;
    // green
    else if ((eDocState & E_SUCCEDED) == E_SUCCEDED)
        eRecState = E_SUCCESSFULLY_RECOVERED;

    return eRecState;
}


void SAL_CALL RecoveryCore::statusChanged(const css::frame::FeatureStateEvent& aEvent)
    throw(css::uno::RuntimeException, std::exception)
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

    aNew.ID          = lInfo.getUnpackedValueOrDefault(STATEPROP_ID         , (sal_Int32)0     );
    aNew.DocState    = lInfo.getUnpackedValueOrDefault(STATEPROP_STATE      , (sal_Int32)0     );
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
                                           INetURLObject::DECODE_WITH_CHARSET);
    }

    // search for already existing items and update her nState value ...
    TURLList::iterator pIt;
    for (  pIt  = m_lURLs.begin();
           pIt != m_lURLs.end()  ;
         ++pIt                   )
    {
        TURLInfo& aOld = *pIt;
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
    // TODO think about mmatching Module name to a corresponding icon
    OUString sURL = aNew.OrgURL;
    if (sURL.isEmpty())
        sURL = aNew.FactoryURL;
    if (sURL.isEmpty())
        sURL = aNew.TempURL;
    if (sURL.isEmpty())
        sURL = aNew.TemplateURL;
    INetURLObject aURL(sURL);
    aNew.StandardImage = SvFileInformationManager::GetFileImage(aURL);

    /* set the right UI state for this item to NOT_RECOVERED_YET ... because nDocState shows the state of
       the last emergency save operation before and is interessting for the used recovery core service only ...
       for now! But if there is a further notification for this item (see lines above!) we must
       map the doc state to an UI state. */
    aNew.RecoveryState = E_NOT_RECOVERED_YET;

    m_lURLs.push_back(aNew);

    if (m_pListener)
        m_pListener->updateItems();
}


void SAL_CALL RecoveryCore::disposing(const css::lang::EventObject& /*aEvent*/)
    throw(css::uno::RuntimeException, std::exception)
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

PluginProgressWindow::PluginProgressWindow(      vcl::Window*                                       pParent  ,
                                           const css::uno::Reference< css::lang::XComponent >& xProgress)
    : Window     (pParent  )
    , m_xProgress(xProgress)
{
    Show();
    Size aParentSize = pParent->GetSizePixel();
    // align the progressbar to its parent
    setPosSizePixel( -9, 0, aParentSize.Width() + 15, aParentSize.Height() - 4 );
}

PluginProgressWindow::~PluginProgressWindow()
{
    disposeOnce();
}

void PluginProgressWindow::dispose()
{
    if (m_xProgress.is())
        m_xProgress->dispose();
    vcl::Window::dispose();
}


PluginProgress::PluginProgress(      vcl::Window*                                             pParent,
                               const css::uno::Reference< css::uno::XComponentContext >& xContext  )
{
    m_pPlugProgressWindow = VclPtr<PluginProgressWindow>::Create(pParent, static_cast< css::lang::XComponent* >(this));
    css::uno::Reference< css::awt::XWindow > xProgressWindow = VCLUnoHelper::GetInterface(m_pPlugProgressWindow);
    m_xProgressFactory = css::task::StatusIndicatorFactory::createWithWindow(xContext, xProgressWindow, sal_False/*DisableReschedule*/, sal_True/*AllowParentShow*/);
    m_xProgress = m_xProgressFactory->createStatusIndicator();
}


PluginProgress::~PluginProgress()
{
}


void SAL_CALL PluginProgress::dispose()
    throw(css::uno::RuntimeException, std::exception)
{
    // m_pPluginProgressWindow was deleted ...
    // So the internal pointer of this progress
    // weill be dead!
    m_xProgress.clear();
}


void SAL_CALL PluginProgress::addEventListener(const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException, std::exception)
{
}


void SAL_CALL PluginProgress::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException, std::exception)
{
}


void SAL_CALL PluginProgress::start(const OUString&,
                                          sal_Int32        nRange)
    throw(css::uno::RuntimeException, std::exception)
{
    if (m_xProgress.is())
        m_xProgress->start(OUString(), nRange);
}


void SAL_CALL PluginProgress::end()
    throw(css::uno::RuntimeException, std::exception)
{
    if (m_xProgress.is())
        m_xProgress->end();
}


void SAL_CALL PluginProgress::setText(const OUString& sText)
    throw(css::uno::RuntimeException, std::exception)
{
    if (m_xProgress.is())
        m_xProgress->setText(sText);
}


void SAL_CALL PluginProgress::setValue(sal_Int32 nValue)
    throw(css::uno::RuntimeException, std::exception)
{
    if (m_xProgress.is())
        m_xProgress->setValue(nValue);
}


void SAL_CALL PluginProgress::reset()
    throw(css::uno::RuntimeException, std::exception)
{
    if (m_xProgress.is())
        m_xProgress->reset();
}


SaveDialog::SaveDialog(vcl::Window* pParent, RecoveryCore* pCore)
    : Dialog(pParent, "DocRecoverySaveDialog",
        "svx/ui/docrecoverysavedialog.ui")
    , m_pCore(pCore)
{
    get(m_pTitleFT, "title");
    get(m_pFileListLB, "filelist");
    m_pFileListLB->set_height_request(m_pFileListLB->GetTextHeight() * 10);
    m_pFileListLB->set_width_request(m_pFileListLB->approximate_char_width() * 72);
    get(m_pOkBtn, "ok");

    // Prepare the office for the following crash save step.
    // E.g. hide all open windows so the user can't influence our
    // operation .-)
    m_pCore->doEmergencySavePrepare();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    m_pTitleFT->SetBackground(rStyleSettings.GetWindowColor());
    m_pTitleFT->set_height_request(m_pTitleFT->get_preferred_size().Height() + 48);

    m_pOkBtn->SetClickHdl( LINK( this, SaveDialog, OKButtonHdl ) );
    m_pFileListLB->SetControlBackground( rStyleSettings.GetDialogColor() );

    // fill listbox with current open documents
    m_pFileListLB->Clear();

    TURLList&                rURLs = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;

    for (  pIt  = rURLs.begin();
           pIt != rURLs.end()  ;
         ++pIt                  )
    {
        const TURLInfo& rInfo = *pIt;
        m_pFileListLB->InsertEntry( rInfo.DisplayName, rInfo.StandardImage );
    }
}

SaveDialog::~SaveDialog()
{
    disposeOnce();
}

void SaveDialog::dispose()
{
    m_pTitleFT.clear();
    m_pFileListLB.clear();
    m_pOkBtn.clear();
    Dialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SaveDialog, OKButtonHdl, Button*, void)
{
    // start crash-save with progress
    ScopedVclPtrInstance< SaveProgressDialog > pProgress(this, m_pCore);
    short nResult = pProgress->Execute();
    pProgress.disposeAndClear();

    // if "CANCEL" => return "CANCEL"
    // if "OK"     => "AUTOLUNCH" always !
    if (nResult == DLG_RET_OK)
        nResult = DLG_RET_OK_AUTOLUNCH;

    EndDialog(nResult);
}

SaveProgressDialog::SaveProgressDialog(vcl::Window* pParent, RecoveryCore* pCore)
    : ModalDialog(pParent, "DocRecoveryProgressDialog",
        "svx/ui/docrecoveryprogressdialog.ui")
    , m_pCore(pCore)
{
    get(m_pProgrParent, "progress");
    Size aSize(LogicToPixel(Size(SAVEPROGR_CONTROLWIDTH, PROGR_HEIGHT)));
    m_pProgrParent->set_width_request(aSize.Width());
    m_pProgrParent->set_height_request(aSize.Height());

    PluginProgress* pProgress   = new PluginProgress(m_pProgrParent, pCore->getComponentContext());
    m_xProgress.set(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);
}

SaveProgressDialog::~SaveProgressDialog()
{
    disposeOnce();
}

void SaveProgressDialog::dispose()
{
    m_pProgrParent.clear();
    ModalDialog::dispose();
}

short SaveProgressDialog::Execute()
{
    ::SolarMutexGuard aLock;

    m_pCore->setProgressHandler(m_xProgress);
    m_pCore->setUpdateListener(this);
    m_pCore->doEmergencySave();
    short nRet = ModalDialog::Execute();
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
    EndDialog(DLG_RET_OK);
}


RecovDocListEntry::RecovDocListEntry(      SvTreeListEntry* pEntry,
                                           sal_uInt16       nFlags,
                                     const OUString&        sText )
    : SvLBoxString( pEntry, nFlags, sText )
{
}


void RecovDocListEntry::Paint(const Point& aPos, SvTreeListBox& aDevice, vcl::RenderContext& rRenderContext,
                              const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    const Image* pImg = nullptr;
    const OUString* pTxt = nullptr;
    RecovDocList* pList = static_cast<RecovDocList*>(&aDevice);

    TURLInfo* pInfo = static_cast<TURLInfo*>(rEntry.GetUserData());
    switch (pInfo->RecoveryState)
    {
        case E_SUCCESSFULLY_RECOVERED:
        {
            pImg = &pList->m_aGreenCheckImg;
            pTxt = &pList->m_aSuccessRecovStr;
        }
        break;

        case E_ORIGINAL_DOCUMENT_RECOVERED: // TODO must be renamed into ORIGINAL DOCUMENT recovered! Because its marked as yellow
        {
            pImg = &pList->m_aYellowCheckImg;
            pTxt = &pList->m_aOrigDocRecovStr;
        }
        break;

        case E_RECOVERY_FAILED:
        {
            pImg = &pList->m_aRedCrossImg;
            pTxt = &pList->m_aRecovFailedStr;
        }
        break;

        case E_RECOVERY_IS_IN_PROGRESS:
        {
            pImg = nullptr;
            pTxt = &pList->m_aRecovInProgrStr;
        }
        break;

        case E_NOT_RECOVERED_YET:
        {
            pImg = nullptr;
            pTxt = &pList->m_aNotRecovYetStr;
        }
        break;
    }

    if (pImg)
        rRenderContext.DrawImage(aPos, *pImg);

    if (pTxt)
    {
        Point aPnt(aPos);
        aPnt.X() += pList->m_aGreenCheckImg.GetSizePixel().Width();
        aPnt.X() += 10;
        rRenderContext.DrawText(aPnt, *pTxt);
    }
}

RecovDocList::RecovDocList(SvSimpleTableContainer& rParent, ResMgr &rResMgr)
    : SvSimpleTable      ( rParent )
    , m_aGreenCheckImg    ( ResId(RID_SVXIMG_GREENCHECK, rResMgr ) )
    , m_aYellowCheckImg   ( ResId(RID_SVXIMG_YELLOWCHECK, rResMgr ) )
    , m_aRedCrossImg      ( ResId(RID_SVXIMG_REDCROSS, rResMgr ) )
    , m_aSuccessRecovStr  ( ResId(RID_SVXSTR_SUCCESSRECOV, rResMgr ) )
    , m_aOrigDocRecovStr  ( ResId(RID_SVXSTR_ORIGDOCRECOV, rResMgr ) )
    , m_aRecovFailedStr   ( ResId(RID_SVXSTR_RECOVFAILED, rResMgr ) )
    , m_aRecovInProgrStr  ( ResId(RID_SVXSTR_RECOVINPROGR, rResMgr ) )
    , m_aNotRecovYetStr   ( ResId(RID_SVXSTR_NOTRECOVYET, rResMgr ) )
{
}

void RecovDocList::InitEntry(SvTreeListEntry* pEntry,
                             const OUString& rText,
                             const Image& rImage1,
                             const Image& rImage2,
                             SvLBoxButtonKind eButtonKind)
{
    SvTabListBox::InitEntry(pEntry, rText, rImage1, rImage2, eButtonKind);
    DBG_ASSERT( TabCount() == 2, "*RecovDocList::InitEntry(): structure missmatch" );

    SvLBoxString&       rCol = static_cast<SvLBoxString&>(pEntry->GetItem(2));
    std::unique_ptr<RecovDocListEntry> p(
            new RecovDocListEntry(pEntry, 0, rCol.GetText()));
    pEntry->ReplaceItem(std::move(p), 2);
}


short impl_askUserForWizardCancel(vcl::Window* pParent, sal_Int16 nRes)
{
    ScopedVclPtrInstance< MessageDialog > aQuery(pParent, SVX_RES(nRes), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
    if (aQuery->Execute() == RET_YES)
        return DLG_RET_OK;
    else
        return DLG_RET_CANCEL;
}

RecoveryDialog::RecoveryDialog(vcl::Window* pParent, RecoveryCore* pCore)
    : Dialog(pParent, "DocRecoveryRecoverDialog",
        "svx/ui/docrecoveryrecoverdialog.ui")
    , m_aTitleRecoveryInProgress(SVX_RESSTR(RID_SVXSTR_RECOVERY_INPROGRESS))
    , m_aRecoveryOnlyFinish (SVX_RESSTR(RID_SVXSTR_RECOVERYONLY_FINISH))
    , m_aRecoveryOnlyFinishDescr(SVX_RESSTR(RID_SVXSTR_RECOVERYONLY_FINISH_DESCR))
    , m_pCore(pCore)
    , m_eRecoveryState(RecoveryDialog::E_RECOVERY_PREPARED)
    , m_bWaitForCore(false)
    , m_bWasRecoveryStarted(false)
{
    get(m_pTitleFT, "title");
    get(m_pDescrFT, "desc");
    get(m_pProgrParent, "progress");
    m_pProgrParent->set_height_request(LogicToPixel(Size(0, PROGR_HEIGHT), MAP_APPFONT).Height());
    get(m_pNextBtn, "next");
    get(m_pCancelBtn, "cancel");

    SvSimpleTableContainer* pFileListLBContainer = get<SvSimpleTableContainer>("filelist");
    Size aSize(LogicToPixel(Size(RECOV_CONTROLWIDTH, RECOV_FILELISTHEIGHT), MAP_APPFONT));
    pFileListLBContainer->set_width_request(aSize.Width());
    pFileListLBContainer->set_height_request(aSize.Height());
    m_pFileListLB = VclPtr<RecovDocList>::Create(*pFileListLBContainer, DIALOG_MGR());

    static long nTabs[] = { 2, 0, 40*RECOV_CONTROLWIDTH/100 };
    m_pFileListLB->SetTabs( &nTabs[0] );
    m_pFileListLB->InsertHeaderEntry(get<FixedText>("nameft")->GetText() + "\t" + get<FixedText>("statusft")->GetText());

    PluginProgress* pProgress   = new PluginProgress(m_pProgrParent, pCore->getComponentContext());
    m_xProgress.set(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    m_pTitleFT->SetBackground(rStyleSettings.GetWindowColor());
    m_pTitleFT->set_height_request(m_pTitleFT->get_preferred_size().Height() + 48);

    m_pFileListLB->SetBackground( rStyleSettings.GetDialogColor() );

    m_pNextBtn->Enable();
    m_pNextBtn->SetClickHdl( LINK( this, RecoveryDialog, NextButtonHdl ) );
    m_pCancelBtn->SetClickHdl( LINK( this, RecoveryDialog, CancelButtonHdl ) );

    // fill list box first time
    TURLList&                rURLList = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;
    for (  pIt  = rURLList.begin();
           pIt != rURLList.end()  ;
         ++pIt                     )
    {
        const TURLInfo& rInfo = *pIt;

        OUString sName( rInfo.DisplayName );
        sName += "\t";
        sName += impl_getStatusString( rInfo );
        SvTreeListEntry* pEntry = m_pFileListLB->InsertEntry(sName, rInfo.StandardImage, rInfo.StandardImage);
        pEntry->SetUserData(const_cast<TURLInfo *>(&rInfo));
    }

    // mark first item
    SvTreeListEntry* pFirst = m_pFileListLB->First();
    if (pFirst)
        m_pFileListLB->SetCursor(pFirst, true);
}

RecoveryDialog::~RecoveryDialog()
{
    disposeOnce();
}

void RecoveryDialog::dispose()
{
    m_pFileListLB.disposeAndClear();
    m_pTitleFT.clear();
    m_pDescrFT.clear();
    m_pProgrParent.clear();
    m_pNextBtn.clear();
    m_pCancelBtn.clear();
    Dialog::dispose();
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
                m_pDescrFT->SetText( m_aTitleRecoveryInProgress );
                m_pNextBtn->Enable(false);
                m_pCancelBtn->Enable(false);
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
                 m_pDescrFT->SetText(m_aRecoveryOnlyFinishDescr);
                 m_pNextBtn->SetText(m_aRecoveryOnlyFinish);
                 m_pNextBtn->Enable();
                 m_pCancelBtn->Enable(false);
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
                 ScopedVclPtrInstance< BrokenRecoveryDialog > pBrokenRecoveryDialog(this, m_pCore, !m_bWasRecoveryStarted);
                 OUString              sSaveDir              = pBrokenRecoveryDialog->getSaveDirURL(); // get the default dir
                 if (pBrokenRecoveryDialog->isExecutionNeeded())
                 {
                     nRet = pBrokenRecoveryDialog->Execute();
                     sSaveDir = pBrokenRecoveryDialog->getSaveDirURL();
                 }
                 pBrokenRecoveryDialog.disposeAndClear();

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
                 short                 nRet                  = DLG_RET_UNKNOWN;
                 ScopedVclPtrInstance< BrokenRecoveryDialog > pBrokenRecoveryDialog(this, m_pCore, !m_bWasRecoveryStarted);
                 OUString              sSaveDir              = pBrokenRecoveryDialog->getSaveDirURL(); // get the default save location

                 // dialog itself checks if there is a need to copy files for this mode.
                 // It uses the information m_bWasRecoveryStarted doing so.
                 if (pBrokenRecoveryDialog->isExecutionNeeded())
                 {
                     nRet     = pBrokenRecoveryDialog->Execute();
                     sSaveDir = pBrokenRecoveryDialog->getSaveDirURL();
                 }
                 pBrokenRecoveryDialog.disposeAndClear();

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
    sal_uIntPtr c = m_pFileListLB->GetEntryCount();
    sal_uIntPtr i = 0;
    for ( i=0; i<c; ++i )
    {
        SvTreeListEntry* pEntry = m_pFileListLB->GetEntry(i);
        if ( !pEntry )
            continue;

        TURLInfo* pInfo = static_cast<TURLInfo*>(pEntry->GetUserData());
        if ( !pInfo )
            continue;

        OUString sStatus = impl_getStatusString( *pInfo );
        if ( !sStatus.isEmpty() )
            m_pFileListLB->SetEntryText( sStatus, pEntry, 1 );
    }

    m_pFileListLB->Invalidate();
    m_pFileListLB->Update();
}


void RecoveryDialog::stepNext(TURLInfo* pItem)
{
    sal_uIntPtr c = m_pFileListLB->GetEntryCount();
    sal_uIntPtr i = 0;
    for (i=0; i<c; ++i)
    {
        SvTreeListEntry* pEntry = m_pFileListLB->GetEntry(i);
        if (!pEntry)
            continue;

        TURLInfo* pInfo = static_cast<TURLInfo*>(pEntry->GetUserData());
        if (pInfo->ID != pItem->ID)
            continue;

        m_pFileListLB->SetCursor(pEntry, true);
        m_pFileListLB->MakeVisible(pEntry);
        m_pFileListLB->Invalidate();
        m_pFileListLB->Update();
        break;
    }
}

void RecoveryDialog::end()
{
    m_bWaitForCore = false;
}

IMPL_LINK_NOARG_TYPED(RecoveryDialog, NextButtonHdl, Button*, void)
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
        EndDialog(DLG_RET_OK);
    }
}

IMPL_LINK_NOARG_TYPED(RecoveryDialog, CancelButtonHdl, Button*, void)
{
    switch (m_eRecoveryState)
    {
        case RecoveryDialog::E_RECOVERY_PREPARED:
            if (impl_askUserForWizardCancel(this, RID_SVXSTR_QUERY_EXIT_RECOVERY) != DLG_RET_CANCEL)
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
        EndDialog();
    }
}

OUString RecoveryDialog::impl_getStatusString( const TURLInfo& rInfo ) const
{
    OUString sStatus;
    switch ( rInfo.RecoveryState )
    {
        case E_SUCCESSFULLY_RECOVERED :
            sStatus = m_pFileListLB->m_aSuccessRecovStr;
            break;
        case E_ORIGINAL_DOCUMENT_RECOVERED :
            sStatus = m_pFileListLB->m_aOrigDocRecovStr;
            break;
        case E_RECOVERY_FAILED :
            sStatus = m_pFileListLB->m_aRecovFailedStr;
            break;
        case E_RECOVERY_IS_IN_PROGRESS :
            sStatus = m_pFileListLB->m_aRecovInProgrStr;
            break;
        case E_NOT_RECOVERED_YET :
            sStatus = m_pFileListLB->m_aNotRecovYetStr;
            break;
        default:
            break;
    }
    return sStatus;
}

BrokenRecoveryDialog::BrokenRecoveryDialog(vcl::Window*       pParent        ,
                                           RecoveryCore* pCore          ,
                                           bool      bBeforeRecovery)
    : ModalDialog   ( pParent, "DocRecoveryBrokenDialog", "svx/ui/docrecoverybrokendialog.ui" )
    , m_pCore       ( pCore                                               )
    , m_bBeforeRecovery (bBeforeRecovery)
    , m_bExecutionNeeded(false)
{
    get(m_pFileListLB, "filelist");
    get(m_pSaveDirED, "savedir");
    get(m_pSaveDirBtn, "change");
    get(m_pOkBtn, "save");
    get(m_pCancelBtn, "cancel");

    m_pSaveDirBtn->SetClickHdl( LINK( this, BrokenRecoveryDialog, SaveButtonHdl ) );
    m_pOkBtn->SetClickHdl( LINK( this, BrokenRecoveryDialog, OkButtonHdl ) );
    m_pCancelBtn->SetClickHdl( LINK( this, BrokenRecoveryDialog, CancelButtonHdl ) );

    m_sSavePath = SvtPathOptions().GetWorkPath();
    INetURLObject aObj( m_sSavePath );
    OUString sPath;
    osl::FileBase::getSystemPathFromFileURL(aObj.GetMainURL( INetURLObject::NO_DECODE ), sPath);
    m_pSaveDirED->SetText( sPath );

    impl_refresh();
}

BrokenRecoveryDialog::~BrokenRecoveryDialog()
{
    disposeOnce();
}

void BrokenRecoveryDialog::dispose()
{
    m_pFileListLB.clear();
    m_pSaveDirED.clear();
    m_pSaveDirBtn.clear();
    m_pOkBtn.clear();
    m_pCancelBtn.clear();
    ModalDialog::dispose();
}


void BrokenRecoveryDialog::impl_refresh()
{
                             m_bExecutionNeeded = false;
    TURLList&                rURLList           = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;
    for (  pIt  = rURLList.begin();
           pIt != rURLList.end()  ;
         ++pIt                     )
    {
        const TURLInfo& rInfo = *pIt;

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

        const sal_Int32 nPos = m_pFileListLB->InsertEntry(rInfo.DisplayName, rInfo.StandardImage );
        m_pFileListLB->SetEntryData( nPos, const_cast<TURLInfo *>(&rInfo) );
    }
    m_sSavePath.clear();
    m_pOkBtn->GrabFocus();
}


bool BrokenRecoveryDialog::isExecutionNeeded()
{
    return m_bExecutionNeeded;
}


OUString BrokenRecoveryDialog::getSaveDirURL()
{
    return m_sSavePath;
}


IMPL_LINK_NOARG_TYPED(BrokenRecoveryDialog, OkButtonHdl, Button*, void)
{
    OUString sPhysicalPath = comphelper::string::strip(m_pSaveDirED->GetText(), ' ');
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath( sPhysicalPath, sURL );
    m_sSavePath = sURL;
    while (m_sSavePath.isEmpty())
        impl_askForSavePath();

    EndDialog(DLG_RET_OK);
}


IMPL_LINK_NOARG_TYPED(BrokenRecoveryDialog, CancelButtonHdl, Button*, void)
{
    EndDialog();
}


IMPL_LINK_NOARG_TYPED(BrokenRecoveryDialog, SaveButtonHdl, Button*, void)
{
    impl_askForSavePath();
}


void BrokenRecoveryDialog::impl_askForSavePath()
{
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker =
        css::ui::dialogs::FolderPicker::create( m_pCore->getComponentContext() );

    INetURLObject aURL(m_sSavePath, INetProtocol::File);
    xFolderPicker->setDisplayDirectory(aURL.GetMainURL(INetURLObject::NO_DECODE));
    short nRet = xFolderPicker->execute();
    if (nRet == css::ui::dialogs::ExecutableDialogResults::OK)
    {
        m_sSavePath = xFolderPicker->getDirectory();
        OUString sPath;
        osl::FileBase::getSystemPathFromFileURL(m_sSavePath, sPath);
        m_pSaveDirED->SetText( sPath );
    }
}

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
