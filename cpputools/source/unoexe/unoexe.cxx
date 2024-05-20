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

#include <stdio.h>
#include <mutex>
#include <string_view>

#include <sal/main.h>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

#include <rtl/process.h>
#include <rtl/ref.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/connection/Acceptor.hpp>
#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <utility>

using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;
using namespace com::sun::star::connection;
using namespace com::sun::star::bridge;
using namespace com::sun::star::container;

namespace unoexe
{

static bool s_quiet = false;

static void out( const char * pText )
{
    if (! s_quiet)
        fputs( pText, stderr );
}

static void out( std::u16string_view rText )
{
    if (! s_quiet)
    {
        OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
        fputs( aText.getStr(), stderr );
    }
}

const char arUsingText[] =
"\nusing:\n\n"
"uno [-c ComponentImplementationName -l LocationUrl | -s ServiceName]\n"
"    [-u uno:(socket[,host=HostName][,port=nnn]|pipe[,name=PipeName]);<protocol>;Name\n"
"        [--singleaccept] [--singleinstance]]\n"
"    [--quiet]\n"
"    [-- Argument1 Argument2 ...]\n";

/// @throws RuntimeException
static bool readOption( OUString * pValue, const char * pOpt,
                        sal_uInt32 * pnIndex, const OUString & aArg)
{
    static constexpr OUString dash(u"-"_ustr);
    if(!aArg.startsWith(dash))
        return false;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return false;

    if (aOpt.equalsIgnoreAsciiCase( aArg.subView(1, aArg.getLength()-1) ))
    {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= rtl_getAppCommandArgCount() || pValue->subView(1) == dash)
        {
            throw RuntimeException( "incomplete option \"-" + aOpt + "\" given!" );
        }
        SAL_INFO("cpputools.unoexe", "> identified option -" << pOpt << " = " << aArg);
        ++(*pnIndex);
        return true;
    }
    else if (aArg.indexOf(aOpt) == 1)
    {
        *pValue = aArg.copy(1 + aOpt.getLength());
        SAL_INFO("cpputools.unoexe", "> identified option -" << pOpt << " = " << aArg);
        ++(*pnIndex);

        return true;
    }
    return false;
}

static bool readOption( bool * pbOpt, const char * pOpt,
                        sal_uInt32 * pnIndex, std::u16string_view aArg)
{
    OUString aOpt = OUString::createFromAscii(pOpt);

    if(o3tl::starts_with(aArg, u"--") && aOpt == aArg.substr(2))
    {
        ++(*pnIndex);
        *pbOpt = true;
        SAL_INFO("cpputools.unoexe", "> identified option --" << pOpt);
        return true;
    }
    return false;
}

/// @throws Exception
template< class T >
static void createInstance(
    Reference< T > & rxOut,
    const Reference< XComponentContext > & xContext,
    const OUString & rServiceName )
{
    Reference< XMultiComponentFactory > xMgr( xContext->getServiceManager() );
    Reference< XInterface > x( xMgr->createInstanceWithContext( rServiceName, xContext ) );

    if (! x.is())
    {
        throw RuntimeException( "cannot get service instance \"" + rServiceName + "\"!" );
    }

    rxOut.set( x.get(), UNO_QUERY_THROW );
}

/// @throws Exception
static Reference< XInterface > loadComponent(
    const Reference< XComponentContext > & xContext,
    const OUString & rImplName, const OUString & rLocation )
{
    // determine loader to be used
    sal_Int32 nDot = rLocation.lastIndexOf( '.' );
    if (nDot <= 0 || nDot >= rLocation.getLength())
    {
        throw RuntimeException(
            "location \"" + rLocation + "\" has no extension!  Cannot determine loader to be used!" );
    }

    Reference< XImplementationLoader > xLoader;

    std::u16string_view aExt( rLocation.subView( nDot +1 ) );

    if (aExt == u"dll" || aExt == u"exe" || aExt == u"dylib" || aExt == u"so")
    {
        createInstance(
            xLoader, xContext, u"com.sun.star.loader.SharedLibrary"_ustr );
    }
    else if (aExt == u"jar" || aExt == u"class")
    {
        createInstance(
            xLoader, xContext, u"com.sun.star.loader.Java"_ustr );
    }
    else
    {
        throw RuntimeException(
            "unknown extension of \"" + rLocation + "\"!  No loader available!" );
    }

    Reference< XInterface > xInstance;

    // activate
    Reference< XInterface > xFactory( xLoader->activate(
        rImplName, OUString(), rLocation, Reference< XRegistryKey >() ) );
    if (xFactory.is())
    {
        Reference< XSingleComponentFactory > xCFac( xFactory, UNO_QUERY );
        if (xCFac.is())
        {
            xInstance = xCFac->createInstanceWithContext( xContext );
        }
        else
        {
            Reference< XSingleServiceFactory > xSFac( xFactory, UNO_QUERY );
            if (xSFac.is())
            {
                out( "\n> warning: ignoring context for implementation \"" );
                out( rImplName );
                out( "\"!" );
                xInstance = xSFac->createInstance();
            }
        }
    }

    if (! xInstance.is())
    {
        throw RuntimeException(
            "activating component \"" + rImplName + "\" from location \"" + rLocation + "\" failed!" );
    }

    return xInstance;
}

namespace {

class OInstanceProvider
    : public WeakImplHelper< XInstanceProvider >
{
    Reference< XComponentContext > _xContext;

    std::mutex                        _aSingleInstanceMutex;
    Reference< XInterface >           _xSingleInstance;
    bool                              _bSingleInstance;

    OUString                          _aImplName;
    OUString                          _aLocation;
    OUString                          _aServiceName;
    Sequence< Any >                   _aInitParams;

    OUString                          _aInstanceName;

    /// @throws Exception
    inline Reference< XInterface > createInstance() const;

public:
    OInstanceProvider( const Reference< XComponentContext > & xContext,
                       OUString aImplName, OUString aLocation,
                       OUString aServiceName, const Sequence< Any > & rInitParams,
                       bool bSingleInstance, OUString aInstanceName )
        : _xContext( xContext )
        , _bSingleInstance( bSingleInstance )
        , _aImplName(std::move( aImplName ))
        , _aLocation(std::move( aLocation ))
        , _aServiceName(std::move( aServiceName ))
        , _aInitParams( rInitParams )
        , _aInstanceName(std::move( aInstanceName ))
        {}

    // XInstanceProvider
    virtual Reference< XInterface > SAL_CALL getInstance( const OUString & rName ) override;
};

}

inline Reference< XInterface > OInstanceProvider::createInstance() const
{
    Reference< XInterface > xRet;
    if (!_aImplName.isEmpty()) // manually via loader
        xRet = loadComponent( _xContext, _aImplName, _aLocation );
    else // via service manager
        unoexe::createInstance( xRet, _xContext, _aServiceName );

    // opt XInit
    Reference< XInitialization > xInit( xRet, UNO_QUERY );
    if (xInit.is())
        xInit->initialize( _aInitParams );

    return xRet;
}

Reference< XInterface > OInstanceProvider::getInstance( const OUString & rName )
{
    try
    {
        if (_aInstanceName == rName)
        {
            Reference< XInterface > xRet;

            if (_aImplName.isEmpty() && _aServiceName.isEmpty())
            {
                OSL_ASSERT( rName == "uno.ComponentContext" );
                xRet = _xContext;
            }
            else if (_bSingleInstance)
            {
                if (! _xSingleInstance.is())
                {
                    std::lock_guard aGuard( _aSingleInstanceMutex );
                    if (! _xSingleInstance.is())
                    {
                        _xSingleInstance = createInstance();
                    }
                }
                xRet = _xSingleInstance;
            }
            else
            {
                xRet = createInstance();
            }

            return xRet;
        }
    }
    catch (Exception & rExc)
    {
        out( "\n> error: " );
        out( rExc.Message );
    }
    throw NoSuchElementException(
        "no such element \"" + rName + "\"!" );
}

namespace {

struct ODisposingListener : public WeakImplHelper< XEventListener >
{
    Condition cDisposed;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject & rEvt ) override;

    static void waitFor( const Reference< XComponent > & xComp );
};

}

void ODisposingListener::disposing( const EventObject & )
{
    cDisposed.set();
}

void ODisposingListener::waitFor( const Reference< XComponent > & xComp )
{
    rtl::Reference<ODisposingListener> xListener = new ODisposingListener;

    xComp->addEventListener( xListener );
    xListener->cDisposed.wait();
}

} // namespace unoexe

using namespace unoexe;

SAL_IMPLEMENT_MAIN()
{
    sal_uInt32 nCount = rtl_getAppCommandArgCount();
    if (nCount == 0)
    {
        out( arUsingText );
        return 0;
    }

    sal_Int32 nRet = 0;
    Reference< XComponentContext > xContext;


    try
    {
        OUString aImplName, aLocation, aServiceName, aUnoUrl;
        Sequence< OUString > aParams;
        bool bSingleAccept = false;
        bool bSingleInstance = false;

        // read command line arguments

        sal_uInt32 nPos = 0;
        // read up to arguments
        while (nPos < nCount)
        {
            OUString arg;

            rtl_getAppCommandArg(nPos, &arg.pData);

            if (arg == "--")
            {
                ++nPos;
                break;
            }

            if (!(readOption( &aImplName, "c", &nPos, arg)                ||
                  readOption( &aLocation, "l", &nPos, arg)                ||
                  readOption( &aServiceName, "s", &nPos, arg)             ||
                  readOption( &aUnoUrl, "u", &nPos, arg)                  ||
                  readOption( &s_quiet, "quiet", &nPos, arg)              ||
                  readOption( &bSingleAccept, "singleaccept", &nPos, arg) ||
                  readOption( &bSingleInstance, "singleinstance", &nPos, arg)))
            {
                throw RuntimeException(
                    "unexpected argument \"" + arg + "\"" );
            }
        }

        if (!(aImplName.isEmpty() || aServiceName.isEmpty()))
            throw RuntimeException(u"give component exOR service name!"_ustr );
        if (aImplName.isEmpty() && aServiceName.isEmpty())
        {
            if (! aUnoUrl.endsWithIgnoreAsciiCase( ";uno.ComponentContext" ))
                throw RuntimeException(
                    u"expected UNO-URL with instance name uno.ComponentContext!"_ustr );
            if (bSingleInstance)
                throw RuntimeException(
                    u"unexpected option --singleinstance!"_ustr );
        }
        if (!aImplName.isEmpty() && aLocation.isEmpty())
            throw RuntimeException(u"give component location!"_ustr );
        if (!aServiceName.isEmpty() && !aLocation.isEmpty())
            out( "\n> warning: service name given, will ignore location!" );

        // read component params
        aParams.realloc( nCount - nPos );
        OUString * pParams = aParams.getArray();

        sal_uInt32 nOffset = nPos;
        for ( ; nPos < nCount; ++nPos )
        {
            rtl_getAppCommandArg( nPos, &pParams[nPos -nOffset].pData );
        }

        xContext = defaultBootstrap_InitialComponentContext();

        // accept, instantiate, etc.

        if (!aUnoUrl.isEmpty()) // accepting connections
        {
            if (aUnoUrl.getLength() < 10 || !aUnoUrl.startsWithIgnoreAsciiCase( "uno:" ))
            {
                throw RuntimeException(u"illegal uno url given!"_ustr );
            }

            sal_Int32 nIndex = 4; // skip initial "uno:"
            bool bTooFewTokens {false};
            const OUString aConnectDescr{ aUnoUrl.getToken( 0, ';', nIndex ) }; // uno:CONNECTDESCR;iiop;InstanceName
            if (nIndex<0) bTooFewTokens = true;
            const OUString aUnoUrlToken{ aUnoUrl.getToken( 0, ';', nIndex ) };
            if (nIndex<0) bTooFewTokens = true;
            const OUString aInstanceName{ aUnoUrl.getToken( 0, ';', nIndex ) };

            // Exactly 3 tokens are required
            if (bTooFewTokens || nIndex>0)
            {
                throw RuntimeException(u"illegal uno url given!"_ustr );
            }

            Reference< XAcceptor > xAcceptor = Acceptor::create(xContext);

            // init params
            Sequence< Any > aInitParams( aParams.getLength() );
            Any * pInitParams = aInitParams.getArray();
            for ( sal_Int32 i = aParams.getLength(); i--; )
            {
                pInitParams[i] <<= aParams[i];
            }

            // instance provider
            Reference< XInstanceProvider > xInstanceProvider( new OInstanceProvider(
                xContext, aImplName, aLocation, aServiceName, aInitParams,
                bSingleInstance, aInstanceName ) );

            // coverity[loop_top] - not really an infinite loop, we can be instructed to exit via the connection
            for (;;)
            {
                // accepting
                out( "\n> accepting " );
                out( aConnectDescr );
                out( "..." );
                Reference< XConnection > xConnection( xAcceptor->accept( aConnectDescr ) );
                out( "connection established." );

                Reference< XBridgeFactory > xBridgeFactory;
                createInstance(
                    xBridgeFactory, xContext,
                    u"com.sun.star.bridge.BridgeFactory"_ustr );

                // bridge
                Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                    OUString(), aUnoUrlToken,
                    xConnection, xInstanceProvider ) );

                if (bSingleAccept)
                {
                    Reference< XComponent > xComp( xBridge, UNO_QUERY_THROW );
                    ODisposingListener::waitFor( xComp );
                    xComp->dispose();
                        // explicitly dispose the remote bridge so that it joins
                        // on all spawned threads before process exit (see
                        // binaryurp/source/bridge.cxx for details)
                    break;
                }
            }
        }
        else // no uno url
        {
            Reference< XInterface > xInstance;
            if (!aImplName.isEmpty()) // manually via loader
                xInstance = loadComponent( xContext, aImplName, aLocation );
            else // via service manager
                createInstance( xInstance, xContext, aServiceName );

            // execution
            Reference< XMain > xMain( xInstance, UNO_QUERY );
            if (xMain.is())
            {
                nRet = xMain->run( aParams );
            }
            else
            {
                Reference< XComponent > xComp( xInstance, UNO_QUERY );
                if (xComp.is())
                    xComp->dispose();
                throw RuntimeException( u"component does not export interface \"com.sun.star.lang.XMain\"!"_ustr );
            }
        }
    }
    catch (Exception & rExc)
    {
        out( "\n> error: " );
        out( rExc.Message );
        out( "\n> dying..." );
        nRet = 1;
    }

    // cleanup
    Reference< XComponent > xComp( xContext, UNO_QUERY );
    if (xComp.is())
        xComp->dispose();

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
