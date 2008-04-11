/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OConnectionPointContainerHelper.hxx,v $
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

#ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
#define _OCONNECTIONPOINTCONTAINERHELPER_HXX

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <com/sun/star/lang/XConnectionPoint.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

//______________________________________________________________________________________________________________
//  includes of my own project
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

#define UNO3_ANY                                        ::com::sun::star::uno::Any
#define UNO3_SEQUENCE                                   ::com::sun::star::uno::Sequence
#define UNO3_TYPE                                       ::com::sun::star::uno::Type
#define UNO3_REFERENCE                                  ::com::sun::star::uno::Reference
#define UNO3_XCONNECTIONPOINTCONTAINER                  ::com::sun::star::lang::XConnectionPointContainer
#define UNO3_XCONNECTIONPOINT                           ::com::sun::star::lang::XConnectionPoint
#define UNO3_MUTEX                                      ::osl::Mutex
#define UNO3_RUNTIMEEXCEPTION                           ::com::sun::star::uno::RuntimeException
#define UNO3_XINTERFACE                                 ::com::sun::star::uno::XInterface
#define UNO3_OMULTITYPEINTERFACECONTAINERHELPER         ::cppu::OMultiTypeInterfaceContainerHelper
#define UNO3_LISTENEREXISTEXCEPTION                     ::com::sun::star::lang::ListenerExistException
#define UNO3_INVALIDLISTENEREXCEPTION                   ::com::sun::star::lang::InvalidListenerException
#define UNO3_WEAKREFERENCE                              ::com::sun::star::uno::WeakReference
#define UNO3_OWEAKOBJECT                                ::cppu::OWeakObject

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  class declaration OConnectionPointContainerHelper
//______________________________________________________________________________________________________________

class OConnectionPointContainerHelper   :   public  UNO3_XCONNECTIONPOINTCONTAINER
                                        ,   public  UNO3_OWEAKOBJECT
{

//______________________________________________________________________________________________________________
//  public methods
//______________________________________________________________________________________________________________

public:

    //__________________________________________________________________________________________________________
    //  construct/destruct
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    OConnectionPointContainerHelper( UNO3_MUTEX& aMutex );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~OConnectionPointContainerHelper();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //__________________________________________________________________________________________________________
    //  XConnectionPointContainer
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getConnectionPointTypes() throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_REFERENCE< UNO3_XCONNECTIONPOINT > SAL_CALL queryConnectionPoint( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(   const   UNO3_TYPE&                          aType       ,
                                    const   UNO3_REFERENCE< UNO3_XINTERFACE >&  xListener   ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise( const   UNO3_TYPE&                          aType       ,
                                    const   UNO3_REFERENCE< UNO3_XINTERFACE >&  xListener   ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    UNO3_OMULTITYPEINTERFACECONTAINERHELPER& impl_getMultiTypeContainer();

//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

private:

    UNO3_MUTEX&                                 m_aSharedMutex          ;
    UNO3_OMULTITYPEINTERFACECONTAINERHELPER     m_aMultiTypeContainer   ;   // Container to hold listener

};  // class OConnectionPointContainerHelper

}   // namespace unocontrols

#endif  // #ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
