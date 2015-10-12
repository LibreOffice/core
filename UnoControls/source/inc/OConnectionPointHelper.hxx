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

#ifndef INCLUDED_UNOCONTROLS_SOURCE_INC_OCONNECTIONPOINTHELPER_HXX
#define INCLUDED_UNOCONTROLS_SOURCE_INC_OCONNECTIONPOINTHELPER_HXX

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <com/sun/star/lang/XConnectionPoint.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/propshlp.hxx>

#include "OConnectionPointContainerHelper.hxx"

//  namespaces

namespace unocontrols{

//  class declaration OConnectionPointHelper

class OConnectionPointHelper    :   public  ::com::sun::star::lang::XConnectionPoint
                                ,   public  ::cppu::OWeakObject
{

//  public methods

public:

    //  construct/destruct

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
                            ::com::sun::star::uno::Type                       aType                       );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~OConnectionPointHelper();

    //  XInterface

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() override;

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() override;

    //  XConnectionPoint

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Type SAL_CALL getConnectionType()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XConnectionPointContainer > SAL_CALL getConnectionPointContainer()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xListener
    ) throw (
        ::com::sun::star::lang::ListenerExistException,
        ::com::sun::star::lang::InvalidListenerException ,
        ::com::sun::star::uno::RuntimeException, std::exception
    ) override;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xListener )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > SAL_CALL getConnections()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

//  private methods

private:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    bool impl_LockContainer();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_UnlockContainer();

//  private variables

private:

    ::osl::Mutex&                                                   m_aSharedMutex;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XConnectionPointContainer >   m_oContainerWeakReference;   // Reference to container-class!. Don't use Reference<...>
                                                                                            // It is a ring-reference => and must be a wekreference!
    OConnectionPointContainerHelper*                                m_pContainerImplementation;
    ::com::sun::star::uno::Type                                                   m_aInterfaceType;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >                       m_xLock;

};  // class OConnectionPointHelper

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_SOURCE_INC_OCONNECTIONPOINTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
