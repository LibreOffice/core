/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:23:53 $
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
#include "precompiled_comphelper.hxx"

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

#include "instancelocker.hxx"

using namespace rtl;
using namespace com::sun::star;

// IndexedPropertyValuesContainer
extern uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer_getSupportedServiceNames() throw();
extern OUString SAL_CALL IndexedPropertyValuesContainer_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL IndexedPropertyValuesContainer_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );

// NamedPropertyValuesContainer
extern uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer_getSupportedServiceNames() throw();
extern OUString SAL_CALL NamedPropertyValuesContainer_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );

// AnyCompareFactory
extern uno::Sequence< OUString > SAL_CALL AnyCompareFactory_getSupportedServiceNames() throw();
extern OUString SAL_CALL AnyCompareFactory_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL AnyCompareFactory_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );

// OfficeInstallationDirectories
extern uno::Sequence< OUString > SAL_CALL OfficeInstallationDirectories_getSupportedServiceNames() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getImplementationName() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getSingletonName() throw();
extern OUString SAL_CALL OfficeInstallationDirectories_getSingletonServiceName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL OfficeInstallationDirectories_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );

// SequenceInputStreamService
extern uno::Sequence< OUString > SAL_CALL SequenceInputStreamService_getSupportedServiceNames() throw();
extern OUString SAL_CALL SequenceInputStreamService_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SequenceInputStreamService_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );

//SequenceOutputStreamService
extern uno::Sequence< OUString > SAL_CALL SequenceOutputStreamService_getSupportedServiceNames() throw();
extern OUString SAL_CALL SequenceOutputStreamService_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SequenceOutputStreamService_createInstance(const uno::Reference< uno::XComponentContext >& rxContext) throw( uno::Exception );

namespace comphelper
{
// UNOMemoryStream
extern uno::Sequence< OUString > SAL_CALL UNOMemoryStream_getSupportedServiceNames() throw();
extern OUString SAL_CALL UNOMemoryStream_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL UNOMemoryStream_createInstance(const uno::Reference< uno::XComponentContext > & rxContext) throw( uno::Exception );
}

// PropertyBag
extern uno::Sequence< OUString > SAL_CALL PropertyBag_getSupportedServiceNames() throw();
extern OUString SAL_CALL PropertyBag_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL PropertyBag_createInstance(const uno::Reference< uno::XComponentContext >& rxContext) throw( uno::Exception );

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

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void *, void * pRegistryKey )
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

            // InstanceLocker
            writeInfo( pKey, OInstanceLocker::impl_staticGetImplementationName(), OInstanceLocker::impl_staticGetSupportedServiceNames() );
            // SequenceInputStreamService
            writeInfo( pKey, SequenceInputStreamService_getImplementationName(), SequenceInputStreamService_getSupportedServiceNames() );
            // SequenceOutputStreamService
            writeInfo( pKey, SequenceOutputStreamService_getImplementationName(), SequenceOutputStreamService_getSupportedServiceNames() );
            // UNOMemoryStream
            writeInfo( pKey, comphelper::UNOMemoryStream_getImplementationName(), comphelper::UNOMemoryStream_getSupportedServiceNames() );
            // PropertyBag
            writeInfo( pKey, PropertyBag_getImplementationName(), PropertyBag_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference<lang::XSingleComponentFactory> xComponentFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( IndexedPropertyValuesContainer_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                IndexedPropertyValuesContainer_createInstance,
                IndexedPropertyValuesContainer_getImplementationName(),
                IndexedPropertyValuesContainer_getSupportedServiceNames() );
        }
        else if( NamedPropertyValuesContainer_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                NamedPropertyValuesContainer_createInstance,
                NamedPropertyValuesContainer_getImplementationName(),
                NamedPropertyValuesContainer_getSupportedServiceNames() );
        }
        else if( AnyCompareFactory_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                AnyCompareFactory_createInstance,
                AnyCompareFactory_getImplementationName(),
                AnyCompareFactory_getSupportedServiceNames() );
        }
        else if( OfficeInstallationDirectories_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                OfficeInstallationDirectories_createInstance,
                OfficeInstallationDirectories_getImplementationName(),
                OfficeInstallationDirectories_getSupportedServiceNames() );
        }
        else if( OInstanceLocker::impl_staticGetImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                OInstanceLocker::impl_staticCreateSelfInstance,
                OInstanceLocker::impl_staticGetImplementationName(),
                OInstanceLocker::impl_staticGetSupportedServiceNames() );
        }
        else if( SequenceInputStreamService_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                SequenceInputStreamService_createInstance,
                SequenceInputStreamService_getImplementationName(),
                SequenceInputStreamService_getSupportedServiceNames() );
        }
        else if( comphelper::UNOMemoryStream_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                comphelper::UNOMemoryStream_createInstance,
                comphelper::UNOMemoryStream_getImplementationName(),
                comphelper::UNOMemoryStream_getSupportedServiceNames() );
        }
        else if ( SequenceOutputStreamService_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                SequenceOutputStreamService_createInstance,
                SequenceOutputStreamService_getImplementationName(),
                SequenceOutputStreamService_getSupportedServiceNames() );
        }
        else if ( PropertyBag_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xComponentFactory = ::cppu::createSingleComponentFactory(
                PropertyBag_createInstance,
                PropertyBag_getImplementationName(),
                PropertyBag_getSupportedServiceNames() );
        }

        if( xComponentFactory.is())
        {
            xComponentFactory->acquire();
            pRet = xComponentFactory.get();
        }
    }
    return pRet;
}

}
