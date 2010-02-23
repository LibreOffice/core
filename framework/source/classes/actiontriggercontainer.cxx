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
#include "precompiled_framework.hxx"
#include <classes/actiontriggercontainer.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

namespace framework
{

ActionTriggerContainer::ActionTriggerContainer( const Reference< XMultiServiceFactory >& rServiceManager ) :
    PropertySetContainer( rServiceManager )
{
}


ActionTriggerContainer::~ActionTriggerContainer()
{
}

// XInterface
Any SAL_CALL ActionTriggerContainer::queryInterface( const Type& aType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                aType ,
                SAL_STATIC_CAST( XMultiServiceFactory*, this ),
                SAL_STATIC_CAST( XServiceInfo* ,  this ));

    if( a.hasValue() )
    {
        return a;
    }

    return PropertySetContainer::queryInterface( aType );
}

void ActionTriggerContainer::acquire() throw()
{
    PropertySetContainer::acquire();
}

void ActionTriggerContainer::release() throw()
{
    PropertySetContainer::release();
}


// XMultiServiceFactory
Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstance( const ::rtl::OUString& aServiceSpecifier )
throw ( ::com::sun::star::uno::Exception, RuntimeException)
{
    if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGER ))
        return (OWeakObject *)( new ActionTriggerPropertySet( m_xServiceManager ));
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERCONTAINER ))
        return (OWeakObject *)( new ActionTriggerContainer( m_xServiceManager ));
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERSEPARATOR ))
        return (OWeakObject *)( new ActionTriggerSeparatorPropertySet( m_xServiceManager ));
    else
        throw com::sun::star::uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unknown service specifier!" )), (OWeakObject *)this );
}


Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/ )
throw ( Exception, RuntimeException)
{
    return createInstance( ServiceSpecifier );
}


Sequence< ::rtl::OUString > SAL_CALL ActionTriggerContainer::getAvailableServiceNames()
throw ( RuntimeException )
{
    Sequence< ::rtl::OUString > aSeq( 3 );

    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGER ));
    aSeq[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERCONTAINER ));
    aSeq[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERSEPARATOR ));

    return aSeq;
}

// XServiceInfo
::rtl::OUString SAL_CALL ActionTriggerContainer::getImplementationName()
throw ( RuntimeException )
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATIONNAME_ACTIONTRIGGERCONTAINER ));
}

sal_Bool SAL_CALL ActionTriggerContainer::supportsService( const ::rtl::OUString& ServiceName )
throw ( RuntimeException )
{
    if ( ServiceName.equalsAscii( SERVICENAME_ACTIONTRIGGERCONTAINER ))
        return sal_True;

    return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL ActionTriggerContainer::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< ::rtl::OUString > seqServiceNames( 1 );

    seqServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERCONTAINER ));
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerContainer::getTypes() throw ( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const Reference< XMultiServiceFactory   >*)NULL ) ,
                        ::getCppuType(( const Reference< XIndexContainer        >*)NULL ) ,
                        ::getCppuType(( const Reference< XIndexAccess           >*)NULL ) ,
                        ::getCppuType(( const Reference< XIndexReplace          >*)NULL ) ,
                        ::getCppuType(( const Reference< XServiceInfo           >*)NULL ) ,
                        ::getCppuType(( const Reference< XTypeProvider          >*)NULL ) ) ;

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerContainer::getImplementationId() throw ( RuntimeException )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static ::cppu::OImplementationId aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

}

