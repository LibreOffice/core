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

#pragma once

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

namespace com::sun::star::lang { class XConnectionPoint; }

namespace unocontrols {

class OConnectionPointContainerHelper final :   public  css::lang::XConnectionPointContainer
                                        ,   public  ::cppu::OWeakObject
{
public:
    OConnectionPointContainerHelper( ::osl::Mutex& aMutex );

    virtual ~OConnectionPointContainerHelper() override;

    //  XInterface

    /**
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() noexcept override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() noexcept override;

    //  XConnectionPointContainer

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getConnectionPointTypes() override;

    virtual css::uno::Reference< css::lang::XConnectionPoint > SAL_CALL queryConnectionPoint(
        const css::uno::Type& aType
    ) override;

    virtual void SAL_CALL advise(
        const   css::uno::Type&                              aType ,
        const   css::uno::Reference< css::uno::XInterface >&  xListener
    ) override;

    virtual void SAL_CALL unadvise(
        const   css::uno::Type&                              aType       ,
        const   css::uno::Reference< css::uno::XInterface >&  xListener
    ) override;

    //  public but impl method!
    //  Is necessary to get container member at OConnectionPoint-instance.
    // Impl methods are not threadsafe!
    // "Parent" function must do this.
    ::cppu::OMultiTypeInterfaceContainerHelper& impl_getMultiTypeContainer() { return m_aMultiTypeContainer; }

private:
    ::osl::Mutex&                                   m_aSharedMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aMultiTypeContainer;   // Container to hold listener
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
