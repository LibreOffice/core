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
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>


namespace connectivity
{

    // OPooledConnection -
    // allows to pool a real connection

    typedef ::cppu::WeakComponentImplHelper<    css::sdbc::XPooledConnection
                                               ,css::lang::XEventListener>    OPooledConnection_Base;

    class OPooledConnection : public ::cppu::BaseMutex
                             ,public OPooledConnection_Base
    {
        css::uno::Reference< css::sdbc::XConnection >             m_xRealConnection;  // the connection from driver
        css::uno::Reference< css::lang::XComponent >              m_xComponent;       // the connection which wraps the real connection
        css::uno::Reference< css::reflection::XProxyFactory >     m_xProxyFactory;
    public:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        OPooledConnection(const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                          const css::uno::Reference< css::reflection::XProxyFactory >& _rxProxyFactory);

        //XPooledConnection
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
