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

#include <connectivity/ConnectionWrapper.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/sequence.hxx>

namespace dbaccess
{
    // OSharedConnection: This class implements a simple forwarding of connection calls.
    // All methods will be forwarded with exception of the set methods, which are not allowed
    // to be called on shared connections. Instances of this class will be created when the
    // datasource is asked for not isolated connection.
    typedef ::cppu::WeakComponentImplHelper< css::sdbc::XConnection
                                           > OSharedConnection_BASE;

    class OSharedConnection :   public ::cppu::BaseMutex
                              , public OSharedConnection_BASE
                              , public ::connectivity::OConnectionWrapper
    {
    protected:
        virtual void SAL_CALL disposing() override;
        virtual ~OSharedConnection() override;
    public:
        explicit OSharedConnection(css::uno::Reference< css::uno::XAggregation >& _rxProxyConnection);

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        virtual void SAL_CALL acquire() noexcept override { OSharedConnection_BASE::acquire(); }
        virtual void SAL_CALL release() noexcept override { OSharedConnection_BASE::release(); }
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override
        {
            return ::comphelper::concatSequences(
                OSharedConnection_BASE::getTypes(),
                ::connectivity::OConnectionWrapper::getTypes()
            );
        }

        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) override
        {
            css::uno::Any aReturn = OSharedConnection_BASE::queryInterface(_rType);
            if ( !aReturn.hasValue() )
                aReturn = ::connectivity::OConnectionWrapper::queryInterface(_rType);
            return aReturn;
        }

        // XCloseable
        virtual void SAL_CALL close(  ) override
        {
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                ::connectivity::checkDisposed(rBHelper.bDisposed);
            }
            dispose();
        }

        // XConnection
        virtual void SAL_CALL setAutoCommit( sal_Bool /*autoCommit*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,css::uno::Any());
        }
        virtual void SAL_CALL setReadOnly( sal_Bool /*readOnly*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,css::uno::Any());
        }
        virtual void SAL_CALL setCatalog( const OUString& /*catalog*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,css::uno::Any());
        }
        virtual void SAL_CALL setTransactionIsolation( sal_Int32 /*level*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,css::uno::Any());
        }
        virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& /*typeMap*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,css::uno::Any());
        }
        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) override;
        virtual OUString SAL_CALL nativeSQL( const OUString& sql ) override;
        virtual sal_Bool SAL_CALL getAutoCommit(  ) override;
        virtual void SAL_CALL commit(  ) override;
        virtual void SAL_CALL rollback(  ) override;
        virtual sal_Bool SAL_CALL isClosed(  ) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) override;
        virtual sal_Bool SAL_CALL isReadOnly(  ) override;
        virtual OUString SAL_CALL getCatalog(  ) override;
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) override;
    };

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
