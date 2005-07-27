/*************************************************************************
 *
 *  $RCSfile: appinit.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2005-07-27 15:28:26 $
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
#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
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
#ifndef  _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XCONTENTPROVIDERFACTORY_HPP_
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#endif
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
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

#include <cppuhelper/bootstrap.hxx>
#include <tools/urlobj.hxx>

#include <rtl/logfile.hxx>
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
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
using namespace ::com::sun::star::ucb;

namespace desktop
{

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

    bool ret =
        ::ucb::ContentBroker::initialize( xServiceFactory, aArgs ) != false;

    // register GnomeUCP if necessary
    ::ucb::ContentBroker* cb = ::ucb::ContentBroker::get();
    if(cb) {
        try {
            Reference< XCurrentContext > xCurrentContext(
                getCurrentContext());
            if (xCurrentContext.is())
            {
                Any aValue = xCurrentContext->getValueByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                       "system.desktop-environment" ) )
                );
                rtl::OUString aDesktopEnvironment;
                if ((aValue >>= aDesktopEnvironment)
                    && aDesktopEnvironment.equalsAscii("GNOME"))
                {
                    Reference<XContentProviderManager> xCPM =
                        cb->getContentProviderManagerInterface();
#if 0

                    Reference<XContentProviderFactory> xCPF(
                        xServiceFactory->createInstance(
                            rtl::OUString::createFromAscii(
                                "com.sun.star.ucb.ContentProviderProxyFactory")),
                        UNO_QUERY);
                    if(xCPF.is())
                        xCPM->registerContentProvider(
                            xCPF->createContentProvider(
                                rtl::OUString::createFromAscii(
                                    "com.sun.star.ucb.GnomeVFSContentProvider"
                                )
                            ),
                            rtl::OUString::createFromAscii(".*"),
                            false);
#else

            // Workaround for P1 #124597#.  Instanciate GNOME-VFS-UCP in the thread that initialized
             // GNOME in order to avoid a deadlock that may occure in case UCP gets initialized from
            // a different thread. The latter may happen when calling the Office remotely via UNO.
            // THIS IS NOT A FIX, JUST A WORKAROUND!

                    Reference<XContentProvider> xCP(
                        xServiceFactory->createInstance(
                            rtl::OUString::createFromAscii(
                                "com.sun.star.ucb.GnomeVFSContentProvider")),
                        UNO_QUERY);
                    if(xCP.is())
                        xCPM->registerContentProvider(
                            xCP,
                            rtl::OUString::createFromAscii(".*"),
                            false);
#endif
                }
            }
        } catch (RuntimeException e) {
        }
    }

    return ret;;
}

Reference< XMultiServiceFactory > Desktop::CreateApplicationServiceManager()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createApplicationServiceManager" );

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
    if( !m_bServicesRegistered )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::registerServices" );

        // read command line parameters
        ::rtl::OUString conDcp;
        ::rtl::OUString aClientDisplay;
        ::rtl::OUString aTmpString;
        sal_Bool        bHeadlessMode = sal_False;

        // interpret command line arguments
        CommandLineArgs* pCmdLine = GetCommandLineArgs();

        // read accept string from configuration
        conDcp = SvtStartOptions().GetConnectionURL();

        if ( pCmdLine->GetAcceptString( aTmpString ))
            conDcp = aTmpString;

        // Headless mode for FAT Office
        bHeadlessMode   = pCmdLine->IsHeadless();
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
        if ( pCmdLine->IsServer() )
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

        CreateTemporaryDirectory();
        m_bServicesRegistered = true;
    }
}

namespace
{
    struct acceptorMap : public rtl::Static< AcceptorMap, acceptorMap > {};
    struct mtxAccMap : public rtl::Static< osl::Mutex, mtxAccMap > {};
    struct CurrentTempURL : public rtl::Static< String, CurrentTempURL > {};
}

static sal_Bool bAccept = sal_False;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
    // make sure nobody adds an acceptor whle we create one...
    osl::MutexGuard aGuard(mtxAccMap::get());
    // check whether the requested acceptor already exists
    AcceptorMap &rMap = acceptorMap::get();
    AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
    if (pIter == rMap.end() ) {

        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= aAcceptString;
        aSeq[1] <<= bAccept;
        Reference<XInitialization> rAcceptor(
            ::comphelper::getProcessServiceFactory()->createInstance(
            OUString::createFromAscii( "com.sun.star.office.Acceptor" )), UNO_QUERY );
        if ( rAcceptor.is() ) {
            try{
                rAcceptor->initialize( aSeq );
                rMap.insert(AcceptorMap::value_type(aAcceptString, rAcceptor));
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
    osl::MutexGuard aGuard(mtxAccMap::get());
    if (!bAccept)
    {
        // from now on, all new acceptors are enabled
        bAccept = sal_True;
        // enable existing acceptors by calling initialize(true)
        // on all existing acceptors
        AcceptorMap &rMap = acceptorMap::get();
        std::for_each(rMap.begin(), rMap.end(), enable());
    }
}

void Desktop::destroyAcceptor(const OUString& aAcceptString)
{
    osl::MutexGuard aGuard(mtxAccMap::get());
    // special case stop all acceptors
    AcceptorMap &rMap = acceptorMap::get();
    if (aAcceptString.compareToAscii("all") == 0) {
        rMap.clear();

    } else {
        // try to remove acceptor from map
        AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
        if (pIter != rMap.end() ) {
            // remove reference from map
            // this is the last reference and the acceptor will be destructed
            rMap.erase(aAcceptString);
        } else {
            OSL_ENSURE(sal_False, "Found no acceptor to remove");
        }
    }
}


void Desktop::DeregisterServices()
{
    // stop all acceptors by clearing the map
    acceptorMap::get().clear();
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
    CurrentTempURL::get() = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::removeTemporaryDirectory" );

    // remove current temporary directory
    String &rCurrentTempURL = CurrentTempURL::get();
    if ( rCurrentTempURL.Len() > 0 )
    {
        if ( ::utl::UCBContentHelper::Kill( rCurrentTempURL ) )
            SvtInternalOptions().SetCurrentTempURL( String() );
    }
}

} // namespace desktop
