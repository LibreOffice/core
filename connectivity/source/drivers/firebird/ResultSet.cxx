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

#include "ResultSet.hxx"
#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include <propertyids.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <time.h>
#include <TConnection.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>

using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::cppu;
using namespace ::dbtools;
using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;

OResultSet::OResultSet(OConnection* pConnection,
                       const uno::Reference< XInterface >& xStatement,
                       isc_stmt_handle& aStatementHandle,
                       XSQLDA* pSqlda)
    : OResultSet_BASE(pConnection->getMutex())
    , OPropertyContainer(OResultSet_BASE::rBHelper)
    , m_bIsBookmarkable(false)
    , m_nFetchSize(1)
    , m_nResultSetType(::com::sun::star::sdbc::ResultSetType::FORWARD_ONLY)
    , m_nFetchDirection(::com::sun::star::sdbc::FetchDirection::FORWARD)
    , m_nResultSetConcurrency(::com::sun::star::sdbc::ResultSetConcurrency::READ_ONLY)
    , m_pConnection(pConnection)
    , m_xStatement(xStatement)
    , m_xMetaData(0)
    , m_pSqlda(pSqlda)
    , m_statementHandle(aStatementHandle)
    , m_bWasNull(false)
    , m_currentRow(0)
    , m_bIsAfterLastRow(false)
    , m_fieldCount(pSqlda? pSqlda->sqld : 0)
{
    SAL_INFO("connectivity.firebird", "OResultSet().");
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
                     PROPERTY_ID_ISBOOKMARKABLE,
                     PropertyAttribute::READONLY,
                     &m_bIsBookmarkable,
                     ::getCppuType(&m_bIsBookmarkable));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                     PROPERTY_ID_FETCHSIZE,
                     PropertyAttribute::READONLY,
                     &m_nFetchSize,
                     ::getCppuType(&m_nFetchSize));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                     PROPERTY_ID_RESULTSETTYPE,
                     PropertyAttribute::READONLY,
                     &m_nResultSetType,
                     ::getCppuType(&m_nResultSetType));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                     PROPERTY_ID_FETCHDIRECTION,
                     PropertyAttribute::READONLY,
                     &m_nFetchDirection,
                     ::getCppuType(&m_nFetchDirection));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                     PROPERTY_ID_RESULTSETCONCURRENCY,
                     PropertyAttribute::READONLY,
                     &m_nResultSetConcurrency,
                     ::getCppuType(&m_nResultSetConcurrency));

    if (!pSqlda)
        return; // TODO: what?

}

OResultSet::~OResultSet()
{
}

// ---- XResultSet -- Row retrieval methods ------------------------------------
sal_Int32 SAL_CALL OResultSet::getRow() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow;
}

sal_Bool SAL_CALL OResultSet::next() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_currentRow++;

    ISC_STATUS fetchStat = isc_dsql_fetch(m_statusVector,
                               &m_statementHandle,
                               1,
                               m_pSqlda);
    if (fetchStat == 0)         // SUCCESSFUL
    {
        return sal_True;
    }
    else if (fetchStat == 100L) // END OF DATASET
    {
        // TODO: shut the statement
        m_bIsAfterLastRow = true;
        return sal_False;
    }
    else
    {
        SAL_WARN("connectivity.firebird", "Error when fetching data");
        // Throws sql exception as appropriate
        evaluateStatusVector(m_statusVector, "isc_dsql_fetch", *this);
        return sal_False;
    }
}

sal_Bool SAL_CALL OResultSet::previous() throw(SQLException, RuntimeException)
{
    throw SQLException("Firebird doesn't support previous()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::isLast() throw(SQLException, RuntimeException)
{
    throw SQLException("Firebird doesn't support isLast()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bIsAfterLastRow;
}

sal_Bool SAL_CALL OResultSet::isFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 1 && !m_bIsAfterLastRow;
}

// Move to front
void SAL_CALL OResultSet::beforeFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow != 0)
        throw SQLException("Firebird doesn't support beforeFirst()", *this, OUString(), 0, Any());
}
// Move to back
void SAL_CALL OResultSet::afterLast() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (!m_bIsAfterLastRow)
        throw SQLException("Firebird doesn't support afterLast()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::first() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow == 0)
    {
        return next();
    }
    else if (m_currentRow == 1 && !m_bIsAfterLastRow)
    {
        return true;
    }
    else
    {
           throw SQLException("Firebird doesn't support first()", *this, OUString(), 0, Any());
    }
}

sal_Bool SAL_CALL OResultSet::last() throw(SQLException, RuntimeException)
{
    // We need to iterate past the last row to know when we've passed the last
    // row, so we can't actually move to last.
        throw SQLException("Firebird doesn't support last()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 aRow) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (aRow > m_currentRow)
    {
        sal_Int32 aIterations = aRow - m_currentRow;
        return relative(aIterations);
    }
    else
    {
        throw SQLException("Firebird doesn't support retrieval of rows before the current row",
                            *this, OUString(), 0, Any());
    }
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (row > 0)
    {
        while (row--)
        {
            if (!next())
                return sal_False;
        }
        return sal_True;
    }
    else
    {
        throw SQLException("Firebird doesn't support relative() for a negative offset",
                           *this, OUString(), 0, Any());
    }
}

void SAL_CALL OResultSet::checkColumnIndex(sal_Int32 index)
    throw (SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if( index < 1 || index > m_fieldCount )
    {
        throw SQLException( "Column Index is outwith valid range", *this, OUString(), 1, Any() );
    }
}

void SAL_CALL OResultSet::checkRowIndex()
    throw (SQLException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if((m_currentRow < 1) || m_bIsAfterLastRow)
    {
        throw SQLException( "Row index is out of valid range.", *this, OUString(),1, Any() );
    }
}

Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OResultSet::getTypes() throw( RuntimeException)
{
    return concatSequences(OPropertySetHelper::getTypes(), OResultSet_BASE::getTypes());
}
// ---- XColumnLocate ---------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::findColumn(const OUString& columnName)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    uno::Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i;

    for(i = 1; i<=nLen; ++i)
    {
        // We assume case sensitive, otherwise you'd have to test
        // xMeta->isCaseSensitive and use qualsIgnoreAsciiCase as needed.
        if (columnName == xMeta->getColumnName(i))
            break;
    }

    // TODO: add appropriate error
    if (i > nLen)
        throw SQLException();

    return i;
}
// -------------------------------------------------------------------------
uno::Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// ---- Internal Utilities ---------------------------------------------------
bool OResultSet::isNull(sal_Int32 columnIndex)
{
    assert(columnIndex <= m_fieldCount);
    XSQLVAR* pVar = m_pSqlda->sqlvar;

    if (pVar[columnIndex-1].sqltype & 1) // Indicates column may contain null
    {
        if (*pVar[columnIndex-1].sqlind == -1)
            return true;
    }
    return false;
}

template <typename T>
T OResultSet::retrieveValue(sal_Int32 columnIndex)
{
    if ((m_bWasNull = isNull(columnIndex)))
        return 0;

    return *m_pSqlda->sqlvar[columnIndex-1].sqldata;
}

template <>
OUString OResultSet::retrieveValue(sal_Int32 columnIndex)
{
    if ((m_bWasNull = isNull(columnIndex)))
        return OUString();

    // &~1 to remove the "can contain NULL" indicator
    int aSqlType = m_pSqlda->sqlvar[columnIndex-1].sqltype & ~1;
    if (aSqlType == SQL_TEXT )
    {
        return OUString(m_pSqlda->sqlvar[columnIndex-1].sqldata,
                        m_pSqlda->sqlvar[columnIndex-1].sqllen,
                        RTL_TEXTENCODING_UTF8);
    }
    else if (aSqlType == SQL_VARYING)
    {
        // First 2 bytes are a short containing the length of the string
        // No idea if sqllen is still valid here?
        short aLength = *((short*) m_pSqlda->sqlvar[columnIndex-1].sqldata);
        return OUString(m_pSqlda->sqlvar[columnIndex-1].sqldata + 2,
                        aLength,
                        RTL_TEXTENCODING_UTF8);
    }
    else
    {
        return OUString();
        // TODO: Possibly do some sort of type conversion?
    }
}

template <>
ISC_QUAD* OResultSet::retrieveValue(sal_Int32 columnIndex)
{
    if ((m_bWasNull = isNull(columnIndex)))
        return 0;
    return (ISC_QUAD*) m_pSqlda->sqlvar[columnIndex-1].sqldata;
}

template <typename T>
T OResultSet::safelyRetrieveValue(sal_Int32 columnIndex)
    throw (SQLException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(columnIndex);
    checkRowIndex();

    return retrieveValue< T >(columnIndex);
}
// ---- Simple Numerical types -----------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    // TODO: maybe retrieve as string and test for "true", "t", "1" etc. instead?
    return safelyRetrieveValue< bool >(columnIndex);
}

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex )
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int8 >(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return Sequence< sal_Int8 >(); // TODO: implement
    //return safelyRetrieveValue(columnIndex);
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int16 >(columnIndex);
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int32 >(columnIndex);
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int64 >(columnIndex);
}

float SAL_CALL OResultSet::getFloat(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return 0.0f; // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

double SAL_CALL OResultSet::getDouble(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return 0.0; // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

// ---- More complex types ---------------------------------------------------
OUString SAL_CALL OResultSet::getString(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< OUString >(columnIndex);
}

Date SAL_CALL OResultSet::getDate(sal_Int32 nIndex)
    throw(SQLException, RuntimeException)
{
    ISC_DATE aISCDate = safelyRetrieveValue< ISC_DATE >(nIndex);

    struct tm aCTime;
    isc_decode_sql_date(&aISCDate, &aCTime);

    return Date(aCTime.tm_mday, aCTime.tm_mon, aCTime.tm_year);
}

Time SAL_CALL OResultSet::getTime(sal_Int32 nIndex)
    throw(SQLException, RuntimeException)
{
    ISC_TIME aISCTime = safelyRetrieveValue< ISC_TIME >(nIndex);

    struct tm aCTime;
    isc_decode_sql_time(&aISCTime, &aCTime);

    // first field is nanoseconds -- not supported in firebird or struct tm.
    // last field denotes UTC (true) or unknown (false)
    return Time(0, aCTime.tm_sec, aCTime.tm_min, aCTime.tm_hour, false);
}

DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return DateTime(); // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

// -------------------------------------------------------------------------
uno::Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection, m_pSqlda);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
uno::Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// -------------------------------------------------------------------------

uno::Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

uno::Reference< XBlob > SAL_CALL OResultSet::getBlob(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ISC_QUAD* pBlobID = safelyRetrieveValue< ISC_QUAD* >(columnIndex);
    if (!pBlobID)
        return 0;
    return m_pConnection->createBlob(pBlobID);
}
// -------------------------------------------------------------------------

uno::Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) typeMap;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Any();
}



// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close() throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close().");

    {
        MutexGuard aGuard(m_pConnection->getMutex());
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}


uno::Reference< XInterface > SAL_CALL OResultSet::getStatement()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}

// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}

void SAL_CALL OResultSet::refreshRow() throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException("refreshRow not supported in firebird",
                                                  *this,
                                                  Any());
}

//----- OIdPropertyArrayUsageHelper ------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper() const
{
    Sequence< Property > aProperties;
    describeProperties(aProperties);
    return new ::cppu::OPropertyArrayHelper(aProperties);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

// ---- XServiceInfo -----------------------------------------------------------
OUString SAL_CALL OResultSet::getImplementationName() throw ( RuntimeException)
{
    return OUString("com.sun.star.sdbcx.firebird.ResultSet");
}

Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames()
    throw( RuntimeException)
{
     Sequence< OUString > aSupported(2);
    aSupported[0] = OUString("com.sun.star.sdbc.ResultSet");
    aSupported[1] = OUString("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
    throw( RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
