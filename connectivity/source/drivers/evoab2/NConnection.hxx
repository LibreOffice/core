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

#include "NDriver.hxx"
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <connectivity/CommonTools.hxx>
#include <connectivity/warningscontainer.hxx>
#include <TConnection.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/module.h>
#include "EApi.h"

namespace connectivity::evoab {

        namespace SDBCAddress {
            typedef enum {
                Unknown     = 0,
                EVO_LOCAL       = 1,
                EVO_LDAP    = 2,
                EVO_GWISE   = 3
            } sdbc_address_type;
        }

        typedef connectivity::OMetaConnection               OConnection_BASE; // implements basics and text encoding

        class OEvoabConnection final :public OConnection_BASE
        {
        private:
            const OEvoabDriver&             m_rDriver;
            SDBCAddress::sdbc_address_type  m_eSDBCAddressType;
            css::uno::Reference< css::sdbcx::XTablesSupplier >
                                            m_xCatalog;
            OString                         m_aPassword;
            ::dbtools::WarningsContainer    m_aWarnings;

            virtual ~OEvoabConnection() override;

        public:
            explicit OEvoabConnection( OEvoabDriver const & _rDriver );
            /// @throws css::sdbc::SQLException
            void construct(const OUString& _rUrl,const css::uno::Sequence< css::beans::PropertyValue >& _rInfo );

            OString const & getPassword() const { return m_aPassword; }
            void         setPassword( OString const & aStr ) { m_aPassword = aStr; }
            // own methods
            const OEvoabDriver& getDriver() const { return m_rDriver; }

            SDBCAddress::sdbc_address_type getSDBCAddressType() const { return m_eSDBCAddressType;}
            void setSDBCAddressType(SDBCAddress::sdbc_address_type _eSDBCAddressType) {m_eSDBCAddressType = _eSDBCAddressType;}

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            css::uno::Reference< css::sdbcx::XTablesSupplier > createCatalog();
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
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
