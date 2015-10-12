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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCONNECTION_HXX

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
#include "TConnection.hxx"

namespace connectivity
{
    namespace macab
    {

        typedef ::cppu::WeakComponentImplHelper<::com::sun::star::sdbc::XConnection,
                                                ::com::sun::star::sdbc::XWarningsSupplier,
                                                ::com::sun::star::lang::XServiceInfo
                                            > OMetaConnection_BASE;

        class MacabDriver;
        class MacabAddressBook;

        //typedef OMetaConnection_BASE              MacabConnection_BASE; // implements basics and text encoding
        typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;

        typedef connectivity::OMetaConnection MacabConnection_BASE;

        class MacabConnection : public MacabConnection_BASE,
                            public OSubComponent<MacabConnection, MacabConnection_BASE>
        {
            friend class OSubComponent<MacabConnection, MacabConnection_BASE>;

        protected:

            // Data attributes

            MacabAddressBook*                   m_pAddressBook; // the address book
            MacabDriver*                                m_pDriver;      // pointer to the owning driver object
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier>
                                                    m_xCatalog;     // needed for the SQL interpreter

        public:
            virtual void construct( const OUString& url,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info) throw(::com::sun::star::sdbc::SQLException);

            explicit MacabConnection(MacabDriver* _pDriver);
            virtual ~MacabConnection();

            void closeAllStatements () throw( ::com::sun::star::sdbc::SQLException);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;

            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;

            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;

            // needed for the SQL interpreter
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL createCatalog();

            // accessors
            inline MacabDriver*         getDriver()         const { return m_pDriver;}
                   MacabAddressBook* getAddressBook()   const;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
