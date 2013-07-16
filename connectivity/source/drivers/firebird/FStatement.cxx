/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <stdio.h>
#include <osl/diagnose.h>
#include "FStatement.hxx"
#include "FConnection.hxx"
#include "FResultSet.hxx"
#include <osl/thread.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include "propertyids.hxx"

#include <ibase.h>

using namespace connectivity::firebird;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
//------------------------------------------------------------------------------

/*
 *    Print the status, the SQLCODE, and exit.
 *    Also, indicate which operation the error occured on.
 */
static int pr_error (const ISC_STATUS* status, const char* operation)
{
    printf("[\n");
    printf("PROBLEM ON \"%s\".\n", operation);

    isc_print_status(status);

    printf("SQLCODE:%d\n", isc_sqlcode(status));

    printf("]\n");

    return 1;
}

//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection )
    : OStatement_BASE(m_aMutex),
    OPropertySetHelper(OStatement_BASE::rBHelper),
    m_pConnection(_pConnection),
    rBHelper(OStatement_BASE::rBHelper)
{
    m_pConnection->acquire();

    ISC_STATUS_ARRAY status;                            // status vector
    isc_db_handle db = m_pConnection->getDBHandler();   // database handle

    // enabling the XSQLDA to accommodate up to 10 select-list items (DEFAULT)
    m_OUTsqlda = (XSQLDA *)malloc(XSQLDA_LENGTH(10));
    m_OUTsqlda->version = SQLDA_VERSION1;
    m_OUTsqlda->sqln = 10;
    m_OUTsqlda->sqld = 0;

    // enabling the XSQLDA to accommodate up to 10 parameter items (DEFAULT)
    m_INsqlda = (XSQLDA *)malloc(XSQLDA_LENGTH(10));
    m_INsqlda->version = SQLDA_VERSION1;
    m_INsqlda->sqln = 10;
    m_INsqlda->sqld = 0;

    m_STMTHandler = 0;          // Set handle to NULL before allocation.
    if (isc_dsql_allocate_statement(status, &db, &m_STMTHandler))
        if (pr_error(status, "allocate statement"))
            return;
}
//-----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    //free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    SAL_INFO("connectivity.firebird", "=> OStatement_BASE2::disposing().");

    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    if (NULL != m_OUTsqlda)
    {
        int i;
        XSQLVAR *var;
        for (i=0, var = m_OUTsqlda->sqlvar; i < m_OUTsqlda->sqld; i++, var++)
            free(var->sqldata);
        free(m_OUTsqlda);
        m_OUTsqlda = NULL;
    }
    if (NULL != m_INsqlda)
    {
        free(m_INsqlda);
        m_INsqlda = NULL;
    }

    ISC_STATUS_ARRAY status;  // status vector
    if (isc_dsql_free_statement(status, &m_STMTHandler, DSQL_drop))
        if (pr_error(status, "fetch data"))
            return;

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw()
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes(
        ::cppu::UnoType< Reference< XMultiPropertySet > >::get(),
        ::cppu::UnoType< Reference< XFastPropertySet > >::get(),
        ::cppu::UnoType< Reference< XPropertySet > >::get());

    return concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    // cancel the current sql statement
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "=> OStatement_Base::close().");

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
    // if you support batches clear it here
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    static const sal_Unicode pattern('"');
    static const sal_Unicode empty(' ');
    OUString query = sql.replace(pattern, empty);

    SAL_INFO("connectivity.firebird", "=> OStatement_Base::executeQuery(). "
             "Got called with sql: " << query);

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    ISC_STATUS_ARRAY status;                            // status vector
    isc_db_handle db = m_pConnection->getDBHandler();   // database handle

    m_TRANSHandler = 0L; // transaction handle
    if (isc_start_transaction(status, &m_TRANSHandler, 1, &db, 0, NULL))
        if (pr_error(status, "start transaction"))
            return sal_False;

    char *sqlStr = strdup(OUStringToOString( query, RTL_TEXTENCODING_UTF8 ).getStr());
    if (isc_dsql_execute_immediate(status, &db, &m_TRANSHandler, 0, sqlStr, 1, NULL))
        if (pr_error(status, "create table"))
            return sal_False;
    free(sqlStr);

    if (isc_commit_transaction(status, &m_TRANSHandler))
        if (pr_error(status, "commit transaction"))
            return sal_False;

    // returns true when a resultset is available
    return sal_False;
}

void SAL_CALL OStatement_Base::prepareQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ISC_STATUS_ARRAY status;                            // status vector
    isc_db_handle db = m_pConnection->getDBHandler();   // database handle

    m_TRANSHandler = 0L; // transaction handle
    if (isc_start_transaction(status, &m_TRANSHandler, 1, &db, 0, NULL))
        if (pr_error(status, "start transaction"))
            return;

    // sets the statement handle (stmt) to refer to the parsed format.
    char *sqlStr = strdup(OUStringToOString( sql, RTL_TEXTENCODING_UTF8 ).getStr());
    if (isc_dsql_prepare(status, &m_TRANSHandler, &m_STMTHandler, 0, sqlStr, 1, m_OUTsqlda))
        if (pr_error(status, "prepare statement"))
            return;
    free(sqlStr);

    // fill the input XSQLDA with information about the parameters
    if (isc_dsql_describe_bind(status, &m_STMTHandler, 1, m_INsqlda))
        if (pr_error(status, "bind statement"))
            return;

    XSQLVAR *var = NULL;
    int i, dtype;

    // determine if the input descriptor can accommodate the number of parameters
    // contained in the statement.
    if (0 == m_INsqlda->sqld)
    {
        free(m_INsqlda);
        m_INsqlda = NULL;
    }
    else
    {
        if (m_INsqlda->sqld > m_INsqlda->sqln)
        {
            int n = m_INsqlda->sqld;
            free(m_INsqlda);
            m_INsqlda = (XSQLDA *)malloc(XSQLDA_LENGTH(n));
            m_INsqlda->sqln = n;
            m_INsqlda->version = SQLDA_VERSION1;
            if (isc_dsql_describe_bind(status, &m_STMTHandler, 1, m_INsqlda))
                if (pr_error(status, "bind statement 2"))
                    return;
        }
    }

    // fill the output XSQLDA with information about the select-list items.
    if (isc_dsql_describe(status, &m_STMTHandler, 1, m_OUTsqlda))
        if (pr_error(status, "describe statement"))
            return;

    // determine if the output descriptor can accommodate the number of select-list
    // items specified in the statement.
    if (m_OUTsqlda->sqld > m_OUTsqlda->sqln)
    {
        int n = m_OUTsqlda->sqld;
        free(m_OUTsqlda);
        m_OUTsqlda = (XSQLDA *)malloc(XSQLDA_LENGTH(n));
        m_OUTsqlda->sqln = n;
        m_OUTsqlda->version = SQLDA_VERSION1;
        if (isc_dsql_describe(status, &m_STMTHandler, 1, m_OUTsqlda))
            if (pr_error(status, "describe statement 2"))
                return;
    }

    // Process each XSQLVAR parameter structure in the output XSQLDA
    for (i=0, var = m_OUTsqlda->sqlvar; i < m_OUTsqlda->sqld; i++, var++)
    {
        dtype = (var->sqltype & ~1); /* drop flag bit for now */
        switch(dtype) {
        case SQL_VARYING:
            var->sqltype = SQL_TEXT;
            var->sqldata = (char *)malloc(sizeof(char)*var->sqllen + 2);
            break;
        case SQL_TEXT:
            var->sqldata = (char *)malloc(sizeof(char)*var->sqllen);
            break;
        case SQL_LONG:
            var->sqldata = (char *)malloc(sizeof(long));
            break;
        case SQL_SHORT:
            var->sqldata = (char *)malloc(sizeof(char)*var->sqllen);
            break;
        case SQL_FLOAT:
            var->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_DOUBLE:
            var->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_D_FLOAT:
            var->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_TIMESTAMP:
            var->sqldata = (char *)malloc(sizeof(time_t));
            break;
        case SQL_INT64:
            var->sqldata = (char *)malloc(sizeof(int));
            break;
            /* process remaining types */
        default:
            OSL_ASSERT( false );
            break;
        }
        if (var->sqltype & 1)
        {
            /* allocate variable to hold NULL status */
            var->sqlind = (short *)malloc(sizeof(short));
        }
    }
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "=> OStatement_Base::executeQuery(). "
             "Got called with sql: " << sql);

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    ISC_STATUS_ARRAY status;                            // status vector

    prepareQuery(sql);

    if (isc_dsql_execute(status, &m_TRANSHandler, &m_STMTHandler, 1, m_INsqlda))
        if (pr_error(status, "execute query"))
            return NULL;

    Reference< OResultSet > pResult( new OResultSet( this) );
    //initializeResultSet( pResult.get() );
    Reference< XResultSet > xRS = pResult.get();

    if (isc_commit_transaction(status, &m_TRANSHandler))
        if (pr_error(status, "commit transaction"))
            return NULL;

    SAL_INFO("connectivity.firebird", "=> OStatement::executeQuery(). "
             "Query executed.");

    close();

    return xRS;
}


// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // just return our connection here
    return (Reference< XConnection >)m_pConnection;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement_Base::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatement_Base::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement::addBatch( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aBatchList.push_back(sql);
}
// -------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    (void) sql;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // the return values gives information about how many rows are affected by executing the sql statement
    return 0;

}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

//  return our save resultset here
    return m_xResultSet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // if your driver supports more than only one resultset
    // and has one more at this moment return true
    return sal_False;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  ::rtl::OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    (void) rConvertedValue;
    (void) rOldValue;
    (void) nHandle;
    (void) rValue;
    sal_Bool bConverted = sal_False;
    // here we have to try to convert
    return bConverted;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    (void) rValue;
    // set the value to what ever is necessary
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OStatement_Base::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    (void) rValue;
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
