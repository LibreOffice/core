/*************************************************************************
 *
 *  $RCSfile: servicefactory.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: dbo $ $Date: 2002-01-11 10:06:02 $
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

#include <vector>

#include <rtl/process.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <osl/module.h>
#include <osl/thread.h>

#include <cppuhelper/shlib.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/access_control.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/security/XAccessController.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;

namespace cppu
{

// private forward decl
//--------------------------------------------------------------------------------------------------
void addFactories(
    char const * const * ppNames /* lib, implname, ..., 0 */,
    OUString const & bootstrapPath,
    Reference< lang::XMultiComponentFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (Exception) );
//--------------------------------------------------------------------------------------------------
Reference< security::XAccessController > createDefaultAccessController() SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
void * SAL_CALL parentThreadCallback(void) SAL_THROW_EXTERN_C();
//--------------------------------------------------------------------------------------------------
void SAL_CALL childThreadCallback( void * xParentContext ) SAL_THROW_EXTERN_C();

//==================================================================================================
static Reference< XInterface > SAL_CALL createInstance(
    Reference< XInterface > const & xFactory,
    Reference< XComponentContext > const & xContext = Reference< XComponentContext >() )
{
    Reference< lang::XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
    if (xFac.is())
    {
        return xFac->createInstanceWithContext( xContext );
    }
    else
    {
        Reference< lang::XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
        if (xFac.is())
        {
            OSL_ENSURE( !xContext.is(), "### ignoring context!" );
            return xFac->createInstance();
        }
    }
    throw Exception(
        OUString( RTL_CONSTASCII_USTRINGPARAM("no factory object given!") ),
        Reference< XInterface >() );
}
//==================================================================================================
Reference< registry::XSimpleRegistry > SAL_CALL createSimpleRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance( loadSharedLibComponentFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("simreg") ), rBootstrapPath,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry") ),
                Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );
    }
    catch (Exception &)
    {
    }

    OSL_ENSURE( 0, "### cannot instanciate simple registry!" );
    return Reference< registry::XSimpleRegistry >();
}
//==================================================================================================
Reference< registry::XSimpleRegistry > SAL_CALL createNestedRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance( loadSharedLibComponentFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("defreg") ), rBootstrapPath,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry") ),
                Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );
    }
    catch (Exception &)
    {
    }

    OSL_ENSURE( 0, "### cannot instanciate simple registry!" );
    return Reference< registry::XSimpleRegistry >();
}

//--------------------------------------------------------------------------------------------------
Reference< lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xMgr( createInstance( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("smgr") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ORegistryServiceManager") ),
        Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );

    // write initial shared lib loader, simple registry, default registry, impl reg
    static char const * ar[] = {
        "cpld", "com.sun.star.comp.stoc.DLLComponentLoader",
        "simreg", "com.sun.star.comp.stoc.SimpleRegistry",
        "defreg", "com.sun.star.comp.stoc.NestedRegistry",
        "tdmgr", "com.sun.star.comp.stoc.TypeDescriptionManager",
        "impreg", "com.sun.star.comp.stoc.ImplementationRegistration",
        0
    };
    addFactories(
        ar, rBootstrapPath,
        xMgr, Reference< registry::XRegistryKey >() );

    return xMgr;
}
//--------------------------------------------------------------------------------------------------
// returns context with UNinitialized smgr
Reference< XComponentContext > bootstrapInitialContext(
    Reference< lang::XMultiComponentFactory > const & xSF,
    Reference< registry::XSimpleRegistry > const & types_xRegistry,
    Reference< registry::XSimpleRegistry > const & services_xRegistry,
    OUString const & rBootstrapPath,
    Bootstrap const & bootstrap )
    SAL_THROW( (Exception) )
{
    Reference< lang::XInitialization > xSFInit( xSF, UNO_QUERY );
    if (! xSFInit.is())
    {
        throw RuntimeException(
            OUSTR("servicemanager does not support XInitialization!"),
            Reference< XInterface >() );
    }

    // basic context values
    ContextEntry_Init entry;
    ::std::vector< ContextEntry_Init > context_values;
    context_values.reserve( 6 );

    // read out singleton infos from registry
    if (services_xRegistry.is())
    {
        Reference< registry::XRegistryKey > xKey( services_xRegistry->getRootKey() );
        if (xKey.is())
        {
            xKey = xKey->openKey( OUString( RTL_CONSTASCII_USTRINGPARAM("/SINGLETONS") ) );
            if (xKey.is())
            {
                entry.bLateInitService = true;

                Sequence< Reference< registry::XRegistryKey > > keys( xKey->openKeys() );
                Reference< registry::XRegistryKey > const * pKeys = keys.getConstArray();
                for ( sal_Int32 nPos = keys.getLength(); nPos--; )
                {
                    try
                    {
                        Reference< registry::XRegistryKey > const & xKey = pKeys[ nPos ];

                        OUStringBuffer buf( 32 );
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/singletons/") );
                        buf.append( xKey->getKeyName().copy( sizeof("/SINGLETONS") /* -\0 +'/' */ ) );
                        entry.name = buf.makeStringAndClear();
                        entry.value <<= xKey->getStringValue();
                        context_values.push_back( entry );
                    }
                    catch (Exception & rExc)
                    {
#ifdef _DEBUG
                        OString aStr( OUStringToOString(
                            xKey->getKeyName().copy( 11 ), RTL_TEXTENCODING_ASCII_US ) );
                        OString aStr2( OUStringToOString(
                            rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
                        ::fprintf( stderr, "### failed reading singleton [%s] service name from registry: %s\n", aStr.getStr(), aStr2.getStr() );
#endif
                    }
                }
            }
        }
    }

    // smgr
    entry.bLateInitService = false;
    entry.name = OUSTR("/singletons/com.sun.star.lang.theServiceManager");
    entry.value <<= xSF;
    context_values.push_back( entry );

    // ac
    entry.bLateInitService = false;
    entry.name = OUSTR(AC_SINGLETON);
    entry.value <<= createDefaultAccessController();
    context_values.push_back( entry );

    // tdmgr
    entry.bLateInitService = true;
    entry.name = OUSTR("/singletons/com.sun.star.reflection.theTypeDescriptionManager");
    entry.value <<= OUSTR("com.sun.star.reflection.TypeDescriptionManager");
    context_values.push_back( entry );

    // tdmgr: cache size
    entry.bLateInitService = false;
    entry.name = OUSTR("/implementations/com.sun.star.comp.stoc.TypeDescriptionManager/CacheSize");
    entry.value <<= (sal_Int32)512;
    context_values.push_back( entry );

    Reference< XComponentContext > xContext;
    Reference< container::XHierarchicalNameAccess > xTDMgr;

    // bootstrap type system
    if (types_xRegistry.is()) // use type registry?
    {
        // rdbtdp: registries to be used
        entry.bLateInitService = false;
        entry.name = OUSTR("/implementations/com.sun.star.comp.stoc.RegistryTypeDescriptionProvider/Registries");
        entry.value <<= Sequence< Reference< registry::XSimpleRegistry > >( &types_xRegistry, 1 );
        context_values.push_back( entry );

        xContext = createComponentContext(
            &context_values[ 0 ], context_values.size(),
            Reference< XComponentContext >() );

        if (xContext->getValueByName(
            OUSTR("/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) >>= xTDMgr)
        {
            // add registry td provider factory to smgr and instance to tdmgr
            Reference< lang::XSingleComponentFactory > xFac( loadSharedLibComponentFactory(
                OUSTR("rdbtdp"), rBootstrapPath,
                OUSTR("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"),
                Reference< lang::XMultiServiceFactory >( xSF, UNO_QUERY ),
                Reference< registry::XRegistryKey >() ), UNO_QUERY );
            OSL_ASSERT( xFac.is() );
            // smgr
            Reference< container::XSet > xSet( xSF, UNO_QUERY );
            xSet->insert( makeAny( xFac ) );
            OSL_ENSURE( xSet->has( makeAny( xFac ) ), "### failed registering registry td provider!" );
            // tdmgr
            xSet.set( xTDMgr, UNO_QUERY );
            OSL_ASSERT( xSet.is() );
            Any types_RDB( makeAny( types_xRegistry ) );
            Any rdbtdp( makeAny( xFac->createInstanceWithArgumentsAndContext(
                Sequence< Any >( &types_RDB, 1 ), xContext ) ) );
            xSet->insert( rdbtdp );
            OSL_ENSURE( xSet->has( rdbtdp ), "### failed inserting registry td provider to tdmgr!" );
        }
    }
    else // no type registry
    {
        xContext = createComponentContext(
            &context_values[ 0 ], context_values.size(),
            Reference< XComponentContext >() );
        xContext->getValueByName(
            OUSTR("/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) >>= xTDMgr;
    }

    if (xTDMgr.is())
    {
        // install callback
        installTypeDescriptionManager( xTDMgr );
    }

    // wrap ac for subsequent services
    OUString ac_service;
    if (bootstrap.getFrom( OUSTR("UNO_AC"), ac_service ) && ac_service.getLength())
    {
        // wrap ac
        ContextEntry_Init entry;
        entry.bLateInitService = true;
        entry.name = OUSTR(AC_SINGLETON);
        entry.value <<= ac_service;
        xContext = createComponentContext( &entry, 1, xContext );
    }

    return xContext;
}

//==================================================================================================
static Reference< lang::XMultiComponentFactory > createImplServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (Exception) )
{
//      osl_registerThreadCallbacks( parentThreadCallback, childThreadCallback );

    Reference< lang::XMultiComponentFactory > xSF( bootstrapInitialSF( rBootstrapPath ) );

    Reference< registry::XSimpleRegistry > xRegistry;

    // open a registry
    sal_Bool bRegistryShouldBeValid = sal_False;
    if (rWriteRegistry.getLength() && !rReadRegistry.getLength())
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createSimpleRegistry( rBootstrapPath ) );
        if (xRegistry.is())
        {
            if (bReadOnly)
            {
                xRegistry->open( rWriteRegistry, sal_True, sal_False );
            }
            else
            {
                xRegistry->open( rWriteRegistry, sal_False, sal_True );
            }
        }
    }
    else if (rWriteRegistry.getLength() && rReadRegistry.getLength()) // default registry
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createNestedRegistry( rBootstrapPath ) );

        Reference< registry::XSimpleRegistry > xWriteReg( createSimpleRegistry( rBootstrapPath ) );
        if (xWriteReg.is())
        {
            if (bReadOnly)
            {
                try
                {
                    xWriteReg->open( rWriteRegistry, sal_True, sal_False );
                }
                catch (registry::InvalidRegistryException &)
                {
                }

                if (! xWriteReg->isValid())
                {
                    throw Exception(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("specified first registry could not be open readonly!") ),
                        Reference< XInterface >() );
                }
            }
            else
            {
                xWriteReg->open( rWriteRegistry, sal_False, sal_True );
            }
        }

        Reference< registry::XSimpleRegistry > xReadReg( createSimpleRegistry( rBootstrapPath ) );
        if (xReadReg.is())
        {
            xReadReg->open( rReadRegistry, sal_True, sal_False );
        }

        Reference< lang::XInitialization > xInit( xRegistry, UNO_QUERY );
        Sequence< Any > aInitSeq( 2 );
        aInitSeq[ 0 ] <<= xWriteReg;
        aInitSeq[ 1 ] <<= xReadReg;
        xInit->initialize( aInitSeq );
    }

    if (bRegistryShouldBeValid && (!xRegistry.is() || !xRegistry->isValid()))
    {
        throw Exception(
            OUString( RTL_CONSTASCII_USTRINGPARAM("specified registry could not be initialized") ),
            Reference< XInterface >() );
    }

    Bootstrap bootstrap;
    Reference< XComponentContext > xContext(
        bootstrapInitialContext( xSF, xRegistry, xRegistry, rBootstrapPath, bootstrap ) );

    // initialize sf
    Reference< lang::XInitialization > xInit( xSF, UNO_QUERY );
    OSL_ASSERT( xInit.is() );
    Sequence< Any > aSFInit( 2 );
    aSFInit[ 0 ] <<= xRegistry;
    aSFInit[ 1 ] <<= xContext; // default context
    xInit->initialize( aSFInit );

    return xSF;
}

//==================================================================================================
Reference< lang::XMultiServiceFactory > SAL_CALL createRegistryServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    return Reference< lang::XMultiServiceFactory >( createImplServiceFactory(
        rWriteRegistry, rReadRegistry, bReadOnly, rBootstrapPath ), UNO_QUERY );
}

//==================================================================================================
Reference< XComponentContext > SAL_CALL bootstrap_InitialComponentContext(
    Reference< registry::XSimpleRegistry > const & xRegistry,
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Bootstrap bootstrap;

//      osl_registerThreadCallbacks( parentThreadCallback, childThreadCallback );

    Reference< lang::XMultiComponentFactory > xSF(
        bootstrapInitialSF( rBootstrapPath ) );
    Reference< XComponentContext > xContext(
        bootstrapInitialContext( xSF, xRegistry, xRegistry, rBootstrapPath, bootstrap ) );

    // initialize sf
    Reference< lang::XInitialization > xInit( xSF, UNO_QUERY );
    OSL_ASSERT( xInit.is() );
    Sequence< Any > aSFInit( 2 );
    aSFInit[ 0 ] <<= xRegistry;
    aSFInit[ 1 ] <<= xContext; // default context
    xInit->initialize( aSFInit );

    return xContext;
}

}
