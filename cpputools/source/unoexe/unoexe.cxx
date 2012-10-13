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
#include <vector>

#include "sal/main.h"
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/module.h>

#include <rtl/process.h>
#include <rtl/string.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <uno/environment.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

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

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;
using ::rtl::OUStringBuffer;

namespace unoexe
{

static sal_Bool isFileUrl(const OUString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

static OUString convertToFileUrl(const OUString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return fileName;
    }

    OUString uUrlFileName;
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None) {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, fileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(fileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}

static sal_Bool s_quiet = false;

//--------------------------------------------------------------------------------------------------
static inline void out( const sal_Char * pText )
{
    if (! s_quiet)
        fprintf( stderr, "%s", pText );
}
//--------------------------------------------------------------------------------------------------
static inline void out( const OUString & rText )
{
    if (! s_quiet)
    {
        OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "%s", aText.getStr() );
    }
}

//--------------------------------------------------------------------------------------------------
static const char arUsingText[] =
"\nusing:\n\n"
"uno [-c ComponentImplementationName -l LocationUrl | -s ServiceName]\n"
"    [-ro ReadOnlyRegistry1] [-ro ReadOnlyRegistry2] ... [-rw ReadWriteRegistry]\n"
"    [-u uno:(socket[,host=HostName][,port=nnn]|pipe[,name=PipeName]);<protocol>;Name\n"
"        [--singleaccept] [--singleinstance]]\n"
"    [--quiet]\n"
"    [-- Argument1 Argument2 ...]\n";

//--------------------------------------------------------------------------------------------------
static sal_Bool readOption( OUString * pValue, const sal_Char * pOpt,
                            sal_uInt32 * pnIndex, const OUString & aArg)
    throw (RuntimeException)
{
    const OUString dash(RTL_CONSTASCII_USTRINGPARAM("-"));
    if(aArg.indexOf(dash) != 0)
        return sal_False;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return sal_False;

    if (aOpt.equalsIgnoreAsciiCase( aArg.copy(1) ))
    {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= rtl_getAppCommandArgCount() || pValue->copy(1).equals(dash))
        {
            OUStringBuffer buf( 32 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("incomplete option \"-") );
            buf.appendAscii( pOpt );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" given!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
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
            return sal_True;
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

        return sal_True;
    }
    return sal_False;
}
//--------------------------------------------------------------------------------------------------
static sal_Bool readOption( sal_Bool * pbOpt, const sal_Char * pOpt,
                            sal_uInt32 * pnIndex, const OUString & aArg)
{
    const OUString dashdash(RTL_CONSTASCII_USTRINGPARAM("--"));
    OUString aOpt = OUString::createFromAscii(pOpt);

    if(aArg.indexOf(dashdash) == 0 && aOpt.equals(aArg.copy(2)))
    {
        ++(*pnIndex);
        *pbOpt = sal_True;
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option --" );
        out( pOpt );
#endif
        return sal_True;
    }
    return sal_False;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
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
        static sal_Bool s_bSet = sal_False;
        if (! s_bSet)
        {
            MutexGuard aGuard( Mutex::getGlobalMutex() );
            if (! s_bSet)
            {
                Reference< XSet > xSet( xMgr, UNO_QUERY );
                if (xSet.is())
                {
                    Reference< XMultiServiceFactory > xSF( xMgr, UNO_QUERY );
                    // acceptor
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( "acceptor.uno" SAL_DLLEXTENSION ),
                        OUString(),
                        OUString( "com.sun.star.comp.io.Acceptor" ),
                        xSF, Reference< XRegistryKey >(),
                        "acceptor" ) ) );
                    // connector
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( "connector.uno" SAL_DLLEXTENSION ),
                        OUString(),
                        OUString( "com.sun.star.comp.io.Connector" ),
                        xSF, Reference< XRegistryKey >(),
                        "connector" ) ) );
                    // bridge factory
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( "binaryurp.uno" SAL_DLLEXTENSION ),
                        OUString(),
                        OUString( "com.sun.star.comp.bridge.BridgeFactory" ),
                        xSF, Reference< XRegistryKey >(),
                        "binaryurp" ) ) );
                }
                s_bSet = sal_True;
            }
        }
        x = xMgr->createInstanceWithContext( rServiceName, xContext );
    }

    if (! x.is())
    {
        OUStringBuffer buf( 64 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot get service instance \"") );
        buf.append( rServiceName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }

    rxOut = Reference< T >::query( x );
    if (! rxOut.is())
    {
        OUStringBuffer buf( 64 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("service instance \"") );
        buf.append( rServiceName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" does not support demanded interface \"") );
        const Type & rType = ::getCppuType( (const Reference< T > *)0 );
        buf.append( rType.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }
}
//--------------------------------------------------------------------------------------------------
static Reference< XSimpleRegistry > nestRegistries(
    const Reference< XSimpleRegistry > & xReadWrite,
    const Reference< XSimpleRegistry > & xReadOnly )
    throw (Exception)
{
    Reference< XSimpleRegistry > xReg( createNestedRegistry() );
    if (! xReg.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no nested registry service!" ) ),
            Reference< XInterface >() );
    }

    Reference< XInitialization > xInit( xReg, UNO_QUERY );
    if (! xInit.is())
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("nested registry does not export interface \"com.sun.star.lang.XInitialization\"!" ) ), Reference< XInterface >() );

    Sequence< Any > aArgs( 2 );
    aArgs[0] <<= xReadWrite;
    aArgs[1] <<= xReadOnly;
    xInit->initialize( aArgs );

    return xReg;
}
//--------------------------------------------------------------------------------------------------
static Reference< XSimpleRegistry > openRegistry(
    const OUString & rURL,
    sal_Bool bReadOnly, sal_Bool bCreate )
    throw (Exception)
{
    Reference< XSimpleRegistry > xNewReg( createSimpleRegistry() );
    if (! xNewReg.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no simple registry service!" ) ),
            Reference< XInterface >() );
    }

    try
    {
        xNewReg->open( convertToFileUrl(rURL), bReadOnly, bCreate );
        if (xNewReg->isValid())
            return xNewReg;
        else
            xNewReg->close();
    }
    catch (Exception &)
    {
    }

    out( "\n> warning: cannot open registry \"" );
    out( rURL );
    if (bReadOnly)
        out( "\" for reading, ignoring!" );
    else
        out( "\" for reading and writing, ignoring!" );
    return Reference< XSimpleRegistry >();
}
//--------------------------------------------------------------------------------------------------
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

        if (aExt.compareToAscii( "dll" ) == 0 ||
            aExt.compareToAscii( "exe" ) == 0 ||
            aExt.compareToAscii( "dylib" ) == 0 ||
            aExt.compareToAscii( "so" ) == 0)
        {
            createInstance(
                xLoader, xContext, OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") ) );
        }
        else if (aExt.compareToAscii( "jar" ) == 0 ||
                 aExt.compareToAscii( "class" ) == 0)
        {
            createInstance(
                xLoader, xContext, OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.Java") ) );
        }
        else
        {
            OUStringBuffer buf( 64 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unknown extension of \"") );
            buf.append( rLocation );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!  No loader available!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
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
            OUStringBuffer buf( 64 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("activating component \"") );
            buf.append( rImplName );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" from location \"") );
            buf.append( rLocation );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" failed!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
        }

        return xInstance;
    }
    else
    {
        OUStringBuffer buf( 64 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("location \"") );
        buf.append( rLocation );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" has no extension!  Cannot determine loader to be used!") );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class OInstanceProvider
    : public WeakImplHelper1< XInstanceProvider >
{
    Reference< XComponentContext > _xContext;

    Mutex                             _aSingleInstanceMutex;
    Reference< XInterface >           _xSingleInstance;
    sal_Bool                          _bSingleInstance;

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
                       sal_Bool bSingleInstance, const OUString & rInstanceName )
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
        throw (NoSuchElementException, RuntimeException);
};
//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
Reference< XInterface > OInstanceProvider::getInstance( const OUString & rName )
    throw (NoSuchElementException, RuntimeException)
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
    OUStringBuffer buf( 64 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("no such element \"") );
    buf.append( rName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
    throw NoSuchElementException( buf.makeStringAndClear(), Reference< XInterface >() );
}

//==================================================================================================
struct ODisposingListener : public WeakImplHelper1< XEventListener >
{
    Condition cDisposed;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject & rEvt )
        throw (RuntimeException);

    //----------------------------------------------------------------------------------------------
    static void waitFor( const Reference< XComponent > & xComp );
};
//__________________________________________________________________________________________________
void ODisposingListener::disposing( const EventObject & )
    throw (RuntimeException)
{
    cDisposed.set();
}
//--------------------------------------------------------------------------------------------------
void ODisposingListener::waitFor( const Reference< XComponent > & xComp )
{
    ODisposingListener * pListener = new ODisposingListener();
    Reference< XEventListener > xListener( pListener );

    xComp->addEventListener( xListener );
    pListener->cDisposed.wait();
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//##################################################################################################
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
        vector< OUString > aReadOnlyRegistries;
        Sequence< OUString > aParams;
        sal_Bool bSingleAccept = sal_False;
        sal_Bool bSingleInstance = sal_False;

        //#### read command line arguments #########################################################

        bool bOldRegistryMimic = false;
        bool bNewRegistryMimic = false;
        OUString aReadWriteRegistry;

        sal_uInt32 nPos = 0;
        // read up to arguments
        while (nPos < nCount)
        {
            OUString arg;

            rtl_getAppCommandArg(nPos, &arg.pData);

            const OUString dashdash(RTL_CONSTASCII_USTRINGPARAM("--"));
            if (dashdash == arg)
            {
                ++nPos;
                break;
            }

            if (readOption( &aImplName, "c", &nPos, arg)                ||
                readOption( &aLocation, "l", &nPos, arg)                ||
                readOption( &aServiceName, "s", &nPos, arg)             ||
                readOption( &aUnoUrl, "u", &nPos, arg)                  ||
                readOption( &s_quiet, "quiet", &nPos, arg)              ||
                readOption( &bSingleAccept, "singleaccept", &nPos, arg) ||
                readOption( &bSingleInstance, "singleinstance", &nPos, arg))
            {
                continue;
            }
            OUString aRegistry;
            if (readOption( &aRegistry, "ro", &nPos, arg))
            {
                aReadOnlyRegistries.push_back( aRegistry );
                bNewRegistryMimic = true;
                continue;
            }
            if (readOption( &aReadWriteRegistry, "rw", &nPos, arg))
            {
                bNewRegistryMimic = true;
                continue;
            }
            if (readOption( &aRegistry, "r", &nPos, arg))
            {
                aReadOnlyRegistries.push_back( aRegistry );
                aReadWriteRegistry = aRegistry;
                out( "\n> warning: DEPRECATED use of option -r, use -ro or -rw!" );
                bOldRegistryMimic = true;
                continue;
            }

            // else illegal argument
            OUStringBuffer buf( 64 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unexpected parameter \"") );
            buf.append(arg);
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
        }

        if (bOldRegistryMimic) // last one was set to be read-write
        {
            aReadOnlyRegistries.pop_back();
            if (bOldRegistryMimic && bNewRegistryMimic)
            {
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("mixing with DEPRECATED registry options!") ),
                    Reference< XInterface >() );
            }
        }

        if (!(aImplName.isEmpty() || aServiceName.isEmpty()))
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("give component exOR service name!" ) ), Reference< XInterface >() );
        if (aImplName.isEmpty() && aServiceName.isEmpty())
        {
            if (! aUnoUrl.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(";uno.ComponentContext") ))
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                  "expected UNO-URL with instance name "
                                  "uno.ComponentContext!") ),
                    Reference<XInterface>() );
            if (bSingleInstance)
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                  "unexpected option --singleinstance!") ),
                    Reference<XInterface>() );
        }
        if (!aImplName.isEmpty() && aLocation.isEmpty())
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("give component location!" ) ), Reference< XInterface >() );
        if (!aServiceName.isEmpty() && !aLocation.isEmpty())
            out( "\n> warning: service name given, will ignore location!" );

        // read component params
        aParams.realloc( nCount - nPos );
        OUString * pParams = aParams.getArray();

        sal_uInt32 nOffset = nPos;
        for ( ; nPos < nCount; ++nPos )
        {
            if (rtl_getAppCommandArg( nPos, &pParams[nPos -nOffset].pData )
                != osl_Process_E_None)
            {
                OSL_ASSERT(false);
            }
        }

        if ((!aReadOnlyRegistries.empty()) ||
            aReadWriteRegistry.getLength() > 0)
        {
            //#### create registry #############################################

            Reference< XSimpleRegistry > xRegistry;

            // ReadOnly registries
            for ( size_t nReg = 0; nReg < aReadOnlyRegistries.size(); ++nReg )
            {
#if OSL_DEBUG_LEVEL > 1
                out( "\n> trying to open ro registry: " );
                out( OUStringToOString(
                         aReadOnlyRegistries[ nReg ],
                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                Reference< XSimpleRegistry > xNewReg(
                    openRegistry(
                        aReadOnlyRegistries[ nReg ], sal_True, sal_False ) );
                if (xNewReg.is())
                    xRegistry = (xRegistry.is() ? nestRegistries(
                                     xNewReg, xRegistry ) : xNewReg);
            }
            if (!aReadWriteRegistry.isEmpty())
            {
#if OSL_DEBUG_LEVEL > 1
                out( "\n> trying to open rw registry: " );
                out( OUStringToOString(
                         aReadWriteRegistry,
                         RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                // ReadWrite registry
                Reference< XSimpleRegistry > xNewReg(
                    openRegistry( aReadWriteRegistry, sal_False, sal_True ) );
                if (xNewReg.is())
                    xRegistry = (xRegistry.is()
                                 ? nestRegistries( xNewReg, xRegistry )
                                 : xNewReg);
            }

            OSL_ASSERT( xRegistry.is() );
            xContext = bootstrap_InitialComponentContext( xRegistry );
        }
        else // defaulting
        {
            xContext = defaultBootstrap_InitialComponentContext();
        }

        //#### accept, instanciate, etc. ###########################################################

        if (!aUnoUrl.isEmpty()) // accepting connections
        {
            sal_Int32 nIndex = 0, nTokens = 0;
            do { aUnoUrl.getToken( 0, ';', nIndex ); nTokens++; } while( nIndex != -1 );
            if (nTokens != 3 || aUnoUrl.getLength() < 10 ||
                !aUnoUrl.copy( 0, 4 ).equalsIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM("uno:") ) ))
            {
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("illegal uno url given!" ) ), Reference< XInterface >() );
            }
            nIndex = 0;
            OUString aConnectDescr( aUnoUrl.getToken( 0, ';', nIndex ).copy( 4 ) ); // uno:CONNECTDESCR;iiop;InstanceName
            OUString aInstanceName( aUnoUrl.getToken( 1, ';', nIndex ) );

            Reference< XAcceptor > xAcceptor;
            createInstance(
                xAcceptor, xContext,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor") ) );

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
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory") ) );

                // bridge
                Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                    OUString(), aUnoUrlToken,
                    xConnection, xInstanceProvider ) );

                if (bSingleAccept)
                {
                    Reference< XComponent > xComp( xBridge, UNO_QUERY );
                    if (! xComp.is())
                        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("bridge factory does not export interface \"com.sun.star.lang.XComponent\"!" ) ), Reference< XInterface >() );
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
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("component does not export interface interface \"com.sun.star.lang.XMain\"!" ) ), Reference< XInterface >() );
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

    out( "\n" );
    return nRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
