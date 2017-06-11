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

#include <memory>
#include <config_features.h>
#include <config_folders.h>

#include <sal/config.h>

#include <iostream>
#include <mutex>
#if defined UNX
#include <signal.h>
#endif

#include "app.hxx"
#include "dp_shared.hxx"
#include "strings.hrc"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"
#include "dispatchwatcher.hxx"
#include "lockfile.hxx"
#include "userinstall.hxx"
#include "desktopcontext.hxx"
#include "migration.hxx"
#if HAVE_FEATURE_UPDATE_MAR
#include "updater.hxx"
#endif

#include <o3tl/runtimetooustring.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/javacontext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/theAutoRecovery.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/SessionListener.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/StartModule.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/task/theJobExecutor.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/office/Quickstart.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

#include <desktop/exithelper.h>
#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/fileurl.hxx>
#include <comphelper/threadpool.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/backupfilehelper.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <officecfg/Office/Update.hxx>
#include <officecfg/Setup.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/uri.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/menuoptions.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <sfx2/safemode.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <basic/sbstar.hxx>
#include <desktop/crashreport.hxx>

#include <svtools/fontsubstconfig.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/graphicfilter.hxx>

#include "langselect.hxx"

#if HAVE_FEATURE_BREAKPAD
#include <fstream>
#endif

#if defined MACOSX
#include <errno.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#pragma warning (disable: 4005)
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif //WNT

#if defined(_WIN32)
#include <process.h>
#define GETPID _getpid
#else
#include <unistd.h>
#define GETPID getpid
#endif

#include "strings.hxx"

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::container;

ResMgr* desktop::Desktop::pResMgr = nullptr;

namespace desktop
{

static oslSignalHandler pSignalHandler = nullptr;

namespace {

#if HAVE_FEATURE_EXTENSIONS

// Remove any existing UserInstallation's extensions cache data remaining from
// old installations.  This addresses at least two problems:
//
// For one, apparently due to the old share/prereg/bundled mechanism (disabled
// since 5c47e5f63a79a9e72ec4a100786b1bbf65137ed4 "fdo#51252 Disable copying
// share/prereg/bundled to avoid startup crashes"), the user/extensions/bundled
// cache could contain corrupted information (like a UNO component registered
// twice, which got changed from active to passive registration in one LO
// version, but the version of the corresponding bundled extension only
// incremented in a later LO version).
//
// For another, UserInstallations have been seen in the wild where no extensions
// were installed per-user (any longer), but user/uno_packages/cache/registry/
// com.sun.star.comp.deployment.component.PackageRegistryBackend/*.rdb files
// contained data nevertheless.
//
// When a LO upgrade is detected (i.e., no user/extensions/buildid or one
// containing an old build ID), then user/extensions and
// user/uno_packages/cache/registry/
// com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc are
// removed.  That should prevent any problems starting the service manager due
// to old junk.  Later on in Desktop::SynchronizeExtensionRepositories, the
// removed cache data is recreated.
//
// Multiple instances of soffice.bin can execute this code in parallel for a
// single UserInstallation, as it is called before RequestHandler is set up.
// Therefore, any errors here only lead to SAL_WARNs.
//
// At least in theory, this function could be removed again once no
// UserInstallation can be poisoned by old junk any more.
bool cleanExtensionCache() {
    OUString buildId(
        "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(buildId); //TODO: detect failure
    OUString extDir(
        "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap")
        ":UserInstallation}/user/extensions");
    rtl::Bootstrap::expandMacros(extDir); //TODO: detect failure
    OUString buildIdFile(extDir + "/buildid");
    osl::File fr(buildIdFile);
    osl::FileBase::RC rc = fr.open(osl_File_OpenFlag_Read);
    switch (rc) {
    case osl::FileBase::E_None:
        {
            rtl::ByteSequence s1;
            rc = fr.readLine(s1);
            osl::FileBase::RC rc2 = fr.close();
            SAL_WARN_IF(
                rc2 != osl::FileBase::E_None, "desktop.app",
                "cannot close " << fr.getURL() << " after reading: " << +rc2);
            // readLine returns E_AGAIN for a zero-size file:
            if (rc != osl::FileBase::E_None && rc != osl::FileBase::E_AGAIN) {
                SAL_WARN( "desktop.app", "cannot read from " << fr.getURL() << ": " << +rc);
                break;
            }
            OUString s2(
                reinterpret_cast< char const * >(s1.getConstArray()),
                s1.getLength(), RTL_TEXTENCODING_ISO_8859_1);
                // using ISO 8859-1 avoids any and all conversion errors; the
                // content should only be a subset of ASCII, anyway
            if (s2 == buildId) {
                return false;
            }
            break;
        }
    case osl::FileBase::E_NOENT:
        break;
    default:
        SAL_WARN( "desktop.app", "cannot open " << fr.getURL() << " for reading: " << +rc);
        break;
    }
    utl::removeTree(extDir);
    OUString userRcFile(
        "$UNO_USER_PACKAGES_CACHE/registry/"
        "com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc");
    rtl::Bootstrap::expandMacros(userRcFile); //TODO: detect failure
    rc = osl::File::remove(userRcFile);
    SAL_WARN_IF(
        rc != osl::FileBase::E_None && rc != osl::FileBase::E_NOENT, "desktop.app",
        "cannot remove file " << userRcFile << ": " << +rc);
    rc = osl::Directory::createPath(extDir);
    SAL_WARN_IF(
        rc != osl::FileBase::E_None && rc != osl::FileBase::E_EXIST, "desktop.app",
        "cannot create path " << extDir << ": " << +rc);
    osl::File fw(buildIdFile);
    rc = fw.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    if (rc != osl::FileBase::E_None) {
        SAL_WARN( "desktop.app", "cannot open " << fw.getURL() << " for writing: " << +rc);
        return true;
    }
    OString buf(OUStringToOString(buildId, RTL_TEXTENCODING_UTF8));
        // using UTF-8 avoids almost all conversion errors (and buildid
        // containing single surrogate halves should never happen, anyway); the
        // content should only be a subset of ASCII, anyway
    sal_uInt64 n = 0;
    rc = fw.write(buf.getStr(), buf.getLength(), n);
    SAL_WARN_IF(
        (rc != osl::FileBase::E_None
         || n != static_cast< sal_uInt32 >(buf.getLength())),
        "desktop.app",
        "cannot write to " << fw.getURL() << ": " << +rc << ", " << n);
    rc = fw.close();
    SAL_WARN_IF(
        rc != osl::FileBase::E_None, "desktop.app",
        "cannot close " << fw.getURL() << " after writing: " << +rc);
    return true;
}

#endif

bool shouldLaunchQuickstart()
{
    bool bQuickstart = Desktop::GetCommandLineArgs().IsQuickstart();
    if (!bQuickstart)
    {
        const SfxPoolItem* pItem=nullptr;
        SfxItemSet aQLSet(SfxGetpApp()->GetPool(), svl::Items<SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER>{});
        SfxGetpApp()->GetOptions(aQLSet);
        SfxItemState eState = aQLSet.GetItemState(SID_ATTR_QUICKLAUNCHER, false, &pItem);
        if (SfxItemState::SET == eState)
            bQuickstart = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    return bQuickstart;
}

void SetRestartState() {
    try {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Setup::Office::OfficeRestartInProgress::set(true, batch);
        batch->commit();
    } catch (css::uno::Exception & e) {
        SAL_WARN("desktop.app", "ignoring Exception \"" << e.Message << "\"");
    }
}

void DoRestartActionsIfNecessary(bool quickstart) {
    if (quickstart) {
        try {
            if (officecfg::Setup::Office::OfficeRestartInProgress::get()) {
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Setup::Office::OfficeRestartInProgress::set(
                    false, batch);
                batch->commit();
                css::office::Quickstart::createStart(
                    comphelper::getProcessComponentContext(),
                    shouldLaunchQuickstart());
            }
        } catch (css::uno::Exception & e) {
            SAL_WARN(
                "desktop.app", "ignoring Exception \"" << e.Message << "\"");
        }
    }
}

}


ResMgr* Desktop::GetDesktopResManager()
{
    if ( !Desktop::pResMgr )
    {
        // Create desktop resource manager and bootstrap process
        // was successful. Use default way to get language specific message.
        if ( Application::IsInExecute() )
            Desktop::pResMgr = ResMgr::CreateResMgr("dkt");

        if ( !Desktop::pResMgr )
        {
            // Use VCL to get the correct language specific message as we
            // are in the bootstrap process and not able to get the installed
            // language!!
            OUString aUILocaleString = langselect::getEmergencyLocale();
            LanguageTag aLanguageTag( aUILocaleString);
            //! ResMgr may modify the Locale for fallback!
            Desktop::pResMgr = ResMgr::SearchCreateResMgr( "dkt", aLanguageTag);
        }
    }

    return Desktop::pResMgr;
}

namespace {


OUString MakeStartupErrorMessage(OUString const & aErrorMessage)
{
    OUStringBuffer    aDiagnosticMessage( 100 );

    aDiagnosticMessage.append(DpResId(STR_BOOTSTRAP_ERR_CANNOT_START));

    aDiagnosticMessage.append( "\n" );

    aDiagnosticMessage.append( aErrorMessage );

    return aDiagnosticMessage.makeStringAndClear();
}

OUString MakeStartupConfigAccessErrorMessage( OUString const & aInternalErrMsg )
{
    OUStringBuffer aDiagnosticMessage( 200 );

    aDiagnosticMessage.append(DpResId(STR_BOOTSTRAP_ERR_CFG_DATAACCESS));

    if ( !aInternalErrMsg.isEmpty() )
    {
        aDiagnosticMessage.append("\n\n");
        aDiagnosticMessage.append(DpResId(STR_INTERNAL_ERRMSG));
        aDiagnosticMessage.append(aInternalErrMsg);
    }

    return aDiagnosticMessage.makeStringAndClear();
}


// shows a simple error box with the given message ... but exits from these process !
// Fatal errors can't be solved by the process ... nor any recovery can help.
// Mostly the installation was damaged and must be repaired manually .. or by calling
// setup again.
// On the other side we must make sure that no further actions will be possible within
// the current office process ! No pipe requests, no menu/toolbar/shortcut actions
// are allowed. Otherwise we will force a "crash inside a crash".
// Thats why we have to use a special native message box here which does not use yield :-)

void FatalError(const OUString& sMessage)
{
    OUString sProductKey = ::utl::Bootstrap::getProductKey();
    if ( sProductKey.isEmpty())
    {
        osl_getExecutableFile( &sProductKey.pData );

        ::sal_uInt32 nLastIndex = sProductKey.lastIndexOf('/');
        if ( nLastIndex > 0 )
            sProductKey = sProductKey.copy( nLastIndex+1 );
    }

    OUStringBuffer sTitle (128);
    sTitle.append      (sProductKey     );
    sTitle.append (" - Fatal Error");

    Application::ShowNativeErrorBox (sTitle.makeStringAndClear (), sMessage);
    _exit(EXITHELPER_FATAL_ERROR);
}

struct theCommandLineArgs : public rtl::Static< CommandLineArgs, theCommandLineArgs > {};

}

CommandLineArgs& Desktop::GetCommandLineArgs()
{
    return theCommandLineArgs::get();
}

namespace
{
    struct OOOVendor
        : public rtl::Static< OUString, OOOVendor > {};
}

OUString ReplaceStringHookProc( const OUString& rStr )
{
    const static OUString sBuildId(utl::Bootstrap::getBuildIdData("development"));
    static OUString sBrandName, sVersion, sAboutBoxVersion, sAboutBoxVersionSuffix, sExtension;

    static std::once_flag aInitOnce;
    std::call_once(aInitOnce, []
    {
        sBrandName = utl::ConfigManager::getProductName();
        sVersion = utl::ConfigManager::getProductVersion();
        sAboutBoxVersion = utl::ConfigManager::getAboutBoxProductVersion();
        sAboutBoxVersionSuffix = utl::ConfigManager::getAboutBoxProductVersionSuffix();
        sExtension = utl::ConfigManager::getProductExtension();
    } );

    OUString sRet(rStr);
    if (sRet.indexOf("%PRODUCT") != -1 || sRet.indexOf("%ABOUTBOX") != -1)
    {
        sRet = sRet.replaceAll( "%PRODUCTNAME", sBrandName );
        sRet = sRet.replaceAll( "%PRODUCTVERSION", sVersion );
        sRet = sRet.replaceAll( "%BUILDID", sBuildId );
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSIONSUFFIX", sAboutBoxVersionSuffix );
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSION", sAboutBoxVersion );
        sRet = sRet.replaceAll( "%PRODUCTEXTENSION", sExtension );
    }

    if ( sRet.indexOf( "%OOOVENDOR" ) != -1 )
    {
        OUString sOOOVendor = OOOVendor::get();

        if ( sOOOVendor.isEmpty() )
        {
            sOOOVendor = utl::ConfigManager::getVendor();
        }

        sRet = sRet.replaceAll( "%OOOVENDOR", sOOOVendor );
    }

    return sRet;
}

Desktop::Desktop()
    : m_bCleanedExtensionCache(false)
    , m_bServicesRegistered(false)
    , m_aBootstrapError(BE_OK)
    , m_aBootstrapStatus(BS_OK)
{
    m_firstRunTimer.SetTimeout(3000); // 3 sec.
    m_firstRunTimer.SetInvokeHandler(LINK(this, Desktop, AsyncInitFirstRun));
    m_firstRunTimer.SetDebugName( "desktop::Desktop m_firstRunTimer" );
}

Desktop::~Desktop()
{
}

void Desktop::Init()
{
    SetBootstrapStatus(BS_OK);

#if HAVE_FEATURE_EXTENSIONS
    m_bCleanedExtensionCache = cleanExtensionCache();
#endif

    // We need to have service factory before going further, but see fdo#37195.
    // Doing this will mmap common.rdb, making it not overwritable on windows,
    // so this can't happen before the synchronization above. Lets rework this
    // so that the above is called *from* CreateApplicationServiceManager or
    // something to enforce this gotcha
    try
    {
        InitApplicationServiceManager();
    }
    catch (css::uno::Exception & e)
    {
        SetBootstrapError( BE_UNO_SERVICEMANAGER, e.Message );
    }

    // Check whether safe mode is enabled
    CommandLineArgs& rCmdLine = GetCommandLineArgs();
    // Check if we are restarting from safe mode - in that case we don't want to enter it again
    if (sfx2::SafeMode::hasRestartFlag())
        sfx2::SafeMode::removeRestartFlag();
    else if (rCmdLine.IsSafeMode() || sfx2::SafeMode::hasFlag())
        Application::EnableSafeMode();

    // When we are in SafeMode we need to do changes before the configuration
    // gets read (langselect::prepareLocale() by UNO API -> Components::Components)
    // This may prepare SafeMode or restore from it by moving data in
    // the UserConfiguration directory
    comphelper::BackupFileHelper::reactOnSafeMode(Application::IsSafeModeEnabled());

    if ( m_aBootstrapError == BE_OK )
    {
        try
        {
            if (!langselect::prepareLocale())
            {
                SetBootstrapError( BE_LANGUAGE_MISSING, OUString() );
            }
        }
        catch (css::uno::Exception & e)
        {
            SetBootstrapError( BE_OFFICECONFIG_BROKEN, e.Message );
        }

        // test code for ProfileSafeMode to allow testing the fail
        // of loading the office configuration initially. To use,
        // either set to true and compile, or set a breakpoint
        // in debugger and change the local bool
        static bool bTryHardOfficeconfigBroken(false);

        if (bTryHardOfficeconfigBroken)
        {
            SetBootstrapError(BE_OFFICECONFIG_BROKEN, OUString());
        }
    }

    if ( true )
    {
        const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();

        // start ipc thread only for non-remote offices
        RequestHandler::Status aStatus = RequestHandler::Enable(true);
        if ( aStatus == RequestHandler::IPC_STATUS_PIPE_ERROR )
        {
#if defined ANDROID
            // Ignore crack pipe errors on Android
#else
            // Keep using this oddly named BE_PATHINFO_MISSING value
            // for pipe-related errors on other platforms. Of course
            // this crack with two (if not more) levels of our own
            // error codes hiding the actual system error code is
            // broken, but that is done all over the code, let's leave
            // reengineering that to another year.
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
#endif
        }
        else if ( aStatus == RequestHandler::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
        }
        else if ( aStatus == RequestHandler::IPC_STATUS_2ND_OFFICE )
        {
            // 2nd office startup should terminate after sending cmdlineargs through pipe
            SetBootstrapStatus(BS_TERMINATE);
        }
        else if ( !rCmdLineArgs.GetUnknown().isEmpty()
                  || rCmdLineArgs.IsHelp() || rCmdLineArgs.IsVersion() )
        {
            // disable IPC thread in an instance that is just showing a help message
            RequestHandler::Disable();
        }
        pSignalHandler = osl_addSignalHandler(SalMainPipeExchangeSignal_impl, nullptr);
    }
}

void Desktop::InitFinished()
{
    CloseSplashScreen();
}

void Desktop::DeInit()
{
    try {
        // instead of removing of the configManager just let it commit all the changes
        utl::ConfigManager::storeConfigItems();
        FlushConfiguration();

        // close splashscreen if it's still open
        CloseSplashScreen();
        Reference< XComponent >(
            comphelper::getProcessComponentContext(), UNO_QUERY_THROW )->
            dispose();
        // nobody should get a destroyed service factory...
        ::comphelper::setProcessServiceFactory( nullptr );

        // clear lockfile
        m_xLockfile.reset();

        RequestHandler::Disable();
        if( pSignalHandler )
            osl_removeSignalHandler( pSignalHandler );
    } catch (const RuntimeException&) {
        // someone threw an exception during shutdown
        // this will leave some garbage behind..
    }
}

bool Desktop::QueryExit()
{
    try
    {
        utl::ConfigManager::storeConfigItems();
    }
    catch ( const RuntimeException& )
    {
    }

    const sal_Char SUSPEND_QUICKSTARTVETO[] = "SuspendQuickstartVeto";

    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference< XPropertySet > xPropertySet(xDesktop, UNO_QUERY_THROW);
    xPropertySet->setPropertyValue( SUSPEND_QUICKSTARTVETO, Any(true) );

    bool bExit = xDesktop->terminate();

    if ( !bExit )
    {
        xPropertySet->setPropertyValue( SUSPEND_QUICKSTARTVETO, Any(false) );
    }
    else if (!Application::IsEventTestingModeEnabled())
    {
        FlushConfiguration();
        try
        {
            // it is no problem to call RequestHandler::Disable() more than once
            // it also looks to be threadsafe
            RequestHandler::Disable();
        }
        catch ( const RuntimeException& )
        {
        }

        m_xLockfile.reset();

    }

    return bExit;
}

void Desktop::HandleBootstrapPathErrors( ::utl::Bootstrap::Status aBootstrapStatus, const OUString& aDiagnosticMessage )
{
    if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
    {
        OUString        aProductKey;
        OUString        aTemp;

        osl_getExecutableFile( &aProductKey.pData );
        sal_uInt32     lastIndex = aProductKey.lastIndexOf('/');
        if ( lastIndex > 0 )
            aProductKey = aProductKey.copy( lastIndex+1 );

        aTemp = ::utl::Bootstrap::getProductKey( aProductKey );
        if ( !aTemp.isEmpty() )
            aProductKey = aTemp;

        OUString const aMessage(aDiagnosticMessage + "\n");

        ScopedVclPtrInstance< MessageDialog > aBootstrapFailedBox(nullptr, aMessage);
        aBootstrapFailedBox->SetText( aProductKey );
        aBootstrapFailedBox->Execute();
    }
}

// Create a error message depending on bootstrap failure code and an optional file url
OUString    Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const OUString& aFileURL )
{
    OUString        aMsg;
    OUString        aFilePath;
    bool            bFileInfo = true;

    switch ( nFailureCode )
    {
        /// the shared installation directory could not be located
        case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_PATH_INVALID);
            bFileInfo = false;
        }
        break;

        /// the bootstrap INI file could not be found or read
        case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_FILE_MISSING);
        }
        break;

        /// the bootstrap INI is missing a required entry
        /// the bootstrap INI contains invalid data
         case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
         case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_FILE_CORRUPT);
        }
        break;

        /// the version locator INI file could not be found or read
        case ::utl::Bootstrap::MISSING_VERSION_FILE:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_FILE_MISSING);
        }
        break;

        /// the version locator INI has no entry for this version
        case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_NO_SUPPORT);
        }
        break;

        /// the user installation directory does not exist
        case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_DIR_MISSING);
        }
        break;

        /// some bootstrap data was invalid in unexpected ways
        case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
        {
            aMsg = DpResId(STR_BOOTSTRAP_ERR_INTERNAL);
            bFileInfo = false;
        }
        break;

        case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
        {
            // This needs to be improved, see #i67575#:
            aMsg = "Invalid version file entry";
            bFileInfo = false;
        }
        break;

        case ::utl::Bootstrap::NO_FAILURE:
        {
            OSL_ASSERT(false);
        }
        break;
    }

    if ( bFileInfo )
    {
        OUString aMsgString( aMsg );

        osl::File::getSystemPathFromFileURL( aFileURL, aFilePath );

        aMsgString = aMsgString.replaceFirst( "$1", aFilePath );
        aMsg = aMsgString;
    }

    return MakeStartupErrorMessage( aMsg );
}

void Desktop::HandleBootstrapErrors(
    BootstrapError aBootstrapError, OUString const & aErrorMessage )
{
    if ( aBootstrapError == BE_PATHINFO_MISSING )
    {
        OUString                    aErrorMsg;
        OUString                    aBuffer;
        utl::Bootstrap::Status        aBootstrapStatus;
        utl::Bootstrap::FailureCode    nFailureCode;

        aBootstrapStatus = ::utl::Bootstrap::checkBootstrapStatus( aBuffer, nFailureCode );
        if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
        {
            switch ( nFailureCode )
            {
                case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
                case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
                {
                    aErrorMsg = CreateErrorMsgString( nFailureCode, OUString() );
                }
                break;

                /// the bootstrap INI file could not be found or read
                /// the bootstrap INI is missing a required entry
                /// the bootstrap INI contains invalid data
                case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
                case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
                case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
                {
                    OUString aBootstrapFileURL;

                    utl::Bootstrap::locateBootstrapFile( aBootstrapFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aBootstrapFileURL );
                }
                break;

                /// the version locator INI file could not be found or read
                /// the version locator INI has no entry for this version
                /// the version locator INI entry is not a valid directory URL
                 case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE:
                {
                    OUString aVersionFileURL;

                    utl::Bootstrap::locateVersionFile( aVersionFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aVersionFileURL );
                }
                break;

                /// the user installation directory does not exist
                 case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
                {
                    OUString aUserInstallationURL;

                    utl::Bootstrap::locateUserInstallation( aUserInstallationURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aUserInstallationURL );
                }
                break;

                case ::utl::Bootstrap::NO_FAILURE:
                {
                    OSL_ASSERT(false);
                }
                break;
            }

            HandleBootstrapPathErrors( aBootstrapStatus, aErrorMsg );
        }
    }
    else if ( aBootstrapError == BE_UNO_SERVICEMANAGER || aBootstrapError == BE_UNO_SERVICE_CONFIG_MISSING )
    {
        // Uno service manager is not available. VCL needs a uno service manager to display a message box!!!
        // Currently we are not able to display a message box with a service manager due to this limitations inside VCL.

        // When UNO is not properly initialized, all kinds of things can fail
        // and cause the process to crash. To give the user a hint even if
        // generating and displaying a message box below crashes, print a
        // hard-coded message on stderr first:
        std::cerr
            << "The application cannot be started.\n"
                // STR_BOOTSTRAP_ERR_CANNOT_START
            << (aBootstrapError == BE_UNO_SERVICEMANAGER
                ? "The component manager is not available.\n"
                    // STR_BOOTSTRAP_ERR_NO_SERVICE
                : "The configuration service is not available.\n");
                    // STR_BOOTSTRAP_ERR_NO_CFG_SERVICE
        if ( !aErrorMessage.isEmpty() )
        {
            std::cerr << "(\"" << aErrorMessage << "\")\n";
        }

        // First sentence. We cannot bootstrap office further!
        OUString            aMessage;
        OUStringBuffer        aDiagnosticMessage( 100 );

        OUString aErrorMsg = DpResId(STR_BOOTSTRAP_ERR_NO_CFG_SERVICE);

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.append( "\n" );
        if ( !aErrorMessage.isEmpty() )
        {
            aDiagnosticMessage.append( "(\"" );
            aDiagnosticMessage.append( aErrorMessage );
            aDiagnosticMessage.append( "\")\n" );
        }

        // Due to the fact the we haven't a backup applicat.rdb file anymore it is not possible to
        // repair the installation with the setup executable besides the office executable. Now
        // we have to ask the user to start the setup on CD/installation directory manually!!
        OUString aStartSetupManually(DpResId(STR_ASK_START_SETUP_MANUALLY));

        aDiagnosticMessage.append(aStartSetupManually);
        aMessage = MakeStartupErrorMessage(aDiagnosticMessage.makeStringAndClear());

        FatalError( aMessage);
    }
    else if ( aBootstrapError == BE_OFFICECONFIG_BROKEN )
    {
        // set flag at BackupFileHelper to be able to know if _exit was called and
        // actions are executed after this. This method we are in will not return,
        // but end up in a _exit() call
        comphelper::BackupFileHelper::setExitWasCalled();

        // enter safe mode, too
        sfx2::SafeMode::putFlag();

        OUString msg(DpResId(STR_CONFIG_ERR_ACCESS_GENERAL));
        if (!aErrorMessage.isEmpty()) {
            msg += "\n(\"" + aErrorMessage + "\")";
        }
        FatalError(MakeStartupErrorMessage(msg));
    }
    else if ( aBootstrapError == BE_USERINSTALL_FAILED )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = DpResId(STR_BOOTSTRAP_ERR_USERINSTALL_FAILED);
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if ( aBootstrapError == BE_LANGUAGE_MISSING )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = DpResId(
            //@@@ FIXME: should use an own resource string => #i36213#
            STR_BOOTSTRAP_ERR_LANGUAGE_MISSING);
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if (( aBootstrapError == BE_USERINSTALL_NOTENOUGHDISKSPACE ) ||
             ( aBootstrapError == BE_USERINSTALL_NOWRITEACCESS      ))
    {
        OUString       aUserInstallationURL;
        OUString       aUserInstallationPath;
        OUString       aMessage;
        OUString       aErrorMsg;
        OUStringBuffer aDiagnosticMessage( 100 );

        utl::Bootstrap::locateUserInstallation( aUserInstallationURL );

        if ( aBootstrapError == BE_USERINSTALL_NOTENOUGHDISKSPACE )
            aErrorMsg = DpResId(STR_BOOSTRAP_ERR_NOTENOUGHDISKSPACE);
        else
            aErrorMsg = DpResId(STR_BOOSTRAP_ERR_NOACCESSRIGHTS);

        osl::File::getSystemPathFromFileURL( aUserInstallationURL, aUserInstallationPath );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.append( aUserInstallationPath );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
}


namespace {

bool crashReportInfoExists()
{
#if HAVE_FEATURE_BREAKPAD
    std::string path = CrashReporter::getIniFileName();
    std::ifstream aFile(path);
    while (aFile.good())
    {
        std::string line;
        std::getline(aFile, line);
        int sep = line.find('=');
        if (sep >= 0)
        {
            std::string key = line.substr(0, sep);
            if (key == "DumpFile")
                return true;
        }
    }
#endif
    return false;
}

#if HAVE_FEATURE_BREAKPAD
void handleCrashReport()
{
    static const char SERVICENAME_CRASHREPORT[] = "com.sun.star.comp.svx.CrashReportUI";

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< css::frame::XSynchronousDispatch > xRecoveryUI(
        xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_CRASHREPORT, xContext),
        css::uno::UNO_QUERY_THROW);

    Reference< css::util::XURLTransformer > xURLParser =
        css::util::URLTransformer::create(::comphelper::getProcessComponentContext());

    css::util::URL aURL;
    css::uno::Any aRet = xRecoveryUI->dispatchWithReturnValue(aURL, css::uno::Sequence< css::beans::PropertyValue >());
    bool bRet = false;
    aRet >>= bRet;
}
#endif

#if !defined ANDROID
void handleSafeMode()
{
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< css::frame::XSynchronousDispatch > xSafeModeUI(
        xContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.svx.SafeModeUI", xContext),
        css::uno::UNO_QUERY_THROW);

    css::util::URL aURL;
    css::uno::Any aRet = xSafeModeUI->dispatchWithReturnValue(aURL, css::uno::Sequence< css::beans::PropertyValue >());
    bool bRet = false;
    aRet >>= bRet;
}
#endif

/** @short  check if recovery must be started or not.

    @param  bCrashed [boolean ... out!]
            the office crashed last times.
            But may be there are no recovery data.
            Useful to trigger the error report tool without
            showing the recovery UI.

    @param  bRecoveryDataExists [boolean ... out!]
            there exists some recovery data.

    @param  bSessionDataExists [boolean ... out!]
            there exists some session data.
            Because the user may be logged out last time from its
            unix session...
*/
void impl_checkRecoveryState(bool& bCrashed           ,
                             bool& bRecoveryDataExists,
                             bool& bSessionDataExists )
{
    bCrashed = officecfg::Office::Recovery::RecoveryInfo::Crashed::get() || crashReportInfoExists();
    bool elements = officecfg::Office::Recovery::RecoveryList::get()->
        hasElements();
    bool session
        = officecfg::Office::Recovery::RecoveryInfo::SessionData::get();
    bRecoveryDataExists = elements && !session;
    bSessionDataExists = elements && session;
}


/*  @short  start the recovery wizard.

    @param  bEmergencySave
            differs between EMERGENCY_SAVE and RECOVERY
*/
bool impl_callRecoveryUI(bool bEmergencySave     ,
                         bool bExistsRecoveryData)
{
    static const char COMMAND_EMERGENCYSAVE[] = "vnd.sun.star.autorecovery:/doEmergencySave";
    static const char COMMAND_RECOVERY[] = "vnd.sun.star.autorecovery:/doAutoRecovery";

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< css::frame::XSynchronousDispatch > xRecoveryUI(
        xContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.svx.RecoveryUI", xContext),
        css::uno::UNO_QUERY_THROW);

    Reference< css::util::XURLTransformer > xURLParser =
        css::util::URLTransformer::create(::comphelper::getProcessComponentContext());

    css::util::URL aURL;
    if (bEmergencySave)
        aURL.Complete = COMMAND_EMERGENCYSAVE;
    else if (bExistsRecoveryData)
        aURL.Complete = COMMAND_RECOVERY;
    else
        return false;

    xURLParser->parseStrict(aURL);

    css::uno::Any aRet = xRecoveryUI->dispatchWithReturnValue(aURL, css::uno::Sequence< css::beans::PropertyValue >());
    bool bRet = false;
    aRet >>= bRet;
    return bRet;
}

}

namespace {

void restartOnMac(bool passArguments) {
#if defined MACOSX
    RequestHandler::Disable();
#if HAVE_FEATURE_MACOSX_SANDBOX
    (void) passArguments; // avoid warnings
    OUString aMessage = DpResId(STR_LO_MUST_BE_RESTARTED);

    MessageDialog aRestartBox(NULL, aMessage);
    aRestartBox.Execute();
#else
    OUString execUrl;
    OSL_VERIFY(osl_getExecutableFile(&execUrl.pData) == osl_Process_E_None);
    OUString execPath;
    OString execPath8;
    if ((osl::FileBase::getSystemPathFromFileURL(execUrl, execPath)
         != osl::FileBase::E_None) ||
        !execPath.convertToString(
            &execPath8, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::abort();
    }
    std::vector< OString > args;
    args.push_back(execPath8);
    bool wait = false;
    if (passArguments) {
        sal_uInt32 n = osl_getCommandArgCount();
        for (sal_uInt32 i = 0; i < n; ++i) {
            OUString arg;
            osl_getCommandArg(i, &arg.pData);
            if (arg.match("--accept=")) {
                wait = true;
            }
            OString arg8;
            if (!arg.convertToString(
                    &arg8, osl_getThreadTextEncoding(),
                    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                     RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                std::abort();
            }
            args.push_back(arg8);
        }
    }
    std::vector< char const * > argPtrs;
    for (std::vector< OString >::iterator i(args.begin()); i != args.end();
         ++i)
    {
        argPtrs.push_back(i->getStr());
    }
    argPtrs.push_back(nullptr);
    execv(execPath8.getStr(), const_cast< char ** >(&argPtrs[0]));
    if (errno == ENOTSUP) { // happens when multithreaded on OS X < 10.6
        pid_t pid = fork();
        if (pid == 0) {
            execv(execPath8.getStr(), const_cast< char ** >(&argPtrs[0]));
        } else if (pid > 0) {
            // Two simultaneously running soffice processes lead to two dock
            // icons, so avoid waiting here unless it must be assumed that the
            // process invoking soffice itself wants to wait for soffice to
            // finish:
            if (!wait) {
                return;
            }
            int stat;
            if (waitpid(pid, &stat, 0) == pid && WIFEXITED(stat)) {
                _exit(WEXITSTATUS(stat));
            }
        }
    }
    std::abort();
#endif
#else
    (void) passArguments; // avoid warnings
#endif
}

#if HAVE_FEATURE_UPDATE_MAR
bool isTimeForUpdateCheck()
{
    sal_uInt64 nLastUpdate = officecfg::Office::Update::Update::LastUpdateTime::get();
    sal_uInt64 nNow = tools::Time::GetSystemTicks();

    sal_uInt64 n7DayInMS = 1000 * 60 * 60 * 12 * 1; // 12 hours in ms
    if (nNow - n7DayInMS >= nLastUpdate)
        return true;

    return false;
}
#endif

}

void Desktop::Exception(ExceptionCategory nCategory)
{
    // protect against recursive calls
    static bool bInException = false;

    SystemWindowFlags nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SystemWindowFlags::NOAUTOMODE );
    if ( bInException )
    {
        OUString aDoubleExceptionString;
        Application::Abort( aDoubleExceptionString );
    }

    bInException = true;
    const CommandLineArgs& rArgs = GetCommandLineArgs();

    // save all modified documents ... if it's allowed doing so.
    bool bRestart                           = false;
    bool bAllowRecoveryAndSessionManagement = (
                                                    ( !rArgs.IsNoRestore()                    ) && // some use cases of office must work without recovery
                                                    ( !rArgs.IsHeadless()                     ) &&
                                                    ( nCategory != ExceptionCategory::UserInterface ) && // recovery can't work without UI ... but UI layer seems to be the reason for this crash
                                                    ( Application::IsInExecute()               )    // crashes during startup and shutdown should be ignored (they indicates a corrupt installation ...)
                                                  );
    if ( bAllowRecoveryAndSessionManagement )
    {
        // Save all open documents so they will be reopened
        // the next time the application is started
        // returns true if at least one document could be saved...
        bRestart = impl_callRecoveryUI(
                        true , // force emergency save
                        false);
    }

    FlushConfiguration();

    switch( nCategory )
    {
        case ExceptionCategory::ResourceNotLoaded:
        {
            OUString aResExceptionString;
            Application::Abort( aResExceptionString );
            break;
        }

        default:
        {
            m_xLockfile.reset();

            if( bRestart )
            {
                RequestHandler::Disable();
                if( pSignalHandler )
                    osl_removeSignalHandler( pSignalHandler );

                restartOnMac(false);
                if ( m_rSplashScreen.is() )
                    m_rSplashScreen->reset();

                _exit( EXITHELPER_CRASH_WITH_RESTART );
            }
            else
            {
                Application::Abort( OUString() );
            }

            break;
        }
    }

    OSL_ASSERT(false); // unreachable
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
}

struct ExecuteGlobals
{
    Reference < css::document::XDocumentEventListener > xGlobalBroadcaster;
    bool bRestartRequested;
    bool bUseSystemFileDialog;
    std::unique_ptr<SvtLanguageOptions> pLanguageOptions;
    std::unique_ptr<SvtPathOptions> pPathOptions;

    ExecuteGlobals()
    : bRestartRequested( false )
    , bUseSystemFileDialog( true )
    {}
};

static ExecuteGlobals* pExecGlobals = nullptr;


//This just calls Execute() for all normal uses of LibreOffice, but for
//ui-testing if built with afl-clang-fast++ then on exit it will pseudo-restart
//(up to 100 times)
void Desktop::DoExecute()
{
#if !defined(__AFL_HAVE_MANUAL_CONTROL)
    Execute();
#else
    while (__AFL_LOOP(1000))
    {
        Execute();
        OpenDefault();
    }
#endif
}

int Desktop::Main()
{
    pExecGlobals = new ExecuteGlobals();

    // Remember current context object
    css::uno::ContextLayer layer( css::uno::getCurrentContext() );

    if ( m_aBootstrapError != BE_OK )
    {
        HandleBootstrapErrors( m_aBootstrapError, m_aBootstrapErrorMessage );
        return EXIT_FAILURE;
    }

    BootstrapStatus eStatus = GetBootstrapStatus();
    if (eStatus == BS_TERMINATE) {
        return EXIT_SUCCESS;
    }

    // Detect desktop environment - need to do this as early as possible
    css::uno::setCurrentContext( new DesktopContext( css::uno::getCurrentContext() ) );

    CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();

#if HAVE_FEATURE_DESKTOP
    OUString aUnknown( rCmdLineArgs.GetUnknown() );
    if ( !aUnknown.isEmpty() )
    {
        displayCmdlineHelp( aUnknown );
        return EXIT_FAILURE;
    }
    if ( rCmdLineArgs.IsHelp() )
    {
        displayCmdlineHelp( OUString() );
        return EXIT_SUCCESS;
    }
    if ( rCmdLineArgs.IsVersion() )
    {
        displayVersion();
        return EXIT_SUCCESS;
    }
#endif

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    // Startup screen
    OpenSplashScreen();

    SetSplashScreenProgress(10);

    userinstall::Status inst_fin = userinstall::finalize();
    if (inst_fin != userinstall::EXISTED && inst_fin != userinstall::CREATED)
    {
        SAL_WARN( "desktop.app", "userinstall failed");
        if ( inst_fin == userinstall::ERROR_NO_SPACE )
            HandleBootstrapErrors(
                BE_USERINSTALL_NOTENOUGHDISKSPACE, OUString() );
        else if ( inst_fin == userinstall::ERROR_CANT_WRITE )
            HandleBootstrapErrors( BE_USERINSTALL_NOWRITEACCESS, OUString() );
        else
            HandleBootstrapErrors( BE_USERINSTALL_FAILED, OUString() );
        return EXIT_FAILURE;
    }
    // refresh path information
    utl::Bootstrap::reloadData();
    SetSplashScreenProgress(20);

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< XRestartManager > xRestartManager( OfficeRestartManager::get(xContext) );

    Reference< XDesktop2 > xDesktop;
    try
    {
        RegisterServices(xContext);

        SetSplashScreenProgress(25);

#if HAVE_FEATURE_DESKTOP
        // check user installation directory for lockfile so we can be sure
        // there is no other instance using our data files from a remote host
        m_xLockfile.reset(new Lockfile);

        if ( !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoLockcheck() && !m_xLockfile->check( Lockfile_execWarning ))
        {
            // Lockfile exists, and user clicked 'no'
            return EXIT_FAILURE;
        }

        // check if accessibility is enabled but not working and allow to quit
        if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
        {
            if( !InitAccessBridge() )
                return EXIT_FAILURE;
        }
#endif

        // terminate if requested...
        if( rCmdLineArgs.IsTerminateAfterInit() )
            return EXIT_SUCCESS;

        //  Read the common configuration items for optimization purpose
        if ( !InitializeConfiguration() )
            return EXIT_FAILURE;

#if HAVE_FEATURE_UPDATE_MAR
        const char* pUpdaterTestReplace = std::getenv("LIBO_UPDATER_TEST_REPLACE");
        if (pUpdaterTestReplace || officecfg::Office::Update::Update::Enabled::get())
        {
            // check if we just updated
            bool bUpdateRunning = officecfg::Office::Update::Update::UpdateRunning::get();
            if (bUpdateRunning)
            {
                OUString aSeeAlso = officecfg::Office::Update::Update::SeeAlso::get();
                OUString aOldBuildID = officecfg::Office::Update::Update::OldBuildID::get();

                OUString aBuildID("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
                rtl::Bootstrap::expandMacros(aBuildID);
                if (aOldBuildID == aBuildID)
                {
                    SAL_INFO("desktop", "failed updater");
                }
                else
                {
                    if (!aSeeAlso.isEmpty())
                    {
                        SAL_INFO("desktop", "See also: " << aSeeAlso);
                                Reference< css::system::XSystemShellExecute > xSystemShell(
                        SystemShellExecute::create(::comphelper::getProcessComponentContext()) );

                        xSystemShell->execute( aSeeAlso, OUString(), SystemShellExecuteFlags::URIS_ONLY );
                    }
                }

                // reset all the configuration values,
                // all values need to be read before this code
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                        comphelper::ConfigurationChanges::create());
                officecfg::Office::Update::Update::UpdateRunning::set(false, batch);
                officecfg::Office::Update::Update::SeeAlso::set(OUString(), batch);
                officecfg::Office::Update::Update::OldBuildID::set(OUString(), batch);
                batch->commit();
            }

            osl::DirectoryItem aPatchInfo;
            osl::DirectoryItem::get(Updater::getUpdateInfoURL(), aPatchInfo);
            osl::DirectoryItem aDirectoryItem;
            osl::DirectoryItem::get(Updater::getUpdateDirURL(), aDirectoryItem);

            if (pUpdaterTestReplace || (aPatchInfo.is() && aDirectoryItem.is()))
            {
                OUString aBuildID("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
                rtl::Bootstrap::expandMacros(aBuildID);
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                        comphelper::ConfigurationChanges::create());
                officecfg::Office::Update::Update::OldBuildID::set(aBuildID, batch);
                officecfg::Office::Update::Update::UpdateRunning::set(true, batch);
                batch->commit();
                update();
            }
            else if (isTimeForUpdateCheck())
            {
                sal_uInt64 nNow = tools::Time::GetSystemTicks();
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                        comphelper::ConfigurationChanges::create());
                officecfg::Office::Update::Update::LastUpdateTime::set(nNow, batch);
                batch->commit();
                m_aUpdateThread = std::thread(update_checker);
            }
        }
#endif

        SetSplashScreenProgress(30);

        // create title string
        OUString aTitle(ReplaceStringHookProc(RID_APPTITLE));

#ifdef DBG_UTIL
        //include buildid in non product builds
        aTitle += " [";
        aTitle += utl::Bootstrap::getBuildIdData("development");
        aTitle += "]";
#endif

        SetDisplayName( aTitle );
        SetSplashScreenProgress(35);
        pExecGlobals->pPathOptions.reset( new SvtPathOptions);
        SetSplashScreenProgress(40);

        xDesktop = css::frame::Desktop::create( xContext );

        // create service for loading SFX (still needed in startup)
        pExecGlobals->xGlobalBroadcaster = Reference < css::document::XDocumentEventListener >
            ( css::frame::theGlobalEventBroadcaster::get(xContext), UNO_SET_THROW );

        /* ensure existence of a default window that messages can be dispatched to
           This is for the benefit of testtool which uses PostUserEvent extensively
           and else can deadlock while creating this window from another thread while
           the main thread is not yet in the event loop.
        */
        Application::GetDefaultDevice();

#if HAVE_FEATURE_EXTENSIONS
        // Check if bundled or shared extensions were added /removed
        // and process those extensions (has to be done before checking
        // the extension dependencies!
        SynchronizeExtensionRepositories();
        bool bAbort = CheckExtensionDependencies();
        if ( bAbort )
            return EXIT_FAILURE;

        if (inst_fin == userinstall::CREATED)
        {
            Migration::migrateSettingsIfNecessary();
        }
#endif

        // keep a language options instance...
        pExecGlobals->pLanguageOptions.reset( new SvtLanguageOptions(true));

        css::document::DocumentEvent aEvent;
        aEvent.EventName = "OnStartApp";
        pExecGlobals->xGlobalBroadcaster->documentEventOccured(aEvent);

        SetSplashScreenProgress(50);

        // Backing Component
        bool bCrashed            = false;
        bool bExistsRecoveryData = false;
        bool bExistsSessionData  = false;

        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);

        OUString pidfileName = rCmdLineArgs.GetPidfileName();
        if ( !pidfileName.isEmpty() )
        {
            OUString pidfileURL;

            if ( osl_getFileURLFromSystemPath(pidfileName.pData, &pidfileURL.pData) == osl_File_E_None )
            {
                osl::File pidfile( pidfileURL );
                osl::FileBase::RC rc;

                osl::File::remove( pidfileURL );
                if ( (rc = pidfile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create ) ) == osl::File::E_None )
                {
                    OString pid( OString::number( GETPID() ) );
                    sal_uInt64 written = 0;
                    if ( pidfile.write(pid.getStr(), pid.getLength(), written) != osl::File::E_None )
                    {
                        SAL_WARN("desktop.app", "cannot write pidfile " << pidfile.getURL());
                    }
                    pidfile.close();
                }
                else
                {
                    SAL_WARN("desktop.app", "cannot open pidfile " << pidfile.getURL() << rc);
                }
            }
            else
            {
                SAL_WARN("desktop.app", "cannot get pidfile URL from path" << pidfileName);
            }
        }

        if ( rCmdLineArgs.IsHeadless() || rCmdLineArgs.IsEventTesting() )
        {
            // Ensure that we use not the system file dialogs as
            // headless mode relies on Application::EnableHeadlessMode()
            // which does only work for VCL dialogs!!
            SvtMiscOptions aMiscOptions;
            pExecGlobals->bUseSystemFileDialog = aMiscOptions.UseSystemFileDialog();
            aMiscOptions.SetUseSystemFileDialog( false );
        }

        pExecGlobals->bRestartRequested = xRestartManager->isRestartRequested(
            true);
        if ( !pExecGlobals->bRestartRequested )
        {
            if ((!rCmdLineArgs.WantsToLoadDocument() && !rCmdLineArgs.IsInvisible() && !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsQuickstart()) &&
                (SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::STARTMODULE)) &&
                (!bExistsRecoveryData                                                  ) &&
                (!bExistsSessionData                                                   ) &&
                (!Application::AnyInput( VclInputFlags::APPEVENT )                          ))
            {
                 ShowBackingComponent(this);
            }
        }
    }
    catch ( const css::lang::WrappedTargetException& wte )
    {
        css::uno::Exception te;
        wte.TargetException >>= te;
        FatalError( MakeStartupConfigAccessErrorMessage(wte.Message + te.Message) );
    }
    catch ( const css::uno::Exception& e )
    {
        FatalError( MakeStartupErrorMessage(e.Message) );
    }
    SetSplashScreenProgress(55);

    SvtFontSubstConfig().Apply();

    SvtTabAppearanceCfg aAppearanceCfg;
    SvtTabAppearanceCfg::SetInitialized();
    aAppearanceCfg.SetApplicationDefaults( this );
    SvtAccessibilityOptions aOptions;
    aOptions.SetVCLSettings();
    SetSplashScreenProgress(60);

    if ( !pExecGlobals->bRestartRequested )
    {
        Application::SetFilterHdl( LINK( this, Desktop, ImplInitFilterHdl ) );

        // Preload function depends on an initialized sfx application!
        SetSplashScreenProgress(75);

        // use system window dialogs
        Application::SetSystemWindowMode( SystemWindowFlags::DIALOG );

        SetSplashScreenProgress(80);

        if ( !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoQuickstart() )
            InitializeQuickstartMode( xContext );

        try
        {
            if ( xDesktop.is() )
                xDesktop->addTerminateListener( new RequestHandlerController );
            SetSplashScreenProgress(100);
        }
        catch ( const css::uno::Exception& e )
        {
            FatalError( MakeStartupErrorMessage(e.Message) );
        }

        // FIXME: move this somewhere sensible.
#if HAVE_FEATURE_OPENCL
        CheckOpenCLCompute(xDesktop);
#endif

        // Release solar mutex just before we wait for our client to connect
        {
            SolarMutexReleaser aReleaser;

            // Post user event to startup first application component window
            // We have to send this OpenClients message short before execute() to
            // minimize the risk that this message overtakes type detection construction!!
            Application::PostUserEvent( LINK( this, Desktop, OpenClients_Impl ) );

            // Post event to enable acceptors
            Application::PostUserEvent( LINK( this, Desktop, EnableAcceptors_Impl) );

            // Acquire solar mutex just before we enter our message loop
        }

        // call Application::Execute to process messages in vcl message loop
        try
        {
#if HAVE_FEATURE_JAVA
            // The JavaContext contains an interaction handler which is used when
            // the creation of a Java Virtual Machine fails
            css::uno::ContextLayer layer2(
                new svt::JavaContext( css::uno::getCurrentContext() ) );
#endif
            // check whether the shutdown is caused by restart just before entering the Execute
            pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested ||
                xRestartManager->isRestartRequested(true);

            if ( !pExecGlobals->bRestartRequested )
            {
                // if this run of the office is triggered by restart, some additional actions should be done
                DoRestartActionsIfNecessary( !rCmdLineArgs.IsInvisible() && !rCmdLineArgs.IsNoQuickstart() );

                DoExecute();
            }
        }
        catch(const css::document::CorruptedFilterConfigurationException& exFilterCfg)
        {
            RequestHandler::SetDowning();
            FatalError( MakeStartupErrorMessage(exFilterCfg.Message) );
        }
        catch(const css::configuration::CorruptedConfigurationException& exAnyCfg)
        {
            RequestHandler::SetDowning();
            FatalError( MakeStartupErrorMessage(exAnyCfg.Message) );
        }
        catch( const css::uno::Exception& exUNO)
        {
            RequestHandler::SetDowning();
            FatalError( exUNO.Message);
        }
        catch( const std::exception& exSTD)
        {
            RequestHandler::SetDowning();
            FatalError(o3tl::runtimeToOUString(exSTD.what()));
        }
        catch( ...)
        {
            RequestHandler::SetDowning();
            FatalError( "Caught Unknown Exception: Aborting!");
        }
    }
    else
    {
        if (xDesktop.is())
            xDesktop->terminate();
    }
    // CAUTION: you do not necessarily get here e.g. on the Mac.
    // please put all deinitialization code into doShutdown
    return doShutdown();
}

int Desktop::doShutdown()
{
    if( ! pExecGlobals )
        return EXIT_SUCCESS;

    if (m_aUpdateThread.joinable())
        m_aUpdateThread.join();

    pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested ||
        OfficeRestartManager::get(comphelper::getProcessComponentContext())->
        isRestartRequested(true);
    if ( pExecGlobals->bRestartRequested )
        SetRestartState();

    if (pExecGlobals->xGlobalBroadcaster.is())
    {
        css::document::DocumentEvent aEvent;
        aEvent.EventName = "OnCloseApp";
        pExecGlobals->xGlobalBroadcaster->documentEventOccured(aEvent);
    }

    delete pResMgr;
    pResMgr = nullptr;
    // Restore old value
    const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();
    if ( rCmdLineArgs.IsHeadless() || rCmdLineArgs.IsEventTesting() )
        SvtMiscOptions().SetUseSystemFileDialog( pExecGlobals->bUseSystemFileDialog );

    OUString pidfileName = rCmdLineArgs.GetPidfileName();
    if ( !pidfileName.isEmpty() )
    {
        OUString pidfileURL;

        if ( osl_getFileURLFromSystemPath(pidfileName.pData, &pidfileURL.pData) == osl_File_E_None )
        {
            if ( osl::File::remove( pidfileURL ) != osl::FileBase::E_None )
            {
                SAL_WARN("desktop.app", "shutdown: cannot remove pidfile " << pidfileURL);
            }
        }
        else
        {
            SAL_WARN("desktop.app", "shutdown: cannot get pidfile URL from path" << pidfileName);
        }
    }

    // remove temp directory
    RemoveTemporaryDirectory();

    // flush evtl. configuration changes so that all config files in user
    // dir are written
    FlushConfiguration();

    if (pExecGlobals->bRestartRequested)
    {
        // a restart is already requested, usually due to a configuration change
        // that needs a restart to get active. If this is the case, do not try
        // to use SecureUserConfig to safe this still untested new configuration
    }
    else
    {
        // Test if SecureUserConfig is active. If yes and we are at this point, regular shutdown
        // is in progress and the currently used configuration was working. Try to secure this
        // working configuration for later eventually necessary restores
        comphelper::BackupFileHelper aBackupFileHelper;

        aBackupFileHelper.tryPush();
        aBackupFileHelper.tryPushExtensionInfo();
    }

    // The acceptors in the AcceptorMap must be released (in DeregisterServices)
    // with the solar mutex unlocked, to avoid deadlock:
    {
        SolarMutexReleaser aReleaser;
        DeregisterServices();
#if HAVE_FEATURE_SCRIPTING
        StarBASIC::DetachAllDocBasicItems();
#endif
    }

    // be sure that path/language options gets destroyed before
    // UCB is deinitialized
    pExecGlobals->pLanguageOptions.reset( nullptr );
    pExecGlobals->pPathOptions.reset( nullptr );

    comphelper::ThreadPool::getSharedOptimalPool().shutdown();

    bool bRR = pExecGlobals->bRestartRequested;
    delete pExecGlobals;
    pExecGlobals = nullptr;

    if ( bRR )
    {
        restartOnMac(true);
        if ( m_rSplashScreen.is() )
            m_rSplashScreen->reset();

        return EXITHELPER_NORMAL_RESTART;
    }
    return EXIT_SUCCESS;
}

IMPL_STATIC_LINK( Desktop, ImplInitFilterHdl, ::ConvertData&, rData, bool )
{
    return GraphicFilter::GetGraphicFilter().GetFilterCallback().Call( rData );
}

bool Desktop::InitializeConfiguration()
{
    try
    {
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext() );
        return true;
    }
    catch( css::lang::ServiceNotRegisteredException & e )
    {
        HandleBootstrapErrors(
            Desktop::BE_UNO_SERVICE_CONFIG_MISSING, e.Message );
    }
    catch( const css::configuration::MissingBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::MISSING_BOOTSTRAP_FILE,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_USER_INSTALL, aMsg );
    }
    catch( const css::configuration::InvalidBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( const css::configuration::InstallationIncompleteException& )
    {
        OUString aVersionFileURL;
        OUString aMsg;
        utl::Bootstrap::PathStatus aPathStatus = utl::Bootstrap::locateVersionFile( aVersionFileURL );
        if ( aPathStatus == utl::Bootstrap::PATH_EXISTS )
            aMsg = CreateErrorMsgString( utl::Bootstrap::MISSING_VERSION_FILE_ENTRY, aVersionFileURL );
        else
            aMsg = CreateErrorMsgString( utl::Bootstrap::MISSING_VERSION_FILE, aVersionFileURL );

        HandleBootstrapPathErrors( ::utl::Bootstrap::MISSING_USER_INSTALL, aMsg );
    }
    catch ( const css::configuration::backend::BackendAccessException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( const css::configuration::backend::BackendSetupException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( const css::configuration::CannotLoadConfigurationException& )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                OUString() ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( const css::uno::Exception& )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                OUString() ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    return false;
}

void Desktop::FlushConfiguration()
{
    css::uno::Reference< css::util::XFlushable >(
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext()),
        css::uno::UNO_QUERY_THROW)->flush();
}

bool Desktop::InitializeQuickstartMode( const Reference< XComponentContext >& rxContext )
{
    try
    {
        // the shutdown icon sits in the systray and allows the user to keep
        // the office instance running for quicker restart
        // this will only be activated if --quickstart was specified on cmdline

        bool bQuickstart = shouldLaunchQuickstart();

        // Try to instantiate quickstart service. This service is not mandatory, so
        // do nothing if service is not available

        // #i105753# the following if was invented for performance
        // unfortunately this broke the Mac behavior which is to always run
        // in quickstart mode since Mac applications do not usually quit
        // when the last document closes.
        // Note that this claim that on OS X we "always run in quickstart mode"
        // has nothing to do with (quick) *starting* (i.e. starting automatically
        // when the user logs in), though, but with not quitting when no documents
        // are open.
        #ifndef MACOSX
        if ( bQuickstart )
        #endif
        {
            css::office::Quickstart::createStart(rxContext, bQuickstart);
        }
        return true;
    }
    catch( const css::uno::Exception& )
    {
        return false;
    }
}

void Desktop::OverrideSystemSettings( AllSettings& rSettings )
{
    if ( !SvtTabAppearanceCfg::IsInitialized () )
        return;

    StyleSettings hStyleSettings   = rSettings.GetStyleSettings();
    MouseSettings hMouseSettings = rSettings.GetMouseSettings();

    DragFullOptions nDragFullOptions = hStyleSettings.GetDragFullOptions();

    SvtTabAppearanceCfg aAppearanceCfg;
    DragMode nDragMode = aAppearanceCfg.GetDragMode();
    switch ( nDragMode )
    {
    case DragMode::FullWindow:
        nDragFullOptions |= DragFullOptions::All;
        break;
    case DragMode::Frame:
        nDragFullOptions &= ~DragFullOptions::All;
        break;
    case DragMode::SystemDep:
    default:
        break;
    }

    MouseFollowFlags nFollow = hMouseSettings.GetFollow();
    hMouseSettings.SetFollow( aAppearanceCfg.IsMenuMouseFollow() ? (nFollow|MouseFollowFlags::Menu) : (nFollow&~MouseFollowFlags::Menu));
    rSettings.SetMouseSettings(hMouseSettings);

    SvtMenuOptions aMenuOpt;
    hStyleSettings.SetUseImagesInMenus(aMenuOpt.GetMenuIconsState());
    hStyleSettings.SetContextMenuShortcuts(aMenuOpt.GetContextMenuShortcuts());
    hStyleSettings.SetDragFullOptions( nDragFullOptions );
    rSettings.SetStyleSettings ( hStyleSettings );
}


class ExitTimer : public Timer
{
  public:
    ExitTimer()
    {
        SetTimeout(500);
        Start();
    }
    virtual void Invoke() override
    {
        exit(42);
    }
};

IMPL_LINK_NOARG(Desktop, OpenClients_Impl, void*, void)
{
    try {
        // #i114963#
        // Enable IPC thread before OpenClients
        //
        // This is because it is possible for another client to connect during the OpenClients() call.
        // This can happen on Windows when document is printed (not opened) and another client wants to print (when printing multiple documents).
        // If the IPC thread is enabled after OpenClients, then the client will not be processed because the application will exit after printing. i.e RequestHandler::AreRequestsPending() will always return false
        //
        // ALSO:
        //
        // Multiple clients may request simultaneous connections.
        // When this server closes down it attempts to recreate the pipe (in RequestHandler::Disable()).
        // It's possible that the client has a pending connection request.
        // When the IPC thread is not running, this connection locks (because maPipe.accept()) is never called
        RequestHandler::SetReady(true);
        OpenClients();

        CloseSplashScreen();
        CheckFirstRun( );
#ifdef _WIN32
        // Registers a COM class factory of the service manager with the windows operating system.
        Reference< XMultiServiceFactory > xSMgr=  comphelper::getProcessServiceFactory();
        xSMgr->createInstance("com.sun.star.bridge.OleApplicationRegistration");
        xSMgr->createInstance("com.sun.star.comp.ole.EmbedServer");
#endif
        const char *pExitPostStartup = getenv ("OOO_EXIT_POST_STARTUP");
        if (pExitPostStartup && *pExitPostStartup)
            new ExitTimer();
    } catch (const css::uno::Exception &e) {
        Application::Abort( "UNO exception during client open: " + e.Message );
    }
}

void Desktop::OpenClients()
{

    // check if a document has been recovered - if there is one of if a document was loaded by cmdline, no default document
    // should be created
    bool bRecovery = false;

    const CommandLineArgs& rArgs = GetCommandLineArgs();

    if (!rArgs.IsQuickstart())
    {
        bool bShowHelp = false;
        OUStringBuffer aHelpURLBuffer;
        if (rArgs.IsHelpWriter()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://swriter/start");
        } else if (rArgs.IsHelpCalc()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://scalc/start");
        } else if (rArgs.IsHelpDraw()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://sdraw/start");
        } else if (rArgs.IsHelpImpress()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://simpress/start");
        } else if (rArgs.IsHelpBase()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://sdatabase/start");
        } else if (rArgs.IsHelpBasic()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://sbasic/start");
        } else if (rArgs.IsHelpMath()) {
            bShowHelp = true;
            aHelpURLBuffer.append("vnd.sun.star.help://smath/start");
        }
        if (bShowHelp) {
            aHelpURLBuffer.append("?Language=");
            aHelpURLBuffer.append(utl::ConfigManager::getLocale());
#if defined UNX
            aHelpURLBuffer.append("&System=UNX");
#elif defined WNT
            aHelpURLBuffer.append("&System=WIN");
#endif
            Application::GetHelp()->Start(
                aHelpURLBuffer.makeStringAndClear(), nullptr);
            return;
        }
    }

    // Disable AutoSave feature in case "--norestore" or a similar command line switch is set on the command line.
    // The reason behind: AutoSave/EmergencySave/AutoRecovery share the same data.
    // But the require that all documents, which are saved as backup should exists inside
    // memory. May be this mechanism will be inconsistent if the configuration exists...
    // but no document inside memory corresponds to this data.
    // Further it's not acceptable to recover such documents without any UI. It can
    // need some time, where the user won't see any results and wait for finishing the office startup...
    bool bAllowRecoveryAndSessionManagement = ( !rArgs.IsNoRestore() ) && ( !rArgs.IsHeadless()  );

#if !defined ANDROID
    // Enter safe mode if requested
    if (Application::IsSafeModeEnabled()) {
        handleSafeMode();
    }
#endif

#if HAVE_FEATURE_BREAKPAD
    if (crashReportInfoExists())
        handleCrashReport();
#endif

    if ( ! bAllowRecoveryAndSessionManagement )
    {
        try
        {
            Reference< XDispatch > xRecovery = css::frame::theAutoRecovery::get( ::comphelper::getProcessComponentContext() );
            Reference< css::util::XURLTransformer > xParser = css::util::URLTransformer::create( ::comphelper::getProcessComponentContext() );

            css::util::URL aCmd;
            aCmd.Complete = "vnd.sun.star.autorecovery:/disableRecovery";
            xParser->parseStrict(aCmd);

            xRecovery->dispatch(aCmd, css::uno::Sequence< css::beans::PropertyValue >());
        }
        catch(const css::uno::Exception& e)
        {
            SAL_WARN( "desktop.app", "Could not disable AutoRecovery." << e.Message);
        }
    }
    else
    {
        bool bCrashed            = false;
        bool bExistsRecoveryData = false;
        bool bExistsSessionData  = false;
        bool const bDisableRecovery = getenv("OOO_DISABLE_RECOVERY") != nullptr;

        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);

        if ( !bDisableRecovery &&
            (
                ( bExistsRecoveryData ) || // => crash with files    => recovery
                ( bCrashed            )    // => crash without files => error report
            )
           )
        {
            try
            {
                bRecovery = impl_callRecoveryUI(
                    false          , // false => force recovery instead of emergency save
                    bExistsRecoveryData);
            }
            catch(const css::uno::Exception& e)
            {
                SAL_WARN( "desktop.app", "Error during recovery" << e.Message);
            }
        }
        else if (bExistsRecoveryData && bDisableRecovery && !rArgs.HasModuleParam())
            // prevent new Writer doc
            bRecovery = true;

        Reference< XSessionManagerListener2 > xSessionListener;
        try
        {
            // specifies whether the UI-interaction on Session shutdown is allowed
            bool bUIOnSessionShutdownAllowed = officecfg::Office::Recovery::SessionShutdown::DocumentStoreUIEnabled::get();
            xSessionListener = SessionListener::createWithOnQuitFlag(
                    ::comphelper::getProcessComponentContext(), bUIOnSessionShutdownAllowed);
        }
        catch(const css::uno::Exception& e)
        {
            SAL_WARN( "desktop.app", "Registration of session listener failed" << e.Message);
        }

        if ( !bExistsRecoveryData && xSessionListener.is() )
        {
            // session management
            try
            {
                xSessionListener->doRestore();
            }
            catch(const css::uno::Exception& e)
            {
                SAL_WARN( "desktop.app", "Error in session management" << e.Message);
            }
        }
    }
#if HAVE_FEATURE_BREAKPAD
    CrashReporter::writeCommonInfo();
    // write this information here to avoid depending on vcl in the crash reporter lib
    CrashReporter::AddKeyValue("Language", Application::GetSettings().GetLanguageTag().getBcp47());
#endif

    RequestHandler::EnableRequests();

    ProcessDocumentsRequest aRequest(rArgs.getCwdUrl());
    aRequest.aOpenList = rArgs.GetOpenList();
    aRequest.aViewList = rArgs.GetViewList();
    aRequest.aStartList = rArgs.GetStartList();
    aRequest.aPrintList = rArgs.GetPrintList();
    aRequest.aPrintToList = rArgs.GetPrintToList();
    aRequest.aPrinterName = rArgs.GetPrinterName();
    aRequest.aForceOpenList = rArgs.GetForceOpenList();
    aRequest.aForceNewList = rArgs.GetForceNewList();
    aRequest.aConversionList = rArgs.GetConversionList();
    aRequest.aConversionParams = rArgs.GetConversionParams();
    aRequest.aConversionOut = rArgs.GetConversionOut();
    aRequest.aInFilter = rArgs.GetInFilter();
    aRequest.bTextCat = rArgs.IsTextCat();
    aRequest.bScriptCat = rArgs.IsScriptCat();

    if ( !aRequest.aOpenList.empty() ||
         !aRequest.aViewList.empty() ||
         !aRequest.aStartList.empty() ||
         !aRequest.aPrintList.empty() ||
         !aRequest.aForceOpenList.empty() ||
         !aRequest.aForceNewList.empty() ||
         ( !aRequest.aPrintToList.empty() && !aRequest.aPrinterName.isEmpty() ) ||
         !aRequest.aConversionList.empty() )
    {
        if ( rArgs.HasModuleParam() )
        {
            SvtModuleOptions    aOpt;

            // Support command line parameters to start a module (as preselection)
            if ( rArgs.IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::EFactory::WRITER );
            else if ( rArgs.IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::EFactory::CALC );
            else if ( rArgs.IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::EFactory::IMPRESS );
            else if ( rArgs.IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::EFactory::DRAW );
        }

        // check for printing disabled
        if( ( !(aRequest.aPrintList.empty() && aRequest.aPrintToList.empty()) )
            && Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        {
            aRequest.aPrintList.clear();
            aRequest.aPrintToList.clear();
            ScopedVclPtrInstance< MessageDialog > aBox(nullptr, DpResId(STR_ERR_PRINTDISABLED));
            aBox->Execute();
        }

        // Process request
        if ( RequestHandler::ExecuteCmdLineRequests(aRequest, false) )
        {
            // Don't do anything if we have successfully called terminate at desktop:
            return;
        }
    }

    // no default document if a document was loaded by recovery or by command line or if soffice is used as server
    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference< XElementAccess > xList( xDesktop->getFrames(), UNO_QUERY_THROW );
    if ( xList->hasElements() )
        return;

    if ( rArgs.IsQuickstart() || rArgs.IsInvisible() || Application::AnyInput( VclInputFlags::APPEVENT ) )
        // soffice was started as tray icon ...
        return;

    if ( bRecovery )
    {
        ShowBackingComponent(nullptr);
    }
    else
    {
        OpenDefault();
    }
}

void Desktop::OpenDefault()
{
    OUString        aName;
    SvtModuleOptions    aOpt;

    const CommandLineArgs& rArgs = GetCommandLineArgs();
    if ( rArgs.IsNoDefault() ) return;
    if ( rArgs.HasModuleParam() )
    {
        // Support new command line parameters to start a module
        if ( rArgs.IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::WRITER );
        else if ( rArgs.IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::CALC );
        else if ( rArgs.IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::IMPRESS );
        else if ( rArgs.IsBase() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::DATABASE );
        else if ( rArgs.IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::DRAW );
        else if ( rArgs.IsMath() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::MATH );
        else if ( rArgs.IsGlobal() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::WRITERGLOBAL );
        else if ( rArgs.IsWeb() && aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::WRITERWEB );
    }

    if ( aName.isEmpty() )
    {
        // Old way to create a default document
        if ( aOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::WRITER );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::CALC );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::IMPRESS );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::DATABASE );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory::DRAW );
        else
            return;
    }

    ProcessDocumentsRequest aRequest(rArgs.getCwdUrl());
    aRequest.aOpenList.push_back(aName);
    RequestHandler::ExecuteCmdLineRequests(aRequest, false);
}


OUString GetURL_Impl(
    const OUString& rName, boost::optional< OUString > const & cwdUrl )
{
    // if rName is a vnd.sun.star.script URL do not attempt to parse it
    // as INetURLObj does not handle handle there URLs
    if (rName.startsWith("vnd.sun.star.script"))
    {
        return rName;
    }

    // don't touch file urls, those should already be in internal form
    // they won't get better here (#112849#)
    if (comphelper::isFileUrl(rName))
    {
        return rName;
    }

    if ( rName.startsWith("service:"))
    {
        return rName;
    }

    // Add path separator to these directory and make given URL (rName) absolute by using of current working directory
    // Attention: "setFinalSlash()" is necessary for calling "smartRel2Abs()"!!!
    // Otherwhise last part will be ignored and wrong result will be returned!!!
    // "smartRel2Abs()" interpret given URL as file not as path. So he truncate last element to get the base path ...
    // But if we add a separator - he doesn't do it anymore.
    INetURLObject aObj;
    if (cwdUrl) {
        aObj.SetURL(*cwdUrl);
        aObj.setFinalSlash();
    }

    // Use the provided parameters for smartRel2Abs to support the usage of '%' in system paths.
    // Otherwise this char won't get encoded and we are not able to load such files later,
    bool bWasAbsolute;
    INetURLObject aURL     = aObj.smartRel2Abs( rName, bWasAbsolute, false, INetURLObject::EncodeMechanism::WasEncoded,
                                                RTL_TEXTENCODING_UTF8, true );
    OUString      aFileURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    ::osl::FileStatus aStatus( osl_FileStatus_Mask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    return aFileURL;
}

void Desktop::HandleAppEvent( const ApplicationEvent& rAppEvent )
{
    switch ( rAppEvent.GetEvent() )
    {
    case ApplicationEvent::Type::Accept:
        // every time an accept parameter is used we create an acceptor
        // with the corresponding accept-string
        createAcceptor(rAppEvent.GetStringData());
        break;
    case ApplicationEvent::Type::Appear:
        if ( !GetCommandLineArgs().IsInvisible() )
        {
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            // find active task - the active task is always a visible task
            Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );
            Reference< css::frame::XFrame > xTask = xDesktop->getActiveFrame();
            if ( !xTask.is() )
            {
                // get any task if there is no active one
                Reference< css::container::XIndexAccess > xList( xDesktop->getFrames(), css::uno::UNO_QUERY );
                if ( xList->getCount() > 0 )
                    xList->getByIndex(0) >>= xTask;
            }

            if ( xTask.is() )
            {
                Reference< css::awt::XTopWindow > xTop( xTask->getContainerWindow(), UNO_QUERY );
                xTop->toFront();
            }
            else
            {
                // no visible task that could be activated found
                Reference< css::awt::XWindow > xContainerWindow;
                Reference< XFrame > xBackingFrame = xDesktop->findFrame( "_blank", 0);
                if (xBackingFrame.is())
                    xContainerWindow = xBackingFrame->getContainerWindow();
                if (xContainerWindow.is())
                {
                    Reference< XController > xStartModule = StartModule::createWithParentWindow(xContext, xContainerWindow);
                    Reference< css::awt::XWindow > xBackingWin(xStartModule, UNO_QUERY);
                    // Attention: You MUST(!) call setComponent() before you call attachFrame().
                    // Because the backing component set the property "IsBackingMode" of the frame
                    // to true inside attachFrame(). But setComponent() reset this state every time ...
                    xBackingFrame->setComponent(xBackingWin, xStartModule);
                    xStartModule->attachFrame(xBackingFrame);
                    xContainerWindow->setVisible(true);

                    VclPtr<vcl::Window> pCompWindow = VCLUnoHelper::GetWindow(xBackingFrame->getComponentWindow());
                    if (pCompWindow)
                        pCompWindow->Update();
                }
            }
        }
        break;
    case ApplicationEvent::Type::Help:
        displayCmdlineHelp(rAppEvent.GetStringData());
        break;
    case ApplicationEvent::Type::Version:
        displayVersion();
        break;
    case ApplicationEvent::Type::Open:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest docsRequest(rCmdLine.getCwdUrl());
                std::vector<OUString> const & data(rAppEvent.GetStringsData());
                docsRequest.aOpenList.insert(
                    docsRequest.aOpenList.end(), data.begin(), data.end());
                RequestHandler::ExecuteCmdLineRequests(docsRequest, false);
            }
        }
        break;
    case ApplicationEvent::Type::OpenHelpUrl:
        // start help for a specific URL
        Application::GetHelp()->Start(rAppEvent.GetStringData(), nullptr);
        break;
    case ApplicationEvent::Type::Print:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest docsRequest(rCmdLine.getCwdUrl());
                std::vector<OUString> const & data(rAppEvent.GetStringsData());
                docsRequest.aPrintList.insert(
                    docsRequest.aPrintList.end(), data.begin(), data.end());
                RequestHandler::ExecuteCmdLineRequests(docsRequest, false);
            }
        }
        break;
    case ApplicationEvent::Type::PrivateDoShutdown:
        {
            Desktop* pD = dynamic_cast<Desktop*>(GetpApp());
            OSL_ENSURE( pD, "no desktop ?!?" );
            if( pD )
                pD->doShutdown();
        }
        break;
    case ApplicationEvent::Type::QuickStart:
        if ( !GetCommandLineArgs().IsInvisible()  )
        {
            // If the office has been started the second time its command line arguments are sent through a pipe
            // connection to the first office. We want to reuse the quickstart option for the first office.
            // NOTICE: The quickstart service must be initialized inside the "main thread", so we use the
            // application events to do this (they are executed inside main thread)!!!
            // Don't start quickstart service if the user specified "--invisible" on the command line!
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            css::office::Quickstart::createStart(xContext, true/*Quickstart*/);
        }
        break;
    case ApplicationEvent::Type::ShowDialog:
        // ignore all errors here. It's clicking a menu entry only ...
        // The user will try it again, in case nothing happens .-)
        try
        {
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );

            Reference< css::util::XURLTransformer > xParser = css::util::URLTransformer::create(xContext);
            css::util::URL aCommand;
            if( rAppEvent.GetStringData() == "PREFERENCES" )
                aCommand.Complete = ".uno:OptionsTreeDialog";
            else if( rAppEvent.GetStringData() == "ABOUT" )
                aCommand.Complete = ".uno:About";
            if( !aCommand.Complete.isEmpty() )
            {
                xParser->parseStrict(aCommand);

                css::uno::Reference< css::frame::XDispatch > xDispatch = xDesktop->queryDispatch(aCommand, OUString(), 0);
                if (xDispatch.is())
                    xDispatch->dispatch(aCommand, css::uno::Sequence< css::beans::PropertyValue >());
            }
        }
        catch(const css::uno::Exception&)
        {}
        break;
    case ApplicationEvent::Type::Unaccept:
        // try to remove corresponding acceptor
        destroyAcceptor(rAppEvent.GetStringData());
        break;
    default:
        SAL_WARN( "desktop.app", "this cannot happen");
        break;
    }
}

void Desktop::OpenSplashScreen()
{
    const CommandLineArgs &rCmdLine = GetCommandLineArgs();
    // Show intro only if this is normal start (e.g. no server, no quickstart, no printing )
    if ( !rCmdLine.IsInvisible() &&
         !rCmdLine.IsHeadless() &&
         !rCmdLine.IsQuickstart() &&
         !rCmdLine.IsMinimized() &&
         !rCmdLine.IsNoLogo() &&
         !rCmdLine.IsTerminateAfterInit() &&
         rCmdLine.GetPrintList().empty() &&
         rCmdLine.GetPrintToList().empty() &&
         rCmdLine.GetConversionList().empty() )
    {
        // Determine application name from command line parameters
        OUString aAppName;
        if ( rCmdLine.IsWriter() )
            aAppName = "writer";
        else if ( rCmdLine.IsCalc() )
            aAppName = "calc";
        else if ( rCmdLine.IsDraw() )
            aAppName = "draw";
        else if ( rCmdLine.IsImpress() )
            aAppName = "impress";
        else if ( rCmdLine.IsBase() )
            aAppName = "base";
        else if ( rCmdLine.IsGlobal() )
            aAppName = "global";
        else if ( rCmdLine.IsMath() )
            aAppName = "math";
        else if ( rCmdLine.IsWeb() )
            aAppName = "web";

        // Which splash to use
        OUString aSplashService( "com.sun.star.office.SplashScreen" );
        if ( rCmdLine.HasSplashPipe() )
            aSplashService = "com.sun.star.office.PipeSplashScreen";

        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= true; // bVisible
        aSeq[1] <<= aAppName;
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        m_rSplashScreen.set(
            xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aSplashService, aSeq, xContext),
            UNO_QUERY);

        if(m_rSplashScreen.is())
                m_rSplashScreen->start("SplashScreen", 100);
    }

}

void Desktop::SetSplashScreenProgress(sal_Int32 iProgress)
{
    if(m_rSplashScreen.is())
    {
        m_rSplashScreen->setValue(iProgress);
    }
}

void Desktop::SetSplashScreenText( const OUString& rText )
{
    if( m_rSplashScreen.is() )
    {
        m_rSplashScreen->setText( rText );
    }
}

void Desktop::CloseSplashScreen()
{
    if(m_rSplashScreen.is())
    {
        m_rSplashScreen->end();
        m_rSplashScreen = nullptr;
    }
}


IMPL_STATIC_LINK_NOARG(Desktop, AsyncInitFirstRun, Timer *, void)
{
    // does initializations which are necessary for the first run of the office
    try
    {
        Reference< XJobExecutor > xExecutor = theJobExecutor::get( ::comphelper::getProcessComponentContext() );
        xExecutor->trigger( "onFirstRunInitialization" );
    }
    catch(const css::uno::Exception&)
    {
        SAL_WARN( "desktop.app", "Desktop::DoFirstRunInitializations: caught an exception while trigger job executor ..." );
    }
}

void Desktop::ShowBackingComponent(Desktop * progress)
{
    if (GetCommandLineArgs().IsNoDefault())
    {
        return;
    }
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create(xContext);
    if (progress != nullptr)
    {
        progress->SetSplashScreenProgress(60);
    }
    Reference< XFrame > xBackingFrame = xDesktop->findFrame( "_blank", 0);
    Reference< css::awt::XWindow > xContainerWindow;

    if (xBackingFrame.is())
        xContainerWindow = xBackingFrame->getContainerWindow();
    if (xContainerWindow.is())
    {
        // set the WB_EXT_DOCUMENT style. Normally, this is done by the TaskCreator service when a "_blank"
        // frame/window is created. Since we do not use the TaskCreator here, we need to mimic its behavior,
        // otherwise documents loaded into this frame will later on miss functionality depending on the style.
        VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        SAL_WARN_IF( !pContainerWindow, "desktop.app", "Desktop::Main: no implementation access to the frame's container window!" );
        pContainerWindow->SetExtendedStyle( pContainerWindow->GetExtendedStyle() | WB_EXT_DOCUMENT );
        if (progress != nullptr)
        {
            progress->SetSplashScreenProgress(75);
        }

        Reference< XController > xStartModule = StartModule::createWithParentWindow( xContext, xContainerWindow);
        // Attention: You MUST(!) call setComponent() before you call attachFrame().
        // Because the backing component set the property "IsBackingMode" of the frame
        // to true inside attachFrame(). But setComponent() reset this state everytimes ...
        xBackingFrame->setComponent(Reference< XWindow >(xStartModule, UNO_QUERY), xStartModule);
        if (progress != nullptr)
        {
            progress->SetSplashScreenProgress(100);
        }
        xStartModule->attachFrame(xBackingFrame);
        if (progress != nullptr)
        {
            progress->CloseSplashScreen();
        }
        xContainerWindow->setVisible(true);
    }
}


void Desktop::CheckFirstRun( )
{
    if (officecfg::Office::Common::Misc::FirstRun::get())
    {
        // use VCL timer, which won't trigger during shutdown if the
        // application exits before timeout
        m_firstRunTimer.Start();

#ifdef _WIN32
        // Check if Quickstarter should be started (on Windows only)
        TCHAR szValue[8192];
        DWORD nValueSize = sizeof(szValue);
        HKEY hKey;
        if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  "Software\\LibreOffice", &hKey ) )
        {
            if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("RunQuickstartAtFirstStart"), nullptr, nullptr, reinterpret_cast<LPBYTE>(szValue), &nValueSize ) )
            {
                css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                css::office::Quickstart::createAutoStart(xContext, true/*Quickstart*/, true/*bAutostart*/);
                RegCloseKey( hKey );
            }
        }
#endif

        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::FirstRun::set(false, batch);
        batch->commit();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
