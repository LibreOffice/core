/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: cd $ $Date: 2001-07-20 09:52:29 $
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

#include "app.hxx"
#include "desktop.hrc"
#include "appinit.hxx"
#include "intro.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "officeacceptthread.hxx"
#include "pluginacceptthread.hxx"
#include "appsys.hxx"
#include "desktopresid.hxx"

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
#ifndef _COM_SUN_STAR_BRIDGE_XCONNECTIONBROKER_HPP_
#include <com/sun/star/bridge/XConnectionBroker.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#include <comphelper/processfactory.hxx>
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
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
#ifndef SVTOOLS_TESTTOOL_HXX
#include <svtools/testtool.hxx>
#endif
#include <svtools/pathoptions.hxx>
#include <svtools/cjkoptions.hxx>
#include <svtools/internaloptions.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>

#include <rtl/logfile.hxx>
#include <setup2/installer.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/bitmap.hxx>
#include <sfx2/sfx.hrc>
#include <ucbhelper/contentbroker.hxx>

#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII##))
#define U2S(STRING)                             ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

using namespace vos;
using namespace rtl;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::system;

static SalMainPipeExchangeSignalHandler* pSignalHandler = 0;

OOfficeAcceptorThread*  pOfficeAcceptThread = 0;
ResMgr*                 Desktop::pResMgr    = 0;
static PluginAcceptThread* pPluginAcceptThread = 0;

// ----------------------------------------------------------------------------

ResMgr* Desktop::GetDesktopResManager()
{
    if ( !Desktop::pResMgr )
    {
        String aMgrName = String::CreateFromAscii( "dkt" );
        aMgrName += String::CreateFromInt32(SOLARUPD);
        return ResMgr::CreateResMgr(U2S(aMgrName));
    }
    return Desktop::pResMgr;
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

void PreloadConfigTrees()
{
    RTL_LOGFILE_CONTEXT( aLog, "PreloadConfigTrees()" );

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

    Any aValue;
    aValue <<= aPreloadPathList;

    Reference < com::sun::star::beans::XPropertySet > (xProvider, UNO_QUERY)->setPropertyValue(rtl::OUString::createFromAscii("PrefetchNodes"), aValue );
}


void ReplaceStringHookProc( UniString& rStr )
{
    static String aBrandName;
    static String aVersion;
    static String aExtension;

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

        aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTEXTENSION );
        aRet >>= aTmp;
        aExtension = aTmp;
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

Desktop aDesktop;

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

Desktop::Desktop() : m_pLabelResMgr( 0 ), m_pIntro( 0 )
{
}

void Desktop::Init()
{
    Reference < XMultiServiceFactory > rSMgr = createApplicationServiceManager();
    if( ! rSMgr.is() )
        exit(0);
    ::comphelper::setProcessServiceFactory( rSMgr );

    if ( !Application::IsRemoteServer() )
    {
        // start ipc thread only for non-remote offices
        RTL_LOGFILE_CONTEXT( aLog, "OfficeIPCThread::EnableOfficeIPCThread" );
        if( !OfficeIPCThread::EnableOfficeIPCThread( ) )
            exit( 0 );
        pSignalHandler = new SalMainPipeExchangeSignalHandler;
    }
}

void Desktop::DeInit()
{
    destroyApplicationServiceManager( ::comphelper::getProcessServiceFactory() );

    if( !Application::IsRemoteServer() )
    {
        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            DELETEZ( pSignalHandler );
    }
}

BOOL Desktop::QueryExit()
{
    return TRUE;
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

    // save all modified documents
    if( Application::IsInExecute() )
    {
        // store to backup path
        String aSavePath( SvtPathOptions().GetBackupPath() );
        SvtInternalOptions aOpt;

        // iterate tasks
        Reference< ::com::sun::star::frame::XTasksSupplier >
                xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                UNO_QUERY );
        Reference< ::com::sun::star::frame::XTask > xTask;
        Reference< ::com::sun::star::container::XEnumeration > xList = xDesktop->getTasks()->createEnumeration();
        while( xList->hasMoreElements() )
        {
            xList->nextElement() >>= xTask;

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
                        ::rtl::OUString aOrigPassword, aOrigFilterName;
                        Sequence < PropertyValue > aArgs( xModel->getArgs() );
                        sal_Int32 nProps = aArgs.getLength();
                        for ( sal_Int32 nProp = 0; nProp<nProps; nProp++ )
                        {
                            const PropertyValue& rProp = aArgs[nProp];
                            if( rProp.Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
                                rProp.Value >>= aOrigFilterName;
                            if( rProp.Name == OUString(RTL_CONSTASCII_USTRINGPARAM("Password")) )
                                rProp.Value >>= aOrigPassword;
                        }

                        // save document as tempfile in backup directory
                        // remember old name or title
                        ::rtl::OUString aOrigURL = xModel->getURL();
                        ::rtl::OUString aOldName, aSaveURL;
                        if ( aOrigURL.getLength() )
                        {
                            ::utl::TempFile aTempFile( &aSavePath );
                            aSaveURL = aTempFile.GetURL();
                            aOldName = aOrigURL;
                        }
                        else
                        {
                            // untitled document
                            String aExt( DEFINE_CONST_UNICODE( ".sav" ) );
                            ::utl::TempFile aTempFile( DEFINE_CONST_UNICODE( "exc" ), &aExt, &aSavePath );
                            aSaveURL = aTempFile.GetURL();
                            // aOldName = Title;
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
                    }
                }
            }
        }

        if ( ( nError & EXC_MAJORTYPE ) != EXC_DISPLAY && ( nError & EXC_MAJORTYPE ) != EXC_REMOTE )
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
            if( !pPluginAcceptThread && !Application::IsRemoteServer() )
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
            }

            exit( 333 );
//            Application::Abort( String() );
            break;
        }
    }

    return TRUE;

    // ConfigManager is disposed, so no way to continue
    // bInException = sal_False;
    // return Application::Exception( nError );
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
}

void Desktop::Main()
{
    RTL_LOGFILE_CONTEXT( aLog, "Desktop::Main()" );

    CommandLineArgs* pCmdLineArgs = GetCommandLineArgs();

    // ----  Startup screen ----
    OpenStartupScreen( "iso" );

    ResMgr::SetReadStringHook( ReplaceStringHookProc );
    SetAppName( DEFINE_CONST_UNICODE("soffice") );

#ifdef TIMEBOMB
    Date aDate;
    Date aFinalDate( 31, 10, 2001 );
    if ( aFinalDate < aDate )
    {
        String aMsg;
        aMsg += DEFINE_CONST_UNICODE("This Early Access Version has expired!\n");
        InfoBox aBox( NULL, aMsg );
        aBox.Execute();
        return;
    }
#endif

    sal_Bool bTerminate = pCmdLineArgs->IsTerminateAfterInit();

    //  Read the common configuration items for optimization purpose
    //  do not do it if terminate flag was specified, to avoid exception
    if( !bTerminate )
    {
        try
        {
            PreloadConfigTrees();
        }
        catch(com::sun::star::uno::Exception &e)
        {
            bTerminate = sal_True;
            rtl::OUString sError = rtl::OUString::createFromAscii("Unable to retrieve application configuration data: ");
            sError += e.Message;
            Application::Abort(sError);
        }
    }

    //  The only step that should be done if terminate flag was specified
    //  Typically called by the plugin only
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "start Installer::InitializeInstallation()" );
    Installer* pInstaller = new Installer;
    pInstaller->InitializeInstallation( Application::GetAppFileName() );
    delete pInstaller;
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "end Installer::InitializeInstallation()" );

    if( !bTerminate )
    {
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "start create SvtPathOptions/SvtCJKOptions" );
        SvtPathOptions* pPathOptions = new SvtPathOptions;
        SvtCJKOptions* pCJKOPptions = new SvtCJKOptions(sal_True);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "end create SvtPathOptions/SvtCJKOptions" );
        registerServices( xSMgr );

        OUString        aDescription;
        Sequence< Any > aSeq( 1 );

        if ( pOfficeAcceptThread )
            aDescription = pOfficeAcceptThread->GetDescriptionString();
        else
            pCmdLineArgs->GetPortalConnectString( aDescription );
        aSeq[0] <<= aDescription;

        Reference < XComponent > xWrapper( xSMgr->createInstanceWithArguments( DEFINE_CONST_UNICODE(
                                                "com.sun.star.office.OfficeWrapper" ), aSeq ),
                                        UNO_QUERY );

        // code from SfxApplicationClass::Main copied!!
        {
            RTL_LOGFILE_CONTEXT( aLog, "SfxApplicationClass::Main()" );
            Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

            Reference< XConnectionBroker >  xServiceManagerBroker;
            Reference< XConnectionBroker >  xPalmPilotManagerBroker;

            RemoteControl aControl;

            // the shutdown icon sits in the systray and allows the user to keep
            // the office instance running for quicker restart
            // this will only be activated if -quickstart was specified on cmdline
            sal_Bool bQuickstart = pCmdLineArgs->IsQuickstart();
            Sequence< Any > aSeq( 1 );
            aSeq[0] <<= bQuickstart;
            Reference < XComponent > xQuickstart( xSMgr->createInstanceWithArguments(
                                                    DEFINE_CONST_UNICODE( "com.sun.star.office.Quickstart" ), aSeq ),
                                                  UNO_QUERY );

            if ( pCmdLineArgs->IsPlugin() )
            {
                OUString    aAcceptString( RTL_CONSTASCII_USTRINGPARAM( "pipe,name=soffice_plugin" ));
                OUString    aUserIdent;
                OSecurity   aSecurity;

                aSecurity.getUserIdent( aUserIdent );
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
                Reference< XTypeDetection > xTypeDetection( xSMgr->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ))),
                                                            UNO_QUERY );
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
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "call Application::Execute()" );
            if ( nAcquireCount )
                Application::AcquireSolarMutex( nAcquireCount );

            // call Application::Execute to process messages in vcl message loop
            Execute();

            // remove temp directory
            removeTemporaryDirectory();

            if( xPalmPilotManagerBroker.is() )
                xPalmPilotManagerBroker->stopAccepting();
            if( xServiceManagerBroker.is() )
                xServiceManagerBroker->stopAccepting();

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
        }

        xWrapper->dispose();
        xWrapper = 0;

        delete pCJKOPptions;
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

IMPL_LINK( Desktop, OpenClients_Impl, void*, pvoid )
{
    OpenClients();
    CloseStartupScreen();

    return 0;
}

void Desktop::OpenClients()
{
    // check if a document has been recovered - if there is one of if a document was loaded by cmdline, no default document
    // should be created
    Reference < XComponent > xFirst;
    BOOL bLoaded = FALSE;

    CommandLineArgs* pArgs = GetCommandLineArgs();
    SvtInternalOptions  aInternalOptions;

    if ( !pArgs->IsServer() && !aInternalOptions.IsRecoveryListEmpty() )
    {
        // crash recovery
        sal_Bool bUserCancel = sal_False;
        ::rtl::OUString sURL;
        ::rtl::OUString sFilter;
        ::rtl::OUString sTempName;

        Reference< XComponentLoader > xDesktop(
                ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );

        // create the parameter array
        Sequence < PropertyValue > aArgs( 4 );
        aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");
        aArgs[1].Name = ::rtl::OUString::createFromAscii("AsTemplate");
        aArgs[2].Name = ::rtl::OUString::createFromAscii("FilterName");
        aArgs[3].Name = ::rtl::OUString::createFromAscii("SalvagedFile");

        // mark it as a user request
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:user");

        while(  !aInternalOptions.IsRecoveryListEmpty() && !bUserCancel )
        {
            // Read and delete top recovery item from list
            aInternalOptions.PopRecoveryItem( sURL, sFilter, sTempName );

            INetURLObject aURL( sURL );

            sal_Bool bIsURL = aURL.GetProtocol() != INET_PROT_NOT_VALID;
            String sRealFileName( sURL );
            String sTempFileName( sTempName );

            String aMsg( DesktopResId( STR_RECOVER_QUERY ) );
            aMsg.SearchAndReplaceAscii( "$1", sRealFileName );
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

                    // remove saved copy
                    ::utl::UCBContentHelper::Kill( sTempFileName );
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
                        aInternalOptions.PopRecoveryItem( sURL, sFilter, sTempName );
                        ::utl::UCBContentHelper::Kill( sTempName );
                    }

                    break;
                }
            }
        }
    }

    // check for open parameters
    String aEmptyStr;
    ::rtl::OUString aOpenList;
    if ( pArgs->GetOpenList( aOpenList ) )
    {
        bLoaded = TRUE;
        ApplicationEvent* pAppEvt = new ApplicationEvent( aEmptyStr, aEmptyStr,
                                        APPEVENT_OPEN_STRING,
                                        aOpenList );
        HandleAppEvent( *pAppEvt );
        delete pAppEvt;
    }


    // check for print parameters
    ::rtl::OUString aPrintList;
    if ( pArgs->GetPrintList( aPrintList ) )
    {
        bLoaded = TRUE;
        ApplicationEvent* pAppEvt = new ApplicationEvent( aEmptyStr, aEmptyStr,
                                        APPEVENT_PRINT_STRING,
                                        aPrintList );
        HandleAppEvent( *pAppEvt );
        delete pAppEvt;
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
    String aName;
    if ( !aName.Len() )
    {
        aName = String( DEFINE_CONST_UNICODE("private:factory/" ) );
        SvtModuleOptions aOpt;
        if ( aOpt.IsWriter() )
            aName += DEFINE_CONST_UNICODE("swriter");
        else if ( aOpt.IsCalc() )
            aName += DEFINE_CONST_UNICODE("scalc");
        else if ( aOpt.IsImpress() )
            aName += DEFINE_CONST_UNICODE("simpress");
        else if ( aOpt.IsDraw() )
            aName += DEFINE_CONST_UNICODE("sdraw");
        else
            return;
    }

    Sequence < PropertyValue > aNoArgs;
    Reference< XComponentLoader > xDesktop(
            ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
            ::com::sun::star::uno::UNO_QUERY );
    xDesktop->loadComponentFromURL( aName, ::rtl::OUString::createFromAscii( "_blank" ), 0, aNoArgs );
}


String GetURL_Impl( const String& rName )
{
    // if the filename is a physical name, it is the client file system, not the file system
    // of the machine where the office is running ( if this are different machines )
    // so in the remote case we can't handle relative filenames as arguments, because they
    // are parsed relative to the program path
    // the file system of the client is addressed through the "file:" protocol
    ::rtl::OUString aProgName, aTmp;
    ::vos::OStartupInfo aInfo;
    aInfo.getExecutableFile( aProgName );
    aTmp = aProgName;
    INetURLObject aObj( aTmp );
    bool bWasAbsolute;
    INetURLObject aURL = aObj.smartRel2Abs( rName, bWasAbsolute );
    return aURL.GetMainURL(INetURLObject::NO_DECODE);
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
        sal_Int32 nCount = rAppEvent.IsPrintEvent() ? 5 : 1;
        Sequence < PropertyValue > aArgs( nCount );
        aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");

        if ( rAppEvent.IsPrintEvent() )
        {
            aArgs[1].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            aArgs[2].Name = ::rtl::OUString::createFromAscii("OpenNewView");
            aArgs[3].Name = ::rtl::OUString::createFromAscii("Hidden");
            aArgs[4].Name = ::rtl::OUString::createFromAscii("Silent");
        }

        // mark request as user interaction from outside
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:OpenEvent");

        for( sal_uInt16 i=0; i<rAppEvent.GetParamCount(); i++ )
        {
            // get file name
            String aName( rAppEvent.GetParam(i) );

            // is the parameter a printername ?
            if( aName.Len()>1 && *aName.GetBuffer()=='@' )
            {
                aPrinterName = aName.Copy(1);
                continue;
            }

#ifdef WNT
            FATToVFat_Impl( aName );
#endif
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
            }

            // load the document
            Reference < XPrintable > xDoc ( xDesktop->loadComponentFromURL( aName, ::rtl::OUString::createFromAscii( "_blank" ), 0, aArgs ), UNO_QUERY );
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
    }
    else if ( rAppEvent.GetEvent() == "APPEAR" )
    {
        // find active task - the active task is always a visible task
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTasksSupplier >
                xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTask > xTask = xDesktop->getActiveTask();
        if ( !xTask.is() )
        {
            // get any task if there is no active one
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > xList = xDesktop->getTasks()->createEnumeration();
            if ( xList->hasMoreElements() )
                xList->nextElement() >>= xTask;
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
            // If the office was started the second time its command line arguments are sent through a pipe
            // connection to the first office. We want to reuse the quickstart option for the first office.
            // NOTICE: The quickstart service must be initialized inside the "main thread"!!!
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

void Desktop::OpenStartupScreen( const char* pLabelPrefix )
{
    RTL_LOGFILE_CONTEXT( aLog, "Desktop::OpenStartupScreen()" );

    if ( pLabelPrefix && !Application::IsRemoteServer() )
    {
        // versuchen, die Label-DLL zu erzeugen
        String aMgrName = String::CreateFromAscii( pLabelPrefix );
        aMgrName += String::CreateFromInt32(SOLARUPD); // aktuelle Versionsnummer
        m_pLabelResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));

        // keine separate Label-DLL vorhanden?
        if ( !m_pLabelResMgr )
        {
            // dann den ResMgr vom Executable verwenden
            m_pLabelResMgr = new ResMgr;
        }

        // Intro nur anzeigen, wenn normaler Start (kein Print/Server etc.)
        OUString aTmpString;
        CommandLineArgs* pCmdLine = GetCommandLineArgs();
        if ( !pCmdLine->IsInvisible() && !pCmdLine->IsQuickstart() && !pCmdLine->IsMinimized() &&
             !pCmdLine->IsEmbedding() && !pCmdLine->GetPrintList( aTmpString ) )
        {
            const USHORT nResId = RID_DEFAULTINTRO;
            ResId aIntroBmpRes( nResId, m_pLabelResMgr );
            m_pIntro = new IntroWindow_Impl( aIntroBmpRes );
        }
    }
}

void Desktop::CloseStartupScreen()
{
    RTL_LOGFILE_CONTEXT( aLog, "Desktop::CloseStartupScreen()" );
    delete m_pIntro;
    m_pIntro = 0;
}

