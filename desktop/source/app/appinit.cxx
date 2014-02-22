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

#include <tools/rcid.h>

#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>

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
    SAL_INFO( "desktop.app", "desktop (sb93797) ::configureUcb" );

    
    
    UniversalContentBroker::create(comphelper::getProcessComponentContext());

#if ENABLE_GNOME_VFS
    try {
        
        
        
        
        Reference< XCurrentContext > xCurrentContext(getCurrentContext());
        Any aValue(xCurrentContext->getValueByName("system.desktop-environment"));
        OUString aDesktopEnvironment;
        if ((aValue >>= aDesktopEnvironment) && aDesktopEnvironment == "GNOME")
        {
            Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            UniversalContentBroker::create(xContext)
                ->registerContentProvider(
                        Reference<XContentProvider>(
                        xContext->getServiceManager()->createInstanceWithContext(
                                "com.sun.star.ucb.GnomeVFSContentProvider", xContext),
                        UNO_QUERY_THROW),
                        ".*", false);
        }
    }
    catch ( const Exception & )
    {
        SAL_WARN( "desktop.app", "missing gnome-vfs component to initialize thread workaround" );
    }
#endif 
}

void Desktop::InitApplicationServiceManager()
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::createApplicationServiceManager" );
    Reference<XMultiServiceFactory> sm;
#ifdef ANDROID
    OUString aUnoRc( "file:
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
        SAL_INFO( "desktop.app", "desktop (cd100003) ::registerServices" );

        
        CommandLineArgs& rCmdLine = GetCommandLineArgs();

        
        sal_Bool bHeadlessMode = rCmdLine.IsHeadless();
        if ( bHeadlessMode )
            Application::EnableHeadlessMode(false);

        
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

namespace
{
    struct acceptorMap : public rtl::Static< AcceptorMap, acceptorMap > {};
    struct CurrentTempURL : public rtl::Static< OUString, CurrentTempURL > {};
}

static sal_Bool bAccept = sal_False;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
    
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
            catch (const com::sun::star::uno::Exception& e)
            {
                
                
                SAL_WARN( "desktop.app", "Acceptor could not be created: " << e.Message);
            }
        }
        else
        {
            
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
    SAL_INFO( "desktop.app", "desktop (lo119109) Desktop::enableAcceptors");
    if (!bAccept)
    {
        
        bAccept = sal_True;
        
        
        AcceptorMap &rMap = acceptorMap::get();
        std::for_each(rMap.begin(), rMap.end(), enable());
    }
}

void Desktop::destroyAcceptor(const OUString& aAcceptString)
{
    
    AcceptorMap &rMap = acceptorMap::get();
    if (aAcceptString.equalsAscii("all")) {
        rMap.clear();

    } else {
        
        AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
        if (pIter != rMap.end() ) {
            
            
            rMap.erase(aAcceptString);
        } else {
            SAL_WARN( "desktop.app", "Found no acceptor to remove");
        }
    }
}


void Desktop::DeregisterServices()
{
    
    acceptorMap::get().clear();
}

void Desktop::CreateTemporaryDirectory()
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::createTemporaryDirectory" );

    OUString aTempBaseURL;
    try
    {
        SvtPathOptions aOpt;
        aTempBaseURL = aOpt.GetTempPath();
    }
    catch (RuntimeException& e)
    {
        
        
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

    
    sal_Int32 nLength = aTempBaseURL.getLength();
    if ( aTempBaseURL.matchAsciiL( "/", 1, nLength-1 ) )
        aTempBaseURL = aTempBaseURL.copy( 0, nLength - 1 );

    OUString aRet;
    OUString aTempPath( aTempBaseURL );

    
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

    
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTempPath, aRet );
    CurrentTempURL::get() = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
    SAL_INFO( "desktop.app", "desktop (cd100003) ::removeTemporaryDirectory" );

    
    OUString &rCurrentTempURL = CurrentTempURL::get();
    if ( !rCurrentTempURL.isEmpty() )
    {
        ::utl::UCBContentHelper::Kill( rCurrentTempURL );
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
