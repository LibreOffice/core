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
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <uno/current_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/module.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <tools/rcid.h>

#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <unotools/startoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>


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

static void configureUcb()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (sb93797) ::configureUcb" );

    // For backwards compatibility, in case some code still uses plain
    // createInstance w/o args directly to obtain an instance:
    UniversalContentBroker::create(comphelper::getProcessComponentContext());

#ifdef GNOME_VFS_ENABLED
    // Instantiate GNOME-VFS UCP in the thread that initialized GNOME in order
    // to avoid a deadlock that may occure in case the UCP gets initialized from
    // a different thread (which may happen when calling remotely via UNO); this
    // is not a fix, just a workaround:
    Reference< XCurrentContext > xCurrentContext(getCurrentContext());
    Any aValue(xCurrentContext->getValueByName("system.desktop-environment"));
    OUString aDesktopEnvironment;
    if ((aValue >>= aDesktopEnvironment) && aDesktopEnvironment == "GNOME")
    {
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext())->
            registerContentProvider(
                Reference<XContentProvider>(
                    comphelper::getProcessServiceFactory()->createInstance(
                        "com.sun.star.ucb.GnomeVFSContentProvider"),
                    UNO_QUERY_THROW),
                ".*", false);
    }
#endif // GNOME_VFS_ENABLED
}

Reference< XMultiServiceFactory > Desktop::CreateApplicationServiceManager()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createApplicationServiceManager" );

#ifdef ANDROID
    rtl::OUString aUnoRc( OUString( "file:///assets/program/unorc"  ) );
    return Reference<XMultiServiceFactory>(
        cppu::defaultBootstrap_InitialComponentContext( aUnoRc )->getServiceManager(),
        UNO_QUERY_THROW);
#else
    return Reference<XMultiServiceFactory>(
        cppu::defaultBootstrap_InitialComponentContext()->getServiceManager(),
        UNO_QUERY_THROW);
#endif
}

void Desktop::DestroyApplicationServiceManager( Reference< XMultiServiceFactory >& xSMgr )
{
    Reference< XPropertySet > xProps( xSMgr, UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            Reference< XComponent > xComp;
            if (xProps->getPropertyValue( OUString( "DefaultContext" )) >>= xComp )
            {
                xComp->dispose();
            }
        }
        catch (const UnknownPropertyException&)
        {
        }
    }
}

void Desktop::RegisterServices()
{
    if( !m_bServicesRegistered )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::registerServices" );

        // read command line parameters
        sal_Bool        bHeadlessMode = sal_False;

        // interpret command line arguments
        CommandLineArgs& rCmdLine = GetCommandLineArgs();

        // Headless mode for FAT Office
        bHeadlessMode   = rCmdLine.IsHeadless();
        if ( bHeadlessMode )
            Application::EnableHeadlessMode(false);

        // read accept string from configuration
        rtl::OUString conDcpCfg(SvtStartOptions().GetConnectionURL());
        if (!conDcpCfg.isEmpty()) {
            createAcceptor(conDcpCfg);
        }

        std::vector< ::rtl::OUString > const & conDcp = rCmdLine.GetAccept();
        for (std::vector< ::rtl::OUString >::const_iterator i(conDcp.begin());
             i != conDcp.end(); ++i)
        {
            createAcceptor(*i);
        }

        configureUcb();

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
    if (pIter == rMap.end() )
    {
        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= aAcceptString;
        aSeq[1] <<= bAccept;
        Reference<XInitialization> rAcceptor(
            ::comphelper::getProcessServiceFactory()->createInstance(
            OUString("com.sun.star.office.Acceptor" )), UNO_QUERY );
        if ( rAcceptor.is() )
        {
            try
            {
                rAcceptor->initialize( aSeq );
                rMap.insert(AcceptorMap::value_type(aAcceptString, rAcceptor));
            }
            catch (const com::sun::star::uno::Exception&)
            {
                // no error handling needed...
                // acceptor just won't come up
                OSL_FAIL("Acceptor could not be created.");
            }
        }
        else
        {
            // there is already an acceptor with this description
            OSL_FAIL("Acceptor already exists.");
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
            OSL_FAIL("Found no acceptor to remove");
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
    catch (RuntimeException& e)
    {
        // Catch runtime exception here: We have to add language dependent info
        // to the exception message. Fallback solution uses hard coded string.
        OUString aMsg;
        DesktopResId aResId( STR_BOOTSTRAP_ERR_NO_PATHSET_SERVICE );
        aResId.SetRT( RSC_STRING );
        if ( aResId.GetResMgr()->IsAvailable( aResId ))
            aMsg = String( aResId );
        else
            aMsg = OUString( "The path manager is not available.\n" );
        e.Message = aMsg + e.Message;
        throw;
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

    ::rtl::OUString aRet;
    ::rtl::OUString aTempPath( aTempBaseURL );

    // create new current temporary directory
    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTempBaseURL, aRet );
    ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
    aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    if ( aTempPath.isEmpty() )
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
