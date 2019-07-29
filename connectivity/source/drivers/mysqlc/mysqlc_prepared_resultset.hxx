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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_RESULTSET_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_RESULTSET_HXX

#include "mysqlc_preparedstatement.hxx"
#include "mysqlc_statement.hxx"
#include "mysqlc_subcomponent.hxx"
#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <connectivity/FValue.hxx>

#include <cppuhelper/compbase12.hxx>

namespace connectivity
{
namespace mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;

typedef ::cppu::WeakComponentImplHelper12<
    css::sdbc::XResultSet, css::sdbc::XRow, css::sdbc::XResultSetMetaDataSupplier,
    css::util::XCancellable, css::sdbc::XWarningsSupplier, css::sdbc::XResultSetUpdate,
    css::sdbc::XRowUpdate, css::sdbcx::XRowLocate, css::sdbcx::XDeleteRows, css::sdbc::XCloseable,
    css::sdbc::XColumnLocate, css::lang::XServiceInfo>
    OPreparedResultSet_BASE;

class OPreparedResultSet final : public OBase_Mutex,
                                 public OPreparedResultSet_BASE,
                                 public ::cppu::OPropertySetHelper,
                                 public OPropertyArrayUsageHelper<OPreparedResultSet>
{
    OConnection& m_rConnection;
    css::uno::WeakReferenceHelper m_aStatement;
    css::uno::Reference<css::sdbc::XResultSetMetaData> m_xMetaData;

    // non-owning pointers
    MYSQL_RES* m_pResult = nullptr;
    MYSQL_STMT* m_pStmt = nullptr;
    MYSQL_FIELD* m_aFields = nullptr;

    rtl_TextEncoding m_encoding;
    sal_Int32 m_nCurrentRow = 0;
    sal_Int32 m_nColumnCount;
    sal_Int32 m_nRowCount;

    // Use c style arrays, because we have to work with pointers
    // on these.
    std::unique_ptr<MYSQL_BIND[]> m_aData;
    std::unique_ptr<BindMetaData[]> m_aMetaData;

    bool m_bWasNull = false;

    // OPropertyArrayUsageHelper
    ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;
    // OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

    sal_Bool SAL_CALL convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue,
                                               sal_Int32 nHandle, const Any& rValue) SAL_OVERRIDE;

    void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                   const Any& rValue) SAL_OVERRIDE;

    void SAL_CALL getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;

    template <typename T> T safelyRetrieveValue(sal_Int32 nColumnIndex);
    template <typename T> T retrieveValue(sal_Int32 nColumnIndex);
    connectivity::ORowSetValue getRowSetValue(sal_Int32 nColumnIndex);

    bool fetchResult();

    // you can't delete objects of this type
    virtual ~OPreparedResultSet() override = default;

public:
    virtual rtl::OUString SAL_CALL getImplementationName() SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const& ServiceName) SAL_OVERRIDE;

    virtual css::uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames() SAL_OVERRIDE;

    OPreparedResultSet(OConnection& rConn, OPreparedStatement* pStmt, MYSQL_STMT* pMyStmt);

    // ::cppu::OComponentHelper
    void SAL_CALL disposing() SAL_OVERRIDE;

    // XInterface
    Any SAL_CALL queryInterface(const css::uno::Type& rType) SAL_OVERRIDE;

    void SAL_CALL acquire() throw() SAL_OVERRIDE;
    void SAL_CALL release() throw() SAL_OVERRIDE;

    //XTypeProvider
    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() SAL_OVERRIDE;

    // XPropertySet
    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() SAL_OVERRIDE;

    // XResultSet
    sal_Bool SAL_CALL next() SAL_OVERRIDE;
    sal_Bool SAL_CALL isBeforeFirst() SAL_OVERRIDE;
    sal_Bool SAL_CALL isAfterLast() SAL_OVERRIDE;
    sal_Bool SAL_CALL isFirst() SAL_OVERRIDE;
    sal_Bool SAL_CALL isLast() SAL_OVERRIDE;

    void SAL_CALL beforeFirst() SAL_OVERRIDE;
    void SAL_CALL afterLast() SAL_OVERRIDE;

    sal_Bool SAL_CALL first() SAL_OVERRIDE;
    sal_Bool SAL_CALL last() SAL_OVERRIDE;

    sal_Int32 SAL_CALL getRow() SAL_OVERRIDE;

    sal_Bool SAL_CALL absolute(sal_Int32 row) SAL_OVERRIDE;
    sal_Bool SAL_CALL relative(sal_Int32 rows) SAL_OVERRIDE;
    sal_Bool SAL_CALL previous() SAL_OVERRIDE;

    void SAL_CALL refreshRow() SAL_OVERRIDE;

    sal_Bool SAL_CALL rowUpdated() SAL_OVERRIDE;
    sal_Bool SAL_CALL rowInserted() SAL_OVERRIDE;
    sal_Bool SAL_CALL rowDeleted() SAL_OVERRIDE;

    css::uno::Reference<css::uno::XInterface> SAL_CALL getStatement() SAL_OVERRIDE;
    // XRow
    sal_Bool SAL_CALL wasNull() SAL_OVERRIDE;

    rtl::OUString SAL_CALL getString(sal_Int32 column) SAL_OVERRIDE;

    sal_Bool SAL_CALL getBoolean(sal_Int32 column) SAL_OVERRIDE;
    sal_Int8 SAL_CALL getByte(sal_Int32 column) SAL_OVERRIDE;
    sal_Int16 SAL_CALL getShort(sal_Int32 column) SAL_OVERRIDE;
    sal_Int32 SAL_CALL getInt(sal_Int32 column) SAL_OVERRIDE;
    sal_Int64 SAL_CALL getLong(sal_Int32 column) SAL_OVERRIDE;

    float SAL_CALL getFloat(sal_Int32 column) SAL_OVERRIDE;
    double SAL_CALL getDouble(sal_Int32 column) SAL_OVERRIDE;

    css::uno::Sequence<sal_Int8> SAL_CALL getBytes(sal_Int32 column) SAL_OVERRIDE;
    css::util::Date SAL_CALL getDate(sal_Int32 column) SAL_OVERRIDE;
    css::util::Time SAL_CALL getTime(sal_Int32 column) SAL_OVERRIDE;
    css::util::DateTime SAL_CALL getTimestamp(sal_Int32 column) SAL_OVERRIDE;

    css::uno::Reference<css::io::XInputStream>
        SAL_CALL getBinaryStream(sal_Int32 column) SAL_OVERRIDE;
    css::uno::Reference<css::io::XInputStream>
        SAL_CALL getCharacterStream(sal_Int32 column) SAL_OVERRIDE;

    Any SAL_CALL getObject(sal_Int32 column, const my_XNameAccessRef& typeMap) SAL_OVERRIDE;

    css::uno::Reference<css::sdbc::XRef> SAL_CALL getRef(sal_Int32 column) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XBlob> SAL_CALL getBlob(sal_Int32 column) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XClob> SAL_CALL getClob(sal_Int32 column) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XArray> SAL_CALL getArray(sal_Int32 column) SAL_OVERRIDE;

    // XResultSetMetaDataSupplier
    css::uno::Reference<css::sdbc::XResultSetMetaData> SAL_CALL getMetaData() SAL_OVERRIDE;

    // XCancellable
    void SAL_CALL cancel() SAL_OVERRIDE;

    // XCloseable
    void SAL_CALL close() SAL_OVERRIDE;

    // XWarningsSupplier
    Any SAL_CALL getWarnings() SAL_OVERRIDE;

    void SAL_CALL clearWarnings() SAL_OVERRIDE;

    // XResultSetUpdate
    void SAL_CALL insertRow() SAL_OVERRIDE;
    void SAL_CALL updateRow() SAL_OVERRIDE;
    void SAL_CALL deleteRow() SAL_OVERRIDE;
    void SAL_CALL cancelRowUpdates() SAL_OVERRIDE;
    void SAL_CALL moveToInsertRow() SAL_OVERRIDE;
    void SAL_CALL moveToCurrentRow() SAL_OVERRIDE;

    // XRowUpdate
    void SAL_CALL updateNull(sal_Int32 column) SAL_OVERRIDE;
    void SAL_CALL updateBoolean(sal_Int32 column, sal_Bool x) SAL_OVERRIDE;
    void SAL_CALL updateByte(sal_Int32 column, sal_Int8 x) SAL_OVERRIDE;
    void SAL_CALL updateShort(sal_Int32 column, sal_Int16 x) SAL_OVERRIDE;
    void SAL_CALL updateInt(sal_Int32 column, sal_Int32 x) SAL_OVERRIDE;
    void SAL_CALL updateLong(sal_Int32 column, sal_Int64 x) SAL_OVERRIDE;
    void SAL_CALL updateFloat(sal_Int32 column, float x) SAL_OVERRIDE;
    void SAL_CALL updateDouble(sal_Int32 column, double x) SAL_OVERRIDE;
    void SAL_CALL updateString(sal_Int32 column, const rtl::OUString& x) SAL_OVERRIDE;
    void SAL_CALL updateBytes(sal_Int32 column, const css::uno::Sequence<sal_Int8>& x) SAL_OVERRIDE;
    void SAL_CALL updateDate(sal_Int32 column, const css::util::Date& x) SAL_OVERRIDE;
    void SAL_CALL updateTime(sal_Int32 column, const css::util::Time& x) SAL_OVERRIDE;
    void SAL_CALL updateTimestamp(sal_Int32 column, const css::util::DateTime& x) SAL_OVERRIDE;
    void SAL_CALL updateBinaryStream(sal_Int32 column,
                                     const css::uno::Reference<css::io::XInputStream>& x,
                                     sal_Int32 length) SAL_OVERRIDE;
    void SAL_CALL updateCharacterStream(sal_Int32 column,
                                        const css::uno::Reference<css::io::XInputStream>& x,
                                        sal_Int32 length) SAL_OVERRIDE;
    void SAL_CALL updateObject(sal_Int32 column, const Any& x) SAL_OVERRIDE;
    void SAL_CALL updateNumericObject(sal_Int32 column, const Any& x, sal_Int32 scale) SAL_OVERRIDE;

    // XColumnLocate
    sal_Int32 SAL_CALL findColumn(const rtl::OUString& columnName) SAL_OVERRIDE;

    // XRowLocate
    Any SAL_CALL getBookmark() SAL_OVERRIDE;

    sal_Bool SAL_CALL moveToBookmark(const Any& bookmark) SAL_OVERRIDE;
    sal_Bool SAL_CALL moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows) SAL_OVERRIDE;
    sal_Int32 SAL_CALL compareBookmarks(const Any& first, const Any& second) SAL_OVERRIDE;
    sal_Bool SAL_CALL hasOrderedBookmarks() SAL_OVERRIDE;
    sal_Int32 SAL_CALL hashBookmark(const Any& bookmark) SAL_OVERRIDE;

    // XDeleteRows
    css::uno::Sequence<sal_Int32>
        SAL_CALL deleteRows(const css::uno::Sequence<Any>& rows) SAL_OVERRIDE;

    /// @throws SQLException
    /// @throws RuntimeException
    void checkColumnIndex(sal_Int32 index);

private:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};
} /* mysqlc */
} /* connectivity */
#endif // CONNECTIVITY_SRESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
