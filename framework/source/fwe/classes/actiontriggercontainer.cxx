/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <classes/actiontriggercontainer.hxx>
#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

namespace framework
{

ActionTriggerContainer::ActionTriggerContainer() :
    PropertySetContainer()
{
}


ActionTriggerContainer::~ActionTriggerContainer()
{
}


Any SAL_CALL ActionTriggerContainer::queryInterface( const Type& aType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                aType ,
                (static_cast< XMultiServiceFactory* >(this)),
                (static_cast< XServiceInfo* >(this)),
                (static_cast< XTypeProvider* >(this)));

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


Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstance( const OUString& aServiceSpecifier )
throw ( ::com::sun::star::uno::Exception, RuntimeException)
{
    if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGER ))
        return (OWeakObject *)( new ActionTriggerPropertySet());
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERCONTAINER ))
        return (OWeakObject *)( new ActionTriggerContainer());
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERSEPARATOR ))
        return (OWeakObject *)( new ActionTriggerSeparatorPropertySet());
    else
        throw com::sun::star::uno::RuntimeException("Unknown service specifier!", (OWeakObject *)this );
}

Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/ )
throw ( Exception, RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL ActionTriggerContainer::getAvailableServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > aSeq( 3 );

    aSeq[0] = OUString( SERVICENAME_ACTIONTRIGGER );
    aSeq[1] = OUString( SERVICENAME_ACTIONTRIGGERCONTAINER );
    aSeq[2] = OUString( SERVICENAME_ACTIONTRIGGERSEPARATOR );

    return aSeq;
}


OUString SAL_CALL ActionTriggerContainer::getImplementationName()
throw ( RuntimeException )
{
    return OUString( IMPLEMENTATIONNAME_ACTIONTRIGGERCONTAINER );
}

sal_Bool SAL_CALL ActionTriggerContainer::supportsService( const OUString& ServiceName )
throw ( RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerContainer::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > seqServiceNames( 1 );

    seqServiceNames[0] = OUString( SERVICENAME_ACTIONTRIGGERCONTAINER );
    return seqServiceNames;
}


Sequence< Type > SAL_CALL ActionTriggerContainer::getTypes() throw ( RuntimeException )
{
    
    
    
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pTypeCollection == NULL )
        {
            
            static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const Reference< XMultiServiceFactory   >*)NULL ) ,
                        ::getCppuType(( const Reference< XIndexContainer        >*)NULL ) ,
                        ::getCppuType(( const Reference< XServiceInfo           >*)NULL ) ,
                        ::getCppuType(( const Reference< XTypeProvider          >*)NULL ) ) ;

            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerContainer::getImplementationId() throw ( RuntimeException )
{
    
    

    
    
    
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pID == NULL )
        {
            
            static ::cppu::OImplementationId aID( false ) ;
            
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
