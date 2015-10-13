/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <classes/actiontriggercontainer.hxx>
#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <cppuhelper/queryinterface.hxx>
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

// XInterface
Any SAL_CALL ActionTriggerContainer::queryInterface( const Type& aType )
throw ( RuntimeException, std::exception )
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

// XMultiServiceFactory
Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstance( const OUString& aServiceSpecifier )
throw ( css::uno::Exception, RuntimeException, std::exception)
{
    if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGER )
        return static_cast<OWeakObject *>( new ActionTriggerPropertySet());
    else if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGERCONTAINER )
        return static_cast<OWeakObject *>( new ActionTriggerContainer());
    else if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGERSEPARATOR )
        return static_cast<OWeakObject *>( new ActionTriggerSeparatorPropertySet());
    else
        throw css::uno::RuntimeException("Unknown service specifier!", static_cast<OWeakObject *>(this) );
}

Reference< XInterface > SAL_CALL ActionTriggerContainer::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/ )
throw ( Exception, RuntimeException, std::exception)
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL ActionTriggerContainer::getAvailableServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence< OUString > aSeq( 3 );

    aSeq[0] = SERVICENAME_ACTIONTRIGGER;
    aSeq[1] = SERVICENAME_ACTIONTRIGGERCONTAINER;
    aSeq[2] = SERVICENAME_ACTIONTRIGGERSEPARATOR;

    return aSeq;
}

// XServiceInfo
OUString SAL_CALL ActionTriggerContainer::getImplementationName()
throw ( RuntimeException, std::exception )
{
    return OUString( IMPLEMENTATIONNAME_ACTIONTRIGGERCONTAINER );
}

sal_Bool SAL_CALL ActionTriggerContainer::supportsService( const OUString& ServiceName )
throw ( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerContainer::getSupportedServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence< OUString > seqServiceNames( 1 );

    seqServiceNames[0] = SERVICENAME_ACTIONTRIGGERCONTAINER;
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerContainer::getTypes() throw ( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static ::cppu::OTypeCollection* pTypeCollection = NULL;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                        cppu::UnoType<XMultiServiceFactory>::get(),
                        cppu::UnoType<XIndexContainer>::get(),
                        cppu::UnoType<XServiceInfo>::get(),
                        cppu::UnoType<XTypeProvider>::get());

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes();
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerContainer::getImplementationId() throw ( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
