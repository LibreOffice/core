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

#include "FConnection.hxx"
#include "FPreparedStatement.hxx"
#include "FResultSetMetaData.hxx"
#include "FResultSet.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
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
    ,m_statementHandle(0)
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

    if (m_statementHandle)
        return;

    ISC_STATUS aErr = 0;

    if (!m_pInSqlda)
    {
        m_pInSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(10));
        m_pInSqlda->version = SQLDA_VERSION1;
        m_pInSqlda->sqln = 10;
    } // TODO: free this on closing

    aErr = prepareAndDescribeStatement(m_sSqlStatement,
                                       m_statementHandle,
                                       m_pOutSqlda,
                                       m_pInSqlda);
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "prepareAndDescribeStatement failed");
    }
    else if (m_statementHandle)
    {
        isc_dsql_describe_bind(m_statusVector,
                               &m_statementHandle,
                               1,
                               m_pInSqlda);
    }

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
                               &m_statementHandle,
                               1,
                               m_pInSqlda);
    }
//         char aItems[] = {
//             isc_info_sql_num_variables
//         };
//         char aResultBuffer[8];
//         isc_dsql_sql_info(m_statusVector,
//                           &m_statementHandle,
//                           sizeof(aItems),
//                           aItems,
//                           sizeof(aResultBuffer),
//                           aResultBuffer);
//         if (aResultBuffer[0] == isc_info_sql_num_variables)
//         {
//             short aVarLength = (short) isc_vax_integer(aResultBuffer+1, 2);
//             m_nNumParams = isc_vax_integer(aResultBuffer+3, aVarLength);
//         }
//     }
    mallocSQLVAR(m_pInSqlda);
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
    SAL_INFO("connectivity.firebird", "close()");

    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    if (m_statementHandle)
    {
        // TODO: implement
    }

    OStatementCommonBase::close();
}

void SAL_CALL OPreparedStatement::setString(sal_Int32 nParameterIndex,
                                            const OUString& x)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "setString(). "
             "parameterIndex: " << nParameterIndex << " , "
             "x: " << x);

    MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    ensurePrepared();

    checkParameterIndex(nParameterIndex);

    OString str = OUStringToOString(x , RTL_TEXTENCODING_UTF8 );

    XSQLVAR* pVar = m_pInSqlda->sqlvar + (nParameterIndex - 1);

    int dtype = (pVar->sqltype & ~1); // drop flag bit for now
    switch (dtype) {
    case SQL_VARYING:
        pVar->sqltype = SQL_TEXT;
    case SQL_TEXT:
        if (str.getLength() > pVar->sqllen)
        { // Cut off overflow
            memcpy(pVar->sqldata, str.getStr(), pVar->sqllen);
        }
        else
        {
            memcpy(pVar->sqldata, str.getStr(), str.getLength());
            // Fill remainder with spaces
            // TODO: would 0 be better here for filling?
            memset(pVar->sqldata + str.getLength(), ' ', pVar->sqllen - str.getLength());
        }
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

    aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &m_statementHandle,
                                1,
                                m_pInSqlda);
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
        evaluateStatusVector(m_statusVector, "isc_dsql_execute", *this);
    }

    // TODO: check we actually got results -- ?

    return sal_True;
    // TODO: implement handling of multiple ResultSets.
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
    throw(SQLException, RuntimeException)
{
    execute();
    // TODO: get the number of rows changed -- look in Statement::executeUpdate for details
    return 0;
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
    throw(SQLException, RuntimeException)
{
    if (!execute())
    {
        // execute succeeded but no results
        throw SQLException(); // TODO: add message to exception
    }

    uno::Reference< OResultSet > pResult(new OResultSet(m_pConnection,
                                                        uno::Reference< XInterface >(*this),
                                                        m_statementHandle,
                                                        m_pOutSqlda));
    m_xResultSet = pResult.get();

    return m_xResultSet;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);


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

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_pConnection->getMutex() );
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) sqlType;
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

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
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
    if ((nParameterIndex == 0) || (nParameterIndex > m_pOutSqlda->sqld))
        throw SQLException();
    // TODO: sane error message here.
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
