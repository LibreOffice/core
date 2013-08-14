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

#include "sal/config.h"

#include <iostream>

#include "app.hxx"
#include "desktop.hrc"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"
#include "dispatchwatcher.hxx"
#include "configinit.hxx"
#include "lockfile.hxx"
#include "userinstall.hxx"
#include "desktopcontext.hxx"
#include "exithelper.h"
#include "migration.hxx"

#include <svtools/javacontext.hxx>
#include <com/sun/star/frame/AutoRecovery.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/SessionListener.hpp>
#include <com/sun/star/frame/XSessionManagerListener.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/StartModule.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include "com/sun/star/util/URLTransformer.hpp"
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/task/JobExecutor.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>
#include <com/sun/star/ui/UIElementFactoryManager.hpp>
#include <com/sun/star/ui/WindowStateConfiguration.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/frame/ToolbarControllerFactory.hpp>
#include <com/sun/star/frame/PopupMenuControllerFactory.hpp>
#include <com/sun/star/office/Quickstart.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/uri.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/menuoptions.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>

#include <svtools/fontsubstconfig.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/graphicfilter.hxx>

#include "langselect.hxx"

#include <config_telepathy.h>

#if ENABLE_TELEPATHY
#include <tubes/manager.hxx>
#endif

#if defined MACOSX
#include <errno.h>
#include <sys/wait.h>
#endif

#ifdef WNT
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

#if defined WNT
#include <process.h>
#define GETPID _getpid
#else
#include <unistd.h>
#define GETPID getpid
#endif

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

ResMgr* desktop::Desktop::pResMgr = 0;

namespace desktop
{

static oslSignalHandler pSignalHandler = 0;
static sal_Bool _bCrashReporterEnabled = sal_True;

namespace {

void removeTree(OUString const & url) {
    osl::Directory dir(url);
    osl::FileBase::RC rc = dir.open();
    switch (rc) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return; //TODO: SAL_WARN if recursive
    default:
        SAL_WARN("desktop.app", "cannot open directory " << dir.getURL() << ": " << +rc);
        return;
    }
    for (;;) {
        osl::DirectoryItem i;
        rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            SAL_WARN( "desktop.app", "cannot iterate directory " << dir.getURL() << ": " << +rc);
            break;
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        rc = i.getFileStatus(stat);
        if (rc != osl::FileBase::E_None) {
            SAL_WARN( "desktop.app", "cannot stat in directory " << dir.getURL() << ": " << +rc);
            continue;
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            removeTree(stat.getFileURL());
        } else {
            rc = osl::File::remove(stat.getFileURL());
            SAL_WARN_IF(
                rc != osl::FileBase::E_None, "desktop.app",
                "cannot remove file " << stat.getFileURL() << ": " << +rc);
        }
    }
    if (dir.isOpen()) {
        rc = dir.close();
        SAL_WARN_IF(
            rc != osl::FileBase::E_None, "desktop.app",
            "cannot close directory " << dir.getURL() << ": " << +rc);
    }
    rc = osl::Directory::remove(url);
    SAL_WARN_IF(
        rc != osl::FileBase::E_None, "desktop.app",
        "cannot remove directory " << url << ": " << +rc);
}

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
// single UserInstallation, as it is called before OfficeIPCThread is set up.
// Therefore, any errors here only lead to SAL_WARNs.
//
// At least in theory, this function could be removed again once no
// UserInstallation can be poisoned by old junk any more.
bool cleanExtensionCache() {
    OUString buildId(
        "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(buildId); //TODO: detect failure
    OUString extDir(
        "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap")
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
            if (rc != osl::FileBase::E_None) {
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
    removeTree(extDir);
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

}

// ----------------------------------------------------------------------------

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
            OUString aUILocaleString = LanguageSelection::getLanguageString();
            LanguageTag aLanguageTag( aUILocaleString);
            //! ResMgr may modify the Locale for fallback!
            Desktop::pResMgr = ResMgr::SearchCreateResMgr( "dkt", aLanguageTag);
            AllSettings as = GetSettings();
            as.SetUILanguageTag(aLanguageTag);
            SetSettings(as);
        }
    }

    return Desktop::pResMgr;
}

namespace {

// ----------------------------------------------------------------------------
// Get a message string securely. There is a fallback string if the resource
// is not available.

OUString GetMsgString(
    sal_uInt16 nId, const OUString& aFallbackMsg,
    bool bAlwaysUseFallbackMsg = false )
{
    if ( !bAlwaysUseFallbackMsg )
    {
        ResMgr* resMgr = Desktop::GetDesktopResManager();
        if ( resMgr )
            return OUString( String( ResId( nId, *resMgr )));
    }
    return aFallbackMsg;
}

OUString MakeStartupErrorMessage(
    OUString const & aErrorMessage, bool bAlwaysUseFallbackMsg = false )
{
    OUStringBuffer    aDiagnosticMessage( 100 );

    aDiagnosticMessage.append(
        GetMsgString(
            STR_BOOTSTRAP_ERR_CANNOT_START, "The program cannot be started.",
            bAlwaysUseFallbackMsg ) );

    aDiagnosticMessage.appendAscii( "\n" );

    aDiagnosticMessage.append( aErrorMessage );

    return aDiagnosticMessage.makeStringAndClear();
}

OUString MakeStartupConfigAccessErrorMessage( OUString const & aInternalErrMsg )
{
    OUStringBuffer aDiagnosticMessage( 200 );

    ResMgr* pResMgr = Desktop::GetDesktopResManager();
    if ( pResMgr )
        aDiagnosticMessage.append( OUString(String(ResId(STR_BOOTSTRAP_ERR_CFG_DATAACCESS, *pResMgr ))) );
    else
        aDiagnosticMessage.appendAscii( "The program cannot be started." );

    if ( !aInternalErrMsg.isEmpty() )
    {
        aDiagnosticMessage.appendAscii( "\n\n" );
        if ( pResMgr )
            aDiagnosticMessage.append( OUString(String(ResId(STR_INTERNAL_ERRMSG, *pResMgr ))) );
        else
            aDiagnosticMessage.appendAscii( "The following internal error has occurred:\n\n" );
        aDiagnosticMessage.append( aInternalErrMsg );
    }

    return aDiagnosticMessage.makeStringAndClear();
}

//=============================================================================
// shows a simple error box with the given message ... but exits from these process !
// Fatal errors cant be solved by the process ... nor any recovery can help.
// Mostly the installation was damaged and must be repaired manually .. or by calling
// setup again.
// On the other side we must make sure that no further actions will be possible within
// the current office process ! No pipe requests, no menu/toolbar/shortuct actions
// are allowed. Otherwise we will force a "crash inside a crash".
// Thats why we have to use a special native message box here which does not use yield :-)
//=============================================================================
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
    sTitle.appendAscii (" - Fatal Error");

    Application::ShowNativeErrorBox (sTitle.makeStringAndClear (), sMessage);
    _exit(EXITHELPER_FATAL_ERROR);
}

static bool ShouldSuppressUI(const CommandLineArgs& rCmdLine)
{
    return  rCmdLine.IsInvisible() ||
            rCmdLine.IsHeadless() ||
            rCmdLine.IsQuickstart();
}

struct theCommandLineArgs : public rtl::Static< CommandLineArgs, theCommandLineArgs > {};

}

CommandLineArgs& Desktop::GetCommandLineArgs()
{
    return theCommandLineArgs::get();
}

namespace
{
    struct BrandName
        : public rtl::Static< String, BrandName > {};
    struct Version
        : public rtl::Static< String, Version > {};
    struct AboutBoxVersion
        : public rtl::Static< String, AboutBoxVersion > {};
    struct AboutBoxVersionSuffix
        : public rtl::Static< String, AboutBoxVersionSuffix > {};
    struct OOOVendor
        : public rtl::Static< String, OOOVendor > {};
    struct Extension
        : public rtl::Static< String, Extension > {};
    struct XMLFileFormatName
        : public rtl::Static< String, XMLFileFormatName > {};
    struct XMLFileFormatVersion
        : public rtl::Static< String, XMLFileFormatVersion > {};
    struct WriterCompatibilityVersionOOo11
        : public rtl::Static< String, WriterCompatibilityVersionOOo11 > {};
}

OUString ReplaceStringHookProc( const OUString& rStr )
{
    OUString sRet(rStr);

    if (sRet.indexOf("%PRODUCT") != -1 || sRet.indexOf("%ABOUTBOX") != -1)
    {
        OUString sBrandName = BrandName::get();
        OUString sVersion = Version::get();
        OUString sBuildId = utl::Bootstrap::getBuildIdData("development");
        OUString sAboutBoxVersion = AboutBoxVersion::get();
        OUString sAboutBoxVersionSuffix = AboutBoxVersionSuffix::get();
        OUString sExtension = Extension::get();
        OUString sXMLFileFormatName = XMLFileFormatName::get();
        OUString sXMLFileFormatVersion = XMLFileFormatVersion::get();

        if ( sBrandName.isEmpty() )
        {
            sBrandName = utl::ConfigManager::getProductName();
            sXMLFileFormatName = utl::ConfigManager::getProductXmlFileFormat();
            sXMLFileFormatVersion =
                utl::ConfigManager::getProductXmlFileFormatVersion();
            sVersion = utl::ConfigManager::getProductVersion();
            sAboutBoxVersion = utl::ConfigManager::getAboutBoxProductVersion();
            sAboutBoxVersionSuffix = utl::ConfigManager::getAboutBoxProductVersionSuffix();
            if ( sExtension.isEmpty() )
            {
                sExtension = utl::ConfigManager::getProductExtension();
            }
        }

        sRet = sRet.replaceAll( "%PRODUCTNAME", sBrandName );
        sRet = sRet.replaceAll( "%PRODUCTVERSION", sVersion );
        sRet = sRet.replaceAll( "%BUILDID", sBuildId );
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSIONSUFFIX", sAboutBoxVersionSuffix );
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSION", sAboutBoxVersion );
        sRet = sRet.replaceAll( "%PRODUCTEXTENSION", sExtension );
        sRet = sRet.replaceAll( "%PRODUCTXMLFILEFORMATNAME", sXMLFileFormatName );
        sRet = sRet.replaceAll( "%PRODUCTXMLFILEFORMATVERSION", sXMLFileFormatVersion );
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

    if ( sRet.indexOf( "%WRITERCOMPATIBILITYVERSIONOOO11" ) != -1 )
    {
        OUString sWriterCompatibilityVersionOOo11 = WriterCompatibilityVersionOOo11::get();
        if ( sWriterCompatibilityVersionOOo11.isEmpty() )
        {
            sWriterCompatibilityVersionOOo11 =
                utl::ConfigManager::getWriterCompatibilityVersionOOo_1_1();
        }

        sRet = sRet.replaceAll( "%WRITERCOMPATIBILITYVERSIONOOO11",
                                        sWriterCompatibilityVersionOOo11 );
    }

    return sRet;
}

Desktop::Desktop()
: m_bCleanedExtensionCache( false )
, m_bServicesRegistered( false )
, m_aBootstrapError( BE_OK )
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::Desktop::Desktop" );
}

Desktop::~Desktop()
{
#if ENABLE_TELEPATHY
    TeleManager::finalize();
#endif
}

void Desktop::Init()
{
    SAL_INFO( "desktop.app",  "desktop (cd100003) ::Desktop::Init" );
    SetBootstrapStatus(BS_OK);

    m_bCleanedExtensionCache = cleanExtensionCache();

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

    if ( m_aBootstrapError == BE_OK )
    {
        // prepare language
        if ( !LanguageSelection::prepareLanguage() )
        {
            if ( LanguageSelection::getStatus() == LanguageSelection::LS_STATUS_CANNOT_DETERMINE_LANGUAGE )
                SetBootstrapError( BE_LANGUAGE_MISSING, OUString() );
            else
                SetBootstrapError( BE_OFFICECONFIG_BROKEN, OUString() );
        }
    }

    if ( 1 )
    {
        const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();

        // start ipc thread only for non-remote offices
        SAL_INFO( "desktop.app",  "desktop (cd100003) ::OfficeIPCThread::EnableOfficeIPCThread" );
        OfficeIPCThread::Status aStatus = OfficeIPCThread::EnableOfficeIPCThread();
        if ( aStatus == OfficeIPCThread::IPC_STATUS_PIPE_ERROR )
        {
#if HAVE_FEATURE_MACOSX_SANDBOX
            // In a sandboxed LO, on 10.8.2 at least, creating the
            // Unix domain socket fails. Ignore that as hopefully
            // people running a sandboxed LO won't attempt starting it
            // from the command-line or otherwise in tricky ways, so
            // the normal OS X mechanism that prevents multiple
            // instances of an app from being started should work
            // fine. I hope.
#elif defined ANDROID
            // Ignore crack pipe errors on Android, too
#else
            // Keep using this oddly named BE_PATHINFO_MISSING value
            // for pipe-related errors on other platforms. Of course
            // this crack with two (if not more) levels of our own
            // error codes hiding the actual system error code is
            // broken, but that is done all over the code, let's leave
            // re-enginering that to another year.
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
#endif
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_2ND_OFFICE )
        {
            // 2nd office startup should terminate after sending cmdlineargs through pipe
            SetBootstrapStatus(BS_TERMINATE);
        }
        else if ( !rCmdLineArgs.GetUnknown().isEmpty()
                  || rCmdLineArgs.IsHelp() || rCmdLineArgs.IsVersion() )
        {
            // disable IPC thread in an instance that is just showing a help message
            OfficeIPCThread::DisableOfficeIPCThread();
        }
        pSignalHandler = osl_addSignalHandler(SalMainPipeExchangeSignal_impl, NULL);
    }
}

void Desktop::InitFinished()
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::Desktop::InitFinished" );

    CloseSplashScreen();
}

void Desktop::DeInit()
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::Desktop::DeInit" );

    try {
        // instead of removing of the configManager just let it commit all the changes
        SAL_INFO( "desktop.app", "<- store config items" );
        utl::ConfigManager::storeConfigItems();
        FlushConfiguration();
        SAL_INFO( "desktop.app", "<- store config items" );

        // close splashscreen if it's still open
        CloseSplashScreen();
        Reference< XComponent >(
            comphelper::getProcessComponentContext(), UNO_QUERY_THROW )->
            dispose();
        // nobody should get a destroyed service factory...
        ::comphelper::setProcessServiceFactory( NULL );

        // clear lockfile
        m_xLockfile.reset();

        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            osl_removeSignalHandler( pSignalHandler );
    } catch (const RuntimeException&) {
        // someone threw an exception during shutdown
        // this will leave some garbage behind..
    }

    SAL_INFO( "desktop.app", "FINISHED WITH Destop::DeInit" );
}

sal_Bool Desktop::QueryExit()
{
    try
    {
        SAL_INFO( "desktop.app", "<- store config items" );
        utl::ConfigManager::storeConfigItems();
        SAL_INFO( "desktop.app", "<- store config items" );
    }
    catch ( const RuntimeException& )
    {
    }

    const sal_Char SUSPEND_QUICKSTARTVETO[] = "SuspendQuickstartVeto";

    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference< XPropertySet > xPropertySet(xDesktop, UNO_QUERY_THROW);
    xPropertySet->setPropertyValue( OUString(SUSPEND_QUICKSTARTVETO ), Any((sal_Bool)sal_True) );

    sal_Bool bExit = xDesktop->terminate();

    if ( !bExit )
    {
        xPropertySet->setPropertyValue( OUString(SUSPEND_QUICKSTARTVETO ), Any((sal_Bool)sal_False) );
    }
    else
    {
        FlushConfiguration();
        try
        {
            // it is no problem to call DisableOfficeIPCThread() more than once
            // it also looks to be threadsafe
            OfficeIPCThread::DisableOfficeIPCThread();
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

        ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
        aBootstrapFailedBox.SetText( aProductKey );
        aBootstrapFailedBox.Execute();
    }
}

// Create a error message depending on bootstrap failure code and an optional file url
OUString    Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const OUString& aFileURL )
{
    OUString        aMsg;
    OUString        aFilePath;
    sal_Bool        bFileInfo = sal_True;

    switch ( nFailureCode )
    {
        /// the shared installation directory could not be located
        case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_PATH_INVALID,
                        OUString( "The installation path is not available." ) );
            bFileInfo = sal_False;
        }
        break;

        /// the bootstrap INI file could not be found or read
        case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( "The configuration file \"$1\" is missing." ) );
        }
        break;

        /// the bootstrap INI is missing a required entry
        /// the bootstrap INI contains invalid data
         case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
         case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_CORRUPT,
                        OUString( "The configuration file \"$1\" is corrupt." ) );
        }
        break;

        /// the version locator INI file could not be found or read
        case ::utl::Bootstrap::MISSING_VERSION_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( "The configuration file \"$1\" is missing." ) );
        }
        break;

        /// the version locator INI has no entry for this version
         case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SUPPORT,
                        OUString( "The main configuration file \"$1\" does not support the current version." ) );
        }
        break;

        /// the user installation directory does not exist
           case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_DIR_MISSING,
                        OUString( "The configuration directory \"$1\" is missing." ) );
        }
        break;

        /// some bootstrap data was invalid in unexpected ways
        case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
                        OUString( "An internal failure occurred." ) );
            bFileInfo = sal_False;
        }
        break;

        case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
        {
            // This needs to be improved, see #i67575#:
            aMsg = OUString(
                "Invalid version file entry"  );
            bFileInfo = sal_False;
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
        String aMsgString( aMsg );

        osl::File::getSystemPathFromFileURL( aFileURL, aFilePath );

        aMsgString.SearchAndReplaceAscii( "$1", aFilePath );
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
        // and cause the process to crash (e.g., a call to GetMsgString may
        // crash when somewhere deep within that call Any::operator <= is used
        // with a PropertyValue, and no binary UNO type description for
        // PropertyValue is available).  To give the user a hint even if
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

        OUString aErrorMsg;

        if ( aBootstrapError == BE_UNO_SERVICEMANAGER )
            aErrorMsg = "The service manager is not available.";
        else
            aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_CFG_SERVICE,
                            OUString( "The configuration service is not available." ) );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.appendAscii( "\n" );
        if ( !aErrorMessage.isEmpty() )
        {
            aDiagnosticMessage.appendAscii( "(\"" );
            aDiagnosticMessage.append( aErrorMessage );
            aDiagnosticMessage.appendAscii( "\")\n" );
        }

        // Due to the fact the we haven't a backup applicat.rdb file anymore it is not possible to
        // repair the installation with the setup executable besides the office executable. Now
        // we have to ask the user to start the setup on CD/installation directory manually!!
        OUString aStartSetupManually( GetMsgString(
            STR_ASK_START_SETUP_MANUALLY,
            OUString( "Start setup application to repair the installation from CD, or the folder containing the installation packages." ),
            aBootstrapError == BE_UNO_SERVICEMANAGER ) );

        aDiagnosticMessage.append( aStartSetupManually );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear(),
            aBootstrapError == BE_UNO_SERVICEMANAGER );

        FatalError( aMessage);
    }
    else if ( aBootstrapError == BE_OFFICECONFIG_BROKEN )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = GetMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
            OUString( "A general error occurred while accessing your central configuration." ) );
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if ( aBootstrapError == BE_USERINSTALL_FAILED )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_USERINSTALL_FAILED,
            OUString( "User installation could not be completed" ) );
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if ( aBootstrapError == BE_LANGUAGE_MISSING )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = GetMsgString(
            //@@@ FIXME: should use an own resource string => #i36213#
            STR_BOOTSTRAP_ERR_LANGUAGE_MISSING,
            OUString( "Language could not be determined." ) );
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
            aErrorMsg = GetMsgString(
                STR_BOOSTRAP_ERR_NOTENOUGHDISKSPACE,
                OUString( "User installation could not be completed due to insufficient free disk space." ) );
        else
            aErrorMsg = GetMsgString(
                STR_BOOSTRAP_ERR_NOACCESSRIGHTS,
                OUString( "User installation could not be processed due to missing access rights." ) );

        osl::File::getSystemPathFromFileURL( aUserInstallationURL, aUserInstallationPath );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.append( aUserInstallationPath );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }

    return;
}


void Desktop::retrieveCrashReporterState()
{
    _bCrashReporterEnabled
        = officecfg::Office::Recovery::CrashReporter::Enabled::get();
}

sal_Bool Desktop::isUIOnSessionShutdownAllowed()
{
    return officecfg::Office::Recovery::SessionShutdown::DocumentStoreUIEnabled
        ::get();
}

//-----------------------------------------------
/** @short  check if crash reporter feature is enabled or
            disabled.
*/
sal_Bool Desktop::isCrashReporterEnabled()
{
    return _bCrashReporterEnabled;
}

//-----------------------------------------------
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
            Because the user may be logged out last time from it's
            unix session...
*/
void impl_checkRecoveryState(sal_Bool& bCrashed           ,
                             sal_Bool& bRecoveryDataExists,
                             sal_Bool& bSessionDataExists )
{
    bCrashed = officecfg::Office::Recovery::RecoveryInfo::Crashed::get();
    bool elements = officecfg::Office::Recovery::RecoveryList::get()->
        hasElements();
    bool session
        = officecfg::Office::Recovery::RecoveryInfo::SessionData::get();
    bRecoveryDataExists = elements && !session;
    bSessionDataExists = elements && session;
}

//-----------------------------------------------
/*  @short  start the recovery wizard.

    @param  bEmergencySave
            differs between EMERGENCY_SAVE and RECOVERY
*/
sal_Bool impl_callRecoveryUI(sal_Bool bEmergencySave     ,
                             sal_Bool bCrashed           ,
                             sal_Bool bExistsRecoveryData)
{
    static OUString SERVICENAME_RECOVERYUI("com.sun.star.comp.svx.RecoveryUI");
    static OUString COMMAND_EMERGENCYSAVE("vnd.sun.star.autorecovery:/doEmergencySave");
    static OUString COMMAND_RECOVERY("vnd.sun.star.autorecovery:/doAutoRecovery");
    static OUString COMMAND_CRASHREPORT("vnd.sun.star.autorecovery:/doCrashReport");

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< css::frame::XSynchronousDispatch > xRecoveryUI(
        xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_RECOVERYUI, xContext),
        css::uno::UNO_QUERY_THROW);

    Reference< css::util::XURLTransformer > xURLParser =
        css::util::URLTransformer::create(::comphelper::getProcessComponentContext());

    css::util::URL aURL;
    if (bEmergencySave)
        aURL.Complete = COMMAND_EMERGENCYSAVE;
    else if (bExistsRecoveryData)
        aURL.Complete = COMMAND_RECOVERY;
    else if (bCrashed && Desktop::isCrashReporterEnabled() )
        aURL.Complete = COMMAND_CRASHREPORT;
    else
        return false;

    xURLParser->parseStrict(aURL);

    css::uno::Any aRet = xRecoveryUI->dispatchWithReturnValue(aURL, css::uno::Sequence< css::beans::PropertyValue >());
    sal_Bool bRet = sal_False;
    aRet >>= bRet;
    return !bEmergencySave || bRet;
}

/*
 * Save all open documents so they will be reopened
 * the next time the application ist started
 *
 * returns sal_True if at least one document could be saved...
 *
 */

sal_Bool Desktop::SaveTasks()
{
    return impl_callRecoveryUI(
        sal_True , // sal_True => force emergency save
        sal_False, // 2. and 3. param not used if 1. = true!
        sal_False);
}

namespace {

void restartOnMac(bool passArguments) {
#if defined MACOSX
    OfficeIPCThread::DisableOfficeIPCThread();
#if HAVE_FEATURE_MACOSX_SANDBOX
    (void) passArguments; // avoid warnings
    ResMgr *resMgr = Desktop::GetDesktopResManager();
    OUString aMessage = OUString( String( ResId( STR_LO_MUST_BE_RESTARTED, *resMgr )));

    ErrorBox aRestartBox( NULL, WB_OK, aMessage );
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
            OSL_VERIFY(osl_getCommandArg(i, &arg.pData) == osl_Process_E_None);
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
    argPtrs.push_back(0);
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

}

sal_uInt16 Desktop::Exception(sal_uInt16 nError)
{
    // protect against recursive calls
    static sal_Bool bInException = sal_False;

    sal_uInt16 nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    Application::SetDefDialogParent( NULL );

    if ( bInException )
    {
        String aDoubleExceptionString;
        Application::Abort( aDoubleExceptionString );
    }

    bInException = sal_True;
    const CommandLineArgs& rArgs = GetCommandLineArgs();

    // save all modified documents ... if it's allowed doing so.
    sal_Bool bRestart                           = sal_False;
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !rArgs.IsNoRestore()                    ) && // some use cases of office must work without recovery
                                                    ( !rArgs.IsHeadless()                     ) &&
                                                    (( nError & EXC_MAJORTYPE ) != EXC_DISPLAY ) && // recovery cant work without UI ... but UI layer seams to be the reason for this crash
                                                    ( Application::IsInExecute()               )    // crashes during startup and shutdown should be ignored (they indicates a corrupt installation ...)
                                                  );
    if ( bAllowRecoveryAndSessionManagement )
        bRestart = SaveTasks();

    FlushConfiguration();

    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
        {
            String aResExceptionString;
            Application::Abort( aResExceptionString );
            break;
        }

        case EXC_SYSOBJNOTCREATED:
        {
            String aSysResExceptionString;
            Application::Abort( aSysResExceptionString );
            break;
        }

        default:
        {
            m_xLockfile.reset();

            if( bRestart )
            {
                OfficeIPCThread::DisableOfficeIPCThread();
                if( pSignalHandler )
                    osl_removeSignalHandler( pSignalHandler );

                restartOnMac(false);
                if ( m_rSplashScreen.is() )
                    m_rSplashScreen->reset();

                _exit( EXITHELPER_CRASH_WITH_RESTART );
            }
            else
            {
                Application::Abort( String() );
            }

            break;
        }
    }

    OSL_ASSERT(false); // unreachable
    return 0;
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
}

namespace {
    void SetDocumentExtendedStyle( const Reference< ::com::sun::star::awt::XWindow > &xContainerWindow )
    {
        // set the WB_EXT_DOCUMENT style. Normally, this is done by the TaskCreator service when a "_blank"
        // frame/window is created. Since we do not use the TaskCreator here, we need to mimic its behavior,
        // otherwise documents loaded into this frame will later on miss functionality depending on the style.
        Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        SAL_WARN_IF( !pContainerWindow, "desktop.app", "Desktop::Main: no implementation access to the frame's container window!" );
        if (!pContainerWindow) {
            fprintf (stderr, "Error: It very much looks as if you have used 'linkoo' (or bin/ooinstall -l)\n"
                     "but have then forgotten to source 'ooenv' into your shell before running !\n"
                     "to save a crash, we will exit now with an error - please '. ./ooenv' first.\n");
            exit (1);
        }
        pContainerWindow->SetExtendedStyle( pContainerWindow->GetExtendedStyle() | WB_EXT_DOCUMENT );
    }
}

struct ExecuteGlobals
{
    Reference < css::document::XEventListener > xGlobalBroadcaster;
    sal_Bool bRestartRequested;
    sal_Bool bUseSystemFileDialog;
    std::auto_ptr<SvtLanguageOptions> pLanguageOptions;
    std::auto_ptr<SvtPathOptions> pPathOptions;

    ExecuteGlobals()
    : bRestartRequested( sal_False )
    , bUseSystemFileDialog( sal_True )
    {}
};

static ExecuteGlobals* pExecGlobals = NULL;

int Desktop::Main()
{
    pExecGlobals = new ExecuteGlobals();

    SAL_INFO( "desktop.app", "desktop (cd100003) ::Desktop::Main" );

    // Remember current context object
    com::sun::star::uno::ContextLayer layer(
        com::sun::star::uno::getCurrentContext() );

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
    com::sun::star::uno::setCurrentContext(
        new DesktopContext( com::sun::star::uno::getCurrentContext() ) );

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
    // setup configuration error handling
    ConfigurationErrorHandler aConfigErrHandler;
    if (!ShouldSuppressUI(rCmdLineArgs))
        aConfigErrHandler.activate();

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    // Startup screen
    SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main { OpenSplashScreen" );
    OpenSplashScreen();
    SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main } OpenSplashScreen" );

    SetSplashScreenProgress(10);

    UserInstall::UserInstallStatus inst_fin = UserInstall::finalize();
    if (inst_fin != UserInstall::Ok && inst_fin != UserInstall::Created)
    {
        SAL_WARN( "desktop.app", "userinstall failed");
        if ( inst_fin == UserInstall::E_NoDiskSpace )
            HandleBootstrapErrors(
                BE_USERINSTALL_NOTENOUGHDISKSPACE, OUString() );
        else if ( inst_fin == UserInstall::E_NoWriteAccess )
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

        // check user installation directory for lockfile so we can be sure
        // there is no other instance using our data files from a remote host
        SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main -> Lockfile" );
        m_xLockfile.reset(new Lockfile);

#if HAVE_FEATURE_DESKTOP
        if ( !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoLockcheck() && !m_xLockfile->check( Lockfile_execWarning ))
        {
            // Lockfile exists, and user clicked 'no'
            return EXIT_FAILURE;
        }
        SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main <- Lockfile" );

        // check if accessibility is enabled but not working and allow to quit
        SAL_INFO( "desktop.app", "{ GetEnableATToolSupport" );
        if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
        {
            bool bQuitApp;

            if( !InitAccessBridge( true, bQuitApp ) )
                if( bQuitApp )
                    return EXIT_FAILURE;
        }
        SAL_INFO( "desktop.app", "} GetEnableATToolSupport" );
#endif

        // terminate if requested...
        if( rCmdLineArgs.IsTerminateAfterInit() )
            return EXIT_SUCCESS;

        //  Read the common configuration items for optimization purpose
        if ( !InitializeConfiguration() )
            return EXIT_FAILURE;

        SetSplashScreenProgress(30);

        // set static variable to enabled/disable crash reporter
        retrieveCrashReporterState();
        if ( !isCrashReporterEnabled() )
        {
            osl_setErrorReporting( sal_False );
            // disable stack trace feature
        }

        // create title string
        LanguageTag aLocale( LANGUAGE_SYSTEM);
        ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( "ofa", aLocale );
        String aTitle = pLabelResMgr ? String( ResId( RID_APPTITLE, *pLabelResMgr ) ) : String();
        delete pLabelResMgr;

#ifdef DBG_UTIL
        //include buildid in non product builds
        OUString aDefault("development");
        aTitle += OUString(" [");
        String aVerId( utl::Bootstrap::getBuildIdData(aDefault));
        aTitle += aVerId;
        aTitle += ']';
#endif

        SetDisplayName( aTitle );
        SetSplashScreenProgress(35);
        SAL_INFO( "desktop.app", "{ create SvtPathOptions and SvtLanguageOptions" );
        pExecGlobals->pPathOptions.reset( new SvtPathOptions);
        SetSplashScreenProgress(40);
        SAL_INFO( "desktop.app", "} create SvtPathOptions and SvtLanguageOptions" );

        xDesktop = css::frame::Desktop::create( xContext );

        // create service for loadin SFX (still needed in startup)
        pExecGlobals->xGlobalBroadcaster = Reference < css::document::XEventListener >
            ( css::frame::GlobalEventBroadcaster::create(xContext), UNO_QUERY_THROW );

        /* ensure existance of a default window that messages can be dispatched to
           This is for the benefit of testtool which uses PostUserEvent extensively
           and else can deadlock while creating this window from another tread while
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

        if (inst_fin == UserInstall::Created)
        {
            Migration::migrateSettingsIfNecessary();
        }
#endif

        // keep a language options instance...
        pExecGlobals->pLanguageOptions.reset( new SvtLanguageOptions(sal_True));

        css::document::EventObject aEvent;
        aEvent.EventName = OUString("OnStartApp");
        pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);

        SetSplashScreenProgress(50);

        // Backing Component
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        SAL_INFO( "desktop.app", "{ impl_checkRecoveryState" );
        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);
        SAL_INFO( "desktop.app", "} impl_checkRecoveryState" );

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
                    OString pid( OString::valueOf( static_cast<sal_Int32>( GETPID() ) ) );
                    sal_uInt64 written = 0;
                    if ( pidfile.write(pid.getStr(), pid.getLength(), written) != osl::File::E_None )
                    {
                        SAL_WARN("desktop.app", "cannot write pidfile " << pidfile.getURL());
                    }
                    pidfile.close();
                }
                else
                {
                    SAL_WARN("desktop.app", "cannot open pidfile " << pidfile.getURL() << osl::FileBase::RC(rc));
                }
            }
            else
            {
                SAL_WARN("desktop.app", "cannot get pidfile URL from path" << pidfileName);
            }
        }

        if ( rCmdLineArgs.IsHeadless() )
        {
            // Ensure that we use not the system file dialogs as
            // headless mode relies on Application::EnableHeadlessMode()
            // which does only work for VCL dialogs!!
            SvtMiscOptions aMiscOptions;
            pExecGlobals->bUseSystemFileDialog = aMiscOptions.UseSystemFileDialog();
            aMiscOptions.SetUseSystemFileDialog( sal_False );
        }

        pExecGlobals->bRestartRequested = xRestartManager->isRestartRequested(
            true);
        if ( !pExecGlobals->bRestartRequested )
        {
            if ((!rCmdLineArgs.WantsToLoadDocument() && !rCmdLineArgs.IsInvisible() && !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsQuickstart()) &&
                (SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE)) &&
                (!bExistsRecoveryData                                                  ) &&
                (!bExistsSessionData                                                   ) &&
                (!Application::AnyInput( VCL_INPUT_APPEVENT )                          ))
            {
                 SAL_INFO( "desktop.app", "{ create BackingComponent" );
                 ShowBackingComponent(this);
                 SAL_INFO( "desktop.app", "} create BackingComponent" );
            }
        }
    }
    catch ( const com::sun::star::lang::WrappedTargetException& wte )
    {
        com::sun::star::uno::Exception te;
        wte.TargetException >>= te;
        FatalError( MakeStartupConfigAccessErrorMessage(wte.Message + te.Message) );
    }
    catch ( const com::sun::star::uno::Exception& e )
    {
        FatalError( MakeStartupErrorMessage(e.Message) );
    }
    SetSplashScreenProgress(55);

    SvtFontSubstConfig().Apply();

    SvtTabAppearanceCfg aAppearanceCfg;
    aAppearanceCfg.SetInitialized();
    aAppearanceCfg.SetApplicationDefaults( this );
    SvtAccessibilityOptions aOptions;
    aOptions.SetVCLSettings();
    SetSplashScreenProgress(60);

#if ENABLE_TELEPATHY
    bool bListen = rCmdLineArgs.IsInvisible();
    TeleManager::init( bListen );
#endif

    if ( !pExecGlobals->bRestartRequested )
    {
        Application::SetFilterHdl( LINK( this, Desktop, ImplInitFilterHdl ) );
        sal_Bool bTerminateRequested = sal_False;

        // Preload function depends on an initialized sfx application!
        SetSplashScreenProgress(75);

        // use system window dialogs
        Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

        SetSplashScreenProgress(80);

        if ( !bTerminateRequested && !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoQuickstart() )
            InitializeQuickstartMode( xContext );

        SAL_INFO( "desktop.app", "desktop (cd100003) createInstance com.sun.star.frame.Desktop" );
        try
        {
            if ( xDesktop.is() )
                xDesktop->addTerminateListener( new OfficeIPCThreadController );
            SetSplashScreenProgress(100);
        }
        catch ( const com::sun::star::uno::Exception& e )
        {
            FatalError( MakeStartupErrorMessage(e.Message) );
        }

        // Release solar mutex just before we wait for our client to connect
        int nAcquireCount = Application::ReleaseSolarMutex();

        // Post user event to startup first application component window
        // We have to send this OpenClients message short before execute() to
        // minimize the risk that this message overtakes type detection contruction!!
        Application::PostUserEvent( LINK( this, Desktop, OpenClients_Impl ) );

        // Post event to enable acceptors
        Application::PostUserEvent( LINK( this, Desktop, EnableAcceptors_Impl) );

        // The configuration error handler currently is only for startup
        aConfigErrHandler.deactivate();

       // Acquire solar mutex just before we enter our message loop
        if ( nAcquireCount )
            Application::AcquireSolarMutex( nAcquireCount );

        // call Application::Execute to process messages in vcl message loop
        SAL_INFO( "desktop.app", "PERFORMANCE - enter Application::Execute()" );

        try
        {
#ifdef SOLAR_JAVA
            // The JavaContext contains an interaction handler which is used when
            // the creation of a Java Virtual Machine fails
            com::sun::star::uno::ContextLayer layer2(
                new svt::JavaContext( com::sun::star::uno::getCurrentContext() ) );
#endif
            // check whether the shutdown is caused by restart just before entering the Execute
            pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested ||
                xRestartManager->isRestartRequested(true);

            if ( !pExecGlobals->bRestartRequested )
            {
                // if this run of the office is triggered by restart, some additional actions should be done
                DoRestartActionsIfNecessary( !rCmdLineArgs.IsInvisible() && !rCmdLineArgs.IsNoQuickstart() );

                Execute();
            }
        }
        catch(const com::sun::star::document::CorruptedFilterConfigurationException& exFilterCfg)
        {
            OfficeIPCThread::SetDowning();
            FatalError( MakeStartupErrorMessage(exFilterCfg.Message) );
        }
        catch(const com::sun::star::configuration::CorruptedConfigurationException& exAnyCfg)
        {
            OfficeIPCThread::SetDowning();
            FatalError( MakeStartupErrorMessage(exAnyCfg.Message) );
        }
        catch( const ::com::sun::star::uno::Exception& exUNO)
        {
            OfficeIPCThread::SetDowning();
            FatalError( exUNO.Message);
        }
        catch( const std::exception& exSTD)
        {
            OfficeIPCThread::SetDowning();
            FatalError( OUString::createFromAscii( exSTD.what()));
        }
        catch( ...)
        {
            OfficeIPCThread::SetDowning();
            FatalError( OUString(RTL_CONSTASCII_USTRINGPARAM( "Caught Unknown Exception: Aborting!")));
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

    pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested ||
        OfficeRestartManager::get(comphelper::getProcessComponentContext())->
        isRestartRequested(true);
    if ( pExecGlobals->bRestartRequested )
        SetRestartState();

    if (pExecGlobals->xGlobalBroadcaster.is())
    {
        css::document::EventObject aEvent;
        aEvent.EventName = OUString("OnCloseApp");
        pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);
    }

    delete pResMgr, pResMgr = NULL;
    // Restore old value
    const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();
    if ( rCmdLineArgs.IsHeadless() )
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
    FlushConfiguration();
    // The acceptors in the AcceptorMap must be released (in DeregisterServices)
    // with the solar mutex unlocked, to avoid deadlock:
    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    DeregisterServices();
    Application::AcquireSolarMutex(nAcquireCount);
    // be sure that path/language options gets destroyed before
    // UCB is deinitialized
    SAL_INFO( "desktop.app", "-> dispose path/language options" );
    pExecGlobals->pLanguageOptions.reset( 0 );
    pExecGlobals->pPathOptions.reset( 0 );
    SAL_INFO( "desktop.app", "<- dispose path/language options" );

    sal_Bool bRR = pExecGlobals->bRestartRequested;
    delete pExecGlobals, pExecGlobals = NULL;

    SAL_INFO( "desktop.app", "FINISHED WITH Destop::Main" );
    if ( bRR )
    {
        restartOnMac(true);
        if ( m_rSplashScreen.is() )
            m_rSplashScreen->reset();

        return EXITHELPER_NORMAL_RESTART;
    }
    return EXIT_SUCCESS;
}

IMPL_LINK( Desktop, ImplInitFilterHdl, ConvertData*, pData )
{
    return GraphicFilter::GetGraphicFilter().GetFilterCallback().Call( pData );
}

bool Desktop::InitializeConfiguration()
{
    SAL_INFO( "desktop.app",  "desktop (jb99855) ::InitConfiguration" );
    try
    {
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext() );
        return true;
    }
    catch( ::com::sun::star::lang::ServiceNotRegisteredException & e )
    {
        this->HandleBootstrapErrors(
            Desktop::BE_UNO_SERVICE_CONFIG_MISSING, e.Message );
    }
    catch( const ::com::sun::star::configuration::MissingBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::MISSING_BOOTSTRAP_FILE,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_USER_INSTALL, aMsg );
    }
    catch( const ::com::sun::star::configuration::InvalidBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( const ::com::sun::star::configuration::InstallationIncompleteException& )
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
    catch ( const com::sun::star::configuration::backend::BackendAccessException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( const com::sun::star::configuration::backend::BackendSetupException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( const ::com::sun::star::configuration::CannotLoadConfigurationException& )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                OUString() ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( const ::com::sun::star::uno::Exception& )
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

sal_Bool Desktop::shouldLaunchQuickstart()
{
    sal_Bool bQuickstart = GetCommandLineArgs().IsQuickstart();
    if (!bQuickstart)
    {
        const SfxPoolItem* pItem=0;
        SfxItemSet aQLSet(SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER);
        SFX_APP()->GetOptions(aQLSet);
        SfxItemState eState = aQLSet.GetItemState(SID_ATTR_QUICKLAUNCHER, sal_False, &pItem);
        if (SFX_ITEM_SET == eState)
            bQuickstart = ((SfxBoolItem*)pItem)->GetValue();
    }
    return bQuickstart;
}


sal_Bool Desktop::InitializeQuickstartMode( const Reference< XComponentContext >& rxContext )
{
    try
    {
        // the shutdown icon sits in the systray and allows the user to keep
        // the office instance running for quicker restart
        // this will only be activated if --quickstart was specified on cmdline
        SAL_INFO( "desktop.app", "desktop (cd100003) createInstance com.sun.star.office.Quickstart" );

        sal_Bool bQuickstart = shouldLaunchQuickstart();

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
        return sal_True;
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        return sal_False;
    }
}

void Desktop::SystemSettingsChanging( AllSettings& rSettings, Window* )
{
    if ( !SvtTabAppearanceCfg::IsInitialized () )
        return;

#   define DRAGFULL_OPTION_ALL \
         ( DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE  \
         | DRAGFULL_OPTION_OBJECTMOVE  | DRAGFULL_OPTION_OBJECTSIZE \
         | DRAGFULL_OPTION_DOCKING     | DRAGFULL_OPTION_SPLIT      \
         | DRAGFULL_OPTION_SCROLL )
#   define DRAGFULL_OPTION_NONE ((sal_uInt32)~DRAGFULL_OPTION_ALL)

    StyleSettings hStyleSettings   = rSettings.GetStyleSettings();
    MouseSettings hMouseSettings = rSettings.GetMouseSettings();

    sal_uInt32 nDragFullOptions = hStyleSettings.GetDragFullOptions();

    SvtTabAppearanceCfg aAppearanceCfg;
    sal_uInt16 nGet = aAppearanceCfg.GetDragMode();
    switch ( nGet )
    {
    case DragFullWindow:
        nDragFullOptions |= DRAGFULL_OPTION_ALL;
        break;
    case DragFrame:
        nDragFullOptions &= DRAGFULL_OPTION_NONE;
        break;
    case DragSystemDep:
    default:
        break;
    }

    sal_uInt32 nFollow = hMouseSettings.GetFollow();
    hMouseSettings.SetFollow( aAppearanceCfg.IsMenuMouseFollow() ? (nFollow|MOUSE_FOLLOW_MENU) : (nFollow&~MOUSE_FOLLOW_MENU));
    rSettings.SetMouseSettings(hMouseSettings);

    SvtMenuOptions aMenuOpt;
    hStyleSettings.SetUseImagesInMenus(aMenuOpt.GetMenuIconsState());
    hStyleSettings.SetDragFullOptions( nDragFullOptions );
    rSettings.SetStyleSettings ( hStyleSettings );
}

// ========================================================================
IMPL_LINK_NOARG(Desktop, AsyncInitFirstRun)
{
    DoFirstRunInitializations();
    return 0L;
}

// ========================================================================

class ExitTimer : public Timer
{
  public:
    ExitTimer()
    {
        SetTimeout(500);
        Start();
    }
    virtual void Timeout()
    {
        exit(42);
    }
};

IMPL_LINK_NOARG(Desktop, OpenClients_Impl)
{
    SAL_INFO( "desktop.app", "PERFORMANCE - DesktopOpenClients_Impl()" );

    try {
        OpenClients();

        OfficeIPCThread::SetReady();

        CloseSplashScreen();
        CheckFirstRun( );
        EnableOleAutomation();

        if (getenv ("OOO_EXIT_POST_STARTUP"))
            new ExitTimer();
    } catch (const ::com::sun::star::uno::Exception &e) {
        String a( "UNO exception during client open:\n"  );
        Application::Abort( a + e.Message );
    }
    return 0;
}

// enable acceptos
IMPL_LINK_NOARG(Desktop, EnableAcceptors_Impl)
{
    enableAcceptors();
    return 0;
}


// Registers a COM class factory of the service manager with the windows operating system.
void Desktop::EnableOleAutomation()
{
      SAL_INFO( "desktop.app",  "desktop (jl97489) ::Desktop::EnableOleAutomation" );
#ifdef WNT
    Reference< XMultiServiceFactory > xSMgr=  comphelper::getProcessServiceFactory();
    xSMgr->createInstance("com.sun.star.bridge.OleApplicationRegistration");
    xSMgr->createInstance("com.sun.star.comp.ole.EmbedServer");
#endif
}

void Desktop::PreloadModuleData( const CommandLineArgs& rArgs )
{
    Sequence < com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = OUString("Hidden");
    args[0].Value <<= sal_True;
    Reference < XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );

    if ( rArgs.IsWriter() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xDesktop->loadComponentFromURL( OUString("private:factory/swriter"),
                OUString("_blank"), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }
    if ( rArgs.IsCalc() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xDesktop->loadComponentFromURL( OUString("private:factory/scalc"),
                OUString("_blank"), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }
    if ( rArgs.IsDraw() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xDesktop->loadComponentFromURL( OUString("private:factory/sdraw"),
                OUString("_blank"), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }
    if ( rArgs.IsImpress() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xDesktop->loadComponentFromURL( OUString("private:factory/simpress"),
                OUString("_blank"), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }
}

void Desktop::PreloadConfigurationData()
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XNameAccess > xNameAccess = css::frame::UICommandDescription::create(xContext);

    OUString aWriterDoc( "com.sun.star.text.TextDocument" );
    OUString aCalcDoc( "com.sun.star.sheet.SpreadsheetDocument" );
    OUString aDrawDoc( "com.sun.star.drawing.DrawingDocument" );
    OUString aImpressDoc( "com.sun.star.presentation.PresentationDocument" );

    // preload commands configuration
    Any a;
    Reference< XNameAccess > xCmdAccess;

    try
    {
        a = xNameAccess->getByName( aWriterDoc );
        a >>= xCmdAccess;
        if ( xCmdAccess.is() )
        {
            xCmdAccess->getByName(".uno:BasicShapes");
            xCmdAccess->getByName(".uno:EditGlossary");
        }
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    try
    {
        a = xNameAccess->getByName( aCalcDoc );
        a >>= xCmdAccess;
        if ( xCmdAccess.is() )
            xCmdAccess->getByName(".uno:InsertObjectStarMath");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    try
    {
        // draw and impress share the same configuration file (DrawImpressCommands.xcu)
        a = xNameAccess->getByName( aDrawDoc );
        a >>= xCmdAccess;
        if ( xCmdAccess.is() )
            xCmdAccess->getByName(".uno:Polygon");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    // preload window state configuration
    xNameAccess = WindowStateConfiguration::create( xContext );
    Reference< XNameAccess > xWindowAccess;
    try
    {
        a = xNameAccess->getByName( aWriterDoc );
        a >>= xWindowAccess;
        if ( xWindowAccess.is() )
            xWindowAccess->getByName("private:resource/toolbar/standardbar");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }
    try
    {
        a = xNameAccess->getByName( aCalcDoc );
        a >>= xWindowAccess;
        if ( xWindowAccess.is() )
            xWindowAccess->getByName("private:resource/toolbar/standardbar");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }
    try
    {
        a = xNameAccess->getByName( aDrawDoc );
        a >>= xWindowAccess;
        if ( xWindowAccess.is() )
            xWindowAccess->getByName("private:resource/toolbar/standardbar");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }
    try
    {
        a = xNameAccess->getByName( aImpressDoc );
        a >>= xWindowAccess;
        if ( xWindowAccess.is() )
            xWindowAccess->getByName("private:resource/toolbar/standardbar");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    // preload user interface element factories
    Sequence< Sequence< css::beans::PropertyValue > > aSeqSeqPropValue;
    Reference< XUIElementFactoryManager > xUIElementFactory = UIElementFactoryManager::create( xContext );
    try
    {
        aSeqSeqPropValue = xUIElementFactory->getRegisteredFactories();
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    // preload popup menu controller factories. As all controllers are in the same
    // configuration file they also get preloaded!

    Reference< css::frame::XUIControllerRegistration > xPopupMenuControllerFactory =
    css::frame::PopupMenuControllerFactory::create( xContext );
    try
    {
        xPopupMenuControllerFactory->hasController(
                    OUString( ".uno:CharFontName" ),
                    OUString() );
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    // preload filter configuration
    Sequence< OUString > aSeq;
    xNameAccess = Reference< XNameAccess >(
                    xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", xContext),
                    UNO_QUERY );
    if ( xNameAccess.is() )
    {
        try
        {
             aSeq = xNameAccess->getElementNames();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload type detection configuration
    xNameAccess = Reference< XNameAccess >(
                    xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", xContext),
                    UNO_QUERY );
    if ( xNameAccess.is() )
    {
        try
        {
             aSeq = xNameAccess->getElementNames();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }
}

void Desktop::OpenClients()
{

    // check if a document has been recovered - if there is one of if a document was loaded by cmdline, no default document
    // should be created
    Reference < XComponent > xFirst;
    bool bRecovery = false;

    const CommandLineArgs& rArgs = GetCommandLineArgs();

    if (!rArgs.IsQuickstart())
    {
        sal_Bool bShowHelp = sal_False;
        OUStringBuffer aHelpURLBuffer;
        if (rArgs.IsHelpWriter()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://swriter/start");
        } else if (rArgs.IsHelpCalc()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://scalc/start");
        } else if (rArgs.IsHelpDraw()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdraw/start");
        } else if (rArgs.IsHelpImpress()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://simpress/start");
        } else if (rArgs.IsHelpBase()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdatabase/start");
        } else if (rArgs.IsHelpBasic()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sbasic/start");
        } else if (rArgs.IsHelpMath()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://smath/start");
        }
        if (bShowHelp) {
            aHelpURLBuffer.appendAscii("?Language=");
            aHelpURLBuffer.append(utl::ConfigManager::getLocale());
#if defined UNX
            aHelpURLBuffer.appendAscii("&System=UNX");
#elif defined WNT
            aHelpURLBuffer.appendAscii("&System=WIN");
#endif
            Application::GetHelp()->Start(
                aHelpURLBuffer.makeStringAndClear(), NULL);
            return;
        }
    }
    else
    {
        OUString            aIniName;

        osl_getExecutableFile( &aIniName.pData );
        sal_uInt32     lastIndex = aIniName.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aIniName    = aIniName.copy( 0, lastIndex+1 );
            aIniName    += "perftune";
#if defined(WNT)
            aIniName    += ".ini";
#else
            aIniName    += "rc";
#endif
        }

        rtl::Bootstrap aPerfTuneIniFile( aIniName );

        OUString aDefault( "0" );
        OUString aPreloadData;

        aPerfTuneIniFile.getFrom( OUString( "QuickstartPreloadConfiguration" ), aPreloadData, aDefault );
        if ( aPreloadData == "1" )
        {
            if ( rArgs.IsWriter()  ||
                 rArgs.IsCalc()    ||
                 rArgs.IsDraw()    ||
                 rArgs.IsImpress()    )
            {
                PreloadModuleData( rArgs );
            }

            PreloadConfigurationData();
        }
    }

    // Disable AutoSave feature in case "--norestore" or a similar command line switch is set on the command line.
    // The reason behind: AutoSave/EmergencySave/AutoRecovery share the same data.
    // But the require that all documents, which are saved as backup should exists inside
    // memory. May be this mechanism will be inconsistent if the configuration exists ...
    // but no document inside memory corrspond to this data.
    // Furter it's not acceptable to recover such documents without any UI. It can
    // need some time, where the user wont see any results and wait for finishing the office startup ...
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !rArgs.IsNoRestore() ) &&
                                                    ( !rArgs.IsHeadless()  )
                                                  );

    if ( ! bAllowRecoveryAndSessionManagement )
    {
        try
        {
            Reference< XDispatch > xRecovery = css::frame::AutoRecovery::create( ::comphelper::getProcessComponentContext() );
            Reference< css::util::XURLTransformer > xParser = css::util::URLTransformer::create( ::comphelper::getProcessComponentContext() );

            css::util::URL aCmd;
            aCmd.Complete = OUString("vnd.sun.star.autorecovery:/disableRecovery");
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
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);

        if ( !getenv ("OOO_DISABLE_RECOVERY") &&
            (
                ( bExistsRecoveryData ) || // => crash with files    => recovery
                ( bCrashed            )    // => crash without files => error report
            )
           )
        {
            try
            {
                bRecovery = impl_callRecoveryUI(
                    sal_False          , // false => force recovery instead of emergency save
                    bCrashed           ,
                    bExistsRecoveryData);
            }
            catch(const css::uno::Exception& e)
            {
                SAL_WARN( "desktop.app", "Error during recovery" << e.Message);
            }
        }

        Reference< XSessionManagerListener2 > xSessionListener;
        try
        {
            // specifies whether the UI-interaction on Session shutdown is allowed
            sal_Bool bAllowUI = isUIOnSessionShutdownAllowed();

            xSessionListener = SessionListener::createWithOnQuitFlag(::comphelper::getProcessComponentContext(), bAllowUI);

//            css::beans::NamedValue aProperty( OUString( "AllowUserInteractionOnQuit"  ),
 //                                             css::uno::makeAny( bAllowUI ) );
  //          css::uno::Sequence< css::uno::Any > aArgs( 1 );
   //         aArgs[0] <<= aProperty;

     //       xSessionListener->initialize( aArgs );
        }
        catch(const com::sun::star::uno::Exception& e)
        {
            SAL_WARN( "desktop.app", "Registration of session listener failed" << e.Message);
        }

        if ( !bExistsRecoveryData )
        {
            // session management
            try
            {
                xSessionListener->doRestore();
            }
            catch(const com::sun::star::uno::Exception& e)
            {
                SAL_WARN( "desktop.app", "Error in session management" << e.Message);
            }
        }
    }

    OfficeIPCThread::EnableRequests();

    ProcessDocumentsRequest aRequest(rArgs.getCwdUrl());
    aRequest.pcProcessed = NULL;

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
            if ( rArgs.IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_WRITER );
            else if ( rArgs.IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_CALC );
            else if ( rArgs.IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_IMPRESS );
            else if ( rArgs.IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_DRAW );
        }

        // check for printing disabled
        if( ( !(aRequest.aPrintList.empty() && aRequest.aPrintToList.empty()) )
            && Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        {
            aRequest.aPrintList.clear();
            aRequest.aPrintToList.clear();
            ResMgr* pDtResMgr = GetDesktopResManager();
            if( pDtResMgr )
            {
                ErrorBox aBox( NULL, ResId( EBX_ERR_PRINTDISABLED, *pDtResMgr ) );
                aBox.Execute();
            }
        }

        // Process request
        if ( OfficeIPCThread::ExecuteCmdLineRequests( aRequest ) )
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

    if ( rArgs.IsQuickstart() || rArgs.IsInvisible() || Application::AnyInput( VCL_INPUT_APPEVENT ) )
        // soffice was started as tray icon ...
        return;

    if ( bRecovery )
    {
        ShowBackingComponent(0);
    }
    else
    {
        OpenDefault();
    }
}

void Desktop::OpenDefault()
{

    SAL_INFO( "desktop.app",  "desktop (cd100003) ::Desktop::OpenDefault" );

    OUString        aName;
    SvtModuleOptions    aOpt;

    const CommandLineArgs& rArgs = GetCommandLineArgs();
    if ( rArgs.IsNoDefault() ) return;
    if ( rArgs.HasModuleParam() )
    {
        // Support new command line parameters to start a module
        if ( rArgs.IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITER );
        else if ( rArgs.IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_CALC );
        else if ( rArgs.IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_IMPRESS );
        else if ( rArgs.IsBase() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DATABASE );
        else if ( rArgs.IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DRAW );
        else if ( rArgs.IsMath() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_MATH );
        else if ( rArgs.IsGlobal() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITERGLOBAL );
        else if ( rArgs.IsWeb() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITERWEB );
    }

    if ( aName.isEmpty() )
    {
        // Old way to create a default document
        if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITER );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_CALC );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_IMPRESS );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DATABASE );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DRAW );
        else
            return;
    }

    ProcessDocumentsRequest aRequest(rArgs.getCwdUrl());
    aRequest.pcProcessed = NULL;
    aRequest.aOpenList.push_back(aName);
    OfficeIPCThread::ExecuteCmdLineRequests( aRequest );
}


String GetURL_Impl(
    const String& rName, boost::optional< OUString > const & cwdUrl )
{
    // if rName is a vnd.sun.star.script URL do not attempt to parse it
    // as INetURLObj does not handle handle there URLs
    if (rName.CompareToAscii("vnd.sun.star.script" , 19) == COMPARE_EQUAL)
    {
        return rName;
    }

    // dont touch file urls, those should already be in internal form
    // they won't get better here (#112849#)
    if (rName.CompareToAscii("file:" , 5) == COMPARE_EQUAL)
    {
        return rName;
    }

    if ( rName.CompareToAscii("service:" , 8) == COMPARE_EQUAL )
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
    INetURLObject aURL     = aObj.smartRel2Abs( rName, bWasAbsolute, false, INetURLObject::WAS_ENCODED,
                                                RTL_TEXTENCODING_UTF8, true );
    String        aFileURL = aURL.GetMainURL(INetURLObject::NO_DECODE);

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
    case ApplicationEvent::TYPE_ACCEPT:
        // every time an accept parameter is used we create an acceptor
        // with the corresponding accept-string
        createAcceptor(rAppEvent.GetStringData());
        break;
    case ApplicationEvent::TYPE_APPEAR:
        if ( !GetCommandLineArgs().IsInvisible() )
        {
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            // find active task - the active task is always a visible task
            Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );
            Reference< css::frame::XFrame > xTask = xDesktop->getActiveFrame();
            if ( !xTask.is() )
            {
                // get any task if there is no active one
                Reference< css::container::XIndexAccess > xList( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
                if ( xList->getCount() > 0 )
                    xList->getByIndex(0) >>= xTask;
            }

            if ( xTask.is() )
            {
                Reference< com::sun::star::awt::XTopWindow > xTop( xTask->getContainerWindow(), UNO_QUERY );
                xTop->toFront();
            }
            else
            {
                // no visible task that could be activated found
                Reference< ::com::sun::star::awt::XWindow > xContainerWindow;
                Reference< XFrame > xBackingFrame = xDesktop->findFrame(OUString( "_blank" ), 0);
                if (xBackingFrame.is())
                    xContainerWindow = xBackingFrame->getContainerWindow();
                if (xContainerWindow.is())
                {
                    Reference< XController > xStartModule = StartModule::createWithParentWindow(xContext, xContainerWindow);
                    Reference< ::com::sun::star::awt::XWindow > xBackingWin(xStartModule, UNO_QUERY);
                    // Attention: You MUST(!) call setComponent() before you call attachFrame().
                    // Because the backing component set the property "IsBackingMode" of the frame
                    // to true inside attachFrame(). But setComponent() reset this state every time ...
                    xBackingFrame->setComponent(xBackingWin, xStartModule);
                    xStartModule->attachFrame(xBackingFrame);
                    xContainerWindow->setVisible(sal_True);

                    Window* pCompWindow = VCLUnoHelper::GetWindow(xBackingFrame->getComponentWindow());
                    if (pCompWindow)
                        pCompWindow->Update();
                }
            }
        }
        break;
    case ApplicationEvent::TYPE_HELP:
        displayCmdlineHelp(rAppEvent.GetStringData());
        break;
    case ApplicationEvent::TYPE_VERSION:
        displayVersion();
        break;
    case ApplicationEvent::TYPE_OPEN:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest(
                    rCmdLine.getCwdUrl());
                std::vector<OUString> const & data(rAppEvent.GetStringsData());
                pDocsRequest->aOpenList.insert(
                    pDocsRequest->aOpenList.end(), data.begin(), data.end());
                pDocsRequest->pcProcessed = NULL;

                OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
                delete pDocsRequest;
            }
        }
        break;
    case ApplicationEvent::TYPE_OPENHELPURL:
        // start help for a specific URL
        Application::GetHelp()->Start(rAppEvent.GetStringData(), NULL);
        break;
    case ApplicationEvent::TYPE_PRINT:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest(
                    rCmdLine.getCwdUrl());
                std::vector<OUString> const & data(rAppEvent.GetStringsData());
                pDocsRequest->aPrintList.insert(
                    pDocsRequest->aPrintList.end(), data.begin(), data.end());
                pDocsRequest->pcProcessed = NULL;

                OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
                delete pDocsRequest;
            }
        }
        break;
    case ApplicationEvent::TYPE_PRIVATE_DOSHUTDOWN:
        {
            Desktop* pD = dynamic_cast<Desktop*>(GetpApp());
            OSL_ENSURE( pD, "no desktop ?!?" );
            if( pD )
                pD->doShutdown();
        }
        break;
    case ApplicationEvent::TYPE_QUICKSTART:
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
    case ApplicationEvent::TYPE_SHOWDIALOG:
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
    case ApplicationEvent::TYPE_UNACCEPT:
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
    sal_Bool bVisible = sal_False;
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
            aAppName = OUString( "writer" );
        else if ( rCmdLine.IsCalc() )
            aAppName = OUString( "calc" );
        else if ( rCmdLine.IsDraw() )
            aAppName = OUString( "draw" );
        else if ( rCmdLine.IsImpress() )
            aAppName = OUString( "impress" );
        else if ( rCmdLine.IsBase() )
            aAppName = OUString( "base" );
        else if ( rCmdLine.IsGlobal() )
            aAppName = OUString( "global" );
        else if ( rCmdLine.IsMath() )
            aAppName = OUString( "math" );
        else if ( rCmdLine.IsWeb() )
            aAppName = OUString( "web" );

        // Which splash to use
        OUString aSplashService( "com.sun.star.office.SplashScreen" );
        if ( rCmdLine.HasSplashPipe() )
            aSplashService = OUString("com.sun.star.office.PipeSplashScreen");

        bVisible = sal_True;
        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= bVisible;
        aSeq[1] <<= aAppName;
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        m_rSplashScreen = Reference<XStatusIndicator>(
            xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aSplashService, aSeq, xContext),
            UNO_QUERY);

        if(m_rSplashScreen.is())
                m_rSplashScreen->start(OUString("SplashScreen"), 100);
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
        m_rSplashScreen = NULL;
    }
}

// ========================================================================
void Desktop::DoFirstRunInitializations()
{
    try
    {
        Reference< XJobExecutor > xExecutor = JobExecutor::create( ::comphelper::getProcessComponentContext() );
        xExecutor->trigger( OUString("onFirstRunInitialization") );
    }
    catch(const ::com::sun::star::uno::Exception&)
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
    if (progress != 0)
    {
        progress->SetSplashScreenProgress(60);
    }
    Reference< XFrame > xBackingFrame = xDesktop->findFrame(OUString( "_blank" ), 0);
    Reference< ::com::sun::star::awt::XWindow > xContainerWindow;

    if (xBackingFrame.is())
        xContainerWindow = xBackingFrame->getContainerWindow();
    if (xContainerWindow.is())
    {
        SetDocumentExtendedStyle(xContainerWindow);
        if (progress != 0)
        {
            progress->SetSplashScreenProgress(75);
        }

        Reference< XController > xStartModule = StartModule::createWithParentWindow( xContext, xContainerWindow);
        // Attention: You MUST(!) call setComponent() before you call attachFrame().
        // Because the backing component set the property "IsBackingMode" of the frame
        // to true inside attachFrame(). But setComponent() reset this state everytimes ...
        xBackingFrame->setComponent(Reference< XWindow >(xStartModule, UNO_QUERY), xStartModule);
        if (progress != 0)
        {
            progress->SetSplashScreenProgress(100);
        }
        xStartModule->attachFrame(xBackingFrame);
        if (progress != 0)
        {
            progress->CloseSplashScreen();
        }
        xContainerWindow->setVisible(sal_True);
    }
}

// ========================================================================
void Desktop::CheckFirstRun( )
{
    if (officecfg::Office::Common::Misc::FirstRun::get())
    {
        // this has once been done using a vos timer. this could lead to problems when
        // the timer would trigger when the app is already going down again, since VCL would
        // no longer be available. Since the old handler would do a postUserEvent to the main
        // thread anyway, we can use a vcl timer here to prevent the race contition (#107197#)
        m_firstRunTimer.SetTimeout(3000); // 3 sec.
        m_firstRunTimer.SetTimeoutHdl(LINK(this, Desktop, AsyncInitFirstRun));
        m_firstRunTimer.Start();

#ifdef WNT
        // Check if Quckstarter should be started (on Windows only)
        TCHAR szValue[8192];
        DWORD nValueSize = sizeof(szValue);
        HKEY hKey;
        if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  "Software\\LibreOffice", &hKey ) )
        {
            if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("RunQuickstartAtFirstStart"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
            {
                css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                css::office::Quickstart::createAutoStart(xContext, true/*Quickstart*/, true/*bAutostart*/);
                RegCloseKey( hKey );
            }
        }
#endif

        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::FirstRun::set(false, batch);
        batch->commit();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
