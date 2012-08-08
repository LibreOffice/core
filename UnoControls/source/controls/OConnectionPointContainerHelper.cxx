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

#include "OConnectionPointContainerHelper.hxx"

#include "OConnectionPointHelper.hxx"

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;

namespace unocontrols{

//______________________________________________________________________________________________________________
//  construct/destruct
//______________________________________________________________________________________________________________

OConnectionPointContainerHelper::OConnectionPointContainerHelper( Mutex& aMutex )
    : m_aSharedMutex        ( aMutex    )
    , m_aMultiTypeContainer ( aMutex    )
{
}

OConnectionPointContainerHelper::~OConnectionPointContainerHelper()
{
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL OConnectionPointContainerHelper::queryInterface( const Type& aType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    Any aReturn ( ::cppu::queryInterface(   aType                                               ,
                                               static_cast< XConnectionPointContainer*  > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( aReturn.hasValue() == sal_False )
    {
        // ... ask baseclasses.
        aReturn = OWeakObject::queryInterface( aType );
    }

    return aReturn ;
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OConnectionPointContainerHelper::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OConnectionPointContainerHelper::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release();
}

//______________________________________________________________________________________________________________
//  XConnectionPointContainer
//______________________________________________________________________________________________________________

Sequence< Type > SAL_CALL OConnectionPointContainerHelper::getConnectionPointTypes() throw( RuntimeException )
{
    // Container is threadsafe himself !
    return m_aMultiTypeContainer.getContainedTypes();
}

//______________________________________________________________________________________________________________
//  XConnectionPointContainer
//______________________________________________________________________________________________________________

Reference< XConnectionPoint > SAL_CALL OConnectionPointContainerHelper::queryConnectionPoint( const Type& aType ) throw( RuntimeException )
{
    // Set default return value, if method failed.
    Reference< XConnectionPoint > xConnectionPoint = Reference< XConnectionPoint >();

    // Get all elements of the container, which have the searched type.
    OInterfaceContainerHelper* pSpecialContainer = m_aMultiTypeContainer.getContainer( aType );
    if ( pSpecialContainer && pSpecialContainer->getLength() > 0 )
    {
        // Ready for multithreading
        MutexGuard aGuard( m_aSharedMutex );
        // If this container contains elements, build a connectionpoint-instance.
        OConnectionPointHelper* pNewConnectionPoint = new OConnectionPointHelper( m_aSharedMutex, this, aType );
        xConnectionPoint = Reference< XConnectionPoint >( (OWeakObject*)pNewConnectionPoint, UNO_QUERY );
    }

    return xConnectionPoint ;
}

//______________________________________________________________________________________________________________
//  XConnectionPointContainer
//______________________________________________________________________________________________________________

void SAL_CALL OConnectionPointContainerHelper::advise(  const   Type&                       aType       ,
                                                        const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    // Container is threadsafe himself !
    m_aMultiTypeContainer.addInterface( aType, xListener );
}

//______________________________________________________________________________________________________________
//  XConnectionPointContainer
//______________________________________________________________________________________________________________

void SAL_CALL OConnectionPointContainerHelper::unadvise(    const   Type&                       aType       ,
                                                            const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    // Container is threadsafe himself !
    m_aMultiTypeContainer.removeInterface( aType, xListener );
}

//______________________________________________________________________________________________________________
//  public but impl method!
//  Is neccessary to get container member at OConnectionPoint-instance.
//______________________________________________________________________________________________________________

OMultiTypeInterfaceContainerHelper& OConnectionPointContainerHelper::impl_getMultiTypeContainer()
{
    // Impl methods are not threadsafe!
    // "Parent" function must do this.
    return m_aMultiTypeContainer;
}

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
