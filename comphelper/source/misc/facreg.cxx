/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:49:53 $
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

#include <string.h>

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include "rtl/ustrbuf.hxx"

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>

using namespace rtl;
using namespace com::sun::star;

// IndexedPropertyValuesContainer
extern uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer_getSupportedServiceNames() throw();
extern OUString SAL_CALL IndexedPropertyValuesContainer_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL IndexedPropertyValuesContainer_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// NamedPropertyValuesContainer
extern uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer_getSupportedServiceNames() throw();
extern OUString SAL_CALL NamedPropertyValuesContainer_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// AnyCompareFactory
extern uno::Sequence< OUString > SAL_CALL AnyCompareFactory_getSupportedServiceNames() throw();
extern OUString SAL_CALL AnyCompareFactory_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL AnyCompareFactory_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// OfficeInstallationDirectories
extern uno::Sequence< OUString > SAL_CALL OfficeInstallationDirectories_getSupportedServiceNames() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getImplementationName() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getSingletonName() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getSingletonServiceName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL OfficeInstallationDirectories_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

//
static void writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

static void registerSingleton( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const OUString& rSingletonName, const OUString& rServiceName )
{
    OUStringBuffer aSingletonKeyName;
    aSingletonKeyName.appendAscii( "/" );
    aSingletonKeyName.append( rImplementationName );
    aSingletonKeyName.appendAscii( "/UNO/SINGLETONS/" );
    aSingletonKeyName.append( rSingletonName );

    uno::Reference< registry::XRegistryKey >  xNewKey( pRegistryKey->createKey( aSingletonKeyName.makeStringAndClear() ) );
    OSL_ENSURE( xNewKey.is(), "could not create a registry key !");

    xNewKey->setStringValue( rServiceName );
}

//
extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            // IndexedPropertyValuesContainer
            writeInfo( pKey, IndexedPropertyValuesContainer_getImplementationName(), IndexedPropertyValuesContainer_getSupportedServiceNames() );
            // NamedPropertyValuesContainer
            writeInfo( pKey, NamedPropertyValuesContainer_getImplementationName(), NamedPropertyValuesContainer_getSupportedServiceNames() );
            // AnyCompareFactory
            writeInfo( pKey, AnyCompareFactory_getImplementationName(), AnyCompareFactory_getSupportedServiceNames() );
            // OfficeInstallationDirectories
            writeInfo( pKey, OfficeInstallationDirectories_getImplementationName(), OfficeInstallationDirectories_getSupportedServiceNames() );
            registerSingleton( pKey, OfficeInstallationDirectories_getImplementationName(), OfficeInstallationDirectories_getSingletonName(), OfficeInstallationDirectories_getSingletonServiceName() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( IndexedPropertyValuesContainer_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                IndexedPropertyValuesContainer_getImplementationName(),
                IndexedPropertyValuesContainer_createInstance,
                IndexedPropertyValuesContainer_getSupportedServiceNames() );
        }
        else if( NamedPropertyValuesContainer_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                NamedPropertyValuesContainer_getImplementationName(),
                NamedPropertyValuesContainer_createInstance,
                NamedPropertyValuesContainer_getSupportedServiceNames() );
        }
        else if( AnyCompareFactory_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                AnyCompareFactory_getImplementationName(),
                AnyCompareFactory_createInstance,
                AnyCompareFactory_getSupportedServiceNames() );
        }
        else if( OfficeInstallationDirectories_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                OfficeInstallationDirectories_getImplementationName(),
                OfficeInstallationDirectories_createInstance,
                OfficeInstallationDirectories_getSupportedServiceNames() );
        }

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}
