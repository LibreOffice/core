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


#include <stdio.h>
#include <math.h>
#include <string>
#include <boost/unordered_map.hpp>
#include <map>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/module.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/conditn.hxx>
#include <osl/time.h>

#ifdef SAL_W32
#include <windows.h>
#else
#include <sys/times.h>
#include <unistd.h>
#endif

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <uno/environment.hxx>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/test/performance/XPerformanceTest.hpp>

#define NLOOP 200000000

using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;
using namespace com::sun::star::bridge;
using namespace com::sun::star::container;
using namespace com::sun::star::test::performance;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

#define SERVICENAME     "com.sun.star.test.performance.PerformanceTest"
#define IMPLNAME        "com.sun.star.comp.performance.PerformanceTest"

namespace benchmark_test
{

static inline sal_uInt32 getSystemTicks()
{
#ifdef SAL_W32
    return (sal_uInt32)GetTickCount();
#else // only UNX supported for now
    static sal_uInt32   nImplTicksPerSecond = 0;
    static double       dImplTicksPerSecond;
    static double       dImplTicksULONGMAX;

    struct tms          aTms;
    sal_uInt32 nTicks = (sal_uInt32)times( &aTms );

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = sysconf(_SC_CLK_TCK);
        dImplTicksPerSecond = nImplTicksPerSecond;
        dImplTicksULONGMAX  = (double)(sal_uInt32)ULONG_MAX;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    fTicks = fmod (fTicks, dImplTicksULONGMAX);

    return (sal_uInt32)fTicks;
#endif
}

//--------------------------------------------------------------------------------------------------
static void out( const sal_Char * pText, FILE * stream = stderr,
                 sal_Int32 nStart = -1, sal_Char cFillchar = ' ' )
{
    static sal_Int32 s_nPos = 0;

    sal_Char ar[2] = { cFillchar, 0 };
    while (s_nPos < nStart)
    {
        ::fprintf( stream, ar );
        ++s_nPos;
    }

    ::fprintf( stream, pText );

    for ( const sal_Char * p = pText; *p; ++p )
    {
        if (*p == '\n')
            s_nPos = 0;
        else
            ++s_nPos;
    }
}
//--------------------------------------------------------------------------------------------------
static inline void out( const OUString & rText, FILE * stream = stderr,
                        sal_Int32 nStart = -1, sal_Char cFillchar = ' ' )
{
    OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
    out( aText.getStr(), stream, nStart, cFillchar );
}
//--------------------------------------------------------------------------------------------------
static inline void out( double fVal, FILE * stream = stderr,
                        sal_Int32 nStart = -1, sal_Char cFillchar = ' ' )
{
    sal_Char ar[128];
    ::snprintf( ar, sizeof(ar), (fVal < 0.000001 ? "%g" : "%f"), fVal );
    out( ar, stream, nStart, cFillchar );
}
//--------------------------------------------------------------------------------------------------
static inline void out( sal_Int64 nVal, FILE * stream = stderr,
                        sal_Int32 nStart = -1, sal_Char cFillchar = ' ' )
{
    sal_Char ar[128];
    ::snprintf( ar, sizeof(ar), "%ld", nVal );
    out( ar, stream, nStart, cFillchar );
}

//==================================================================================================
Reference< XSingleServiceFactory > loadLibComponentFactory(
    const OUString & rLibName, const OUString & rImplName,
    const Reference< XMultiServiceFactory > & xSF, const Reference< XRegistryKey > & xKey )
{
    Reference< XSingleServiceFactory > xRet;

    OUStringBuffer aLibNameBuf( 32 );
#ifdef SAL_UNX
    aLibNameBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM("lib") );
    aLibNameBuf.append( rLibName );
    aLibNameBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM(".so") );
#else
    aLibNameBuf.append( rLibName );
    aLibNameBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM(".dll") );
#endif
    OUString aLibName( aLibNameBuf.makeStringAndClear() );
    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );

    if (lib)
    {
        void * pSym;

        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        if (pSym = osl_getSymbol( lib, aGetEnvName.pData ))
        {
            uno_Environment * pCurrentEnv = 0;
            uno_Environment * pEnv = 0;
            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))( &pEnvTypeName, &pEnv );

            sal_Bool bNeedsMapping =
                (pEnv || 0 != rtl_str_compare( pEnvTypeName, CPPU_CURRENT_LANGUAGE_BINDING_NAME ));

            OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

            if (bNeedsMapping)
            {
                if (! pEnv)
                    uno_getEnvironment( &pEnv, aEnvTypeName.pData, 0 );
                if (pEnv)
                {
                    OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
                    uno_getEnvironment( &pCurrentEnv, aCppEnvTypeName.pData, 0 );
                    if (pCurrentEnv)
                        bNeedsMapping = (pEnv != pCurrentEnv);
                }
            }

            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY) );
            if (pSym = osl_getSymbol( lib, aGetFactoryName.pData ))
            {
                OString aImplName( OUStringToOString( rImplName, RTL_TEXTENCODING_ASCII_US ) );

                if (bNeedsMapping)
                {
                    if (pEnv && pCurrentEnv)
                    {
                        Mapping aCurrent2Env( pCurrentEnv, pEnv );
                        Mapping aEnv2Current( pEnv, pCurrentEnv );

                        if (aCurrent2Env.is() && aEnv2Current.is())
                        {
                            void * pSMgr = aCurrent2Env.mapInterface(
                                xSF.get(), ::getCppuType( (const Reference< XMultiServiceFactory > *)0 ) );
                            void * pKey = aCurrent2Env.mapInterface(
                                xKey.get(), ::getCppuType( (const Reference< XRegistryKey > *)0 ) );

                            void * pSSF = (*((component_getFactoryFunc)pSym))(
                                aImplName.getStr(), pSMgr, pKey );

                            if (pKey)
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pKey );
                            if (pSMgr)
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pSMgr );

                            if (pSSF)
                            {
                                aEnv2Current.mapInterface(
                                    reinterpret_cast< void ** >( &xRet ),
                                    pSSF, ::getCppuType( (const Reference< XSingleServiceFactory > *)0 ) );
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pSSF );
                            }
                        }
                    }
                }
                else
                {
                    XSingleServiceFactory * pRet = (XSingleServiceFactory *)
                        (*((component_getFactoryFunc)pSym))(
                            aImplName.getStr(), xSF.get(), xKey.get() );
                    if (pRet)
                    {
                        xRet = pRet;
                        pRet->release();
                    }
                }
            }

            if (pEnv)
                (*pEnv->release)( pEnv );
            if (pCurrentEnv)
                (*pCurrentEnv->release)( pCurrentEnv );
        }

        // ========================= PREVIOUS VERSION =========================
        else
        {
            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(CREATE_COMPONENT_FACTORY_FUNCTION) );
            if (pSym = osl_getSymbol( lib, aGetFactoryName.pData ))
            {
                OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
                OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
                Mapping aUno2Cpp( aUnoEnvTypeName, aCppEnvTypeName );
                Mapping aCpp2Uno( aCppEnvTypeName, aUnoEnvTypeName );
                OSL_ENSURE( aUno2Cpp.is() && aCpp2Uno.is(), "### cannot get uno mappings!" );

                if (aUno2Cpp.is() && aCpp2Uno.is())
                {
                    uno_Interface * pUComponentFactory = 0;

                    uno_Interface * pUSFactory = (uno_Interface *)aCpp2Uno.mapInterface(
                        xSF.get(), ::getCppuType( (const Reference< XMultiServiceFactory > *)0 ) );
                    uno_Interface * pUKey = (uno_Interface *)aCpp2Uno.mapInterface(
                        xKey.get(), ::getCppuType( (const Reference< XRegistryKey > *)0 ) );

                    pUComponentFactory = (*((CreateComponentFactoryFunc)pSym))(
                        rImplName.getStr(), pUSFactory, pUKey );

                    if (pUKey)
                        (*pUKey->release)( pUKey );
                    if (pUSFactory)
                        (*pUSFactory->release)( pUSFactory );

                    if (pUComponentFactory)
                    {
                        XSingleServiceFactory * pXFactory =
                            (XSingleServiceFactory *)aUno2Cpp.mapInterface(
                                pUComponentFactory, ::getCppuType( (const Reference< XSingleServiceFactory > *)0 ) );
                        (*pUComponentFactory->release)( pUComponentFactory );

                        if (pXFactory)
                        {
                            xRet = pXFactory;
                            pXFactory->release();
                        }
                    }
                }
            }
        }

        if (! xRet.is())
            osl_unloadModule( lib );
    }

    return xRet;
}
//--------------------------------------------------------------------------------------------------
template< class T >
static void createInstance( Reference< T > & rxOut,
                            const Reference< XMultiServiceFactory > & xMgr,
                            const OUString & rServiceName )
    throw (RuntimeException)
{
    Reference< XInterface > x( xMgr->createInstance( rServiceName ), UNO_QUERY );

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
                    // acceptor
                    xSet->insert( makeAny( loadLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.Acceptor") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // connector
                    xSet->insert( makeAny( loadLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("connectr") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.Connector") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // iiop bridge
                    xSet->insert( makeAny( loadLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.various") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // bridge factory
                    xSet->insert( makeAny( loadLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.BridgeFactory") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // uno url resolver
                    xSet->insert( makeAny( loadLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("uuresolver") ),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.bridge.UnoUrlResolver") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // java loader
//                      xSet->insert( makeAny( loadLibComponentFactory(
//                          OUString( RTL_CONSTASCII_USTRINGPARAM("javaloader") ),
//                          OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.JavaComponentLoader") ),
//                          xMgr, Reference< XRegistryKey >() ) ) );
                }
                s_bSet = sal_True;
            }
        }
        x = xMgr->createInstance( rServiceName );
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
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//==================================================================================================
class TestImpl : public WeakImplHelper2< XServiceInfo, XMain >
{
    Reference< XMultiServiceFactory > _xSMgr;

    Reference< XInterface > _xDirect;
    Reference< XInterface > getDirect() throw (Exception);
    Reference< XInterface > resolveObject( const OUString & rUnoUrl ) throw (Exception);

public:
    TestImpl( const Reference< XMultiServiceFactory > & xSMgr );
    virtual ~TestImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // XMain
    virtual sal_Int32 SAL_CALL run( const Sequence< OUString > & rArgs ) throw (RuntimeException);
};

//##################################################################################################

//__________________________________________________________________________________________________
TestImpl::TestImpl( const Reference< XMultiServiceFactory > & xSMgr )
    : _xSMgr( xSMgr )
{
}
//__________________________________________________________________________________________________
TestImpl::~TestImpl()
{
}

//==================================================================================================
static Reference< XInterface > SAL_CALL TestImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( *new TestImpl( xSMgr ) );
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString TestImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool TestImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > TestImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return benchmark_test::getSupportedServiceNames();
}

//__________________________________________________________________________________________________
Reference< XInterface > TestImpl::getDirect()
    throw (Exception)
{
    if (! _xDirect.is())
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! _xDirect.is())
        {
            Reference< XSingleServiceFactory > xFac( loadLibComponentFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("perfobj") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.performance.PerformanceTestObject") ),
                _xSMgr, Reference< XRegistryKey >() ) );
            if (! xFac.is())
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("no test object available!") ), Reference< XInterface >() );
            _xDirect = xFac->createInstance();
        }
    }
    return _xDirect;
}
//--------------------------------------------------------------------------------------------------
Reference< XInterface > TestImpl::resolveObject( const OUString & rUnoUrl )
    throw (Exception)
{
    Reference< XUnoUrlResolver > xResolver;
    createInstance(
        xResolver, _xSMgr,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.UnoUrlResolver") ) );

    Reference< XInterface > xResolvedObject( xResolver->resolve( rUnoUrl ) );

    if (! xResolvedObject.is())
    {
        OUStringBuffer buf( 32 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot resolve object \"") );
        buf.append( rUnoUrl );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }

    return xResolvedObject;
}

//==================================================================================================
class TimeEntry
{
    sal_Int64           nLoop;
    sal_uInt32          nTicks;

public:
    TimeEntry()
        {}
    TimeEntry( sal_Int64 nLoop_, sal_uInt32 nTicks_ )
        : nLoop( nLoop_ )
        , nTicks( nTicks_ )
        {}

    inline double secPerCall() const
        { return (((double)nTicks) / (nLoop * 1000)); }

    double ratio( const TimeEntry & rEntry ) const;
};
//__________________________________________________________________________________________________
double TimeEntry::ratio( const TimeEntry & rEntry ) const
{
    double f = rEntry.nTicks * nLoop;
    if (f == 0.0)
    {
        return 0.0;
    }
    else
    {
        return (((double)(nTicks * rEntry.nLoop)) / f);
    }
}

//==================================================================================================
typedef std::map< std::string, TimeEntry > t_TimeEntryMap;

//==================================================================================================
struct TimingSheet
{
    t_TimeEntryMap      _entries;
    void insert( const sal_Char * pText, sal_Int64 nLoop, sal_uInt32 nTicks );
};
//__________________________________________________________________________________________________
void TimingSheet::insert( const sal_Char * pText, sal_Int64 nLoop, sal_uInt32 nTicks )
{
    _entries[ pText ] = TimeEntry( nLoop, nTicks );
}

//==================================================================================================
typedef boost::unordered_map< std::string, TimingSheet > t_TimingSheetMap;

//--------------------------------------------------------------------------------------------------
static void benchmark(
    TimingSheet & rSheet, const Reference< XInterface > & xInstance, sal_Int64 nLoop )
    throw (Exception)
{
    Reference< XPerformanceTest > xBench( xInstance, UNO_QUERY );
    if (! xBench.is())
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("illegal test object!") ), Reference< XInterface >() );

    sal_Int64 i;
    sal_uInt32 tStart, tEnd;

    const Type & rKnownType = ::getCppuType( (const Reference< XPerformanceTest > *)0 );
    const Type & rUnKnownType = ::getCppuType( (const Reference< XSet > *)0 );

    ComplexTypes aDummyStruct;

    //------------------------------------
    // oneway calls
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->async();
    sal_uInt32 tEndSend = getSystemTicks();
    xBench->sync();
    tEnd = getSystemTicks();
    rSheet.insert( "1a: sending simple oneway calls (no params, no return)", nLoop, tEndSend - tStart );
    rSheet.insert( "1b: simple oneway calls (no params, no return)", nLoop, tEnd - tStart );
    // synchron calls
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->sync();
    xBench->sync();
    tEnd = getSystemTicks();
    rSheet.insert( "1c: simple synchron calls (no params no return)", nLoop+1, tEnd - tStart );

    // acquire
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->acquire();
    tEnd = getSystemTicks();
    rSheet.insert( "2a: interface acquire() calls", nLoop, tEnd - tStart );
    // release
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->release();
    tEnd = getSystemTicks();
    rSheet.insert( "2b: interface release() calls", nLoop, tEnd - tStart );

    // queryInterface() for known type
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->queryInterface( rKnownType );
    tEnd = getSystemTicks();
    rSheet.insert( "2c: interface query for implemented type", nLoop, tEnd - tStart );
    // queryInterface() for unknown type
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->queryInterface( rUnKnownType );
    tEnd = getSystemTicks();
    rSheet.insert( "2d: interface query for unknown type", nLoop, tEnd - tStart );

    // create and forget objects
    Reference< XPerformanceTest > xBench2( xBench );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench2 = xBench2->createObject();
    tEnd = getSystemTicks();
    rSheet.insert( "3a: create and release test objects", nLoop, tEnd - tStart );

    // hold new objects
    Sequence< Reference< XInterface > > aSeq( nLoop / 100 );
    Reference< XInterface > * pSeq = aSeq.getArray();
    xBench2 = xBench;
    i = aSeq.getLength();
    tStart = getSystemTicks();
    while (i--)
        pSeq[i] = xBench2 = xBench2->createObject();
    tEnd = getSystemTicks();
    rSheet.insert( "3b: create and hold test objects", nLoop, tEnd - tStart );

    // structs
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->complex_in( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "4a: complex_in() calls (in struct; return struct)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->complex_inout( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "4b: complex_inout() calls (inout struct; return struct)", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->complex_oneway( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "4c: complex_oneway() oneway calls (in struct)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->complex_noreturn( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "4d: complex_noreturn() calls (in struct)", nLoop, tEnd - tStart );

    // attributes, get() methods
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getLong();
    tEnd = getSystemTicks();
    rSheet.insert( "5a: getLong() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getLong_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "5b: get long attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setLong( 0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5c: setLong() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setLong_attr( 0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5d: set long attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getHyper();
    tEnd = getSystemTicks();
    rSheet.insert( "5e: getHyper() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getHyper_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "5f: get hyper attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setHyper( 0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5g: setHyper() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setHyper_attr( 0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5h: set hyper attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getFloat();
    tEnd = getSystemTicks();
    rSheet.insert( "5i: getFloat() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getFloat_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "5j: get float attribute",nLoop,  tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setFloat( 0.0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5k: setFloat() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setFloat_attr( 0.0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5l: set float attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getDouble();
    tEnd = getSystemTicks();
    rSheet.insert( "5m: getDouble() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getDouble_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "5n: get double attribute", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setDouble( 0.0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5o: setDouble() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setDouble_attr( 0.0 );
    tEnd = getSystemTicks();
    rSheet.insert( "5p: set double attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getString();
    tEnd = getSystemTicks();
    rSheet.insert( "6a: getString() call (empty)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getString_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "6b: get empty string attribute", nLoop, tEnd - tStart );

    i = nLoop;
    OUString aDummyString;
    tStart = getSystemTicks();
    while (i--)
        xBench->setString( aDummyString );
    tEnd = getSystemTicks();
    rSheet.insert( "6c: setString() call (emtpy)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setString_attr( aDummyString );
    tEnd = getSystemTicks();
    rSheet.insert( "6d: set empty string attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getInterface();
    tEnd = getSystemTicks();
    rSheet.insert( "7a: getInterface() call (null)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getInterface_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "7b: get interface attribute", nLoop, tEnd - tStart );

    i = nLoop;
    Reference< XInterface > aDummyInterface;
    tStart = getSystemTicks();
    while (i--)
        xBench->setInterface( aDummyInterface );
    tEnd = getSystemTicks();
    rSheet.insert( "7c: setInterface() call (null)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setInterface_attr( Reference< XInterface >() );
    tEnd = getSystemTicks();
    rSheet.insert( "7d: set interface attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getAny();
    tEnd = getSystemTicks();
    rSheet.insert( "8a: getAny() call (empty)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getAny_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "8b: get empty any attribute", nLoop, tEnd - tStart );

    i = nLoop;
    Any aDummyAny;
    tStart = getSystemTicks();
    while (i--)
        xBench->setAny( aDummyAny );
    tEnd = getSystemTicks();
    rSheet.insert( "8c: setAny() call (empty)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setAny_attr( aDummyAny );
    tEnd = getSystemTicks();
    rSheet.insert( "8d: set empty any attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getSequence();
    tEnd = getSystemTicks();
    rSheet.insert( "9a: getSequence() call (empty)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getSequence_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "9b: get empty sequence attribute", nLoop, tEnd - tStart );
    i = nLoop;
    Sequence< Reference< XInterface > > aDummySeq;
    tStart = getSystemTicks();
    while (i--)
        xBench->setSequence( aDummySeq );
    tEnd = getSystemTicks();
    rSheet.insert( "9c: setSequence() call (empty)", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setSequence_attr( aDummySeq );
    tEnd = getSystemTicks();
    rSheet.insert( "9d: set empty sequence attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getStruct();
    tEnd = getSystemTicks();
    rSheet.insert( "Aa: getStruct() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->getStruct_attr();
    tEnd = getSystemTicks();
    rSheet.insert( "Ab: get struct attribute", nLoop, tEnd - tStart );

    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setStruct( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "Ac: setStruct() call", nLoop, tEnd - tStart );
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
        xBench->setStruct_attr( aDummyStruct );
    tEnd = getSystemTicks();
    rSheet.insert( "Ad: set struct attribute", nLoop, tEnd - tStart );

    // load
//      i = nLoop;
//      tStart = getSystemTicks();
//      while (i--)
//          xBench->setSequence( aSeq );
//      tEnd = getSystemTicks();
//      rSheet.insert( "transfer of exisiting objects", nLoop, tEnd - tStart );

    // exceptions
    i = nLoop;
    tStart = getSystemTicks();
    while (i--)
    {
        try
        {
            xBench->raiseRuntimeException();
        }
        catch (RuntimeException &)
        {
        }
    }
    tEnd = getSystemTicks();
    rSheet.insert( "Ba: raising RuntimeException", nLoop, tEnd - tStart );

    //------------------------------------
}

//--------------------------------------------------------------------------------------------------
static OUString extractParam( const Sequence< OUString > & rArgs, const OUString & rParam )
{
    const OUString * pArgs = rArgs.getConstArray();
    for ( sal_Int32 nPos = rArgs.getLength(); nPos--; )
    {
        if (pArgs[nPos].indexOf( rParam ) == 0 &&
            pArgs[nPos].getLength() > (rParam.getLength()+1))
        {
            return pArgs[nPos].copy( rParam.getLength() +1 ); // XXX=bla
        }
    }
    return OUString();
}

const sal_Int32 nMagicNumberDirect = 34000;

//XMain
//__________________________________________________________________________________________________
sal_Int32 TestImpl::run( const Sequence< OUString > & rArgs )
    throw (RuntimeException)
{
    // defaults
    FILE * stream = stderr;
    sal_Int64 nLoop = NLOOP;
    OUString aArg( RTL_CONSTASCII_USTRINGPARAM("dms") );

    try
    {
        OUString aLoopStr( extractParam( rArgs, OUString( RTL_CONSTASCII_USTRINGPARAM("loop") ) ) );
        if (aLoopStr.getLength())
        {
            sal_Int64 n = aLoopStr.toInt64();
            if (n > 0)
                nLoop = n;
        }

        OUString aDurationStr( extractParam( rArgs , OUString( RTL_CONSTASCII_USTRINGPARAM("duration" ) ) ) );
        if( aDurationStr.getLength() )
        {
            sal_Int64 n = aDurationStr.toInt64();
            if( n >0 )
                nLoop = nMagicNumberDirect * n;
        }

        OUString aLogStr( extractParam( rArgs, OUString( RTL_CONSTASCII_USTRINGPARAM("log") ) ) );
        if (aLogStr.getLength())
        {
            if (aLogStr.compareToAscii( "stderr" ) == 0)
            {
                stream = stderr;
            }
            else if (aLogStr.compareToAscii( "stdout" ) == 0)
            {
                stream = stdout;
            }
            else
            {
                OString aFileName( OUStringToOString( aLogStr, RTL_TEXTENCODING_ASCII_US ) );
                stream = ::fopen( aFileName.getStr(), "w" );
                if (! stream)
                {
                    OUStringBuffer buf( 32 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot open file for writing: \"") );
                    buf.append( aLogStr );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
                    throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
                }
            }
        }

        OUString aArgStr( extractParam( rArgs, OUString( RTL_CONSTASCII_USTRINGPARAM("opt") ) ) );
        if (aArgStr.getLength())
        {
            aArg = aArgStr;
        }

        if (! rArgs.getLength())
            out( "\n> no options given, using defaults" );

        // params
        out( "\n> opt=" );
        out( aArg );
        out( " log=" );
        if (stream == stderr)
            out( "stderr" );
        else if (stream == stdout)
            out( "stdout loop=" );
        else
            out( aLogStr );
        out( " loop=" );
        out( nLoop );
        out( "\n" );
        t_TimingSheetMap aSheets;
        TimingSheet aDirect;

        //------------------------------------------------------------------------------------------

        if (aArg.indexOf( 'd' ) >= 0)
        {
            // in process direct test
            sal_uInt32 nStart = getSystemTicks();
            benchmark( aDirect, getDirect(), nLoop );
            sal_uInt32 nEnd = getSystemTicks();
            fprintf( stderr, "Duration (direct in process): %g s\n", (nEnd - nStart)/1000.  );
        }

        //------------------------------------------------------------------------------------------

        if (aArg.indexOf( 'm' ) >= 0)
        {
            // in process uno dispatch
            Environment aCppEnv, aAnoCppEnv;
            OUString aCurrentLanguageBindingName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
            uno_getEnvironment( reinterpret_cast< uno_Environment ** >( &aCppEnv ),
                                aCurrentLanguageBindingName.pData, 0 );
            // anonymous
            uno_createEnvironment( reinterpret_cast< uno_Environment ** >( &aAnoCppEnv ),
                                   aCurrentLanguageBindingName.pData, 0 );

            // pseudo mapping uno<->uno: does nothing!
            Mapping aMapping( aCppEnv.get(), aAnoCppEnv.get(), OUString( RTL_CONSTASCII_USTRINGPARAM("pseudo") ) );
            if (! aMapping.is())
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("no pseudo mapping available!") ), Reference< XInterface >() );

            Reference< XInterface > xMapped;
            Reference< XInterface > xDirect( getDirect() );
            aMapping.mapInterface( reinterpret_cast< void ** >( &xMapped ), xDirect.get(),
                                   ::getCppuType( &xDirect ) );
            if (! xMapped.is())
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("mapping object failed!") ), Reference< XInterface >() );

            sal_uInt32 nStart = getSystemTicks();
            benchmark( aSheets[ "mapped in process" ], xMapped, nLoop / 100 );
            sal_uInt32 nEnd = getSystemTicks();
            fprintf( stderr, "Duration (mapped in process): %g s\n", (nStart - nEnd)/1000. );
        }

        //------------------------------------------------------------------------------------------

        if (aArg.indexOf( 's' ) >= 0)
        {
            // start server process
            oslSecurity hSecurity = osl_getCurrentSecurity();
            if (! hSecurity)
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("cannot get current security handle!") ), Reference< XInterface >() );

            OUString aArgs[] = {
                OUString( RTL_CONSTASCII_USTRINGPARAM("-c") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.performance.PerformanceTestObject") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("-l") ),
#ifdef SAL_UNX
                OUString( RTL_CONSTASCII_USTRINGPARAM("libperfobj.so") ),
#else
                OUString( RTL_CONSTASCII_USTRINGPARAM("perfobj.dll") ),
#endif
                OUString( RTL_CONSTASCII_USTRINGPARAM("-r") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("applicat.rdb") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("-u") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("uno:socket,host=localhost,port=6000;iiop;TestRemoteObject") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("--singleaccept") )
            };
            rtl_uString * pArgs[] = {
                aArgs[0].pData,
                aArgs[1].pData,
                aArgs[2].pData,
                aArgs[3].pData,
                aArgs[4].pData,
                aArgs[5].pData,
                aArgs[6].pData,
                aArgs[7].pData,
                aArgs[8].pData,
            };

            out( "\n> executing: \"uno" );
            for ( sal_Int32 nPos = 0; nPos < (sizeof(aArgs) / sizeof(OUString)); ++nPos )
            {
                out( " " );
                out( aArgs[nPos] );
            }
            out( "\" ..." );

            oslProcess hProcess = 0;
            OUString aUnoExe( RTL_CONSTASCII_USTRINGPARAM("uno") );
            OUString aWorkingDir( RTL_CONSTASCII_USTRINGPARAM(".") );
            osl_executeProcess(
                aUnoExe.pData, pArgs, sizeof(aArgs) / sizeof(OUString),
                osl_Process_SEARCHPATH | osl_Process_DETACHED | osl_Process_NORMAL,
                hSecurity, aWorkingDir.pData, 0, 0, &hProcess );

            osl_freeSecurityHandle( hSecurity );
            if (! hProcess)
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("cannot start server process!") ), Reference< XInterface >() );
            osl_freeProcessHandle( hProcess );

            // wait three seconds
            TimeValue threeSeconds;
            threeSeconds.Seconds = 3;
            osl_waitThread( &threeSeconds );

            // connect and resolve outer process object
            Reference< XInterface > xResolvedObject( resolveObject( OUString(
                RTL_CONSTASCII_USTRINGPARAM("uno:socket,host=localhost,port=6000;iiop;TestRemoteObject") ) ) );

            benchmark( aSheets[ "remote same host" ], xResolvedObject, nLoop / 300 );
        }

        //------------------------------------------------------------------------------------------

        if (aArg.indexOf( 'r' ) >= 0)
        {
            // remote
            OUString aUnoUrl( extractParam( rArgs, OUString( RTL_CONSTASCII_USTRINGPARAM("url") ) ) );
            if (! aUnoUrl.getLength())
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("performance test r(emote) needs additional uno url!") ), Reference< XInterface >() );

            // connect and resolve outer process object
            Reference< XInterface > xResolvedObject( resolveObject( aUnoUrl ) );

            sal_Int32 t1 = getSystemTicks();
            OString o = OUStringToOString( aUnoUrl, RTL_TEXTENCODING_ASCII_US );
            benchmark( aSheets[ o.getStr() ], xResolvedObject, nLoop / 900 );
            sal_Int32 t2 = getSystemTicks();
            fprintf( stderr, "Duration (%s): %g s\n", o.getStr(),(t2 - t1)/1000. );
        }

        //------------------------------------------------------------------------------------------

        if (aArg.indexOf( 'j' ) >= 0)
        {
            // java
              benchmark( aSheets[ "java in process" ],
                       _xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.benchmark.JavaTestObject"))),
                       nLoop / 1000 );
        }

        //------------------------------------------------------------------------------------------
        // dump out tables

        out( "\nTimes( ratio to direct in process )", stream );
#if OSL_DEBUG_LEVEL > 1
        out ("; compiled with OSL_DEBUG_LEVEL > 1", stream );
#endif
        out( ":", stream );

        sal_Int32 nPos = 60;
        out( "[direct in process]", stream, nPos );
        t_TimingSheetMap::const_iterator iSheets( aSheets.begin() );
        for ( ; iSheets != aSheets.end(); ++iSheets )
        {
            nPos += 40;
            out( "[", stream, nPos );
            out( (*iSheets).first.c_str(), stream );
            out( "]", stream );
        }
        for ( t_TimeEntryMap::const_iterator iTopics( aDirect._entries.begin() );
              iTopics != aDirect._entries.end(); ++iTopics )
        {
            const std::string & rTopic = (*iTopics).first;

            out( "\n", stream );
            out( rTopic.c_str(), stream );

            out( ":", stream, 58, '.' );

            sal_Int32 nPos = 60;

            double secs = (*iTopics).second.secPerCall();
            if (secs > 0.0)
            {
                out( secs * 1000, stream, nPos );
                out( "ms", stream );
            }
            else
            {
                out( "NA", stream, nPos );
            }

            iSheets = aSheets.begin();
            for ( ; iSheets != aSheets.end(); ++iSheets )
            {
                const t_TimeEntryMap::const_iterator iFind( (*iSheets).second._entries.find( rTopic ) );
                OSL_ENSURE( iFind != (*iSheets).second._entries.end(), "####" );

                nPos += 40;

                double secs = (*iFind).second.secPerCall();
                if (secs != 0.0)
                {
                    out( secs * 1000, stream, nPos );
                    out( "ms", stream );

                    out( " (", stream );
                    double ratio = (*iFind).second.ratio( (*iTopics).second );
                    if (ratio != 0.0)
                    {
                        out( ratio, stream );
                        out( " x)", stream );
                    }
                    else
                    {
                        out( "NA)", stream );
                    }
                }
                else
                {
                    out( "NA", stream, nPos );
                }
            }
        }
    }
    catch (Exception & rExc)
    {
        if (stream != stderr && stream != stdout)
            ::fclose( stream );
        throw RuntimeException( rExc.Message, rExc.Context );
    }

    if (stream != stderr && stream != stdout)
        ::fclose( stream );

    out( "\n> done.\n" );
    return 0;
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );
            xNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            benchmark_test::TestImpl_create,
            benchmark_test::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
