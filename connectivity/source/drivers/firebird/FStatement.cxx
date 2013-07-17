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

#include "propertyids.hxx"
#include "FStatement.hxx"
#include "FConnection.hxx"
#include "FResultSet.hxx"

#include <ibase.h>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::firebird;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::std;

OStatement_Base::OStatement_Base(OConnection* _pConnection)
    : OStatement_BASE(m_aMutex),
      OPropertySetHelper(OStatement_BASE::rBHelper),
      m_pConnection(_pConnection),
      rBHelper(OStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
//     // enabling the XSQLDA to accommodate up to 10 parameter items (DEFAULT)
//     m_INsqlda = (XSQLDA *)malloc(XSQLDA_LENGTH(10));
//     m_INsqlda->version = SQLDA_VERSION1;
//     m_INsqlda->sqln = 10;
//     m_INsqlda->sqld = 0;

}

OStatement_Base::~OStatement_Base()
{
}

void OStatement_Base::disposeResultSet()
{
    //free the cursor if alive
//     uno::Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
//     if (xComp.is())
//         xComp->dispose();
//     m_xResultSet = uno::Reference< XResultSet>();
}

void OStatement_BASE2::disposing()
{
    SAL_INFO("connectivity.firebird", "=> OStatement_BASE2::disposing().");

    MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

//     if (NULL != m_OUTsqlda)
//     {
//         int i;
//         XSQLVAR *var;
//         for (i=0, var = m_OUTsqlda->sqlvar; i < m_OUTsqlda->sqld; i++, var++)
//             free(var->sqldata);
//         free(m_OUTsqlda);
//         m_OUTsqlda = NULL;
//     }
//     if (NULL != m_INsqlda)
//     {
//         free(m_INsqlda);
//         m_INsqlda = NULL;
//     }

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
        ::cppu::UnoType< uno::Reference< XMultiPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XFastPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XPropertySet > >::get());

    return concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    // cancel the current sql statement
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "=> OStatement_Base::close().");

    {
        MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatement_Base::queryInterface(rType);
    return aRet;
}

// ---- XStatement -----------------------------------------------------------
sal_Int32 SAL_CALL OStatement_Base::executeUpdate(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    int aErr = isc_dsql_execute_immediate(m_statusVector,
                                          &m_pConnection->getDBHandle(),
                                          &m_pConnection->getTransaction(),
                                          0,
                                          OUStringToOString(sql, RTL_TEXTENCODING_UTF8).getStr(),
                                          1,
                                          NULL);

    if (aErr)
        SAL_WARN("connectivity.firebird", "isc_dsql_execute_immediate failed" );

    m_pConnection->evaluateStatusVector(m_statusVector, sql);
    // TODO: get number of changed rows with SELECT ROW_COUNT (use executeQuery)
    //     return getUpdateCount();
    return 0;
}

int OStatement_Base::prepareAndDescribeStatement(const OUString& sql,
                                                 isc_stmt_handle& aStatementHandle,
                                                 XSQLDA*& pOutSqlda,
                                                 XSQLVAR*& pVar)
{
    if (!pOutSqlda)
    {
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(10));
        pOutSqlda->version = SQLDA_VERSION1;
        pOutSqlda->sqln = 10;
    }

    int aErr = 0;

    aErr = isc_dsql_allocate_statement(m_statusVector,
                                       &m_pConnection->getDBHandle(),
                                       &aStatementHandle);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_allocate_statement failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_prepare(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &aStatementHandle,
                                0,
                                OUStringToOString(sql, RTL_TEXTENCODING_UTF8).getStr(),
                                1,
                                NULL);
    }

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_prepare failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_describe(m_statusVector,
                                 &aStatementHandle,
                                 1,
                                 pOutSqlda);
    }

    // Ensure we have enough space in pOutSqlda
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_describe failed");
        return aErr;
    }
    else if (!aErr && (pOutSqlda->sqld > pOutSqlda->sqln))
    {
        int n = pOutSqlda->sqld;
        free(pOutSqlda);
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(n));
        pOutSqlda->version = SQLDA_VERSION1;
        aErr = isc_dsql_describe(m_statusVector,
                                 &aStatementHandle,
                                 1,
                                 pOutSqlda);
    }
    pVar = pOutSqlda->sqlvar;

    // Process each XSQLVAR parameter structure in the output XSQLDA
    if (aErr)
    {
        SAL_WARN("connectivity.firebird","isc_dsql_describe failed when resizing pOutSqlda");
        return aErr;
    }
    else
    {
        for (int i=0; i < pOutSqlda->sqld; i++, pVar++)
        {
            int dtype = (pVar->sqltype & ~1); /* drop flag bit for now */
            switch(dtype) {
            case SQL_VARYING:
                pVar->sqltype = SQL_TEXT;
                pVar->sqldata = (char *)malloc(sizeof(char)*pVar->sqllen + 2);
                break;
            case SQL_TEXT:
                pVar->sqldata = (char *)malloc(sizeof(char)*pVar->sqllen);
                break;
            case SQL_LONG:
                pVar->sqldata = (char *)malloc(sizeof(long));
                break;
            case SQL_SHORT:
                pVar->sqldata = (char *)malloc(sizeof(char)*pVar->sqllen);
                break;
            case SQL_FLOAT:
                pVar->sqldata = (char *)malloc(sizeof(double));
                break;
            case SQL_DOUBLE:
                pVar->sqldata = (char *)malloc(sizeof(double));
                break;
            case SQL_D_FLOAT:
                pVar->sqldata = (char *)malloc(sizeof(double));
                break;
            case SQL_TIMESTAMP:
                pVar->sqldata = (char *)malloc(sizeof(time_t));
                break;
            case SQL_INT64:
                pVar->sqldata = (char *)malloc(sizeof(int));
                break;
                /* process remaining types */
            default:
                OSL_ASSERT( false );
                break;
            }
            if (pVar->sqltype & 1)
            {
                /* allocate variable to hold NULL status */
                pVar->sqlind = (short *)malloc(sizeof(short));
            }
        }
    }

    return aErr;
}

uno::Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery(const OUString& sql) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    XSQLDA* pOutSqlda = 0;
    XSQLVAR* pVar = 0;
    isc_stmt_handle aStatementHandle = 0;
    int aErr = 0;


    aErr = prepareAndDescribeStatement(sql,
                                       aStatementHandle,
                                       pOutSqlda,
                                       pVar);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "prepareAndDescribeStatement failed");
    }
    else
    {
        aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &aStatementHandle,
                                1,
                                NULL);
        if (aErr)
            SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
    }

    uno::Reference< OResultSet > pResult(new OResultSet(this, pOutSqlda));
    //initializeResultSet( pResult.get() );
     m_xResultSet = pResult.get();

    // TODO: deal with cleanup
//    close();
    m_pConnection->evaluateStatusVector(m_statusVector, sql);
    return m_xResultSet;
}

sal_Bool SAL_CALL OStatement_Base::execute(const OUString& sql) throw(SQLException, RuntimeException)
{
    // TODO: is this needed, and for all execute methods>
    static const sal_Unicode pattern('"');
    static const sal_Unicode empty(' ');
    OUString query = sql.replace(pattern, empty);

    // TODO: use the isc_ validation functions?

    SAL_INFO("connectivity.firebird", "=> OStatement_Base::executeQuery(). "
             "Got called with sql: " << query);

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    XSQLDA* pOutSqlda = 0;
    XSQLVAR* pVar = 0;
    isc_stmt_handle aStatementHandle = 0;
    int aErr = 0;


    aErr = prepareAndDescribeStatement(sql,
                                       aStatementHandle,
                                       pOutSqlda,
                                       pVar);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
    }
    else
    {
        aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &aStatementHandle,
                                1,
                                NULL);
        if (aErr)
            SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
    }

    m_pConnection->evaluateStatusVector(m_statusVector, sql);

    // returns true when a resultset is available
    return sal_False;
}

uno::Reference< XConnection > SAL_CALL OStatement_Base::getConnection()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (uno::Reference< XConnection >)m_pConnection;
}

// ---- XMultipleResults - UNSUPPORTED ----------------------------------------
uno::Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return uno::Reference< XResultSet >();
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatement_BASE::rBHelper.bDisposed);

//     return m_xResultSet;
}

sal_Bool SAL_CALL OStatement_Base::getMoreResults() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return sal_False;
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatement_BASE::rBHelper.bDisposed);
}

sal_Int32 SAL_CALL OStatement_Base::getUpdateCount() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return 0;
}

// ---- XBatchExecution - UNSUPPORTED ----------------------------------------
void SAL_CALL OStatement::addBatch(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    (void) sql;
}

void SAL_CALL OStatement::clearBatch() throw(SQLException, RuntimeException)
{
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch() throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 >();
}

// ---- XWarningsSupplier - UNSUPPORTED ----------------------------------------
Any SAL_CALL OStatement_Base::getWarnings() throw(SQLException, RuntimeException)
{
    return Any();
}

void SAL_CALL OStatement_Base::clearWarnings() throw(SQLException, RuntimeException)
{
}

::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  OUString);
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
    throw (IllegalArgumentException)
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

IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");

void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}

void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}

void SAL_CALL OStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}

void SAL_CALL OStatement::release() throw()
{
    OStatement_BASE2::release();
}

uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
