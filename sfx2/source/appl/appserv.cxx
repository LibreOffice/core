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
#include <svtools/miscopt.hxx>
#include <svtools/restartdialog.hxx>
#include <svl/visitem.hxx>

#include <unotools/configmgr.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/weld.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basic/sberrors.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/toolbox.hxx>

#include <unotools/moduleoptions.hxx>
#include <rtl/bootstrap.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
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
#include <sfx2/devtools/DevelopmentToolDockingWindow.hxx>

#include <comphelper/types.hxx>
#include <officecfg/Office/Common.hxx>
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
                return "Writer";
            case vcl::EnumContext::Application::Calc:
                return "Calc";
            case vcl::EnumContext::Application::Impress:
                return "Impress";
            case vcl::EnumContext::Application::Draw:
                return "Draw";
            case vcl::EnumContext::Application::Formula:
                return "Formula";
            case vcl::EnumContext::Application::Base:
                return "Base";
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
                    Sequence< OUString > vPackages { "libreoffice-base" };
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
            SfxStringItem aURL(SID_FILE_NAME, ".component:Bibliography/View1");
            SfxStringItem aRef(SID_REFERER, "private:user");
            SfxStringItem aTarget(SID_TARGETNAME, "_blank");
            SfxViewFrame::Current()->GetDispatcher()->ExecuteList(SID_OPENDOC,
                SfxCallMode::ASYNCHRON, { &aURL, &aRef, &aTarget });
        }
        catch (const Exception &)
        {
            TOOLS_INFO_EXCEPTION( "sfx.appl", "trying to load bibliography database");
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
            {"ViewOnly",    makeAny(true)},
            {"ReadOnly",    makeAny(true)}
        }));

        OUString aURL;
        if ( checkURL ( pBaseName, ".fodt", aURL ) ||
             checkURL ( pBaseName, ".html", aURL ) ||
             checkURL ( pBaseName, "", aURL ) ) {
            xDesktop->loadComponentFromURL( aURL, "_blank", 0, args );
        }
    } catch (const css::uno::Exception &) {
    }
}

namespace
{
    Reference<XFrame> GetRequestFrame(const SfxRequest& rReq)
    {
        const SfxItemSet* pArgs = rReq.GetInternalArgs_Impl();
        const SfxPoolItem* pItem = nullptr;
        Reference <XFrame> xFrame;
        if (pArgs && pArgs->GetItemState(SID_FILLFRAME, false, &pItem) == SfxItemState::SET)
        {
            assert( dynamic_cast< const SfxUnoFrameItem *>( pItem ) && "SfxApplication::OfaExec_Impl: XFrames are to be transported via SfxUnoFrameItem by now!" );
            xFrame = static_cast< const SfxUnoFrameItem*>( pItem )->GetFrame();
        }
        return xFrame;
    }

    class LicenseDialog : public weld::GenericDialogController
    {
    public:
        LicenseDialog(weld::Window* pParent)
            : GenericDialogController(pParent, "sfx/ui/licensedialog.ui",  "LicenseDialog")
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
            : MessageDialogController(pParent, "sfx/ui/safemodequerydialog.ui", "SafeModeQueryDialog")
        {
        }

        virtual short run() override
        {
            short nRet = MessageDialogController::run();
            if (nRet == RET_OK)
            {
                sfx2::SafeMode::putFlag();
                uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
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
    const SfxPoolItem* pItem = nullptr;
    if (pIntArgs && pIntArgs->GetItemState(SID_DIALOG_PARENT, false, &pItem) == SfxItemState::SET)
    {
        assert(dynamic_cast<const SfxUnoAnyItem*>(pItem));
        auto aAny = static_cast<const SfxUnoAnyItem*>(pItem)->GetValue();
        Reference<awt::XWindow> xWindow;
        aAny >>= xWindow;
        return Application::GetFrameWeld(xWindow);
    }

    Reference<XFrame> xFrame(GetRequestFrame(*this));
    if (!xFrame)
    {
        SAL_WARN("sfx.appl", "no parent for dialogs");
        return nullptr;
    }
    return Application::GetFrameWeld(xFrame->getContainerWindow());
}

void SfxApplication::MiscExec_Impl( SfxRequest& rReq )
{
    bool bDone = false;
    switch ( rReq.GetSlot() )
    {
        case SID_SETOPTIONS:
        {
            if( rReq.GetArgs() )
                SetOptions_Impl( *rReq.GetArgs() );
            break;
        }

        case SID_QUITAPP:
        case SID_LOGOUT:
        {
            // protect against reentrant calls
            if ( pImpl->bInQuit )
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

                SfxStringItem aNameItem( SID_FILE_NAME, "vnd.sun.star.cmd:logout" );
                SfxStringItem aReferer( SID_REFERER, "private/user" );
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

            SfxItemSetFixed<SID_CONFIG, SID_CONFIG> aSet( GetPool() );

            if ( pStringItem )
            {
                aSet.Put( SfxStringItem(
                    SID_CONFIG, pStringItem->GetValue() ) );
            }

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

            Reference < XDesktop2 > xDesktop  = Desktop::create( ::comphelper::getProcessComponentContext() );
            Reference< XIndexAccess > xTasks = xDesktop->getFrames();
            if ( !xTasks.is() )
                break;

            sal_Int32 n=0;
            do
            {
                if ( xTasks->getCount() <= n )
                    break;

                Any aAny = xTasks->getByIndex(n);
                Reference < XCloseable > xTask;
                aAny >>= xTask;
                try
                {
                    xTask->close(true);
                    n++;
                }
                catch( CloseVetoException& )
                {
                }
            }
            while( true );

            bool bOk = ( n == 0);
            rReq.SetReturnValue( SfxBoolItem( 0, bOk ) );
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
                if ( pObjSh->IsModified() )
                {
                    pObjSh->ExecuteSlot( aReq );
                    const SfxBoolItem *pItem = dynamic_cast<const SfxBoolItem*>( aReq.GetReturnValue()  );
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
#ifndef ENABLE_WASM_STRIP_PINGUSER
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

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPINDEX:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                pHelp->Start(".uno:HelpIndex", rReq.GetFrameWeld()); // show start page
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
#ifndef ENABLE_WASM_STRIP_PINGUSER
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
            uno::Reference< uno::XComponentContext > xContext =
                    ::comphelper::getProcessComponentContext();

            // Get information about current frame and module
            Reference<XFrame> xCurrentFrame;
            vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
            OUString aCurrentMode;

            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if( pViewFrame )
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

                aCurrentMode = comphelper::getString( aAppNode.getNodeValue( "Active" ) );

                if ( aCurrentMode == aNewName )
                {
                    bDone = true;
                    break;
                }

                // Save new toolbar mode for a current module
                aAppNode.setNodeValue( "Active", makeAny( aNewName ) );
                aAppNode.commit();
            }

            // Apply settings for all frames
            pViewFrame = SfxViewFrame::GetFirst();
            while( pViewFrame )
            {
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
                        Any aValue = xPropSet->getPropertyValue( "LayoutManager" );
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

                        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );

                        if ( aCommandArg == aNewName )
                        {
                            aMandatoryToolbars = aModeNode.getNodeValue( "Toolbars" ).get< uno::Sequence<OUString> >();
                            aUserToolbars = aModeNode.getNodeValue( "UserToolbars" ).get< uno::Sequence<OUString> >();
                            aSidebarMode = comphelper::getString( aModeNode.getNodeValue( "Sidebar" ) );
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
                                xPropertySet->getPropertyValue( "Type" ) >>= nType;
                                xPropertySet->getPropertyValue( "ResourceURL" ) >>= aResName;

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

                    // Show toolbars
                    for ( const OUString& rName : std::as_const(aMandatoryToolbars) )
                    {
                        xLayoutManager->createElement( rName );
                        xLayoutManager->showElement( rName );
                    }

                    for ( const OUString& rName : std::as_const(aUserToolbars) )
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

                            OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );

                            if ( aCommandArg == aCurrentMode )
                            {
                                aModeNode.setNodeValue( "UserToolbars", makeAny( aBackup ) );
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
        case SID_TOOLBAR_MODE_UI:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(
                pFact->CreateToolbarmodeDialog(rReq.GetFrameWeld()));
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
                        Any aValue = xPropSet->getPropertyValue("LayoutManager");
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
            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            auto nID = rReq.GetSlot();
            pViewFrame->ToggleChildWindow(nID);

            bDone = true;
            break;
        }
        case SID_INSPECT_SELECTED_OBJECT:
        {
            SfxViewShell* pViewShell = SfxViewShell::Current();
            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();

            pViewFrame->ShowChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW, true);

            SfxChildWindow* pChild = pViewFrame->GetChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW);
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
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if (pViewFrame)
            {
                Reference<XFrame> xCurrentFrame;
                uno::Reference<uno::XComponentContext> xContext
                    = ::comphelper::getProcessComponentContext();
                xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();
                const Reference<frame::XModuleManager> xModuleManager
                    = frame::ModuleManager::create(xContext);
                const utl::OConfigurationTreeRoot aAppNode(
                    xContext, "org.openoffice.Office.UI.GlobalSettings/Toolbars/States", true);
                if (aAppNode.isValid())
                {
                    bool isLocked = comphelper::getBOOL(aAppNode.getNodeValue("Locked"));
                    aAppNode.setNodeValue("Locked", makeAny(!isLocked));
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
                    if ( !SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::DATABASE) )
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                    break;
                case SID_QUITAPP:
                {
                    if ( pImpl->nDocModalMode )
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
                        if ( pObjSh->IsModified() )
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
                        SfxObjectShell* pCurrentShell = SfxObjectShell::Current();

                        const SfxPoolItem *pItem;
                        SfxItemState aState = pCurrentShell ?
                            pCurrentShell->GetDispatcher()->QueryState(SID_ATTR_ZOOM, pItem) : SfxItemState::DISABLED;
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
                            Any aValue = xPropSet->getPropertyValue("LayoutManager");
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
                            = xLayoutManager->getElement("private:resource/menubar/menubar").is()
                              && xLayoutManager->isElementVisible(
                                     "private:resource/menubar/menubar");

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
                        auto* pViewFrame = pViewShell->GetViewFrame();
                        if (pViewFrame && pViewFrame->KnowsChildWindow(nWhich))
                        {
                            rSet.Put(SfxBoolItem(nWhich, pViewFrame->HasChildWindow(nWhich)));
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
                        auto* pViewFrame = pViewShell->GetViewFrame();
                        if (pViewFrame && pViewFrame->KnowsChildWindow(SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW))
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
            Reference <XFrame> xFrame(GetRequestFrame(rReq));
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            VclPtr<VclAbstractDialog> pDlg =
                pFact->CreateFrameDialog(rReq.GetFrameWeld(), xFrame, rReq.GetSlot(), sPageURL );
            short nRet = pDlg->Execute();
            pDlg.disposeAndClear();
            SfxViewFrame* pView = SfxViewFrame::GetFirst();
            while ( pView )
            {
                if (nRet == RET_OK)
                {
                    SfxObjectShell* pObjSh = pView->GetObjectShell();
                    if (pObjSh)
                        pObjSh->SetConfigOptionsChecked(false);
                }
                pView->GetBindings().InvalidateAll(false);
                pView = SfxViewFrame::GetNext( *pView );
            }
            break;
        }

        case SID_MORE_DICTIONARIES:
        {
            uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
                "AdditionsTag", OUString("Dictionary")) };
            comphelper::dispatchCommand(".uno:AdditionsDialog", aArgs);
            break;
        }
#if HAVE_FEATURE_SCRIPTING
        case SID_BASICIDE_APPEAR:
        {
            SfxViewFrame* pView = lcl_getBasicIDEViewFrame( nullptr );
            if ( !pView )
            {
                SfxObjectShell* pBasicIDE = SfxObjectShell::CreateObject( "com.sun.star.script.BasicIDE" );
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
                    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    Reference< XSynchronousFrameLoader > xLoader(
                        xContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.office.FrameLoader", xContext),
                        UNO_QUERY_THROW );
                    ::comphelper::NamedValueCollection aLoadArgs;
                    aLoadArgs.put( "Model", pBasicIDE->GetModel() );
                    aLoadArgs.put( "URL", OUString( "private:factory/sbasic"  ) );

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
                    pView->SetName( "BASIC:1" );
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
            const SfxPoolItem* pItem;
            bool bChooseOnly = false;
            Reference< XModel > xLimitToModel;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_RECORDMACRO, false, &pItem) )
            {
                bool bRecord = static_cast<const SfxBoolItem*>(pItem)->GetValue();
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
            const SfxPoolItem* pItem;
            sal_Int16 nTabId = 0;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_MACROORGANIZER, false, &pItem) )
            {
                nTabId = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            }

            SfxApplication::MacroOrganizer(rReq.GetFrameWeld(), nTabId);
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
                const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
                if ( pViewFrame )
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
            const SfxPoolItem* pItem;
            OUString aLanguage;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_SCRIPTORGANIZER, false, &pItem) )
            {
                aLanguage = static_cast<const SfxScriptOrganizerItem*>(pItem)->getLanguage();
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
            SfxItemPool* pSetPool = pSet ? pSet->GetPool() : nullptr;
            if ( pSet && pSet->GetItemState( pSetPool->GetWhich( SID_AUTO_CORRECT_DLG ), false, &pItem ) == SfxItemState::SET )
                aSet.Put( *pItem );

            ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateAutoCorrTabDialog(rReq.GetFrameWeld(), &aSet));
            pDlg->Execute();

            break;
        }

        case SID_NEWSD :
        {
            SvtModuleOptions aModuleOpt;
            if ( !aModuleOpt.IsImpress() )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(rReq.GetFrameWeld(),
                                                                         VclMessageType::Warning, VclButtonsType::Ok,
                                                                         SfxResId(STR_MODULENOTINSTALLED)));
                xBox->run();
                return;
            }

            Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            Reference< frame::XDispatchProvider > xProv = drawing::ModuleDispatcher::create( xContext );

            OUString aCmd = OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
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
            Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            Reference< frame::XDispatchProvider > xProv = text::ModuleDispatcher::create( xContext );

            OUString aCmd = OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
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
                Reference< uno::XComponentContext > xORB = ::comphelper::getProcessComponentContext();
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
    SvtModuleOptions aModuleOpt;

    if( !aModuleOpt.IsWriter())
    {
        rSet.DisableItem( FN_LABEL );
        rSet.DisableItem( FN_BUSINESS_CARD );
        rSet.DisableItem( FN_XFORMS_INIT );
    }
    if ( comphelper::LibreOfficeKit::isActive() )
        rSet.DisableItem( SID_AUTO_CORRECT_DLG );

    bool bMacrosDisabled
        = officecfg::Office::Common::Security::Scripting::DisableMacrosExecution::get();
    if (bMacrosDisabled)
    {
        rSet.DisableItem(SID_RUNMACRO);
        rSet.DisableItem(SID_MACROORGANIZER);
        rSet.DisableItem(SID_SCRIPTORGANIZER);
        rSet.DisableItem(SID_BASICIDE_APPEAR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
