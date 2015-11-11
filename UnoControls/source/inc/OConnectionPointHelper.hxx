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

class OConnectionPointHelper    :   public  css::lang::XConnectionPoint
                                ,   public  ::cppu::OWeakObject
{

public:

    OConnectionPointHelper( ::osl::Mutex&                       aMutex                      ,
                            OConnectionPointContainerHelper*    pContainerImplementation    ,
                            css::uno::Type                       aType                       );

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

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
        throw( css::uno::RuntimeException, std::exception ) override;

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

    virtual css::uno::Type SAL_CALL getConnectionType()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::lang::XConnectionPointContainer > SAL_CALL getConnectionPointContainer()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL advise(
        const css::uno::Reference< css::uno::XInterface >& xListener
    ) throw (
        css::lang::ListenerExistException,
        css::lang::InvalidListenerException ,
        css::uno::RuntimeException, std::exception
    ) override;

    virtual void SAL_CALL unadvise( const css::uno::Reference< css::uno::XInterface >& xListener )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > SAL_CALL getConnections()
        throw( css::uno::RuntimeException, std::exception ) override;

private:

    bool impl_LockContainer();

    void impl_UnlockContainer();

private:

    ::osl::Mutex&                                                     m_aSharedMutex;
    css::uno::WeakReference< css::lang::XConnectionPointContainer >   m_oContainerWeakReference;   // Reference to container-class!. Don't use Reference<...>
                                                                                            // It is a ring-reference => and must be a wekreference!
    OConnectionPointContainerHelper*                                  m_pContainerImplementation;
    css::uno::Type                                                    m_aInterfaceType;
    css::uno::Reference< css::uno::XInterface >                       m_xLock;

};  // class OConnectionPointHelper

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_SOURCE_INC_OCONNECTIONPOINTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
