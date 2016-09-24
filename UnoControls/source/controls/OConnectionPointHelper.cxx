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

#include "OConnectionPointHelper.hxx"

#include "OConnectionPointContainerHelper.hxx"

#include <cppuhelper/queryinterface.hxx>

//  namespaces

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace unocontrols{

//  construct/destruct

OConnectionPointHelper::OConnectionPointHelper(
    Mutex&                              aMutex                      ,
    OConnectionPointContainerHelper*    pContainerImplementation    ,
    Type                                aType
)   : m_aSharedMutex                ( aMutex                    )
    , m_oContainerWeakReference     ( pContainerImplementation  )
    , m_pContainerImplementation    ( pContainerImplementation  )
    , m_aInterfaceType              ( aType                     )
{
}

OConnectionPointHelper::~OConnectionPointHelper()
{
}

//  XInterface

Any SAL_CALL OConnectionPointHelper::queryInterface( const Type& aType ) throw( RuntimeException, std::exception )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                            static_cast< XConnectionPoint*  > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( !aReturn.hasValue() )
    {
        // ... ask baseclasses.
        aReturn = OWeakObject::queryInterface( aType );
    }

    return aReturn;
}

//  XInterface

void SAL_CALL OConnectionPointHelper::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//  XInterface

void SAL_CALL OConnectionPointHelper::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release();
}

//  XConnectionPoint

Type SAL_CALL OConnectionPointHelper::getConnectionType() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );

    // Set default return value, if method failed.
    if ( !impl_LockContainer() )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }

    // If container reference valid, return right type of supported interfaces of THIS connectionpoint.
    Type aReturnType = m_aInterfaceType;
    // Don't forget this!
    impl_UnlockContainer();

    return aReturnType;
}

//  XConnectionPoint

Reference< XConnectionPointContainer > SAL_CALL OConnectionPointHelper::getConnectionPointContainer() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Convert weakreference to correct uno3-reference and return value. It can be NULL, if container destroyed!
    return Reference< XConnectionPointContainer >( m_oContainerWeakReference.get(), UNO_QUERY );
}

//  XConnectionPoint

void SAL_CALL OConnectionPointHelper::advise( const Reference< XInterface >& xListener ) throw( ListenerExistException      ,
                                                                                                InvalidListenerException    ,
                                                                                                RuntimeException, std::exception            )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );

    // If type of listener not the same for this special container ...
    Any aCheckType = xListener->queryInterface( m_aInterfaceType );
    if ( aCheckType.hasValue() )
    {
        // ... throw an exception.
        throw InvalidListenerException();
    }

    // ListenerExistException is obsolete!?
    // Its the same container for XConnectionPointContainer and XConnectionPoint. But only here we must control, if a listener already exist!?
    // You can add a listener more than one time at XConnectionPointContainer, but here only one ...

    // Operation is permitted only, if reference to container is valid!
    if ( !impl_LockContainer() )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }
    // Forward it to OConnectionPointHelperContainer!
    m_pContainerImplementation->advise( m_aInterfaceType, xListener );
    // Don't forget this!
    impl_UnlockContainer();
}

//  XConnectionPoint

void SAL_CALL OConnectionPointHelper::unadvise( const Reference< XInterface >& xListener ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Operation is permitted only, if reference to container is valid!
    if ( !impl_LockContainer() )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();

    }
    // Forward it to OConnectionPointHelperContainer!
    m_pContainerImplementation->unadvise( m_aInterfaceType, xListener );
    // Don't forget this!
    impl_UnlockContainer();
}

//  XConnectionPoint

Sequence< Reference< XInterface > > SAL_CALL OConnectionPointHelper::getConnections() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Operation is permitted only, if reference to container is valid!
    if ( !impl_LockContainer() )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }
    // Set default return value, if method failed.
    Sequence< Reference< XInterface > > seqReturnConnections;
    // Get reference to private member of OConnectionPointHelperContainer!
    OMultiTypeInterfaceContainerHelper& aSharedContainer = m_pContainerImplementation->impl_getMultiTypeContainer();
    // Get pointer to specialized container which hold all interfaces of searched type.
    OInterfaceContainerHelper* pSpecialContainer = aSharedContainer.getContainer( m_aInterfaceType );
    // Get elements of searched type, if some else exist.
    if ( pSpecialContainer != nullptr )
    {
        seqReturnConnections = pSpecialContainer->getElements();
    }
    // Don't forget this!
    impl_UnlockContainer();

    return seqReturnConnections;
}

//  private method

bool OConnectionPointHelper::impl_LockContainer()
{
    // Convert weakreference to hard uno3-reference and return state.
    // If this reference different from NULL, there exist a hard reference to container. Container-instance can't be destroyed.
    // Don't forget to "unlock" this reference!
    m_xLock = m_oContainerWeakReference.get();
    return m_xLock.is();
}

//  private method

void OConnectionPointHelper::impl_UnlockContainer()
{
    // Free hard uno3-reference to container.
    // see also "impl_LockContainer()"
    m_xLock.clear();
}

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
