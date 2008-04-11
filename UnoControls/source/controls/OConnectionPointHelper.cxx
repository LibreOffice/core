/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OConnectionPointHelper.cxx,v $
 * $Revision: 1.4 $
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

//______________________________________________________________________________________________________________
//  my own include
//______________________________________________________________________________________________________________

#include "OConnectionPointHelper.hxx"

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  include of my own project
//______________________________________________________________________________________________________________
#include "OConnectionPointContainerHelper.hxx"

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

OConnectionPointHelper::OConnectionPointHelper( Mutex&                              aMutex                      ,
                                                 OConnectionPointContainerHelper*   pContainerImplementation    ,
                                                UNO3_TYPE                           aType                       )
    : m_aSharedMutex                ( aMutex                    )
    , m_oContainerWeakReference     ( pContainerImplementation  )
    , m_pContainerImplementation    ( pContainerImplementation  )
    , m_aInterfaceType              ( aType                     )
{
}

OConnectionPointHelper::~OConnectionPointHelper()
{
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL OConnectionPointHelper::queryInterface( const Type& aType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XConnectionPoint*   > ( this )
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

void SAL_CALL OConnectionPointHelper::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OConnectionPointHelper::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release();
}

//______________________________________________________________________________________________________________
//  XConnectionPoint
//______________________________________________________________________________________________________________

Type SAL_CALL OConnectionPointHelper::getConnectionType() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );

    // Set default return value, if method failed.
    if ( impl_LockContainer() == sal_False )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }

    // If container reference valid, return right type of supported interfaces of THIS connectionpoint.
    Type aReturnType = m_aInterfaceType ;
    // Don't forget this!
    impl_UnlockContainer();

    return aReturnType;
}

//______________________________________________________________________________________________________________
//  XConnectionPoint
//______________________________________________________________________________________________________________

Reference< XConnectionPointContainer > SAL_CALL OConnectionPointHelper::getConnectionPointContainer() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Convert weakreference to correct uno3-reference and return value. It can be NULL, if container destroyed!
    return Reference< XConnectionPointContainer >( m_oContainerWeakReference.get(), UNO_QUERY );
}

//______________________________________________________________________________________________________________
//  XConnectionPoint
//______________________________________________________________________________________________________________

void SAL_CALL OConnectionPointHelper::advise( const Reference< XInterface >& xListener ) throw( ListenerExistException      ,
                                                                                                InvalidListenerException    ,
                                                                                                RuntimeException            )
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
    // You can add a listener more then one time at XConnectionPointContainer, but here only one ...

    // Operation is permitted only, if reference to container is valid!
    if ( impl_LockContainer() == sal_False )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }
    // Forward it to OConnectionPointHelperContainer!
    m_pContainerImplementation->advise( m_aInterfaceType, xListener );
    // Don't forget this!
    impl_UnlockContainer();
}

//______________________________________________________________________________________________________________
//  XConnectionPoint
//______________________________________________________________________________________________________________

void SAL_CALL OConnectionPointHelper::unadvise( const Reference< XInterface >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Operation is permitted only, if reference to container is valid!
    if ( impl_LockContainer() == sal_False )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();

    }
    // Forward it to OConnectionPointHelperContainer!
    m_pContainerImplementation->unadvise( m_aInterfaceType, xListener );
    // Don't forget this!
    impl_UnlockContainer();
}

//______________________________________________________________________________________________________________
//  XConnectionPoint
//______________________________________________________________________________________________________________

Sequence< Reference< XInterface > > SAL_CALL OConnectionPointHelper::getConnections() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aSharedMutex );
    // Operation is permitted only, if reference to container is valid!
    if ( impl_LockContainer() == sal_False )
    {
        // Container not exist! Its an runtime error.
        throw RuntimeException();
    }
    // Set default return value, if method failed.
    Sequence< Reference< XInterface > > seqReturnConnections = Sequence< Reference< XInterface > >();
    // Get reference to private member of OConnectionPointHelperContainer!
    OMultiTypeInterfaceContainerHelper& aSharedContainer = m_pContainerImplementation->impl_getMultiTypeContainer();
    // Get pointer to specialized container which hold all interfaces of searched type.
    OInterfaceContainerHelper* pSpecialContainer = aSharedContainer.getContainer( m_aInterfaceType );
    // Get elements of searched type, if somelse exist.
    if ( pSpecialContainer != NULL )
    {
        seqReturnConnections = pSpecialContainer->getElements();
    }
    // Don't forget this!
    impl_UnlockContainer();

    return seqReturnConnections;
}

//______________________________________________________________________________________________________________
//  private method
//______________________________________________________________________________________________________________

sal_Bool OConnectionPointHelper::impl_LockContainer()
{
    // Convert weakreference to hard uno3-reference and return state.
    // If this reference different from NULL, there exist a hard reference to container. Container-instance can't be destroyed.
    // Don't forget to "unlock" this reference!
    m_xLock = m_oContainerWeakReference.get();
    return m_xLock.is();
}

//______________________________________________________________________________________________________________
//  private method
//______________________________________________________________________________________________________________

void OConnectionPointHelper::impl_UnlockContainer()
{
    // Free hard uno3-reference to container.
    // see also "impl_LockContainer()"
    m_xLock = Reference< XInterface >();
}

}   // namespace unocontrols
