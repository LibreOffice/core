/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_services.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:26:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_RESOURCE_SERVICES_HXX
#include "res_services.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
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
