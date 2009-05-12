/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graphicuno.cxx,v $
 * $Revision: 1.6 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>
#include "provider.hxx"
#include "renderer.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include "comphelper/servicedecl.hxx"

using namespace com::sun::star;
namespace sdecl = comphelper::service_decl;

namespace unographic {
extern sdecl::ServiceDecl const serviceDecl;
// --------------------
// - *_createInstance -
// --------------------

static uno::Reference< uno::XInterface > SAL_CALL GraphicProvider_createInstance( const uno::Reference< lang::XMultiServiceFactory >& )
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicProvider );
}

// -----------------------------------------------------------------------------

static uno::Reference< uno::XInterface > SAL_CALL GraphicRendererVCL_createInstance( const uno::Reference< lang::XMultiServiceFactory >&)
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicRendererVCL );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            uno::Reference< registry::XRegistryKey >    xNewKey;
            uno::Sequence< ::rtl::OUString >            aServices;

            // GraphicProvider
            xNewKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey )->createKey(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                        GraphicProvider::getImplementationName_Static() +
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) );

            aServices = GraphicProvider::getSupportedServiceNames_Static();

            int i;
            for( i = 0; i < aServices.getLength(); i++ )
                xNewKey->createKey( aServices.getConstArray()[ i ] );

            // GraphicRendererVCL
            xNewKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey )->createKey(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                        GraphicRendererVCL::getImplementationName_Static() +
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) );

            aServices = ( GraphicRendererVCL::getSupportedServiceNames_Static() );

            for( i = 0; i < aServices.getLength(); i++ )
                xNewKey->createKey( aServices.getConstArray()[ i ] );

               if ( !component_writeInfoHelper( reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ), reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ), serviceDecl ) )
                return false;

            bRet = true;
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    void * pRet = 0;

    if( pServiceManager && ( 0 == GraphicProvider::getImplementationName_Static().compareToAscii( pImplName ) ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( ::cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            GraphicProvider::getImplementationName_Static(),
            GraphicProvider_createInstance,
            GraphicProvider::getSupportedServiceNames_Static() ) );

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    else if( pServiceManager && ( 0 == GraphicRendererVCL::getImplementationName_Static().compareToAscii( pImplName ) ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( ::cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            GraphicRendererVCL::getImplementationName_Static(),
            GraphicRendererVCL_createInstance,
            GraphicRendererVCL::getSupportedServiceNames_Static() ) );

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    else
    {
        pRet =  component_getFactoryHelper( pImplName, reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ), serviceDecl );

    }
    return pRet;
}

}
