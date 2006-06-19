/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: register.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:28:52 $
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


#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#include "xfactory.hxx"
#include "xcreator.hxx"

using namespace ::com::sun::star;


extern "C" {

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( pServiceManager )
    {
        if ( aImplName.equals( OOoEmbeddedObjectFactory::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                OOoEmbeddedObjectFactory::impl_staticGetImplementationName(),
                                                OOoEmbeddedObjectFactory::impl_staticCreateSelfInstance,
                                                OOoEmbeddedObjectFactory::impl_staticGetSupportedServiceNames() );
        }
        else if ( aImplName.equals( OOoSpecialEmbeddedObjectFactory::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                OOoSpecialEmbeddedObjectFactory::impl_staticGetImplementationName(),
                                                OOoSpecialEmbeddedObjectFactory::impl_staticCreateSelfInstance,
                                                OOoSpecialEmbeddedObjectFactory::impl_staticGetSupportedServiceNames() );
        }
        else if ( aImplName.equals( UNOEmbeddedObjectCreator::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                UNOEmbeddedObjectCreator::impl_staticGetImplementationName(),
                                                UNOEmbeddedObjectCreator::impl_staticCreateSelfInstance,
                                                UNOEmbeddedObjectCreator::impl_staticGetSupportedServiceNames() );
        }
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            sal_Int32 nInd = 0;
            uno::Reference< registry::XRegistryKey > xKey( reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ) );

            uno::Reference< registry::XRegistryKey >  xNewKey;

            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        OOoEmbeddedObjectFactory::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );
            uno::Sequence< ::rtl::OUString > rServices = OOoEmbeddedObjectFactory::impl_staticGetSupportedServiceNames();
            for( nInd = 0; nInd < rServices.getLength(); nInd++ )
                xNewKey->createKey( rServices.getConstArray()[nInd] );

            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        OOoSpecialEmbeddedObjectFactory::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );
            rServices = OOoSpecialEmbeddedObjectFactory::impl_staticGetSupportedServiceNames();
            for( nInd = 0; nInd < rServices.getLength(); nInd++ )
                xNewKey->createKey( rServices.getConstArray()[nInd] );

            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        UNOEmbeddedObjectCreator::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );
            rServices = UNOEmbeddedObjectCreator::impl_staticGetSupportedServiceNames();
            for( nInd = 0; nInd < rServices.getLength(); nInd++ )
                xNewKey->createKey( rServices.getConstArray()[nInd] );

            return sal_True;
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

} // extern "C"

