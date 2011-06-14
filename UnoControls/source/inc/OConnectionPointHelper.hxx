/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _OCONNECTIONPOINTHELPER_HXX
#define _OCONNECTIONPOINTHELPER_HXX

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
#include "OConnectionPointContainerHelper.hxx"

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO     ::com::sun::star::uno
#define CSS_LANG    ::com::sun::star::lang

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  class declaration OConnectionPointHelper
//______________________________________________________________________________________________________________

class OConnectionPointHelper    :   public  CSS_LANG::XConnectionPoint
                                ,   public  ::cppu::OWeakObject
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

    OConnectionPointHelper( ::osl::Mutex&                       aMutex                      ,
                            OConnectionPointContainerHelper*    pContainerImplementation    ,
                            CSS_UNO::Type                       aType                       );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~OConnectionPointHelper();

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

    virtual CSS_UNO::Any SAL_CALL queryInterface( const CSS_UNO::Type& aType )
        throw( CSS_UNO::RuntimeException );

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
    //  XConnectionPoint
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Type SAL_CALL getConnectionType()
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Reference< CSS_LANG::XConnectionPointContainer > SAL_CALL getConnectionPointContainer()
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(
        const CSS_UNO::Reference< CSS_UNO::XInterface >& xListener
    ) throw (
        CSS_LANG::ListenerExistException,
        CSS_LANG::InvalidListenerException ,
        CSS_UNO::RuntimeException
    );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise( const CSS_UNO::Reference< CSS_UNO::XInterface >& xListener )
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Sequence< CSS_UNO::Reference< CSS_UNO::XInterface > > SAL_CALL getConnections()
        throw( CSS_UNO::RuntimeException );

//______________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________

private:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    sal_Bool impl_LockContainer();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_UnlockContainer();

//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

private:

    ::osl::Mutex&                                                   m_aSharedMutex              ;
    CSS_UNO::WeakReference< CSS_LANG::XConnectionPointContainer >   m_oContainerWeakReference   ;   // Reference to container-class!. Don't use Reference<...>
                                                                                            // It is a ring-reference => and must be a wekreference!
    OConnectionPointContainerHelper*                                m_pContainerImplementation  ;
    CSS_UNO::Type                                                   m_aInterfaceType            ;
    CSS_UNO::Reference< CSS_UNO::XInterface >                       m_xLock                     ;

};  // class OConnectionPointHelper

// The namespace aliases are only used in this header
#undef CSS_UNO
#undef CSS_LANG

}   // namespace unocontrols

#endif  // #ifndef _OCONNECTIONPOINTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
