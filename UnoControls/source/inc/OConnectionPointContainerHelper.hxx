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

#ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
#define _OCONNECTIONPOINTCONTAINERHELPER_HXX

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <com/sun/star/lang/XConnectionPoint.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

//  namespaces

namespace unocontrols{

//  class declaration OConnectionPointContainerHelper

class OConnectionPointContainerHelper   :   public  ::com::sun::star::lang::XConnectionPointContainer
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

    OConnectionPointContainerHelper( ::osl::Mutex& aMutex );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~OConnectionPointContainerHelper();

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
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    //  XConnectionPointContainer

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getConnectionPointTypes()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const ::com::sun::star::uno::Type& aType
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise(
        const   ::com::sun::star::uno::Type&                              aType ,
        const   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise(
        const   ::com::sun::star::uno::Type&                              aType       ,
        const   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  xListener
    ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ::cppu::OMultiTypeInterfaceContainerHelper& impl_getMultiTypeContainer();

//  private variables

private:

    ::osl::Mutex&                                   m_aSharedMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aMultiTypeContainer;   // Container to hold listener

};  // class OConnectionPointContainerHelper

}   // namespace unocontrols

#endif  // #ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
