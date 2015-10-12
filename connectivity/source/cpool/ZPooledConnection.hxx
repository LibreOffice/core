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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLEDCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLEDCONNECTION_HXX

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/reflection/XProxyFactory.hpp>


namespace connectivity
{

    // OPooledConnection -
    // allows to pool a real connection

    typedef ::cppu::WeakComponentImplHelper<    ::com::sun::star::sdbc::XPooledConnection
                                               ,::com::sun::star::lang::XEventListener>    OPooledConnection_Base;

    class OPooledConnection : public ::comphelper::OBaseMutex
                             ,public OPooledConnection_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xRealConnection;  // the connection fom driver
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >              m_xComponent;       // the connection which wraps the real connection
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >     m_xProxyFactory;
    public:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        OPooledConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory);

        //XPooledConnection
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLEDCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
