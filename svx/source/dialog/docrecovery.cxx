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
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/scrbar.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/AutoRecovery.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include "svtools/treelistentry.hxx"
#include <officecfg/Office/Recovery.hxx>

namespace svx{
    namespace DocRecovery{

using namespace ::rtl;
using namespace ::osl;

//===============================================
TabDialog4Recovery::TabDialog4Recovery(Window* pParent)
    : TabDialog    (pParent, SVX_RES( RID_SVX_TABDLG_DOCRECOVERY ))
    , m_pActualPage(m_lTabPages.begin()                           )
{
}

//===============================================
TabDialog4Recovery::~TabDialog4Recovery()
{
    m_lTabPages.clear();
}

//===============================================
void TabDialog4Recovery::addTabPage(IExtendedTabPage* pPage)
{
    if (pPage)
        m_lTabPages.push_back(pPage);
}

//===============================================
short TabDialog4Recovery::Execute()
{
    ::SolarMutexGuard aLock;

    Show();
    m_pActualPage = m_lTabPages.begin();
    while(true)
    {
        IExtendedTabPage* pPage = *m_pActualPage;
        SetViewWindow(pPage);
        pPage->Show();
        pPage->setDefButton();
        short nRet = pPage->execute();
        pPage->Hide();

        switch(nRet)
        {
            case DLG_RET_OK :
                {
                    ++m_pActualPage;
                    if (m_pActualPage == m_lTabPages.end())
                        return nRet;
                }
                break;

            case DLG_RET_BACK :
                {
                    if (m_pActualPage != m_lTabPages.begin())
                        --m_pActualPage;
                }
                break;

            case DLG_RET_UNKNOWN :
            case DLG_RET_CANCEL :
            case DLG_RET_OK_AUTOLUNCH :
                return nRet;
        }
    }
}

//===============================================
RecoveryCore::RecoveryCore(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                 bool                                            bUsedForSaving)
    : m_xContext        ( rxContext    )
    , m_pListener       ( 0            )
    , m_bListenForSaving(bUsedForSaving)
{
    impl_startListening();
}

//===============================================
RecoveryCore::~RecoveryCore()
{
    impl_stopListening();
}

//===============================================
css::uno::Reference< css::uno::XComponentContext > RecoveryCore::getComponentContext()
{
    return m_xContext;
}

//===============================================
TURLList* RecoveryCore::getURLListAccess()
{
    return &m_lURLs;
}

//===============================================
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

//===============================================
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
        if (!RecoveryCore::isBrokenTempEntry(rInfo))
            continue;

        lCopyArgs[2].Value <<= rInfo.ID;
        m_xRealCore->dispatch(aCopyURL, lCopyArgs);
    }
}

//===============================================
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

//===============================================
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

//===============================================
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

//===============================================
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

//===============================================
void RecoveryCore::setProgressHandler(const css::uno::Reference< css::task::XStatusIndicator >& xProgress)
{
    m_xProgress = xProgress;
}

//===============================================
void RecoveryCore::setUpdateListener(IRecoveryUpdateListener* pListener)
{
    m_pListener = pListener;
}

//===============================================
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

//===============================================
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

//===============================================
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

//===============================================
ERecoveryState RecoveryCore::mapDocState2RecoverState(sal_Int32 eDocState)
{
    // ???
    ERecoveryState eRecState = E_NOT_RECOVERED_YET;

    /* Attention:
        Some of the following states can occure at the
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

//===============================================
void SAL_CALL RecoveryCore::statusChanged(const css::frame::FeatureStateEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // a) special notification about start/stop async dispatch!
    //    FeatureDescriptor = "start" || "stop"
    if (aEvent.FeatureDescriptor.equals(RECOVERY_OPERATIONSTATE_START))
    {
        if (m_pListener)
            m_pListener->start();
        return;
    }

    if (aEvent.FeatureDescriptor.equals(RECOVERY_OPERATIONSTATE_STOP))
    {
        if (m_pListener)
            m_pListener->end();
        return;
    }

    // b) normal notification about changed items
    //    FeatureDescriptor = "Update"
    //    State             = Lits of information [seq< NamedValue >]
    if (! aEvent.FeatureDescriptor.equals(RECOVERY_OPERATIONSTATE_UPDATE))
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
    String sURL = aNew.OrgURL;
    if (!sURL.Len())
        sURL = aNew.FactoryURL;
    if (!sURL.Len())
        sURL = aNew.TempURL;
    if (!sURL.Len())
        sURL = aNew.TemplateURL;
    INetURLObject aURL(sURL);
    aNew.StandardImage = SvFileInformationManager::GetFileImage(aURL, false);

    /* set the right UI state for this item to NOT_RECOVERED_YET ... because nDocState shows the state of
       the last emergency save operation before and is interessting for the used recovery core service only ...
       for now! But if there is a further notification for this item (see lines above!) we must
       map the doc state to an UI state. */
    aNew.RecoveryState = E_NOT_RECOVERED_YET;

    // patch DisplayName! Because the document title contain more then the file name ...
    sal_Int32 i = aNew.DisplayName.indexOf(" - ");
    if (i > 0)
        aNew.DisplayName = aNew.DisplayName.copy(0, i);

    m_lURLs.push_back(aNew);

    if (m_pListener)
        m_pListener->updateItems();
}

//===============================================
void SAL_CALL RecoveryCore::disposing(const css::lang::EventObject& /*aEvent*/)
    throw(css::uno::RuntimeException)
{
    m_xRealCore.clear();
}

//===============================================
void RecoveryCore::impl_startListening()
{
    // listening already initialized ?
    if (m_xRealCore.is())
        return;
    m_xRealCore = css::frame::AutoRecovery::create(m_xContext);

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

//===============================================
void RecoveryCore::impl_stopListening()
{
    // Ignore it, if this instance doesnt listen currently
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

//===============================================
css::util::URL RecoveryCore::impl_getParsedURL(const OUString& sURL)
{
    css::util::URL aURL;
    aURL.Complete = sURL;

    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
    xParser->parseStrict(aURL);

    return aURL;
}

//===============================================
PluginProgressWindow::PluginProgressWindow(      Window*                                       pParent  ,
                                           const css::uno::Reference< css::lang::XComponent >& xProgress)
    : Window     (pParent  )
    , m_xProgress(xProgress)
{
    Show();
    Size aParentSize = pParent->GetSizePixel();
    // align the progressbar to its parent
    setPosSizePixel( -9, 0, aParentSize.Width() + 15, aParentSize.Height() - 4 );
}

//===============================================
PluginProgressWindow::~PluginProgressWindow()
{
    if (m_xProgress.is())
        m_xProgress->dispose();
}

//===============================================
PluginProgress::PluginProgress(      Window*                                             pParent,
                               const css::uno::Reference< css::uno::XComponentContext >& xContext  )
{
    m_pPlugProgressWindow = new PluginProgressWindow(pParent, static_cast< css::lang::XComponent* >(this));
    css::uno::Reference< css::awt::XWindow > xProgressWindow = VCLUnoHelper::GetInterface(m_pPlugProgressWindow);
    m_xProgressFactory = css::task::StatusIndicatorFactory::createWithWindow(xContext, xProgressWindow, sal_False/*DisableReschedule*/, sal_True/*AllowParentShow*/);
    m_xProgress = m_xProgressFactory->createStatusIndicator();
}

//===============================================
PluginProgress::~PluginProgress()
{
}

//===============================================
void SAL_CALL PluginProgress::dispose()
    throw(css::uno::RuntimeException)
{
    // m_pPluginProgressWindow was deleted ...
    // So the internal pointer of this progress
    // weill be dead!
    m_xProgress.clear();
}

//===============================================
void SAL_CALL PluginProgress::addEventListener(const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException)
{
}

//===============================================
void SAL_CALL PluginProgress::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
    throw(css::uno::RuntimeException)
{
}

//===============================================
void SAL_CALL PluginProgress::start(const OUString&,
                                          sal_Int32        nRange)
    throw(css::uno::RuntimeException)
{
    if (m_xProgress.is())
        m_xProgress->start(OUString(), nRange);
}

//===============================================
void SAL_CALL PluginProgress::end()
    throw(css::uno::RuntimeException)
{
    if (m_xProgress.is())
        m_xProgress->end();
}

//===============================================
void SAL_CALL PluginProgress::setText(const OUString& sText)
    throw(css::uno::RuntimeException)
{
    if (m_xProgress.is())
        m_xProgress->setText(sText);
}

//===============================================
void SAL_CALL PluginProgress::setValue(sal_Int32 nValue)
    throw(css::uno::RuntimeException)
{
    if (m_xProgress.is())
        m_xProgress->setValue(nValue);
}

//===============================================
void SAL_CALL PluginProgress::reset()
    throw(css::uno::RuntimeException)
{
    if (m_xProgress.is())
        m_xProgress->reset();
}

//===============================================
SaveDialog::SaveDialog(Window*       pParent,
                       RecoveryCore* pCore  )
    : IExtendedTabPage( pParent, SVX_RES( RID_SVXPAGE_DOCRECOVERY_SAVE ) )
    , m_aTitleFT     ( this   , SVX_RES  ( FT_SAVE_TITLE               ) )
    , m_aTitleWin    ( this   , SVX_RES  ( WIN_SAVE_TITLE              ) )
    , m_aTitleFL     ( this   , SVX_RES  ( FL_SAVE_TITLE               ) )
    , m_aDescrFT     ( this   , SVX_RES  ( FT_SAVE_DESCR               ) )
    , m_aFileListFT  ( this   , SVX_RES  ( FT_SAVE_FILELIST            ) )
    , m_aFileListLB  ( this   , SVX_RES  ( LB_SAVE_FILELIST            ) )
    , m_aBottomFL    ( this   , SVX_RES  ( FL_SAVE_BOTTOM              ) )
    , m_aOkBtn       ( this   , SVX_RES  ( BT_SAVE_OK                  ) )
    , m_pCore        ( pCore                                           )
{
    FreeResource();

    // Prepare the office for the following crash save step.
    // E.g. hide all open widows so the user cant influence our
    // operation .-)
    m_pCore->doEmergencySavePrepare();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Wallpaper aBackground(rStyleSettings.GetWindowColor());
    m_aTitleWin.SetBackground(aBackground);
    m_aTitleFT.SetBackground (aBackground);

    Font aFont(m_aTitleFT.GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    m_aTitleFT.SetFont(aFont);

    m_aOkBtn.SetClickHdl( LINK( this, SaveDialog, OKButtonHdl ) );
    m_aFileListLB.SetControlBackground( rStyleSettings.GetDialogColor() );

    // fill listbox with current open documents
    m_aFileListLB.Clear();

    TURLList*                pURLs = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;

    for (  pIt  = pURLs->begin();
           pIt != pURLs->end()  ;
         ++pIt                  )
    {
        const TURLInfo& rInfo = *pIt;
        m_aFileListLB.InsertEntry( rInfo.DisplayName, rInfo.StandardImage );
    }
}

//===============================================
SaveDialog::~SaveDialog()
{
}

//===============================================
IMPL_LINK_NOARG(SaveDialog, OKButtonHdl)
{
    m_nResult = DLG_RET_OK;
    return 0;
}

//===============================================
short SaveDialog::execute()
{
    ::SolarMutexGuard aLock;

    // wait for user input "OK"
    m_nResult = DLG_RET_UNKNOWN;
    while(m_nResult == DLG_RET_UNKNOWN)
        Application::Yield();

    // start crash-save with progress
    if (m_nResult == DLG_RET_OK)
    {
        SaveProgressDialog* pProgress = new SaveProgressDialog(this, m_pCore);
        m_nResult = pProgress->Execute();
        delete pProgress;
    }
    // if "CANCEL" => return "CANCEL"
    // if "OK"     => "AUTOLUNCH" always !
    if (m_nResult == DLG_RET_OK)
        m_nResult = DLG_RET_OK_AUTOLUNCH;

    return m_nResult;
}

//===============================================
void SaveDialog::setDefButton()
{
    m_aOkBtn.GrabFocus();
}

//===============================================
SaveProgressDialog::SaveProgressDialog(Window*       pParent,
                                       RecoveryCore* pCore  )
    : ModalDialog   ( pParent        , SVX_RES( RID_SVX_MDLG_DOCRECOVERY_PROGR ) )
    , m_aHintFT     ( this           , SVX_RES  ( FT_SAVEPROGR_HINT              ) )
    , m_aProgrFT    ( this           , SVX_RES  ( FT_SAVEPROGR_PROGR             ) )
    , m_aProgrParent( this           , SVX_RES  ( WIN_SAVEPROGR_PROGR            ) )
    , m_pCore       ( pCore                                                      )
{
    FreeResource();
    PluginProgress* pProgress   = new PluginProgress( &m_aProgrParent, pCore->getComponentContext() );
    m_xProgress = css::uno::Reference< css::task::XStatusIndicator >(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);
}

//===============================================
SaveProgressDialog::~SaveProgressDialog()
{
}

//===============================================
short SaveProgressDialog::Execute()
{
    ::SolarMutexGuard aLock;

    m_pCore->setProgressHandler(m_xProgress);
    m_pCore->setUpdateListener(this);
    m_pCore->doEmergencySave();
    short nRet = ModalDialog::Execute();
    m_pCore->setUpdateListener(0);
    return nRet;
}

//===============================================
void SaveProgressDialog::updateItems()
{
}

//===============================================
void SaveProgressDialog::stepNext(TURLInfo* )
{
    /* TODO

        wenn die m_pCore noch ein Member m_nCurrentItem haette
        koennte man dort erkennen, wer gerade drann war, wer demnaechst
        dran ist ... Diese Info kann man dann in unserem Progress FixText anzeigen ...
    */
}

//===============================================
void SaveProgressDialog::start()
{
}

//===============================================
void SaveProgressDialog::end()
{
    EndDialog(DLG_RET_OK);
}

//===============================================
RecovDocListEntry::RecovDocListEntry(      SvTreeListEntry* pEntry,
                                           sal_uInt16       nFlags,
                                     const OUString&        sText )
    : SvLBoxString( pEntry, nFlags, sText )
{
}

//===============================================
void RecovDocListEntry::Paint(
    const Point& aPos, SvTreeListBox& aDevice, const SvViewDataEntry* /*pView*/, const SvTreeListEntry* pEntry)
{
    const Image*        pImg  = 0;
    const OUString*     pTxt  = 0;
          RecovDocList* pList = static_cast< RecovDocList* >(&aDevice);

    TURLInfo* pInfo  = (TURLInfo*)pEntry->GetUserData();
    switch(pInfo->RecoveryState)
    {
        case E_SUCCESSFULLY_RECOVERED :
        {
            pImg = &pList->m_aGreenCheckImg;
            pTxt = &pList->m_aSuccessRecovStr;
        }
        break;

        case E_ORIGINAL_DOCUMENT_RECOVERED : // TODO must be renamed into ORIGINAL DOCUMENT recovered! Because its marked as yellow
        {
            pImg = &pList->m_aYellowCheckImg;
            pTxt = &pList->m_aOrigDocRecovStr;
        }
        break;

        case E_RECOVERY_FAILED :
        {
            pImg = &pList->m_aRedCrossImg;
            pTxt = &pList->m_aRecovFailedStr;
        }
        break;

        case E_RECOVERY_IS_IN_PROGRESS :
        {
            pImg = 0;
            pTxt = &pList->m_aRecovInProgrStr;
        }
        break;

        case E_NOT_RECOVERED_YET :
        {
            pImg = 0;
            pTxt = &pList->m_aNotRecovYetStr;
        }
        break;
    }

    if (pImg)
        aDevice.DrawImage(aPos, *pImg);

    if (pTxt)
    {
        Point aPnt(aPos);
        aPnt.X() += pList->m_aGreenCheckImg.GetSizePixel().Width();
        aPnt.X() += 10;
        aDevice.DrawText(aPnt, *pTxt);
    }
}
//===============================================
RecovDocList::RecovDocList(SvSimpleTableContainer& rParent, ResMgr &rResMgr)
    : SvSimpleTable      ( rParent )
    , m_aGreenCheckImg    ( ResId(IMG_GREENCHECK, rResMgr ) )
    , m_aYellowCheckImg   ( ResId(IMG_YELLOWCHECK, rResMgr ) )
    , m_aRedCrossImg      ( ResId(IMG_REDCROSS, rResMgr ) )
    , m_aSuccessRecovStr  ( ResId(STR_SUCCESSRECOV, rResMgr ) )
    , m_aOrigDocRecovStr  ( ResId(STR_ORIGDOCRECOV, rResMgr ) )
    , m_aRecovFailedStr   ( ResId(STR_RECOVFAILED, rResMgr ) )
    , m_aRecovInProgrStr  ( ResId(STR_RECOVINPROGR, rResMgr ) )
    , m_aNotRecovYetStr   ( ResId(STR_NOTRECOVYET, rResMgr ) )
{
}

//===============================================
RecovDocList::~RecovDocList()
{
}

//===============================================
void RecovDocList::InitEntry(SvTreeListEntry* pEntry,
                             const OUString& rText,
                             const Image& rImage1,
                             const Image& rImage2,
                             SvLBoxButtonKind eButtonKind)
{
    SvTabListBox::InitEntry(pEntry, rText, rImage1, rImage2, eButtonKind);
    DBG_ASSERT( TabCount() == 2, "*RecovDocList::InitEntry(): structure missmatch" );

    SvLBoxString*       pCol = (SvLBoxString*)pEntry->GetItem(2);
    RecovDocListEntry*  p    = new RecovDocListEntry(pEntry, 0, pCol->GetText());
    pEntry->ReplaceItem(p, 2);
}

//===============================================
short impl_askUserForWizardCancel(Window* pParent, sal_Int16 nRes)
{
    QueryBox aQuery(pParent, SVX_RES(nRes));
    if (aQuery.Execute() == RET_YES)
        return DLG_RET_OK;
    else
        return DLG_RET_CANCEL;
}

//===============================================
RecoveryDialog::RecoveryDialog(Window*       pParent,
                               RecoveryCore* pCore  )
    : IExtendedTabPage( pParent      , SVX_RES( RID_SVXPAGE_DOCRECOVERY_RECOVER ) )
    , m_aTitleFT            ( this           , SVX_RES  ( FT_RECOV_TITLE                 ) )
    , m_aTitleWin           ( this           , SVX_RES  ( WIN_RECOV_TITLE                ) )
    , m_aTitleFL            ( this           , SVX_RES  ( FL_RECOV_TITLE                 ) )
    , m_aDescrFT            ( this           , SVX_RES  ( FT_RECOV_DESCR                 ) )
    , m_aProgressFT         ( this           , SVX_RES  ( FT_RECOV_PROGR                 ) )
    , m_aProgrParent        ( this           , SVX_RES  ( WIN_RECOV_PROGR                ) )
    , m_aFileListFT         ( this           , SVX_RES  ( FT_RECOV_FILELIST              ) )
    , m_aFileListLBContainer( this           , SVX_RES  ( LB_RECOV_FILELIST              ) )
    , m_aFileListLB         (m_aFileListLBContainer, DIALOG_MGR())
    , m_aBottomFL           ( this           , SVX_RES  ( FL_RECOV_BOTTOM                ) )
    , m_aNextBtn            ( this           , SVX_RES  ( BTN_RECOV_NEXT                 ) )
    , m_aCancelBtn          ( this           , SVX_RES  ( BTN_RECOV_CANCEL               ) )
    , m_aNextStr            (SVX_RESSTR(STR_RECOVERY_NEXT))
    , m_aTitleRecoveryInProgress(SVX_RESSTR(STR_RECOVERY_INPROGRESS))
    , m_aTitleRecoveryReport(SVX_RESSTR(STR_RECOVERY_REPORT))
    , m_aRecoveryOnlyFinish (SVX_RESSTR(STR_RECOVERYONLY_FINISH))
    , m_aRecoveryOnlyFinishDescr(SVX_RESSTR(STR_RECOVERYONLY_FINISH_DESCR))
    , m_pDefButton          ( NULL                                                       )
    , m_pCore               ( pCore                                                      )
    , m_eRecoveryState      (RecoveryDialog::E_RECOVERY_PREPARED)
    , m_bWaitForUser        (false)
    , m_bWaitForCore        (false)
    , m_bUserDecideNext     (false)
    , m_bWasRecoveryStarted (false)
    , m_bRecoveryOnly       (false)
{
    static long nTabs[] = { 2, 0, 40*RECOV_CONTROLWIDTH/100 };
    m_aFileListLB.SetTabs( &nTabs[0] );
    m_aFileListLB.InsertHeaderEntry(SVX_RESSTR(STR_HEADERBAR));

    FreeResource();

    bool bCrashRepEnabled(officecfg::Office::Recovery::CrashReporter::Enabled::get(pCore->getComponentContext()));
    m_bRecoveryOnly = !bCrashRepEnabled;

    PluginProgress* pProgress   = new PluginProgress( &m_aProgrParent, pCore->getComponentContext() );
    m_xProgress = css::uno::Reference< css::task::XStatusIndicator >(static_cast< css::task::XStatusIndicator* >(pProgress), css::uno::UNO_QUERY_THROW);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Wallpaper aBackground( rStyleSettings.GetWindowColor() );
    m_aTitleWin.SetBackground(aBackground);
    m_aTitleFT.SetBackground (aBackground);

    Font aFont(m_aTitleFT.GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    m_aTitleFT.SetFont(aFont);

    m_aFileListLB.SetBackground( rStyleSettings.GetDialogColor() );

    m_aNextBtn.Enable(sal_True);
    m_aNextBtn.SetClickHdl( LINK( this, RecoveryDialog, NextButtonHdl ) );
    m_aCancelBtn.SetClickHdl( LINK( this, RecoveryDialog, CancelButtonHdl ) );

    // fill list box first time
    TURLList*                pURLList = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;
    for (  pIt  = pURLList->begin();
           pIt != pURLList->end()  ;
         ++pIt                     )
    {
        const TURLInfo& rInfo = *pIt;

        String sName( rInfo.DisplayName );
        sName += '\t';
        sName += impl_getStatusString( rInfo );
        SvTreeListEntry* pEntry = m_aFileListLB.InsertEntry(sName, rInfo.StandardImage, rInfo.StandardImage);
        pEntry->SetUserData((void*)&rInfo);
    }

    // mark first item
    SvTreeListEntry* pFirst = m_aFileListLB.First();
    if (pFirst)
        m_aFileListLB.SetCursor(pFirst, sal_True);
}

//===============================================
RecoveryDialog::~RecoveryDialog()
{
}

//===============================================
short RecoveryDialog::execute()
{
    ::SolarMutexGuard aSolarLock;

    switch(m_eRecoveryState)
    {
        case RecoveryDialog::E_RECOVERY_PREPARED :
             {
                // Dialog was started first time ...
                // wait for user decision ("start" or "cancel" recovery)
                // This decision will be made inside the NextBtn handler.
                m_aNextBtn.Enable(sal_True);
                m_aCancelBtn.Enable(sal_True);
                m_bWaitForUser = true;
                while(m_bWaitForUser)
                    Application::Yield();
                if (m_bUserDecideNext)
                    m_eRecoveryState = RecoveryDialog::E_RECOVERY_IN_PROGRESS;
                else
                    m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED;
                return execute();
             }

        case RecoveryDialog::E_RECOVERY_IN_PROGRESS :
             {
                // user decided to start recovery ...
                m_bWasRecoveryStarted = true;
                // do it asynchronous (to allow repaints)
                // and wait for this asynchronous operation.
                m_aDescrFT.SetText( m_aTitleRecoveryInProgress );
                m_aNextBtn.Enable(sal_False);
                m_aCancelBtn.Enable(sal_False);
                m_pCore->setProgressHandler(m_xProgress);
                m_pCore->setUpdateListener(this);
                m_pCore->doRecovery();

                m_bWaitForCore = true;
                while(m_bWaitForCore)
                    Application::Yield();

                m_pCore->setUpdateListener(0);
                m_eRecoveryState = RecoveryDialog::E_RECOVERY_CORE_DONE;
                return execute();
             }

        case RecoveryDialog::E_RECOVERY_CORE_DONE :
             {
                 // the core finished it's task.
                 // let the user decide the next step.
                 if ( m_bRecoveryOnly )
                 {
                     m_aDescrFT.SetText(m_aRecoveryOnlyFinishDescr);
                     m_aNextBtn.SetText(m_aRecoveryOnlyFinish);
                     m_aNextBtn.Enable(sal_True);
                     m_aCancelBtn.Enable(sal_False);
                 }
                 else
                 {
                    m_aDescrFT.SetText(m_aTitleRecoveryReport);
                    m_aNextBtn.SetText(m_aNextStr);
                    m_aNextBtn.Enable(sal_True);
                    m_aCancelBtn.Enable(sal_True);
                 }

                 m_bWaitForUser = true;
                 while(m_bWaitForUser)
                     Application::Yield();

                if (m_bUserDecideNext)
                    m_eRecoveryState = RecoveryDialog::E_RECOVERY_DONE;
                else
                    m_eRecoveryState = RecoveryDialog::E_RECOVERY_CANCELED;
                return execute();
             }

        case RecoveryDialog::E_RECOVERY_DONE :
             {
                 // All documents was reovered.
                 // User decided to step to the "next" wizard page.
                 // Do it ... but check first, if there exist some
                 // failed recovery documents. They must be saved to
                 // a user selected directrory.
                 short                 nRet                  = DLG_RET_UNKNOWN;
                 BrokenRecoveryDialog* pBrokenRecoveryDialog = new BrokenRecoveryDialog(this, m_pCore, !m_bWasRecoveryStarted);
                 String                sSaveDir              = pBrokenRecoveryDialog->getSaveDirURL(); // get the default dir
                 if (pBrokenRecoveryDialog->isExecutionNeeded())
                 {
                     nRet = pBrokenRecoveryDialog->Execute();
                     sSaveDir = pBrokenRecoveryDialog->getSaveDirURL();
                 }
                 delete pBrokenRecoveryDialog;

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
                 BrokenRecoveryDialog* pBrokenRecoveryDialog = new BrokenRecoveryDialog(this, m_pCore, !m_bWasRecoveryStarted);
                 String                sSaveDir              = pBrokenRecoveryDialog->getSaveDirURL(); // get the default save location

                 // dialog itself checks if there is a need to copy files for this mode.
                 // It uses the information m_bWasRecoveryStarted doing so.
                 if (pBrokenRecoveryDialog->isExecutionNeeded())
                 {
                     nRet     = pBrokenRecoveryDialog->Execute();
                     sSaveDir = pBrokenRecoveryDialog->getSaveDirURL();
                 }
                 delete pBrokenRecoveryDialog;

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

        case RecoveryDialog::E_RECOVERY_HANDLED :
             {
                 m_bWaitForUser = true;
                 while(m_bWaitForUser)
                     Application::Yield();

                 // TODO: show BrokenRecoveryDialog again, ift he user
                 // doesnt accepted it last time.

                 if (m_bUserDecideNext)
                     return DLG_RET_OK;
                 else
                     return DLG_RET_CANCEL;
             }
    }

    // should never be reached .-)
    OSL_FAIL("Should never be reached!");
    return DLG_RET_OK;
}

//===============================================
void RecoveryDialog::setDefButton()
{
    if ( m_aNextBtn.IsEnabled() )
        m_aNextBtn.GrabFocus();
    else
        m_pDefButton = &m_aNextBtn;
}

//===============================================
void RecoveryDialog::start()
{
}

//===============================================
void RecoveryDialog::updateItems()
{
    sal_uIntPtr c = m_aFileListLB.GetEntryCount();
    sal_uIntPtr i = 0;
    for ( i=0; i<c; ++i )
    {
        SvTreeListEntry* pEntry = m_aFileListLB.GetEntry(i);
        if ( !pEntry )
            continue;

        TURLInfo* pInfo = (TURLInfo*)pEntry->GetUserData();
        if ( !pInfo )
            continue;

        String sStatus = impl_getStatusString( *pInfo );
        if ( sStatus.Len() > 0 )
            m_aFileListLB.SetEntryText( sStatus, pEntry, 1 );
    }

    m_aFileListLB.Invalidate();
    m_aFileListLB.Update();
}

//===============================================
void RecoveryDialog::stepNext(TURLInfo* pItem)
{
    sal_uIntPtr c = m_aFileListLB.GetEntryCount();
    sal_uIntPtr i = 0;
    for (i=0; i<c; ++i)
    {
        SvTreeListEntry* pEntry = m_aFileListLB.GetEntry(i);
        if (!pEntry)
            continue;

        TURLInfo* pInfo = (TURLInfo*)pEntry->GetUserData();
        if (pInfo->ID != pItem->ID)
            continue;

        m_aFileListLB.SetCursor(pEntry, sal_True);
        m_aFileListLB.MakeVisible(pEntry);
        m_aFileListLB.Invalidate();
        m_aFileListLB.Update();
        break;
    }
}

//===============================================
void RecoveryDialog::end()
{
    if ( m_pDefButton )
    {
        m_pDefButton->GrabFocus();
        m_pDefButton = NULL;
    }
    m_bWaitForCore = false;
}

//===============================================
IMPL_LINK_NOARG(RecoveryDialog, NextButtonHdl)
{
    m_bUserDecideNext = true;
    m_bWaitForUser    = false;
    return 0;
}

//===============================================
IMPL_LINK_NOARG(RecoveryDialog, CancelButtonHdl)
{
    if (m_eRecoveryState == RecoveryDialog::E_RECOVERY_PREPARED)
    {
        if (impl_askUserForWizardCancel(this, RID_SVXQB_EXIT_RECOVERY) == DLG_RET_CANCEL)
            return 0;
    }
    m_bUserDecideNext = false;
    m_bWaitForUser    = false;
    return 0;
}

//===============================================
OUString RecoveryDialog::impl_getStatusString( const TURLInfo& rInfo ) const
{
    OUString sStatus;
    switch ( rInfo.RecoveryState )
    {
        case E_SUCCESSFULLY_RECOVERED :
            sStatus = m_aFileListLB.m_aSuccessRecovStr;
            break;
        case E_ORIGINAL_DOCUMENT_RECOVERED :
            sStatus = m_aFileListLB.m_aOrigDocRecovStr;
            break;
        case E_RECOVERY_FAILED :
            sStatus = m_aFileListLB.m_aRecovFailedStr;
            break;
        case E_RECOVERY_IS_IN_PROGRESS :
            sStatus = m_aFileListLB.m_aRecovInProgrStr;
            break;
        case E_NOT_RECOVERED_YET :
            sStatus = m_aFileListLB.m_aNotRecovYetStr;
            break;
        default:
            break;
    }
    return sStatus;
}

//===============================================
BrokenRecoveryDialog::BrokenRecoveryDialog(Window*       pParent        ,
                                           RecoveryCore* pCore          ,
                                           bool      bBeforeRecovery)
    : ModalDialog   ( pParent, SVX_RES( RID_SVX_MDLG_DOCRECOVERY_BROKEN ) )
    , m_aDescrFT    ( this   , SVX_RES( FT_BROKEN_DESCR                   ) )
    , m_aFileListFT ( this   , SVX_RES( FT_BROKEN_FILELIST                ) )
    , m_aFileListLB ( this   , SVX_RES( LB_BROKEN_FILELIST                ) )
    , m_aSaveDirFT  ( this   , SVX_RES( FT_BROKEN_SAVEDIR                 ) )
    , m_aSaveDirED  ( this   , SVX_RES( ED_BROKEN_SAVEDIR                 ) )
    , m_aSaveDirBtn ( this   , SVX_RES( BTN_BROKEN_SAVEDIR                ) )
    , m_aBottomFL   ( this   , SVX_RES( FL_BROKEN_BOTTOM                  ) )
    , m_aOkBtn      ( this   , SVX_RES( BTN_BROKEN_OK                     ) )
    , m_aCancelBtn  ( this   , SVX_RES( BTN_BROKEN_CANCEL                 ) )
    , m_pCore       ( pCore                                               )
    , m_bBeforeRecovery (bBeforeRecovery)
    , m_bExecutionNeeded(sal_False)
{
    FreeResource();

    m_aSaveDirBtn.SetClickHdl( LINK( this, BrokenRecoveryDialog, SaveButtonHdl ) );
    m_aOkBtn.SetClickHdl( LINK( this, BrokenRecoveryDialog, OkButtonHdl ) );
    m_aCancelBtn.SetClickHdl( LINK( this, BrokenRecoveryDialog, CancelButtonHdl ) );

    m_sSavePath = SvtPathOptions().GetWorkPath();
    INetURLObject aObj( m_sSavePath );
    OUString sPath;
    ::utl::LocalFileHelper::ConvertURLToSystemPath( aObj.GetMainURL( INetURLObject::NO_DECODE ), sPath );
    m_aSaveDirED.SetText( sPath );

    impl_refresh();
}

//===============================================
BrokenRecoveryDialog::~BrokenRecoveryDialog()
{
}

//===============================================
void BrokenRecoveryDialog::impl_refresh()
{
                             m_bExecutionNeeded = sal_False;
    TURLList*                pURLList           = m_pCore->getURLListAccess();
    TURLList::const_iterator pIt;
    for (  pIt  = pURLList->begin();
           pIt != pURLList->end()  ;
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

        m_bExecutionNeeded = sal_True;

        sal_uInt16 nPos = m_aFileListLB.InsertEntry(rInfo.DisplayName, rInfo.StandardImage );
        m_aFileListLB.SetEntryData( nPos, (void*)&rInfo );
    }
    m_sSavePath = OUString();
    m_aOkBtn.GrabFocus();
}

//===============================================
sal_Bool BrokenRecoveryDialog::isExecutionNeeded()
{
    return m_bExecutionNeeded;
}

//===============================================
OUString BrokenRecoveryDialog::getSaveDirURL()
{
    return m_sSavePath;
}

//===============================================
IMPL_LINK_NOARG(BrokenRecoveryDialog, OkButtonHdl)
{
    String sPhysicalPath = comphelper::string::strip(m_aSaveDirED.GetText(), ' ');
    OUString sURL;
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sPhysicalPath, sURL );
    m_sSavePath = sURL;
    while (m_sSavePath.isEmpty())
        impl_askForSavePath();

    EndDialog(DLG_RET_OK);
    return 0;
}

//===============================================
IMPL_LINK_NOARG(BrokenRecoveryDialog, CancelButtonHdl)
{
    EndDialog(DLG_RET_CANCEL);
    return 0;
}

//===============================================
IMPL_LINK_NOARG(BrokenRecoveryDialog, SaveButtonHdl)
{
    impl_askForSavePath();
    return 0;
}

//===============================================
void BrokenRecoveryDialog::impl_askForSavePath()
{
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker =
        css::ui::dialogs::FolderPicker::create( m_pCore->getComponentContext() );

    INetURLObject aURL(m_sSavePath, INET_PROT_FILE);
    xFolderPicker->setDisplayDirectory(aURL.GetMainURL(INetURLObject::NO_DECODE));
    short nRet = xFolderPicker->execute();
    if (nRet == css::ui::dialogs::ExecutableDialogResults::OK)
    {
        m_sSavePath = xFolderPicker->getDirectory();
        OUString sPath;
        ::utl::LocalFileHelper::ConvertURLToSystemPath( m_sSavePath, sPath );
        m_aSaveDirED.SetText( sPath );
    }
}

//===============================================
    ///////////////////////////////////////////////////////////////////////
    // Error Report Welcome Dialog
    ///////////////////////////////////////////////////////////////////////

    ErrorRepWelcomeDialog::ErrorRepWelcomeDialog( Window* _pParent, sal_Bool _bAllowBack )
            :IExtendedTabPage        ( _pParent, SVX_RES( RID_SVXPAGE_ERR_REP_WELCOME ) )
            ,maTitleWin     ( this, SVX_RES( WIN_RECOV_TITLE ) )
            ,maTitleFT      ( this, SVX_RES( FT_RECOV_TITLE ) )
            ,maTitleFL      ( this, SVX_RES( FL_RECOV_TITLE ) )
            ,maDescrFT      ( this, SVX_RES( FT_RECOV_DESCR ) )
            ,maBottomFL     ( this, SVX_RES( FL_RECOV_BOTTOM ) )
            ,maPrevBtn      ( this, SVX_RES( BTN_RECOV_PREV ) )
            ,maNextBtn      ( this, SVX_RES( BTN_RECOV_NEXT ) )
            ,maCancelBtn    ( this, SVX_RES( BTN_RECOV_CANCEL ) )
        {
            FreeResource();

            Wallpaper       aBack( GetSettings().GetStyleSettings().GetWindowColor() );
            maTitleWin.SetBackground( aBack );
            maTitleFT.SetBackground( aBack );

            Font    aFnt( maTitleFT.GetFont() );
            aFnt.SetWeight( WEIGHT_BOLD );
            maTitleFT.SetFont( aFnt );

            maPrevBtn.SetClickHdl( LINK( this, ErrorRepWelcomeDialog, PrevBtnHdl ) );
            maPrevBtn.Enable( _bAllowBack );

            maNextBtn.SetClickHdl( LINK( this, ErrorRepWelcomeDialog, NextBtnHdl ) );
            maNextBtn.Enable( sal_True );

            maCancelBtn.SetClickHdl( LINK( this, ErrorRepWelcomeDialog, CancelBtnHdl ) );
            maCancelBtn.Enable( sal_True );
        }

        ErrorRepWelcomeDialog::~ErrorRepWelcomeDialog()
        {
        }

        IMPL_LINK_NOARG(ErrorRepWelcomeDialog, PrevBtnHdl)
        {
            m_nResult = DLG_RET_BACK;
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepWelcomeDialog, NextBtnHdl)
        {
            m_nResult = DLG_RET_OK;
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepWelcomeDialog, CancelBtnHdl)
        {
            m_nResult = DLG_RET_CANCEL;
            return 0;
        }

        short ErrorRepWelcomeDialog::execute()
        {
            ::SolarMutexGuard aLock;
            Show();
            m_nResult = DLG_RET_UNKNOWN;
            while(m_nResult == DLG_RET_UNKNOWN)
                Application::Yield();
            return m_nResult;
        }

        void ErrorRepWelcomeDialog::setDefButton()
        {
            maNextBtn.GrabFocus();
        }

    ///////////////////////////////////////////////////////////////////////
    // Error Report Send Dialog and its MultiLineEdit
    ///////////////////////////////////////////////////////////////////////

        ErrorDescriptionEdit::ErrorDescriptionEdit( Window* pParent, const ResId& rResId ) :

            MultiLineEdit( pParent, rResId )

        {
            SetModifyHdl( LINK( this, ErrorDescriptionEdit, ModifyHdl ) );
            if ( GetVScrollBar() )
                GetVScrollBar()->Hide();
        }

        ErrorDescriptionEdit::~ErrorDescriptionEdit()
        {
        }

        IMPL_LINK_NOARG(ErrorDescriptionEdit, ModifyHdl)
        {
            if ( !GetVScrollBar() )
                return 0;

            ExtTextEngine* pTextEngine = GetTextEngine();
            DBG_ASSERT( pTextEngine, "no text engine" );

            sal_uIntPtr i, nParaCount = pTextEngine->GetParagraphCount();
            sal_uInt16 nLineCount = 0;

            for ( i = 0; i < nParaCount; ++i )
                nLineCount = nLineCount + pTextEngine->GetLineCount(i);

            sal_uInt16 nVisCols = 0, nVisLines = 0;
            GetMaxVisColumnsAndLines( nVisCols, nVisLines );
            GetVScrollBar()->Show( nLineCount > nVisLines );

            return 0;
        }

        ErrorRepSendDialog::ErrorRepSendDialog( Window* _pParent )
            :IExtendedTabPage       ( _pParent, SVX_RES( RID_SVXPAGE_ERR_REP_SEND ) )
            ,maTitleWin     ( this, SVX_RES( WIN_RECOV_TITLE ) )
            ,maTitleFT      ( this, SVX_RES( FT_RECOV_TITLE ) )
            ,maTitleFL      ( this, SVX_RES( FL_RECOV_TITLE ) )
            ,maDescrFT      ( this, SVX_RES( FT_RECOV_DESCR ) )

            ,maDocTypeFT    ( this, SVX_RES( FT_ERRSEND_DOCTYPE ) )
            ,maDocTypeED    ( this, SVX_RES( ED_ERRSEND_DOCTYPE ) )
            ,maUsingFT      ( this, SVX_RES( FT_ERRSEND_USING ) )
            ,maUsingML      ( this, SVX_RES( ML_ERRSEND_USING ) )
            ,maShowRepBtn   ( this, SVX_RES( BTN_ERRSEND_SHOWREP ) )
            ,maOptBtn       ( this, SVX_RES( BTN_ERRSEND_OPT ) )
            ,maContactCB    ( this, SVX_RES( CB_ERRSEND_CONTACT ) )
            ,maEMailAddrFT  ( this, SVX_RES( FT_ERRSEND_EMAILADDR ) )
            ,maEMailAddrED  ( this, SVX_RES( ED_ERRSEND_EMAILADDR ) )

            ,maBottomFL     ( this, SVX_RES( FL_RECOV_BOTTOM ) )
            ,maPrevBtn      ( this, SVX_RES( BTN_RECOV_PREV ) )
            ,maNextBtn      ( this, SVX_RES( BTN_RECOV_NEXT ) )
            ,maCancelBtn    ( this, SVX_RES( BTN_RECOV_CANCEL ) )
        {
            FreeResource();

            initControls();

            Wallpaper aBack( GetSettings().GetStyleSettings().GetWindowColor() );
            maTitleWin.SetBackground( aBack );
            maTitleFT.SetBackground( aBack );

            Font aFnt( maTitleFT.GetFont() );
            aFnt.SetWeight( WEIGHT_BOLD );
            maTitleFT.SetFont( aFnt );

            maShowRepBtn.SetClickHdl( LINK( this, ErrorRepSendDialog, ShowRepBtnHdl ) );
            maOptBtn.SetClickHdl( LINK( this, ErrorRepSendDialog, OptBtnHdl ) );
            maContactCB.SetClickHdl( LINK( this, ErrorRepSendDialog, ContactCBHdl ) );
            maPrevBtn.SetClickHdl( LINK( this, ErrorRepSendDialog, PrevBtnHdl ) );
            maNextBtn.SetClickHdl( LINK( this, ErrorRepSendDialog, SendBtnHdl ) );
            maCancelBtn.SetClickHdl( LINK( this, ErrorRepSendDialog, CancelBtnHdl ) );

            ReadParams();

            ContactCBHdl( 0 );
        }

        ErrorRepSendDialog::~ErrorRepSendDialog()
        {
        }

        short ErrorRepSendDialog::execute()
        {
            ::SolarMutexGuard aLock;
            Show();
            m_nResult = DLG_RET_UNKNOWN;
            while(m_nResult == DLG_RET_UNKNOWN)
                Application::Yield();
            return m_nResult;
        }

        void ErrorRepSendDialog::setDefButton()
        {
            // set first focus
            maDocTypeED.GrabFocus();
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, PrevBtnHdl)
        {
            m_nResult = DLG_RET_BACK;
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, CancelBtnHdl)
        {
            m_nResult = DLG_RET_CANCEL;
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, SendBtnHdl)
        {

            SaveParams();
            SendReport();

            m_nResult = DLG_RET_OK;
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, ShowRepBtnHdl)
        {
            ErrorRepPreviewDialog aDlg( this );
            aDlg.Execute();
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, OptBtnHdl)
        {
            ErrorRepOptionsDialog aDlg( this, maParams );
            aDlg.Execute();
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepSendDialog, ContactCBHdl)
        {
            bool    bCheck = maContactCB.IsChecked();
            maEMailAddrFT.Enable( bCheck );
            maEMailAddrED.Enable( bCheck );
            return 0;
        }

        void ErrorRepSendDialog::initControls()
        {
            // if the text is too short for two lines, insert a newline
            String sText = maDocTypeFT.GetText();
            if ( maDocTypeFT.GetCtrlTextWidth( sText ) <= maDocTypeFT.GetSizePixel().Width() )
            {
                sText.Insert( '\n', 0 );
                maDocTypeFT.SetText( sText );
            }

            // if the button text is too wide, then broaden the button
            sText = maShowRepBtn.GetText();
            long nTxtW = maShowRepBtn.GetCtrlTextWidth( sText );
            long nBtnW = maShowRepBtn.GetSizePixel().Width();
            if ( nTxtW >= nBtnW )
            {
                const long nMinDelta = 10;
                long nDelta = std::max( nTxtW - nBtnW, nMinDelta );
                sal_uInt32 i = 0;
                Window* pWins[] =
                {
                    &maShowRepBtn, &maOptBtn,
                    &maDescrFT, &maDocTypeFT, &maDocTypeED, &maUsingFT,
                    &maUsingML, &maContactCB, &maEMailAddrFT, &maEMailAddrED
                };
                // the first two buttons need a new size (wider) and position (more left)
                Window** pCurrent = pWins;
                const sal_uInt32 nBtnCount = 2;
                for ( ; i < nBtnCount; ++i, ++pCurrent )
                {
                    Size aNewSize = (*pCurrent)->GetSizePixel();
                    aNewSize.Width() += nDelta;
                    (*pCurrent)->SetSizePixel( aNewSize );
                    Point aNewPos = (*pCurrent)->GetPosPixel();
                    aNewPos.X() -= nDelta;
                    (*pCurrent)->SetPosPixel( aNewPos );
                }

                // loop through all the other windows and adjust their size
                for ( ; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
                {
                    Size aSize = (*pCurrent)->GetSizePixel();
                    aSize.Width() -= nDelta;
                    (*pCurrent)->SetSizePixel( aSize );
                }
            }
        }

        OUString ErrorRepSendDialog::GetDocType( void ) const
        {
            return maDocTypeED.GetText();
        }

        OUString ErrorRepSendDialog::GetUsing( void ) const
        {
            return maUsingML.GetText();
        }

        bool ErrorRepSendDialog::IsContactAllowed( void ) const
        {
            return maContactCB.IsChecked();
        }

        OUString ErrorRepSendDialog::GetEMailAddress( void ) const
        {
            return maEMailAddrED.GetText();
        }


    ///////////////////////////////////////////////////////////////////////
    // Error Report Options Dialog
    ///////////////////////////////////////////////////////////////////////

        ErrorRepOptionsDialog::ErrorRepOptionsDialog( Window* _pParent, ErrorRepParams& _rParams )
            :ModalDialog    ( _pParent, SVX_RES( RID_SVX_MDLG_ERR_REP_OPTIONS ) )
            ,maProxyFL( this, SVX_RES( FL_ERROPT_PROXY ) )
            ,maSystemBtn( this, SVX_RES( BTN_ERROPT_SYSTEM ) )
            ,maDirectBtn( this, SVX_RES( BTN_ERROPT_DIRECT ) )
            ,maManualBtn( this, SVX_RES( BTN_ERROPT_MANUAL ) )
            ,maProxyServerFT( this, SVX_RES( FT_ERROPT_PROXYSERVER ) )
            ,maProxyServerEd( this, SVX_RES( ED_ERROPT_PROXYSERVER ) )
            ,maProxyPortFT( this, SVX_RES( FT_ERROPT_PROXYPORT ) )
            ,maProxyPortEd( this, SVX_RES( ED_ERROPT_PROXYPORT ) )
            ,maDescriptionFT( this, SVX_RES( FT_ERROPT_DESCRIPTION ) )
            ,maButtonsFL( this, SVX_RES( FL_ERROPT_BUTTONS ) )
            ,maOKBtn( this, SVX_RES( BTN_ERROPT_OK ) )
            ,maCancelBtn( this, SVX_RES( BTN_ERROPT_CANCEL ) )
            ,mrParams( _rParams )
        {
            FreeResource();

            maManualBtn.SetToggleHdl( LINK( this, ErrorRepOptionsDialog, ManualBtnHdl ) );
            maCancelBtn.SetClickHdl( LINK( this, ErrorRepOptionsDialog, CancelBtnHdl ) );
            maOKBtn.SetClickHdl( LINK( this, ErrorRepOptionsDialog, OKBtnHdl ) );

            maProxyServerEd.SetText( mrParams.maHTTPProxyServer );
            maProxyPortEd.SetText( mrParams.maHTTPProxyPort );

#ifndef WNT
            // no "Use system settings" button on non windows systems
            // so hide this button
            maSystemBtn.Hide();
            long nDelta = maDirectBtn.GetPosPixel().Y() - maSystemBtn.GetPosPixel().Y();
            // and loop through all these controls and adjust their position
            Window* pWins[] =
            {
                &maDirectBtn, &maManualBtn, &maProxyServerFT,
                &maProxyServerEd, &maProxyPortFT, &maProxyPortEd, &maDescriptionFT
            };
            Window** pCurrent = pWins;
            for ( sal_uInt32 i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
            {
                Point aPos = (*pCurrent)->GetPosPixel();
                aPos.Y() -= nDelta;
                (*pCurrent)->SetPosPixel( aPos );
            }
#endif


            switch ( mrParams.miHTTPConnectionType )
            {
            default:
#ifdef WNT
            case 0:
                maSystemBtn.Check( sal_True );
                break;
#endif
            case 1:
                maDirectBtn.Check( sal_True );
                break;
            case 2:
                maManualBtn.Check( sal_True );
                break;
            }

            ManualBtnHdl( 0 );
        }

        ErrorRepOptionsDialog::~ErrorRepOptionsDialog()
        {
        }

        IMPL_LINK_NOARG(ErrorRepOptionsDialog, ManualBtnHdl)
        {
            bool    bCheck = maManualBtn.IsChecked();
            maProxyServerFT.Enable( bCheck );
            maProxyServerEd.Enable( bCheck );
            maProxyPortFT.Enable( bCheck );
            maProxyPortEd.Enable( bCheck );
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepOptionsDialog, OKBtnHdl)
        {
            if ( maManualBtn.IsChecked() )
                mrParams.miHTTPConnectionType = 2;
            else if ( maDirectBtn.IsChecked() )
                mrParams.miHTTPConnectionType = 1;
            else if ( maSystemBtn.IsChecked() )
                mrParams.miHTTPConnectionType = 0;

            mrParams.maHTTPProxyServer = maProxyServerEd.GetText();
            mrParams.maHTTPProxyPort = maProxyPortEd.GetText();

            EndDialog(DLG_RET_OK);
            return 0;
        }

        IMPL_LINK_NOARG(ErrorRepOptionsDialog, CancelBtnHdl)
        {
            EndDialog(DLG_RET_CANCEL);
            return 0;
        }

    ///////////////////////////////////////////////////////////////////////
    // Error Report Edit (MultiLineEdit with fixed font)
    ///////////////////////////////////////////////////////////////////////

        ErrorRepEdit::ErrorRepEdit( Window* pParent, const ResId& rResId ) :
            ExtMultiLineEdit( pParent, rResId )
        {
            // fixed font for error report
            Color   aColor  = GetTextColor();

            Font aFont = OutputDevice::GetDefaultFont(
                DEFAULTFONT_FIXED, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE );

            // Set font color because the default font color is transparent !!!
            aFont.SetColor( aColor );

            GetTextEngine()->SetFont( aFont );

            // no blinking cursor and a little left margin
            EnableCursor( sal_False );
            SetLeftMargin( 4 );
        }

        ErrorRepEdit::~ErrorRepEdit()
        {
        }

    ///////////////////////////////////////////////////////////////////////
    // Error Report Preview Dialog
    ///////////////////////////////////////////////////////////////////////


        static OUString GetCrashConfigDir()
        {

#if defined(WNT)
            OUString    ustrValue = OUString("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/bootstrap.ini:UserInstallation}");
#elif defined( MACOSX )
            OUString    ustrValue = OUString("~");
#else
            OUString    ustrValue = OUString("$SYSUSERCONFIG");
#endif
            Bootstrap::expandMacros( ustrValue );

#if defined(WNT)
            ustrValue += "/user/crashdata";
#endif
            return ustrValue;
        }

#if defined(WNT)
#define PRVFILE "crashdat.prv"
#else
#define PRVFILE ".crash_report_preview"
#endif

        static OUString GetPreviewURL()
        {
            OUString aURL = GetCrashConfigDir();

            aURL += OUString( "/"  );
            aURL += OUString( PRVFILE  );

            return aURL;
        }

        static String LoadCrashFile( const OUString &rURL )
        {
            String  aFileContent;
            ::osl::File aFile( rURL );

            printf( "Loading %s:", OString( rURL.getStr(), rURL.getLength(), osl_getThreadTextEncoding() ).getStr() );
            if ( ::osl::FileBase::E_None == aFile.open( osl_File_OpenFlag_Read ) )
            {
                OString  aContent;
                ::osl::FileBase::RC result;
                sal_uInt64  aBytesRead;

                do
                {
                    sal_Char    aBuffer[256];

                    result = aFile.read( aBuffer, sizeof(aBuffer), aBytesRead );

                    if ( ::osl::FileBase::E_None == result )
                    {
                        OString  aTemp( aBuffer, static_cast< xub_StrLen >( aBytesRead ) );
                        aContent += aTemp;
                    }
                } while ( ::osl::FileBase::E_None == result && aBytesRead );

                OUString ustrContent( aContent.getStr(), aContent.getLength(), RTL_TEXTENCODING_UTF8 );
                aFileContent = ustrContent;

                aFile.close();

                printf( "SUCCEEDED\n" );
            }
            else
                printf( "FAILED\n" );

            return aFileContent;
        }



        ErrorRepPreviewDialog::ErrorRepPreviewDialog( Window* _pParent )
            :ModalDialog    ( _pParent, SVX_RES( RID_SVX_MDLG_ERR_REP_PREVIEW ) )
            ,maContentML( this, SVX_RES( ML_ERRPREVIEW_CONTENT ) )
            ,maOKBtn( this, SVX_RES( BTN_ERRPREVIEW_OK ) )

        {
            FreeResource();

            mnMinHeight = ( maContentML.GetSizePixel().Height() / 2 );

            String  aPreview = LoadCrashFile( GetPreviewURL() );
            ErrorRepSendDialog *pMainDlg = (ErrorRepSendDialog *)_pParent;

            String aSeparator = OUString( "\r\n\r\n================\r\n\r\n"  );

            String aContent = pMainDlg->GetDocType();
            if ( aContent.Len() > 0 )
                aContent += aSeparator;
            aContent += pMainDlg->GetUsing();
            if ( aContent.Len() > 0 )
                aContent += aSeparator;
            aContent += aPreview;

            maContentML.SetText( aContent );
        }

        ErrorRepPreviewDialog::~ErrorRepPreviewDialog()
        {
        }

        void ErrorRepPreviewDialog::Resize()
        {
            Size a3Sz = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
            Size aWinSz = GetSizePixel();
            Size aBtnSz = maOKBtn.GetSizePixel();
            Point aEditPnt = maContentML.GetPosPixel();

            long nNewHeight = std::max( aWinSz.Height() - aEditPnt.Y() - 3 * a3Sz.Height() - aBtnSz.Height(), mnMinHeight );
            long nNewWidth = aWinSz.Width() - 4 * a3Sz.Width();

            Size aNewSize( nNewWidth, nNewHeight );
            maContentML.SetSizePixel( aNewSize );
            Point aNewPoint( std::max( aEditPnt.X() + aNewSize.Width() - aBtnSz.Width(), aEditPnt.X() ),
                             aEditPnt.Y() + aNewSize.Height() + a3Sz.Height() );
            maOKBtn.SetPosPixel( aNewPoint );
        }
    }   // namespace DocRecovery
}   // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
