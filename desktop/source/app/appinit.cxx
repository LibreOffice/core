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

#include <app.hxx>
#include <dp_shared.hxx>
#include "cmdlineargs.hxx"
#include <strings.hrc>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <officecfg/Setup.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>

#include <iostream>
#include <map>

#if defined EMSCRIPTEN
#include <bindings_uno.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
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
#elif defined(IOS)
    OUString uri( "$APP_DATA_DIR" );
    rtl_bootstrap_expandMacros( &uri.pData );
    OUString aUnoRc("file://" + uri  + "/unorc");
    sm.set(
           cppu::defaultBootstrap_InitialComponentContext( aUnoRc )->getServiceManager(),
           UNO_QUERY_THROW);
#else
    sm.set(
        cppu::defaultBootstrap_InitialComponentContext()->getServiceManager(),
        UNO_QUERY_THROW);
#endif
    comphelper::setProcessServiceFactory(sm);
#if defined EMSCRIPTEN
    init_unoembind_uno();
#endif
}

void Desktop::RegisterServices()
{
    if( m_bServicesRegistered )
        return;

    // interpret command line arguments
    CommandLineArgs& rCmdLine = GetCommandLineArgs();

    // Headless mode for FAT Office, auto cancels any dialogs that popup
    if (rCmdLine.IsHeadless())
        Application::EnableHeadlessMode(false);

    // read accept string from configuration
    OUString conDcpCfg(
        officecfg::Setup::Office::ooSetupConnectionURL::get());
    if (!conDcpCfg.isEmpty()) {
        createAcceptor(conDcpCfg);
    }

    std::vector< OUString > const & conDcp = rCmdLine.GetAccept();
    for (auto const& elem : conDcp)
    {
        createAcceptor(elem);
    }

    configureUcb();

    CreateTemporaryDirectory();
    m_bServicesRegistered = true;
}

typedef std::map< OUString, css::uno::Reference<css::lang::XInitialization> > AcceptorMap;

namespace
{
    AcceptorMap& acceptorMap()
    {
        static AcceptorMap SINGLETON;
        return SINGLETON;
    }
    OUString& CurrentTempURL()
    {
        static OUString SINGLETON;
        return SINGLETON;
    }
}

static bool bAccept = false;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
    // check whether the requested acceptor already exists
    AcceptorMap &rMap = acceptorMap();
    AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
    if (pIter != rMap.end() )
    {
        // there is already an acceptor with this description
        SAL_WARN( "desktop.app", "Acceptor already exists.");
        return;
    }

    Sequence< Any > aSeq{ Any(aAcceptString), Any(bAccept) };
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference<XInitialization> rAcceptor(
        xContext->getServiceManager()->createInstanceWithContext("com.sun.star.office.Acceptor", xContext),
        UNO_QUERY );
    if ( rAcceptor.is() )
    {
        try
        {
            rAcceptor->initialize( aSeq );
            rMap.emplace(aAcceptString, rAcceptor);
        }
        catch (const css::uno::Exception&)
        {
            // no error handling needed...
            // acceptor just won't come up
            TOOLS_WARN_EXCEPTION( "desktop.app", "Acceptor could not be created");
        }
    }
    else
    {
        ::std::cerr << "UNO Remote Protocol acceptor could not be created, presumably because it has been disabled in configuration." << ::std::endl;
    }
}

namespace {

class enable
{
    private:
    Sequence<Any> m_aSeq{ Any(true) };
    public:
    enable() = default;
    void operator() (const AcceptorMap::value_type& val) {
        if (val.second.is()) {
            val.second->initialize(m_aSeq);
        }
    }
};

}

// enable acceptors
IMPL_STATIC_LINK_NOARG(Desktop, EnableAcceptors_Impl, void*, void)
{
    if (!bAccept)
    {
        // from now on, all new acceptors are enabled
        bAccept = true;
        // enable existing acceptors by calling initialize(true)
        // on all existing acceptors
        AcceptorMap &rMap = acceptorMap();
        std::for_each(rMap.begin(), rMap.end(), enable());
    }
}

void Desktop::destroyAcceptor(const OUString& aAcceptString)
{
    // special case stop all acceptors
    AcceptorMap &rMap = acceptorMap();
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
    acceptorMap().clear();
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
        OUString aMsg = DpResId(STR_BOOTSTRAP_ERR_NO_PATHSET_SERVICE);
        e.Message = aMsg + e.Message;
        throw;
    }

    // create new current temporary directory
    OUString aTempPath = ::utl::SetTempNameBaseDirectory( aTempBaseURL );
    if ( aTempPath.isEmpty()
         && ::osl::File::getTempDirURL( aTempBaseURL ) == osl::FileBase::E_None )
    {
        aTempPath = ::utl::SetTempNameBaseDirectory( aTempBaseURL );
    }

    // set new current temporary directory
    OUString aRet;
    if (osl::FileBase::getFileURLFromSystemPath( aTempPath, aRet )
        != osl::FileBase::E_None)
    {
        aRet.clear();
    }
    CurrentTempURL() = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
    // remove current temporary directory
    OUString &rCurrentTempURL = CurrentTempURL();
    if ( !rCurrentTempURL.isEmpty() )
    {
        ::utl::UCBContentHelper::Kill( rCurrentTempURL );
    }
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
