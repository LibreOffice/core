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

#include "sal/main.h"
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

#include <rtl/process.h>
#include <rtl/string.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

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

using namespace std;
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

static inline void out( const sal_Char * pText )
{
    if (! s_quiet)
        fprintf( stderr, "%s", pText );
}

static inline void out( const OUString & rText )
{
    if (! s_quiet)
    {
        OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "%s", aText.getStr() );
    }
}

static const char arUsingText[] =
"\nusing:\n\n"
"uno [-c ComponentImplementationName -l LocationUrl | -s ServiceName]\n"
"    [-u uno:(socket[,host=HostName][,port=nnn]|pipe[,name=PipeName]);<protocol>;Name\n"
"        [--singleaccept] [--singleinstance]]\n"
"    [--quiet]\n"
"    [-- Argument1 Argument2 ...]\n";

static bool readOption( OUString * pValue, const sal_Char * pOpt,
                        sal_uInt32 * pnIndex, const OUString & aArg)
    throw (RuntimeException)
{
    const OUString dash("-");
    if(!aArg.startsWith(dash))
        return false;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return false;

    if (aOpt.equalsIgnoreAsciiCase( aArg.copy(1) ))
    {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= rtl_getAppCommandArgCount() || pValue->copy(1).equals(dash))
        {
            throw RuntimeException( "incomplete option \"-" + aOpt + "\" given!" );
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            out( "\n> identified option -" );
            out( pOpt );
            out( " = " );
            OString tmp = OUStringToOString(aArg, RTL_TEXTENCODING_ASCII_US);
              out( tmp.getStr() );
#endif
            ++(*pnIndex);
            return true;
        }
    }
      else if (aArg.indexOf(aOpt) == 1)
    {
        *pValue = aArg.copy(1 + aOpt.getLength());
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option -" );
        out( pOpt );
        out( " = " );
        OString tmp = OUStringToOString(aArg.copy(aOpt.getLength()), RTL_TEXTENCODING_ASCII_US);
        out( tmp.getStr() );
#endif
        ++(*pnIndex);

        return true;
    }
    return false;
}

static bool readOption( bool * pbOpt, const sal_Char * pOpt,
                        sal_uInt32 * pnIndex, const OUString & aArg)
{
    OUString aOpt = OUString::createFromAscii(pOpt);

    if(aArg.startsWith("--") && aOpt.equals(aArg.copy(2)))
    {
        ++(*pnIndex);
        *pbOpt = true;
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option --" );
        out( pOpt );
#endif
        return true;
    }
    return false;
}

template< class T >
void createInstance(
    Reference< T > & rxOut,
    const Reference< XComponentContext > & xContext,
    const OUString & rServiceName )
    throw (Exception)
{
    Reference< XMultiComponentFactory > xMgr( xContext->getServiceManager() );
    Reference< XInterface > x( xMgr->createInstanceWithContext( rServiceName, xContext ) );

    if (! x.is())
    {
        throw RuntimeException( "cannot get service instance \"" + rServiceName + "\"!" );
    }

    rxOut.set( x, UNO_QUERY );
    if (! rxOut.is())
    {
        const Type & rType = cppu::UnoType<T>::get();
        throw RuntimeException(
            "service instance \"" + rServiceName +
            "\" does not support demanded interface \"" +
            rType.getTypeName() + "\"!" );
    }
}

static Reference< XInterface > loadComponent(
    const Reference< XComponentContext > & xContext,
    const OUString & rImplName, const OUString & rLocation )
    throw (Exception)
{
    // determine loader to be used
    sal_Int32 nDot = rLocation.lastIndexOf( '.' );
    if (nDot > 0 && nDot < rLocation.getLength())
    {
        Reference< XImplementationLoader > xLoader;

        OUString aExt( rLocation.copy( nDot +1 ) );

        if (aExt == "dll" || aExt == "exe" || aExt == "dylib" || aExt == "so")
        {
            createInstance(
                xLoader, xContext, "com.sun.star.loader.SharedLibrary" );
        }
        else if (aExt == "jar" || aExt == "class")
        {
            createInstance(
                xLoader, xContext, "com.sun.star.loader.Java" );
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
                    out( "\n> warning: ignroing context for implementation \"" );
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
    else
    {
        throw RuntimeException(
            "location \"" + rLocation + "\" has no extension!  Cannot determine loader to be used!" );
    }
}

class OInstanceProvider
    : public WeakImplHelper< XInstanceProvider >
{
    Reference< XComponentContext > _xContext;

    Mutex                             _aSingleInstanceMutex;
    Reference< XInterface >           _xSingleInstance;
    bool                              _bSingleInstance;

    OUString                          _aImplName;
    OUString                          _aLocation;
    OUString                          _aServiceName;
    Sequence< Any >                   _aInitParams;

    OUString                          _aInstanceName;

    inline Reference< XInterface > createInstance() throw (Exception);

public:
    OInstanceProvider( const Reference< XComponentContext > & xContext,
                       const OUString & rImplName, const OUString & rLocation,
                       const OUString & rServiceName, const Sequence< Any > & rInitParams,
                       bool bSingleInstance, const OUString & rInstanceName )
        : _xContext( xContext )
        , _bSingleInstance( bSingleInstance )
        , _aImplName( rImplName )
        , _aLocation( rLocation )
        , _aServiceName( rServiceName )
        , _aInitParams( rInitParams )
        , _aInstanceName( rInstanceName )
        {}

    // XInstanceProvider
    virtual Reference< XInterface > SAL_CALL getInstance( const OUString & rName )
        throw (NoSuchElementException, RuntimeException, std::exception) override;
};

inline Reference< XInterface > OInstanceProvider::createInstance()
    throw (Exception)
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
    throw (NoSuchElementException, RuntimeException, std::exception)
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
                    MutexGuard aGuard( _aSingleInstanceMutex );
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

struct ODisposingListener : public WeakImplHelper< XEventListener >
{
    Condition cDisposed;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject & rEvt )
        throw (RuntimeException, std::exception) override;

    static void waitFor( const Reference< XComponent > & xComp );
};

void ODisposingListener::disposing( const EventObject & )
    throw (RuntimeException, std::exception)
{
    cDisposed.set();
}

void ODisposingListener::waitFor( const Reference< XComponent > & xComp )
{
    ODisposingListener * pListener = new ODisposingListener();
    Reference< XEventListener > xListener( pListener );

    xComp->addEventListener( xListener );
    pListener->cDisposed.wait();
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
            throw RuntimeException("give component exOR service name!" );
        if (aImplName.isEmpty() && aServiceName.isEmpty())
        {
            if (! aUnoUrl.endsWithIgnoreAsciiCase( ";uno.ComponentContext" ))
                throw RuntimeException(
                    "expected UNO-URL with instance name uno.ComponentContext!" );
            if (bSingleInstance)
                throw RuntimeException(
                    "unexpected option --singleinstance!" );
        }
        if (!aImplName.isEmpty() && aLocation.isEmpty())
            throw RuntimeException("give component location!" );
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

        // accept, instanciate, etc.

        if (!aUnoUrl.isEmpty()) // accepting connections
        {
            sal_Int32 nIndex = 0, nTokens = 0;
            do { aUnoUrl.getToken( 0, ';', nIndex ); nTokens++; } while( nIndex != -1 );
            if (nTokens != 3 || aUnoUrl.getLength() < 10 ||
                !aUnoUrl.copy( 0, 4 ).equalsIgnoreAsciiCase( "uno:" ))
            {
                throw RuntimeException("illegal uno url given!" );
            }
            nIndex = 0;
            OUString aConnectDescr( aUnoUrl.getToken( 0, ';', nIndex ).copy( 4 ) ); // uno:CONNECTDESCR;iiop;InstanceName
            OUString aInstanceName( aUnoUrl.getToken( 1, ';', nIndex ) );

            Reference< XAcceptor > xAcceptor = Acceptor::create(xContext);

            // init params
            Sequence< Any > aInitParams( aParams.getLength() );
            const OUString * p = aParams.getConstArray();
            Any * pInitParams = aInitParams.getArray();
            for ( sal_Int32 i = aParams.getLength(); i--; )
            {
                pInitParams[i] = makeAny( p[i] );
            }

            // instance provider
            Reference< XInstanceProvider > xInstanceProvider( new OInstanceProvider(
                xContext, aImplName, aLocation, aServiceName, aInitParams,
                bSingleInstance, aInstanceName ) );

            nIndex = 0;
            OUString aUnoUrlToken( aUnoUrl.getToken( 1, ';', nIndex ) );
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
                    "com.sun.star.bridge.BridgeFactory" );

                // bridge
                Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                    OUString(), aUnoUrlToken,
                    xConnection, xInstanceProvider ) );

                if (bSingleAccept)
                {
                    Reference< XComponent > xComp( xBridge, UNO_QUERY );
                    if (! xComp.is())
                        throw RuntimeException( "bridge factory does not export interface \"com.sun.star.lang.XComponent\"!" );
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
                throw RuntimeException( "component does not export interface interface \"com.sun.star.lang.XMain\"!" );
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

#if OSL_DEBUG_LEVEL > 1
    out( "\n" );
#endif
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
