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

#include "Connection.hxx"
#include "PreparedStatement.hxx"
#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <propertyids.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::firebird;

using namespace ::comphelper;
using namespace ::osl;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.firebird.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OPreparedStatement::OPreparedStatement( OConnection* _pConnection,
                                        const TTypeInfoVector& _TypeInfo,
                                        const OUString& sql)
    :OStatementCommonBase(_pConnection)
    ,m_aTypeInfo(_TypeInfo)
    ,m_sSqlStatement(sql)
    ,m_pOutSqlda(0)
    ,m_pInSqlda(0)
{
    SAL_INFO("connectivity.firebird", "OPreparedStatement(). "
             "sql: " << sql);
}

void OPreparedStatement::ensurePrepared()
    throw (SQLException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    if (m_aStatementHandle)
        return;

    ISC_STATUS aErr = 0;

    if (!m_pInSqlda)
    {
        m_pInSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(10));
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = 10;
    }

    prepareAndDescribeStatement(m_sSqlStatement,
                               m_pOutSqlda,
                               m_pInSqlda);


    aErr = isc_dsql_describe_bind(m_statusVector,
                                  &m_aStatementHandle,
                                  1,
                                  m_pInSqlda);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_describe_bind failed");
    }
    else if (m_pInSqlda->sqld > m_pInSqlda->sqln) // Not large enough
    {
        short nItems = m_pInSqlda->sqld;
        free(m_pInSqlda);
        m_pInSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(nItems));
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = nItems;
        isc_dsql_describe_bind(m_statusVector,
                               &m_aStatementHandle,
                               1,
                               m_pInSqlda);
    }

    if (!aErr)
        mallocSQLVAR(m_pInSqlda);
    else
        evaluateStatusVector(m_statusVector, m_sSqlStatement, *this);
}

OPreparedStatement::~OPreparedStatement()
{
}

void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatementCommonBase::acquire();
}

void SAL_CALL OPreparedStatement::release() throw()
{
    OStatementCommonBase::release();
}

Any SAL_CALL OPreparedStatement::queryInterface(const Type& rType)
    throw(RuntimeException)
{
    Any aRet = OStatementCommonBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_Base::queryInterface(rType);
    return aRet;
}

uno::Sequence< Type > SAL_CALL OPreparedStatement::getTypes()
    throw(RuntimeException)
{
    return concatSequences(OPreparedStatement_Base::getTypes(),
                           OStatementCommonBase::getTypes());
}

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData()
    throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection, m_pOutSqlda);

    return m_xMetaData;
}

void SAL_CALL OPreparedStatement::close() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    OStatementCommonBase::close();
    if (m_pInSqlda)
    {
        freeSQLVAR(m_pInSqlda);
        free(m_pInSqlda);
        m_pInSqlda = 0;
    }
    if (m_pOutSqlda)
    {
        freeSQLVAR(m_pOutSqlda);
        free(m_pOutSqlda);
        m_pOutSqlda = 0;
    }
}

void SAL_CALL OPreparedStatement::disposing()
{
    close();
}

void SAL_CALL OPreparedStatement::setString(sal_Int32 nParameterIndex,
                                            const OUString& x)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird",
             "setString(" << nParameterIndex << " , " << x << ")");

    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nParameterIndex);
    setParameterNull(nParameterIndex, false);

    OString str = OUStringToOString(x , RTL_TEXTENCODING_UTF8 );

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);

    int dtype = (pVar->sqltype & ~1); // drop flag bit for now

    if (str.getLength() > pVar->sqllen)
        str = str.copy(0, pVar->sqllen);

    switch (dtype) {
    case SQL_VARYING:
    {
        // First 2 bytes indicate string size
        if (str.getLength() > (2^16)-1)
        {
            str = str.copy(0, (2^16)-1);
        }
        const short nLength = str.getLength();
        memcpy(pVar->sqldata, &nLength, 2);
        // Actual data
        memcpy(pVar->sqldata + 2, str.getStr(), str.getLength());
        break;
    }
    case SQL_TEXT:
        memcpy(pVar->sqldata, str.getStr(), str.getLength());
        // Fill remainder with spaces
        // TODO: would 0 be better here for filling?
        memset(pVar->sqldata + str.getLength(), ' ', pVar->sqllen - str.getLength());
        break;
    default:
        // TODO: sane error message
        throw SQLException();
    }
}

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return Reference< XConnection >(m_pConnection);
}

sal_Bool SAL_CALL OPreparedStatement::execute()
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "executeQuery(). "
        "Got called with sql: " <<  m_sSqlStatement);

    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    ensurePrepared();

    ISC_STATUS aErr;

    if (m_xResultSet.is()) // Checks whether we have already run the statement.
    {
        disposeResultSet();
        // Closes the cursor from the last run.
        // This doesn't actually free the statement -- using DSQL_close closes
        // the cursor and keeps the statement, using DSQL_drop frees the statement
        // (and associated cursors).
        aErr = isc_dsql_free_statement(m_statusVector,
                                       &m_aStatementHandle,
                                       DSQL_close);
        if (aErr)
            evaluateStatusVector(m_statusVector,
                                 "isc_dsql_free_statement: close cursor",
                                 *this);
    }

    aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &m_aStatementHandle,
                                1,
                                m_pInSqlda);
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
        evaluateStatusVector(m_statusVector, "isc_dsql_execute", *this);
    }

    m_xResultSet = new OResultSet(m_pConnection,
                                  uno::Reference< XInterface >(*this),
                                  m_aStatementHandle,
                                  m_pOutSqlda);

    return m_xResultSet.is();
    // TODO: implement handling of multiple ResultSets.
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
    throw(SQLException, RuntimeException)
{
    execute();
    return getStatementChangeCount();
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
    throw(SQLException, RuntimeException)
{
    if (!execute())
    {
        // execute succeeded but no results
        throw SQLException(); // TODO: add message to exception
    }

    return m_xResultSet;
}

//----- XParameters -----------------------------------------------------------
void SAL_CALL OPreparedStatement::setNull(sal_Int32 nIndex, sal_Int32 nSqlType)
    throw(SQLException, RuntimeException)
{
    (void) nSqlType;
    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    setParameterNull(nIndex, true);
}

void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 nIndex, sal_Bool x)
    throw(SQLException, RuntimeException)
{
    (void) nIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    // TODO: decide how to deal with bools. Probably just as a byte, although
    // it might be best to just determine the db type and set as appropriate?
}

template <typename T>
void OPreparedStatement::setValue(sal_Int32 nIndex, T nValue, ISC_SHORT nType)
    throw(SQLException)
{
    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nIndex);
    setParameterNull(nIndex, false);

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nIndex - 1);

    if ((pVar->sqltype & ~1) != nType)
        throw SQLException(); // TODO: cast instead?

    memcpy(pVar->sqldata, &nValue, sizeof(nValue));
}

void SAL_CALL OPreparedStatement::setByte(sal_Int32 nIndex, sal_Int8 nValue)
    throw(SQLException, RuntimeException)
{
    (void) nIndex;
    (void) nValue;
    ::dbtools::throwFunctionNotSupportedException("setByte not supported in firebird",
                                                  *this,
                                                  Any());
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 nIndex, sal_Int16 nValue)
    throw(SQLException, RuntimeException)
{
    setValue< sal_Int16 >(nIndex, nValue, SQL_SHORT);
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 nIndex, sal_Int32 nValue)
    throw(SQLException, RuntimeException)
{
    setValue< sal_Int32 >(nIndex, nValue, SQL_LONG);
}

void SAL_CALL OPreparedStatement::setLong(sal_Int32 nIndex, sal_Int64 nValue)
    throw(SQLException, RuntimeException)
{
    setValue< sal_Int64 >(nIndex, nValue, SQL_INT64);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aData;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) sqlType;
    (void) scale;
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) sqlType;
    (void) typeName;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}

// ---- Batch methods -- unsupported -----------------------------------------
void SAL_CALL OPreparedStatement::clearBatch()
    throw(SQLException, RuntimeException)
{
    // Unsupported
}

void SAL_CALL OPreparedStatement::addBatch()
    throw(SQLException, RuntimeException)
{
    // Unsupported by firebird
}

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch()
    throw(SQLException, RuntimeException)
{
    // Unsupported by firebird
    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OStatementCommonBase::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}

void OPreparedStatement::checkParameterIndex(sal_Int32 nParameterIndex)
    throw(SQLException)
{
    ensurePrepared();
    if ((nParameterIndex == 0) || (nParameterIndex > m_pInSqlda->sqld))
        throw SQLException();
    // TODO: sane error message here.
}

void OPreparedStatement::setParameterNull(sal_Int32 nParameterIndex,
                                          bool bSetNull)
{
    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);
    if (pVar->sqltype & 1)
    {
        if (bSetNull)
            *pVar->sqlind = -1;
        else
            *pVar->sqlind = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
