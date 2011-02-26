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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <algorithm>

#include "app.hxx"
#include "cmdlineargs.hxx"
#include "desktopresid.hxx"
#include "desktop.hrc"
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>


#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#include <uno/current_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/module.h>
#include <osl/security.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <comphelper/regpathhelper.hxx>
#include <tools/debug.hxx>
#include <tools/tempfile.hxx>
#include <ucbhelper/configurationkeys.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/rcid.h>

#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/svapp.hxx>
#include <unotools/startoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>


#define DEFINE_CONST_OUSTRING(CONSTASCII)       OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

#define DESKTOP_TEMPDIRNAME                     "soffice.tmp"

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::ucb;

using ::rtl::OUString;

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
    osl::Security().getUserIdent(aPipe);

    rtl::OUStringBuffer aPortal;
    if (rPortalConnect.getLength() != 0)
    {
        aPortal.append(sal_Unicode(','));
        aPortal.append(rPortalConnect);
    }

    Sequence< Any > aArgs(6);
    aArgs[0]
        <<= bServer ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UCB_CONFIGURATION_KEY1_SERVER)) :
                      rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UCB_CONFIGURATION_KEY1_LOCAL));
    aArgs[1]
        <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UCB_CONFIGURATION_KEY2_OFFICE));
    aArgs[2] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PIPE"));
    aArgs[3] <<= aPipe;
    aArgs[4] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PORTAL"));
    aArgs[5] <<= aPortal.makeStringAndClear();

    bool ret =
        ::ucbhelper::ContentBroker::initialize( xServiceFactory, aArgs ) != false;

#ifdef GNOME_VFS_ENABLED
    // register GnomeUCP if necessary
    ::ucbhelper::ContentBroker* cb = ::ucbhelper::ContentBroker::get();
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
                    && aDesktopEnvironment.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("GNOME")))
                {
                    Reference<XContentProviderManager> xCPM =
                        cb->getContentProviderManagerInterface();


            // Workaround for P1 #124597#.  Instanciate GNOME-VFS-UCP in the thread that initialized
             // GNOME in order to avoid a deadlock that may occure in case UCP gets initialized from
            // a different thread. The latter may happen when calling the Office remotely via UNO.
            // THIS IS NOT A FIX, JUST A WORKAROUND!

                    try
                    {
                        Reference<XContentProvider> xCP(
                            xServiceFactory->createInstance(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.ucb.GnomeVFSContentProvider"))),
                            UNO_QUERY);
                        if(xCP.is())
                            xCPM->registerContentProvider(
                                xCP,
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".*")),
                                false);
                    } catch (...)
                    {
                    }
                }
            }
        } catch (RuntimeException &e) {
        }
    }
#endif // GNOME_VFS_ENABLED

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
            throw com::sun::star::uno::Exception(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RegisterServices, configureUcb")), NULL);
        }

        CreateTemporaryDirectory();
        m_bServicesRegistered = true;
    }
}

namespace
{
    struct acceptorMap : public rtl::Static< AcceptorMap, acceptorMap > {};
    struct CurrentTempURL : public rtl::Static< String, CurrentTempURL > {};
}

static sal_Bool bAccept = sal_False;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
    // check whether the requested acceptor already exists
    AcceptorMap &rMap = acceptorMap::get();
    AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
    if (pIter == rMap.end() ) {

        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= aAcceptString;
        aSeq[1] <<= bAccept;
        Reference<XInitialization> rAcceptor(
            ::comphelper::getProcessServiceFactory()->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.Acceptor" ))), UNO_QUERY );
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

    ::rtl::OUString aTempBaseURL;
    try
    {
        SvtPathOptions aOpt;
        aTempBaseURL = aOpt.GetTempPath();
    }
    catch ( RuntimeException& e )
    {
        // Catch runtime exception here: We have to add language dependent info
        // to the exception message. Fallback solution uses hard coded string.
        OUString aMsg;
        DesktopResId aResId( STR_BOOTSTRAP_ERR_NO_PATHSET_SERVICE );
        aResId.SetRT( RSC_STRING );
        if ( aResId.GetResMgr()->IsAvailable( aResId ))
            aMsg = String( aResId );
        else
            aMsg = OUString( RTL_CONSTASCII_USTRINGPARAM( "The path manager is not available.\n" ));
        e.Message = aMsg + e.Message;
        throw e;
    }

    // remove possible old directory and base directory
    SvtInternalOptions  aInternalOpt;

    // set temp base directory
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

        nLength = aTempBaseURL.getLength();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
