/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:02:43 $
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

#include <string.h>
#include <vector>

#include "rtl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/random.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#if OSL_DEBUG_LEVEL > 0
#include "rtl/strbuf.hxx"
#endif
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "osl/security.hxx"
#include "osl/thread.hxx"

#include "cppuhelper/shlib.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/component_context.hxx"
#include "cppuhelper/access_control.hxx"

#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"

#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) sizeof (x) / sizeof *(x)


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

static void MyDummySymbolWithinLibrary(){}
OUString const & get_this_libpath()
{
    static OUString s_path;
    if (0 == s_path.getLength())
    {
        OUString path;
        Module::getUrlFromAddress( (void*) MyDummySymbolWithinLibrary, path );
        path = path.copy( 0, path.lastIndexOf( '/' ) );
        MutexGuard guard( Mutex::getGlobalMutex() );
        if (0 == s_path.getLength())
            s_path = path;
    }
    return s_path;
}

Bootstrap const & get_unorc() SAL_THROW( () )
{
    static rtlBootstrapHandle s_bstrap = 0;
    if (! s_bstrap)
    {
        OUString iniName(
            get_this_libpath() + OUSTR("/" SAL_CONFIGFILE("uno")) );
        rtlBootstrapHandle bstrap = rtl_bootstrap_args_open( iniName.pData );

        ClearableMutexGuard guard( Mutex::getGlobalMutex() );
        if (s_bstrap)
        {
            guard.clear();
            rtl_bootstrap_args_close( bstrap );
        }
        else
        {
            s_bstrap = bstrap;
        }
    }
    return *(Bootstrap const *)&s_bstrap;
}


void addFactories(
    char const * const * ppNames /* lib, implname, ..., 0 */,
    OUString const & bootstrapPath,
    Reference< lang::XMultiComponentFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (Exception) )
{
    Reference< container::XSet > xSet( xMgr, UNO_QUERY );
    OSL_ASSERT( xSet.is() );
    Reference< lang::XMultiServiceFactory > xSF( xMgr, UNO_QUERY );

    while (*ppNames)
    {
        OUString lib( OUString::createFromAscii( *ppNames++ ) );
        OUString implName( OUString::createFromAscii( *ppNames++ ) );

        Any aFac( makeAny( loadSharedLibComponentFactory(
                               lib, bootstrapPath, implName, xSF, xKey ) ) );
        xSet->insert( aFac );
#if OSL_DEBUG_LEVEL > 1
        if (xSet->has( aFac ))
        {
            Reference< lang::XServiceInfo > xInfo;
            if (aFac >>= xInfo)
            {
                ::fprintf(
                    stderr, "> implementation %s supports: ", ppNames[ -1 ] );
                Sequence< OUString > supported(
                    xInfo->getSupportedServiceNames() );
                for ( sal_Int32 nPos = supported.getLength(); nPos--; )
                {
                    OString str( OUStringToOString(
                        supported[ nPos ], RTL_TEXTENCODING_ASCII_US ) );
                    ::fprintf( stderr, nPos ? "%s, " : "%s\n", str.getStr() );
                }
            }
            else
            {
                ::fprintf(
                    stderr,
                    "> implementation %s provides NO lang::XServiceInfo!!!\n",
                    ppNames[ -1 ] );
            }
        }
#endif
#if OSL_DEBUG_LEVEL > 0
        if (! xSet->has( aFac ))
        {
            OStringBuffer buf( 64 );
            buf.append( "### failed inserting shared lib \"" );
            buf.append( ppNames[ -2 ] );
            buf.append( "\"!!!" );
            OString str( buf.makeStringAndClear() );
            OSL_ENSURE( 0, str.getStr() );
        }
#endif
    }
}

// private forward decl
Reference< lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) );

Reference< XComponentContext > bootstrapInitialContext(
    Reference< lang::XMultiComponentFactory > const & xSF,
    Reference< registry::XSimpleRegistry > const & types_xRegistry,
    Reference< registry::XSimpleRegistry > const & services_xRegistry,
    OUString const & rBootstrapPath, Bootstrap const & bootstrap )
    SAL_THROW( (Exception) );

Reference< XComponentContext > SAL_CALL createInitialCfgComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    SAL_THROW( () );

Reference< registry::XSimpleRegistry > SAL_CALL createRegistryWrapper(
    const Reference< XComponentContext >& xContext );

template< class T >
static inline beans::PropertyValue createPropertyValue(
    OUString const & name, T const & value )
    SAL_THROW( () )
{
    return beans::PropertyValue(
        name, -1, makeAny( value ), beans::PropertyState_DIRECT_VALUE );
}

static OUString findBoostrapArgument(
    const Bootstrap & bootstrap,
    const OUString & arg_name,
    sal_Bool * pFallenBack )
    SAL_THROW(())
{
    OUString result;

    OUString prefixed_arg_name = OUSTR("UNO_");
    prefixed_arg_name += arg_name.toAsciiUpperCase();

    // environment not set -> try relative to executable
    if(!bootstrap.getFrom(prefixed_arg_name, result))
    {
        if(pFallenBack)
            *pFallenBack = sal_True;

        OUString fileName;
        bootstrap.getIniName(fileName);

        // cut the rc extension
        OUStringBuffer result_buf( 64 );
        result_buf.append(
            fileName.copy(
                0, fileName.getLength() - strlen(SAL_CONFIGFILE(""))) );
        result_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("_") );
        result_buf.append( arg_name.toAsciiLowerCase() );
        result_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(".rdb") );
        result = result_buf.makeStringAndClear();

#if OSL_DEBUG_LEVEL > 1
        OString result_dbg =
            OUStringToOString(result, RTL_TEXTENCODING_ASCII_US);
        OString arg_name_dbg =
            OUStringToOString(arg_name, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE(
            "cppuhelper::findBoostrapArgument - "
            "setting %s relative to executable: %s\n",
            arg_name_dbg.getStr(),
            result_dbg.getStr() );
#endif
    }
    else
    {
        if(pFallenBack)
            *pFallenBack = sal_False;

#if OSL_DEBUG_LEVEL > 1
        OString prefixed_arg_name_dbg = OUStringToOString(
            prefixed_arg_name, RTL_TEXTENCODING_ASCII_US );
        OString result_dbg = OUStringToOString(
            result, RTL_TEXTENCODING_ASCII_US );
        OSL_TRACE(
            "cppuhelper::findBoostrapArgument - found %s in env: %s",
            prefixed_arg_name_dbg.getStr(),
            result_dbg.getStr() );
#endif
    }

    return result;
}

static Reference< registry::XSimpleRegistry > nestRegistries(
    const OUString baseDir,
    const Reference< lang::XSingleServiceFactory > & xSimRegFac,
    const Reference< lang::XSingleServiceFactory > & xNesRegFac,
    OUString csl_rdbs,
    const OUString & write_rdb,
    sal_Bool forceWrite_rdb,
    sal_Bool bFallenBack,
    Bootstrap const & bootstrap )
    SAL_THROW((Exception))
{
    sal_Int32 index;
    Reference< registry::XSimpleRegistry > lastRegistry;

    if(write_rdb.getLength()) // is there a write registry given?
    {
        lastRegistry.set(xSimRegFac->createInstance(), UNO_QUERY);

        try
        {
            lastRegistry->open(write_rdb, sal_False, forceWrite_rdb);
        }
        catch (registry::InvalidRegistryException & invalidRegistryException)
        {
#if OSL_DEBUG_LEVEL > 1
            OString rdb_name_tmp = OUStringToOString(
                write_rdb, RTL_TEXTENCODING_ASCII_US);
            OString message_dbg = OUStringToOString(
                invalidRegistryException.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE(
                "warning: couldn't open %s cause of %s",
                rdb_name_tmp.getStr(), message_dbg.getStr() );
#endif
        }

        if(!lastRegistry->isValid())
            lastRegistry.clear();
    }

    do
    {
        index = csl_rdbs.indexOf((sal_Unicode)' ');
        OUString rdb_name = (index == -1) ? csl_rdbs : csl_rdbs.copy(0, index);
        csl_rdbs = (index == -1) ? OUString() : csl_rdbs.copy(index + 1);

        if (! rdb_name.getLength())
            continue;

        bool optional = ('?' == rdb_name[ 0 ]);
        if (optional)
            rdb_name = rdb_name.copy( 1 );

        if (rdb_name.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") ))
        {
            rdb_name = ::rtl::Uri::decode(
                rdb_name.copy( sizeof ("vnd.sun.star.expand:") -1 ),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            bootstrap.expandMacrosFrom( rdb_name );
        }

        try
        {
            Reference<registry::XSimpleRegistry> simpleRegistry(
                xSimRegFac->createInstance(), UNO_QUERY );

            osl::FileBase::getAbsoluteFileURL(baseDir, rdb_name, rdb_name);
            simpleRegistry->open(rdb_name, sal_True, sal_False);

            if(lastRegistry.is())
            {
                Reference< registry::XSimpleRegistry > nestedRegistry(
                    xNesRegFac->createInstance(), UNO_QUERY );
                Reference< lang::XInitialization > nestedRegistry_xInit(
                    nestedRegistry, UNO_QUERY );

                Sequence<Any> aArgs(2);
                aArgs[0] <<= lastRegistry;
                aArgs[1] <<= simpleRegistry;

                nestedRegistry_xInit->initialize(aArgs);

                lastRegistry = nestedRegistry;
            }
            else
                lastRegistry = simpleRegistry;
        }
        catch(registry::InvalidRegistryException & invalidRegistryException)
        {
            if (! optional)
            {
                // if a registry was explicitly given, the exception shall fly
                if( ! bFallenBack )
                    throw;
            }

#if OSL_DEBUG_LEVEL > 1
            OString rdb_name_tmp = OUStringToOString(
                rdb_name, RTL_TEXTENCODING_ASCII_US );
            OString message_dbg = OUStringToOString(
                invalidRegistryException.Message, RTL_TEXTENCODING_ASCII_US );
            OSL_TRACE(
                "warning: couldn't open %s cause of %s",
                rdb_name_tmp.getStr(), message_dbg.getStr() );
#endif
        }
    }
    while(index != -1 && csl_rdbs.getLength()); // are there more rdbs in list?

    return lastRegistry;
}

static Reference< XComponentContext >
SAL_CALL defaultBootstrap_InitialComponentContext(
    Bootstrap const & bootstrap )
    SAL_THROW( (Exception) )
{
    OUString bootstrapPath( get_this_libpath() );
    OUString iniDir;

    osl_getProcessWorkingDir(&iniDir.pData);

    Reference<lang::XMultiComponentFactory> smgr_XMultiComponentFactory(
        bootstrapInitialSF(bootstrapPath) );
    Reference<lang::XMultiServiceFactory> smgr_XMultiServiceFactory(
        smgr_XMultiComponentFactory, UNO_QUERY );

    Reference<registry::XRegistryKey> xEmptyKey;
    Reference<lang::XSingleServiceFactory> xSimRegFac(
        loadSharedLibComponentFactory(
            OUSTR("simplereg.uno" SAL_DLLEXTENSION), bootstrapPath,
            OUSTR("com.sun.star.comp.stoc.SimpleRegistry"),
            smgr_XMultiServiceFactory,
            xEmptyKey),
        UNO_QUERY);

    Reference<lang::XSingleServiceFactory> xNesRegFac(
        loadSharedLibComponentFactory(
            OUSTR("nestedreg.uno" SAL_DLLEXTENSION), bootstrapPath,
            OUSTR("com.sun.star.comp.stoc.NestedRegistry"),
            smgr_XMultiServiceFactory,
            xEmptyKey),
        UNO_QUERY);

    sal_Bool bFallenback_types;
    OUString cls_uno_types =
        findBoostrapArgument( bootstrap, OUSTR("TYPES"), &bFallenback_types );

    Reference<registry::XSimpleRegistry> types_xRegistry =
        nestRegistries(
            iniDir, xSimRegFac, xNesRegFac, cls_uno_types,
            OUString(), sal_False, bFallenback_types, bootstrap );

    // ==== bootstrap from services registry ====

    sal_Bool bFallenback_services;
    OUString cls_uno_services = findBoostrapArgument(
        bootstrap, OUSTR("SERVICES"), &bFallenback_services );

    sal_Bool fallenBackWriteRegistry;
    OUString write_rdb = findBoostrapArgument(
        bootstrap, OUSTR("WRITERDB"), &fallenBackWriteRegistry );
    if (fallenBackWriteRegistry)
    {
        // no standard write rdb anymore
        write_rdb = OUString();
    }

    Reference<registry::XSimpleRegistry> services_xRegistry = nestRegistries(
        iniDir, xSimRegFac, xNesRegFac, cls_uno_services, write_rdb,
        !fallenBackWriteRegistry, bFallenback_services, bootstrap );

    Reference< XComponentContext > xContext(
        bootstrapInitialContext(
            smgr_XMultiComponentFactory, types_xRegistry, services_xRegistry,
            bootstrapPath, bootstrap ) );

    // initialize sf
    Reference< lang::XInitialization > xInit(
        smgr_XMultiComponentFactory, UNO_QUERY );
    OSL_ASSERT( xInit.is() );
    Sequence< Any > aSFInit( 1 );
    aSFInit[ 0 ] <<= services_xRegistry;
    xInit->initialize( aSFInit );

    return xContext;
}


Reference< XComponentContext >
SAL_CALL defaultBootstrap_InitialComponentContext(
    OUString const & iniFile )
    SAL_THROW( (Exception) )
{
    Bootstrap bootstrap( iniFile );
    if (bootstrap.getHandle() == 0)
        throw io::IOException(OUSTR("Cannot open for reading: ") + iniFile, 0);
    return defaultBootstrap_InitialComponentContext( bootstrap );
}

Reference< XComponentContext >
SAL_CALL defaultBootstrap_InitialComponentContext()
    SAL_THROW( (Exception) )
{
    return defaultBootstrap_InitialComponentContext( get_unorc() );
}

BootstrapException::BootstrapException()
{
}

BootstrapException::BootstrapException( const ::rtl::OUString & rMessage )
    :m_aMessage( rMessage )
{
}

BootstrapException::BootstrapException( const BootstrapException & e )
{
    m_aMessage = e.m_aMessage;
}

BootstrapException::~BootstrapException()
{
}

BootstrapException & BootstrapException::operator=( const BootstrapException & e )
{
    m_aMessage = e.m_aMessage;
    return *this;
}

const ::rtl::OUString & BootstrapException::getMessage() const
{
    return m_aMessage;
}

Reference< XComponentContext > SAL_CALL bootstrap()
{
    Reference< XComponentContext > xRemoteContext;

    try
    {
        // create default local component context
        Reference< XComponentContext > xLocalContext(
            defaultBootstrap_InitialComponentContext() );
        if ( !xLocalContext.is() )
            throw BootstrapException( OUSTR( "no local component context!" ) );

        // URL to office executable
        OUString sOfficeURL( get_this_libpath() + OUSTR( "/soffice" ) );

        // create a random pipe name
        rtlRandomPool hPool = rtl_random_createPool();
        if ( hPool == 0 )
            throw BootstrapException( OUSTR( "cannot create random pool!" ) );
        sal_uInt8 bytes[ 32 ];
        if ( rtl_random_getBytes( hPool, bytes, ARLEN( bytes ) )
            != rtl_Random_E_None )
            throw BootstrapException( OUSTR( "random pool error!" ) );
        rtl_random_destroyPool( hPool );
        ::rtl::OUStringBuffer buf;
        for ( sal_uInt32 i = 0; i < ARLEN( bytes ); ++i )
            buf.append( static_cast< sal_Int32 >( bytes[ i ] ), 0x10 );
        OUString sPipeName( buf.makeStringAndClear() );

        // accept string
        OSL_ASSERT( buf.getLength() == 0 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "-accept=pipe,name=" ) );
        buf.append( sPipeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ";urp;" ) );

        // arguments
        OUString args [] = {
            OUSTR( "-nologo" ), OUSTR( "-nodefault" ), buf.makeStringAndClear()
        };
        rtl_uString * ar_args [] = {
            args[ 0 ].pData, args[ 1 ].pData, args[ 2 ].pData
        };
        ::osl::Security sec;

        // start office process
        oslProcess hProcess = 0;
        oslProcessError rc = osl_executeProcess(
            sOfficeURL.pData, ar_args, ARLEN( ar_args ),
            osl_Process_DETACHED,
            sec.getHandle(),
            0, // => current working dir
            0, 0, // => no env vars
            &hProcess );
        switch ( rc )
        {
            case osl_Process_E_None:
                break;
            case osl_Process_E_NotFound:
                throw BootstrapException( OUSTR( "image not found!" ) );
            case osl_Process_E_TimedOut:
                throw BootstrapException( OUSTR( "timout occured!" ) );
            case osl_Process_E_NoPermission:
                throw BootstrapException( OUSTR( "permission denied!" ) );
            case osl_Process_E_Unknown:
                throw BootstrapException( OUSTR( "unknown error!" ) );
            case osl_Process_E_InvalidError:
            default:
                throw BootstrapException( OUSTR( "unmapped error!" ) );
        }

        // create a URL resolver
        Reference< bridge::XUnoUrlResolver > xUrlResolver(
            bridge::UnoUrlResolver::create( xLocalContext ) );

        // connection string
        OSL_ASSERT( buf.getLength() == 0 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "uno:pipe,name=" ) );
        buf.append( sPipeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
            ";urp;StarOffice.ComponentContext" ) );
        OUString sConnectString( buf.makeStringAndClear() );

        // wait until office is started
        for ( ; ; )
        {
            try
            {
                // try to connect to office
                xRemoteContext.set(
                    xUrlResolver->resolve( sConnectString ), UNO_QUERY_THROW );
                break;
            }
            catch ( connection::NoConnectException & )
            {
                // wait 500 ms, then try to connect again
                TimeValue tv = { 0 /* secs */, 500000000 /* nanosecs */ };
                ::osl::Thread::wait( tv );
            }
        }
    }
    catch ( Exception & e )
    {
        throw BootstrapException(
            OUSTR( "unexpected UNO exception caught: " ) + e.Message );
    }

    return xRemoteContext;
}

} // namespace cppu
