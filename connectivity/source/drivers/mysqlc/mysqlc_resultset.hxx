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

#include <cppuhelper/compbase12.hxx>

namespace connectivity
{
namespace mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;

/*
        **  OResultSet
        */
typedef ::cppu::WeakComponentImplHelper12<
    css::sdbc::XResultSet, css::sdbc::XRow, css::sdbc::XResultSetMetaDataSupplier,
    css::util::XCancellable, css::sdbc::XWarningsSupplier, css::sdbc::XResultSetUpdate,
    css::sdbc::XRowUpdate, css::sdbcx::XRowLocate, css::sdbcx::XDeleteRows, css::sdbc::XCloseable,
    css::sdbc::XColumnLocate, css::lang::XServiceInfo>
    OResultSet_BASE;

class OResultSet final : public OBase_Mutex,
                         public OResultSet_BASE,
                         public ::cppu::OPropertySetHelper,
                         public OPropertyArrayUsageHelper<OResultSet>
{
    using DataFields = std::vector<OString>;
    std::vector<DataFields> m_aRows;
    std::vector<OUString> m_aFields;
    MYSQL* m_pMysql = nullptr;
    css::uno::WeakReferenceHelper m_aStatement;
    css::uno::Reference<css::sdbc::XResultSetMetaData> m_xMetaData;
    MYSQL_RES* m_pResult;
    rtl_TextEncoding m_encoding;
    bool m_bWasNull = false; // did the last getXXX result null?
    bool m_bResultFetched = false;

    sal_Int32 getDataLength(sal_Int32 column)
    {
        return m_aRows[m_nRowPosition][column - 1].getLength();
    }
    bool checkNull(sal_Int32 column);

    /**
     * Position of cursor indexed from 0
     */
    sal_Int32 m_nRowPosition = -1;
    sal_Int32 m_nRowCount = 0;

    // OPropertyArrayUsageHelper
    ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
    // OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    sal_Bool SAL_CALL convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue,
                                               sal_Int32 nHandle, const Any& rValue) override;

    void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) override;

    void SAL_CALL getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const override;

    virtual ~OResultSet() override = default;

    /**
     * Ensures that the results of the query has already been fetched.
     */
    void ensureResultFetched();

    /**
     * Ensures that meta data of the corresponding result set has been already
     * queried. It should be called before freeing the result set, unless the
     * information is lost.
     */
    void ensureFieldInfoFetched();

    /**
     * Check the following things:
     * - cursor is out of range. Throws exception if true.
     * - column index is out of range. Throws exception if true.
     * - result set is fetched. If no, then it fetches the result.
     */
    void checkBordersAndEnsureFetched(sal_Int32 index);

    /**
     * Fetches all the data from the MYSQL_RES object related to the class. It
     * frees the MYSQL_RES object afterwards, so it cannot be used anymore.
     */
    void fetchResult();

public:
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    OResultSet(OConnection& rConn, OCommonStatement* pStmt, MYSQL_RES* pResult,
               rtl_TextEncoding _encoding);

    // ::cppu::OComponentHelper
    void SAL_CALL disposing() override;

    // XInterface
    Any SAL_CALL queryInterface(const css::uno::Type& rType) override;

    void SAL_CALL acquire() throw() override;
    void SAL_CALL release() throw() override;

    //XTypeProvider
    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;

    // XPropertySet
    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;

    // XResultSet
    sal_Bool SAL_CALL next() override;
    sal_Bool SAL_CALL isBeforeFirst() override;
    sal_Bool SAL_CALL isAfterLast() override;
    sal_Bool SAL_CALL isFirst() override;
    sal_Bool SAL_CALL isLast() override;

    void SAL_CALL beforeFirst() override;
    void SAL_CALL afterLast() override;

    sal_Bool SAL_CALL first() override;
    sal_Bool SAL_CALL last() override;

    sal_Int32 SAL_CALL getRow() override;

    sal_Bool SAL_CALL absolute(sal_Int32 row) override;
    sal_Bool SAL_CALL relative(sal_Int32 rows) override;
    sal_Bool SAL_CALL previous() override;

    void SAL_CALL refreshRow() override;

    sal_Bool SAL_CALL rowUpdated() override;
    sal_Bool SAL_CALL rowInserted() override;
    sal_Bool SAL_CALL rowDeleted() override;

    css::uno::Reference<css::uno::XInterface> SAL_CALL getStatement() override;
    // XRow
    sal_Bool SAL_CALL wasNull() override;

    OUString SAL_CALL getString(sal_Int32 column) override;

    sal_Bool SAL_CALL getBoolean(sal_Int32 column) override;
    sal_Int8 SAL_CALL getByte(sal_Int32 column) override;
    sal_Int16 SAL_CALL getShort(sal_Int32 column) override;
    sal_Int32 SAL_CALL getInt(sal_Int32 column) override;
    sal_Int64 SAL_CALL getLong(sal_Int32 column) override;

    float SAL_CALL getFloat(sal_Int32 column) override;
    double SAL_CALL getDouble(sal_Int32 column) override;

    css::uno::Sequence<sal_Int8> SAL_CALL getBytes(sal_Int32 column) override;
    css::util::Date SAL_CALL getDate(sal_Int32 column) override;
    css::util::Time SAL_CALL getTime(sal_Int32 column) override;
    css::util::DateTime SAL_CALL getTimestamp(sal_Int32 column) override;

    css::uno::Reference<css::io::XInputStream> SAL_CALL getBinaryStream(sal_Int32 column) override;
    css::uno::Reference<css::io::XInputStream>
        SAL_CALL getCharacterStream(sal_Int32 column) override;

    Any SAL_CALL getObject(
        sal_Int32 column, const css::uno::Reference<css::container::XNameAccess>& typeMap) override;

    css::uno::Reference<css::sdbc::XRef> SAL_CALL getRef(sal_Int32 column) override;
    css::uno::Reference<css::sdbc::XBlob> SAL_CALL getBlob(sal_Int32 column) override;
    css::uno::Reference<css::sdbc::XClob> SAL_CALL getClob(sal_Int32 column) override;
    css::uno::Reference<css::sdbc::XArray> SAL_CALL getArray(sal_Int32 column) override;

    // XResultSetMetaDataSupplier
    css::uno::Reference<css::sdbc::XResultSetMetaData> SAL_CALL getMetaData() override;

    // XCancellable
    void SAL_CALL cancel() override;

    // XCloseable
    void SAL_CALL close() override;

    // XWarningsSupplier
    Any SAL_CALL getWarnings() override;

    void SAL_CALL clearWarnings() override;

    // XResultSetUpdate
    void SAL_CALL insertRow() override;
    void SAL_CALL updateRow() override;
    void SAL_CALL deleteRow() override;
    void SAL_CALL cancelRowUpdates() override;
    void SAL_CALL moveToInsertRow() override;
    void SAL_CALL moveToCurrentRow() override;

    // XRowUpdate
    void SAL_CALL updateNull(sal_Int32 column) override;
    void SAL_CALL updateBoolean(sal_Int32 column, sal_Bool x) override;
    void SAL_CALL updateByte(sal_Int32 column, sal_Int8 x) override;
    void SAL_CALL updateShort(sal_Int32 column, sal_Int16 x) override;
    void SAL_CALL updateInt(sal_Int32 column, sal_Int32 x) override;
    void SAL_CALL updateLong(sal_Int32 column, sal_Int64 x) override;
    void SAL_CALL updateFloat(sal_Int32 column, float x) override;
    void SAL_CALL updateDouble(sal_Int32 column, double x) override;
    void SAL_CALL updateString(sal_Int32 column, const OUString& x) override;
    void SAL_CALL updateBytes(sal_Int32 column, const css::uno::Sequence<sal_Int8>& x) override;
    void SAL_CALL updateDate(sal_Int32 column, const css::util::Date& x) override;
    void SAL_CALL updateTime(sal_Int32 column, const css::util::Time& x) override;
    void SAL_CALL updateTimestamp(sal_Int32 column, const css::util::DateTime& x) override;
    void SAL_CALL updateBinaryStream(sal_Int32 column,
                                     const css::uno::Reference<css::io::XInputStream>& x,
                                     sal_Int32 length) override;
    void SAL_CALL updateCharacterStream(sal_Int32 column,
                                        const css::uno::Reference<css::io::XInputStream>& x,
                                        sal_Int32 length) override;
    void SAL_CALL updateObject(sal_Int32 column, const Any& x) override;
    void SAL_CALL updateNumericObject(sal_Int32 column, const Any& x, sal_Int32 scale) override;

    // XColumnLocate
    sal_Int32 SAL_CALL findColumn(const OUString& columnName) override;

    // XRowLocate
    Any SAL_CALL getBookmark() override;

    sal_Bool SAL_CALL moveToBookmark(const Any& bookmark) override;
    sal_Bool SAL_CALL moveRelativeToBookmark(const Any& bookmark, sal_Int32 rows) override;
    sal_Int32 SAL_CALL compareBookmarks(const Any& first, const Any& second) override;
    sal_Bool SAL_CALL hasOrderedBookmarks() override;
    sal_Int32 SAL_CALL hashBookmark(const Any& bookmark) override;

    // XDeleteRows
    css::uno::Sequence<sal_Int32> SAL_CALL deleteRows(const css::uno::Sequence<Any>& rows) override;

    void checkColumnIndex(sal_Int32 index);
    void checkRowIndex();

private:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};
} /* mysqlc */
} /* connectivity */
#endif // CONNECTIVITY_SRESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
