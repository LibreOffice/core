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


#include <algorithm>

#include <config_vclplug.h>

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
#include <cppuhelper/bootstrap.hxx>
#include <officecfg/Setup.hxx>
#include <osl/file.hxx>
#include <osl/module.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

#include <tools/rcid.h>

#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <map>

using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::ucb;

namespace desktop
{


static void configureUcb()
{
    // For backwards compatibility, in case some code still uses plain
    // createInstance w/o args directly to obtain an instance:
    UniversalContentBroker::create(comphelper::getProcessComponentContext());
}

void Desktop::InitApplicationServiceManager()
{
    Reference<XMultiServiceFactory> sm;
#ifdef ANDROID
    OUString aUnoRc( "file:///assets/program/unorc" );
    sm.set(
        cppu::defaultBootstrap_InitialComponentContext( aUnoRc )->getServiceManager(),
        UNO_QUERY_THROW);
#else
    sm.set(
        cppu::defaultBootstrap_InitialComponentContext()->getServiceManager(),
        UNO_QUERY_THROW);
#endif
    comphelper::setProcessServiceFactory(sm);
}

void Desktop::RegisterServices(Reference< XComponentContext > const & context)
{
    if( !m_bServicesRegistered )
    {
        // interpret command line arguments
        CommandLineArgs& rCmdLine = GetCommandLineArgs();

        // Headless mode for FAT Office, auto cancels any dialogs that popup
        if (rCmdLine.IsEventTesting())
            Application::EnableEventTestingMode();
        else if (rCmdLine.IsHeadless())
            Application::EnableHeadlessMode(false);

        // read accept string from configuration
        OUString conDcpCfg(
            officecfg::Setup::Office::ooSetupConnectionURL::get(context));
        if (!conDcpCfg.isEmpty()) {
            createAcceptor(conDcpCfg);
        }

        std::vector< OUString > const & conDcp = rCmdLine.GetAccept();
        for (std::vector< OUString >::const_iterator i(conDcp.begin());
             i != conDcp.end(); ++i)
        {
            createAcceptor(*i);
        }

        configureUcb();

        CreateTemporaryDirectory();
        m_bServicesRegistered = true;
    }
}

typedef std::map< OUString, css::uno::Reference<css::lang::XInitialization> > AcceptorMap;

namespace
{
    struct acceptorMap : public rtl::Static< AcceptorMap, acceptorMap > {};
    struct CurrentTempURL : public rtl::Static< OUString, CurrentTempURL > {};
}

static bool bAccept = false;

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
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference<XInitialization> rAcceptor(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.office.Acceptor", xContext),
            UNO_QUERY );
        if ( rAcceptor.is() )
        {
            try
            {
                rAcceptor->initialize( aSeq );
                rMap.insert(AcceptorMap::value_type(aAcceptString, rAcceptor));
            }
            catch (const css::uno::Exception& e)
            {
                // no error handling needed...
                // acceptor just won't come up
                SAL_WARN( "desktop.app", "Acceptor could not be created: " << e.Message);
            }
        }
        else
        {
            // there is already an acceptor with this description
            SAL_WARN( "desktop.app", "Acceptor already exists.");
        }
    }
}

class enable
{
    private:
    Sequence<Any> m_aSeq;
    public:
    enable() : m_aSeq(1) {
        m_aSeq[0] <<= true;
    }
    void operator() (const AcceptorMap::value_type& val) {
        if (val.second.is()) {
            val.second->initialize(m_aSeq);
        }
    }
};

void Desktop::enableAcceptors()
{
    if (!bAccept)
    {
        // from now on, all new acceptors are enabled
        bAccept = true;
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
    if (aAcceptString == "all") {
        rMap.clear();

    } else {
        // try to remove acceptor from map
        AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
        if (pIter != rMap.end() ) {
            // remove reference from map
            // this is the last reference and the acceptor will be destructed
            rMap.erase(aAcceptString);
        } else {
            SAL_WARN( "desktop.app", "Found no acceptor to remove");
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
    OUString aTempBaseURL;
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
            aMsg = OUString( aResId );
        else
            aMsg = "The path manager is not available.\n";
        e.Message = aMsg + e.Message;
        throw;
    }

    // set temp base directory
    if ( aTempBaseURL.endsWith( "/" ) )
        aTempBaseURL = aTempBaseURL.copy( 0, aTempBaseURL.getLength() - 1 );

    OUString aRet;
    OUString aTempPath( aTempBaseURL );

    // create new current temporary directory
    osl::FileBase::getSystemPathFromFileURL( aTempBaseURL, aRet );
    ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
    aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    if ( aTempPath.isEmpty() )
    {
        ::osl::File::getTempDirURL( aTempBaseURL );

        if ( aTempBaseURL.endsWith( "/" ) )
            aTempBaseURL = aTempBaseURL.copy( 0, aTempBaseURL.getLength() - 1 );

        aTempPath = aTempBaseURL;
        ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
        aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    }

    // set new current temporary directory
    if (osl::FileBase::getFileURLFromSystemPath( aTempPath, aRet )
        != osl::FileBase::E_None)
    {
        aRet.clear();
    }
    CurrentTempURL::get() = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
    // remove current temporary directory
    OUString &rCurrentTempURL = CurrentTempURL::get();
    if ( !rCurrentTempURL.isEmpty() )
    {
        ::utl::UCBContentHelper::Kill( rCurrentTempURL );
    }
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
