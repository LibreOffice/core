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

#include <sal/config.h>

#include <string_view>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/component.hxx>

#include <libpq-fe.h>

#include "pq_connection.hxx"
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

namespace pq_sdbc_driver
{

const sal_Int32 STATEMENT_CURSOR_NAME = 0;
const sal_Int32 STATEMENT_ESCAPE_PROCESSING = 1;
const sal_Int32 STATEMENT_FETCH_DIRECTION = 2;
const sal_Int32 STATEMENT_FETCH_SIZE = 3;
const sal_Int32 STATEMENT_MAX_FIELD_SIZE = 4;
const sal_Int32 STATEMENT_MAX_ROWS = 5;
const sal_Int32 STATEMENT_QUERY_TIME_OUT = 6;
const sal_Int32 STATEMENT_RESULT_SET_CONCURRENCY = 7;
const sal_Int32 STATEMENT_RESULT_SET_TYPE = 8;

#define STATEMENT_SIZE 9

typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                           css::sdbc::XCloseable,
                                           css::sdbc::XWarningsSupplier,
                                           css::sdbc::XMultipleResults,
                                           css::sdbc::XGeneratedResultSet,
                                           css::sdbc::XResultSetMetaDataSupplier
                                           > Statement_BASE;

class Statement : public Statement_BASE,
                  public cppu::OPropertySetHelper
{
private:
    css::uno::Any m_props[STATEMENT_SIZE];
    css::uno::Reference< css::sdbc::XConnection > m_connection;
    ConnectionSettings *m_pSettings;
    css::uno::Reference< css::sdbc::XCloseable > m_lastResultset;
    ::rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
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
    Statement( const rtl::Reference< comphelper::RefCountedMutex > & refMutex,
               const css::uno::Reference< css::sdbc::XConnection> & con,
               struct ConnectionSettings *pSettings );

    virtual ~Statement() override;
public: // XInterface
    virtual void SAL_CALL acquire() throw() override { Statement_BASE::acquire(); }
    virtual void SAL_CALL release() throw() override { Statement_BASE::release(); }
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & reqType ) override;

public: // XCloseable
    virtual void SAL_CALL close(  ) override;

public: // XStatement
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(
        const OUString& sql ) override;
    virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) override;
    virtual sal_Bool SAL_CALL execute( const OUString& sql ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;

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

public: // OComponentHelper
    virtual void SAL_CALL disposing() override;

public: // XResultSetMetaDataSupplier (is required by framework (see
        // dbaccess/source/core/api/preparedstatement.cxx::getMetaData() )
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;

private:
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    void checkClosed();
    /// @throws css::sdbc::SQLException
    void raiseSQLException( std::u16string_view sql, const char * errorMsg );
};


struct CommandData
{
    ConnectionSettings **ppSettings;
    sal_Int32 *pLastOidInserted;
    sal_Int32 *pMultipleResultUpdateCount;
    bool *pMultipleResultAvailable;
    OUString *pLastTableInserted;
    css::uno::Reference< css::sdbc::XCloseable > *pLastResultset;
    OString *pLastQuery;
    ::rtl::Reference< comphelper::RefCountedMutex > refMutex;
    css::uno::Reference< css::uno::XInterface > owner;
    css::uno::Reference< css::sdbcx::XTablesSupplier > tableSupplier;
    sal_Int32 concurrency;
};

bool executePostgresCommand( const OString & cmd, struct CommandData *data );
css::uno::Reference< css::sdbc::XResultSet > getGeneratedValuesFromLastInsert(
    ConnectionSettings *pConnectionSettings,
    const css::uno::Reference< css::sdbc::XConnection > &connection,
    sal_Int32 nLastOid,
    const OUString & lastTableInserted,
    const OString & lastQuery );


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
