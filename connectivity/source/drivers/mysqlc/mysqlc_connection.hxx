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

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/string.hxx>
#include <rtl/ref.hxx>

#include <mysql.h>

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
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::sdbc::SQLWarning;
using ::com::sun::star::uno::RuntimeException;

typedef ::cppu::WeakComponentImplHelper3<css::sdbc::XConnection, css::sdbc::XWarningsSupplier,
                                         css::lang::XServiceInfo>
    OMetaConnection_BASE;

struct ConnectionSettings
{
    rtl_TextEncoding encoding;
    OUString connectionURL;
    bool readOnly;
    ConnectionSettings()
        : encoding(RTL_TEXTENCODING_DONTKNOW)
        , readOnly(false)
    {
    }
};

class MysqlCDriver;

typedef OMetaConnection_BASE OConnection_BASE;

typedef std::vector<css::uno::WeakReferenceHelper> OWeakRefArray;

class OConnection final : public OBase_Mutex, public OConnection_BASE
{
private:
    MYSQL m_mysql;
    ConnectionSettings m_settings;
    css::uno::Reference<css::container::XNameAccess> m_typeMap;
    css::uno::Reference<css::util::XStringSubstitution> m_xParameterSubstitution;

    // Data attributes

    css::uno::WeakReference<css::sdbc::XDatabaseMetaData> m_xMetaData;

    OWeakRefArray m_aStatements; // vector containing a list
        // of all the Statement objects
        // for this Connection

    rtl::Reference<MysqlCDriver> m_xDriver; // Pointer to the owning driver object
public:
    MYSQL* getMysqlConnection() { return &m_mysql; }

    /// @throws SQLException
    /// @throws RuntimeException
    sal_Int32 getMysqlVersion();

    /// @throws SQLException
    void construct(const OUString& url, const css::uno::Sequence<css::beans::PropertyValue>& info);

    OConnection(MysqlCDriver& _rDriver);
    virtual ~OConnection() override;

    rtl_TextEncoding getConnectionEncoding() const { return m_settings.encoding; }

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XConnection
    css::uno::Reference<css::sdbc::XStatement> SAL_CALL createStatement() override;

    css::uno::Reference<css::sdbc::XPreparedStatement>
        SAL_CALL prepareStatement(const OUString& sql) override;

    css::uno::Reference<css::sdbc::XPreparedStatement>
        SAL_CALL prepareCall(const OUString& sql) override;

    OUString SAL_CALL nativeSQL(const OUString& sql) override;

    void SAL_CALL setAutoCommit(sal_Bool autoCommit) override;

    sal_Bool SAL_CALL getAutoCommit() override;

    void SAL_CALL commit() override;

    void SAL_CALL rollback() override;

    sal_Bool SAL_CALL isClosed() override;

    css::uno::Reference<css::sdbc::XDatabaseMetaData> SAL_CALL getMetaData() override;

    void SAL_CALL setReadOnly(sal_Bool readOnly) override;

    sal_Bool SAL_CALL isReadOnly() override;

    void SAL_CALL setCatalog(const OUString& catalog) override;

    OUString SAL_CALL getCatalog() override;

    void SAL_CALL setTransactionIsolation(sal_Int32 level) override;

    sal_Int32 SAL_CALL getTransactionIsolation() override;

    css::uno::Reference<css::container::XNameAccess> SAL_CALL getTypeMap() override;

    void SAL_CALL
    setTypeMap(const css::uno::Reference<css::container::XNameAccess>& typeMap) override;
    // XCloseable
    void SAL_CALL close() override;
    // XWarningsSupplier
    css::uno::Any SAL_CALL getWarnings() override;
    void SAL_CALL clearWarnings() override;

    // TODO: Not used
    //sal_Int32 sdbcColumnType(OUString typeName);
    const ConnectionSettings& getConnectionSettings() const { return m_settings; }
    OUString transFormPreparedStatement(const OUString& _sSQL);

    const MysqlCDriver& getDriver() const { return *m_xDriver.get(); }

}; /* OConnection */
// TODO: Not used.
//inline OUString getPattern(OUString p) { return (p.getLength()) ? p : ASC2OU("%"); }
} /* mysqlc */
} /* connectivity */
#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_CONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
