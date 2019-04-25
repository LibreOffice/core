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
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/uno3.hxx>
#include <connectivity/ConnectionWrapper.hxx>

namespace connectivity
{


    // OConnectionWeakWrapper - wraps all methods to the real connection from the driver
    // but when disposed it doesn't dispose the real connection

    typedef ::cppu::WeakComponentImplHelper< css::sdbc::XConnection > OConnectionWeakWrapper_BASE;

    class OConnectionWeakWrapper : public ::cppu::BaseMutex
                                 , public OConnectionWeakWrapper_BASE
                                 , public OConnectionWrapper
    {
    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;
        virtual ~OConnectionWeakWrapper() override;
    public:
        explicit OConnectionWeakWrapper(css::uno::Reference< css::uno::XAggregation >& _xConnection);

        // XServiceInfo
        DECLARE_SERVICE_INFO();
        DECLARE_XTYPEPROVIDER()
        DECLARE_XINTERFACE( )

        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) override;
        virtual OUString SAL_CALL nativeSQL( const OUString& sql ) override;
        virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) override;
        virtual sal_Bool SAL_CALL getAutoCommit(  ) override;
        virtual void SAL_CALL commit(  ) override;
        virtual void SAL_CALL rollback(  ) override;
        virtual sal_Bool SAL_CALL isClosed(  ) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) override;
        virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) override;
        virtual sal_Bool SAL_CALL isReadOnly(  ) override;
        virtual void SAL_CALL setCatalog( const OUString& catalog ) override;
        virtual OUString SAL_CALL getCatalog(  ) override;
        virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) override;
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) override;
        virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        // XCloseable
        virtual void SAL_CALL close(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
