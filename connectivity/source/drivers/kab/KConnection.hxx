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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONNECTION_HXX

#include <map>
#include <connectivity/OSubComponent.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/compbase.hxx>
#include <shell/kde_headers.h>

namespace KABC
{
    class StdAddressBook;
    class AddressBook;
}
namespace com { namespace sun { namespace star { namespace sdbc {
    class XDriver;
} } } }

namespace connectivity
{
    namespace kab
    {

        typedef ::cppu::WeakComponentImplHelper<css::sdbc::XConnection,
                                                css::sdbc::XWarningsSupplier,
                                                css::lang::XServiceInfo
                                            > OMetaConnection_BASE;

        typedef OMetaConnection_BASE                KabConnection_BASE; // implements basics and text encoding
        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;

        class KabConnection : public comphelper::OBaseMutex,
                            public KabConnection_BASE,
                            public OSubComponent<KabConnection, KabConnection_BASE>
        {
            friend class OSubComponent<KabConnection, KabConnection_BASE>;

        protected:

            // Data attributes

            css::uno::WeakReference< css::sdbc::XDatabaseMetaData > m_xMetaData;

            OWeakRefArray                           m_aStatements;  // vector containing a list of all the Statement objects
                                                                    // for this Connection

            ::KABC::StdAddressBook*                 m_pAddressBook; // the address book
            css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
            css::uno::Reference< css::sdbcx::XTablesSupplier>
                                                    m_xCatalog;     // needed for the SQL interpreter

        public:
            void construct();

            KabConnection(
                css::uno::Reference<css::uno::XComponentContext> const &
                    componentContext,
                css::uno::Reference<css::sdbc::XDriver> const & driver);
            virtual ~KabConnection();

            void closeAllStatements () throw( css::sdbc::SQLException);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL commit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL rollback(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getCatalog(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // needed for the SQL interpreter
            css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL createCatalog();

            css::uno::Reference<css::uno::XComponentContext>
            getComponentContext() const
            { return m_xComponentContext; }

                   ::KABC::AddressBook* getAddressBook()    const;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
