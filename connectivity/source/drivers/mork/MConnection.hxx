/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCONNECTION_HXX

#include <TConnection.hxx>
#include "MColumnAlias.hxx"

#include <rtl/ref.hxx>

#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

// do we want here namespace too?
class MorkParser;

namespace connectivity::mork
    {
        class MorkDriver;
        class ErrorDescriptor;

        typedef connectivity::OMetaConnection OConnection_BASE; // implements basics and text encoding

        class OConnection final : public OConnection_BASE
        {
            // Data attributes

            rtl::Reference<MorkDriver> m_xDriver;              //  Pointer to the owning
                                                               //  driver object
            OColumnAlias    m_aColumnAlias;
            // Mork Parser (abook)
            std::unique_ptr<MorkParser> m_pBook;
            // Mork Parser (history)
            std::unique_ptr<MorkParser> m_pHistory;
            // Store Catalog
            css::uno::Reference< css::sdbcx::XTablesSupplier> m_xCatalog;

        public:
            /// @throws css::sdbc::SQLException
            void construct( const OUString& url);
            explicit OConnection(MorkDriver* const driver);
            virtual ~OConnection() override;

            const rtl::Reference<MorkDriver>& getDriver() const {return m_xDriver;};
            MorkParser* getMorkParser(std::string_view t) {return t == "CollectedAddressBook" ? m_pHistory.get() : m_pBook.get();};

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();
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
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings() override;

            const OColumnAlias & getColumnAlias() const   { return m_aColumnAlias; }

            static bool getForceLoadTables() {return true;}

            // Added to enable me to use SQLInterpreter which requires an
            // XNameAccess i/f to access tables.
            css::uno::Reference< css::sdbcx::XTablesSupplier > createCatalog();

            void throwSQLException( const ErrorDescriptor& _rError, const css::uno::Reference< css::uno::XInterface >& _rxContext );
            void throwSQLException( const char* pErrorResourceId, const css::uno::Reference< css::uno::XInterface >& _rxContext );
        };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
