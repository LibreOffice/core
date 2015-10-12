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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_SHAREDCONNECTION_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_SHAREDCONNECTION_HXX

#include "connectivity/ConnectionWrapper.hxx"
#include <cppuhelper/component.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XCommandPreparation.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <comphelper/sequence.hxx>

namespace dbaccess
{
    // OSharedConnection: This class implements a simple forwarding of connection calls.
    // All methods will be forwarded with exception of the set methods, which are not allowed
    // to be called on shared connections. Instances of this class will be created when the
    // datasource is asked for not isolated connection.
    typedef ::cppu::WeakComponentImplHelper< css::sdbc::XConnection
                                           > OSharedConnection_BASE;
    typedef ::connectivity::OConnectionWrapper  OSharedConnection_BASE2;

    class OSharedConnection :   public ::comphelper::OBaseMutex
                              , public OSharedConnection_BASE
                              , public OSharedConnection_BASE2
    {
    protected:
        virtual void SAL_CALL disposing() override;
        virtual ~OSharedConnection();
    public:
        explicit OSharedConnection(css::uno::Reference< css::uno::XAggregation >& _rxProxyConnection);

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL acquire() throw() override { OSharedConnection_BASE::acquire(); }
        virtual void SAL_CALL release() throw() override { OSharedConnection_BASE::release(); }
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override
        {
            return ::comphelper::concatSequences(
                OSharedConnection_BASE::getTypes(),
                OSharedConnection_BASE2::getTypes()
            );
        }

        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override
        {
            css::uno::Any aReturn = OSharedConnection_BASE::queryInterface(_rType);
            if ( !aReturn.hasValue() )
                aReturn = OSharedConnection_BASE2::queryInterface(_rType);
            return aReturn;
        }

        // XCloseable
        virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                ::connectivity::checkDisposed(rBHelper.bDisposed);
            }
            dispose();
        }

        // XConnection
        virtual void SAL_CALL setAutoCommit( sal_Bool /*autoCommit*/ ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            throw css::sdbc::SQLException("This call is not allowed when sharing connections.",*this,"S10000",0,css::uno::Any());
        }
        virtual void SAL_CALL setReadOnly( sal_Bool /*readOnly*/ ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            throw css::sdbc::SQLException("This call is not allowed when sharing connections.",*this,"S10000",0,css::uno::Any());
        }
        virtual void SAL_CALL setCatalog( const OUString& /*catalog*/ ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            throw css::sdbc::SQLException("This call is not allowed when sharing connections.",*this,"S10000",0,css::uno::Any());
        }
        virtual void SAL_CALL setTransactionIsolation( sal_Int32 /*level*/ ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            throw css::sdbc::SQLException("This call is not allowed when sharing connections.",*this,"S10000",0,css::uno::Any());
        }
        virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& /*typeMap*/ ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override
        {
            throw css::sdbc::SQLException("This call is not allowed when sharing connections.",*this,"S10000",0,css::uno::Any());
        }
        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL commit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL rollback(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isClosed(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCatalog(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    };

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_SHAREDCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
