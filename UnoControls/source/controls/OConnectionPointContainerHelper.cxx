/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OConnectionPointContainerHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:17:17 $
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

//______________________________________________________________________________________________________________
//  my own include
//______________________________________________________________________________________________________________

#ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
#include "OConnectionPointContainerHelper.hxx"
#endif

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  include of my own project
//______________________________________________________________________________________________________________

#ifndef _OCONNECTIONPOINTHELPER_HXX
#include "OConnectionPointHelper.hxx"
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
    if ( pSpecialContainer->getLength() > 0 )
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
