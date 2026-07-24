/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

namespace dbaccess
{
    // OSharedConnection: This class implements a simple forwarding of connection calls.
    // All methods will be forwarded with exception of the set methods, which are not allowed
    // to be called on shared connections. Instances of this class will be created when the
    // datasource is asked for not isolated connection.
    typedef ::cppu::ImplInheritanceHelper<connectivity::OConnectionWrapper,
                                          css::sdbc::XConnection
                                           > OSharedConnection_BASE;

    class OSharedConnection : public OSharedConnection_BASE
    {
    protected:
        virtual ~OSharedConnection() override;
    public:
        explicit OSharedConnection(css::uno::Reference< css::uno::XAggregation >& _rxProxyConnection);

        // XCloseable
        virtual void close(  ) override
        {
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                ::connectivity::checkDisposed(rBHelper.bDisposed);
            }
            dispose();
        }

        // XConnection
        virtual void setAutoCommit( bool /*autoCommit*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,cpo::uno::Any());
        }
        virtual void setReadOnly( bool /*readOnly*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,cpo::uno::Any());
        }
        virtual void setCatalog( const OUString& /*catalog*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,cpo::uno::Any());
        }
        virtual void setTransactionIsolation( sal_Int32 /*level*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,cpo::uno::Any());
        }
        virtual void setTypeMap( const css::uno::Reference< css::container::XNameAccess >& /*typeMap*/ ) override
        {
            throw css::sdbc::SQLException(u"This call is not allowed when sharing connections."_ustr,*this,u"S10000"_ustr,0,cpo::uno::Any());
        }
        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareCall( const OUString& sql ) override;
        virtual OUString nativeSQL( const OUString& sql ) override;
        virtual bool getAutoCommit(  ) override;
        virtual void commit(  ) override;
        virtual void rollback(  ) override;
        virtual bool isClosed(  ) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > getMetaData(  ) override;
        virtual bool isReadOnly(  ) override;
        virtual OUString getCatalog(  ) override;
        virtual sal_Int32 getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > getTypeMap(  ) override;
    };

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
