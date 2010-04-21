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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "res_services.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/registry/XRegistryKey.hpp>
/** === end UNO includes === **/

/** === begin UNO using === **/
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::uno::UNO_QUERY;
/** === end UNO using === **/

#include <vector>

namespace res
{
    ::std::vector< ComponentInfo > getComponentInfos()
    {
        ::std::vector< ::res::ComponentInfo > aComponentInfos;
        aComponentInfos.push_back( getComponentInfo_VclStringResourceLoader() );
        aComponentInfos.push_back( getComponentInfo_OpenOfficeResourceLoader() );
        return aComponentInfos;
    }
}

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, XRegistryKey * pRegistryKey )
{
    try
    {
        ::std::vector< ::res::ComponentInfo > aComponentInfos( ::res::getComponentInfos() );
        for (   ::std::vector< ::res::ComponentInfo >::const_iterator loop = aComponentInfos.begin();
                loop != aComponentInfos.end();
                ++loop
            )
        {
            Reference< XRegistryKey > xNewKey =
                pRegistryKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") )
                    + loop->sImplementationName + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ) ) );

            for( sal_Int32 i = 0; i < loop->aSupportedServices.getLength(); ++i )
                xNewKey->createKey( loop->aSupportedServices.getConstArray()[i]);

            if ( loop->sSingletonName.getLength() )
            {
                OSL_ENSURE( loop->aSupportedServices.getLength() == 1, "singletons must support exactly one service!" );
                xNewKey = pRegistryKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") )
                    + loop->sImplementationName + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SINGLETONS/" ) )
                    + loop->sSingletonName );
                xNewKey->setStringValue( loop->aSupportedServices[ 0 ] );
            }
        }

        return sal_True;
    }
    catch (Exception &)
    {
        // not allowed to throw an exception over the c function.
        //OSL_ENSURE( sal_False, "Exception cannot register component!" );
        return sal_False;
    }
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, XMultiServiceFactory * /*pServiceManager*/, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    ::std::vector< ::res::ComponentInfo > aComponentInfos( ::res::getComponentInfos() );
    for (   ::std::vector< ::res::ComponentInfo >::const_iterator loop = aComponentInfos.begin();
            loop != aComponentInfos.end();
            ++loop
        )
    {
        if ( 0 == loop->sImplementationName.compareToAscii( pImplName ) )
        {
            // create the factory
            Reference< XSingleServiceFactory > xFactory( ::cppu::createSingleComponentFactory(
                    loop->pFactory,  loop->sImplementationName, loop->aSupportedServices ),
                    UNO_QUERY );
            // acquire, because we return an interface pointer instead of a reference
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}   // extern "C"
