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

#ifndef _PQ_STATEMENT_HXX_
#define _PQ_STATEMENT_HXX_
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <libpq-fe.h>

#include "pq_connection.hxx"
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

namespace pq_sdbc_driver
{

static const sal_Int32 STATEMENT_CURSOR_NAME = 0;
static const sal_Int32 STATEMENT_ESCAPE_PROCESSING = 1;
static const sal_Int32 STATEMENT_FETCH_DIRECTION = 2;
static const sal_Int32 STATEMENT_FETCH_SIZE = 3;
static const sal_Int32 STATEMENT_MAX_FIELD_SIZE = 4;
static const sal_Int32 STATEMENT_MAX_ROWS = 5;
static const sal_Int32 STATEMENT_QUERY_TIME_OUT = 6;
static const sal_Int32 STATEMENT_RESULT_SET_CONCURRENCY = 7;
static const sal_Int32 STATEMENT_RESULT_SET_TYPE = 8;

#define STATEMENT_SIZE 9

class Statement : public cppu::OComponentHelper,
                  public cppu::OPropertySetHelper,
                  public com::sun::star::sdbc::XStatement,
                  public com::sun::star::sdbc::XCloseable,
                  public com::sun::star::sdbc::XWarningsSupplier,
                  public com::sun::star::sdbc::XMultipleResults,
                  public com::sun::star::sdbc::XGeneratedResultSet,
                  public com::sun::star::sdbc::XResultSetMetaDataSupplier
{
private:
    com::sun::star::uno::Any m_props[STATEMENT_SIZE];
    com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > m_connection;
    ConnectionSettings *m_pSettings;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XCloseable > m_lastResultset;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    sal_Bool  m_multipleResultAvailable;
    sal_Int32 m_multipleResultUpdateCount;
    sal_Int32 m_lastOidInserted;
    OUString m_lastTableInserted;
    OString m_lastQuery;

public:
    /**
     * @param ppConnection The piece of memory, pConnection points to, is accessisble
     *                     as long as a reference to paramenter con is held.
     */
    Statement( const rtl::Reference< RefCountedMutex > & refMutex,
               const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection> & con,
               struct ConnectionSettings *pSettings );

    virtual ~Statement();
public: // XInterface
    virtual void SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() { OComponentHelper::release(); }
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException);

public: // XCloseable
    virtual void SAL_CALL close(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XStatement
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery(
        const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL execute( const OUString& sql )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearWarnings(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

    // XPropertySet
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException);

public: // XGeneratedResultSet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL
    getGeneratedValues(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // XMultipleResults
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getUpdateCount(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMoreResults(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

public: // OComponentHelper
    virtual void SAL_CALL disposing();

public: // XResultSetMetaDataSupplier (is required by framework (see
        // dbaccess/source/core/api/preparedstatement.cxx::getMetaData() )
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

private:
    void checkClosed() throw (com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException);
    void raiseSQLException( const OUString & sql, const char * errorMsg, const char *errorType = 0 )
        throw ( com::sun::star::sdbc::SQLException );
};


struct CommandData
{
    ConnectionSettings **ppSettings;
    sal_Int32 *pLastOidInserted;
    sal_Int32 *pMultipleResultUpdateCount;
    sal_Bool *pMultipleResultAvailable;
    OUString *pLastTableInserted;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XCloseable > *pLastResultset;
    OString *pLastQuery;
    ::rtl::Reference< RefCountedMutex > refMutex;
    ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > owner;
    ::com::sun::star::uno::Reference< com::sun::star::sdbcx::XTablesSupplier > tableSupplier;
    sal_Int32 concurrency;
};

bool executePostgresCommand( const OString & cmd, struct CommandData *data );
com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet > getGeneratedValuesFromLastInsert(
    ConnectionSettings *pConnectionSettings,
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &connection,
    sal_Int32 nLastOid,
    const OUString & lastTableInserted,
    const OString & lastQuery );


}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
