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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NCONNECTION_HXX

#include "NDriver.hxx"
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/OSubComponent.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <connectivity/CommonTools.hxx>
#include <connectivity/warningscontainer.hxx>
#include "TConnection.hxx"
#include <cppuhelper/weakref.hxx>
#include <osl/module.h>
#include "EApi.h"

namespace connectivity
{
    namespace evoab
    {

        namespace SDBCAddress {
            typedef enum {
                Unknown     = 0,
                EVO_LOCAL       = 1,
                EVO_LDAP    = 2,
                EVO_GWISE   = 3
            } sdbc_address_type;
        }

        typedef connectivity::OMetaConnection               OConnection_BASE; // implements basics and text encoding

        class OEvoabConnection  :public OConnection_BASE
                                ,public connectivity::OSubComponent<OEvoabConnection, OConnection_BASE>
        {
            friend class connectivity::OSubComponent<OEvoabConnection, OConnection_BASE>;

        private:
            const OEvoabDriver&             m_rDriver;
            SDBCAddress::sdbc_address_type  m_eSDBCAddressType;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >
                                            m_xCatalog;
            OString                  m_aPassword;
            ::dbtools::WarningsContainer    m_aWarnings;

            virtual ~OEvoabConnection();

        public:
            explicit OEvoabConnection( OEvoabDriver& _rDriver );
            virtual void construct(const OUString& _rUrl,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo ) throw( ::com::sun::star::sdbc::SQLException);

            inline OString getPassword() { return m_aPassword; }
            inline void         setPassword( OString const & aStr ) { m_aPassword = aStr; }
            // own methods
            inline const OEvoabDriver& getDriver() const { return m_rDriver; }

            SDBCAddress::sdbc_address_type getSDBCAddressType() const { return m_eSDBCAddressType;}
            void setSDBCAddressType(SDBCAddress::sdbc_address_type _eSDBCAddressType) {m_eSDBCAddressType = _eSDBCAddressType;}

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
