/*************************************************************************
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 14:19:39 $
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
#ifdef __cplusplus
extern "C"
{
#endif

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

void SAL_CALL registerSingleton( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const OUString& rSingletonName, const OUString& rServiceName )
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

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
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

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
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
