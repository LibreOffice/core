/*************************************************************************
 *
 *  $RCSfile: servicefactory.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: dbo $ $Date: 2001-05-08 15:56:02 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <cppuhelper/tdmgr.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

using namespace rtl;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;
using namespace com::sun::star;


namespace cppu
{

//==================================================================================================
Reference< XComponentContext > SAL_CALL createInitialComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< registry::XSimpleRegistry > const & xRegistry )
    SAL_THROW( () );

//==================================================================================================
static Reference< XInterface > SAL_CALL createInstance(
    Reference< XInterface > const & xFactory,
    Reference< XComponentContext > const & xContext = Reference< XComponentContext >() )
{
    Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
    if (xFac.is())
    {
        return xFac->createInstanceWithContext( xContext );
    }
    else
    {
        Reference< XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
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
static Reference< XMultiServiceFactory > createImplServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    Reference< XInterface > xSMFac( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("smgr") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ORegistryServiceManager") ),
        Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() ) );

    Reference< XMultiServiceFactory > xSF( createInstance( xSMFac ), UNO_QUERY );
    Reference< XInitialization > xSFInit( xSF, UNO_QUERY );
    if (! xSFInit.is())
    {
        throw Exception(
            OUString( RTL_CONSTASCII_USTRINGPARAM("servicemanager does not support XInitialization!") ),
            Reference< XInterface >() );
    }

    Reference< XInterface > xSimRegFac( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("simreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry") ),
        xSF, Reference< XRegistryKey >() ) );

    Reference< XInterface > xNesRegFac( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("defreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry") ),
        xSF, Reference< XRegistryKey >() ) );

    // write initial shared lib loader, simple registry, default registry, impl reg
    Reference< XSet > xSet( xSF, UNO_QUERY );

    // loader
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("cpld") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.DLLComponentLoader") ),
        xSF, Reference< XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering loader!" );
    }
    // simple registry
    xSet->insert( makeAny( xSimRegFac ) );
    OSL_ENSURE( xSet->has( makeAny( xSimRegFac ) ), "### failed registering simple registry!" );
    // nested registry
    xSet->insert( makeAny( xNesRegFac ) );
    OSL_ENSURE( xSet->has( makeAny( xNesRegFac ) ), "### failed registering default registry!" );
    // implementation registration
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("impreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ImplementationRegistration") ),
        xSF, Reference< XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering impl reg!" );
    }

    Reference< XSimpleRegistry > xRegistry;

    // open a registry
    sal_Bool bRegistryShouldBeValid = sal_False;
    if (rWriteRegistry.getLength() && !rReadRegistry.getLength())
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createInstance( xSimRegFac ), UNO_QUERY );
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
        xRegistry.set( createInstance( xNesRegFac ), UNO_QUERY );

        Reference< XSimpleRegistry > xWriteReg( createInstance( xSimRegFac ), UNO_QUERY );
        if (xWriteReg.is())
        {
            if (bReadOnly)
            {
                try
                {
                    xWriteReg->open( rWriteRegistry, sal_True, sal_False );
                }
                catch (InvalidRegistryException &)
                {
                }

                if (!xWriteReg->isValid())
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

        Reference< XSimpleRegistry > xReadReg( createInstance( xSimRegFac ), UNO_QUERY );
        if (xReadReg.is())
        {
            xReadReg->open( rReadRegistry, sal_True, sal_False );
        }

        Reference< XInitialization > xInit( xRegistry, UNO_QUERY );
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

    // default initial context
    ContextEntry_Init context_values[ 4 ];
    sal_Int32 nEntries = 1;
    // smgr
    context_values[ 0 ].bLateInitService = false;
    context_values[ 0 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.ServiceManager") );
    context_values[ 0 ].value = makeAny( xSF );

        // additional service factories
        // registry td provider
        {
        Any aFac( makeAny( loadSharedLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM("rdbtdp") ), rBootstrapPath,
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider") ),
            xSF, Reference< XRegistryKey >() ) ) );
        xSet->insert( aFac );
        OSL_ENSURE( xSet->has( aFac ), "### failed registering registry td provider!" );
        }
        // td manager
        {
        Any aFac( makeAny( loadSharedLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM("tdmgr") ), rBootstrapPath,
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.TypeDescriptionManager") ),
            xSF, Reference< XRegistryKey >() ) ) );
        xSet->insert( aFac );
        OSL_ENSURE( xSet->has( aFac ), "### failed registering registry td provider!" );
        }

    // xxx todo
    if (xRegistry.is())
    {
        // rdbtdp: registries to be used
        context_values[ 1 ].bLateInitService = false;
        context_values[ 1 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider.Registries") );
        context_values[ 1 ].value = makeAny( Sequence< Reference< XSimpleRegistry > >( &xRegistry, 1 ) );
        // tdmgr: cache size
        context_values[ 2 ].bLateInitService = false;
        context_values[ 2 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.TypeDescriptionManager.CacheSize") );
        context_values[ 2 ].value = makeAny( (sal_Int32)512 );
        // tdmgr
        context_values[ 3 ].bLateInitService = true;
        context_values[ 3 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionManager") );
        context_values[ 3 ].value = makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionManager") ) );

        nEntries = 4;
    }

    Reference< XComponentContext > xContext( createInitialComponentContext(
        context_values, nEntries, xRegistry ) );

    Sequence< Any > aSFInit( 2 );
    aSFInit[ 0 ] <<= xRegistry;
    aSFInit[ 1 ] <<= xContext;
    xSFInit->initialize( aSFInit );

    Reference< container::XHierarchicalNameAccess > xTDMgr;
    if (xContext->getValueByName( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.reflection.TypeDescriptionManager") ) ) >>= xTDMgr)
    {
        installTypeDescriptionManager( xTDMgr );
    }

    return xSF;
}

//==================================================================================================
Reference< XMultiServiceFactory > SAL_CALL createRegistryServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return createImplServiceFactory( rWriteRegistry, rReadRegistry, bReadOnly, rBootstrapPath );
}

}

