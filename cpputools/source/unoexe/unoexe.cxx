/*************************************************************************
 *
 *  $RCSfile: unoexe.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-12 18:57:50 $
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

#include <stdio.h>
#include <vector>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/module.h>

#include <rtl/string.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <uno/environment.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
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

using namespace std;
using namespace rtl;
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

//--------------------------------------------------------------------------------------------------
static inline void out( const sal_Char * pText )
{
    fprintf( stderr, pText );
}
//--------------------------------------------------------------------------------------------------
static inline void out( const OUString & rText )
{
    OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, aText.getStr() );
}

//--------------------------------------------------------------------------------------------------
static const char arUsingText[] =
"\nusing:\n\n"
"uno (-c ComponentImplementationName -l LocationUrl | -s ServiceName)\n"
"    [-ro ReadOnlyRegistry1] [-ro ReadOnlyRegistry2] ... [-rw ReadWriteRegistry]\n"
"    [-u uno:(socket[,host=HostName][,port=nnn]|pipe[,name=PipeName]);iiop;Name\n"
"        [--singleaccept] [--singleinstance]]\n"
"    [-- Argument1 Argument2 ...]\n";

//--------------------------------------------------------------------------------------------------
static sal_Bool readOption( OUString * pValue, sal_Char const * pOpt,
                            sal_Int32 * pnIndex, const sal_Char * argv[], sal_Int32 argc )
    throw (RuntimeException)
{
    const sal_Char * pArg = argv[ *pnIndex ];
    if (pArg[ 0 ] != '-')
        return sal_False;

    OString aArg( pArg +1 );
    OString aOpt( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return sal_False;

    if (aArg.getLength() == aOpt.getLength() && aOpt.equalsIgnoreCase( aArg ))
    {
        // take next argument
        ++(*pnIndex);
        if (*pnIndex >= argc || argv[ *pnIndex ][0] == '-')
        {
            OUStringBuffer buf( 32 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("incomplete option \"-") );
            buf.appendAscii( pOpt );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" given!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
        }
        else
        {
            *pValue = OUString::createFromAscii( argv[ *pnIndex ] );
#ifdef DEBUG
            out( "\n> identified option -" );
            out( pOpt );
            out( " = " );
            out( argv[ *pnIndex ] );
#endif
            ++(*pnIndex);
            return sal_True;
        }
    }
    else if (aOpt.equalsIgnoreCase( aArg.copy( 0, aOpt.getLength() ) ))
    {
        *pValue = OUString::createFromAscii( pArg + 1 + aOpt.getLength() );
#ifdef DEBUG
        out( "\n> identified option -" );
        out( pOpt );
        out( " = " );
        out( pArg + 1 + aOpt.getLength() );
#endif
        ++(*pnIndex);
        return sal_True;
    }
    return sal_False;
}
//--------------------------------------------------------------------------------------------------
static sal_Bool readOption( sal_Bool * pbOpt, const sal_Char * pOpt,
                            sal_Int32 * pnIndex, const sal_Char * argv[], sal_Int32 argc )
{
    const sal_Char * pArg = argv[ *pnIndex ];
    if (pArg[0] == '-' && pArg[1] == '-' && rtl_str_equalsIgnoreCase( pArg+2, pOpt ))
    {
        ++(*pnIndex);
        *pbOpt = sal_True;
#ifdef DEBUG
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
    const Reference< XMultiServiceFactory > & xMgr,
    const OUString & rServiceName )
    throw (Exception)
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
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor") ), OUString(),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.io.Acceptor") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // connector
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("connectr") ), OUString(),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.io.Connector") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // iiop bridge
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge") ), OUString(),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.remotebridges.Bridge.various") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
                    // bridge factory
                    xSet->insert( makeAny( loadSharedLibComponentFactory(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr") ), OUString(),
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.remotebridges.BridgeFactory") ),
                        xMgr, Reference< XRegistryKey >() ) ) );
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
static Reference< XSimpleRegistry > nestRegistries(
    const Reference< XMultiServiceFactory > & xMgr,
    const Reference< XSimpleRegistry > & xReadWrite,
    const Reference< XSimpleRegistry > & xReadOnly )
    throw (Exception)
{
    Reference< XSimpleRegistry > xReg;
    createInstance(
        xReg, xMgr,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.NestedRegistry") ) );

    Reference< XInitialization > xInit( xReg, UNO_QUERY );
    if (! xInit.is())
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("nested registry does not export interface \"com.sun.star.lang.XInitialization\"!" ) ), Reference< XInterface >() );

    Sequence< Any > aArgs( 2 );
    aArgs[0] = makeAny( xReadWrite );
    aArgs[1] = makeAny( xReadOnly );
    xInit->initialize( aArgs );

    return xReg;
}
//--------------------------------------------------------------------------------------------------
static Reference< XSimpleRegistry > openRegistry(
    const Reference< XMultiServiceFactory > & xMgr,
    const OUString & rURL,
    sal_Bool bReadOnly, sal_Bool bCreate )
    throw (Exception)
{
    Reference< XSimpleRegistry > xNewReg;
    createInstance(
        xNewReg, xMgr,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry") ) );

    try
    {
        xNewReg->open( rURL, bReadOnly, bCreate );
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
    const Reference< XMultiServiceFactory > & xMgr,
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
            aExt.compareToAscii( "so" ) == 0)
        {
            createInstance(
                xLoader, xMgr,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") ) );
        }
        else if (aExt.compareToAscii( "jar" ) == 0 ||
                 aExt.compareToAscii( "class" ) == 0)
        {
            createInstance(
                xLoader, xMgr,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.Java") ) );
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
        Reference< XSingleServiceFactory > xFactory( xLoader->activate(
            rImplName, OUString(), rLocation, Reference< XRegistryKey >() ), UNO_QUERY );
        if (xFactory.is())
            xInstance = xFactory->createInstance();

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
    Reference< XMultiServiceFactory > _xMgr;

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
    OInstanceProvider( const Reference< XMultiServiceFactory > & xMgr,
                       const OUString & rImplName, const OUString & rLocation,
                       const OUString & rServiceName, const Sequence< Any > & rInitParams,
                       sal_Bool bSingleInstance, const OUString & rInstanceName )
        : _xMgr( xMgr )
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
    if (_aImplName.getLength()) // manually via loader
        xRet = loadComponent( _xMgr, _aImplName, _aLocation );
    else // via service manager
        unoexe::createInstance( xRet, _xMgr, _aServiceName );

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

            if (_bSingleInstance)
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
void ODisposingListener::disposing( const EventObject & rEvt )
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

extern "C" int SAL_CALL main( int argc, const char * argv[] )
{
    if (argc <= 1)
    {
        out( arUsingText );
        return 0;
    }

    sal_Int32 nRet = 0;
    Reference< XMultiServiceFactory > xMgr;
    Reference< XSimpleRegistry > xRegistry;

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

        sal_Int32 nPos = 1;
        // read up to arguments
        while (nPos < argc)
        {
            if (rtl_str_compare( argv[nPos], "--" ) == 0)
            {
                ++nPos;
                break;
            }

            if (readOption( &aImplName, "c", &nPos, argv, argc )                ||
                readOption( &aLocation, "l", &nPos, argv, argc )                ||
                readOption( &aServiceName, "s", &nPos, argv, argc )             ||
                readOption( &aUnoUrl, "u", &nPos, argv, argc )                  ||
                readOption( &bSingleAccept, "singleaccept", &nPos, argv, argc ) ||
                readOption( &bSingleInstance, "singleinstance", &nPos, argv, argc ))
            {
                continue;
            }
            OUString aRegistry;
            if (readOption( &aRegistry, "ro", &nPos, argv, argc ))
            {
                aReadOnlyRegistries.push_back( aRegistry );
                bNewRegistryMimic = true;
                continue;
            }
            if (readOption( &aReadWriteRegistry, "rw", &nPos, argv, argc ))
            {
                bNewRegistryMimic = true;
                continue;
            }
            if (readOption( &aRegistry, "r", &nPos, argv, argc ))
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
            buf.appendAscii( argv[nPos] );
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

        if ((aImplName.getLength() != 0) == (aServiceName.getLength() != 0))
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("give component exOR service name!" ) ), Reference< XInterface >() );
        if (aImplName.getLength() && !aLocation.getLength())
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("give component location!" ) ), Reference< XInterface >() );
        if (aServiceName.getLength() && aLocation.getLength())
            out( "\n> warning: service name given, will ignore location!" );

        // read component params
        aParams.realloc( argc - nPos );
        OUString * pParams = aParams.getArray();

        sal_Int32 nOffset = nPos;
        for ( ; nPos < argc; ++nPos )
        {
            pParams[nPos -nOffset] = OUString::createFromAscii( argv[nPos] );
        }

        //#### create registry #####################################################################

          xMgr = createServiceFactory();
        if (! xMgr.is())
            throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("cannot boot strap service manager!" ) ), Reference< XInterface >() );

        // ReadOnly registries
        for ( size_t nReg = 0; nReg < aReadOnlyRegistries.size(); ++nReg )
        {
#ifdef DEBUG
            out( "\n> trying to open ro registry: " );
            out( OUStringToOString( aReadOnlyRegistries[ nReg ], RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
            Reference< XSimpleRegistry > xNewReg(
                openRegistry( xMgr, aReadOnlyRegistries[ nReg ], sal_True, sal_False ) );
            if (xNewReg.is())
                xRegistry = (xRegistry.is() ? nestRegistries( xMgr, xNewReg, xRegistry ) : xNewReg);
        }
        if (aReadWriteRegistry.getLength())
        {
#ifdef DEBUG
            out( "\n> trying to open rw registry: " );
            out( OUStringToOString( aReadWriteRegistry, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
            // ReadWrite registry
            Reference< XSimpleRegistry > xNewReg(
                openRegistry( xMgr, aReadWriteRegistry, sal_False, sal_True ) );
            if (xNewReg.is())
                xRegistry = (xRegistry.is() ? nestRegistries( xMgr, xNewReg, xRegistry ) : xNewReg);
        }

        // init service manager with registry
        if (xRegistry.is())
        {
            Reference< XInitialization > xInit( xMgr, UNO_QUERY );
            if (! xInit.is())
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("service manager does not export interface \"com.sun.star.lang.XInitialization\"!" ) ), Reference< XInterface >() );

            Any aReg( makeAny( xRegistry ) );
            xInit->initialize( Sequence< Any >( &aReg, 1 ) );
        }
        else
        {
            out( "\n> warning: no registry given!" );
        }

        //#### accept, instanciate, etc. ###########################################################

        if (aUnoUrl.getLength()) // accepting connections
        {
            if (aUnoUrl.getTokenCount( ';' ) != 3 || aUnoUrl.getLength() < 10 ||
                !aUnoUrl.copy( 0, 4 ).equalsIgnoreCase( OUString( RTL_CONSTASCII_USTRINGPARAM("uno:") ) ))
            {
                throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("illegal uno url given!" ) ), Reference< XInterface >() );
            }
            OUString aConnectDescr( aUnoUrl.getToken( 0 ).copy( 4 ) ); // uno:CONNECTDESCR;iiop;InstanceName
            OUString aInstanceName( aUnoUrl.getToken( 2 ) );

            Reference< XAcceptor > xAcceptor;
            createInstance(
                xAcceptor, xMgr,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor") ) );

            // init params
            Sequence< Any > aInitParams( aParams.getLength() );
            const OUString * pParams = aParams.getConstArray();
            Any * pInitParams = aInitParams.getArray();
            for ( sal_Int32 nPos = aParams.getLength(); nPos--; )
            {
                pInitParams[nPos] = makeAny( pParams[nPos] );
            }

            // instance provider
            Reference< XInstanceProvider > xInstanceProvider( new OInstanceProvider(
                xMgr, aImplName, aLocation, aServiceName, aInitParams,
                bSingleInstance, aInstanceName ) );

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
                    xBridgeFactory, xMgr,
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory") ) );

                // bridge
                Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                    OUString(), aUnoUrl.getToken(1),
                    xConnection, xInstanceProvider ) );

                if (bSingleAccept)
                {
                    Reference< XComponent > xComp( xBridge, UNO_QUERY );
                    if (! xComp.is())
                        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("bridge factory does not export interface \"com.sun.star.lang.XComponent\"!" ) ), Reference< XInterface >() );
                    ODisposingListener::waitFor( xComp );
                    break;
                }
            }
        }
        else // no uno url
        {
            Reference< XInterface > xInstance;
            if (aImplName.getLength()) // manually via loader
                xInstance = loadComponent( xMgr, aImplName, aLocation );
            else // via service manager
                createInstance( xInstance, xMgr, aServiceName );

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
    if (xMgr.is())
    {
        Reference< XComponent > xComp( xMgr, UNO_QUERY );
        if (xComp.is())
            xComp->dispose();
    }
    if (xRegistry.is())
    {
        xRegistry->close();
    }

    out( "\n" );
    return nRet;
}

}


