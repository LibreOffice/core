/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "exithelper.hxx"
#include "migration.hxx"

#include <svtools/javacontext.hxx>
#include <com/sun/star/frame/XSessionManagerListener.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
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
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/ui/XUIElementFactoryRegistration.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>

#include <toolkit/unohlp.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/confignode.hxx>
#include <unotools/moduleoptions.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/uri.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/folderrestriction.hxx>
#include <rtl/logfile.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <ucbhelper/contentbroker.hxx>

#include <svtools/fontsubstconfig.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/filter.hxx>

#include "langselect.hxx"

#ifdef ENABLE_TELEPATHY
#include <tubes/manager.hxx>
#endif

#if defined MACOSX
#include <errno.h>
#include <sys/wait.h>
#endif

#ifdef WNT
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif //WNT


using rtl::OUString;
using rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::container;

namespace css = ::com::sun::star;

ResMgr*                 desktop::Desktop::pResMgr = 0;

namespace desktop
{

static oslSignalHandler pSignalHandler = 0;
static sal_Bool _bCrashReporterEnabled = sal_True;

namespace {

void removeTree(OUString const & url) {
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return; //TODO: SAL_WARN if recursive
    default:
        throw css::uno::RuntimeException(
            "cannot open directory " + url,
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                ("cannot iterate directory " + url + ": "
                 + OUString::valueOf(static_cast< sal_Int32 >(rc))),
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        rc = i.getFileStatus(stat);
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                ("cannot stat in directory " + url + ": "
                 + OUString::valueOf(static_cast< sal_Int32 >(rc))),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            removeTree(stat.getFileURL());
        } else {
            rc = osl::File::remove(stat.getFileURL());
            if (rc != osl::FileBase::E_None) {
                throw css::uno::RuntimeException(
                    ("cannot remove file " + stat.getFileURL() + ": "
                     + OUString::valueOf(static_cast< sal_Int32 >(rc))),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    }
    osl::FileBase::RC rc = osl::Directory::remove(url);
    SAL_WARN_IF(
        rc != osl::FileBase::E_None, "desktop",
        "cannot remove directory " << url << ": " +rc);
        // at least on Windows XP removing some existing directories fails with
        // osl::FileBase::E_ACCESS because they are read-only; but keeping those
        // directories around should be harmless once they are empty
}

// Remove any existing UserInstallation's user/extensions/bundled cache
// remaining from old installations.  Apparently due to the old
// share/prereg/bundled mechanism (disabled since
// 5c47e5f63a79a9e72ec4a100786b1bbf65137ed4 "fdo#51252 Disable copying
// share/prereg/bundled to avoid startup crashes"), that cache could contain
// corrupted information (like a UNO component registered twice, which got
// changed from active to passive registration in one LO version, but the
// version of the corresponding bundled extension only incremented in a later LO
// version).  At least in theory, this function could be removed again once no
// UserInstallation can be poisoned by that old share/prereg/bundled mechanism
// any more.  (But then Desktop::SynchronizeExtensionRepositories might need to
// be revisited, see 2d2b19dea1ab401b1b4971ff5b12b87bb11fd666 "Force
// ExtensionManager resync when the implementation changes" which effectively
// got reverted again now.  Now, a mismatch between a UserInstallation's
// user/extensions/bundled and an installation's share/extensions will always be
// detected here and lead to a removal of user/extensions/bundled, so that
// Desktop::SynchronizeExtensionRepositories will then definitely resync
// share/extensions.)
// As a special case, if you create a UserInstallation with LO >= 3.6.1, then
// run an old LO <= 3.5.x using share/prereg/bundled on the same
// UserInstallation (so that it partially overwrites user/extensions/bundled,
// potentially duplicating component information, but not touching
// user/extensions/bundled/buildid), and then run the new LO >= 3.6.1 on the
// same UserInstallation again, it can fail to start (due to the duplicated
// component information).  Even though such downgrading scenarios at best work
// by luck in general, the special token LIBO_NON_PREREG_BUNDLED_EXTENSIONS=TRUE
// is used to detect and fix that problem:  The assumption is that if an old LO
// <= 3.5.x messed with user/extensions/bundled in the meantime, then it would
// have rewritten the unorc (dropping the token), and LO >= 3.6.1 can detect
// that.
void refreshBundledExtensionsDir() {
    OUString buildId(
        "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(buildId); //TODO: detect failure
    OUString dir("$BUNDLED_EXTENSIONS_USER");
    rtl::Bootstrap::expandMacros(dir); //TODO: detect failure
    OUString url(dir + "/buildid");
    OUString nonPrereg(
        "${$BUNDLED_EXTENSIONS_USER/registry/"
        "com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc:"
        "LIBO_NON_PREREG_BUNDLED_EXTENSIONS}");
    rtl::Bootstrap::expandMacros(nonPrereg);
    if (nonPrereg == "TRUE") {
        osl::File f(url);
        switch (f.open(osl_File_OpenFlag_Read)) {
        case osl::FileBase::E_None:
            {
                rtl::ByteSequence s1;
                osl::FileBase::RC rc = f.readLine(s1);
                if (f.close() != osl::FileBase::E_None) {
                    SAL_WARN(
                        "desktop", "cannot close " + url + " after reading");
                }
                if (rc != osl::FileBase::E_None) {
                    throw css::uno::RuntimeException(
                        "cannot read from " + url,
                        css::uno::Reference< css::uno::XInterface >());
                }
                OUString s2(
                    reinterpret_cast< char const * >(s1.getConstArray()),
                    s1.getLength(), RTL_TEXTENCODING_ISO_8859_1);
                    // using ISO 8859-1 avoids any and all conversion errors;
                    // the content should only be a subset of ASCII, anyway
                if (s2 == buildId) {
                    return;
                }
                break;
            }
        case osl::FileBase::E_NOENT:
            break;
        default:
            throw css::uno::RuntimeException(
                "cannot open " + url + " for reading",
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    removeTree(dir);
    switch (osl::Directory::createPath(dir)) {
    case osl::FileBase::E_None:
    case osl::FileBase::E_EXIST:
        break;
    default:
        throw css::uno::RuntimeException(
            "cannot create path " + dir,
            css::uno::Reference< css::uno::XInterface >());
    }
    osl::File f(url);
    if (f.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create) !=
        osl::FileBase::E_None)
    {
        throw css::uno::RuntimeException(
            "cannot open " + url + " for writing",
            css::uno::Reference< css::uno::XInterface >());
    }
    OString buf(OUStringToOString(buildId, RTL_TEXTENCODING_UTF8));
        // using UTF-8 avoids almost all conversion errors (and buildid
        // containing single surrogate halves should never happen, anyway); the
        // content should only be a subset of ASCII, anyway
    sal_uInt64 n;
    if (f.write(buf.getStr(), buf.getLength(), n) != osl::FileBase::E_None
        || n != static_cast< sal_uInt32 >(buf.getLength()))
    {
        throw css::uno::RuntimeException(
            "cannot write to " + url,
            css::uno::Reference< css::uno::XInterface >());
    }
    if (f.close() != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            "cannot close " + url + " after writing",
            css::uno::Reference< css::uno::XInterface >());
    }
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
            sal_Int32 nIndex = 0;
            OUString aLanguage = aUILocaleString.getToken( 0, '-', nIndex);
            OUString aCountry = aUILocaleString.getToken( 0, '-', nIndex);
            OUString aVariant = aUILocaleString.getToken( 0, '-', nIndex);

            ::com::sun::star::lang::Locale aLocale( aLanguage, aCountry, aVariant );

            Desktop::pResMgr = ResMgr::SearchCreateResMgr( "dkt", aLocale);
            AllSettings as = GetSettings();
            as.SetUILocale(aLocale);
            SetSettings(as);
        }
    }

    return Desktop::pResMgr;
}

// ----------------------------------------------------------------------------
// Get a message string securely. There is a fallback string if the resource
// is not available.

OUString Desktop::GetMsgString( sal_uInt16 nId, const OUString& aFaultBackMsg )
{
    ResMgr* resMgr = GetDesktopResManager();
    if ( !resMgr )
        return aFaultBackMsg;
    else
        return OUString( String( ResId( nId, *resMgr )));
}

OUString MakeStartupErrorMessage(OUString const & aErrorMessage)
{
    OUStringBuffer    aDiagnosticMessage( 100 );

    ResMgr* pResMgr = Desktop::GetDesktopResManager();
    if ( pResMgr )
        aDiagnosticMessage.append( OUString(String(ResId(STR_BOOTSTRAP_ERR_CANNOT_START, *pResMgr))) );
    else
        aDiagnosticMessage.appendAscii( "The program cannot be started." );

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
void FatalError(const ::rtl::OUString& sMessage)
{
    ::rtl::OUString sProductKey = ::utl::Bootstrap::getProductKey();
    if ( sProductKey.isEmpty())
    {
        osl_getExecutableFile( &sProductKey.pData );

        ::sal_uInt32 nLastIndex = sProductKey.lastIndexOf('/');
        if ( nLastIndex > 0 )
            sProductKey = sProductKey.copy( nLastIndex+1 );
    }

    ::rtl::OUStringBuffer sTitle (128);
    sTitle.append      (sProductKey     );
    sTitle.appendAscii (" - Fatal Error");

    Application::ShowNativeErrorBox (sTitle.makeStringAndClear (), sMessage);
    _exit(ExitHelper::E_FATAL_ERROR);
}

static bool ShouldSuppressUI(const CommandLineArgs& rCmdLine)
{
    return  rCmdLine.IsInvisible() ||
            rCmdLine.IsHeadless() ||
            rCmdLine.IsQuickstart();
}

namespace
{
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

rtl::OUString ReplaceStringHookProc( const rtl::OUString& rStr )
{
    rtl::OUString sRet(rStr);

    if ( sRet.indexOf( "%PRODUCT" ) != -1 )
    {
        rtl::OUString sBrandName = BrandName::get();
        rtl::OUString sVersion = Version::get();
        rtl::OUString sAboutBoxVersion = AboutBoxVersion::get();
        rtl::OUString sAboutBoxVersionSuffix = AboutBoxVersionSuffix::get();
        rtl::OUString sExtension = Extension::get();
        rtl::OUString sXMLFileFormatName = XMLFileFormatName::get();
        rtl::OUString sXMLFileFormatVersion = XMLFileFormatVersion::get();

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
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSIONSUFFIX", sAboutBoxVersionSuffix );
        sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSION", sAboutBoxVersion );
        sRet = sRet.replaceAll( "%PRODUCTEXTENSION", sExtension );
        sRet = sRet.replaceAll( "%PRODUCTXMLFILEFORMATNAME", sXMLFileFormatName );
        sRet = sRet.replaceAll( "%PRODUCTXMLFILEFORMATVERSION", sXMLFileFormatVersion );
    }

    if ( sRet.indexOf( "%OOOVENDOR" ) != -1 )
    {
        rtl::OUString sOOOVendor = OOOVendor::get();

        if ( sOOOVendor.isEmpty() )
        {
            sOOOVendor = utl::ConfigManager::getVendor();
        }

        sRet = sRet.replaceAll( "%OOOVENDOR", sOOOVendor );
    }

    if ( sRet.indexOf( "%WRITERCOMPATIBILITYVERSIONOOO11" ) != -1 )
    {
        rtl::OUString sWriterCompatibilityVersionOOo11 = WriterCompatibilityVersionOOo11::get();
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
: m_bServicesRegistered( false )
, m_aBootstrapError( BE_OK )
{
    RTL_LOGFILE_TRACE( "desktop (cd100003) ::Desktop::Desktop" );
}

Desktop::~Desktop()
{
#ifdef ENABLE_TELEPATHY
    TeleManager::finalize();
#endif
}

void Desktop::Init()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Init" );
    SetBootstrapStatus(BS_OK);

    refreshBundledExtensionsDir();

    // We need to have service factory before going further, but see fdo#37195.
    // Doing this will mmap common.rdb, making it not overwritable on windows,
    // so this can't happen before the synchronization above. Lets rework this
    // so that the above is called *from* ensureProcessServiceFactory or
    // something to enforce this gotcha
    ensureProcessServiceFactory();

    if( !::comphelper::getProcessServiceFactory().is())
    {
        OSL_FAIL("Service factory should have been crated in soffice_main().");
        SetBootstrapError( BE_UNO_SERVICEMANAGER );
    }

    if ( GetBootstrapError() == BE_OK )
    {
        // prepare language
        if ( !LanguageSelection::prepareLanguage() )
        {
            if ( LanguageSelection::getStatus() == LanguageSelection::LS_STATUS_CANNOT_DETERMINE_LANGUAGE )
                SetBootstrapError( BE_LANGUAGE_MISSING );
            else
                SetBootstrapError( BE_OFFICECONFIG_BROKEN );
        }
    }

    if ( GetBootstrapError() == BE_OK )
    {
        const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();
        // start ipc thread only for non-remote offices
        RTL_LOGFILE_CONTEXT( aLog2, "desktop (cd100003) ::OfficeIPCThread::EnableOfficeIPCThread" );
        OfficeIPCThread::Status aStatus = OfficeIPCThread::EnableOfficeIPCThread();
        if ( aStatus == OfficeIPCThread::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING );
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_2ND_OFFICE )
        {
            // 2nd office startup should terminate after sending cmdlineargs through pipe
            SetBootstrapStatus(BS_TERMINATE);
        }
        else if ( rCmdLineArgs.IsHelp() )
        {
            // disable IPC thread in an instance that is just showing a help message
            OfficeIPCThread::DisableOfficeIPCThread();
        }
        pSignalHandler = osl_addSignalHandler(SalMainPipeExchangeSignal_impl, NULL);
    }
}

void Desktop::InitFinished()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::InitFinished" );

    CloseSplashScreen();
}

// GetCommandLineArgs() requires this code to work, otherwise it will abort, and
// on Unix command line args needs to be checked before Desktop::Init()
void Desktop::ensureProcessServiceFactory()
{
    if (!comphelper::getProcessServiceFactory().is())
    {
        try
        {
            comphelper::setProcessServiceFactory(
                CreateApplicationServiceManager());
        }
        catch (const css::uno::Exception& e)
        {
            // Application::ShowNativeErrorBox would only work after InitVCL, so
            // all we can realistically do here is hope the user can see stderr:
            std::cerr << "UNO Exception: " << e.Message << std::endl;
            // Let exceptions escape and tear down the process, it is completely
            // broken anyway:
            throw;
        }
    }
}

void Desktop::DeInit()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::DeInit" );

    try {
        // instead of removing of the configManager just let it commit all the changes
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );
        utl::ConfigManager::storeConfigItems();
        FlushConfiguration();
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );

        // close splashscreen if it's still open
        CloseSplashScreen();
        Reference<XMultiServiceFactory> xXMultiServiceFactory(::comphelper::getProcessServiceFactory());
        DestroyApplicationServiceManager( xXMultiServiceFactory );
        // nobody should get a destroyd service factory...
        ::comphelper::setProcessServiceFactory( NULL );

        // clear lockfile
        m_pLockfile.reset();

        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            osl_removeSignalHandler( pSignalHandler );
    } catch (const RuntimeException&) {
        // someone threw an exception during shutdown
        // this will leave some garbage behind..
    }

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "FINISHED WITH Destop::DeInit" );
}

sal_Bool Desktop::QueryExit()
{
    try
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );
        utl::ConfigManager::storeConfigItems();
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );
    }
    catch ( const RuntimeException& )
    {
    }

    const sal_Char SUSPEND_QUICKSTARTVETO[] = "SuspendQuickstartVeto";

    Reference< ::com::sun::star::frame::XDesktop >
            xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUString("com.sun.star.frame.Desktop") ),
                UNO_QUERY );

    Reference < ::com::sun::star::beans::XPropertySet > xPropertySet( xDesktop, UNO_QUERY );
    if ( xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_True;
        xPropertySet->setPropertyValue( OUString(SUSPEND_QUICKSTARTVETO ), a );
    }

    sal_Bool bExit = ( !xDesktop.is() || xDesktop->terminate() );


    if ( !bExit && xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_False;
        xPropertySet->setPropertyValue( OUString(SUSPEND_QUICKSTARTVETO ), a );
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

        m_pLockfile.reset();
    }

    return bExit;
}

void Desktop::HandleBootstrapPathErrors( ::utl::Bootstrap::Status aBootstrapStatus, const OUString& aDiagnosticMessage )
{
    if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
    {
        ::rtl::OUString        aProductKey;
        ::rtl::OUString        aTemp;

        osl_getExecutableFile( &aProductKey.pData );
        sal_uInt32     lastIndex = aProductKey.lastIndexOf('/');
        if ( lastIndex > 0 )
            aProductKey = aProductKey.copy( lastIndex+1 );

        aTemp = ::utl::Bootstrap::getProductKey( aProductKey );
        if ( !aTemp.isEmpty() )
            aProductKey = aTemp;

        OUString        aMessage;
        OUStringBuffer    aBuffer( 100 );
        aBuffer.append( aDiagnosticMessage );

        aBuffer.appendAscii( "\n" );

        ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
        aBootstrapFailedBox.SetText( aProductKey );
        aBootstrapFailedBox.Execute();
    }
}

// Create a error message depending on bootstrap failure code and an optional file url
::rtl::OUString    Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const ::rtl::OUString& aFileURL )
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

void Desktop::HandleBootstrapErrors( BootstrapError aBootstrapError )
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
        fputs(
            aBootstrapError == BE_UNO_SERVICEMANAGER
            ? ("The application cannot be started. " "\n"
               "The component manager is not available." "\n")
                // STR_BOOTSTRAP_ERR_CANNOT_START, STR_BOOTSTRAP_ERR_NO_SERVICE
            : ("The application cannot be started. " "\n"
               "The configuration service is not available." "\n"),
                // STR_BOOTSTRAP_ERR_CANNOT_START,
                // STR_BOOTSTRAP_ERR_NO_CFG_SERVICE
            stderr);

        // First sentence. We cannot bootstrap office further!
        OUString            aMessage;
        OUStringBuffer        aDiagnosticMessage( 100 );

        OUString aErrorMsg;

        if ( aBootstrapError == BE_UNO_SERVICEMANAGER )
            aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SERVICE,
                            OUString( "The service manager is not available." ) );
        else
            aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_CFG_SERVICE,
                            OUString( "The configuration service is not available." ) );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.appendAscii( "\n" );

        // Due to the fact the we haven't a backup applicat.rdb file anymore it is not possible to
        // repair the installation with the setup executable besides the office executable. Now
        // we have to ask the user to start the setup on CD/installation directory manually!!
        OUString aStartSetupManually( GetMsgString(
            STR_ASK_START_SETUP_MANUALLY,
            OUString( "Start setup application to repair the installation from CD, or the folder containing the installation packages." ) ));

        aDiagnosticMessage.append( aStartSetupManually );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );

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
        aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
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
            Usefull to trigger the error report tool without
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
    static ::rtl::OUString SERVICENAME_RECOVERYUI("com.sun.star.comp.svx.RecoveryUI");
    static ::rtl::OUString COMMAND_EMERGENCYSAVE("vnd.sun.star.autorecovery:/doEmergencySave");
    static ::rtl::OUString COMMAND_RECOVERY("vnd.sun.star.autorecovery:/doAutoRecovery");
    static ::rtl::OUString COMMAND_CRASHREPORT("vnd.sun.star.autorecovery:/doCrashReport");

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

    Reference< css::frame::XSynchronousDispatch > xRecoveryUI(
        xSMGR->createInstance(SERVICENAME_RECOVERYUI),
        css::uno::UNO_QUERY_THROW);

    Reference< css::util::XURLTransformer > xURLParser(
        css::util::URLTransformer::create(::comphelper::getProcessComponentContext()) );

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
    rtl::OUString execUrl;
    OSL_VERIFY(osl_getExecutableFile(&execUrl.pData) == osl_Process_E_None);
    rtl::OUString execPath;
    rtl::OString execPath8;
    if ((osl::FileBase::getSystemPathFromFileURL(execUrl, execPath)
         != osl::FileBase::E_None) ||
        !execPath.convertToString(
            &execPath8, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::abort();
    }
    std::vector< rtl::OString > args;
    args.push_back(execPath8);
    bool wait = false;
    if (passArguments) {
        sal_uInt32 n = osl_getCommandArgCount();
        for (sal_uInt32 i = 0; i < n; ++i) {
            rtl::OUString arg;
            OSL_VERIFY(osl_getCommandArg(i, &arg.pData) == osl_Process_E_None);
            if (arg.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("--accept="))) {
                wait = true;
            }
            rtl::OString arg8;
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
    for (std::vector< rtl::OString >::iterator i(args.begin()); i != args.end();
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
            m_pLockfile.reset();

            if( bRestart )
            {
                OfficeIPCThread::DisableOfficeIPCThread();
                if( pSignalHandler )
                    osl_removeSignalHandler( pSignalHandler );

                restartOnMac(false);
                if ( m_rSplashScreen.is() )
                    m_rSplashScreen->reset();

                _exit( ExitHelper::E_CRASH_WITH_RESTART );
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
        OSL_ENSURE( pContainerWindow, "Desktop::Main: no implementation access to the frame's container window!" );
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

    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Main" );

    // Remember current context object
    com::sun::star::uno::ContextLayer layer(
        com::sun::star::uno::getCurrentContext() );

    BootstrapError eError = GetBootstrapError();
    if ( eError != BE_OK )
    {
        HandleBootstrapErrors( eError );
        return EXIT_FAILURE;
    }

    BootstrapStatus eStatus = GetBootstrapStatus();
    if (eStatus == BS_TERMINATE) {
        return EXIT_FAILURE;
    }

    // Detect desktop environment - need to do this as early as possible
    com::sun::star::uno::setCurrentContext(
        new DesktopContext( com::sun::star::uno::getCurrentContext() ) );

    CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();

    // setup configuration error handling
    ConfigurationErrorHandler aConfigErrHandler;
    if (!ShouldSuppressUI(rCmdLineArgs))
        aConfigErrHandler.activate();

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    // Startup screen
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main { OpenSplashScreen" );
    OpenSplashScreen();
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main } OpenSplashScreen" );

    SetSplashScreenProgress(10);

    UserInstall::UserInstallStatus inst_fin = UserInstall::finalize();
    if (inst_fin != UserInstall::Ok && inst_fin != UserInstall::Created)
    {
        OSL_FAIL("userinstall failed");
        if ( inst_fin == UserInstall::E_NoDiskSpace )
            HandleBootstrapErrors( BE_USERINSTALL_NOTENOUGHDISKSPACE );
        else if ( inst_fin == UserInstall::E_NoWriteAccess )
            HandleBootstrapErrors( BE_USERINSTALL_NOWRITEACCESS );
        else
            HandleBootstrapErrors( BE_USERINSTALL_FAILED );
        return EXIT_FAILURE;
    }
    // refresh path information
    utl::Bootstrap::reloadData();
    SetSplashScreenProgress(20);

    Reference< XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();

    Reference< XDesktop > xDesktop;
    Reference< ::com::sun::star::task::XRestartManager > xRestartManager;
    try
    {
        RegisterServices();

        SetSplashScreenProgress(25);

#ifndef UNX
        if ( rCmdLineArgs.IsHelp() )
        {
            displayCmdlineHelp();
            return EXIT_SUCCESS;
        }
#endif

        // check user installation directory for lockfile so we can be sure
        // there is no other instance using our data files from a remote host
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main -> Lockfile" );
        m_pLockfile.reset( new Lockfile );
#ifndef ANDROID
        if ( !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoLockcheck() && !m_pLockfile->check( Lockfile_execWarning ))
        {
            // Lockfile exists, and user clicked 'no'
            return EXIT_FAILURE;
        }
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main <- Lockfile" );

        // check if accessibility is enabled but not working and allow to quit
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ GetEnableATToolSupport" );
        if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
        {
            sal_Bool bQuitApp;

            if( !InitAccessBridge( true, bQuitApp ) )
                if( bQuitApp )
                    return EXIT_FAILURE;
        }
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} GetEnableATToolSupport" );
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
        ::com::sun::star::lang::Locale aLocale;
        ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( "ofa", aLocale );
        String aTitle = pLabelResMgr ? String( ResId( RID_APPTITLE, *pLabelResMgr ) ) : String();
        delete pLabelResMgr;

#ifdef DBG_UTIL
        //include version ID in non product builds
        ::rtl::OUString aDefault("development");
        aTitle += rtl::OUString(" [");
        String aVerId( utl::Bootstrap::getProductSource(aDefault));
        aTitle += aVerId;
        aTitle += ']';
#endif

        SetDisplayName( aTitle );
        SetSplashScreenProgress(35);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create SvtPathOptions and SvtLanguageOptions" );
        pExecGlobals->pPathOptions.reset( new SvtPathOptions);
        SetSplashScreenProgress(40);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create SvtPathOptions and SvtLanguageOptions" );

        // Check special env variable
        std::vector< String > aUnrestrictedFolders;
        svt::getUnrestrictedFolders( aUnrestrictedFolders );

        if ( !aUnrestrictedFolders.empty() )
        {
            // Set different working directory. The first entry is
            // the new work path.
            String aWorkPath = aUnrestrictedFolders[0];
            SvtPathOptions().SetWorkPath( aWorkPath );
        }

        xDesktop = Reference<XDesktop>( xSMgr->createInstance(
            OUString( "com.sun.star.frame.Desktop" )), UNO_QUERY );

        // create service for loadin SFX (still needed in startup)
        pExecGlobals->xGlobalBroadcaster = Reference < css::document::XEventListener >
            ( xSMgr->createInstance(
            rtl::OUString( "com.sun.star.frame.GlobalEventBroadcaster" ) ), UNO_QUERY );

        /* ensure existance of a default window that messages can be dispatched to
           This is for the benefit of testtool which uses PostUserEvent extensively
           and else can deadlock while creating this window from another tread while
           the main thread is not yet in the event loop.
        */
        Application::GetDefaultDevice();

#ifndef ANDROID
        // Check if bundled or shared extensions were added /removed
        // and process those extensions (has to be done before checking
        // the extension dependencies!
        SynchronizeExtensionRepositories();
        bool bAbort = CheckExtensionDependencies();
        if ( bAbort )
            return EXIT_FAILURE;

        {
            ::comphelper::ComponentContext aContext( xSMgr );
            xRestartManager.set( aContext.getSingleton( ::rtl::OUString( "com.sun.star.task.OfficeRestartManager"  ) ), UNO_QUERY );
        }

        // check whether the shutdown is caused by restart
        pExecGlobals->bRestartRequested = ( xRestartManager.is() && xRestartManager->isRestartRequested( sal_True ) );

        if (inst_fin == UserInstall::Created)
        {
            Migration::migrateSettingsIfNecessary();
        }
#endif

        // keep a language options instance...
        pExecGlobals->pLanguageOptions.reset( new SvtLanguageOptions(sal_True));

        if (pExecGlobals->xGlobalBroadcaster.is())
        {
            css::document::EventObject aEvent;
            aEvent.EventName = ::rtl::OUString("OnStartApp");
            pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);
        }

        SetSplashScreenProgress(50);

        // Backing Component
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ impl_checkRecoveryState" );
        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} impl_checkRecoveryState" );

        {
            ::comphelper::ComponentContext aContext( xSMgr );
            xRestartManager.set( aContext.getSingleton( ::rtl::OUString( "com.sun.star.task.OfficeRestartManager"  ) ), UNO_QUERY );
        }

        // check whether the shutdown is caused by restart
        pExecGlobals->bRestartRequested = ( xRestartManager.is() && xRestartManager->isRestartRequested( sal_True ) );

        if ( rCmdLineArgs.IsHeadless() )
        {
            // Ensure that we use not the system file dialogs as
            // headless mode relies on Application::EnableHeadlessMode()
            // which does only work for VCL dialogs!!
            SvtMiscOptions aMiscOptions;
            pExecGlobals->bUseSystemFileDialog = aMiscOptions.UseSystemFileDialog();
            aMiscOptions.SetUseSystemFileDialog( sal_False );
        }

        if ( !pExecGlobals->bRestartRequested )
        {
            if ((!rCmdLineArgs.WantsToLoadDocument() && !rCmdLineArgs.IsInvisible() && !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsQuickstart()) &&
                (SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE)) &&
                (!bExistsRecoveryData                                                  ) &&
                (!bExistsSessionData                                                   ) &&
                (!Application::AnyInput( VCL_INPUT_APPEVENT )                          ))
            {
                 RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create BackingComponent" );
                 ShowBackingComponent(this);
                 RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create BackingComponent" );
            }
        }
    }
    catch ( const com::sun::star::lang::WrappedTargetException& wte )
    {
        com::sun::star::uno::Exception te;
        wte.TargetException >>= te;
        FatalError( MakeStartupConfigAccessErrorMessage(wte.Message + te.Message) );
        return EXIT_FAILURE;
    }
    catch ( const com::sun::star::uno::Exception& e )
    {
        FatalError( MakeStartupErrorMessage(e.Message) );
        return EXIT_FAILURE;
    }
    SetSplashScreenProgress(55);

    SvtFontSubstConfig().Apply();

    SvtTabAppearanceCfg aAppearanceCfg;
    aAppearanceCfg.SetInitialized();
    aAppearanceCfg.SetApplicationDefaults( this );
    SvtAccessibilityOptions aOptions;
    aOptions.SetVCLSettings();
    SetSplashScreenProgress(60);

#ifdef ENABLE_TELEPATHY
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
            InitializeQuickstartMode( xSMgr );

        RTL_LOGFILE_CONTEXT( aLog2, "desktop (cd100003) createInstance com.sun.star.frame.Desktop" );
        try
        {
            if ( xDesktop.is() )
                xDesktop->addTerminateListener( new OfficeIPCThreadController );
            SetSplashScreenProgress(100);
        }
        catch ( const com::sun::star::uno::Exception& e )
        {
            FatalError( MakeStartupErrorMessage(e.Message) );
            return EXIT_FAILURE;
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
        RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Application::Execute()" );

        try
        {
            // The JavaContext contains an interaction handler which is used when
            // the creation of a Java Virtual Machine fails
            com::sun::star::uno::ContextLayer layer2(
                new svt::JavaContext( com::sun::star::uno::getCurrentContext() ) );

            // check whether the shutdown is caused by restart just before entering the Execute
            pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested || ( xRestartManager.is() && xRestartManager->isRestartRequested( sal_True ) );

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

    if ( pExecGlobals->bRestartRequested )
        SetRestartState();

    if (pExecGlobals->xGlobalBroadcaster.is())
    {
        css::document::EventObject aEvent;
        aEvent.EventName = ::rtl::OUString("OnCloseApp");
        pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);
    }

    delete pResMgr, pResMgr = NULL;
    // Restore old value
    const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();
    if ( rCmdLineArgs.IsHeadless() )
        SvtMiscOptions().SetUseSystemFileDialog( pExecGlobals->bUseSystemFileDialog );

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
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "-> dispose path/language options" );
    pExecGlobals->pLanguageOptions.reset( 0 );
    pExecGlobals->pPathOptions.reset( 0 );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- dispose path/language options" );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "-> deinit ucb" );
    ::ucbhelper::ContentBroker::deinitialize();
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- deinit ucb" );

    sal_Bool bRR = pExecGlobals->bRestartRequested;
    delete pExecGlobals, pExecGlobals = NULL;

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "FINISHED WITH Destop::Main" );
    if ( bRR )
    {
        restartOnMac(true);
        if ( m_rSplashScreen.is() )
            m_rSplashScreen->reset();

        return ExitHelper::E_NORMAL_RESTART;
    }
    return EXIT_SUCCESS;
}

IMPL_LINK( Desktop, ImplInitFilterHdl, ConvertData*, pData )
{
    return GraphicFilter::GetGraphicFilter().GetFilterCallback().Call( pData );
}

bool Desktop::InitializeConfiguration()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (jb99855) ::InitConfiguration" );
    try
    {
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext() );
        return true;
    }
    catch( const ::com::sun::star::lang::ServiceNotRegisteredException& )
    {
        this->HandleBootstrapErrors( Desktop::BE_UNO_SERVICE_CONFIG_MISSING );
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


sal_Bool Desktop::InitializeQuickstartMode( Reference< XMultiServiceFactory >& rSMgr )
{
    try
    {
        // the shutdown icon sits in the systray and allows the user to keep
        // the office instance running for quicker restart
        // this will only be activated if --quickstart was specified on cmdline
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.office.Quickstart" );

        sal_Bool bQuickstart = shouldLaunchQuickstart();

        // Try to instanciate quickstart service. This service is not mandatory, so
        // do nothing if service is not available

        // #i105753# the following if was invented for performance
        // unfortunately this broke the QUARTZ behavior which is to always run
        // in quickstart mode since Mac applications do not usually quit
        // when the last document closes
        #ifndef QUARTZ
        if ( bQuickstart )
        #endif
        {
            Sequence< Any > aSeq( 1 );
            aSeq[0] <<= bQuickstart;
            Reference < XComponent > xQuickstart( rSMgr->createInstanceWithArguments(
                                                rtl::OUString( "com.sun.star.office.Quickstart" ), aSeq ),
                                                UNO_QUERY );
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
    RTL_LOGFILE_PRODUCT_CONTEXT( aLog, "PERFORMANCE - DesktopOpenClients_Impl()" );

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
      RTL_LOGFILE_CONTEXT( aLog, "desktop (jl97489) ::Desktop::EnableOleAutomation" );
#ifdef WNT
    Reference< XMultiServiceFactory > xSMgr=  comphelper::getProcessServiceFactory();
    xSMgr->createInstance(rtl::OUString("com.sun.star.bridge.OleApplicationRegistration"));
    xSMgr->createInstance(rtl::OUString("com.sun.star.comp.ole.EmbedServer"));
#endif
}

void Desktop::PreloadModuleData( const CommandLineArgs& rArgs )
{
    Reference< XMultiServiceFactory > rFactory = ::comphelper::getProcessServiceFactory();

    Sequence < com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = ::rtl::OUString("Hidden");
    args[0].Value <<= sal_True;
    Reference < XComponentLoader > xLoader( ::comphelper::getProcessServiceFactory()->createInstance(
        ::rtl::OUString("com.sun.star.frame.Desktop") ), UNO_QUERY );

    if ( !xLoader.is() )
        return;

    if ( rArgs.IsWriter() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( rtl::OUString("private:factory/swriter"),
                ::rtl::OUString("_blank"), 0, args ), UNO_QUERY_THROW );
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
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( rtl::OUString("private:factory/scalc"),
                ::rtl::OUString("_blank"), 0, args ), UNO_QUERY_THROW );
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
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( rtl::OUString("private:factory/sdraw"),
                ::rtl::OUString("_blank"), 0, args ), UNO_QUERY_THROW );
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
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( rtl::OUString("private:factory/simpress"),
                ::rtl::OUString("_blank"), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }
}

void Desktop::PreloadConfigurationData()
{
    Reference< XMultiServiceFactory > rFactory = ::comphelper::getProcessServiceFactory();
    Reference< XNameAccess > xNameAccess( rFactory->createInstance(
        rtl::OUString( "com.sun.star.frame.UICommandDescription" )), UNO_QUERY );

    rtl::OUString aWriterDoc( "com.sun.star.text.TextDocument" );
    rtl::OUString aCalcDoc( "com.sun.star.sheet.SpreadsheetDocument" );
    rtl::OUString aDrawDoc( "com.sun.star.drawing.DrawingDocument" );
    rtl::OUString aImpressDoc( "com.sun.star.presentation.PresentationDocument" );

    // preload commands configuration
    if ( xNameAccess.is() )
    {
        Any a;
        Reference< XNameAccess > xCmdAccess;

        try
        {
            a = xNameAccess->getByName( aWriterDoc );
            a >>= xCmdAccess;
            if ( xCmdAccess.is() )
            {
                xCmdAccess->getByName( rtl::OUString( ".uno:BasicShapes" ));
                xCmdAccess->getByName( rtl::OUString( ".uno:EditGlossary" ));
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
                xCmdAccess->getByName( rtl::OUString( ".uno:InsertObjectStarMath" ));
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
                xCmdAccess->getByName( rtl::OUString( ".uno:Polygon" ));
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload window state configuration
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    rtl::OUString( "com.sun.star.ui.WindowStateConfiguration" )), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        Any a;
        Reference< XNameAccess > xWindowAccess;
        try
        {
            a = xNameAccess->getByName( aWriterDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( rtl::OUString( "private:resource/toolbar/standardbar" ));
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aCalcDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( rtl::OUString( "private:resource/toolbar/standardbar" ));
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aDrawDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( rtl::OUString( "private:resource/toolbar/standardbar" ));
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aImpressDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( rtl::OUString( "private:resource/toolbar/standardbar" ));
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload user interface element factories
    Sequence< Sequence< css::beans::PropertyValue > > aSeqSeqPropValue;
    Reference< ::com::sun::star::ui::XUIElementFactoryRegistration > xUIElementFactory(
        rFactory->createInstance(
            rtl::OUString( "com.sun.star.ui.UIElementFactoryManager" )),
            UNO_QUERY );
    if ( xUIElementFactory.is() )
    {
        try
        {
            aSeqSeqPropValue = xUIElementFactory->getRegisteredFactories();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload popup menu controller factories. As all controllers are in the same
    // configuration file they also get preloaded!
    Reference< ::com::sun::star::frame::XUIControllerRegistration > xPopupMenuControllerFactory(
        rFactory->createInstance(
            rtl::OUString( "com.sun.star.frame.PopupMenuControllerFactory" )),
            UNO_QUERY );
    if ( xPopupMenuControllerFactory.is() )
    {
        try
        {
            xPopupMenuControllerFactory->hasController(
                        rtl::OUString( ".uno:CharFontName" ),
                        OUString() );
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload filter configuration
    Sequence< OUString > aSeq;
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    rtl::OUString( "com.sun.star.document.FilterFactory" )), UNO_QUERY );
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
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    rtl::OUString( "com.sun.star.document.TypeDetection" )), UNO_QUERY );
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
    SvtInternalOptions  aInternalOptions;

    Reference<XMultiServiceFactory> rFactory = ::comphelper::getProcessServiceFactory();

    if (!rArgs.IsQuickstart())
    {
        sal_Bool bShowHelp = sal_False;
        ::rtl::OUStringBuffer aHelpURLBuffer;
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
            aIniName    += OUString( "perftune" );
#if defined(WNT)
            aIniName    += OUString( ".ini" );
#else
            aIniName    += OUString( "rc" );
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
            Reference< XDispatch > xRecovery(
                    ::comphelper::getProcessServiceFactory()->createInstance( OUString("com.sun.star.frame.AutoRecovery") ),
                    ::com::sun::star::uno::UNO_QUERY_THROW );

            Reference< css::util::XURLTransformer > xParser( css::util::URLTransformer::create(::comphelper::getProcessComponentContext()) );

            css::util::URL aCmd;
            aCmd.Complete = ::rtl::OUString("vnd.sun.star.autorecovery:/disableRecovery");
            xParser->parseStrict(aCmd);

            xRecovery->dispatch(aCmd, css::uno::Sequence< css::beans::PropertyValue >());
        }
        catch(const css::uno::Exception& e)
        {
            OUString aMessage = OUString("Could not disable AutoRecovery.\n")
                + e.Message;
            OSL_FAIL(OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
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
                OUString aMessage = OUString("Error during recovery\n")
                    + e.Message;
                OSL_FAIL(OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }

        Reference< XInitialization > xSessionListener;
        try
        {
            xSessionListener = Reference< XInitialization >(::comphelper::getProcessServiceFactory()->createInstance(
                        OUString("com.sun.star.frame.SessionListener")), UNO_QUERY_THROW);

            // specifies whether the UI-interaction on Session shutdown is allowed
            sal_Bool bAllowUI = isUIOnSessionShutdownAllowed();
            css::beans::NamedValue aProperty( ::rtl::OUString( "AllowUserInteractionOnQuit"  ),
                                              css::uno::makeAny( bAllowUI ) );
            css::uno::Sequence< css::uno::Any > aArgs( 1 );
            aArgs[0] <<= aProperty;

            xSessionListener->initialize( aArgs );
        }
        catch(const com::sun::star::uno::Exception& e)
        {
            OUString aMessage = OUString("Registration of session listener failed\n")
                + e.Message;
            OSL_FAIL(OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        }

        if ( !bExistsRecoveryData )
        {
            // session management
            try
            {
                Reference< XSessionManagerListener > r(xSessionListener, UNO_QUERY_THROW);
                r->doRestore();
            }
            catch(const com::sun::star::uno::Exception& e)
            {
                OUString aMessage = OUString("Error in session management\n")
                    + e.Message;
                OSL_FAIL(OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
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
    Reference< XFramesSupplier > xTasksSupplier(
            ::comphelper::getProcessServiceFactory()->createInstance( OUString("com.sun.star.frame.Desktop") ),
            ::com::sun::star::uno::UNO_QUERY_THROW );
    Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY_THROW );
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

    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::OpenDefault" );

    ::rtl::OUString        aName;
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
    const String& rName, boost::optional< rtl::OUString > const & cwdUrl )
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

    // Add path seperator to these directory and make given URL (rName) absolute by using of current working directory
    // Attention: "setFinalSlash()" is necessary for calling "smartRel2Abs()"!!!
    // Otherwhise last part will be ignored and wrong result will be returned!!!
    // "smartRel2Abs()" interpret given URL as file not as path. So he truncate last element to get the base path ...
    // But if we add a seperator - he doesn't do it anymore.
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
        createAcceptor(rAppEvent.GetData());
        break;
    case ApplicationEvent::TYPE_APPEAR:
        if ( !GetCommandLineArgs().IsInvisible() )
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

            // find active task - the active task is always a visible task
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >
                  xDesktop( xSMGR->createInstance( OUString("com.sun.star.frame.Desktop") ),
                            ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xTask = xDesktop->getActiveFrame();
            if ( !xTask.is() )
            {
                // get any task if there is no active one
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > xList( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
                if ( xList->getCount()>0 )
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
                Reference< XFrame > xBackingFrame;
                Reference< ::com::sun::star::awt::XWindow > xContainerWindow;
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xDesktopFrame( xDesktop, UNO_QUERY );

                xBackingFrame = xDesktopFrame->findFrame(OUString( "_blank" ), 0);
                if (xBackingFrame.is())
                    xContainerWindow = xBackingFrame->getContainerWindow();
                if (xContainerWindow.is())
                {
                    Sequence< Any > lArgs(1);
                    lArgs[0] <<= xContainerWindow;
                    Reference< XController > xBackingComp(
                        xSMGR->createInstanceWithArguments(OUString( "com.sun.star.frame.StartModule" ), lArgs),
                        UNO_QUERY);
                    if (xBackingComp.is())
                    {
                        Reference< ::com::sun::star::awt::XWindow > xBackingWin(xBackingComp, UNO_QUERY);
                        // Attention: You MUST(!) call setComponent() before you call attachFrame().
                        // Because the backing component set the property "IsBackingMode" of the frame
                        // to true inside attachFrame(). But setComponent() reset this state everytimes ...
                        xBackingFrame->setComponent(xBackingWin, xBackingComp);
                        xBackingComp->attachFrame(xBackingFrame);
                        xContainerWindow->setVisible(sal_True);

                        Window* pCompWindow = VCLUnoHelper::GetWindow(xBackingFrame->getComponentWindow());
                        if (pCompWindow)
                            pCompWindow->Update();
                    }
                }
            }
        }
        break;
    case ApplicationEvent::TYPE_HELP:
#ifndef UNX
        // in non unix version allow showing of cmdline help window
        displayCmdlineHelp();
#endif
        break;
    case ApplicationEvent::TYPE_OPEN:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest(
                    rCmdLine.getCwdUrl());
                pDocsRequest->aOpenList.push_back(rAppEvent.GetData());
                pDocsRequest->pcProcessed = NULL;

                OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
                delete pDocsRequest;
            }
        }
        break;
    case ApplicationEvent::TYPE_OPENHELPURL:
        // start help for a specific URL
        Application::GetHelp()->Start(rAppEvent.GetData(), NULL);
        break;
    case ApplicationEvent::TYPE_PRINT:
        {
            const CommandLineArgs& rCmdLine = GetCommandLineArgs();
            if ( !rCmdLine.IsInvisible() && !rCmdLine.IsTerminateAfterInit() )
            {
                ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest(
                    rCmdLine.getCwdUrl());
                pDocsRequest->aPrintList.push_back(rAppEvent.GetData());
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
            sal_Bool bQuickstart( sal_True );
            Sequence< Any > aSeq( 1 );
            aSeq[0] <<= bQuickstart;

            Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                                                           rtl::OUString( "com.sun.star.office.Quickstart" )),
                                                       UNO_QUERY );
            if ( xQuickstart.is() )
                xQuickstart->initialize( aSeq );
        }
        break;
    case ApplicationEvent::TYPE_SHOWDIALOG:
        // ignore all errors here. It's clicking a menu entry only ...
        // The user will try it again, in case nothing happens .-)
        try
        {
            Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

            Reference< css::frame::XDispatchProvider >
                xDesktop( xSMGR->createInstance( OUString("com.sun.star.frame.Desktop") ),
                ::com::sun::star::uno::UNO_QUERY );

            // check provider ... we know it's weak reference only
            if ( ! xDesktop.is())
                return;

            Reference< css::util::XURLTransformer > xParser( css::util::URLTransformer::create(::comphelper::getProcessComponentContext()) );
            css::util::URL aCommand;
            if( rAppEvent.GetData() == ::rtl::OUString("PREFERENCES") )
                aCommand.Complete = rtl::OUString( ".uno:OptionsTreeDialog"  );
            else if( rAppEvent.GetData() == ::rtl::OUString("ABOUT") )
                aCommand.Complete = rtl::OUString( ".uno:About"  );
            if( !aCommand.Complete.isEmpty() )
            {
                xParser->parseStrict(aCommand);

                css::uno::Reference< css::frame::XDispatch > xDispatch = xDesktop->queryDispatch(aCommand, rtl::OUString(), 0);
                if (xDispatch.is())
                    xDispatch->dispatch(aCommand, css::uno::Sequence< css::beans::PropertyValue >());
            }
        }
        catch(const css::uno::Exception&)
        {}
        break;
    case ApplicationEvent::TYPE_UNACCEPT:
        // try to remove corresponding acceptor
        destroyAcceptor(rAppEvent.GetData());
        break;
    default:
        OSL_FAIL("this cannot happen");
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
            aAppName = rtl::OUString( "writer" );
        else if ( rCmdLine.IsCalc() )
            aAppName = rtl::OUString( "calc" );
        else if ( rCmdLine.IsDraw() )
            aAppName = rtl::OUString( "draw" );
        else if ( rCmdLine.IsImpress() )
            aAppName = rtl::OUString( "impress" );
        else if ( rCmdLine.IsBase() )
            aAppName = rtl::OUString( "base" );
        else if ( rCmdLine.IsGlobal() )
            aAppName = rtl::OUString( "global" );
        else if ( rCmdLine.IsMath() )
            aAppName = rtl::OUString( "math" );
        else if ( rCmdLine.IsWeb() )
            aAppName = rtl::OUString( "web" );

        // Which splash to use
        OUString aSplashService( "com.sun.star.office.SplashScreen" );
        if ( rCmdLine.HasSplashPipe() )
            aSplashService = OUString("com.sun.star.office.PipeSplashScreen");

        bVisible = sal_True;
        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= bVisible;
        aSeq[1] <<= aAppName;
        m_rSplashScreen = Reference<XStatusIndicator>(
            comphelper::getProcessServiceFactory()->createInstanceWithArguments(
            aSplashService, aSeq), UNO_QUERY);

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

void Desktop::SetSplashScreenText( const ::rtl::OUString& rText )
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
        Reference< XJobExecutor > xExecutor( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString("com.sun.star.task.JobExecutor") ), UNO_QUERY );
        if( xExecutor.is() )
            xExecutor->trigger( ::rtl::OUString("onFirstRunInitialization") );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        OSL_FAIL( "Desktop::DoFirstRunInitializations: caught an exception while trigger job executor ..." );
    }
}

void Desktop::ShowBackingComponent(Desktop * progress)
{
    if (GetCommandLineArgs().IsNoDefault())
    {
        return;
    }
    Reference< XMultiServiceFactory > xSMgr(
        comphelper::getProcessServiceFactory(), UNO_SET_THROW);
    Reference< XFrame > xDesktopFrame(
        xSMgr->createInstance("com.sun.star.frame.Desktop"), UNO_QUERY);
    if (xDesktopFrame.is())
    {
        if (progress != 0)
        {
            progress->SetSplashScreenProgress(60);
        }
        Reference< XFrame > xBackingFrame;
        Reference< ::com::sun::star::awt::XWindow > xContainerWindow;

        xBackingFrame = xDesktopFrame->findFrame(OUString( "_blank" ), 0);
        if (xBackingFrame.is())
            xContainerWindow = xBackingFrame->getContainerWindow();
        if (xContainerWindow.is())
        {
            SetDocumentExtendedStyle(xContainerWindow);
            if (progress != 0)
            {
                progress->SetSplashScreenProgress(75);
            }
            Sequence< Any > lArgs(1);
            lArgs[0] <<= xContainerWindow;

            Reference< XController > xBackingComp(
                xSMgr->createInstanceWithArguments(OUString( "com.sun.star.frame.StartModule" ), lArgs), UNO_QUERY);
            if (xBackingComp.is())
            {
                Reference< ::com::sun::star::awt::XWindow > xBackingWin(xBackingComp, UNO_QUERY);
                // Attention: You MUST(!) call setComponent() before you call attachFrame().
                // Because the backing component set the property "IsBackingMode" of the frame
                // to true inside attachFrame(). But setComponent() reset this state everytimes ...
                xBackingFrame->setComponent(xBackingWin, xBackingComp);
                if (progress != 0)
                {
                    progress->SetSplashScreenProgress(100);
                }
                xBackingComp->attachFrame(xBackingFrame);
                if (progress != 0)
                {
                    progress->CloseSplashScreen();
                }
                xContainerWindow->setVisible(sal_True);
            }
        }
    }
}

// ========================================================================
void Desktop::CheckFirstRun( )
{
    const ::rtl::OUString sCommonMiscNodeName("/org.openoffice.Office.Common/Misc");
    const ::rtl::OUString sFirstRunNodeName("FirstRun");

    // --------------------------------------------------------------------
    // check if this is the first office start

    // for this, open the Common/Misc node where this info is stored
    ::utl::OConfigurationTreeRoot aCommonMisc = ::utl::OConfigurationTreeRoot::createWithServiceFactory(
        ::comphelper::getProcessServiceFactory( ),
        sCommonMiscNodeName,
        2,
        ::utl::OConfigurationTreeRoot::CM_UPDATABLE
    );

    // read the flag
    OSL_ENSURE( aCommonMisc.isValid(), "Desktop::CheckFirstRun: could not open the config node needed!" );
    sal_Bool bIsFirstRun = sal_False;
    aCommonMisc.getNodeValue( sFirstRunNodeName ) >>= bIsFirstRun;

    if ( !bIsFirstRun )
        // nothing to do ....
        return;

    // --------------------------------------------------------------------
    // it is the first run
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
            sal_Bool bQuickstart( sal_True );
            sal_Bool bAutostart( sal_True );
            Sequence< Any > aSeq( 2 );
            aSeq[0] <<= bQuickstart;
            aSeq[1] <<= bAutostart;

            Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                OUString::createFromAscii( "com.sun.star.office.Quickstart" )),UNO_QUERY );
            if ( xQuickstart.is() )
                xQuickstart->initialize( aSeq );
            RegCloseKey( hKey );
        }
    }
#endif

    // --------------------------------------------------------------------
    // reset the config flag

    // set the value
    aCommonMisc.setNodeValue( sFirstRunNodeName, makeAny( (sal_Bool)sal_False ) );
    // commit the changes
    aCommonMisc.commit();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
