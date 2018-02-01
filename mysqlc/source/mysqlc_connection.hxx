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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_CONNECTION_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_CONNECTION_HXX

#include <memory>
#include "mysqlc_subcomponent.hxx"
#include "mysqlc_types.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>

#include <cppconn/driver.h>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/string.hxx>
#include <rtl/ref.hxx>

#include <map>


namespace sql
{
    class SQLException;
}

namespace connectivity
{
    class OMetaConnection;
    class ODatabaseMetaData;

    namespace mysqlc
    {
        using ::com::sun::star::sdbc::SQLWarning;
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::RuntimeException;
        typedef css::uno::Reference< css::container::XNameAccess > my_XNameAccessRef;

        typedef ::cppu::WeakComponentImplHelper3<   css::sdbc::XConnection,
                                                    css::sdbc::XWarningsSupplier,
                                                    css::lang::XServiceInfo
                                                > OMetaConnection_BASE;
        struct ConnectionSettings
        {
            rtl_TextEncoding encoding;
            std::unique_ptr<sql::Connection> cppConnection;
            rtl::OUString schema;
            rtl::OUString connectionURL;
            bool readOnly;
        };

        class MysqlCDriver;

        typedef OMetaConnection_BASE OConnection_BASE;

        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;

        class OConnection final : public OBase_Mutex,
                            public OConnection_BASE
        {
            ConnectionSettings  m_settings;
            css::uno::Reference< css::container::XNameAccess > m_typeMap;
            css::uno::Reference< css::util::XStringSubstitution > m_xParameterSubstitution;

            // Data attributes

            css::uno::WeakReference< css::sdbc::XDatabaseMetaData > m_xMetaData;

            OWeakRefArray   m_aStatements;  // vector containing a list
                                            // of all the Statement objects
                                            // for this Connection

            rtl::Reference<MysqlCDriver> m_xDriver; // Pointer to the owning driver object
            sql::Driver*    cppDriver;

        public:
            /// @throws SQLException
            /// @throws RuntimeException
            sal_Int32 getMysqlVersion();

            /// @throws SQLException
            void construct(const rtl::OUString& url,const css::uno::Sequence< css::beans::PropertyValue >& info);

            OConnection(MysqlCDriver& _rDriver, sql::Driver * cppDriver);
            virtual ~OConnection();

            rtl_TextEncoding getConnectionEncoding() const { return m_settings.encoding; }


            // OComponentHelper
            virtual void SAL_CALL disposing() SAL_OVERRIDE;

            // XServiceInfo
            virtual rtl::OUString SAL_CALL getImplementationName() SAL_OVERRIDE;

            virtual sal_Bool SAL_CALL supportsService(
                rtl::OUString const & ServiceName)
                SAL_OVERRIDE;

            virtual css::uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames()
                SAL_OVERRIDE;

            // XConnection
            css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement() SAL_OVERRIDE;

            css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement(const rtl::OUString& sql) SAL_OVERRIDE;

            css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall(const rtl::OUString& sql) SAL_OVERRIDE;

            rtl::OUString SAL_CALL nativeSQL(const rtl::OUString& sql) SAL_OVERRIDE;

            void SAL_CALL setAutoCommit(sal_Bool autoCommit) SAL_OVERRIDE;

            sal_Bool SAL_CALL getAutoCommit() SAL_OVERRIDE;

            void SAL_CALL commit() SAL_OVERRIDE;

            void SAL_CALL rollback() SAL_OVERRIDE;

            sal_Bool SAL_CALL isClosed() SAL_OVERRIDE;

            css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData() SAL_OVERRIDE;

            void SAL_CALL setReadOnly(sal_Bool readOnly) SAL_OVERRIDE;

            sal_Bool SAL_CALL isReadOnly() SAL_OVERRIDE;

            void SAL_CALL setCatalog(const rtl::OUString& catalog) SAL_OVERRIDE;

            rtl::OUString SAL_CALL getCatalog() SAL_OVERRIDE;

            void SAL_CALL setTransactionIsolation(sal_Int32 level) SAL_OVERRIDE;

            sal_Int32 SAL_CALL getTransactionIsolation() SAL_OVERRIDE;

            my_XNameAccessRef SAL_CALL getTypeMap() SAL_OVERRIDE;

            void SAL_CALL setTypeMap(const my_XNameAccessRef& typeMap) SAL_OVERRIDE;
            // XCloseable
            void SAL_CALL close() SAL_OVERRIDE;
            // XWarningsSupplier
            css::uno::Any SAL_CALL getWarnings() SAL_OVERRIDE;
            void SAL_CALL clearWarnings() SAL_OVERRIDE;

            // TODO: Not used
            //sal_Int32 sdbcColumnType(rtl::OUString typeName);
            const ConnectionSettings& getConnectionSettings() const { return m_settings; }
            rtl::OUString transFormPreparedStatement(const rtl::OUString& _sSQL);

            const MysqlCDriver&  getDriver()         const { return *m_xDriver.get();}

        }; /* OConnection */
        // TODO: Not used.
        //inline rtl::OUString getPattern(rtl::OUString p) { return (p.getLength()) ? p : ASC2OU("%"); }
    } /* mysqlc */
} /* connectivity */
#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_CONNECTION_HXX

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
