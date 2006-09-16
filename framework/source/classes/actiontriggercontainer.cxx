/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: actiontriggercontainer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:43:15 $
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
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_CLASSES_ACTIONTRIGGERCONTAINER_HXX_
#include <classes/actiontriggercontainer.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>

using namespace rtl;
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
        throw com::sun::star::uno::RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM( "Unknown service specifier!" )), (OWeakObject *)this );
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

    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGER ));
    aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERCONTAINER ));
    aSeq[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERSEPARATOR ));

    return aSeq;
}

// XServiceInfo
::rtl::OUString SAL_CALL ActionTriggerContainer::getImplementationName()
throw ( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATIONNAME_ACTIONTRIGGERCONTAINER ));
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

    seqServiceNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERCONTAINER ));
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
