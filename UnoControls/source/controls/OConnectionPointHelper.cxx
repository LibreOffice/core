/*************************************************************************
 *
 *  $RCSfile: OConnectionPointHelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:11:17 $
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

//______________________________________________________________________________________________________________
//  my own include
//______________________________________________________________________________________________________________

#ifndef _OCONNECTIONPOINTHELPER_HXX
#include "OConnectionPointHelper.hxx"
#endif

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  include of my own project
//______________________________________________________________________________________________________________

#ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
#include "OConnectionPointContainerHelper.hxx"
#endif

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

void SAL_CALL OConnectionPointHelper::acquire() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OConnectionPointHelper::release() throw( RuntimeException )
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
