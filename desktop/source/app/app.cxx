/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>
#include <config_folders.h>

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

#include <svl/languageoptions.hxx>
#include <svtools/javacontext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/theAutoRecovery.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
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
#include <com/sun/star/task/theJobExecutor.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/office/Quickstart.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <officecfg/Setup.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/uri.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/menuoptions.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
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
#endif 

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

namespace {

void removeTree(OUString const & url) {
    osl::Directory dir(url);
    osl::FileBase::RC rc = dir.open();
    switch (rc) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return; 
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
        if (stat.getFileType() == osl::FileStatus::Directory) { 
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



//







//




//







//



//


bool cleanExtensionCache() {
    OUString buildId(
        "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(buildId); 
    OUString extDir(
        "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap")
        ":UserInstallation}/user/extensions");
    rtl::Bootstrap::expandMacros(extDir); 
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
    rtl::Bootstrap::expandMacros(userRcFile); 
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

bool shouldLaunchQuickstart()
{
    bool bQuickstart = Desktop::GetCommandLineArgs().IsQuickstart();
    if (!bQuickstart)
    {
        const SfxPoolItem* pItem=0;
        SfxItemSet aQLSet(SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER);
        SFX_APP()->GetOptions(aQLSet);
        SfxItemState eState = aQLSet.GetItemState(SID_ATTR_QUICKLAUNCHER, false, &pItem);
        if (SFX_ITEM_SET == eState)
            bQuickstart = ((SfxBoolItem*)pItem)->GetValue();
    }
    return bQuickstart;
}

void SetRestartState() {
    try {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
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
                boost::shared_ptr< comphelper::ConfigurationChanges > batch(
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
        
        
        if ( Application::IsInExecute() )
            Desktop::pResMgr = ResMgr::CreateResMgr("dkt");

        if ( !Desktop::pResMgr )
        {
            
            
            
            OUString aUILocaleString = langselect::getEmergencyLocale();
            LanguageTag aLanguageTag( aUILocaleString);
            
            Desktop::pResMgr = ResMgr::SearchCreateResMgr( "dkt", aLanguageTag);
            AllSettings as = GetSettings();
            as.SetUILanguageTag(aLanguageTag);
            SetSettings(as);
        }
    }

    return Desktop::pResMgr;
}

namespace {





OUString GetMsgString(
    sal_uInt16 nId, const OUString& aFallbackMsg,
    bool bAlwaysUseFallbackMsg = false )
{
    if ( !bAlwaysUseFallbackMsg )
    {
        ResMgr* resMgr = Desktop::GetDesktopResManager();
        if ( resMgr )
            return ResId(nId, *resMgr).toString();
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
        aDiagnosticMessage.append( ResId(STR_BOOTSTRAP_ERR_CFG_DATAACCESS, *pResMgr).toString() );
    else
        aDiagnosticMessage.appendAscii( "The program cannot be started." );

    if ( !aInternalErrMsg.isEmpty() )
    {
        aDiagnosticMessage.appendAscii( "\n\n" );
        if ( pResMgr )
            aDiagnosticMessage.append( ResId(STR_INTERNAL_ERRMSG, *pResMgr).toString() );
        else
            aDiagnosticMessage.appendAscii( "The following internal error has occurred:\n\n" );
        aDiagnosticMessage.append( aInternalErrMsg );
    }

    return aDiagnosticMessage.makeStringAndClear();
}











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
        : public rtl::Static< OUString, BrandName > {};
    struct Version
        : public rtl::Static< OUString, Version > {};
    struct AboutBoxVersion
        : public rtl::Static< OUString, AboutBoxVersion > {};
    struct AboutBoxVersionSuffix
        : public rtl::Static< OUString, AboutBoxVersionSuffix > {};
    struct OOOVendor
        : public rtl::Static< OUString, OOOVendor > {};
    struct Extension
        : public rtl::Static< OUString, Extension > {};
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

        if ( sBrandName.isEmpty() )
        {
            sBrandName = utl::ConfigManager::getProductName();
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
    }

    if ( true )
    {
        const CommandLineArgs& rCmdLineArgs = GetCommandLineArgs();

        
        SAL_INFO( "desktop.app",  "desktop (cd100003) ::OfficeIPCThread::EnableOfficeIPCThread" );
        OfficeIPCThread::Status aStatus = OfficeIPCThread::EnableOfficeIPCThread();
        if ( aStatus == OfficeIPCThread::IPC_STATUS_PIPE_ERROR )
        {
#if HAVE_FEATURE_MACOSX_SANDBOX
            
            
            
            
            
            
            
#elif defined ANDROID
            
#else
            
            
            
            
            
            
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
#endif
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING, OUString() );
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_2ND_OFFICE )
        {
            
            SetBootstrapStatus(BS_TERMINATE);
        }
        else if ( !rCmdLineArgs.GetUnknown().isEmpty()
                  || rCmdLineArgs.IsHelp() || rCmdLineArgs.IsVersion() )
        {
            
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
        
        SAL_INFO( "desktop.app", "<- store config items" );
        utl::ConfigManager::storeConfigItems();
        FlushConfiguration();
        SAL_INFO( "desktop.app", "<- store config items" );

        
        CloseSplashScreen();
        Reference< XComponent >(
            comphelper::getProcessComponentContext(), UNO_QUERY_THROW )->
            dispose();
        
        ::comphelper::setProcessServiceFactory( NULL );

        
        m_xLockfile.reset();

        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            osl_removeSignalHandler( pSignalHandler );
    } catch (const RuntimeException&) {
        
        
    }

    SAL_INFO( "desktop.app", "FINISHED WITH Destop::DeInit" );
}

bool Desktop::QueryExit()
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


OUString    Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const OUString& aFileURL )
{
    OUString        aMsg;
    OUString        aFilePath;
    sal_Bool        bFileInfo = sal_True;

    switch ( nFailureCode )
    {
        
        case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_PATH_INVALID,
                        OUString( "The installation path is not available." ) );
            bFileInfo = sal_False;
        }
        break;

        
        case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( "The configuration file \"$1\" is missing." ) );
        }
        break;

        
        
         case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
         case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_CORRUPT,
                        OUString( "The configuration file \"$1\" is corrupt." ) );
        }
        break;

        
        case ::utl::Bootstrap::MISSING_VERSION_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( "The configuration file \"$1\" is missing." ) );
        }
        break;

        
         case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SUPPORT,
                        OUString( "The main configuration file \"$1\" does not support the current version." ) );
        }
        break;

        
           case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_DIR_MISSING,
                        OUString( "The configuration directory \"$1\" is missing." ) );
        }
        break;

        
        case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
                        OUString( "An internal failure occurred." ) );
            bFileInfo = sal_False;
        }
        break;

        case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
        {
            
            aMsg = "Invalid version file entry";
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

                
                
                
                case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
                case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
                case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
                {
                    OUString aBootstrapFileURL;

                    utl::Bootstrap::locateBootstrapFile( aBootstrapFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aBootstrapFileURL );
                }
                break;

                
                
                
                 case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE:
                {
                    OUString aVersionFileURL;

                    utl::Bootstrap::locateVersionFile( aVersionFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aVersionFileURL );
                }
                break;

                
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
        
        

        
        
        
        
        
        
        
        std::cerr
            << "The application cannot be started.\n"
                
            << (aBootstrapError == BE_UNO_SERVICEMANAGER
                ? "The component manager is not available.\n"
                    
                : "The configuration service is not available.\n");
                    
        if ( !aErrorMessage.isEmpty() )
        {
            std::cerr << "(\"" << aErrorMessage << "\")\n";
        }

        
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
        OUString msg(
            GetMsgString(
                STR_CONFIG_ERR_ACCESS_GENERAL,
                ("A general error occurred while accessing your central"
                 " configuration.")));
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


sal_Bool Desktop::isUIOnSessionShutdownAllowed()
{
    return officecfg::Office::Recovery::SessionShutdown::DocumentStoreUIEnabled
        ::get();
}


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


/*  @short  start the recovery wizard.

    @param  bEmergencySave
            differs between EMERGENCY_SAVE and RECOVERY
*/
sal_Bool impl_callRecoveryUI(sal_Bool bEmergencySave     ,
                             sal_Bool bExistsRecoveryData)
{
    static OUString SERVICENAME_RECOVERYUI("com.sun.star.comp.svx.RecoveryUI");
    static OUString COMMAND_EMERGENCYSAVE("vnd.sun.star.autorecovery:/doEmergencySave");
    static OUString COMMAND_RECOVERY("vnd.sun.star.autorecovery:/doAutoRecovery");

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
 * the next time the application is started
 *
 * returns sal_True if at least one document could be saved...
 *
 */

sal_Bool Desktop::SaveTasks()
{
    return impl_callRecoveryUI(
        sal_True , 
        sal_False);
}

namespace {

void restartOnMac(bool passArguments) {
#if defined MACOSX
    OfficeIPCThread::DisableOfficeIPCThread();
#if HAVE_FEATURE_MACOSX_SANDBOX
    (void) passArguments; 
    ResMgr *resMgr = Desktop::GetDesktopResManager();
    OUString aMessage = ResId(STR_LO_MUST_BE_RESTARTED, *resMgr).toString();

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
    if (errno == ENOTSUP) { 
        pid_t pid = fork();
        if (pid == 0) {
            execv(execPath8.getStr(), const_cast< char ** >(&argPtrs[0]));
        } else if (pid > 0) {
            
            
            
            
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
    (void) passArguments; 
#endif
}

}

sal_uInt16 Desktop::Exception(sal_uInt16 nError)
{
    
    static sal_Bool bInException = sal_False;

    sal_uInt16 nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    Application::SetDefDialogParent( NULL );

    if ( bInException )
    {
        OUString aDoubleExceptionString;
        Application::Abort( aDoubleExceptionString );
    }

    bInException = sal_True;
    const CommandLineArgs& rArgs = GetCommandLineArgs();

    
    sal_Bool bRestart                           = sal_False;
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !rArgs.IsNoRestore()                    ) && 
                                                    ( !rArgs.IsHeadless()                     ) &&
                                                    (( nError & EXC_MAJORTYPE ) != EXC_DISPLAY ) && 
                                                    ( Application::IsInExecute()               )    
                                                  );
    if ( bAllowRecoveryAndSessionManagement )
        bRestart = SaveTasks();

    FlushConfiguration();

    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
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
                Application::Abort( OUString() );
            }

            break;
        }
    }

    OSL_ASSERT(false); 
    return 0;
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
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
    
    ConfigurationErrorHandler aConfigErrHandler;
    if (!ShouldSuppressUI(rCmdLineArgs))
        aConfigErrHandler.activate();

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    
    SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main { OpenSplashScreen" );
    OpenSplashScreen();
    SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main } OpenSplashScreen" );

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
    
    utl::Bootstrap::reloadData();
    SetSplashScreenProgress(20);

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< XRestartManager > xRestartManager( OfficeRestartManager::get(xContext) );

    Reference< XDesktop2 > xDesktop;
    try
    {
        RegisterServices(xContext);

        SetSplashScreenProgress(25);

        
        
        SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main -> Lockfile" );
        m_xLockfile.reset(new Lockfile);

#if HAVE_FEATURE_DESKTOP
        if ( !rCmdLineArgs.IsHeadless() && !rCmdLineArgs.IsInvisible() &&
             !rCmdLineArgs.IsNoLockcheck() && !m_xLockfile->check( Lockfile_execWarning ))
        {
            
            return EXIT_FAILURE;
        }
        SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::Main <- Lockfile" );

        
        SAL_INFO( "desktop.app", "{ GetEnableATToolSupport" );
        if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
        {
            if( !InitAccessBridge() )
                return EXIT_FAILURE;
        }
        SAL_INFO( "desktop.app", "} GetEnableATToolSupport" );
#endif

        
        if( rCmdLineArgs.IsTerminateAfterInit() )
            return EXIT_SUCCESS;

        
        if ( !InitializeConfiguration() )
            return EXIT_FAILURE;

        SetSplashScreenProgress(30);

        
        osl_setErrorReporting( false );

        
        LanguageTag aLocale( LANGUAGE_SYSTEM);
        ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( "ofa", aLocale );
        OUString aTitle = pLabelResMgr ? ResId(RID_APPTITLE, *pLabelResMgr).toString() : OUString();
        delete pLabelResMgr;

#ifdef DBG_UTIL
        
        OUString aDefault("development");
        aTitle += " [";
        aTitle += utl::Bootstrap::getBuildIdData(aDefault);
        aTitle += "]";
#endif

        SetDisplayName( aTitle );
        SetSplashScreenProgress(35);
        SAL_INFO( "desktop.app", "{ create SvtPathOptions and SvtLanguageOptions" );
        pExecGlobals->pPathOptions.reset( new SvtPathOptions);
        SetSplashScreenProgress(40);
        SAL_INFO( "desktop.app", "} create SvtPathOptions and SvtLanguageOptions" );

        xDesktop = css::frame::Desktop::create( xContext );

        
        pExecGlobals->xGlobalBroadcaster = Reference < css::document::XEventListener >
            ( css::frame::theGlobalEventBroadcaster::get(xContext), UNO_QUERY_THROW );

        /* ensure existance of a default window that messages can be dispatched to
           This is for the benefit of testtool which uses PostUserEvent extensively
           and else can deadlock while creating this window from another tread while
           the main thread is not yet in the event loop.
        */
        Application::GetDefaultDevice();

#if HAVE_FEATURE_EXTENSIONS
        
        
        
        SynchronizeExtensionRepositories();
        bool bAbort = CheckExtensionDependencies();
        if ( bAbort )
            return EXIT_FAILURE;

        if (inst_fin == userinstall::CREATED)
        {
            Migration::migrateSettingsIfNecessary();
        }
#endif

        
        pExecGlobals->pLanguageOptions.reset( new SvtLanguageOptions(true));

        css::document::EventObject aEvent;
        aEvent.EventName = "OnStartApp";
        pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);

        SetSplashScreenProgress(50);

        
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

        
        SetSplashScreenProgress(75);

        
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

        
        int nAcquireCount = Application::ReleaseSolarMutex();

        
        
        
        Application::PostUserEvent( LINK( this, Desktop, OpenClients_Impl ) );

        
        Application::PostUserEvent( LINK( this, Desktop, EnableAcceptors_Impl) );

        
        aConfigErrHandler.deactivate();

       
        if ( nAcquireCount )
            Application::AcquireSolarMutex( nAcquireCount );

        
        SAL_INFO( "desktop.app", "PERFORMANCE - enter Application::Execute()" );

        try
        {
#if HAVE_FEATURE_JAVA
            
            
            com::sun::star::uno::ContextLayer layer2(
                new svt::JavaContext( com::sun::star::uno::getCurrentContext() ) );
#endif
            
            pExecGlobals->bRestartRequested = pExecGlobals->bRestartRequested ||
                xRestartManager->isRestartRequested(true);

            if ( !pExecGlobals->bRestartRequested )
            {
                
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
            FatalError( OUString( "Caught Unknown Exception: Aborting!"));
        }
    }
    else
    {
        if (xDesktop.is())
            xDesktop->terminate();
    }
    
    
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
        aEvent.EventName = "OnCloseApp";
        pExecGlobals->xGlobalBroadcaster->notifyEvent(aEvent);
    }

    delete pResMgr, pResMgr = NULL;
    
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

    
    RemoveTemporaryDirectory();
    FlushConfiguration();
    
    
    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    DeregisterServices();
    Application::AcquireSolarMutex(nAcquireCount);
    
    
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
        
        
        
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( const com::sun::star::configuration::backend::BackendSetupException& exception)
    {
        
        
        
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

sal_Bool Desktop::InitializeQuickstartMode( const Reference< XComponentContext >& rxContext )
{
    try
    {
        
        
        
        SAL_INFO( "desktop.app", "desktop (cd100003) createInstance com.sun.star.office.Quickstart" );

        sal_Bool bQuickstart = shouldLaunchQuickstart();

        
        

        
        
        
        
        
        
        
        
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

void Desktop::OverrideSystemSettings( AllSettings& rSettings )
{
    if ( !SvtTabAppearanceCfg::IsInitialized () )
        return;

    StyleSettings hStyleSettings   = rSettings.GetStyleSettings();
    MouseSettings hMouseSettings = rSettings.GetMouseSettings();

    sal_uInt32 nDragFullOptions = hStyleSettings.GetDragFullOptions();

    SvtTabAppearanceCfg aAppearanceCfg;
    sal_uInt16 nDragMode = aAppearanceCfg.GetDragMode();
    switch ( nDragMode )
    {
    case DragFullWindow:
        nDragFullOptions |= DRAGFULL_OPTION_ALL;
        break;
    case DragFrame:
        nDragFullOptions &= ((sal_uInt32)~DRAGFULL_OPTION_ALL);
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


IMPL_LINK_NOARG(Desktop, AsyncInitFirstRun)
{
    DoFirstRunInitializations();
    return 0L;
}



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
        OUString a( "UNO exception during client open:\n"  );
        Application::Abort( a + e.Message );
    }
    return 0;
}


IMPL_LINK_NOARG(Desktop, EnableAcceptors_Impl)
{
    enableAcceptors();
    return 0;
}



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
    args[0].Name = "Hidden";
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
    Reference< XNameAccess > xNameAccess = css::frame::theUICommandDescription::get(xContext);

    OUString aWriterDoc( "com.sun.star.text.TextDocument" );
    OUString aCalcDoc( "com.sun.star.sheet.SpreadsheetDocument" );
    OUString aDrawDoc( "com.sun.star.drawing.DrawingDocument" );
    OUString aImpressDoc( "com.sun.star.presentation.PresentationDocument" );

    
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
        
        a = xNameAccess->getByName( aDrawDoc );
        a >>= xCmdAccess;
        if ( xCmdAccess.is() )
            xCmdAccess->getByName(".uno:Polygon");
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    
    xNameAccess = theWindowStateConfiguration::get( xContext );
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

    
    Sequence< Sequence< css::beans::PropertyValue > > aSeqSeqPropValue;
    Reference< XUIElementFactoryManager > xUIElementFactory = theUIElementFactoryManager::get( xContext );
    try
    {
        aSeqSeqPropValue = xUIElementFactory->getRegisteredFactories();
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    
    

    Reference< css::frame::XUIControllerRegistration > xPopupMenuControllerFactory =
    css::frame::thePopupMenuControllerFactory::get( xContext );
    try
    {
        xPopupMenuControllerFactory->hasController(
                    OUString( ".uno:CharFontName" ),
                    OUString() );
    }
    catch ( const ::com::sun::star::uno::Exception& )
    {
    }

    
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

    
    
    Reference < XComponent > xFirst;
    bool bRecovery = false;

    const CommandLineArgs& rArgs = GetCommandLineArgs();

    if (!rArgs.IsQuickstart())
    {
        sal_Bool bShowHelp = sal_False;
        OUStringBuffer aHelpURLBuffer;
        if (rArgs.IsHelpWriter()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpCalc()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpDraw()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpImpress()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpBase()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpBasic()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
        } else if (rArgs.IsHelpMath()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help:
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

    
    
    
    
    
    
    
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !rArgs.IsNoRestore() ) &&
                                                    ( !rArgs.IsHeadless()  )
                                                  );

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
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);

        if ( !getenv ("OOO_DISABLE_RECOVERY") &&
            (
                ( bExistsRecoveryData ) || 
                ( bCrashed            )    
            )
           )
        {
            try
            {
                bRecovery = impl_callRecoveryUI(
                    sal_False          , 
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
            
            xSessionListener = SessionListener::createWithOnQuitFlag(
                    ::comphelper::getProcessComponentContext(), isUIOnSessionShutdownAllowed());
        }
        catch(const com::sun::star::uno::Exception& e)
        {
            SAL_WARN( "desktop.app", "Registration of session listener failed" << e.Message);
        }

        if ( !bExistsRecoveryData && xSessionListener.is() )
        {
            
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

            
            if ( rArgs.IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_WRITER );
            else if ( rArgs.IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_CALC );
            else if ( rArgs.IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_IMPRESS );
            else if ( rArgs.IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_DRAW );
        }

        
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

        
        if ( OfficeIPCThread::ExecuteCmdLineRequests( aRequest ) )
        {
            
            return;
        }
    }

    
    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference< XElementAccess > xList( xDesktop->getFrames(), UNO_QUERY_THROW );
    if ( xList->hasElements() )
        return;

    if ( rArgs.IsQuickstart() || rArgs.IsInvisible() || Application::AnyInput( VCL_INPUT_APPEVENT ) )
        
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


OUString GetURL_Impl(
    const OUString& rName, boost::optional< OUString > const & cwdUrl )
{
    
    
    if (rName.startsWith("vnd.sun.star.script"))
    {
        return rName;
    }

    
    
    if (rName.startsWith("file:"))
    {
        return rName;
    }

    if ( rName.startsWith("service:"))
    {
        return rName;
    }

    
    
    
    
    
    INetURLObject aObj;
    if (cwdUrl) {
        aObj.SetURL(*cwdUrl);
        aObj.setFinalSlash();
    }

    
    
    bool bWasAbsolute;
    INetURLObject aURL     = aObj.smartRel2Abs( rName, bWasAbsolute, false, INetURLObject::WAS_ENCODED,
                                                RTL_TEXTENCODING_UTF8, true );
    OUString      aFileURL = aURL.GetMainURL(INetURLObject::NO_DECODE);

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
        
        
        createAcceptor(rAppEvent.GetStringData());
        break;
    case ApplicationEvent::TYPE_APPEAR:
        if ( !GetCommandLineArgs().IsInvisible() )
        {
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            
            Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );
            Reference< css::frame::XFrame > xTask = xDesktop->getActiveFrame();
            if ( !xTask.is() )
            {
                
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
                
                Reference< ::com::sun::star::awt::XWindow > xContainerWindow;
                Reference< XFrame > xBackingFrame = xDesktop->findFrame(OUString( "_blank" ), 0);
                if (xBackingFrame.is())
                    xContainerWindow = xBackingFrame->getContainerWindow();
                if (xContainerWindow.is())
                {
                    Reference< XController > xStartModule = StartModule::createWithParentWindow(xContext, xContainerWindow);
                    Reference< ::com::sun::star::awt::XWindow > xBackingWin(xStartModule, UNO_QUERY);
                    
                    
                    
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
            
            
            
            
            
            Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            css::office::Quickstart::createStart(xContext, true/*Quickstart*/);
        }
        break;
    case ApplicationEvent::TYPE_SHOWDIALOG:
        
        
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

        
        OUString aSplashService( "com.sun.star.office.SplashScreen" );
        if ( rCmdLine.HasSplashPipe() )
            aSplashService = "com.sun.star.office.PipeSplashScreen";

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


void Desktop::DoFirstRunInitializations()
{
    try
    {
        Reference< XJobExecutor > xExecutor = theJobExecutor::get( ::comphelper::getProcessComponentContext() );
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
        
        
        
        Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        SAL_WARN_IF( !pContainerWindow, "desktop.app", "Desktop::Main: no implementation access to the frame's container window!" );
        pContainerWindow->SetExtendedStyle( pContainerWindow->GetExtendedStyle() | WB_EXT_DOCUMENT );
        if (progress != 0)
        {
            progress->SetSplashScreenProgress(75);
        }

        Reference< XController > xStartModule = StartModule::createWithParentWindow( xContext, xContainerWindow);
        
        
        
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


void Desktop::CheckFirstRun( )
{
    if (officecfg::Office::Common::Misc::FirstRun::get())
    {
        
        
        
        
        m_firstRunTimer.SetTimeout(3000); 
        m_firstRunTimer.SetTimeoutHdl(LINK(this, Desktop, AsyncInitFirstRun));
        m_firstRunTimer.Start();

#ifdef WNT
        
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
