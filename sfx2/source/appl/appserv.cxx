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
#include <config_wasm_strip.h>

#include <com/sun/star/drawing/ModuleDispatcher.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/text/ModuleDispatcher.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/AddressBookSourcePilot.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include <svtools/addresstemplate.hxx>
#include <svtools/restartdialog.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/visitem.hxx>

#include <unotools/configmgr.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/weld.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basic/sberrors.hxx>
#include <vcl/help.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/toolbox.hxx>

#include <unotools/moduleoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <rtl/bootstrap.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <appdata.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <sorgitm.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/templatedlg.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/safemode.hxx>
#include <sfx2/sfxuno.hxx>
#include <DevelopmentToolDockingWindow.hxx>

#include <comphelper/types.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <unotools/confignode.hxx>
#include <memory>

#include <openuriexternally.hxx>

#include "getbasctlfunction.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;

namespace
{
    OUString lcl_getAppName( vcl::EnumContext::Application eApp )
    {
        switch ( eApp )
        {
            case vcl::EnumContext::Application::Writer:
                return u"Writer"_ustr;
            case vcl::EnumContext::Application::Calc:
                return u"Calc"_ustr;
            case vcl::EnumContext::Application::Impress:
                return u"Impress"_ustr;
            case vcl::EnumContext::Application::Draw:
                return u"Draw"_ustr;
            case vcl::EnumContext::Application::Formula:
                return u"Formula"_ustr;
            case vcl::EnumContext::Application::Base:
                return u"Base"_ustr;
            default:
                return OUString();
        }
    }

    // lp#527938, debian#602953, fdo#33266, i#105408
    bool lcl_isBaseAvailable()
    {
        try
        {
            // if we get css::sdbc::DriverManager, libsdbc2 is there
            // and the bibliography is assumed to work
            return css::sdbc::DriverManager::create(comphelper::getProcessComponentContext()).is();
        }
        catch (const Exception &)
        {
            TOOLS_INFO_EXCEPTION("sfx.appl", "assuming Base to be missing");
            return false;
        }
    }
    void lcl_tryLoadBibliography()
    {
        // lp#527938, debian#602953, fdo#33266, i#105408
        // make sure we actually can instantiate services from base first
        if(!lcl_isBaseAvailable())
        {
            if (officecfg::Office::Common::PackageKit::EnableBaseInstallation::get())
            {
                try
                {
                    using namespace org::freedesktop::PackageKit;
                    using namespace svtools;
                    Reference< XSyncDbusSessionHelper > xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
                    Sequence< OUString > vPackages { u"libreoffice-base"_ustr };
                    xSyncDbusSessionHelper->InstallPackageNames(vPackages, OUString());
                    // I'll be back (hopefully)!
                    SolarMutexGuard aGuard;
                    executeRestartDialog(comphelper::getProcessComponentContext(), nullptr, RESTART_REASON_BIBLIOGRAPHY_INSTALL);
                }
                catch (const Exception &)
                {
                    TOOLS_INFO_EXCEPTION("sfx.appl", "trying to install LibreOffice Base");
                }
            }
            return;
        }

        try // fdo#48775
        {
            SfxStringItem aURL(SID_FILE_NAME, u".component:Bibliography/View1"_ustr);
            SfxStringItem aRef(SID_REFERER, u"private:user"_ustr);
            SfxStringItem aTarget(SID_TARGETNAME, u"_blank"_ustr);
            if (const SfxViewFrame* pViewFrame = SfxViewFrame::Current())
            {
                pViewFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON, { &aURL, &aRef, &aTarget });
            }
        }
        catch (const Exception &)
        {
            TOOLS_INFO_EXCEPTION( "sfx.appl", "trying to load bibliography database");
        }
    }
    void lcl_disableActiveEmbeddedObjects(const SfxObjectShell* pObjSh)
    {
        if (!pObjSh)
            return;

        comphelper::EmbeddedObjectContainer& rContainer = pObjSh->getEmbeddedObjectContainer();
        if (!rContainer.HasEmbeddedObjects())
            return;

        const uno::Sequence<OUString> aNames = rContainer.GetObjectNames();
        for (const auto& rName : aNames)
        {
            uno::Reference<embed::XEmbeddedObject> xEmbeddedObj
                = rContainer.GetEmbeddedObject(rName);
            if (!xEmbeddedObj.is())
                continue;

            try
            {
                if (xEmbeddedObj->getCurrentState() != embed::EmbedStates::LOADED)
                {
                    xEmbeddedObj->changeState(embed::EmbedStates::LOADED);
                }
            }
            catch (const uno::Exception&)
            {
            }
        }
    }
}
/// Find the correct location of the document (CREDITS.fodt, etc.), and return
/// it in rURL if found.
static bool checkURL( const char *pName, const char *pExt, OUString &rURL )
{
    using namespace osl;
    DirectoryItem aDirItem;

#ifdef MACOSX
    rURL = "$BRAND_BASE_DIR/Resources/" + OUString::createFromAscii( pName ) +
           OUString::createFromAscii( pExt );
#else
    rURL = "$BRAND_BASE_DIR/" + OUString::createFromAscii( pName ) +
           OUString::createFromAscii( pExt );
#endif
    rtl::Bootstrap::expandMacros( rURL );

    if (!rURL.isEmpty())
        return DirectoryItem::get( rURL, aDirItem ) == DirectoryItem::E_None;
    else
        return false;
}

/// Displays CREDITS or LICENSE in any of the available version
static void showDocument( const char* pBaseName )
{
    try {
        Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        auto args(::comphelper::InitPropertySequence({
            {"ViewOnly",    Any(true)},
            {"ReadOnly",    Any(true)}
        }));

        OUString aURL;
        if ( checkURL ( pBaseName, ".fodt", aURL ) ||
             checkURL ( pBaseName, ".html", aURL ) ||
             checkURL ( pBaseName, "", aURL ) ) {
            xDesktop->loadComponentFromURL( aURL, u"_blank"_ustr, 0, args );
        }
    } catch (const css::uno::Exception &) {
    }
}

namespace
{
    Reference<XFrame> GetRequestFrame(const SfxRequest& rReq)
    {
        const SfxItemSet* pArgs = rReq.GetInternalArgs_Impl();
        const SfxUnoFrameItem* pItem = nullptr;
        Reference <XFrame> xFrame;
        if (pArgs && (pItem = pArgs->GetItemIfSet(SID_FILLFRAME, false)))
        {
            xFrame = pItem->GetFrame();
        }
        return xFrame;
    }

    Reference<XFrame> GetDocFrame(const SfxRequest& rReq)
    {
        const SfxFrameItem* pFrameItem = rReq.GetArg<SfxFrameItem>(SID_DOCFRAME);
        SfxFrame* pFrame = pFrameItem ? pFrameItem->GetFrame() : nullptr;
        return pFrame ? pFrame->GetFrameInterface() : nullptr;
    }

    class LicenseDialog : public weld::GenericDialogController
    {
    public:
        LicenseDialog(weld::Window* pParent)
            : GenericDialogController(pParent, u"sfx/ui/licensedialog.ui"_ustr,  u"LicenseDialog"_ustr)
        {
        }

        virtual short run() override
        {
            short nRet = GenericDialogController::run();
            if (nRet == RET_OK)
                showDocument("LICENSE");
            return nRet;
        }
    };

    class SafeModeQueryDialog : public weld::MessageDialogController
    {
    public:
        SafeModeQueryDialog(weld::Window* pParent)
            : MessageDialogController(pParent, u"sfx/ui/safemodequerydialog.ui"_ustr, u"SafeModeQueryDialog"_ustr)
        {
        }

        virtual short run() override
        {
            short nRet = MessageDialogController::run();
            if (nRet == RET_OK)
            {
                sfx2::SafeMode::putFlag();
                const uno::Reference< uno::XComponentContext >& xContext = comphelper::getProcessComponentContext();
                css::task::OfficeRestartManager::get(xContext)->requestRestart(
                    css::uno::Reference< css::task::XInteractionHandler >());
            }
            return nRet;
        }
    };
}

weld::Window* SfxRequest::GetFrameWeld() const
{
    const SfxItemSet* pIntArgs = GetInternalArgs_Impl();
    const SfxUnoAnyItem* pItem = nullptr;
    if (pIntArgs && (pItem = pIntArgs->GetItemIfSet(SID_DIALOG_PARENT, false)))
    {
        Reference<awt::XWindow> xWindow;
        pItem->GetValue() >>= xWindow;
        return Application::GetFrameWeld(xWindow);
    }

    Reference<XFrame> xFrame(GetRequestFrame(*this));
    if (!xFrame)
        xFrame = GetDocFrame(*this);
    if (!xFrame)
    {
        SAL_WARN("sfx.appl", "no parent for dialogs");
        return nullptr;
    }
    return Application::GetFrameWeld(xFrame->getContainerWindow());
}

void SfxApplication::MiscExec_Impl( SfxRequest& rReq )
{
    const bool bIsLOK = comphelper::LibreOfficeKit::isActive();
    static svtools::EditableColorConfig aEditableConfig;
    static bool aColorConfigInitialized = false;
    if (!aColorConfigInitialized && bIsLOK)
    {
        // preload color schemes
        aEditableConfig.LoadScheme("Light");
        aEditableConfig.LoadScheme("Dark");
        aColorConfigInitialized = true;
    }

    bool bDone = false;
    switch ( rReq.GetSlot() )
    {
        case SID_SETOPTIONS:
        {
            if( rReq.GetArgs() )
                SetOptions( *rReq.GetArgs() );
            break;
        }

        case SID_QUITAPP:
        case SID_LOGOUT:
        {
            // protect against reentrant calls and avoid closing the same files in parallel
            if (pImpl->bInQuit || pImpl->bClosingDocs)
                return;

            if ( rReq.GetSlot() == SID_LOGOUT )
            {
                for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                    pObjSh; pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
                {
                    if ( !pObjSh->IsModified() )
                        continue;

                    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pObjSh );
                    if ( !pFrame || !pFrame->GetWindow().IsReallyVisible() )
                        continue;

                    if (pObjSh->PrepareClose())
                        pObjSh->SetModified( false );
                    else
                        return;
                }

                SfxStringItem aNameItem( SID_FILE_NAME, u"vnd.sun.star.cmd:logout"_ustr );
                SfxStringItem aReferer( SID_REFERER, u"private/user"_ustr );
                pImpl->pAppDispat->ExecuteList(SID_OPENDOC,
                        SfxCallMode::SLOT, { &aNameItem, &aReferer });
                return;
            }

            // try from nested requests again after 100ms
            if( Application::GetDispatchLevel() > 1 )
            {
                /* Don't save the request for closing the application and try it later
                   again. This is an UI bound functionality ... and the user will  try it again
                   if the dialog is closed. But we should not close the application automatically
                   if this dialog is closed by the user ...
                   So we ignore this request now and wait for a new user decision.
                */
                SAL_INFO("sfx.appl", "QueryExit => sal_False, DispatchLevel == " << Application::GetDispatchLevel() );
                return;
            }

            // block reentrant calls
            pImpl->bInQuit = true;
            Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );

            rReq.ForgetAllArgs();

            // if terminate() failed, pImpl->bInQuit will now be sal_False, allowing further calls of SID_QUITAPP
            bool bTerminated = xDesktop->terminate();
            if (!bTerminated)
                // if terminate() was successful, SfxApplication is now dead!
                pImpl->bInQuit = false;

            // Set return value, terminate if possible
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bTerminated ) );
            return;
        }

        case SID_CONFIG:
        case SID_TOOLBOXOPTIONS:
        case SID_CONFIGSTATUSBAR:
        case SID_CONFIGMENU:
        case SID_CONFIGACCEL:
        case SID_CONFIGEVENT:
        {
            SfxAbstractDialogFactory* pFact =
                SfxAbstractDialogFactory::Create();

            const SfxStringItem* pStringItem = rReq.GetArg<SfxStringItem>(SID_CONFIG);

            SfxItemSetFixed<SID_CONFIG, SID_CONFIG, SID_MACROINFO, SID_MACROINFO> aSet( GetPool() );

            // SID_CONFIG property will determine the default page shown
            if ( pStringItem )
            {
                aSet.Put( SfxStringItem(
                    SID_CONFIG, pStringItem->GetValue() ) );
            }
            else if (rReq.GetSlot() == SID_CONFIGEVENT)
            {
                aSet.Put( SfxStringItem(
                    SID_CONFIG, u"private:resource/event/"_ustr ) );
            }
            else if (rReq.GetSlot() == SID_TOOLBOXOPTIONS)
            {
                aSet.Put( SfxStringItem(
                    SID_CONFIG, u"private:resource/toolbar/"_ustr ) );
            }

#if HAVE_FEATURE_SCRIPTING
            // Preselect a macro in the 'keyboard' page
            if (auto const item = rReq.GetArg<SfxMacroInfoItem>(SID_MACROINFO)) {
                aSet.Put(*item);
            }
#endif

            Reference <XFrame> xFrame(GetRequestFrame(rReq));
            ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateCustomizeTabDialog(rReq.GetFrameWeld(),
                &aSet, xFrame ));

            const short nRet = pDlg->Execute();

            if ( nRet )
                bDone = true;
            break;
        }

        case SID_CLOSEDOCS:
        {
            // protect against reentrant calls and avoid closing the same files in parallel
            if (pImpl->bInQuit || pImpl->bClosingDocs)
                return;

            pImpl->bClosingDocs = true;
            // closed all status for all visible frames
            bool bClosedAll = true;

            // Iterate over all documents and close them
            for (SfxObjectShell *pObjSh = SfxObjectShell::GetFirst(); pObjSh;)
            {
                SfxObjectShell* pNxtObjSh = SfxObjectShell::GetNext(*pObjSh);
                // can close immediately
                if (!pObjSh->IsModified())
                {
                    // don't close the last remaining frame for close dispatch
                    if (pNxtObjSh || !bClosedAll)
                        pObjSh->DoClose();
                }
                else
                {
                    // skip invisible frames when asking user to close
                    SfxViewFrame* pFrame = SfxViewFrame::GetFirst(pObjSh);
                    if (pFrame && pFrame->GetWindow().IsReallyVisible())
                    {
                        // asks user to close
                        if (pObjSh->PrepareClose())
                        {
                            pObjSh->SetModified(false);
                            // get next pointer again after asking user since it can become invalid pointer from being manually closed by user
                            // don't close the last remaining frame for close dispatch
                            if ((pNxtObjSh = SfxObjectShell::GetNext(*pObjSh)) || !bClosedAll)
                                pObjSh->DoClose();
                        }
                        // user disagrees to close
                        else
                        {
                            bClosedAll = false;
                            // get next pointer again after asking user since it can become invalid pointer from being manually closed by user
                            pNxtObjSh = SfxObjectShell::GetNext(*pObjSh);
                        }
                    }
                }
                pObjSh = pNxtObjSh;
            }

            pImpl->bClosingDocs = false;

            // close dispatch status
            bool bDispatchOk = true;
            // open backing window
            if (bClosedAll)
            {
                // don't use pViewFrame = SfxViewFrame::Current() as dispatch won't load sometimes
                SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
                SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst(pObjSh);
                if (pViewFrame)
                {
                    Reference<XFrame> xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
                    if (xCurrentFrame.is())
                    {
                        uno::Reference<frame::XDispatchProvider> xProvider(xCurrentFrame, uno::UNO_QUERY);
                        if (xProvider.is())
                        {
                            uno::Reference<frame::XDispatchHelper> xDispatcher
                                = frame::DispatchHelper::create(::comphelper::getProcessComponentContext());
                            // use .uno:CloseDoc to be able to close windows of the same document
                            css::uno::Any aResult =
                                xDispatcher->executeDispatch(xProvider,
                                                             u".uno:CloseDoc"_ustr,
                                                             u"_self"_ustr,
                                                             0,
                                                             uno::Sequence<beans::PropertyValue>());
                            css::frame::DispatchResultEvent aEvent;
                            bDispatchOk = (aResult >>= aEvent) && (aEvent.State == frame::DispatchResultState::SUCCESS);
                        }
                    }
                }
            }
            // terminate the application if the dispatch fails or
            // if there is no visible frame left after the command is run (e.g user manually closes the document again that was already cancelled for closing)
            if (!bDispatchOk || (!bClosedAll && !SfxObjectShell::GetFirst()))
            {
                SfxRequest aReq(SID_QUITAPP, SfxCallMode::SLOT, GetPool());
                MiscExec_Impl(aReq);
            }

            rReq.SetReturnValue(SfxBoolItem(0, bDispatchOk));
            bDone = true;
            break;
        }

        case SID_SAVEDOCS:
        {
            bool bOK = true;
            for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                  pObjSh;
                  pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
            {
                SfxRequest aReq( SID_SAVEDOC, SfxCallMode::SLOT, pObjSh->GetPool() );
                if ( pObjSh->IsModified() && !pObjSh->isSaveLocked() )
                {
                    pObjSh->ExecuteSlot( aReq );
                    const SfxBoolItem* pItem(dynamic_cast<const SfxBoolItem*>(aReq.GetReturnValue().getItem()));
                    if ( !pItem || !pItem->GetValue() )
                        bOK = false;
                }
            }

            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            rReq.Done();
            break;
        }

        case SID_SEND_FEEDBACK:
        {
            OUString module = SfxHelp::GetCurrentModuleIdentifier();
            OUString sURL(officecfg::Office::Common::Menus::SendFeedbackURL::get() + //officecfg/registry/data/org/openoffice/Office/Common.xcu => https://hub.libreoffice.org/send-feedback/
                "?LOversion=" + utl::ConfigManager::getAboutBoxProductVersion() +
                "&LOlocale=" + utl::ConfigManager::getUILocale() +
                "&LOmodule=" + module.subView(module.lastIndexOf('.') + 1 )  );
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }

        case SID_Q_AND_A:
        {
            // Askbot has URL's normalized to languages, not locales
            // Get language from locale: ll or lll or ll-CC or lll-CC

            OUString sURL(officecfg::Office::Common::Menus::QA_URL::get() + //https://hub.libreoffice.org/forum/
                "?LOlocale=" + utl::ConfigManager::getUILocale());
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
        case SID_DOCUMENTATION:
        {
            // Open documentation page based on locales
            OUString sURL(officecfg::Office::Common::Menus::DocumentationURL::get() + //https://hub.libreoffice.org/documentation/
                "?LOlocale=" + utl::ConfigManager::getUILocale());
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
#if !ENABLE_WASM_STRIP_PINGUSER
        case SID_GETINVOLVED:
        {
            // Open get involved/join us page based on locales
            OUString sURL(officecfg::Office::Common::Menus::GetInvolvedURL::get() + //https://hub.libreoffice.org/joinus/
                "?LOlocale=" + utl::ConfigManager::getUILocale());
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
        case SID_DONATION:
        {
            // Open donation page based on language + script (BCP47) with language as fall back.
            OUString aLang = LanguageTag(utl::ConfigManager::getUILocale()).getLanguage();
            OUString aBcp47 = LanguageTag(utl::ConfigManager::getUILocale()).getBcp47();
            OUString sURL(officecfg::Office::Common::Menus::DonationURL::get() + //https://hub.libreoffice.org/donation/
                "?BCP47=" + aBcp47 + "&LOlang=" + aLang );
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
        case SID_WHATSNEW:
        {
            // Open release notes depending on version and locale
            OUString sURL(officecfg::Office::Common::Menus::ReleaseNotesURL::get() + //https://hub.libreoffice.org/ReleaseNotes/
                "?LOvers=" + utl::ConfigManager::getProductVersion() +
                "&LOlocale=" + LanguageTag(utl::ConfigManager::getUILocale()).getBcp47() );
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
        case SID_CREDITS:
        {
            OUString sURL(officecfg::Office::Common::Menus::CreditsURL::get());
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
        break;
        case SID_HYPHENATIONMISSING:
        {
            // Open wiki page about hyphenation
            OUString sURL(officecfg::Office::Common::Menus::HyphenationMissingURL::get() + //https://hub.libreoffice.org/HyphenationMissing/
                "?LOlocale=" + utl::ConfigManager::getUILocale());
            sfx2::openUriExternally(sURL, false, rReq.GetFrameWeld());
            break;
        }
#endif
        case SID_SHOW_LICENSE:
        {
            LicenseDialog aDialog(rReq.GetFrameWeld());
            aDialog.run();
            break;
        }

        case SID_SHOW_CREDITS:
        {
            showDocument( "CREDITS" );
            break;
        }

        case FN_CHANGE_THEME:
        {
            const SfxStringItem* pNewThemeArg = rReq.GetArg<SfxStringItem>(FN_PARAM_NEW_THEME);
            OUString sSchemeName = ThemeColors::GetThemeColors().GetThemeName();
            AppearanceMode eAppearnceMode = MiscSettings::GetAppColorMode();

            if (!pNewThemeArg)
            {
                // we do not override custom themes if the unocommand was triggered from the UI
                // by clicking on a toolbar/notebookbar button for example.
                if (!ThemeColors::IsCustomTheme(sSchemeName))
                {
                    bool bChangeToLightTheme = eAppearnceMode == AppearanceMode::DARK
                                               || (eAppearnceMode == AppearanceMode::AUTO
                                                   && MiscSettings::GetUseDarkMode());

                    // note that a theme and an appearance mode are not orthogonal anymore, for
                    // "Custom Themes", appearance mode is AUTO, for the "Automatic", "Light" and
                    // "Dark" default themes, it's AUTO, LIGHT & DARK respectively.
                    if (bChangeToLightTheme)
                    {
                        sSchemeName = svtools::LIGHT_COLOR_SCHEME;
                        eAppearnceMode = AppearanceMode::LIGHT;
                    }
                    else
                    {
                        sSchemeName = svtools::DARK_COLOR_SCHEME;
                        eAppearnceMode = AppearanceMode::DARK;
                    }
                }
            }
            else
                sSchemeName = pNewThemeArg->GetValue();

            aEditableConfig.LoadScheme(sSchemeName);
            MiscSettings::SetAppColorMode(eAppearnceMode);

            // kit explicitly ignores changes to the global color scheme, except for the current ViewShell,
            // so an attempted change to the same global color scheme when the now current ViewShell ignored
            // the last change requires re-sending the change. In which case individual shells will have to
            // decide if this color-scheme change is a change from their perspective to avoid unnecessary
            // invalidations.
            if (!pNewThemeArg || bIsLOK || aEditableConfig.GetCurrentSchemeName() != sSchemeName)
            {
                if (bIsLOK)
                    aEditableConfig.SetCurrentSchemeName(sSchemeName);
                else
                    aEditableConfig.LoadScheme(sSchemeName);
            }

            Invalidate(FN_CHANGE_THEME);
            break;
        }
        case FN_INVERT_BACKGROUND:
        {
            const SfxStringItem* pNewThemeArg = rReq.GetArg<SfxStringItem>(FN_PARAM_NEW_THEME);

            svtools::EditableColorConfig aColorConfig;
            ::Color aDefLightColor = svtools::ColorConfig::GetDefaultColor(svtools::DOCCOLOR, 0);
            ::Color aDefDarkColor = svtools::ColorConfig::GetDefaultColor(svtools::DOCCOLOR, 1);

            OUString aNewTheme;
            if (!pNewThemeArg) {
                ::Color aCurrentColor = aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;

                if (aCurrentColor == aDefLightColor) {
                    aNewTheme = OUString("Dark");
                } else {
                    aNewTheme = OUString("Light");
                }
            } else {
                aNewTheme = pNewThemeArg->GetValue();
            }

            svtools::ColorConfigValue aValue;

            if(aNewTheme == "Dark")
                aValue.nColor = aDefDarkColor;
            else
                aValue.nColor = aDefLightColor;

            aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
            break;
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPINDEX:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                pHelp->Start(u".uno:HelpIndex"_ustr, rReq.GetFrameWeld()); // show start page
                bDone = true;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPTIPS:
        {
            // Evaluate Parameter
            const SfxBoolItem* pOnItem = rReq.GetArg<SfxBoolItem>(SID_HELPTIPS);
            bool bOn = pOnItem
                            ? pOnItem->GetValue()
                            : !Help::IsQuickHelpEnabled();

            if ( bOn )
                Help::EnableQuickHelp();
            else
                Help::DisableQuickHelp();
            auto xChanges = comphelper::ConfigurationChanges::create();
            officecfg::Office::Common::Help::Tip::set(bOn, xChanges);
            xChanges->commit();
            Invalidate(SID_HELPTIPS);
            bDone = true;

            // Record if possible
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPTIPS, bOn) );
            break;
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_EXTENDEDHELP:
        {
            Help::StartExtHelp();
            break;
        }
        case SID_HELPBALLOONS:
        {
            // Evaluate Parameter
            const SfxBoolItem* pOnItem = rReq.GetArg<SfxBoolItem>(SID_HELPBALLOONS);
            bool bOn = pOnItem
                            ? pOnItem->GetValue()
                            : !Help::IsBalloonHelpEnabled();

            if ( bOn )
                Help::EnableBalloonHelp();
            else
                Help::DisableBalloonHelp();
            auto xChanges = comphelper::ConfigurationChanges::create();
            officecfg::Office::Common::Help::ExtendedTip::set(bOn, xChanges);
            xChanges->commit();
            Invalidate(SID_HELPBALLOONS);
            bDone = true;

            // Record if possible
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPBALLOONS, bOn) );
            break;
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if !ENABLE_WASM_STRIP_PINGUSER
        case SID_TIPOFTHEDAY:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateTipOfTheDayDialog(rReq.GetFrameWeld()));
            pDlg->StartExecuteAsync(nullptr);
            bDone = true;
            break;
        }
#endif
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_WIDGET_TEST_DIALOG:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            VclPtr<VclAbstractDialog> pDlg(pFact->CreateWidgetTestDialog(rReq.GetFrameWeld()));
            pDlg->StartExecuteAsync([pDlg](sal_Int32 /*nResult*/){
                pDlg->disposeOnce();
            });
            bDone = true;
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ABOUT:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateAboutDialog(rReq.GetFrameWeld()));
            pDlg->StartExecuteAsync(nullptr);
            bDone = true;
            break;
        }

        case SID_TEMPLATE_MANAGER:
        {
            SfxTemplateManagerDlg aDialog(rReq.GetFrameWeld());
            aDialog.run();
            bDone = true;
            break;
        }

        case SID_TEMPLATE_ADDRESSBOOKSOURCE:
        {
            svt::AddressBookSourceDialog aDialog(rReq.GetFrameWeld(), ::comphelper::getProcessComponentContext());
            aDialog.run();
            bDone = true;
            break;
        }

#if HAVE_FEATURE_SCRIPTING
        case SID_BASICSTOP:
            StarBASIC::Stop();
            break;

        case SID_BASICBREAK :
            BasicDLL::BasicBreak();
            break;
#endif

        case SID_ZOOM_50_PERCENT:
        case SID_ZOOM_75_PERCENT:
        case SID_ZOOM_100_PERCENT:
        case SID_ZOOM_150_PERCENT:
        case SID_ZOOM_200_PERCENT:
        case SID_ZOOM_OPTIMAL:
        case SID_ZOOM_ENTIRE_PAGE:
        case SID_ZOOM_PAGE_WIDTH:
        {
            SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
            if (!pCurrentShell)
                return;

            // make sure aZoom is initialized with a proper value if SetType
            // doesn't work
            SvxZoomItem aZoom( SvxZoomType::PERCENT, 100 );

            switch (rReq.GetSlot())
            {
                case SID_ZOOM_50_PERCENT:
                    aZoom.SetValue(50);
                    break;
                case SID_ZOOM_75_PERCENT:
                    aZoom.SetValue(75);
                    break;
                case SID_ZOOM_100_PERCENT:
                    aZoom.SetValue(100);
                    break;
                case SID_ZOOM_150_PERCENT:
                    aZoom.SetValue(150);
                    break;
                case SID_ZOOM_200_PERCENT:
                    aZoom.SetValue(200);
                    break;
                case SID_ZOOM_OPTIMAL:
                    aZoom.SetType( SvxZoomType::OPTIMAL );
                    break;
                case SID_ZOOM_ENTIRE_PAGE:
                    aZoom.SetType( SvxZoomType::WHOLEPAGE );
                    break;
                case SID_ZOOM_PAGE_WIDTH:
                    aZoom.SetType( SvxZoomType::PAGEWIDTH );
                    break;
            }

            pCurrentShell->GetDispatcher()->ExecuteList(SID_ATTR_ZOOM, SfxCallMode::ASYNCHRON, { &aZoom });

            break;
        }
        case SID_TOOLBAR_MODE:
        {
            const SfxStringItem* pModeName = rReq.GetArg<SfxStringItem>( SID_TOOLBAR_MODE );

            if ( !pModeName )
            {
                bDone = true;
                break;
            }

            OUString aNewName(pModeName->GetValue());
            const uno::Reference< uno::XComponentContext >& xContext =
                    ::comphelper::getProcessComponentContext();

            // Get information about current frame and module
            Reference<XFrame> xCurrentFrame;
            vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
            OUString aCurrentMode;

            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if (pViewFrame)
            {
                xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();

                const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
                eCurrentApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xCurrentFrame ) );

                OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" +
                    lcl_getAppName( eCurrentApp );

                const utl::OConfigurationTreeRoot aAppNode(
                                                    xContext,
                                                    aPath,
                                                    true);
                if ( !aAppNode.isValid() )
                {
                    bDone = true;
                    break;
                }

                aCurrentMode = comphelper::getString( aAppNode.getNodeValue( u"Active"_ustr ) );

                if ( !comphelper::LibreOfficeKit::isActive() && aCurrentMode == aNewName )
                {
                    bDone = true;
                    break;
                }

                // Save new toolbar mode for a current module
                aAppNode.setNodeValue( u"Active"_ustr, Any( aNewName ) );
                aAppNode.commit();
            }

            // Apply settings for all frames
            pViewFrame = SfxViewFrame::GetFirst();
            while( pViewFrame )
            {
                // in LOK case we want to apply changes only to the current view
                if (comphelper::LibreOfficeKit::isActive() &&
                    pViewFrame != &SfxViewShell::Current()->GetViewFrame())
                {
                    pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
                    continue;
                }

                Reference<XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();

                // We want to change mode only for a current app module, ignore other apps
                const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
                vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
                if ( eApp != eCurrentApp )
                {
                    pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
                    continue;
                }

                Reference<css::beans::XPropertySet> xPropSet( xFrame, UNO_QUERY );
                Reference<css::frame::XLayoutManager> xLayoutManager;
                if ( xPropSet.is() )
                {
                    try
                    {
                        Any aValue = xPropSet->getPropertyValue( u"LayoutManager"_ustr );
                        aValue >>= xLayoutManager;
                    }
                    catch ( const css::uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch ( css::uno::Exception& )
                    {
                    }
                }

                if ( xLayoutManager.is() )
                {
                    css::uno::Sequence<OUString> aMandatoryToolbars;
                    css::uno::Sequence<OUString> aUserToolbars;
                    std::vector<OUString> aBackupList;
                    OUString aSidebarMode;

                    OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" +
                        lcl_getAppName( eApp ) +
                        "/Modes";

                    // Read mode settings
                    const utl::OConfigurationTreeRoot aModesNode(
                                            xContext,
                                            aPath,
                                            true);
                    if ( !aModesNode.isValid() )
                    {
                        bDone = true;
                        break;
                    }

                    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );

                    for ( const auto& rModeNodeName : aModeNodeNames )
                    {
                        const utl::OConfigurationNode aModeNode( aModesNode.openNode( rModeNodeName ) );
                        if ( !aModeNode.isValid() )
                            continue;

                        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( u"CommandArg"_ustr ) );

                        if ( aCommandArg == aNewName )
                        {
                            aMandatoryToolbars = aModeNode.getNodeValue( u"Toolbars"_ustr ).get< uno::Sequence<OUString> >();
                            aUserToolbars = aModeNode.getNodeValue( u"UserToolbars"_ustr ).get< uno::Sequence<OUString> >();
                            aSidebarMode = comphelper::getString( aModeNode.getNodeValue( u"Sidebar"_ustr ) );
                            break;
                        }
                    }

                    // Backup visible toolbar list and hide all toolbars
                    const Sequence<Reference<XUIElement>> aUIElements = xLayoutManager->getElements();
                    for ( const Reference< XUIElement >& xUIElement : aUIElements )
                    {
                        Reference< XPropertySet > xPropertySet( xUIElement, UNO_QUERY );
                        if ( xPropertySet.is() && xUIElement.is() )
                        {
                            try
                            {
                                OUString aResName;
                                sal_Int16 nType( -1 );
                                xPropertySet->getPropertyValue( u"Type"_ustr ) >>= nType;
                                xPropertySet->getPropertyValue( u"ResourceURL"_ustr ) >>= aResName;

                                if (( nType == css::ui::UIElementType::TOOLBAR ) &&
                                    !aResName.isEmpty() )
                                {
                                    if ( xLayoutManager->isElementVisible( aResName ) )
                                    {
                                        aBackupList.push_back( aResName );
                                    }
                                    xLayoutManager->hideElement( aResName );
                                }
                            }
                            catch ( const Exception& )
                            {
                            }
                        }
                    }

                    // Show/Hide the Notebookbar
                    const SfxStringItem pItem(SID_NOTEBOOKBAR, aNewName);
                    pViewFrame->GetDispatcher()->ExecuteList(SID_NOTEBOOKBAR, SfxCallMode::SYNCHRON, {&pItem});
                    SfxPoolItemHolder aNbItem;
                    pViewFrame->GetDispatcher()->QueryState(SID_NOTEBOOKBAR, aNbItem);

                    // Show toolbars
                    for (const OUString& rName : aMandatoryToolbars)
                    {
                        xLayoutManager->createElement( rName );
                        xLayoutManager->showElement( rName );
                    }

                    for (const OUString& rName : aUserToolbars)
                    {
                        xLayoutManager->createElement( rName );
                        xLayoutManager->showElement( rName );
                    }

                    // Sidebar
                    pViewFrame->ShowChildWindow( SID_SIDEBAR );

                    if (comphelper::LibreOfficeKit::isActive())
                        aSidebarMode = "Opened";

                    sfx2::sidebar::SidebarController* pSidebar =
                            sfx2::sidebar::SidebarController::GetSidebarControllerForFrame( xFrame );
                    if ( pSidebar )
                    {
                        if ( aSidebarMode == "Arrow" )
                        {
                            pSidebar->FadeOut();
                        }
                        else if ( aSidebarMode == "Tabs" )
                        {
                            pSidebar->FadeIn();
                            pSidebar->RequestOpenDeck();
                            pSidebar->RequestCloseDeck();
                        }
                        else if ( aSidebarMode == "Opened" )
                        {
                            pSidebar->FadeIn();
                            pSidebar->RequestOpenDeck();
                        }
                    }

                    // Save settings
                    if ( pViewFrame == SfxViewFrame::Current() )
                    {
                        css::uno::Sequence<OUString> aBackup( comphelper::containerToSequence(aBackupList) );

                        for ( const auto& rModeNodeName : aModeNodeNames )
                        {
                            const utl::OConfigurationNode aModeNode( aModesNode.openNode( rModeNodeName ) );
                            if ( !aModeNode.isValid() )
                                continue;

                            OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( u"CommandArg"_ustr ) );

                            if ( aCommandArg == aCurrentMode )
                            {
                                aModeNode.setNodeValue( u"UserToolbars"_ustr, Any( aBackup ) );
                                break;
                            }
                        }
                        aModesNode.commit();
                    }
                }

                pViewFrame = SfxViewFrame::GetNext(*pViewFrame);
            }

            bDone = true;
            break;
        }
        case SID_UI_PICKER:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(
                pFact->CreateUIPickerDialog(rReq.GetFrameWeld()));
            pDlg->Execute();
            bDone = true;
            break;
        }
        case SID_AVAILABLE_TOOLBARS:
        {
            const SfxStringItem* pToolbarName = rReq.GetArg<SfxStringItem>(SID_AVAILABLE_TOOLBARS);

            if ( pToolbarName )
            {
                Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );
                Reference< XFrame > xFrame = xDesktop->getActiveFrame();

                Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                Reference< css::frame::XLayoutManager > xLayoutManager;
                if ( xPropSet.is() )
                {
                    try
                    {
                        Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
                        aValue >>= xLayoutManager;
                    }
                    catch ( const css::uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch ( css::uno::Exception& )
                    {
                    }
                }

                if ( xLayoutManager.is() )
                {
                    OUString aToolbarName = "private:resource/toolbar/" +
                        pToolbarName->GetValue();

                    // Evaluate Parameter
                    bool bShow( !xLayoutManager->isElementVisible( aToolbarName ));

                    if ( bShow )
                    {
                        xLayoutManager->createElement( aToolbarName );
                        xLayoutManager->showElement( aToolbarName );
                    }
                    else
                        xLayoutManager->hideElement( aToolbarName );
                }
            }

            bDone = true;
            break;
        }
        case SID_MENUBAR:
        {
            sfx2::SfxNotebookBar::ToggleMenubar();
            bDone = true;
            break;
        }
        case SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW:
        {
            SfxViewShell* pViewShell = SfxViewShell::Current();
            SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();
            auto nID = rReq.GetSlot();
            rViewFrame.ToggleChildWindow(nID);

            bDone = true;
            break;
        }
        case SID_INSPECT_SELECTED_OBJECT:
        {
            SfxViewShell* pViewShell = SfxViewShell::Current();
            SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();

            rViewFrame.ShowChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW, true);

            SfxChildWindow* pChild = rViewFrame.GetChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW);
            if (!pChild)
                return;

            auto pDockingWin = dynamic_cast<DevelopmentToolDockingWindow*>(pChild->GetWindow());
            if (pDockingWin)
            {
                pDockingWin->changeToCurrentSelection();
            }

            bDone = true;
            break;
        }
        case SID_SAFE_MODE:
        {
            SafeModeQueryDialog aDialog(rReq.GetFrameWeld());
            aDialog.run();
            break;
        }
        case SID_TOOLBAR_LOCK:
        {
            if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
            {
                Reference<XFrame> xCurrentFrame;
                const uno::Reference<uno::XComponentContext>& xContext
                    = ::comphelper::getProcessComponentContext();
                xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
                const Reference<frame::XModuleManager> xModuleManager
                    = frame::ModuleManager::create(xContext);
                const utl::OConfigurationTreeRoot aAppNode(
                    xContext, u"org.openoffice.Office.UI.GlobalSettings/Toolbars/States"_ustr, true);
                if (aAppNode.isValid())
                {
                    bool isLocked = comphelper::getBOOL(aAppNode.getNodeValue(u"Locked"_ustr));
                    aAppNode.setNodeValue(u"Locked"_ustr, Any(!isLocked));
                    aAppNode.commit();
                    //TODO: apply immediately w/o restart needed
                    SolarMutexGuard aGuard;
                    svtools::executeRestartDialog(comphelper::getProcessComponentContext(), nullptr,
                                                  svtools::RESTART_REASON_UI_CHANGE);
                }
            }
            break;
        }
        default:
            break;
    }

    if ( bDone )
        rReq.Done();
}

void SfxApplication::MiscState_Impl(SfxItemSet &rSet)
{
    const WhichRangesContainer & pRanges = rSet.GetRanges();
    DBG_ASSERT(!pRanges.empty(), "Set without range");
    for ( auto const & pRange : pRanges )
    {
        for(sal_uInt16 nWhich = pRange.first; nWhich <= pRange.second; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_TEMPLATE_ADDRESSBOOKSOURCE:
                    if (!SvtModuleOptions().IsDataBaseInstalled())
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                    break;
                case SID_QUITAPP:
                {
                    if (pImpl->nDocModalMode || pImpl->bClosingDocs)
                        rSet.DisableItem(nWhich);
                    else
                        rSet.Put(SfxStringItem(nWhich, SfxResId(STR_QUITAPP)));
                    break;
                }

                case SID_CONFIG:
                case SID_TOOLBOXOPTIONS:
                case SID_CONFIGSTATUSBAR:
                case SID_CONFIGMENU:
                case SID_CONFIGACCEL:
                case SID_CONFIGEVENT:
                {
                    if( officecfg::Office::Common::Misc::DisableUICustomization::get() )
                        rSet.DisableItem(nWhich);
                    break;
                }

#if HAVE_FEATURE_SCRIPTING
                case SID_BASICSTOP:
                    if ( !StarBASIC::IsRunning() )
                        rSet.DisableItem(nWhich);
                    break;
#endif

                case FN_CHANGE_THEME:
                {
                    const bool bIsDarkMode
                        = MiscSettings::GetAppColorMode() == AppearanceMode::DARK
                          || (MiscSettings::GetAppColorMode() == AppearanceMode::AUTO
                              && MiscSettings::GetUseDarkMode());
                    rSet.Put(SfxBoolItem(FN_CHANGE_THEME, bIsDarkMode));
                    break;
                }
                case SID_HELPTIPS:
                {
                    rSet.Put( SfxBoolItem( SID_HELPTIPS, Help::IsQuickHelpEnabled() ) );
                }
                break;
                case SID_HELPBALLOONS:
                {
                    rSet.Put( SfxBoolItem( SID_HELPBALLOONS, Help::IsBalloonHelpEnabled() ) );
                }
                break;

                case SID_EXTENDEDHELP:
                {
                }
                break;

                case SID_CLOSEDOCS:
                {
                    if ( pImpl->nDocModalMode || pImpl->bInQuit )
                    {
                        rSet.DisableItem(nWhich);
                        return;
                    }
                    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
                    Reference< XIndexAccess > xTasks = xDesktop->getFrames();
                    if ( !xTasks.is() || !xTasks->getCount() )
                        rSet.DisableItem(nWhich);
                    break;
                }

                case SID_SAVEDOCS:
                {
                    bool bModified = false;
                    for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                          pObjSh;
                          pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
                    {
                        if ( pObjSh->IsModified() && !pObjSh->isSaveLocked() )
                        {
                            bModified = true;
                            break;
                        }
                    }

                    if ( !bModified )
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_TEMPLATE_MANAGER:
                    {
                        if ( !officecfg::Office::Common::Misc::ExperimentalMode::get() )
                        {
                           rSet.DisableItem( nWhich );
                           rSet.Put( SfxVisibilityItem( nWhich, false ) );
                        }
                    }
                    break;

                case SID_ZOOM_50_PERCENT:
                case SID_ZOOM_75_PERCENT:
                case SID_ZOOM_100_PERCENT:
                case SID_ZOOM_150_PERCENT:
                case SID_ZOOM_200_PERCENT:
                case SID_ZOOM_OPTIMAL:
                case SID_ZOOM_ENTIRE_PAGE:
                case SID_ZOOM_PAGE_WIDTH:
                    {
                        SfxObjectShell* pCurrentShell(SfxObjectShell::Current());

                        SfxPoolItemHolder aResult;
                        const SfxItemState aState(pCurrentShell ?
                            pCurrentShell->GetDispatcher()->QueryState(SID_ATTR_ZOOM, aResult) : SfxItemState::DISABLED);
                        if ( aState == SfxItemState::DISABLED )
                            rSet.DisableItem( nWhich );
                    }
                    break;

                case SID_MENUBAR:
                {
                    Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );
                    Reference< XFrame > xFrame = xDesktop->getActiveFrame();

                    Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                    Reference< css::frame::XLayoutManager > xLayoutManager;
                    if ( xPropSet.is() )
                    {
                        try
                        {
                            Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
                            aValue >>= xLayoutManager;
                        }
                        catch ( const css::uno::RuntimeException& )
                        {
                            throw;
                        }
                        catch ( css::uno::Exception& )
                        {
                        }
                    }

                    if ( xLayoutManager.is() )
                    {
                        const bool bState
                            = xLayoutManager->getElement(u"private:resource/menubar/menubar"_ustr).is()
                              && xLayoutManager->isElementVisible(
                                     u"private:resource/menubar/menubar"_ustr);

                        SfxBoolItem aItem( SID_MENUBAR, bState );
                        rSet.Put( aItem );
                    }
                    break;
                }
                case SID_SAFE_MODE:
                {
                    // no restart in safe mode when already in safe mode
                    if ( Application::IsSafeModeEnabled() )
                       rSet.DisableItem( SID_SAFE_MODE );
                    break;
                }
                case SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW:
                {
                    bool bSuccess = false;
                    auto* pViewShell = SfxViewShell::Current();
                    if (pViewShell)
                    {
                        auto& rViewFrame = pViewShell->GetViewFrame();
                        if (rViewFrame.KnowsChildWindow(nWhich))
                        {
                            rSet.Put(SfxBoolItem(nWhich, rViewFrame.HasChildWindow(nWhich)));
                            bSuccess = true;
                        }
                    }

                    if (!bSuccess)
                        rSet.DisableItem(nWhich);
                }
                break;
                case SID_INSPECT_SELECTED_OBJECT:
                {
                    bool bSuccess = false;
                    auto* pViewShell = SfxViewShell::Current();
                    if (pViewShell)
                    {
                        auto& rViewFrame = pViewShell->GetViewFrame();
                        if (rViewFrame.KnowsChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW))
                        {
                            bSuccess = true;
                        }
                    }
                    if (!bSuccess)
                        rSet.DisableItem(nWhich);
                }
                break;
                case SID_TOOLBAR_LOCK:
                {
                    rSet.Put( SfxBoolItem( SID_TOOLBAR_LOCK, ToolBox::AlwaysLocked() ));
                }
                break;
                default:
                    break;
            }
        }
    }
}

#if HAVE_FEATURE_SCRIPTING

#ifndef DISABLE_DYNLOADING

typedef rtl_uString* (*basicide_choose_macro)(void*, void*, void*, sal_Bool);

#else

extern "C" rtl_uString* basicide_choose_macro(void*, void*, void*, sal_Bool);

#endif

static OUString ChooseMacro(weld::Window* pParent, const Reference<XModel>& rxLimitToDocument, const Reference<XFrame>& xDocFrame, bool bChooseOnly)
{
#ifndef DISABLE_DYNLOADING
    basicide_choose_macro pSymbol = reinterpret_cast<basicide_choose_macro>(sfx2::getBasctlFunction("basicide_choose_macro"));
#else
#define pSymbol basicide_choose_macro
#endif

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol(pParent, rxLimitToDocument.get(), xDocFrame.get(), bChooseOnly);
    OUString aScriptURL( pScriptURL );
    rtl_uString_release( pScriptURL );
    return aScriptURL;

#ifdef DISABLE_DYNLOADING
#undef pSymbol
#endif
}

#endif

namespace
{
#if HAVE_FEATURE_SCRIPTING
    weld::Window* lcl_getDialogParent(const Reference<XFrame>& rxFrame)
    {
        Reference<awt::XWindow> xContainerWindow;
        if (rxFrame.is())
            xContainerWindow = rxFrame->getContainerWindow();
        return Application::GetFrameWeld(xContainerWindow);
    }

    SfxViewFrame* lcl_getBasicIDEViewFrame( SfxObjectShell const * i_pBasicIDE )
    {
        SfxViewFrame* pView = SfxViewFrame::GetFirst( i_pBasicIDE );
        while ( pView )
        {
            if ( pView->GetObjectShell()->GetFactory().GetDocumentServiceName() == "com.sun.star.script.BasicIDE" )
                break;
            pView = SfxViewFrame::GetNext( *pView, i_pBasicIDE );
        }
        return pView;
    }
    Reference< XFrame > lcl_findStartModuleFrame( const Reference<XComponentContext> & rxContext )
    {
        try
        {
            Reference < XDesktop2 > xDesktop = Desktop::create( rxContext );
            Reference < XIndexAccess > xContainer( xDesktop->getFrames(), UNO_QUERY_THROW );

            Reference< XModuleManager2 > xCheck = ModuleManager::create(rxContext);

            sal_Int32 nCount = xContainer->getCount();
            for ( sal_Int32 i=0; i<nCount; ++i )
            {
                try
                {
                    Reference < XFrame > xFrame( xContainer->getByIndex(i), UNO_QUERY_THROW );
                    OUString sModule = xCheck->identify( xFrame );
                    if ( sModule == "com.sun.star.frame.StartModule" )
                        return xFrame;
                }
                catch( const UnknownModuleException& )
                {
                    // silence
                }
                catch(const Exception&)
                {
                    // re-throw, caught below
                    throw;
                }
            }
        }
        catch( const Exception& )
        {
               DBG_UNHANDLED_EXCEPTION("sfx.appl");
        }
        return nullptr;
    }
#endif // HAVE_FEATURE_SCRIPTING
}

void SfxApplication::OfaExec_Impl( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_OPTIONS_TREEDIALOG:
        {
            OUString sPageURL;
            const SfxStringItem* pURLItem = rReq.GetArg<SfxStringItem>(SID_OPTIONS_PAGEURL);
            if ( pURLItem )
                sPageURL = pURLItem->GetValue();

            sal_uInt16 nPageID = 0;
            const SfxUInt16Item* pIDItem = rReq.GetArg<SfxUInt16Item>(SID_OPTIONS_PAGEID);
            if (pIDItem)
                nPageID = pIDItem->GetValue();

            Reference <XFrame> xFrame(GetRequestFrame(rReq));
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            VclPtr<VclAbstractDialog> pDlg =
                pFact->CreateFrameDialog(rReq.GetFrameWeld(), xFrame, rReq.GetSlot(), nPageID, sPageURL);
            short nRet = pDlg->Execute();
            pDlg.disposeAndClear();
            SfxViewFrame* pView = SfxViewFrame::GetFirst();
            bool bDisableActiveContent
                    = officecfg::Office::Common::Security::Scripting::DisableActiveContent::get();

            while ( pView )
            {
                if (nRet == RET_OK)
                {
                    SfxObjectShell* pObjSh = pView->GetObjectShell();
                    if (pObjSh)
                    {
                        pObjSh->SetConfigOptionsChecked(false);

                        // when active content is disabled via options dialog,
                        // disable all current active embedded objects
                        if (bDisableActiveContent)
                            lcl_disableActiveEmbeddedObjects(pObjSh);
                    }
                }
                pView->GetBindings().InvalidateAll(false);
                pView = SfxViewFrame::GetNext( *pView );
            }
            break;
        }

        case SID_OPTIONS_SECURITY:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            VclPtr<AbstractSecurityOptionsDialog> pDlg =
                pFact->CreateSvxSecurityOptionsDialog(rReq.GetFrameWeld());

            if (pDlg->Execute() == RET_OK) {
                pDlg->SetSecurityOptions();
            }

            pDlg.disposeAndClear();
            break;
        }

        case SID_ADDITIONS_DIALOG:
        {
            OUString sAdditionsTag = u""_ustr;

            const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(FN_PARAM_ADDITIONS_TAG);
            if (pStringArg)
                sAdditionsTag = pStringArg->GetValue();

            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            VclPtr<AbstractAdditionsDialog> pDialog(
                pFact->CreateAdditionsDialog(rReq.GetFrameWeld(), sAdditionsTag));
            pDialog->StartExecuteAsync(
                [pDialog] (sal_Int32 /*nResult*/)->void
                {
                    pDialog->disposeOnce();
                }
                );
            break;
        }

        case SID_MORE_DICTIONARIES:
        {
            uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
                u"AdditionsTag"_ustr, u"Dictionary"_ustr) };
            comphelper::dispatchCommand(u".uno:AdditionsDialog"_ustr, aArgs);
            break;
        }
#if HAVE_FEATURE_SCRIPTING
        case SID_BASICIDE_APPEAR:
        {
            SfxViewFrame* pView = lcl_getBasicIDEViewFrame( nullptr );
            if ( !pView )
            {
                SfxObjectShell* pBasicIDE = SfxObjectShell::CreateObject( u"com.sun.star.script.BasicIDE"_ustr );
                pBasicIDE->DoInitNew();
                pBasicIDE->SetModified( false );
                try
                {
                    // load the Basic IDE via direct access to the SFX frame loader. A generic loadComponentFromURL
                    // (which could be done via SfxViewFrame::LoadDocumentIntoFrame) is not feasible here, since the Basic IDE
                    // does not really play nice with the framework's concept. For instance, it is a "singleton document",
                    // which conflicts, at the latest, with the framework's concept of loading into _blank frames.
                    // So, since we know that our frame loader can handle it, we skip the generic framework loader
                    // mechanism, and the type detection (which doesn't know about the Basic IDE).
                    const Reference< XComponentContext >& xContext( ::comphelper::getProcessComponentContext() );
                    Reference< XSynchronousFrameLoader > xLoader(
                        xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.comp.office.FrameLoader"_ustr, xContext),
                        UNO_QUERY_THROW );
                    ::comphelper::NamedValueCollection aLoadArgs;
                    aLoadArgs.put( u"Model"_ustr, pBasicIDE->GetModel() );
                    aLoadArgs.put( u"URL"_ustr, u"private:factory/sbasic"_ustr );

                    Reference< XFrame > xTargetFrame( lcl_findStartModuleFrame( xContext ) );
                    if ( !xTargetFrame.is() )
                        xTargetFrame = SfxFrame::CreateBlankFrame();
                    ENSURE_OR_THROW( xTargetFrame.is(), "could not obtain a frameto load the Basic IDE into!" );

                    xLoader->load( aLoadArgs.getPropertyValues(), xTargetFrame );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("sfx.appl");
                }

                pView = lcl_getBasicIDEViewFrame( pBasicIDE );
                if ( pView )
                    pView->SetName( u"BASIC:1"_ustr );
            }

            if ( pView )
                pView->GetFrame().Appear();

            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs && pView )
            {
                SfxViewShell* pViewShell = pView->GetViewShell();
                SfxObjectShell* pObjShell = pView->GetObjectShell();
                if ( pViewShell && pObjShell )
                {
                    SfxRequest aReq( SID_BASICIDE_SHOWWINDOW, SfxCallMode::SYNCHRON, pObjShell->GetPool() );
                    aReq.SetArgs( *pArgs );
                    pViewShell->ExecuteSlot( aReq );
                }
            }

            rReq.Done();
        }
        break;

        case SID_BASICCHOOSER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxBoolItem* pItem;
            bool bChooseOnly = false;
            Reference< XModel > xLimitToModel;
            if(pArgs && (pItem = pArgs->GetItemIfSet(SID_RECORDMACRO, false)) )
            {
                bool bRecord = pItem->GetValue();
                if ( bRecord )
                {
                    // !Hack
                    bChooseOnly = false;
                    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
                    OSL_ENSURE( pCurrentShell, "macro recording outside an SFX document?" );
                    if ( pCurrentShell )
                        xLimitToModel = pCurrentShell->GetModel();
                }
            }

            Reference <XFrame> xFrame(GetRequestFrame(rReq));
            rReq.SetReturnValue(SfxStringItem(rReq.GetSlot(), ChooseMacro(rReq.GetFrameWeld(), xLimitToModel, xFrame, bChooseOnly)));
            rReq.Done();
        }
        break;

        case SID_MACROORGANIZER:
        {
            SAL_INFO("sfx.appl", "handling SID_MACROORGANIZER");
            const SfxItemSet* pArgs = rReq.GetArgs();
            sal_Int16 nTabId = 0;
            Reference <XFrame> xFrame;
            if (pArgs)
            {
                if (const SfxUInt16Item* pItem = pArgs->GetItemIfSet(SID_MACROORGANIZER, false))
                    nTabId = pItem->GetValue();
                if (const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_2))
                {
                    // if set then default to showing the macros of the document associated
                    // with this frame
                    if (pItem->GetValue())
                        xFrame = GetRequestFrame(rReq);
                }
            }
            SfxApplication::MacroOrganizer(rReq.GetFrameWeld(), xFrame, nTabId);
            rReq.Done();
        }
        break;

        case SID_RUNMACRO:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            SAL_INFO("sfx.appl", "SfxApplication::OfaExec_Impl: case ScriptOrg");

            Reference <XFrame> xFrame(GetRequestFrame(rReq));
            if ( !xFrame.is() )
            {
                if (const SfxViewFrame* pViewFrame = SfxViewFrame::Current())
                    xFrame = pViewFrame->GetFrame().GetFrameInterface();
            }

            do  // artificial loop for flow control
            {
                VclPtr<AbstractScriptSelectorDialog> pDlg(pFact->CreateScriptSelectorDialog(lcl_getDialogParent(xFrame), xFrame));
                OSL_ENSURE( pDlg, "SfxApplication::OfaExec_Impl( SID_RUNMACRO ): no dialog!" );
                if ( !pDlg )
                    break;
                pDlg->SetRunLabel();

                pDlg->StartExecuteAsync([pDlg, xFrame](sal_Int32 nDialogResult) {
                    if ( !nDialogResult )
                    {
                        pDlg->disposeOnce();
                        return;
                    }

                    Sequence< Any > args;
                    Sequence< sal_Int16 > outIndex;
                    Sequence< Any > outArgs;
                    Any ret;

                    Reference< XInterface > xScriptContext;

                    Reference< XController > xController;
                    if ( xFrame.is() )
                        xController = xFrame->getController();
                    if ( xController.is() )
                        xScriptContext = xController->getModel();
                    if ( !xScriptContext.is() )
                        xScriptContext = xController;

                    SfxObjectShell::CallXScript( xScriptContext, pDlg->GetScriptURL(), args, ret, outIndex, outArgs );
                    pDlg->disposeOnce();
                });
            }
            while ( false );
            rReq.Done();
        }
        break;

        case SID_SCRIPTORGANIZER:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            SAL_INFO("sfx.appl", "SfxApplication::OfaExec_Impl: case ScriptOrg");
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxScriptOrganizerItem* pItem;
            OUString aLanguage;
            if(pArgs && (pItem = pArgs->GetItemIfSet(SID_SCRIPTORGANIZER, false) ))
            {
                aLanguage = pItem->getLanguage();
            }

            OUString aLang( aLanguage );
            SAL_INFO("sfx.appl", "SfxApplication::OfaExec_Impl: about to create dialog for: " << aLang);
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSvxScriptOrgDialog(rReq.GetFrameWeld(), aLanguage));
            if( pDlg )
            {
                pDlg->Execute();
            }
            else
            {
                SAL_WARN("sfx.appl", "no dialog!!!");
            }
            rReq.Done();
        }
        break;

        case SID_MACROMANAGER:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

            Reference<XFrame> xFrame(GetRequestFrame(rReq));
            if (!xFrame.is())
            {
                if (const SfxViewFrame* pViewFrame = SfxViewFrame::Current())
                    xFrame = pViewFrame->GetFrame().GetFrameInterface();
            }

            VclPtr<AbstractMacroManagerDialog> pDlg(
                pFact->CreateMacroManagerDialog(lcl_getDialogParent(xFrame), xFrame));
            OSL_ENSURE(pDlg, "SfxApplication::OfaExec_Impl(SID_MACROMANAGER): no dialog!");
            if (pDlg)
            {
                pDlg->StartExecuteAsync(
                    [pDlg, xFrame](sal_Int32 nDialogResult)
                    {
                        if (!nDialogResult)
                        {
                            pDlg->disposeOnce();
                            return;
                        }

                        Sequence<Any> args;
                        Sequence<sal_Int16> outIndex;
                        Sequence<Any> outArgs;
                        Any ret;

                        Reference<XInterface> xScriptContext;

                        Reference<XController> xController;
                        if (xFrame.is())
                            xController = xFrame->getController();
                        if (xController.is())
                            xScriptContext = xController->getModel();
                        if (!xScriptContext.is())
                            xScriptContext = xController;

                        SfxObjectShell::CallXScript(xScriptContext, pDlg->GetScriptURL(), args, ret,
                                                    outIndex, outArgs);
                        pDlg->disposeOnce();
                    });
                pDlg->LoadLastUsedMacro();
            }
            rReq.Done();
        }
        break;
#endif // HAVE_FEATURE_SCRIPTING

        case SID_OFFICE_CHECK_PLZ:
        {
            bool bRet = false;
            const SfxStringItem* pStringItem = rReq.GetArg<SfxStringItem>(rReq.GetSlot());

            if ( pStringItem )
            {
                bRet = true /*!!!SfxIniManager::CheckPLZ( aPLZ )*/;
            }
#if HAVE_FEATURE_SCRIPTING
            else
                SbxBase::SetError( ERRCODE_BASIC_WRONG_ARGS );
#endif
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bRet ) );
        }
        break;

        case SID_AUTO_CORRECT_DLG:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            SfxItemSetFixed<SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG> aSet(GetPool());
            const SfxPoolItem* pItem=nullptr;
            const SfxItemSet* pSet = rReq.GetArgs();
            if ( pSet && pSet->GetItemState( SID_AUTO_CORRECT_DLG, false, &pItem ) == SfxItemState::SET )
                aSet.Put( *pItem );

            ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateAutoCorrTabDialog(rReq.GetFrameWeld(), &aSet));
            pDlg->Execute();

            break;
        }

        case SID_NEWSD :
        {
            if (!SvtModuleOptions().IsImpressInstalled())
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(rReq.GetFrameWeld(),
                                                                         VclMessageType::Warning, VclButtonsType::Ok,
                                                                         SfxResId(STR_MODULENOTINSTALLED)));
                xBox->run();
                return;
            }

            const Reference< uno::XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
            Reference< frame::XDispatchProvider > xProv = drawing::ModuleDispatcher::create( xContext );

            OUString aCmd = GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName();
            Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
            Sequence < beans::PropertyValue > aSeq;
            if ( rReq.GetArgs() )
                TransformItems( rReq.GetSlot(), *rReq.GetArgs(), aSeq );
            Any aResult = xHelper->executeDispatch( xProv, aCmd, OUString(), 0, aSeq );
            frame::DispatchResultEvent aEvent;
            bool bSuccess = (aResult >>= aEvent) &&
                                (aEvent.State == frame::DispatchResultState::SUCCESS);
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bSuccess ) );
        }
        break;

        case FN_LABEL :
        case FN_BUSINESS_CARD :
        case FN_XFORMS_INIT :
        {
            const Reference< uno::XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
            Reference< frame::XDispatchProvider > xProv = text::ModuleDispatcher::create( xContext );

            OUString aCmd = GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName();
            Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
            Sequence < beans::PropertyValue > aSeq;
            if ( rReq.GetArgs() )
                TransformItems( rReq.GetSlot(), *rReq.GetArgs(), aSeq );
            Any aResult = xHelper->executeDispatch( xProv, aCmd, OUString(), 0, aSeq );
            frame::DispatchResultEvent aEvent;
            bool bSuccess = (aResult >>= aEvent) &&
                                (aEvent.State == frame::DispatchResultState::SUCCESS);
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bSuccess ) );
        }
        break;

        case SID_ADDRESS_DATA_SOURCE:
        {
            try
            {
                const Reference< uno::XComponentContext >& xORB = ::comphelper::getProcessComponentContext();
                Reference< ui::dialogs::XExecutableDialog > xDialog = ui::dialogs::AddressBookSourcePilot::createWithParent(xORB, nullptr);
                xDialog->execute();
            }
            catch(const css::uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("sfx.appl");
            }
        }
        break;

        case SID_COMP_BIBLIOGRAPHY:
            lcl_tryLoadBibliography();
        break;
    }
}

void SfxApplication::OfaState_Impl(SfxItemSet &rSet)
{
    if (!SvtModuleOptions().IsWriterInstalled())
    {
        rSet.DisableItem( FN_LABEL );
        rSet.DisableItem( FN_BUSINESS_CARD );
        rSet.DisableItem( FN_XFORMS_INIT );
    }
    if ( comphelper::LibreOfficeKit::isActive() )
        rSet.DisableItem( SID_AUTO_CORRECT_DLG );

    if (SvtSecurityOptions::IsMacroDisabled())
    {
        rSet.DisableItem(SID_RUNMACRO);
        rSet.DisableItem(SID_MACROORGANIZER);
        rSet.DisableItem(SID_SCRIPTORGANIZER);
        rSet.DisableItem(SID_BASICIDE_APPEAR);
        rSet.DisableItem(SID_MACROMANAGER);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
