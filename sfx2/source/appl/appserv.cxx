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

#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/drawing/ModuleDispatcher.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/text/ModuleDispatcher.hpp>
#include <com/sun/star/ui/dialogs/AddressBookSourcePilot.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>

#include <com/sun/star/frame/XComponentLoader.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#include <svtools/addresstemplate.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/restartdialog.hxx>
#include <svl/visitem.hxx>
#include <unotools/intlwrapper.hxx>

#include <unotools/configmgr.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/layout.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <vcl/help.hxx>
#include <vcl/stdtext.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/helpopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/hintpost.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/new.hxx>
#include "sfxtypes.hxx"
#include <sfx2/tabdlg.hxx>
#include "arrdecl.hxx"
#include <sfx2/sfx.hrc>
#include "app.hrc"
#include <sfx2/passwd.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/childwin.hxx>
#include "appdata.hxx"
#include <sfx2/minfitem.hxx>
#include <sfx2/event.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfxpicklist.hxx"
#include "imestatuswindow.hxx"
#include <sfx2/sfxdlg.hxx>
#include <sfx2/dialogs.hrc>
#include "sorgitm.hxx"
#include <sfx2/sfxhelp.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/templatedlg.hxx>

#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;

namespace
{
    // lp#527938, debian#602953, fdo#33266, i#105408
    static bool lcl_isBaseAvailable()
    {
        try
        {
            // if we get com::sun::star::sdbc::DriverManager, libsdbc2 is there
            // and the bibliography is assumed to work
            return com::sun::star::sdbc::DriverManager::create(comphelper::getProcessComponentContext()).is();
        }
        catch (Exception & e)
        {
            SAL_INFO(
                "sfx.appl",
                "assuming Base to be missing; caught " << e.Message);
            return false;
        }
    }
    static void lcl_tryLoadBibliography()
    {
        // lp#527938, debian#602953, fdo#33266, i#105408
        // make sure we actually can instanciate services from base first
        if(!lcl_isBaseAvailable())
        {
            try
            {
                using namespace org::freedesktop::PackageKit;
                using namespace svtools;
                Reference< XSyncDbusSessionHelper > xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
                Sequence< OUString > vPackages(1);
                vPackages[0] = "libreoffice-base";
                OUString sInteraction;
                xSyncDbusSessionHelper->InstallPackageNames(0, vPackages, sInteraction);
                // Ill be back (hopefully)!
                SolarMutexGuard aGuard;
                executeRestartDialog(comphelper::getProcessComponentContext(), NULL, RESTART_REASON_BIBLIOGRAPHY_INSTALL);
            }
            catch (const Exception & e)
            {
                SAL_INFO(
                    "sfx.appl",
                    "trying to install LibreOffice Base, caught " << e.Message);
            }
            return;
        }

        try // fdo#48775
        {
            SfxStringItem aURL(SID_FILE_NAME, OUString(".component:Bibliography/View1"));
            SfxStringItem aRef(SID_REFERER, OUString("private:user"));
            SfxStringItem aTarget(SID_TARGETNAME, OUString("_blank"));
            SfxViewFrame::Current()->GetDispatcher()->Execute( SID_OPENDOC, SfxCallMode::ASYNCHRON, &aURL, &aRef, &aTarget, 0L);
        }
        catch (const Exception & e)
        {
            SAL_INFO( "sfx.appl",
                      "trying to load bibliography database, caught " << e.Message);
        }
    }
}
/// Find the correct location of the document (LICENSE.fodt, etc.), and return
/// it in rURL if found.
static bool checkURL( const char *pName, const char *pExt, OUString &rURL )
{
    using namespace osl;
    DirectoryItem aDirItem;

    rURL = "$BRAND_BASE_DIR/" + OUString::createFromAscii( pName ) +
           OUString::createFromAscii( pExt );
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
        Sequence < com::sun::star::beans::PropertyValue > args(2);
        args[0].Name = "ViewOnly";
        args[0].Value <<= sal_True;
        args[1].Name = "ReadOnly";
        args[1].Value <<= sal_True;

        OUString aURL;
        if ( checkURL ( pBaseName, ".fodt", aURL ) ||
             checkURL ( pBaseName, ".html", aURL ) ||
             checkURL ( pBaseName, "", aURL ) ) {
            xDesktop->loadComponentFromURL( aURL, OUString("_blank"), 0, args );
        }
    } catch (const ::com::sun::star::uno::Exception &) {
    }
}

namespace
{
    class LicenseDialog : public ModalDialog
    {
    private:
        DECL_LINK(ShowHdl, void *);
    public:
        LicenseDialog(vcl::Window *pParent=NULL);
    };

    LicenseDialog::LicenseDialog(vcl::Window *pParent)
        : ModalDialog(pParent, "LicenseDialog", "sfx/ui/licensedialog.ui")
    {
        get<PushButton>("show")->SetClickHdl(LINK(this, LicenseDialog, ShowHdl));
    }

    IMPL_LINK_NOARG(LicenseDialog, ShowHdl)
    {
        EndDialog(RET_OK);
        showDocument("LICENSE");
        return 0;
    }
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
        case SID_EXITANDRETURN:
        case SID_LOGOUT:
        {
            // protect against reentrant calls
            if ( pAppData_Impl->bInQuit )
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

                    if (pObjSh->PrepareClose(true))
                        pObjSh->SetModified( false );
                    else
                        return;
                }

                SfxStringItem aNameItem( SID_FILE_NAME, OUString("vnd.sun.star.cmd:logout") );
                SfxStringItem aReferer( SID_REFERER, "private/user" );
                pAppData_Impl->pAppDispat->Execute( SID_OPENDOC, SfxCallMode::SLOT, &aNameItem, &aReferer, 0L );
                return;
            }

            // aus verschachtelten Requests nach 100ms nochmal probieren
            if( Application::GetDispatchLevel() > 1 )
            {
                /* Dont save the request for closing the application and try it later
                   again. This is an UI bound functionality ... and the user will  try it again
                   if the dialog is closed. But we should not close the application automatically
                   if this dialog is closed by the user ...
                   So we ignore this request now and wait for a new user decision.
                */
                OSL_TRACE( "QueryExit => sal_False (DispatchLevel == %u)", Application::GetDispatchLevel() );
                return;
            }

            // block reentrant calls
            pAppData_Impl->bInQuit = true;
            Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );

            rReq.ForgetAllArgs();

            // if terminate() failed, pAppData_Impl->bInQuit will now be sal_False, allowing further calls of SID_QUITAPP
            bool bTerminated = xDesktop->terminate();
            if (!bTerminated)
                // if terminate() was successful, SfxApplication is now dead!
                pAppData_Impl->bInQuit = false;

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

            if ( pFact )
            {
                SFX_REQUEST_ARG(rReq, pStringItem,
                    SfxStringItem, SID_CONFIG, false);

                SfxItemSet aSet(
                    GetPool(), SID_CONFIG, SID_CONFIG );

                if ( pStringItem )
                {
                    aSet.Put( SfxStringItem(
                        SID_CONFIG, pStringItem->GetValue() ) );
                }

                Reference< XFrame > xFrame;
                const SfxItemSet* pIntSet = rReq.GetInternalArgs_Impl();
                SFX_ITEMSET_ARG( pIntSet, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, false );
                if ( pFrameItem )
                    xFrame = pFrameItem->GetFrame();

                boost::scoped_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateTabDialog(
                    RID_SVXDLG_CUSTOMIZE,
                    NULL, &aSet, xFrame ));

                if ( pDlg )
                {
                    const short nRet = pDlg->Execute();

                    if ( nRet )
                        bDone = true;
                }
            }
            break;
        }

        case SID_CLOSEDOCS:
        case SID_CLOSEWINS:
        {

            Reference < XDesktop2 > xDesktop  = Desktop::create( ::comphelper::getProcessComponentContext() );
            Reference< XIndexAccess > xTasks( xDesktop->getFrames(), UNO_QUERY );
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
                    xTask->close(sal_True);
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
                    const SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, aReq.GetReturnValue() );
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
            OUString sURL("http://hub.libreoffice.org/send-feedback/?LOversion=" + utl::ConfigManager::getAboutBoxProductVersion() +
                "&LOlocale=" + utl::ConfigManager::getLocale() + "&LOmodule=" + module.copy(module.lastIndexOf('.') + 1 )  );
            try
            {
                uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                    com::sun::star::system::SystemShellExecute::create(::comphelper::getProcessComponentContext()) );
                xSystemShellExecute->execute( sURL, OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
            }
            catch ( uno::Exception& )
            {
            }
            break;
        }

        case SID_SHOW_LICENSE:
        {
            LicenseDialog aDialog;
            aDialog.Execute();
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
                pHelp->Start( OUString(".uno:HelpIndex"), NULL ); // show start page
                bDone = true;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPTIPS:
        {
            // Evaluate Parameter
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPTIPS, false);
            bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsQuickHelpEnabled();

            if ( bOn )
                Help::EnableQuickHelp();
            else
                Help::DisableQuickHelp();
            SvtHelpOptions().SetHelpTips( bOn );
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
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPBALLOONS, false);
            bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsBalloonHelpEnabled();

            if ( bOn )
                Help::EnableBalloonHelp();
            else
                Help::DisableBalloonHelp();
            SvtHelpOptions().SetExtendedHelp( bOn );
            Invalidate(SID_HELPBALLOONS);
            bDone = true;

            // Record if possible
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPBALLOONS, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ABOUT:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                boost::scoped_ptr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( 0, RID_DEFAULTABOUT ));
                pDlg->Execute();
                bDone = true;
            }
            break;
        }

        case SID_TEMPLATE_MANAGER:
        {
            SfxTemplateManagerDlg dlg;
            dlg.Execute();
            bDone = true;
            break;
        }

        case SID_TEMPLATE_ADDRESSBOKSOURCE:
        {
            svt::AddressBookSourceDialog aDialog(GetTopWindow(), ::comphelper::getProcessComponentContext());
            aDialog.Execute();
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

        case SID_SHOW_IME_STATUS_WINDOW:
            if (pAppData_Impl->m_xImeStatusWindow->canToggle())
            {
                SfxBoolItem const * pItem = static_cast< SfxBoolItem const * >(
                    rReq.GetArg(SID_SHOW_IME_STATUS_WINDOW, false,
                                TYPE(SfxBoolItem)));
                bool bShow = pItem == 0
                    ? !pAppData_Impl->m_xImeStatusWindow->isShowing()
                    : pItem->GetValue();
                pAppData_Impl->m_xImeStatusWindow->show(bShow);
                if (pItem == 0)
                    rReq.AppendItem(SfxBoolItem(SID_SHOW_IME_STATUS_WINDOW,
                                                bShow));
            }
            bDone = true;
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
            if (!pCurrentShell)
                return;

            // make sure aZoom is initialized with a proper value if SetType
            // doesn't work
            SvxZoomItem aZoom( SVX_ZOOM_PERCENT, 100 );

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
                    aZoom.SetType( SVX_ZOOM_OPTIMAL );
                    break;
                case SID_ZOOM_ENTIRE_PAGE:
                    aZoom.SetType( SVX_ZOOM_WHOLEPAGE );
                    break;
                case SID_ZOOM_PAGE_WIDTH:
                    aZoom.SetType( SVX_ZOOM_PAGEWIDTH );
                    break;
            }

            pCurrentShell->GetDispatcher()->Execute(SID_ATTR_ZOOM, SfxCallMode::ASYNCHRON, &aZoom, 0L);

            break;
        }
        case SID_AVAILABLE_TOOLBARS:
        {
            SfxStringItem const * pToolbarName = static_cast< SfxStringItem const *>(
                    rReq.GetArg(SID_AVAILABLE_TOOLBARS, false, TYPE(SfxStringItem)));

            if ( pToolbarName )
            {
                Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );
                Reference< XFrame > xFrame = xDesktop->getActiveFrame();

                Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                if ( xPropSet.is() )
                {
                    try
                    {
                        Any aValue = xPropSet->getPropertyValue("LayoutManager");
                        aValue >>= xLayoutManager;
                    }
                    catch ( const ::com::sun::star::uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
                    {
                    }
                }

                if ( xLayoutManager.is() )
                {
                    OUString aToolbarResName( "private:resource/toolbar/" );
                    OUStringBuffer aBuf( aToolbarResName );
                    aBuf.append( pToolbarName->GetValue() );

                    // Evaluate Parameter
                    OUString aToolbarName( aBuf.makeStringAndClear() );
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

        default:
            break;
    }

    if ( bDone )
        rReq.Done();
}



void SfxApplication::MiscState_Impl(SfxItemSet &rSet)
{
    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set without range");
    while ( *pRanges )
    {
        for(sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_TEMPLATE_ADDRESSBOKSOURCE:
                    if ( !SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE) )
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                    break;
                case SID_EXITANDRETURN:
                case SID_QUITAPP:
                {
                    if ( pAppData_Impl->nDocModalMode )
                        rSet.DisableItem(nWhich);
                    else
                        rSet.Put(SfxStringItem(nWhich, SfxResId(STR_QUITAPP).toString()));
                    break;
                }

                case SID_CONFIG:
                case SID_TOOLBOXOPTIONS:
                case SID_CONFIGSTATUSBAR:
                case SID_CONFIGMENU:
                case SID_CONFIGACCEL:
                case SID_CONFIGEVENT:
                {
                    if( SvtMiscOptions().DisableUICustomization() )
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
                case SID_CLOSEWINS:
                {
                    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
                    Reference< XIndexAccess > xTasks( xDesktop->getFrames(), UNO_QUERY );
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

                case SID_SHOW_IME_STATUS_WINDOW:
                    if (pAppData_Impl->m_xImeStatusWindow->canToggle())
                        rSet.Put(SfxBoolItem(
                                     SID_SHOW_IME_STATUS_WINDOW,
                                     pAppData_Impl->m_xImeStatusWindow->
                                         isShowing()));
                    else
                        rSet.DisableItem(SID_SHOW_IME_STATUS_WINDOW);
                    break;

                case SID_TEMPLATE_MANAGER:
                    {
                        SvtMiscOptions aMiscOptions;
                        if ( !aMiscOptions.IsExperimentalMode() )
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

                default:
                    break;
            }
        }

        ++pRanges;
    }
}

#if HAVE_FEATURE_SCRIPTING

#ifndef DISABLE_DYNLOADING

typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(XModel*, sal_Bool, rtl_uString*);
typedef void (SAL_CALL *basicide_macro_organizer)( sal_Int16 );

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" rtl_uString* basicide_choose_macro(XModel*, sal_Bool, rtl_uString*);
extern "C" void basicide_macro_organizer( sal_Int16 );

#endif

OUString ChooseMacro( const Reference< XModel >& rxLimitToDocument, bool bChooseOnly, const OUString& rMacroDesc = OUString() )
{
#ifndef DISABLE_DYNLOADING
    osl::Module aMod;

    // load basctl module
    aMod.loadRelative(&thisModule, SVLIBRARY("basctl"), 0);

    // get symbol
    basicide_choose_macro pSymbol = (basicide_choose_macro) aMod.getFunctionSymbol("basicide_choose_macro");
    SAL_WARN_IF(!pSymbol, "sfx.doc", "SfxApplication::MacroOrganizer, no symbol!");
    if (!pSymbol)
        return OUString();
    aMod.release();
#else
#define pSymbol basicide_choose_macro
#endif

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol( rxLimitToDocument.get(), bChooseOnly, rMacroDesc.pData );
    OUString aScriptURL( pScriptURL );
    rtl_uString_release( pScriptURL );
    return aScriptURL;

#ifdef DISABLE_DYNLOADING
#undef pSymbol
#endif
}

#endif

ResMgr* SfxApplication::GetOffResManager_Impl()
{
    if ( !pAppData_Impl->pOfaResMgr )
        pAppData_Impl->pOfaResMgr = ResMgr::CreateResMgr( "ofa");
    return pAppData_Impl->pOfaResMgr;
}

namespace
{
#if HAVE_FEATURE_SCRIPTING
    vcl::Window* lcl_getDialogParent( const Reference< XFrame >& _rxFrame, vcl::Window* _pFallback )
    {
        if ( !_rxFrame.is() )
            return _pFallback;

        try
        {
            Reference< awt::XWindow > xContainerWindow( _rxFrame->getContainerWindow(), UNO_SET_THROW );
            vcl::Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            OSL_ENSURE( pWindow, "lcl_getDialogParent: cool, somebody implemented a VCL-less toolkit!" );

            if ( pWindow )
                return pWindow->GetSystemWindow();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return _pFallback;
    }

    static OUString lcl_getBasicIDEServiceName()
    {
        return OUString( "com.sun.star.script.BasicIDE");
    }

    SfxViewFrame* lcl_getBasicIDEViewFrame( SfxObjectShell* i_pBasicIDE )
    {
        SfxViewFrame* pView = SfxViewFrame::GetFirst( i_pBasicIDE );
        while ( pView )
        {
            if ( pView->GetObjectShell()->GetFactory().GetDocumentServiceName() == lcl_getBasicIDEServiceName() )
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
               DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
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
            SFX_REQUEST_ARG( rReq, pURLItem, SfxStringItem, SID_OPTIONS_PAGEURL, false );
            if ( pURLItem )
                sPageURL = pURLItem->GetValue();
            const SfxItemSet* pArgs = rReq.GetInternalArgs_Impl();
            const SfxPoolItem* pItem = NULL;
            Reference < XFrame > xFrame;
            if ( pArgs && pArgs->GetItemState( SID_FILLFRAME, false, &pItem ) == SfxItemState::SET )
            {
                OSL_ENSURE( pItem->ISA( SfxUnoFrameItem ), "SfxApplication::OfaExec_Impl: XFrames are to be transported via SfxUnoFrameItem by now!" );
                xFrame = static_cast< const SfxUnoFrameItem*>( pItem )->GetFrame();
            }
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                VclAbstractDialog* pDlg =
                    pFact->CreateFrameDialog( NULL, xFrame, rReq.GetSlot(), sPageURL );
                short nRet = pDlg->Execute();
                delete pDlg;
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
            }
            break;
        }

        case SID_MORE_DICTIONARIES:
        {
            try
            {
                uno::Reference< uno::XComponentContext > xContext =
                    ::comphelper::getProcessComponentContext();
                uno::Reference< css::system::XSystemShellExecute > xSystemShell(
                    css::system::SystemShellExecute::create(xContext) );

                // read repository URL from configuration
                OUString sTemplRepoURL(officecfg::Office::Common::Dictionaries::RepositoryURL::get());

                if ( xSystemShell.is() && !sTemplRepoURL.isEmpty() )
                {
                    OUStringBuffer aURLBuf( sTemplRepoURL );
                    aURLBuf.append("?lang=");

                    // read locale from configuration
                    OUString sLocale(officecfg::Setup::L10N::ooLocale::get());
                    if (sLocale.isEmpty())
                        sLocale = "en-US";

                    aURLBuf.append( sLocale );
                    xSystemShell->execute(
                        aURLBuf.makeStringAndClear(),
                        OUString(),
                        css::system::SystemShellExecuteFlags::URIS_ONLY );
                }
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
                SAL_WARN( "sfx.appl", "SfxApplication::OfaExec_Impl(SID_MORE_DICTIONARIES): caught an exception!" );
            }
            break;
        }
#if HAVE_FEATURE_SCRIPTING
        case SID_BASICIDE_APPEAR:
        {
            SfxViewFrame* pView = lcl_getBasicIDEViewFrame( NULL );
            if ( !pView )
            {
                SfxObjectShell* pBasicIDE = SfxObjectShell::CreateObject( lcl_getBasicIDEServiceName() );
                pBasicIDE->DoInitNew( 0 );
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
                    DBG_UNHANDLED_EXCEPTION();
                }

                pView = lcl_getBasicIDEViewFrame( pBasicIDE );
                if ( pView )
                    pView->SetName( OUString( "BASIC:1" ) );
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

            rReq.SetReturnValue( SfxStringItem( rReq.GetSlot(), ChooseMacro( xLimitToModel, bChooseOnly ) ) );
            rReq.Done();
        }
        break;

        case SID_MACROORGANIZER:
        {
            OSL_TRACE("handling SID_MACROORGANIZER");
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            sal_Int16 nTabId = 0;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_MACROORGANIZER, false, &pItem) )
            {
                nTabId = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            }

            SfxApplication::MacroOrganizer( nTabId );
            rReq.Done();
        }
        break;

        case SID_RUNMACRO:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            OSL_TRACE("SfxApplication::OfaExec_Impl: case ScriptOrg");

            Reference< XFrame > xFrame;
            const SfxItemSet* pIntSet = rReq.GetInternalArgs_Impl();
            SFX_ITEMSET_ARG( pIntSet, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, false );
            if ( pFrameItem )
                xFrame = pFrameItem->GetFrame();

            if ( !xFrame.is() )
            {
                const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
                if ( pViewFrame )
                    xFrame = pViewFrame->GetFrame().GetFrameInterface();
            }

            do  // artificial loop for flow control
            {
                boost::scoped_ptr<AbstractScriptSelectorDialog> pDlg(pFact->CreateScriptSelectorDialog(
                    lcl_getDialogParent( xFrame, GetTopWindow() ), false, xFrame ));
                OSL_ENSURE( pDlg, "SfxApplication::OfaExec_Impl( SID_RUNMACRO ): no dialog!" );
                if ( !pDlg )
                    break;
                pDlg->SetRunLabel();

                short nDialogResult = pDlg->Execute();
                if ( !nDialogResult )
                    break;

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
            }
            while ( false );
            rReq.Done();
        }
        break;

        case SID_SCRIPTORGANIZER:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            OSL_TRACE("SfxApplication::OfaExec_Impl: case ScriptOrg");
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            OUString aLanguage;
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_SCRIPTORGANIZER, false, &pItem) )
            {
                aLanguage = static_cast<const SfxScriptOrganizerItem*>(pItem)->getLanguage();
            }

            OUString aLang( aLanguage );
            OSL_TRACE("SfxApplication::OfaExec_Impl: about to create dialog for: %s", OUStringToOString( aLang , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
            // not sure about the vcl::Window*
            VclAbstractDialog* pDlg = pFact->CreateSvxScriptOrgDialog( GetTopWindow(), aLanguage );
            if( pDlg )
            {
                pDlg->Execute();
                delete pDlg;
            }
            else
            {
                OSL_TRACE("no dialog!!!");
            }
            rReq.Done();
        }
        break;
#endif // HAVE_FEATURE_SCRIPTING

        case SID_OFFICE_CHECK_PLZ:
        {
            bool bRet = false;
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, rReq.GetSlot(), false);

            if ( pStringItem )
            {
                bRet = true /*!!!SfxIniManager::CheckPLZ( aPLZ )*/;
            }
#if HAVE_FEATURE_SCRIPTING
            else
                SbxBase::SetError( SbxERR_WRONG_ARGS );
#endif
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bRet ) );
        }
        break;

        case SID_AUTO_CORRECT_DLG:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                SfxItemSet aSet(GetPool(), SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG);
                const SfxPoolItem* pItem=NULL;
                const SfxItemSet* pSet = rReq.GetArgs();
                SfxItemPool* pSetPool = pSet ? pSet->GetPool() : NULL;
                if ( pSet && pSet->GetItemState( pSetPool->GetWhich( SID_AUTO_CORRECT_DLG ), false, &pItem ) == SfxItemState::SET )
                    aSet.Put( *pItem );

                boost::scoped_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateTabDialog( RID_OFA_AUTOCORR_DLG, NULL, &aSet, NULL ));
                pDlg->Execute();
            }

            break;
        }

        case SID_SD_AUTOPILOT :
        case SID_NEWSD :
        {
            SvtModuleOptions aModuleOpt;
            if ( !aModuleOpt.IsImpress() )
            {
                MessageDialog( 0, SFX2_RESSTR( STR_MODULENOTINSTALLED )).Execute();
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
                Reference< ui::dialogs::XExecutableDialog > xDialog;
                xDialog = ui::dialogs::AddressBookSourcePilot::createWithParent(xORB, 0);
                xDialog->execute();
            }
            catch(const ::com::sun::star::uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
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

    if ( !aModuleOpt.IsImpress() )
        rSet.DisableItem( SID_SD_AUTOPILOT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
