/*************************************************************************
 *
 *  $RCSfile: appinit.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 13:51:11 $
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

#include <algorithm>

#include "app.hxx"
#include "cmdlineargs.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif


#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include <cppuhelper/bootstrap.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#include <comphelper/regpathhelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _UCBHELPER_CONFIGURATIONKEYS_HXX_
#include <ucbhelper/configurationkeys.hxx>
#endif
#ifndef _Installer_hxx
#include <setup2/installer.hxx>
#endif

#include <cppuhelper/bootstrap.hxx>
#include <tools/urlobj.hxx>

#include <rtl/logfile.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/svapp.hxx>
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif


#define DEFINE_CONST_OUSTRING(CONSTASCII)       OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

#define DESKTOP_TEMPDIRNAME                     "soffice.tmp"

using namespace rtl;
using namespace vos;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;

namespace desktop
{

char const INSTALLER_INITFILENAME[] = "initialize.ini";

static String aCurrentTempURL;

// -----------------------------------------------------------------------------

static bool configureUcb(bool bServer, rtl::OUString const & rPortalConnect)
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (sb93797) ::configureUcb" );
    Reference< XMultiServiceFactory >
        xServiceFactory( comphelper::getProcessServiceFactory() );
    if (!xServiceFactory.is())
    {
        DBG_ERROR("configureUcb(): No XMultiServiceFactory");
        return false;
    }

    rtl::OUString aPipe;
    vos::OSecurity().getUserIdent(aPipe);

    rtl::OUStringBuffer aPortal;
    if (rPortalConnect.getLength() != 0)
    {
        aPortal.append(sal_Unicode(','));
        aPortal.append(rPortalConnect);
    }

    Sequence< Any > aArgs(6);
    aArgs[0]
        <<= rtl::OUString::createFromAscii(bServer ?
                                               UCB_CONFIGURATION_KEY1_SERVER :
                                               UCB_CONFIGURATION_KEY1_LOCAL);
    aArgs[1]
        <<= rtl::OUString::createFromAscii(UCB_CONFIGURATION_KEY2_OFFICE);
    aArgs[2] <<= rtl::OUString::createFromAscii("PIPE");
    aArgs[3] <<= aPipe;
    aArgs[4] <<= rtl::OUString::createFromAscii("PORTAL");
    aArgs[5] <<= aPortal.makeStringAndClear();

    return ::ucb::ContentBroker::initialize( xServiceFactory, aArgs ) != false;
}

sal_Bool Desktop::InitializeInstallation( const OUString& rAppFilename )
{
    OUString aAppPath;
    OUString aFinishInstallation;
    osl::FileBase::getFileURLFromSystemPath( rAppFilename, aFinishInstallation );

    OUStringBuffer aAppPathBuf( aFinishInstallation.getLength() + sizeof( INSTALLER_INITFILENAME ) );

    sal_Int32 nPos = aFinishInstallation.lastIndexOf( '/' );
    if ( nPos >= 0 )
        aAppPathBuf.append( aFinishInstallation.copy( 0, nPos ));
    else
        aAppPathBuf.append( aFinishInstallation );
    aAppPathBuf.appendAscii( "/" );
    aAppPathBuf.appendAscii( INSTALLER_INITFILENAME );
    aAppPath = aAppPathBuf.makeStringAndClear();

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
                (*(pfunc_InstallerInitializeInstallation)pInitFunc)( rAppFilename.getStr() );
            osl_unloadModule( aSetupModule );
        }

        return sal_True;
    }

    return sal_False;
}

Reference< XMultiServiceFactory > Desktop::CreateApplicationServiceManager()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createApplicationServiceManager" );

    OUString aUserDir;
    if ( GetCommandLineArgs()->GetUserDir( aUserDir ))
    {
        OUString aUserDirURL;

        if ( osl::FileBase::getFileURLFromSystemPath( aUserDir, aUserDirURL ) == 0 )
        {
            // now must be a valid file URL. For best results make absolute using
            OUString aProcessWorkDirURL;

            oslProcessError nProcessError = osl_getProcessWorkingDir( &aProcessWorkDirURL.pData );
            if ( nProcessError == osl_Process_E_None )
            {
                osl::FileBase::getAbsoluteFileURL( aProcessWorkDirURL, aUserDirURL, aUserDirURL );

                // now override the bootstrap setting:
                rtl::Bootstrap::set( OUString::createFromAscii( "UserInstallation" ), aUserDirURL );
            }
        }
    }

    try
    {
        Reference<XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        Reference<XMultiServiceFactory> xMS(xComponentContext->getServiceManager(), UNO_QUERY);

        return xMS;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return Reference< XMultiServiceFactory >();
}

void Desktop::DestroyApplicationServiceManager( Reference< XMultiServiceFactory >& xSMgr )
{
    Reference< XPropertySet > xProps( xSMgr, UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            Reference< XComponent > xComp;
            if (xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComp )
            {
                xComp->dispose();
            }
        }
        catch ( UnknownPropertyException& )
        {
        }
    }
}

void Desktop::RegisterServices( Reference< XMultiServiceFactory >& xSMgr )
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::registerServices" );

    // read command line parameters
    ::rtl::OUString conDcp;
    ::rtl::OUString aClientDisplay;
    ::rtl::OUString aUserDir;
    ::rtl::OUString aTmpString;
    sal_Bool        bHeadlessMode = sal_False;

    // interpret command line arguments
    CommandLineArgs* pCmdLine = GetCommandLineArgs();

    // read accept string from configuration
    if ( !Application::IsRemoteServer() )
        conDcp = SvtStartOptions().GetConnectionURL();

    if ( pCmdLine->GetAcceptString( aTmpString ))
        conDcp = aTmpString;
    pCmdLine->GetUserDir( aUserDir );

    // Headless mode for FAT Office
    bHeadlessMode   = pCmdLine->IsHeadless();
    if ( Application::IsRemoteServer() )
    {
        pCmdLine->GetClientDisplay( aClientDisplay );
    }

    if ( bHeadlessMode )
        Application::EnableHeadlessMode();

    if ( conDcp.getLength() > 0 )
    {
        // accept incoming connections (scripting and one rvp)
        RTL_LOGFILE_CONTEXT( aLog, "desktop (lo119109) desktop::Desktop::createAcceptor()" );
        createAcceptor(conDcp);
    }

    // improves parallel processing on Sun ONE Webtop
    // servicemanager up -> copy user installation
    if ( Application::IsRemoteServer() )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.portal.InstallUser" );
        Any aAny;
        Reference <XInterface> xRef =  xSMgr->createInstanceWithArguments(
            OUString::createFromAscii( "com.sun.star.portal.InstallUser" ),
            Sequence<Any>( &aAny, 1 ) );
    }
    else if ( pCmdLine->IsServer() )
    {
        // Check some mandatory environment states if "-server" is possible. Otherwise ignore
        // this parameter.
        Reference< com::sun::star::container::XContentEnumerationAccess > rContent( xSMgr , UNO_QUERY );
        if( rContent.is() )
        {
            OUString sPortalService = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.portal.InstallUser" ) );
            Reference < com::sun::star::container::XEnumeration > rEnum = rContent->createContentEnumeration( sPortalService );
            if ( !rEnum.is() )
            {
                // Reset server parameter so it is ignored in the furthermore startup process
                pCmdLine->SetBoolParam( CommandLineArgs::CMD_BOOLPARAM_SERVER, sal_False );
            }
        }
    }

    ::rtl::OUString aPortalConnect;
    bool bServer = (bool)pCmdLine->IsServer();

    pCmdLine->GetPortalConnectString( aPortalConnect );
    if ( !configureUcb( bServer, aPortalConnect ) )
    {
        DBG_ERROR( "Can't configure UCB" );
        throw com::sun::star::uno::Exception(rtl::OUString::createFromAscii("RegisterServices, configureUcb"), NULL);
    }

//  UCB_Helper::Initialize(); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    CreateTemporaryDirectory();
}

AcceptorMap     Desktop::m_acceptorMap;
osl::Mutex          Desktop::m_mtxAccMap;
static sal_Bool bAccept = sal_False;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
    // make sure nobody adds an acceptor whle we create one...
    osl::MutexGuard aGuard(m_mtxAccMap);
    // check whether the requested acceptor already exists
    AcceptorMap::const_iterator pIter = m_acceptorMap.find(aAcceptString);
    if (pIter == m_acceptorMap.end() ) {

        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= aAcceptString;
        aSeq[1] <<= bAccept;
        Reference<XInitialization> rAcceptor(
            ::comphelper::getProcessServiceFactory()->createInstance(
            OUString::createFromAscii( "com.sun.star.office.Acceptor" )), UNO_QUERY );
        if ( rAcceptor.is() ) {
            try{
                rAcceptor->initialize( aSeq );
                m_acceptorMap.insert(AcceptorMap::value_type(aAcceptString, rAcceptor));
            } catch (com::sun::star::uno::Exception&) {
            // no error handling needed...
            // acceptor just won't come up
            OSL_ENSURE(sal_False, "Acceptor could not be created.");
        }
    } else {
        // there is already an acceptor with this description
        OSL_ENSURE(sal_False, "Acceptor already exists.");
    }

    }
}

class enable
{
    private:
    Sequence<Any> m_aSeq;
    public:
    enable() : m_aSeq(1) {
        m_aSeq[0] <<= sal_True;
    }
    void operator() (const AcceptorMap::value_type& val) {
        if (val.second.is()) {
            val.second->initialize(m_aSeq);
        }
    }
};

void Desktop::enableAcceptors()
{
    RTL_LOGFILE_CONTEXT(aLog, "desktop (lo119109) Desktop::enableAcceptors");
    osl::MutexGuard aGuard(m_mtxAccMap);
    if (!bAccept)
    {
        // from now on, all new acceptors are enabled
        bAccept = sal_True;
        // enable existing acceptors by calling initialize(true)
        // on all existing acceptors
        std::for_each(m_acceptorMap.begin(), m_acceptorMap.end(), enable());
    }
}

void Desktop::destroyAcceptor(const OUString& aAcceptString)
{
    osl::MutexGuard aGuard(m_mtxAccMap);
    // special case stop all acceptors
    if (aAcceptString.compareToAscii("all") == 0) {
        m_acceptorMap.clear();

    } else {
        // try to remove acceptor from map
        AcceptorMap::const_iterator pIter = m_acceptorMap.find(aAcceptString);
        if (pIter != m_acceptorMap.end() ) {
            // remove reference from map
            // this is the last reference and the acceptor will be destructed
            m_acceptorMap.erase(aAcceptString);
        } else {
            OSL_ENSURE(sal_False, "Found no acceptor to remove");
        }
    }
}


void Desktop::DeregisterServices()
{
    // stop all acceptors by clearing the map
    m_acceptorMap.clear();
}

void Desktop::CreateTemporaryDirectory()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createTemporaryDirectory" );

    // remove possible old directory and base directory
    SvtPathOptions      aOpt;
    SvtInternalOptions  aInternalOpt;

    // set temp base directory
    ::rtl::OUString aTempBaseURL( aOpt.GetTempPath() );
    sal_Int32 nLength = aTempBaseURL.getLength();
    if ( aTempBaseURL.matchAsciiL( "/", 1, nLength-1 ) )
        aTempBaseURL = aTempBaseURL.copy( 0, nLength - 1 );

    String aOldTempURL = aInternalOpt.GetCurrentTempURL();
    if ( aOldTempURL.Len() > 0 )
    {
        // remove old temporary directory
        ::utl::UCBContentHelper::Kill( aOldTempURL );
    }

    String aRet;
    ::rtl::OUString aTempPath( aTempBaseURL );

    // create new current temporary directory
    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTempBaseURL, aRet );
    ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
    aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    if ( !aTempPath.getLength() )
    {
        ::osl::File::getTempDirURL( aTempBaseURL );

        sal_Int32 nLength = aTempBaseURL.getLength();
        if ( aTempBaseURL.matchAsciiL( "/", 1, nLength-1 ) )
            aTempBaseURL = aTempBaseURL.copy( 0, nLength - 1 );

        aTempPath = aTempBaseURL;
        ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
        aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    }

    // set new current temporary directory
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTempPath, aRet );
    aInternalOpt.SetCurrentTempURL( aRet );
    aCurrentTempURL = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::removeTemporaryDirectory" );

    // remove current temporary directory
    if ( aCurrentTempURL.Len() > 0 )
    {
        if ( ::utl::UCBContentHelper::Kill( aCurrentTempURL ) )
            SvtInternalOptions().SetCurrentTempURL( String() );
    }
}

} // namespace desktop
