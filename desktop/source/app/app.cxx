/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.80 $
 *
 *  last change: $Author: as $ $Date: 2002-05-24 11:24:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <unistd.h>
#include "app.hxx"
#include "desktop.hrc"
#include "appinit.hxx"
#include "intro.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "officeacceptthread.hxx"
#include "pluginacceptthread.hxx"
#include "desktopresid.hxx"
#include "dispatchwatcher.hxx"

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_MISSINGBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INSTALLATIONINCOMPLETEEXCEPTION_HPP_
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <com/sun/star/beans/XMaterialHolder.hpp>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#ifndef _VOS_TIMER_HXX_
#include <vos/timer.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef AUTOMATION_HXX
#include <automation/automation.hxx>
#endif
#ifndef _Installer_hxx
#include <setup2/installer.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _VOS_PROFILE_HXX_
#include <vos/profile.hxx>
#endif

#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII##))
#define U2S(STRING)                             ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

using namespace vos;
using namespace rtl;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;

static SalMainPipeExchangeSignalHandler* pSignalHandler = 0;

OOfficeAcceptorThread*      pOfficeAcceptThread = 0;
ResMgr*                     Desktop::pResMgr    = 0;
static PluginAcceptThread*  pPluginAcceptThread = 0;
static oslModule            aTestToolModule     = 0;

// ----------------------------------------------------------------------------

char const INSTALLER_INITFILENAME[] = "initialize.ini";
char const INSTALLMODE_STANDALONE[] = "STANDALONE";
char const INSTALLMODE_NETWORK[]    = "NETWORK";
char const INSTALLMODE_ALLUSERS[]   = "ALL_USERS";
#ifndef BUILD_SOSL

// include strings for copyright notice
#include "copyright_ascii.txt"

char const OEM_PRELOAD_SECTION[]    = "Bootstrap";
char const OEM_PRELOAD[]            = "Preload";
#endif

// ----------------------------------------------------------------------------

#ifndef BUILD_SOSL
sal_Bool IsOEMPreload()
{
    if ( !Application::IsRemoteServer() )
    {
        OUString aSofficeIniFileURL;

        Bootstrap().getIniName( aSofficeIniFileURL );
        if ( aSofficeIniFileURL.getLength() > 0 )
        {
            OProfile aProfile( aSofficeIniFileURL );
            sal_Bool bResult = aProfile.readBool( OEM_PRELOAD_SECTION, OEM_PRELOAD, sal_False );
            aProfile.close();

            return bResult;
        }
    }

    return sal_False;
}

// reset preload flag in soffice.ini/sofficerc file
void ResetOEMPreload()
{
    OUString aSofficeIniFileURL;

    Bootstrap().getIniName( aSofficeIniFileURL );
    if ( aSofficeIniFileURL.getLength() > 0 )
    {
        OProfile aProfile( aSofficeIniFileURL );
        aProfile.writeBool( OEM_PRELOAD_SECTION, OEM_PRELOAD, sal_False );
        aProfile.flush();
        aProfile.close();
    }
}

// check OEM preload
sal_Bool CheckOEMPreload()
{
    try
    {
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();

        // create OEM preload service
        Reference < com::sun::star::ui::dialogs::XExecutableDialog > xDialog( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.preload.OEMPreloadWizard" ) ) ), UNO_QUERY );

        if ( xDialog.is() )
        {
            // execute OEM preload dialog and check return value
            sal_Int16 nResult = xDialog->execute();
            if ( nResult != com::sun::star::ui::dialogs::ExecutableDialogResults::OK )
                return sal_False;
            else
                ResetOEMPreload();
        }
    }
    catch ( RuntimeException& )
    {
    }

    return sal_True;
}

#endif

// ----------------------------------------------------------------------------

void InitTestToolLib()
{
#ifndef BUILD_SOSL
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::InitTestToolLib" );

    OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "CreateRemoteControl" ));
    OUString    aModulePath;

    ::vos::OStartupInfo().getExecutableFile( aModulePath );
    sal_uInt32  lastIndex = aModulePath.lastIndexOf('/');
    if ( lastIndex > 0 )
        aModulePath = aModulePath.copy( 0, lastIndex+1 );

    aModulePath += OUString::createFromAscii( SVLIBRARY( "sts" ) );

    // Shortcut for Performance: We expect that the test tool library is not installed
    // (only for testing purpose). It should be located beside our executable.
    // We don't want to pay for searching through LD_LIBRARY_PATH so we check for
    // existence only in our executable path!!
    osl::DirectoryItem  aItem;
    osl::FileBase::RC   nResult = osl::DirectoryItem::get( aModulePath, aItem );

    if ( nResult == osl::FileBase::E_None )
    {
        aTestToolModule = osl_loadModule( aModulePath.pData, SAL_LOADMODULE_DEFAULT );
        if ( aTestToolModule )
        {
            void* pInitFunc = osl_getSymbol( aTestToolModule, aFuncName.pData );
            if ( pInitFunc )
                (*(pfunc_CreateRemoteControl)pInitFunc)();
        }
    }
#endif
}

void DeInitTestToolLib()
{
#ifndef BUILD_SOSL
    if ( aTestToolModule )
    {
        OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "DestroyRemoteControl" ));

        void* pDeInitFunc = osl_getSymbol( aTestToolModule, aFuncName.pData );
        if ( pDeInitFunc )
            (*(pfunc_DestroyRemoteControl)pDeInitFunc)();

        osl_unloadModule( aTestToolModule );
    }
#endif
}

ResMgr* Desktop::GetDesktopResManager()
{
    if ( !Desktop::pResMgr )
    {
        String aMgrName = String::CreateFromAscii( "dkt" );
        aMgrName += String::CreateFromInt32(SOLARUPD); // current version number

        // Create desktop resource manager and bootstrap process
        // was successful. Use default way to get language specific message.
        if ( Application::IsInExecute() )
            Desktop::pResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));

        if ( !Desktop::pResMgr )
        {
            // Use VCL to get the correct language specific message as we
            // are in the bootstrap process and not able to get the installed
            // language!!
            LanguageType aLanguageType = LANGUAGE_DONTKNOW;

            Desktop::pResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLanguageType );
        }
    }

    return Desktop::pResMgr;
}

// ----------------------------------------------------------------------------
// Get a message string securely. There is a fault back string if the resource
// is not available.

OUString Desktop::GetMsgString( USHORT nId, const OUString& aFaultBackMsg )
{
    ResMgr* pResMgr = GetDesktopResManager();
    if ( !pResMgr )
        return aFaultBackMsg;
    else
        return OUString( ResId( nId, pResMgr ));
}

CommandLineArgs* GetCommandLineArgs()
{
    static CommandLineArgs* pArgs = 0;
    if ( !pArgs )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pArgs )
            pArgs = new CommandLineArgs( ::vos::OExtCommandLine() );
    }

    return pArgs;
}

BOOL InitializeInstallation( const UniString& rAppFilename )
{
    UniString aAppPath( rAppFilename );
    rtl::OUString aFinishInstallation;
    osl::FileBase::getFileURLFromSystemPath( aAppPath, aFinishInstallation );
    aAppPath = UniString( aFinishInstallation );

    xub_StrLen nPos = aAppPath.SearchBackward( '/' );
    aAppPath.Erase( nPos );
    aAppPath += '/';
    aAppPath += DEFINE_CONST_UNICODE( INSTALLER_INITFILENAME );

    osl::DirectoryItem aDI;
    if( osl::DirectoryItem::get( aAppPath, aDI ) == osl_File_E_None )
    {
        // Load initialization code only on demand. This is done if the the 'initialize.ini'
        // is written next to the executable. After initialization this file is removed.
        // The implementation disposes the old service manager and creates an new one so we
        // cannot use a service for InitializeInstallation!!
        OUString aFuncName( RTL_CONSTASCII_USTRINGPARAM( INSTALLER_INITIALIZEINSTALLATION_CFUNCNAME ));
        OUString aModulePath = OUString::createFromAscii( SVLIBRARY( "set" ) );

        oslModule aSetupModule = osl_loadModule( aModulePath.pData, SAL_LOADMODULE_DEFAULT );
        if ( aSetupModule )
        {
            void* pInitFunc = osl_getSymbol( aSetupModule, aFuncName.pData );
            if ( pInitFunc )
                (*(pfunc_InstallerInitializeInstallation)pInitFunc)( rAppFilename.GetBuffer() );
            osl_unloadModule( aSetupModule );
        }

        return TRUE;
    }

    return FALSE;
}

Desktop aDesktop;

void PreloadConfigTrees()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (dg93727) ::PreloadConfigTrees" );

    // these tree are preloaded to get a faster startup for the office
    Sequence <rtl::OUString> aPreloadPathList(6);
    aPreloadPathList[0] =  rtl::OUString::createFromAscii("org.openoffice.Office.Common");
    aPreloadPathList[1] =  rtl::OUString::createFromAscii("org.openoffice.ucb.Configuration");
    aPreloadPathList[2] =  rtl::OUString::createFromAscii("org.openoffice.Office.Writer");
    aPreloadPathList[3] =  rtl::OUString::createFromAscii("org.openoffice.Office.WriterWeb");
    aPreloadPathList[4] =  rtl::OUString::createFromAscii("org.openoffice.Office.Calc");
    aPreloadPathList[5] =  rtl::OUString::createFromAscii("org.openoffice.Office.Impress");

    Reference< XMultiServiceFactory > xProvider(
            ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")), UNO_QUERY);

    if ( xProvider.is() )
    {
        Any aValue;
        aValue <<= aPreloadPathList;

        Reference < com::sun::star::beans::XPropertySet > (xProvider, UNO_QUERY)->setPropertyValue(rtl::OUString::createFromAscii("PrefetchNodes"), aValue );
    }
    else
    {
        aDesktop.HandleBootstrapErrors( Desktop::BE_UNO_SERVICE_CONFIG_MISSING );
    }
}

static String aBrandName;
static String aVersion;
static String aExtension;

void ReplaceStringHookProc( UniString& rStr )
{
    static int nAll = 0, nPro = 0;

    if ( !aBrandName.Len() )
    {
        Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
        rtl::OUString aTmp;
        aRet >>= aTmp;
        aBrandName = aTmp;

        aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTVERSION );
        aRet >>= aTmp;
        aVersion = aTmp;

        if ( !aExtension.Len() )
        {
            aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTEXTENSION );
            aRet >>= aTmp;
            aExtension = aTmp;
        }
    }

    nAll++;
    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
    {
        nPro++;
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", aBrandName );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTVERSION", aVersion );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTEXTENSION", aExtension );
    }
}

/*
BOOL SVMain()
{
    BOOL bInit = InitVCL( Reference < XMultiServiceFactory >() );

    if( bInit )
    {
        GetpApp()->Main();
    }

    DeInitVCL();
    return bInit;
}
*/

Desktop::Desktop() : m_pIntro( 0 ), m_aBootstrapError( BE_OK )
{
    RTL_LOGFILE_TRACE( "desktop (cd100003) ::Desktop::Desktop" );
}

void Desktop::Init()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Init" );

    Reference < XMultiServiceFactory > rSMgr = createApplicationServiceManager();
    if( ! rSMgr.is() )
    {
        SetBootstrapError( BE_UNO_SERVICEMANAGER );
    }

    ::comphelper::setProcessServiceFactory( rSMgr );

    if ( !Application::IsRemoteServer() )
    {
        // start ipc thread only for non-remote offices
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::OfficeIPCThread::EnableOfficeIPCThread" );

        OfficeIPCThread::Status aStatus = OfficeIPCThread::EnableOfficeIPCThread();
        if ( aStatus == OfficeIPCThread::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING );
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_2ND_OFFICE )
        {
            // 2nd office startup should terminate after sending cmdlineargs through pipe
            _exit( 0 );
        }

        pSignalHandler = new SalMainPipeExchangeSignalHandler;
    }
}

void Desktop::DeInit()
{
    Reference<XMultiServiceFactory> xXMultiServiceFactory(::comphelper::getProcessServiceFactory());
    destroyApplicationServiceManager( xXMultiServiceFactory );

    if( !Application::IsRemoteServer() )
    {
        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            DELETEZ( pSignalHandler );
    }
}

BOOL Desktop::QueryExit()
{
    const sal_Char SUSPEND_QUICKSTARTVETO[] = "SuspendQuickstartVeto";

    DBG_ERROR("QueryExit!");
    Reference< ::com::sun::star::frame::XDesktop >
            xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                UNO_QUERY );

    Reference < ::com::sun::star::beans::XPropertySet > xPropertySet( xDesktop, UNO_QUERY );
    if ( xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_True;
        xPropertySet->setPropertyValue( OUSTRING(RTL_CONSTASCII_USTRINGPARAM( SUSPEND_QUICKSTARTVETO )), a );
    }

    BOOL bExit = ( !xDesktop.is() || xDesktop->terminate() );

    if ( !bExit && xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_False;
        xPropertySet->setPropertyValue( OUSTRING(RTL_CONSTASCII_USTRINGPARAM( SUSPEND_QUICKSTARTVETO )), a );
    }

    return bExit;
}

void Desktop::StartSetup( const OUString& aParameters )
{
    OUString aProgName;
    OUString aSysPathFileName;
    OUString aDir;

    ::vos::OStartupInfo aInfo;
    aInfo.getExecutableFile( aProgName );

    sal_uInt32  lastIndex = aProgName.lastIndexOf('/');
    if ( lastIndex > 0 )
    {
        aProgName   = aProgName.copy( 0, lastIndex+1 );
        aDir        = aProgName;

        aProgName   += OUString( RTL_CONSTASCII_USTRINGPARAM( "setup" ));
#ifdef WNT
        aProgName   += OUString( RTL_CONSTASCII_USTRINGPARAM( ".exe" ));
#endif
    }

    OUString                aArgListArray[1];
    ::vos::OSecurity        aSecurity;
    ::vos::OEnvironment     aEnv;
    ::vos::OArgumentList    aArgList;

    aArgListArray[0] = aParameters;
    OArgumentList aArgumentList( aArgListArray, 1 );

    ::vos::OProcess aProcess( aProgName, aDir );
    ::vos::OProcess::TProcessError aProcessError =
        aProcess.execute( OProcess::TOption_Detached,
                          aSecurity,
                          aArgumentList,
                          aEnv );

    if ( aProcessError != OProcess::E_None )
    {
        OUString aMessage( GetMsgString(
            STR_SETUP_ERR_CANNOT_START,
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Couldn't start setup application! Please start it manually." )) ));

        ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
        aBootstrapFailedBox.Execute();
    }
}

void Desktop::HandleBootstrapPathErrors( ::utl::Bootstrap::Status aBootstrapStatus, const OUString& aDiagnosticMessage )
{
    if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
    {
        sal_Bool            bWorkstationInstallation = sal_False;
        ::rtl::OUString     aBaseInstallURL;
        ::rtl::OUString     aUserInstallURL;
        ::rtl::OUString     aProductKey;
        ::rtl::OUString     aTemp;
        ::vos::OStartupInfo aInfo;

        aInfo.getExecutableFile( aProductKey );
        sal_uInt32  lastIndex = aProductKey.lastIndexOf('/');
        if ( lastIndex > 0 )
            aProductKey = aProductKey.copy( lastIndex+1 );

        aTemp = ::utl::Bootstrap::getProductKey( aProductKey );
        if ( aTemp.getLength() > 0 )
            aProductKey = aTemp;

        ::utl::Bootstrap::PathStatus aBaseInstallStatus = ::utl::Bootstrap::locateBaseInstallation( aBaseInstallURL );
        ::utl::Bootstrap::PathStatus aUserInstallStatus = ::utl::Bootstrap::locateUserInstallation( aUserInstallURL );

        if (( aBaseInstallStatus == ::utl::Bootstrap::PATH_EXISTS &&
              aUserInstallStatus == ::utl::Bootstrap::PATH_EXISTS       ))
        {
            if ( aBaseInstallURL != aUserInstallURL )
                bWorkstationInstallation = sal_True;
        }

        if ( Application::IsRemoteServer() )
        {
            OString aTmpStr = OUStringToOString( aDiagnosticMessage, RTL_TEXTENCODING_ASCII_US );
            fprintf( stderr, aTmpStr.getStr() );
        }
        else
        {
            OUString        aMessage;
            OUStringBuffer  aBuffer( 100 );
            aBuffer.append( aDiagnosticMessage );

            aBuffer.appendAscii( "\n" );

            if (( aBootstrapStatus == ::utl::Bootstrap::MISSING_USER_INSTALL ) || bWorkstationInstallation )
            {
                // Check installation mode to suppress error message if we are currently running with a network installation.
                OUString aInstallMode( RTL_CONSTASCII_USTRINGPARAM( INSTALLMODE_STANDALONE ));

                aInstallMode = utl::Bootstrap::getInstallMode( aInstallMode );
                if ( aInstallMode.equalsIgnoreAsciiCaseAscii( INSTALLMODE_NETWORK ) ||
                     aInstallMode.equalsIgnoreAsciiCaseAscii( INSTALLMODE_ALLUSERS ) )
                {
                    // network installation => start setup without error message
                    OUString aParameters;
                    StartSetup( aParameters );
                }
                else
                {
                    OUString aAskSetupStr( GetMsgString(
                        STR_ASK_START_SETUP,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "Start setup application to check installation?" )) ));

                    aBuffer.append( aAskSetupStr );
                    aMessage = aBuffer.makeStringAndClear();

                    ErrorBox aBootstrapFailedBox( NULL, WB_YES_NO, aMessage );
                    aBootstrapFailedBox.SetText( aProductKey );
                    int nResult = aBootstrapFailedBox.Execute();

                    if ( nResult == RET_YES )
                    {
                        OUString aParameters;
                        StartSetup( aParameters );
                    }
                }
            }
            else if (( aBootstrapStatus == utl::Bootstrap::INVALID_USER_INSTALL ) ||
                     ( aBootstrapStatus == utl::Bootstrap::INVALID_BASE_INSTALL )    )
            {
                OUString aAskSetupRepairStr( GetMsgString(
                    STR_ASK_START_SETUP_REPAIR,
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "Start setup application to repair installation?" )) ));

                aBuffer.append( aAskSetupRepairStr );
                aMessage = aBuffer.makeStringAndClear();

                ErrorBox aBootstrapFailedBox( NULL, WB_YES_NO, aMessage );
                aBootstrapFailedBox.SetText( aProductKey );
                int nResult = aBootstrapFailedBox.Execute();

                if ( nResult == RET_YES )
                {
                     OUString aParameters( RTL_CONSTASCII_USTRINGPARAM( "-repair" ));
                    StartSetup( aParameters );
                }
            }
        }

        _exit( 333 );
    }
}

// Create a error message depending on bootstrap failure code and an optional file url
::rtl::OUString Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const ::rtl::OUString& aFileURL )
{
    OUStringBuffer  aDiagnosticMessage( 100 );
    OUString        aMsg;
    OUString        aFilePath;
    sal_Bool        bFileInfo = sal_True;

    // First sentence. We cannot bootstrap office further!
    aDiagnosticMessage.append( GetMsgString( STR_BOOTSTRAP_ERR_CANNOT_START,
                                             OUString( RTL_CONSTASCII_USTRINGPARAM( "The program cannot be started." )) ));
    aDiagnosticMessage.appendAscii( "\n" );

    switch ( nFailureCode )
    {
        /// the shared installation directory could not be located
        case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_PATH_INVALID,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The installation path is not available." )) );
            bFileInfo = sal_False;
        }
        break;

        /// the bootstrap INI file could not be found or read
        case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is missing." )) );
        }
        break;

        /// the bootstrap INI is missing a required entry
        /// the bootstrap INI contains invalid data
         case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
         case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_CORRUPT,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is corrupt." )) );
        }
        break;

        /// the version locator INI file could not be found or read
        case ::utl::Bootstrap::MISSING_VERSION_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is missing." )) );
        }
        break;

        /// the version locator INI has no entry for this version
         case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SUPPORT,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The main configuration file \"$1\" does not support the current version." )) );
        }
        break;

        /// the user installation directory does not exist
           case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_DIR_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration directory \"$1\" is missing." )) );
        }
        break;

        /// some bootstrap data was invalid in unexpected ways
        case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "An internal failure occurred." )) );
            bFileInfo = sal_False;
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

    aDiagnosticMessage.append( aMsg );

    return aDiagnosticMessage.makeStringAndClear();
}

void Desktop::HandleBootstrapErrors( BootstrapError aBootstrapError )
{
    if ( aBootstrapError == BE_PATHINFO_MISSING )
    {
        OUString                    aErrorMsg;
        OUString                    aBuffer;
        utl::Bootstrap::Status      aBootstrapStatus;
        utl::Bootstrap::FailureCode nFailureCode;

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
            }

            HandleBootstrapPathErrors( aBootstrapStatus, aErrorMsg );
        }
    }
    else if ( aBootstrapError == BE_UNO_SERVICEMANAGER || aBootstrapError == BE_UNO_SERVICE_CONFIG_MISSING )
    {
        // Uno service manager is not available. VCL needs a uno service manager to display a message box!!!
        // Currently we are not able to display a message box with a service manager due to this limitations inside VCL.

        if ( Application::IsRemoteServer() )
        {
            OStringBuffer aErrorMsgBuffer( 50 );

            aErrorMsgBuffer.append( "The program cannot be started. " );

            if ( aBootstrapError == BE_UNO_SERVICEMANAGER )
                aErrorMsgBuffer.append( "The service manager is not available.\n" );
            else
                aErrorMsgBuffer.append( "The configuration service is not available.\n" );

            OString aErrorMsg = aErrorMsgBuffer.makeStringAndClear();
            fprintf( stderr, aErrorMsg.getStr() );
        }
        else
        {
            // First sentence. We cannot bootstrap office further!
            OUString            aProductKey;
            OUString            aMessage;
            OUString            aTemp;
            OUStringBuffer      aDiagnosticMessage( 100 );
            ::vos::OStartupInfo aInfo;

            aDiagnosticMessage.append( GetMsgString( STR_BOOTSTRAP_ERR_CANNOT_START,
                                                     OUString( RTL_CONSTASCII_USTRINGPARAM( "The program cannot be started." )) ));
            aDiagnosticMessage.appendAscii( "\n" );

            OUString aErrorMsg;

            if ( aBootstrapError == BE_UNO_SERVICEMANAGER )
                aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SERVICE,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( "The service manager is not available." )) );
            else
                aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_CFG_SERVICE,
                                OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration service is not available." )) );

            aDiagnosticMessage.append( aErrorMsg );
            aDiagnosticMessage.appendAscii( "\n" );

            // Due to the fact the we haven't a backup applicat.rdb file anymore it is not possible to
            // repair the installation with the setup executable besides the office executable. Now
            // we have to ask the user to start the setup on CD/installation directory manually!!
            OUString aStartSetupManually( GetMsgString(
                STR_ASK_START_SETUP_MANUALLY,
                OUString( RTL_CONSTASCII_USTRINGPARAM( "Start setup application to repair the installation from CD, or the folder containing the installation packages." )) ));

            aDiagnosticMessage.append( aStartSetupManually );
            aMessage = aDiagnosticMessage.makeStringAndClear();

            aInfo.getExecutableFile( aProductKey );
            sal_uInt32  lastIndex = aProductKey.lastIndexOf('/');
            if ( lastIndex > 0 )
                aProductKey = aProductKey.copy( lastIndex+1 );

            aTemp = ::utl::Bootstrap::getProductKey( aProductKey );
            if ( aTemp.getLength() > 0 )
                aProductKey = aTemp;

            ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
            aBootstrapFailedBox.SetText( aProductKey );
            aBootstrapFailedBox.Execute();
        }
    }

    _exit( 333 );
}

USHORT Desktop::Exception(USHORT nError)
{
    // protect against recursive calls
    static BOOL bInException = FALSE;

    sal_uInt16 nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    Application::SetDefModalDialogParent( NULL );

    if ( bInException )
    {
        String aDoubleExceptionString;
        Application::Abort( aDoubleExceptionString );
    }

    bInException = TRUE;
    BOOL bRecovery = FALSE;
    CommandLineArgs* pArgs = GetCommandLineArgs();

    // save all modified documents
    if( Application::IsInExecute() )
    {
        // store to backup path
        String aSavePath( SvtPathOptions().GetBackupPath() );
        SvtInternalOptions aOpt;

        // iterate tasks
        Reference< ::com::sun::star::frame::XFramesSupplier >
                xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                UNO_QUERY );
        Reference< ::com::sun::star::frame::XFrame > xTask;
        Reference< ::com::sun::star::container::XIndexAccess > xList( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
        sal_Int32 nCount = xList->getCount();
        for( sal_Int32 i=0; i<nCount; ++i )
        {
            ::com::sun::star::uno::Any aVal = xList->getByIndex(i);
            if ( !(aVal>>=xTask) || ! xTask.is() )
                continue;

            // ask for controller
            Reference< ::com::sun::star::frame::XController > xCtrl = xTask->getController();
            if ( xCtrl.is() )
            {
                // ask for model
                Reference< ::com::sun::star::frame::XModel > xModel( xCtrl->getModel(), UNO_QUERY );
                Reference< ::com::sun::star::util::XModifiable > xModifiable( xModel, UNO_QUERY );
                if ( xModifiable.is() && xModifiable->isModified() )
                {
                    // ask if modified
                    Reference< ::com::sun::star::frame::XStorable > xStor( xModel, UNO_QUERY );
                    if ( xStor.is() )
                    {
                        // get the media descriptor and retrieve filter name and password
                        ::rtl::OUString aOrigPassword, aOrigFilterName, aTitle;
                        Sequence < PropertyValue > aArgs( xModel->getArgs() );
                        sal_Int32 nProps = aArgs.getLength();
                        for ( sal_Int32 nProp = 0; nProp<nProps; nProp++ )
                        {
                            const PropertyValue& rProp = aArgs[nProp];
                            if( rProp.Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
                                rProp.Value >>= aOrigFilterName;
                            if( rProp.Name == OUString(RTL_CONSTASCII_USTRINGPARAM("Password")) )
                                rProp.Value >>= aOrigPassword;
                            if( rProp.Name == OUString(RTL_CONSTASCII_USTRINGPARAM("Title")) )
                                rProp.Value >>= aTitle;
                        }

                        // save document as tempfile in backup directory
                        // remember old name or title
                        ::rtl::OUString aOrigURL = xModel->getURL();
                        ::rtl::OUString aOldName, aSaveURL;
                        if ( aOrigURL.getLength() )
                        {
                            ::utl::TempFile aTempFile( &aSavePath );
                            aSaveURL = aTempFile.GetURL();
                            aOldName = INetURLObject( aOrigURL ).GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
                        }
                        else
                        {
                            // untitled document
                            String aExt( DEFINE_CONST_UNICODE( ".sav" ) );
                            ::utl::TempFile aTempFile( DEFINE_CONST_UNICODE( "exc" ), &aExt, &aSavePath );
                            aSaveURL = aTempFile.GetURL();
                            aOldName = aTitle;
                        }

                        if ( aOrigPassword.getLength() )
                        {
                            // if the document was loaded with a password, it should be stored with password
                            Sequence < PropertyValue > aSaveArgs(1);
                            aSaveArgs[0].Name = DEFINE_CONST_UNICODE("Password");
                            aSaveArgs[0].Value <<= aOrigPassword;

                            xStor->storeToURL( aSaveURL, aSaveArgs );
                        }
                        else
                            xStor->storeToURL( aSaveURL, Sequence < PropertyValue >() );

                        // remember original name and filter
                        aOpt.PushRecoveryItem(  aOldName, aOrigFilterName, aSaveURL );
                        bRecovery = TRUE;
                    }
                }
            }
        }

        if ( !pArgs->IsNoRestore() && ( nError & EXC_MAJORTYPE ) != EXC_DISPLAY && ( nError & EXC_MAJORTYPE ) != EXC_REMOTE )
            WarningBox( NULL, DesktopResId(STR_RECOVER_PREPARED) ).Execute();
    }

    // store configuration data
    ::utl::ConfigManager::GetConfigManager()->StoreConfigItems();

    // because there is no method to flush the condiguration data, we must dispose the ConfigManager
    Reference < XComponent > xComp( ::utl::ConfigManager::GetConfigManager()->GetConfigurationProvider(), UNO_QUERY );
    xComp->dispose();

    switch( nError & EXC_MAJORTYPE )
    {
/*
        case EXC_USER:
            if( nError == EXC_OUTOFMEMORY )
            {
                // not possible without a special NewHandler!
                String aMemExceptionString;
                Application::Abort( aMemExceptionString );
            }
            break;
*/
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
            if ( pArgs->IsNoRestore() )
                _exit( 333 );

            if( bRecovery && !pPluginAcceptThread && !Application::IsRemoteServer() )
            {
                OfficeIPCThread::DisableOfficeIPCThread();
                if( pSignalHandler )
                    DELETEZ( pSignalHandler );

                ::rtl::OUString aProgName, aTmp;
                ::vos::OStartupInfo aInfo;
                aInfo.getExecutableFile( aProgName );

                Reference< XSystemShellExecute > xSystemShellExecute( ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" )), UNO_QUERY );
                if ( xSystemShellExecute.is() )
                {
                    ::rtl::OUString aSysPathFileName;
                    ::osl::FileBase::RC nError = ::osl::FileBase::getSystemPathFromFileURL( aProgName, aSysPathFileName );
                    if ( nError == ::osl::FileBase::E_None )
                         xSystemShellExecute->execute( aSysPathFileName, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                }

                _exit( 333 );
            }
            else
            {
                bInException = sal_False;
                return 0;
            }

            break;
        }
    }

    return 0;

    // ConfigManager is disposed, so no way to continue
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
}

void Desktop::Main()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Main" );

    // Error handling inside Desktop::Main() because vcl is not
    // initialized before!!!
    if ( m_aBootstrapError != BE_OK )
    {
        HandleBootstrapErrors( m_aBootstrapError );
    }

    CommandLineArgs* pCmdLineArgs = GetCommandLineArgs();

    ResMgr::SetReadStringHook( ReplaceStringHookProc );
    SetAppName( DEFINE_CONST_UNICODE("soffice") );

    // ----  Startup screen ----
    OpenStartupScreen();

    //  Read the common configuration items for optimization purpose
    //  do not do it if terminate flag was specified, to avoid exception
    sal_Bool bTerminate = pCmdLineArgs->IsTerminateAfterInit();
    if( !bTerminate )
    {
        try
        {
            PreloadConfigTrees();
        }
        catch( ::com::sun::star::configuration::MissingBootstrapFileException& e )
        {
            OUString aMsg( CreateErrorMsgString( utl::Bootstrap::MISSING_BOOTSTRAP_FILE,
                                                    e.BootstrapFileURL ));
            HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_USER_INSTALL, aMsg );
        }
        catch( ::com::sun::star::configuration::InvalidBootstrapFileException& e )
        {
            OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY,
                                                    e.BootstrapFileURL ));
            HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
        }
        catch( ::com::sun::star::configuration::InstallationIncompleteException& )
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
        catch ( ::com::sun::star::configuration::CannotLoadConfigurationException& )
        {
            OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                 OUString() ));
            HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                 OUString() ));
            HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
        }
    }

    LanguageType aLanguageType;
    String aMgrName = String::CreateFromAscii( "iso" );
    aMgrName += String::CreateFromInt32(SOLARUPD); // current build version
    ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLanguageType );
    String aTitle = String( ResId( RID_APPTITLE, pLabelResMgr ) );
    delete pLabelResMgr;

#ifndef BUILD_SOSL
    // get the tabreg service for an evaluation version
    // without this service office shouldn't run at all
    String aEval;
    sal_Bool bExpired = sal_True;
    Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();
    Reference < XMaterialHolder > xHolder( xSMgr->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.tab.tabreg" ) ) ), UNO_QUERY );
    if ( xHolder.is() )
    {
        // get a sequence of strings for the defined locales
        // a registered version doesn't provide data
        bExpired = sal_False;
        Any aData = xHolder->getMaterial();
        Sequence < NamedValue > aSeq;
        if ( aData >>= aSeq )
        {
            // this is an evaluation version, because it provides "material"
            bExpired = sal_True;

            // determine current locale
            ::rtl::OUString aLocale;
            ::rtl::OUString aTmp;
            Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
            aRet >>= aLocale;

            sal_Int32 nCount = aSeq.getLength();
            if ( nCount )
            {
                // first entry is for expiry
                const NamedValue& rValue = aSeq[0];
                if ( rValue.Name.equalsAscii("expired") )
                    rValue.Value >>= bExpired;
            }

            // find string for matching locale
            sal_Int32 n;
            for ( n=0; n<nCount; n++ )
            {
                const NamedValue& rValue = aSeq[n];
                if ( rValue.Name == aLocale )
                {
                    rValue.Value >>= aTmp;
                    aEval = aTmp;
                    break;
                }
            }

            if ( n == nCount )
            {
                // try matching only first part of locale, if tab service provides it
                ::rtl::OUString aShortLocale;
                sal_Int32 nPos = aLocale.indexOf('_');
                if ( nPos > 0 )
                {
                    aShortLocale = aLocale.copy( 0, nPos );
                    for ( n=0; n<nCount; n++ )
                    {
                        const NamedValue& rValue = aSeq[n];
                        if ( rValue.Name == aShortLocale )
                        {
                            rValue.Value >>= aTmp;
                            aEval = aTmp;
                            break;
                        }
                    }
                }
            }

            if ( n == nCount )
            {
                // current locale is unknown for service, use default english
                sal_Int32 nCount = aSeq.getLength();
                for ( n=0; n<nCount; n++ )
                {
                    const NamedValue& rValue = aSeq[n];
                    if ( rValue.Name.equalsAscii("en") )
                    {
                        rValue.Value >>= aTmp;
                        aEval = aTmp;
                        break;
                    }
                }

                if ( n == nCount )
                    // strange version, no english string
                    bExpired = sal_True;
            }
        }
    }

    if ( aEval.Len() )
    {
        if ( aTitle.GetChar(aTitle.Len()-1) != ' ' )
            aTitle += ' ';
        aTitle += aEval;
    }

    if ( bExpired )
    {
        InfoBox aBox( NULL, aTitle );
        aBox.Execute();
        return;
    }

#endif

#ifndef PRODUCT
    ::rtl::OUString aDefault;
    aTitle += DEFINE_CONST_UNICODE(" [");
    String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
    aTitle += aVerId;
    aTitle += 0x005D ; // 5Dh ^= ']'
#endif

    SetDisplayName( aTitle );

    //  The only step that should be done if terminate flag was specified
    //  Typically called by the plugin only
    {
        RTL_LOGFILE_CONTEXT( aLog, "setup2 (ok93719) ::Installer::InitializeInstallation" );
        InitializeInstallation( Application::GetAppFileName() );
    }

    if( !bTerminate )
    {
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create SvtPathOptions and SvtLanguageOptions" );
        SvtPathOptions* pPathOptions = new SvtPathOptions;
        SvtLanguageOptions* pLanguageOptions = new SvtLanguageOptions(sal_True);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create SvtPathOptions and SvtLanguageOptions" );
        registerServices( xSMgr );

        OUString        aDescription;
        Sequence< Any > aSeq( 1 );

        if ( pOfficeAcceptThread )
            aDescription = pOfficeAcceptThread->GetDescriptionString();
        else
            pCmdLineArgs->GetPortalConnectString( aDescription );
        aSeq[0] <<= aDescription;

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ createInstance com.sun.star.office.OfficeWrapper" );
        Reference < XComponent > xWrapper( xSMgr->createInstanceWithArguments( DEFINE_CONST_UNICODE(
                                                "com.sun.star.office.OfficeWrapper" ), aSeq ),
                                        UNO_QUERY );
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} createInstance com.sun.star.office.OfficeWrapper" );

        sal_Bool bTerminateRequested = sal_False;

#ifndef BUILD_SOSL
        // preload function depends on an initialized sfx application!
        if ( IsOEMPreload() )
        {
            if ( !CheckOEMPreload() )
            {
                try
                {
                    bTerminateRequested = sal_True;
                    Reference< XDesktop > xDesktop( xSMgr->createInstance(
                                                                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))),
                                                                UNO_QUERY );
                    xDesktop->terminate();
                }
                catch( ::com::sun::star::uno::Exception& )
                {
                }
            }
        }
#endif

        {
            Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

            InitTestToolLib();

            if ( !bTerminateRequested )
            {
                try
                {
                    // the shutdown icon sits in the systray and allows the user to keep
                    // the office instance running for quicker restart
                    // this will only be activated if -quickstart was specified on cmdline
                    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.office.Quickstart" );

                    sal_Bool bQuickstart = pCmdLineArgs->IsQuickstart();
                    Sequence< Any > aSeq( 1 );
                    aSeq[0] <<= bQuickstart;

                    // Try to instanciate quickstart service. This service is not mandatory, so
                    // do nothing if service is not available.
                    Reference < XComponent > xQuickstart( xSMgr->createInstanceWithArguments(
                                                            DEFINE_CONST_UNICODE( "com.sun.star.office.Quickstart" ), aSeq ),
                                                          UNO_QUERY );
                }
                catch( ::com::sun::star::uno::Exception& )
                {
                }
            }

            if ( pCmdLineArgs->IsPlugin() )
            {
                RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (cd100003) create PluginAcceptThread" );

                OSecurity   aSecurity;
                OUString    aUserIdent;
                OUString    aVersionStr;

                aSecurity.getUserIdent( aUserIdent );

                OSL_ENSURE( pCmdLineArgs->GetVersionString( aVersionStr ), "No plugin version is specified!\n" );

                OUString    aAcceptString( RTL_CONSTASCII_USTRINGPARAM( "pipe,name=soffice_plugin" ));
                aAcceptString += aVersionStr;
                aAcceptString += aUserIdent;

                pPluginAcceptThread = new PluginAcceptThread(   xSMgr,
                                                                new OInstanceProvider( xSMgr ),
                                                                aAcceptString );

                // We have to acquire the plugin accept thread object to be sure
                // that the instance is still alive after an exception was thrown
                pPluginAcceptThread->acquire();
                pPluginAcceptThread->create();
            }

            if ( !Application::IsRemoteServer() )
            {
                // Create TypeDetection service to have filter informations for quickstart feature
                RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.document.TypeDetection" );
                Reference< XTypeDetection > xTypeDetection( xSMgr->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ))),
                                                            UNO_QUERY );
                Reference< XDesktop > xDesktop( xSMgr->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))),
                                                            UNO_QUERY );
                if ( xDesktop.is() )
                    xDesktop->addTerminateListener( new OfficeIPCThreadController );
            }

            // Release solar mutex just before we wait for our client to connect
            int nAcquireCount = 0;
            ::vos::IMutex& rMutex = Application::GetSolarMutex();
            if ( rMutex.tryToAcquire() )
                nAcquireCount = Application::ReleaseSolarMutex() - 1;

            Application::WaitForClientConnect();

            // Post user event to startup first application component window
            // We have to send this OpenClients message short before execute() to
            // minimize the risk that this message overtakes type detection contruction!!
            Application::PostUserEvent( LINK( this, Desktop, OpenClients_Impl ) );

            // Acquire solar mutex just before we enter our message loop
            if ( nAcquireCount )
                Application::AcquireSolarMutex( nAcquireCount );

            // call Application::Execute to process messages in vcl message loop
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "call ::Application::Execute" );
            Execute();

            // remove temp directory
            removeTemporaryDirectory();

            if( pOfficeAcceptThread )
            {
                pOfficeAcceptThread->stopAccepting();
#ifndef LINUX
                pOfficeAcceptThread->join();
                delete pOfficeAcceptThread;
#endif
                pOfficeAcceptThread = 0;
            }

            if ( pPluginAcceptThread )
            {
                pPluginAcceptThread->terminate();
                pPluginAcceptThread->release();
            }

            DeInitTestToolLib();
        }

        xWrapper->dispose();
        xWrapper = 0;

        delete pLanguageOptions;
        delete pPathOptions;
    }

    ::ucb::ContentBroker::deinitialize();

    // instead of removing of the configManager just let it commit all the changes
    utl::ConfigManager::GetConfigManager()->StoreConfigItems();
}

void Desktop::SystemSettingsChanging( AllSettings& rSettings, Window* pFrame )
{
//  OFF_APP()->SystemSettingsChanging( rSettings, pFrame );
}

// ========================================================================
typedef ::vos::OTimer OFirstOfficeRunInitTimer_Base;
class OFirstOfficeRunInitTimer : public OFirstOfficeRunInitTimer_Base
{
private:
    Link    m_aAsyncExpireHandler;

public:
    OFirstOfficeRunInitTimer( const Link& _rExpireHdl );

private:
    virtual void SAL_CALL onShot();
};

// ========================================================================
OFirstOfficeRunInitTimer::OFirstOfficeRunInitTimer( const Link& _rExpireHdl )
    :OFirstOfficeRunInitTimer_Base( TTimeValue( 3, 0 ) )
    ,m_aAsyncExpireHandler( _rExpireHdl )
{
    acquire();
}

// ========================================================================
void SAL_CALL OFirstOfficeRunInitTimer::onShot()
{
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        Application::PostUserEvent( m_aAsyncExpireHandler );
    }

    // delete ourself - we're not needed anymore
    release();
}

// ========================================================================
IMPL_LINK( Desktop, AsyncInitFirstRun, void*, NOTINTERESTEDIN )
{
    DoFirstRunInitializations();
    return 0L;
}

// ========================================================================
IMPL_LINK( Desktop, OpenClients_Impl, void*, pvoid )
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::OpenClients_Impl" );

    OpenClients();
    CloseStartupScreen();

    CheckFirstRun( );

    EnableOleAutomation();
    return 0;
}
// Registers a COM class factory of the service manager with the windows operating system.
void Desktop::EnableOleAutomation()
{
      RTL_LOGFILE_CONTEXT( aLog, "desktop (jl97489) ::Desktop::EnableOleAutomation" );
#ifdef WNT
    Reference< XMultiServiceFactory > xSMgr=  comphelper::getProcessServiceFactory();
    xSMgr->createInstance(DEFINE_CONST_UNICODE("com.sun.star.bridge.OleApplicationRegistration"));
#endif
}

void Desktop::OpenClients()
{
    // check if a document has been recovered - if there is one of if a document was loaded by cmdline, no default document
    // should be created
    Reference < XComponent > xFirst;
    BOOL bLoaded = FALSE;

    CommandLineArgs* pArgs = GetCommandLineArgs();
    SvtInternalOptions  aInternalOptions;

    if ( !pArgs->IsServer() && !pArgs->IsNoRestore() && !aInternalOptions.IsRecoveryListEmpty() )
    {
        // crash recovery
        sal_Bool bUserCancel = sal_False;
        ::rtl::OUString sName;
        ::rtl::OUString sFilter;
        ::rtl::OUString sTempName;

        Reference< XComponentLoader > xDesktop(
                ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );

        // create the parameter array
        Sequence < PropertyValue > aArgs( 5 );
        aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");
        aArgs[1].Name = ::rtl::OUString::createFromAscii("AsTemplate");
        aArgs[2].Name = ::rtl::OUString::createFromAscii("FilterName");
        aArgs[3].Name = ::rtl::OUString::createFromAscii("SalvagedFile");

        Reference < com::sun::star::task::XInteractionHandler > xInteraction(
            ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
            com::sun::star::uno::UNO_QUERY );

        aArgs[4].Name = OUString::createFromAscii( "InteractionHandler" );
        aArgs[4].Value <<= xInteraction;

        // mark it as a user request
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:user");

        while(  !aInternalOptions.IsRecoveryListEmpty() && !bUserCancel )
        {
            // Read and delete top recovery item from list
            aInternalOptions.PopRecoveryItem( sName, sFilter, sTempName );

            INetURLObject aURL( sName );

            sal_Bool bIsURL = aURL.GetProtocol() != INET_PROT_NOT_VALID;
            String sTempFileName( sTempName );
            String sRealFileName;
            if ( bIsURL )
                sRealFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );

            String aMsg( DesktopResId( STR_RECOVER_QUERY ) );
            aMsg.SearchAndReplaceAscii( "$1", sName );
            MessBox aBox( NULL, WB_YES_NO_CANCEL | WB_DEF_YES | WB_3DLOOK, String( DesktopResId( STR_RECOVER_TITLE ) ), aMsg );
            switch( aBox.Execute() )
            {
                case RET_YES:
                {
                    // recover a file
                    aArgs[2].Value <<= ::rtl::OUString( sFilter );
                    if ( bIsURL )
                    {
                        // get the original URL for the recovered document
                        aArgs[1].Value <<= sal_False;
                        aArgs[3].Value <<= ::rtl::OUString( sRealFileName );
                    }
                    else
                    {
                        // this was an untitled document ( open as template )
                        aArgs[1].Value <<= sal_True;
                        aArgs[3].Value <<= ::rtl::OUString();
                    }

                    // load the document
                    Reference < XComponent > xDoc = xDesktop->loadComponentFromURL( sTempFileName, ::rtl::OUString::createFromAscii( "_blank" ), 0, aArgs );
                    if ( !xFirst.is() )
                        // remember the first successfully recovered file
                        xFirst = xDoc;

                    // backup copy will be removed when document is closed
                    break;
                }

                case RET_NO:
                {
                    // skip this file
                    ::utl::UCBContentHelper::Kill( sTempFileName );
                    break;
                }

                case RET_CANCEL:
                {
                    // cancel recovering
                    ::utl::UCBContentHelper::Kill( sTempFileName );
                    bUserCancel = sal_True;

                    // delete recovery list and all files
                    while( aInternalOptions.IsRecoveryListEmpty() == sal_False )
                    {
                        aInternalOptions.PopRecoveryItem( sName, sFilter, sTempName );
                        ::utl::UCBContentHelper::Kill( sTempName );
                    }

                    break;
                }
            }
        }
    }

    if ( !pArgs->IsServer() )
    {
        ProcessDocumentsRequest aRequest;

        pArgs->GetOpenList( aRequest.aOpenList );
        pArgs->GetPrintList( aRequest.aPrintList );
        pArgs->GetPrintToList( aRequest.aPrintToList );
        pArgs->GetPrinterName( aRequest.aPrinterName );
        pArgs->GetForceOpenList( aRequest.aForceOpenList );
        pArgs->GetForceNewList( aRequest.aForceNewList );

        if ( aRequest.aOpenList.getLength() > 0 ||
             aRequest.aPrintList.getLength() > 0 ||
             aRequest.aForceOpenList.getLength() > 0 ||
             aRequest.aForceNewList.getLength() > 0 ||
             ( aRequest.aPrintToList.getLength() > 0 && aRequest.aPrinterName.getLength() > 0 ))
        {
            bLoaded = sal_True;

            // Process request
            OfficeIPCThread::ExecuteCmdLineRequests( aRequest );
        }
    }

    // no default document if a document was loaded by recovery or by command line or if soffice is used as server
    if ( bLoaded || xFirst.is() || pArgs->IsServer() )
        return;

    if( pArgs->IsQuickstart()   ||
        pArgs->IsInvisible()    ||
        pArgs->IsPlugin()       ||
        pArgs->IsBean()             )

        // soffice was started as tray icon
        return;

    {
        OpenDefault();
    }
}

void Desktop::OpenDefault()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::OpenDefault" );

    ::rtl::OUString aName;
    if ( !aName.getLength() )
    {
        SvtModuleOptions aOpt;
        if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITER );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_CALC );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_IMPRESS );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DRAW );
        else
            return;
    }

    Sequence < PropertyValue > aArgs(1);
    Reference < com::sun::star::task::XInteractionHandler > xInteraction(
        ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
        com::sun::star::uno::UNO_QUERY );

    aArgs[0].Name = OUString::createFromAscii( "InteractionHandler" );
    aArgs[0].Value <<= xInteraction;

    Reference< XComponentLoader > xDesktop(
            ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
            ::com::sun::star::uno::UNO_QUERY );
    xDesktop->loadComponentFromURL( aName, ::rtl::OUString::createFromAscii( "_blank" ), 0, aArgs );
}


String GetURL_Impl( const String& rName )
{
    // if the filename is a physical name, it is the client file system, not the file system
    // of the machine where the office is running ( if this are different machines )
    // so in the remote case we can't handle relative filenames as arguments, because they
    // are parsed relative to the program path
    // the file system of the client is addressed through the "file:" protocol

    // Get current working directory to support relativ pathes
    ::rtl::OUString aWorkingDir;
    osl_getProcessWorkingDir( &aWorkingDir.pData );

    // Add path seperator to these directory and make given URL (rName) absolute by using of current working directory
    // Attention: "setFianlSlash()" is neccessary for calling "smartRel2Abs()"!!!
    // Otherwhise last part will be ignored and wrong result will be returned!!!
    // "smartRel2Abs()" interpret given URL as file not as path. So he truncate last element to get the base path ...
    // But if we add a seperator - he doesn't do it anymore.
    INetURLObject aObj( aWorkingDir );
    aObj.setFinalSlash();

    bool bWasAbsolute;
    INetURLObject aURL     = aObj.smartRel2Abs( rName, bWasAbsolute );
    String        aFileURL = aURL.GetMainURL(INetURLObject::NO_DECODE);

    ::osl::FileStatus aStatus( FileStatusMask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    return aFileURL;
}

void Desktop::HandleAppEvent( const ApplicationEvent& rAppEvent )
{
    if ( rAppEvent.IsOpenEvent() || rAppEvent.IsPrintEvent() )
    {
        String aPrinterName;
        Reference< XComponentLoader > xDesktop(
                ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );

        // create parameter array
        sal_Int32 nCount = rAppEvent.IsPrintEvent() ? 5 : 2;
        Sequence < PropertyValue > aArgs( nCount );
        aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");

        if ( rAppEvent.IsPrintEvent() )
        {
            aArgs[1].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            aArgs[2].Name = ::rtl::OUString::createFromAscii("OpenNewView");
            aArgs[3].Name = ::rtl::OUString::createFromAscii("Hidden");
            aArgs[4].Name = ::rtl::OUString::createFromAscii("Silent");
        }
        else
        {
            Reference < com::sun::star::task::XInteractionHandler > xInteraction(
                ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
                com::sun::star::uno::UNO_QUERY );

            aArgs[1].Name = OUString::createFromAscii( "InteractionHandler" );
            aArgs[1].Value <<= xInteraction;
        }

        // mark request as user interaction from outside
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:OpenEvent");

        for( sal_uInt16 i=0; i<rAppEvent.GetParamCount(); i++ )
        {
            // get file name
            String aName( rAppEvent.GetParam(i) );
            ::rtl::OUString aTarget( DEFINE_CONST_UNICODE("_default") );

            // is the parameter a printername ?
            if( aName.Len()>1 && *aName.GetBuffer()=='@' )
            {
                aPrinterName = aName.Copy(1);
                continue;
            }

            aName = GetURL_Impl(aName);

            if ( rAppEvent.IsPrintEvent() )
            {
                // documents opened for printing are opened readonly because they must be opened as a new document and this
                // document could be open already
                aArgs[1].Value <<= sal_True;

                // always open a new document for printing, because it must be disposed afterwards
                aArgs[2].Value <<= sal_True;

                // printing is done in a hidden view
                aArgs[3].Value <<= sal_True;

                // load document for printing without user interaction
                aArgs[4].Value <<= sal_True;

                // hidden documents should never be put into open tasks
                aTarget = ::rtl::OUString( DEFINE_CONST_UNICODE("_blank") );
            }

            try
            {
            Reference < XPrintable > xDoc;
            if(
                ( aName.CompareToAscii( ".uno"  , 4 ) == COMPARE_EQUAL )  ||
                ( aName.CompareToAscii( "slot:" , 5 ) == COMPARE_EQUAL )  ||
                ( aName.CompareToAscii( "macro:", 6 ) == COMPARE_EQUAL )
              )
            {
                URL             aURL ;
                aURL.Complete = aName;

                Reference < XDispatch >         xDispatcher ;
                Reference < XDispatchProvider > xProvider   ( xDesktop, UNO_QUERY );
                Reference < XURLTransformer >   xParser     ( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer")) ), ::com::sun::star::uno::UNO_QUERY );

                if( xParser.is() == sal_True )
                    xParser->parseStrict( aURL );

                if( xProvider.is() == sal_True )
                    xDispatcher = xProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );

                if( xDispatcher.is() == sal_True )
                {
                    // We have to be listener to catch errors during dispatching URLs.
                    // Otherwise it would be possible to have an office running without an open
                    // window!!
                    Reference < XNotifyingDispatch > xDisp( xDispatcher, UNO_QUERY );
                    if ( xDisp.is() )
                        xDisp->dispatchWithNotification( aURL, aArgs, DispatchWatcher::GetDispatchWatcher() );
                    else
                        xDispatcher->dispatch( aURL, aArgs );
                }
            }
            else
            {
                INetURLObject aObj( aName );
                if ( aObj.GetProtocol() == INET_PROT_PRIVATE )
                    aTarget = ::rtl::OUString( DEFINE_CONST_UNICODE("_blank") );
                xDoc = Reference < XPrintable >( xDesktop->loadComponentFromURL( aName, aTarget, 0, aArgs ), UNO_QUERY );
                if ( !xDoc.is() )
                {
                    // error case
                    Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
                    Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );

                    if ( !xList->hasElements() )
                    {
                        // We don't have any task open so we have to shutdown ourself!!
                        Reference< XDesktop > xDesktop( xTasksSupplier, UNO_QUERY );
                        if ( xDesktop.is() )
                            xDesktop->terminate();
                        return;
                    }
                }
            }

            if ( rAppEvent.IsPrintEvent() )
            {
                if ( xDoc.is() )
                {
                    if ( aPrinterName.Len() )
                    {
                        // create the printer
                        Sequence < PropertyValue > aPrinterArgs( 1 );
                        aPrinterArgs[0].Name = ::rtl::OUString::createFromAscii("Name");
                        aPrinterArgs[0].Value <<= ::rtl::OUString( aPrinterName );
                        xDoc->setPrinter( aPrinterArgs );
                    }

                    // print ( also without user interaction )
                    Sequence < PropertyValue > aPrinterArgs( 1 );
                    aPrinterArgs[0].Name = ::rtl::OUString::createFromAscii("Silent");
                    aPrinterArgs[0].Value <<= ( sal_Bool ) sal_True;
                    xDoc->print( aPrinterArgs );
                }
                else
                {
                    // place error message here ...
                }

                // remove the document
                Reference < XComponent > xComp( xDoc, UNO_QUERY );
                if ( xComp.is() )
                    xComp->dispose();
            }
            }
            catch ( com::sun::star::uno::Exception& )
            {
            }
        }

        // remove this pending request
        OfficeIPCThread::RequestsCompleted( 1 );
    }
    else if ( rAppEvent.GetEvent() == "APPEAR" )
    {
        // find active task - the active task is always a visible task
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >
                xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
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
            // no visible task that could be activated found
            OpenDefault();
    }
    else if ( rAppEvent.GetEvent() == "QUICKSTART" )
    {
        CommandLineArgs* pCmdLineArgs = GetCommandLineArgs();

        if ( !pCmdLineArgs->IsQuickstart() )
        {
            // If the office has been started the second time its command line arguments are sent through a pipe
            // connection to the first office. We want to reuse the quickstart option for the first office.
            // NOTICE: The quickstart service must be initialized inside the "main thread", so we use the
            // application events to do this (they are executed inside main thread)!!!
            sal_Bool bQuickstart( sal_True );
            Sequence< Any > aSeq( 1 );
            aSeq[0] <<= bQuickstart;

            Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    DEFINE_CONST_UNICODE( "com.sun.star.office.Quickstart" )),
                                                  UNO_QUERY );
            if ( xQuickstart.is() )
                xQuickstart->initialize( aSeq );
        }
    }
}

void Desktop::OpenStartupScreen()
{
     RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::OpenStartupScreen" );

    ::rtl::OUString     aTmpString;
    CommandLineArgs*    pCmdLine = GetCommandLineArgs();

    // Show intro only if this is normal start (e.g. no server, no quickstart, no printing )
    if ( !Application::IsRemoteServer() &&
         !pCmdLine->IsInvisible() &&
         !pCmdLine->IsQuickstart() &&
         !pCmdLine->IsMinimized() &&
         !pCmdLine->IsTerminateAfterInit() &&
         !pCmdLine->GetPrintList( aTmpString ) &&
         !pCmdLine->GetPrintToList( aTmpString ) )
    {
        String          aBmpFileName;
        ::rtl::OUString aProductKey;
        ::rtl::OUString aIniPath;
        ::rtl::OUString aLogo( RTL_CONSTASCII_USTRINGPARAM( "1" ) );
        Bitmap          aIntroBmp;

        // load bitmap depends on productname ("StarOffice", "StarSuite",...)
        aProductKey = ::utl::Bootstrap::getProductKey( aProductKey );
        aLogo       = ::utl::Bootstrap::getLogoData( aLogo );

        sal_Bool    bLogo   = (sal_Bool)aLogo.toInt32();
        if ( bLogo )
        {
            xub_StrLen nIndex = 0;

            aBmpFileName = aProductKey;
            aBmpFileName = aBmpFileName.GetToken( 0, (sal_Unicode)' ', nIndex );
            aBmpFileName += String( DEFINE_CONST_UNICODE("_intro.bmp") );

            // retrieve our current installation path
            ::rtl::OUString aExecutePath;
            ::vos::OStartupInfo().getExecutableFile( aExecutePath );
            sal_uInt32  lastIndex = aExecutePath.lastIndexOf('/');
            if ( lastIndex > 0 )
                aExecutePath = aExecutePath.copy( 0, lastIndex+1 );

            INetURLObject aObj( aExecutePath, INET_PROT_FILE );
            aObj.insertName( aBmpFileName );
            SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
            if ( !aStrm.GetError() )
            {
                // Default case, we load the intro bitmap from a seperate file
                // (e.g. staroffice_intro.bmp or starsuite_intro.bmp)
                aStrm >> aIntroBmp;
            }
            else
            {
                // Save case:
                // Create resource manager for intro bitmap. Due to our problem that we don't have
                // any language specific information, we have to search for the correct resource
                // file. The bitmap resource is language independent.
                const USHORT nResId = RID_DEFAULTINTRO;
                LanguageType aLanguageType;
                String       aMgrName = String::CreateFromAscii( "iso" );

                aMgrName += String::CreateFromInt32(SOLARUPD); // current build version
                ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLanguageType );

                ResId aIntroBmpRes( nResId, pLabelResMgr );
                aIntroBmp = Bitmap( aIntroBmpRes );
                delete pLabelResMgr;
            }

            m_pIntro = new IntroWindow_Impl( aIntroBmp );
        }
    }
}

void Desktop::CloseStartupScreen()
{
    // close splash screen and delete window
    delete m_pIntro;
    m_pIntro = 0;
    RTL_LOGFILE_TRACE( "desktop (cd100003) ::Desktop::CloseStartupScreen" );
}


// ========================================================================
void Desktop::DoFirstRunInitializations()
{
    try
    {
        Reference< XJobExecutor > xExecutor( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.task.JobExecutor" ) ), UNO_QUERY );
        if( xExecutor.is() )
            xExecutor->trigger( ::rtl::OUString::createFromAscii("onFirstRunInitialization") );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        OSL_ENSURE( sal_False, "Desktop::DoFirstRunInitializations: caught an exception while trigger job executor ..." );
    }
}

// ========================================================================
void Desktop::CheckFirstRun( )
{
    const ::rtl::OUString sCommonMiscNodeName = ::rtl::OUString::createFromAscii( "/org.openoffice.Office.Common/Misc" );
    const ::rtl::OUString sFirstRunNodeName = ::rtl::OUString::createFromAscii( "FirstRun" );

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

    // do the initialization asynchronously
    ::vos::ORef< ::vos::OTimer > xInitTimer = new OFirstOfficeRunInitTimer( LINK( this, Desktop, AsyncInitFirstRun ) );
    xInitTimer->start();
    OSL_ENSURE( xInitTimer->isTicking() && !xInitTimer->isExpired(),
        "Desktop::CheckFirstRun: strange timer behaviour!" );

    // --------------------------------------------------------------------
    // reset the config flag

    // set the value
    aCommonMisc.setNodeValue( sFirstRunNodeName, makeAny( (sal_Bool)sal_False ) );
    // commit the changes
    aCommonMisc.commit();
}
