/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#pragma once
#include <vector>

#include <libpq-fe.h>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

#include "pq_connection.hxx"
namespace pq_sdbc_driver
{

const sal_Int32 PREPARED_STATEMENT_CURSOR_NAME = 0;
const sal_Int32 PREPARED_STATEMENT_ESCAPE_PROCESSING = 1;
const sal_Int32 PREPARED_STATEMENT_FETCH_DIRECTION = 2;
const sal_Int32 PREPARED_STATEMENT_FETCH_SIZE = 3;
const sal_Int32 PREPARED_STATEMENT_MAX_FIELD_SIZE = 4;
const sal_Int32 PREPARED_STATEMENT_MAX_ROWS = 5;
const sal_Int32 PREPARED_STATEMENT_QUERY_TIME_OUT = 6;
const sal_Int32 PREPARED_STATEMENT_RESULT_SET_CONCURRENCY = 7;
const sal_Int32 PREPARED_STATEMENT_RESULT_SET_TYPE = 8;

#define PREPARED_STATEMENT_SIZE 9

typedef ::cppu::WeakComponentImplHelper<    css::sdbc::XPreparedStatement,
                                            css::sdbc::XParameters,
                                            css::sdbc::XCloseable,
                                            css::sdbc::XWarningsSupplier,
                                            css::sdbc::XMultipleResults,
                                            css::sdbc::XGeneratedResultSet,
                                            css::sdbc::XResultSetMetaDataSupplier
                                            > PreparedStatement_BASE;
class PreparedStatement : public PreparedStatement_BASE,
                          public cppu::OPropertySetHelper
{
private:
    css::uno::Any m_props[PREPARED_STATEMENT_SIZE];
    css::uno::Reference< css::sdbc::XConnection > m_connection;
    ConnectionSettings *m_pSettings;
    css::uno::Reference< css::sdbc::XCloseable > m_lastResultset;
    OString m_stmt;
    OString m_executedStatement;
    ::rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
    std::vector< OString > m_vars;
    std::vector< OString > m_splittedStatement;
    bool  m_multipleResultAvailable;
    sal_Int32 m_multipleResultUpdateCount;
    sal_Int32 m_lastOidInserted;
    OUString m_lastTableInserted;
    OString m_lastQuery;

public:
    /**
     * @param ppConnection The piece of memory, pConnection points to, is accessible
     *                     as long as a reference to parameter con is held.
     */
    PreparedStatement( const rtl::Reference< comphelper::RefCountedMutex > & refMutex,
                       const css::uno::Reference< css::sdbc::XConnection> & con,
                       struct ConnectionSettings *pSettings,
                       const OString &stmt );

     virtual ~PreparedStatement() override;
public: // XInterface
    virtual void SAL_CALL acquire() noexcept override { PreparedStatement_BASE::acquire(); }
    virtual void SAL_CALL release() noexcept override { PreparedStatement_BASE::release(); }
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & reqType ) override;

public: // XCloseable
    virtual void SAL_CALL close(  ) override;

public: // XPreparedStatement
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery() override;
    virtual sal_Int32 SAL_CALL executeUpdate(  ) override;
    virtual sal_Bool SAL_CALL execute(  ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;
public: // XParameters
    virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
    virtual void SAL_CALL setObjectNull(
        sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
    virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) override;
    virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
    virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
    virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
    virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
    virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) override;
    virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) override;
    virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) override;
    virtual void SAL_CALL setBytes(
        sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x ) override;
    virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
    virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
    virtual void SAL_CALL setTimestamp(
        sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
    virtual void SAL_CALL setBinaryStream(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::io::XInputStream >& x,
        sal_Int32 length ) override;
    virtual void SAL_CALL setCharacterStream(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::io::XInputStream >& x,
        sal_Int32 length ) override;
    virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const css::uno::Any& x ) override;
    virtual void SAL_CALL setObjectWithInfo(
        sal_Int32 parameterIndex,
        const css::uno::Any& x,
        sal_Int32 targetSqlType,
        sal_Int32 scale ) override;
    virtual void SAL_CALL setRef(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::sdbc::XRef >& x ) override;
    virtual void SAL_CALL setBlob(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::sdbc::XBlob >& x ) override;
    virtual void SAL_CALL setClob(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::sdbc::XClob >& x ) override;
    virtual void SAL_CALL setArray(
        sal_Int32 parameterIndex,
        const css::uno::Reference< css::sdbc::XArray >& x ) override;
    virtual void SAL_CALL clearParameters(  ) override;

public: // XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  ) override;
    virtual void SAL_CALL clearWarnings(  ) override;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any & rConvertedValue,
        css::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    css::uno::Reference < css::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() override;

public: // XGeneratedResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    getGeneratedValues(  ) override;

public: // XMultipleResults
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet(  ) override;
    virtual sal_Int32 SAL_CALL getUpdateCount(  ) override;
    virtual sal_Bool SAL_CALL getMoreResults(  ) override;

public: // XResultSetMetaDataSupplier (is required by framework (see
        // dbaccess/source/core/api/preparedstatement.cxx::getMetaData() )
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;

public: // OComponentHelper
    virtual void SAL_CALL disposing() override;

private:
    void checkColumnIndex( sal_Int32 parameterIndex );
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    void checkClosed();
    /// @throws css::sdbc::SQLException
    void raiseSQLException( const char * errorMsg );
//     PGresult *pgExecute( OString *pQuery );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
