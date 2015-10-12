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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_PREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_PREPAREDSTATEMENT_HXX
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

static const sal_Int32 PREPARED_STATEMENT_CURSOR_NAME = 0;
static const sal_Int32 PREPARED_STATEMENT_ESCAPE_PROCESSING = 1;
static const sal_Int32 PREPARED_STATEMENT_FETCH_DIRECTION = 2;
static const sal_Int32 PREPARED_STATEMENT_FETCH_SIZE = 3;
static const sal_Int32 PREPARED_STATEMENT_MAX_FIELD_SIZE = 4;
static const sal_Int32 PREPARED_STATEMENT_MAX_ROWS = 5;
static const sal_Int32 PREPARED_STATEMENT_QUERY_TIME_OUT = 6;
static const sal_Int32 PREPARED_STATEMENT_RESULT_SET_CONCURRENCY = 7;
static const sal_Int32 PREPARED_STATEMENT_RESULT_SET_TYPE = 8;

#define PREPARED_STATEMENT_SIZE 9

class PreparedStatement : public cppu::OComponentHelper,
                          public cppu::OPropertySetHelper,
                          public com::sun::star::sdbc::XPreparedStatement,
                          public com::sun::star::sdbc::XParameters,
                          public com::sun::star::sdbc::XCloseable,
                          public com::sun::star::sdbc::XWarningsSupplier,
                          public com::sun::star::sdbc::XMultipleResults,
                          public com::sun::star::sdbc::XGeneratedResultSet,
                          public com::sun::star::sdbc::XResultSetMetaDataSupplier
{
private:
    com::sun::star::uno::Any m_props[PREPARED_STATEMENT_SIZE];
    com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > m_connection;
    ConnectionSettings *m_pSettings;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XCloseable > m_lastResultset;
    OString m_stmt;
    OString m_executedStatement;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    OStringVector m_vars;
    OStringVector m_splittedStatement;
    bool  m_multipleResultAvailable;
    sal_Int32 m_multipleResultUpdateCount;
    sal_Int32 m_lastOidInserted;
    OUString m_lastTableInserted;
    OString m_lastQuery;

public:
    /**
     * @param ppConnection The piece of memory, pConnection points to, is accessisble
     *                     as long as a reference to paramenter con is held.
     */
    PreparedStatement( const rtl::Reference< RefCountedMutex > & refMutex,
                       const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection> & con,
                       struct ConnectionSettings *pSettings,
                       const OString &stmt );

     virtual ~PreparedStatement();
public: // XInterface
    virtual void SAL_CALL acquire() throw() override { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() override { OComponentHelper::release(); }
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

public: // XCloseable
    virtual void SAL_CALL close(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XPreparedStatement
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery()
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL executeUpdate(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL execute(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
public: // XParameters
    virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setObjectNull(
        sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBytes(
        sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTimestamp(
        sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBinaryStream(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,
        sal_Int32 length )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCharacterStream(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,
        sal_Int32 length )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setObjectWithInfo(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Any& x,
        sal_Int32 targetSqlType,
        sal_Int32 scale )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRef(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBlob(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setClob(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setArray(
        sal_Int32 parameterIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearParameters(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception, std::exception) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

public: // XGeneratedResultSet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL
    getGeneratedValues(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XMultipleResults
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getUpdateCount(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMoreResults(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XResultSetMetaDataSupplier (is required by framework (see
        // dbaccess/source/core/api/preparedstatement.cxx::getMetaData() )
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // OComponentHelper
    virtual void SAL_CALL disposing() override;

private:
    void checkColumnIndex( sal_Int32 parameterIndex );
    void checkClosed() throw (com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException);
    void raiseSQLException( const char * errorMsg, const char *errorType = 0 )
        throw ( com::sun::star::sdbc::SQLException );
//     PGresult *pgExecute( OString *pQuery );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
