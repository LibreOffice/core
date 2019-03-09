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

#include <odbc/OResultSet.hxx>
#include <odbc/OTools.hxx>
#include <odbc/OResultSetMetaData.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <sal/log.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::odbc;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

#define ODBC_SQL_NOT_DEFINED    99UL
static_assert(ODBC_SQL_NOT_DEFINED != SQL_UB_OFF, "ODBC_SQL_NOT_DEFINED must be unique");
static_assert(ODBC_SQL_NOT_DEFINED != SQL_UB_ON, "ODBC_SQL_NOT_DEFINED must be unique");
static_assert(ODBC_SQL_NOT_DEFINED != SQL_UB_FIXED, "ODBC_SQL_NOT_DEFINED must be unique");
static_assert(ODBC_SQL_NOT_DEFINED != SQL_UB_VARIABLE, "ODBC_SQL_NOT_DEFINED must be unique");

namespace
{
    const SQLLEN nMaxBookmarkLen = 20;
}


//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
OUString SAL_CALL OResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.odbc.ResultSet");
}

 Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


OResultSet::OResultSet(SQLHANDLE _pStatementHandle ,OStatement_Base* pStmt) :   OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_bFetchDataInOrder(true)
                        ,m_aStatementHandle(_pStatementHandle)
                        ,m_aConnectionHandle(pStmt->getConnectionHandle())
                        ,m_pStatement(pStmt)
                        ,m_xStatement(*pStmt)
                        ,m_nTextEncoding(pStmt->getOwnConnection()->getTextEncoding())
                        ,m_nRowPos(0)
                        ,m_nUseBookmarks(ODBC_SQL_NOT_DEFINED)
                        ,m_nCurrentFetchState(0)
                        ,m_bWasNull(true)
                        ,m_bEOF(true)
                        ,m_bRowInserted(false)
                        ,m_bRowDeleted(false)
                        ,m_bUseFetchScroll(false)
{
    osl_atomic_increment( &m_refCount );
    try
    {
        m_pRowStatusArray.reset( new SQLUSMALLINT[1] ); // the default value
        setStmtOption<SQLUSMALLINT*, SQL_IS_POINTER>(SQL_ATTR_ROW_STATUS_PTR, m_pRowStatusArray.get());
    }
    catch(const Exception&)
    { // we don't want our result destroy here
    }
    SQLULEN nCurType = 0;
    try
    {
        nCurType = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE);
        SQLUINTEGER nValueLen = m_pStatement->getCursorProperties(nCurType,false);
        if( (nValueLen & SQL_CA2_SENSITIVITY_DELETIONS) != SQL_CA2_SENSITIVITY_DELETIONS ||
            (nValueLen & SQL_CA2_CRC_EXACT) != SQL_CA2_CRC_EXACT)
            m_pSkipDeletedSet.reset( new OSkipDeletedSet(this) );
    }
    catch(const Exception&)
    { // we don't want our result destroy here
    }
    try
    {
        SQLUINTEGER nValueLen = 0;
        // Reference: http://msdn.microsoft.com/en-us/library/windows/desktop/ms715441%28v=vs.85%29.aspx
        // LibreOffice ODBC binds columns only on update, so we don't care about SQL_GD_ANY_COLUMN / SQL_GD_BOUND
        // TODO: maybe a problem if a column is updated, then an earlier column fetched?
        //       an updated column is bound...
        // TODO: aren't we assuming SQL_GD_OUTPUT_PARAMS?
        //       If yes, we should at least OSL_ENSURE it,
        //       even better throw an exception any OUT parameter registration if !SQL_GD_OUTPUT_PARAMS.
        // If !SQL_GD_ANY_ORDER, cache the whole row so that callers can access columns in any order.
        // In other words, isolate them from ODBC restrictions.
        // TODO: we assume SQL_GD_BLOCK, unless fetchSize is 1
        OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_GETDATA_EXTENSIONS,nValueLen,nullptr);
        m_bFetchDataInOrder = ((SQL_GD_ANY_ORDER & nValueLen) != SQL_GD_ANY_ORDER);
    }
    catch(const Exception&)
    {
        m_bFetchDataInOrder = true;
    }
    try
    {
        // TODO: this does *not* do what it appears.
        //       We use SQLFetchScroll unconditionally in several places
        //       the *only* difference this makes is whether ::next() uses SQLFetchScroll or SQLFetch
        //       so this test seems pointless
        if ( getOdbcFunction(ODBC3SQLFunctionId::GetFunctions) )
        {
            SQLUSMALLINT nSupported = 0;
            m_bUseFetchScroll = ( N3SQLGetFunctions(m_aConnectionHandle,SQL_API_SQLFETCHSCROLL,&nSupported) == SQL_SUCCESS && nSupported == 1 );
        }
    }
    catch(const Exception&)
    {
        m_bUseFetchScroll = false;
    }

    osl_atomic_decrement( &m_refCount );
}

OResultSet::~OResultSet()
{
}

void OResultSet::construct()
{
    osl_atomic_increment( &m_refCount );
    allocBuffer();
    osl_atomic_decrement( &m_refCount );
}

void OResultSet::disposing()
{
    N3SQLCloseCursor(m_aStatementHandle);
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    releaseBuffer();

    setStmtOption<SQLUSMALLINT*, SQL_IS_POINTER>(SQL_ATTR_ROW_STATUS_PTR, nullptr);
    m_xStatement.clear();
    m_xMetaData.clear();
}

SQLRETURN OResultSet::unbind(bool _bUnbindHandle)
{
    SQLRETURN nRet = 0;
    if ( _bUnbindHandle )
        nRet = N3SQLFreeStmt(m_aStatementHandle,SQL_UNBIND);

    if ( !m_aBindVector.empty() )
    {
        for(auto& [rPtrAddr, rType] : m_aBindVector)
        {
            switch (rType)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                    delete static_cast< OString* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::BIGINT:
                    delete static_cast< sal_Int64* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    delete static_cast< OString* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                    delete static_cast< double* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::LONGVARCHAR:
                case DataType::CLOB:
                    delete [] static_cast< char* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::LONGVARBINARY:
                case DataType::BLOB:
                    delete [] static_cast< char* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::DATE:
                    delete static_cast< DATE_STRUCT* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::TIME:
                    delete static_cast< TIME_STRUCT* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::TIMESTAMP:
                    delete static_cast< TIMESTAMP_STRUCT* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::BIT:
                case DataType::TINYINT:
                    delete static_cast< sal_Int8* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::SMALLINT:
                    delete static_cast< sal_Int16* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::INTEGER:
                    delete static_cast< sal_Int32* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::FLOAT:
                    delete static_cast< float* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
                case DataType::BINARY:
                case DataType::VARBINARY:
                    delete static_cast< sal_Int8* >(reinterpret_cast< void * >(rPtrAddr));
                    break;
            }
        }
        m_aBindVector.clear();
    }
    return nRet;
}

TVoidPtr OResultSet::allocBindColumn(sal_Int32 _nType,sal_Int32 _nColumnIndex)
{
    TVoidPtr aPair;
    switch (_nType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new OString()),_nType);
            break;
        case DataType::BIGINT:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new sal_Int64(0)),_nType);
            break;
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new OString()),_nType);
            break;
        case DataType::REAL:
        case DataType::DOUBLE:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new double(0.0)),_nType);
            break;
        case DataType::LONGVARCHAR:
        case DataType::CLOB:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new char[2]),_nType);  // only for finding
            break;
        case DataType::LONGVARBINARY:
        case DataType::BLOB:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new char[2]),_nType);  // only for finding
            break;
        case DataType::DATE:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new DATE_STRUCT),_nType);
            break;
        case DataType::TIME:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new TIME_STRUCT),_nType);
            break;
        case DataType::TIMESTAMP:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new TIMESTAMP_STRUCT),_nType);
            break;
        case DataType::BIT:
        case DataType::TINYINT:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new sal_Int8(0)),_nType);
            break;
        case DataType::SMALLINT:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new sal_Int16(0)),_nType);
            break;
        case DataType::INTEGER:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new sal_Int32(0)),_nType);
            break;
        case DataType::FLOAT:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new float(0)),_nType);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
            aPair = TVoidPtr(reinterpret_cast< sal_Int64 >(new sal_Int8[m_aRow[_nColumnIndex].getSequence().getLength()]),_nType);
            break;
        default:
            SAL_WARN( "connectivity.odbc", "Unknown type");
            aPair = TVoidPtr(0,_nType);
    }
    return aPair;
}

void OResultSet::allocBuffer()
{
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();

    m_aBindVector.reserve(nLen);
    m_aRow.resize(nLen+1);

    m_aRow[0].setTypeKind(DataType::VARBINARY);
    m_aRow[0].setBound( false );

    for(sal_Int32 i = 1;i<=nLen;++i)
    {
        sal_Int32 nType = xMeta->getColumnType(i);
        m_aRow[i].setTypeKind( nType );
        m_aRow[i].setBound( false );
    }
    m_aLengthVector.resize(nLen + 1);
}

void OResultSet::releaseBuffer()
{
    unbind(false);
    m_aLengthVector.clear();
}

Any SAL_CALL OResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OResultSet::getTypes(  )
{
    OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                            cppu::UnoType<css::beans::XFastPropertySet>::get(),
                            cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}


sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName )
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
    {
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

void OResultSet::ensureCacheForColumn(sal_Int32 columnIndex)
{
    SAL_INFO( "connectivity.odbc", "odbc  lionel@mamane.lu OResultSet::ensureCacheForColumn" );

    assert(columnIndex >= 0);

    const TDataRow::size_type oldCacheSize = m_aRow.size();
    const TDataRow::size_type uColumnIndex = static_cast<TDataRow::size_type>(columnIndex);

    if (oldCacheSize > uColumnIndex)
        // nothing to do
        return;

    m_aRow.resize(columnIndex + 1);
    TDataRow::iterator i (m_aRow.begin() + oldCacheSize);
    const TDataRow::const_iterator end(m_aRow.end());
    for (; i != end; ++i)
    {
        i->setBound(false);
    }
}
void OResultSet::invalidateCache()
{
    for(auto& rItem : m_aRow)
    {
        rItem.setBound(false);
    }
}

Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBinaryStream", *this );

    return nullptr;
}

Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBinaryStream", *this );

    return nullptr;
}

template < typename T > T OResultSet::impl_getValue( const sal_Int32 _nColumnIndex, SQLSMALLINT nType )
{
    T val;

    OTools::getValue(m_pStatement->getOwnConnection(), m_aStatementHandle, _nColumnIndex, nType, m_bWasNull, **this, &val, sizeof(val));

    return val;
}

// this function exists for the implicit conversion to sal_Bool (compared to a direct call to impl_getValue)
bool OResultSet::impl_getBoolean( sal_Int32 columnIndex )
{
    return impl_getValue<sal_Int8>(columnIndex, SQL_C_BIT);
}

template < typename T > T OResultSet::getValue( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    fillColumn(columnIndex);
    m_bWasNull = m_aRow[columnIndex].isNull();
    return m_aRow[columnIndex];
}
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex )
{
    return getValue<bool>( columnIndex );
}

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex )
{
    return getValue<sal_Int8>( columnIndex );
}


Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    fillColumn(columnIndex);
    m_bWasNull = m_aRow[columnIndex].isNull();

    Sequence< sal_Int8 > nRet;
    switch(m_aRow[columnIndex].getTypeKind())
    {
    case DataType::BINARY:
    case DataType::VARBINARY:
    case DataType::LONGVARBINARY:
        nRet = m_aRow[columnIndex];
        break;
    default:
    {
        OUString const & sRet = m_aRow[columnIndex].getString();
        nRet = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(sRet.getStr()),sizeof(sal_Unicode)*sRet.getLength());
    }
    }
    return nRet;
}
Sequence< sal_Int8 > OResultSet::impl_getBytes( sal_Int32 columnIndex )
{
    const SWORD nColumnType = impl_getColumnType_nothrow(columnIndex);

    switch(nColumnType)
    {
    case SQL_WVARCHAR:
    case SQL_WCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_VARCHAR:
    case SQL_CHAR:
    case SQL_LONGVARCHAR:
    {
        OUString const & aRet = OTools::getStringValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,nColumnType,m_bWasNull,**this,m_nTextEncoding);
        return Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),sizeof(sal_Unicode)*aRet.getLength());
    }
    default:
        return OTools::getBytesValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,SQL_C_BINARY,m_bWasNull,**this);
    }
}

Date OResultSet::impl_getDate( sal_Int32 columnIndex )
{
    DATE_STRUCT aDate = impl_getValue< DATE_STRUCT> ( columnIndex,
                                                      m_pStatement->getOwnConnection()->useOldDateFormat() ? SQL_C_DATE : SQL_C_TYPE_DATE  );

    return Date(aDate.day, aDate.month, aDate.year);
}

Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex )
{
    return getValue<Date>( columnIndex );
}


double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex )
{
    return getValue<double>( columnIndex );
}


float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex )
{
    return getValue<float>( columnIndex );
}

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex )
{
    return getValue<sal_Int16>( columnIndex );
}

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex )
{
    return getValue<sal_Int32>( columnIndex );
}

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex )
{
    return getValue<sal_Int64>( columnIndex );
}
sal_Int64 OResultSet::impl_getLong( sal_Int32 columnIndex )
{
    try
    {
        return impl_getValue<sal_Int64>(columnIndex, SQL_C_SBIGINT);
    }
    catch(const SQLException&)
    {
        return getString(columnIndex).toInt64();
    }
}

sal_Int32 SAL_CALL OResultSet::getRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_pSkipDeletedSet ? m_pSkipDeletedSet->getMappedPosition(getDriverPos()) : getDriverPos();
}

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pStatement->getOwnConnection(),m_aStatementHandle);
    return m_xMetaData;
}

Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getArray", *this );
    return nullptr;
}


Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getClob", *this );
    return nullptr;
}

Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getBlob", *this );
    return nullptr;
}


Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRow::getRef", *this );
    return nullptr;
}


Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    return getValue<ORowSetValue>( columnIndex ).makeAny();
}

OUString OResultSet::impl_getString( sal_Int32 columnIndex )
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    const SWORD nColumnType = impl_getColumnType_nothrow(columnIndex);
    return OTools::getStringValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,nColumnType,m_bWasNull,**this,m_nTextEncoding);
}
OUString OResultSet::getString( sal_Int32 columnIndex )
{
    return getValue<OUString>( columnIndex );
}

Time OResultSet::impl_getTime( sal_Int32 columnIndex )
{
    TIME_STRUCT aTime = impl_getValue< TIME_STRUCT > ( columnIndex,
                                                      m_pStatement->getOwnConnection()->useOldDateFormat() ? SQL_C_TIME : SQL_C_TYPE_TIME );

    return Time(0, aTime.second,aTime.minute,aTime.hour, false);
}
Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex )
{
    return getValue<Time>( columnIndex );
}

DateTime OResultSet::impl_getTimestamp( sal_Int32 columnIndex )
{
    TIMESTAMP_STRUCT aTime = impl_getValue< TIMESTAMP_STRUCT > ( columnIndex,
                                                                 m_pStatement->getOwnConnection()->useOldDateFormat() ? SQL_C_TIMESTAMP : SQL_C_TYPE_TIMESTAMP );

    return DateTime(aTime.fraction,
                    aTime.second,
                    aTime.minute,
                    aTime.hour,
                    aTime.day,
                    aTime.month,
                    aTime.year,
                    false);
}
DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex )
{
    return getValue<DateTime>( columnIndex );
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_nRowPos == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPos != 0 && m_nCurrentFetchState == SQL_NO_DATA;
}

sal_Bool SAL_CALL OResultSet::isFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPos == 1;
}

sal_Bool SAL_CALL OResultSet::isLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bEOF && m_nCurrentFetchState != SQL_NO_DATA;
}

void SAL_CALL OResultSet::beforeFirst(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        previous();
    m_nCurrentFetchState = SQL_SUCCESS;
}

void SAL_CALL OResultSet::afterLast(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(last())
        next();
    m_bEOF = true;
}


void SAL_CALL OResultSet::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}


sal_Bool SAL_CALL OResultSet::first(  )
{
    return moveImpl(IResultSetHelper::FIRST,0);
}


sal_Bool SAL_CALL OResultSet::last(  )
{
    return moveImpl(IResultSetHelper::LAST,0);
}

sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row )
{
    return moveImpl(IResultSetHelper::ABSOLUTE1,row);
}

sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row )
{
    return moveImpl(IResultSetHelper::RELATIVE1,row);
}

sal_Bool SAL_CALL OResultSet::previous(  )
{
    return moveImpl(IResultSetHelper::PRIOR,0);
}

Reference< XInterface > SAL_CALL OResultSet::getStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return m_xStatement;
}


sal_Bool SAL_CALL OResultSet::rowDeleted()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    bool bRet = m_bRowDeleted;
    m_bRowDeleted = false;

    return bRet;
}

sal_Bool SAL_CALL OResultSet::rowInserted(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    bool bInserted = m_bRowInserted;
    m_bRowInserted = false;

    return bInserted;
}

sal_Bool SAL_CALL OResultSet::rowUpdated(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_pRowStatusArray[0] == SQL_ROW_UPDATED;
}


sal_Bool SAL_CALL OResultSet::next(  )
{
    return moveImpl(IResultSetHelper::NEXT,1);
}


sal_Bool SAL_CALL OResultSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}


void SAL_CALL OResultSet::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    N3SQLCancel(m_aStatementHandle);
}

void SAL_CALL OResultSet::clearWarnings(  )
{
}

Any SAL_CALL OResultSet::getWarnings(  )
{
    return Any();
}

void SAL_CALL OResultSet::insertRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    SQLLEN nRealLen = 0;
    Sequence<sal_Int8> aBookmark(nMaxBookmarkLen);
    static_assert(static_cast<size_t>(nMaxBookmarkLen) >= sizeof(SQLLEN), "must be larger");

    SQLRETURN nRet = N3SQLBindCol(m_aStatementHandle,
                                0,
                                SQL_C_VARBOOKMARK,
                                aBookmark.getArray(),
                                nMaxBookmarkLen,
                                &nRealLen
                                );

    bool bPositionByBookmark = ( nullptr != getOdbcFunction( ODBC3SQLFunctionId::BulkOperations ) );
    if ( bPositionByBookmark )
    {
        nRet = N3SQLBulkOperations( m_aStatementHandle, SQL_ADD );
        fillNeededData( nRet );
    }
    else
    {
        if(isBeforeFirst())
            next(); // must be done
        nRet = N3SQLSetPos( m_aStatementHandle, 1, SQL_ADD, SQL_LOCK_NO_CHANGE );
        fillNeededData( nRet );
    }
    aBookmark.realloc(nRealLen);
    try
    {
        OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    }
    catch(const SQLException&)
    {
        nRet = unbind();
        throw;
    }

    nRet = unbind();
    OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    if ( bPositionByBookmark )
    {
        setStmtOption<SQLLEN*, SQL_IS_POINTER>(SQL_ATTR_FETCH_BOOKMARK_PTR, reinterpret_cast<SQLLEN*>(aBookmark.getArray()));

        nRet = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_BOOKMARK,0);
    }
    else
        nRet = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,0); // OJ 06.03.2004
    // sometimes we got an error but we are not interested in anymore #106047# OJ
    //  OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    if(m_pSkipDeletedSet)
    {
        if(moveToBookmark(makeAny(aBookmark)))
        {
            sal_Int32 nRowPos = getDriverPos();
            if ( -1 == m_nRowPos )
            {
                nRowPos = m_aPosToBookmarks.size() + 1;
            }
            if ( nRowPos == m_nRowPos )
                ++nRowPos;
            m_nRowPos = nRowPos;
            m_pSkipDeletedSet->insertNewPosition(nRowPos);
            m_aPosToBookmarks[aBookmark] = nRowPos;
        }
    }
    m_bRowInserted = true;

}

void SAL_CALL OResultSet::updateRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    SQLRETURN nRet;

    try
    {
        bool bPositionByBookmark = ( nullptr != getOdbcFunction( ODBC3SQLFunctionId::BulkOperations ) );
        if ( bPositionByBookmark )
        {
            getBookmark();
            assert(m_aRow[0].isBound());
            Sequence<sal_Int8> aBookmark(m_aRow[0].getSequence());
            SQLLEN nRealLen = aBookmark.getLength();
            nRet = N3SQLBindCol(m_aStatementHandle,
                                0,
                                SQL_C_VARBOOKMARK,
                                aBookmark.getArray(),
                                aBookmark.getLength(),
                                &nRealLen
                                );
            OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
            nRet = N3SQLBulkOperations(m_aStatementHandle, SQL_UPDATE_BY_BOOKMARK);
            fillNeededData(nRet);
            // the driver should not have touched this
            // (neither the contents of aBookmark FWIW)
            assert(nRealLen == aBookmark.getLength());
        }
        else
        {
            nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
            fillNeededData(nRet);
        }
        OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
        // unbind all columns so we can fetch all columns again with SQLGetData
        // (and also so that our buffers don't clobber anything, and
        //  so that a subsequent fetch does not overwrite m_aRow[0])
        invalidateCache();
        nRet = unbind();
        OSL_ENSURE(nRet == SQL_SUCCESS,"ODBC insert could not unbind the columns after success");
    }
    catch(...)
    {
        // unbind all columns so that a subsequent fetch does not overwrite m_aRow[0]
        nRet = unbind();
        OSL_ENSURE(nRet == SQL_SUCCESS,"ODBC insert could not unbind the columns after failure");
        throw;
    }
}

void SAL_CALL OResultSet::deleteRow(  )
{
    SQLRETURN nRet = SQL_SUCCESS;
    sal_Int32 nPos = getDriverPos();
    nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_bRowDeleted = ( m_pRowStatusArray[0] == SQL_ROW_DELETED );
    if ( m_bRowDeleted )
    {
        TBookmarkPosMap::iterator aIter = std::find_if(m_aPosToBookmarks.begin(), m_aPosToBookmarks.end(),
            [&nPos](const TBookmarkPosMap::value_type& rEntry) { return rEntry.second == nPos; });
        if (aIter != m_aPosToBookmarks.end())
            m_aPosToBookmarks.erase(aIter);
    }
    if ( m_pSkipDeletedSet )
        m_pSkipDeletedSet->deletePosition(nPos);
}


void SAL_CALL OResultSet::cancelRowUpdates(  )
{
}


void SAL_CALL OResultSet::moveToInsertRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    invalidateCache();
    // first unbound all columns
    OSL_VERIFY( unbind() == SQL_SUCCESS );
    //  SQLRETURN nRet = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE ,(SQLPOINTER)1,SQL_IS_INTEGER);
}


void SAL_CALL OResultSet::moveToCurrentRow(  )
{
    invalidateCache();
}

void OResultSet::updateValue(sal_Int32 columnIndex, SQLSMALLINT _nType, void const * _pValue)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_aBindVector.push_back(allocBindColumn(OTools::MapOdbcType2Jdbc(_nType),columnIndex));
    void* pData = reinterpret_cast<void*>(m_aBindVector.rbegin()->first);
    OSL_ENSURE(pData != nullptr,"Data for update is NULL!");
    OTools::bindValue(  m_pStatement->getOwnConnection(),
                        m_aStatementHandle,
                        columnIndex,
                        _nType,
                        0,
                        _pValue,
                        pData,
                        &m_aLengthVector[columnIndex],
                        **this,
                        m_nTextEncoding,
                        m_pStatement->getOwnConnection()->useOldDateFormat());
}

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_aBindVector.push_back(allocBindColumn(DataType::CHAR,columnIndex));
    void* pData = reinterpret_cast<void*>(m_aBindVector.rbegin()->first);
    OTools::bindValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,SQL_CHAR,0,nullptr,pData,&m_aLengthVector[columnIndex],**this,m_nTextEncoding,m_pStatement->getOwnConnection()->useOldDateFormat());
}


void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x )
{
    updateValue(columnIndex,SQL_BIT,&x);
}

void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    updateValue(columnIndex,SQL_CHAR,&x);
}


void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    updateValue(columnIndex,SQL_TINYINT,&x);
}

void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    updateValue(columnIndex,SQL_INTEGER,&x);
}

void SAL_CALL OResultSet::updateLong( sal_Int32 /*columnIndex*/, sal_Int64 /*x*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRowUpdate::updateLong", *this );
}

void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x )
{
    updateValue(columnIndex,SQL_REAL,&x);
}


void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x )
{
    updateValue(columnIndex,SQL_DOUBLE,&x);
}

void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    sal_Int32 nType = m_aRow[columnIndex].getTypeKind();
    SQLSMALLINT nOdbcType = OTools::jdbcTypeToOdbc(nType);
    m_aRow[columnIndex] = x;
    m_aRow[columnIndex].setTypeKind(nType); // OJ: otherwise longvarchar will be recognized by fillNeededData
    m_aRow[columnIndex].setBound(true);
    updateValue(columnIndex,nOdbcType, &x);
}

void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x )
{
    sal_Int32 nType = m_aRow[columnIndex].getTypeKind();
    SQLSMALLINT nOdbcType = OTools::jdbcTypeToOdbc(nType);
    m_aRow[columnIndex] = x;
    m_aRow[columnIndex].setTypeKind(nType); // OJ: otherwise longvarbinary will be recognized by fillNeededData
    m_aRow[columnIndex].setBound(true);
    updateValue(columnIndex,nOdbcType, &x);
}

void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const Date& x )
{
    DATE_STRUCT aVal = OTools::DateToOdbcDate(x);
    updateValue(columnIndex,SQL_DATE,&aVal);
}


void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    TIME_STRUCT aVal = OTools::TimeToOdbcTime(x);
    updateValue(columnIndex,SQL_TIME,&aVal);
}


void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const DateTime& x )
{
    TIMESTAMP_STRUCT aVal = OTools::DateTimeToTimestamp(x);
    updateValue(columnIndex,SQL_TIMESTAMP,&aVal);
}


void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length )
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}

void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length )
{
    updateBinaryStream(columnIndex,x,length);
}

void SAL_CALL OResultSet::refreshRow(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    //  SQLRETURN nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_REFRESH,SQL_LOCK_NO_CHANGE);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}

void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}


void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}

// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  )
{
    fillColumn(0);
    if(m_aRow[0].isNull())
        throw SQLException();
    return m_aRow[0].makeAny();
}
Sequence<sal_Int8> OResultSet::impl_getBookmark(  )
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    TBookmarkPosMap::const_iterator aFind = std::find_if(m_aPosToBookmarks.begin(),m_aPosToBookmarks.end(),
        [this] (const TBookmarkPosMap::value_type& bookmarkPos) {
            return bookmarkPos.second == m_nRowPos;
        });

    if ( aFind == m_aPosToBookmarks.end() )
    {
        if ( m_nUseBookmarks == ODBC_SQL_NOT_DEFINED )
        {
            m_nUseBookmarks = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS);
        }
        if(m_nUseBookmarks == SQL_UB_OFF)
            throw SQLException();

        Sequence<sal_Int8> bookmark = OTools::getBytesValue(m_pStatement->getOwnConnection(),m_aStatementHandle,0,SQL_C_VARBOOKMARK,m_bWasNull,**this);
        m_aPosToBookmarks[bookmark] = m_nRowPos;
        OSL_ENSURE(bookmark.getLength(),"Invalid bookmark from length 0!");
        return bookmark;
    }
    else
    {
        return aFind->first;
    }
}

sal_Bool SAL_CALL OResultSet::moveToBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    invalidateCache();
    Sequence<sal_Int8> aBookmark;
    bookmark >>= aBookmark;
    OSL_ENSURE(aBookmark.getLength(),"Invalid bookmark from length 0!");
    if(aBookmark.getLength())
    {
        SQLRETURN nReturn = setStmtOption<SQLLEN*, SQL_IS_POINTER>(SQL_ATTR_FETCH_BOOKMARK_PTR, reinterpret_cast<SQLLEN*>(aBookmark.getArray()));

        if ( SQL_INVALID_HANDLE != nReturn && SQL_ERROR != nReturn )
        {
            m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_BOOKMARK,0);
            OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
            TBookmarkPosMap::const_iterator aFind = m_aPosToBookmarks.find(aBookmark);
            if(aFind != m_aPosToBookmarks.end())
                m_nRowPos = aFind->second;
            else
                m_nRowPos = -1;
            return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
        }
    }
    return false;
}

sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    invalidateCache();
    Sequence<sal_Int8> aBookmark;
    bookmark >>= aBookmark;
    setStmtOption<SQLLEN*, SQL_IS_POINTER>(SQL_ATTR_FETCH_BOOKMARK_PTR, reinterpret_cast<SQLLEN*>(aBookmark.getArray()));

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_BOOKMARK,rows);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}

sal_Int32 SAL_CALL OResultSet::compareBookmarks( const Any& lhs, const  Any& rhs )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return (lhs == rhs) ? CompareBookmark::EQUAL : CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  )
{
    return false;
}

sal_Int32 SAL_CALL OResultSet::hashBookmark( const  Any& /*bookmark*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( "XRowLocate::hashBookmark", *this );
    return 0;
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const  Sequence<  Any >& rows )
{
    Sequence< sal_Int32 > aRet(rows.getLength());
    sal_Int32 *pRet = aRet.getArray();

    const Any *pBegin   = rows.getConstArray();
    const Any *pEnd     = pBegin + rows.getLength();

    for(;pBegin != pEnd;++pBegin,++pRet)
    {
        try
        {
            if(moveToBookmark(*pBegin))
            {
                deleteRow();
                *pRet = 1;
            }
        }
        catch(const SQLException&)
        {
            *pRet = 0;
        }
    }
    return aRet;
}

template < typename T, SQLINTEGER BufferLength > T OResultSet::getStmtOption (SQLINTEGER fOption) const
{
    T result (0);
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLGetStmtAttr(m_aStatementHandle, fOption, &result, BufferLength, nullptr);
    return result;
}
template < typename T, SQLINTEGER BufferLength > SQLRETURN OResultSet::setStmtOption (SQLINTEGER fOption, T value) const
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    SQLPOINTER sv = reinterpret_cast<SQLPOINTER>(value);
    return N3SQLSetStmtAttr(m_aStatementHandle, fOption, sv, BufferLength);
}

sal_Int32 OResultSet::getResultSetConcurrency() const
{
    sal_uInt32 nValue = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CONCURRENCY);
    if(SQL_CONCUR_READ_ONLY == nValue)
        nValue = ResultSetConcurrency::READ_ONLY;
    else
        nValue = ResultSetConcurrency::UPDATABLE;

    return nValue;
}

sal_Int32 OResultSet::getResultSetType() const
{
    sal_uInt32 nValue = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_SENSITIVITY);
    if(SQL_SENSITIVE == nValue)
        nValue = ResultSetType::SCROLL_SENSITIVE;
    else if(SQL_INSENSITIVE == nValue)
        nValue = ResultSetType::SCROLL_INSENSITIVE;
    else
    {
        SQLULEN nCurType = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE);
        if(SQL_CURSOR_KEYSET_DRIVEN == nCurType)
            nValue = ResultSetType::SCROLL_SENSITIVE;
        else if(SQL_CURSOR_STATIC  == nCurType)
            nValue = ResultSetType::SCROLL_INSENSITIVE;
        else if(SQL_CURSOR_FORWARD_ONLY == nCurType)
            nValue = ResultSetType::FORWARD_ONLY;
        else if(SQL_CURSOR_DYNAMIC == nCurType)
            nValue = ResultSetType::SCROLL_SENSITIVE;
    }
    return nValue;
}

sal_Int32 OResultSet::getFetchSize() const
{
    return getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_ARRAY_SIZE);
}

OUString OResultSet::getCursorName() const
{
    SQLCHAR pName[258];
    SQLSMALLINT nRealLen = 0;
    N3SQLGetCursorName(m_aStatementHandle,pName,256,&nRealLen);
    return OUString::createFromAscii(reinterpret_cast<char*>(pName));
}

bool  OResultSet::isBookmarkable() const
{
    if(!m_aConnectionHandle)
        return false;

    const SQLULEN nCursorType = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE);

    sal_Int32 nAttr = 0;
    try
    {
        switch(nCursorType)
        {
        case SQL_CURSOR_FORWARD_ONLY:
            return false;
        case SQL_CURSOR_STATIC:
            OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_STATIC_CURSOR_ATTRIBUTES1,nAttr,nullptr);
            break;
        case SQL_CURSOR_KEYSET_DRIVEN:
            OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_KEYSET_CURSOR_ATTRIBUTES1,nAttr,nullptr);
            break;
        case SQL_CURSOR_DYNAMIC:
            OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nAttr,nullptr);
            break;
        }
    }
    catch(const Exception&)
    {
        return false;
    }

    if ( m_nUseBookmarks == ODBC_SQL_NOT_DEFINED )
    {
        m_nUseBookmarks = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS);
    }

    return (m_nUseBookmarks != SQL_UB_OFF) && (nAttr & SQL_CA1_BOOKMARK) == SQL_CA1_BOOKMARK;
}

void OResultSet::setFetchDirection(sal_Int32 _par0)
{
    ::dbtools::throwFunctionNotSupportedSQLException( "setFetchDirection", *this );

    OSL_ENSURE(_par0>0,"Illegal fetch direction!");
    if ( _par0 > 0 )
    {
        setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_CURSOR_TYPE, _par0);
    }
}

void OResultSet::setFetchSize(sal_Int32 _par0)
{
    OSL_ENSURE(_par0>0,"Illegal fetch size!");
    if ( _par0 != 1 )
    {
        throw css::beans::PropertyVetoException("SDBC/ODBC layer not prepared for fetchSize > 1", *this);
    }
    setStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_ARRAY_SIZE, _par0);
    m_pRowStatusArray.reset( new SQLUSMALLINT[_par0] );
    setStmtOption<SQLUSMALLINT*, SQL_IS_POINTER>(SQL_ATTR_ROW_STATUS_PTR, m_pRowStatusArray.get());
}

IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
        PROPERTY_ID_ISBOOKMARKABLE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return false;
}

void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(getINT32(rValue));
            break;
        default:
            ;
    }
}

void OResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            rValue <<= isBookmarkable();
            break;
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}

void OResultSet::fillColumn(const sal_Int32 _nColumn)
{
    ensureCacheForColumn(_nColumn);

    if (m_aRow[_nColumn].isBound())
        return;

    sal_Int32 curCol;
    if(m_bFetchDataInOrder)
    {
        // m_aRow necessarily has a prefix of bound values, then all unbound values
        // EXCEPT for column 0
        // so use binary search to find the earliest unbound value before or at _nColumn
        sal_Int32 lower=0;
        sal_Int32 upper=_nColumn;

        while (lower < upper)
        {
            const sal_Int32 middle=(upper-lower)/2 + lower;
            if(m_aRow[middle].isBound())
            {
                lower=middle+1;
            }
            else
            {
                upper=middle;
            }
        }

        curCol = upper;
    }
    else
    {
        curCol = _nColumn;
    }

    TDataRow::iterator pColumn      = m_aRow.begin() + curCol;
    const TDataRow::const_iterator pColumnEnd   = m_aRow.begin() + _nColumn + 1;

    if(curCol==0)
    {
        try
        {
            *pColumn=impl_getBookmark();
        }
        catch (SQLException &)
        {
            pColumn->setNull();
        }
        pColumn->setBound(true);
        ++curCol;
        ++pColumn;
    }

    for (; pColumn != pColumnEnd; ++curCol, ++pColumn)
    {
        const sal_Int32 nType = pColumn->getTypeKind();
        switch (nType)
        {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DECIMAL:
        case DataType::NUMERIC:
        case DataType::LONGVARCHAR:
        case DataType::CLOB:
            *pColumn=impl_getString(curCol);
            break;
        case DataType::FLOAT:
            *pColumn = impl_getValue<float>(curCol, SQL_C_FLOAT);
            break;
        case DataType::REAL:
        case DataType::DOUBLE:
            *pColumn = impl_getValue<double>(curCol, SQL_C_DOUBLE);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
        case DataType::BLOB:
            *pColumn = impl_getBytes(curCol);
            break;
        case DataType::DATE:
            *pColumn = impl_getDate(curCol);
            break;
        case DataType::TIME:
            *pColumn = impl_getTime(curCol);
            break;
        case DataType::TIMESTAMP:
            *pColumn = impl_getTimestamp(curCol);
            break;
        case DataType::BIT:
            *pColumn = impl_getBoolean(curCol);
            break;
        case DataType::TINYINT:
            *pColumn = impl_getValue<sal_Int8>(curCol, SQL_C_TINYINT);
            break;
        case DataType::SMALLINT:
            *pColumn = impl_getValue<sal_Int16>(curCol, SQL_C_SHORT);
            break;
        case DataType::INTEGER:
            *pColumn = impl_getValue<sal_Int32>(curCol, SQL_C_LONG);
            break;
        case DataType::BIGINT:
            *pColumn = impl_getLong(curCol);
            break;
        default:
            SAL_WARN( "connectivity.odbc","Unknown DataType");
        }

        if ( m_bWasNull )
            pColumn->setNull();
        pColumn->setBound(true);
        if(nType != pColumn->getTypeKind())
        {
            pColumn->setTypeKind(nType);
        }
    }
}

void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}

void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

bool OResultSet::move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool /*_bRetrieveData*/)
{
    SQLSMALLINT nFetchOrientation = SQL_FETCH_NEXT;
    switch(_eCursorPosition)
    {
        case IResultSetHelper::NEXT:
            nFetchOrientation = SQL_FETCH_NEXT;
            break;
        case IResultSetHelper::PRIOR:
            nFetchOrientation = SQL_FETCH_PRIOR;
            break;
        case IResultSetHelper::FIRST:
            nFetchOrientation = SQL_FETCH_FIRST;
            break;
        case IResultSetHelper::LAST:
            nFetchOrientation = SQL_FETCH_LAST;
            break;
        case IResultSetHelper::RELATIVE1:
            nFetchOrientation = SQL_FETCH_RELATIVE;
            break;
        case IResultSetHelper::ABSOLUTE1:
            nFetchOrientation = SQL_FETCH_ABSOLUTE;
            break;
        case IResultSetHelper::BOOKMARK: // special case here because we are only called with position numbers
        {
            TBookmarkPosMap::const_iterator aIter = std::find_if(m_aPosToBookmarks.begin(), m_aPosToBookmarks.end(),
                [&_nOffset](const TBookmarkPosMap::value_type& rEntry) { return rEntry.second == _nOffset; });
            if (aIter != m_aPosToBookmarks.end())
                return moveToBookmark(makeAny(aIter->first));
            SAL_WARN( "connectivity.odbc", "Bookmark not found!");
        }
        return false;
    }

    m_bEOF = false;
    invalidateCache();

    SQLRETURN nOldFetchStatus = m_nCurrentFetchState;
    // TODO FIXME: both of these will misbehave for
    // _eCursorPosition == IResultSetHelper::NEXT/PREVIOUS
    // when fetchSize > 1
    if ( !m_bUseFetchScroll && _eCursorPosition == IResultSetHelper::NEXT )
        m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    else
        m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,nFetchOrientation,_nOffset);

    SAL_INFO(
        "connectivity.odbc",
        "move(" << nFetchOrientation << "," << _nOffset << "), FetchState = "
            << m_nCurrentFetchState);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    const bool bSuccess = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if ( bSuccess )
    {
        switch(_eCursorPosition)
        {
            case IResultSetHelper::NEXT:
                ++m_nRowPos;
                break;
            case IResultSetHelper::PRIOR:
                --m_nRowPos;
                break;
            case IResultSetHelper::FIRST:
                m_nRowPos = 1;
                break;
            case IResultSetHelper::LAST:
                m_bEOF = true;
                break;
            case IResultSetHelper::RELATIVE1:
                m_nRowPos += _nOffset;
                break;
            case IResultSetHelper::ABSOLUTE1:
            case IResultSetHelper::BOOKMARK: // special case here because we are only called with position numbers
                m_nRowPos = _nOffset;
                break;
        } // switch(_eCursorPosition)
        if ( m_nUseBookmarks == ODBC_SQL_NOT_DEFINED )
        {
            m_nUseBookmarks = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_USE_BOOKMARKS);
        }
        if ( m_nUseBookmarks == SQL_UB_OFF )
        {
            m_aRow[0].setNull();
        }
        else
        {
            ensureCacheForColumn(0);
            Sequence<sal_Int8> bookmark  = OTools::getBytesValue(m_pStatement->getOwnConnection(),m_aStatementHandle,0,SQL_C_VARBOOKMARK,m_bWasNull,**this);
            m_aPosToBookmarks[bookmark] = m_nRowPos;
            OSL_ENSURE(bookmark.getLength(),"Invalid bookmark from length 0!");
            m_aRow[0] = bookmark;
        }
        m_aRow[0].setBound(true);
    }
    else if ( IResultSetHelper::PRIOR == _eCursorPosition && m_nCurrentFetchState == SQL_NO_DATA )
        // we went beforeFirst
        m_nRowPos = 0;
    else if(IResultSetHelper::NEXT == _eCursorPosition && m_nCurrentFetchState == SQL_NO_DATA && nOldFetchStatus != SQL_NO_DATA)
        // we went afterLast
        ++m_nRowPos;

    return bSuccess;
}

sal_Int32 OResultSet::getDriverPos() const
{
    sal_Int32 nValue = getStmtOption<SQLULEN, SQL_IS_UINTEGER>(SQL_ATTR_ROW_NUMBER);
    SAL_INFO(
        "connectivity.odbc",
        "RowNum = " << nValue << ", RowPos = " << m_nRowPos);
    return nValue ? nValue : m_nRowPos;
}

bool OResultSet::isRowDeleted() const
{
    return m_pRowStatusArray[0] == SQL_ROW_DELETED;
}

bool OResultSet::moveImpl(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    return (m_pSkipDeletedSet != nullptr)
                ?   m_pSkipDeletedSet->skipDeleted(_eCursorPosition,_nOffset,true/*_bRetrieveData*/)
                :   move(_eCursorPosition,_nOffset,true/*_bRetrieveData*/);
}

void OResultSet::fillNeededData(SQLRETURN _nRet)
{
    SQLRETURN nRet = _nRet;
    if( nRet == SQL_NEED_DATA)
    {
        void* pColumnIndex = nullptr;
        nRet = N3SQLParamData(m_aStatementHandle,&pColumnIndex);

        do
        {
            if (nRet != SQL_SUCCESS && nRet != SQL_SUCCESS_WITH_INFO && nRet != SQL_NEED_DATA)
                break;

            sal_IntPtr nColumnIndex ( reinterpret_cast<sal_IntPtr>(pColumnIndex));
            Sequence< sal_Int8 > aSeq;
            switch(m_aRow[nColumnIndex].getTypeKind())
            {
                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::BLOB:
                    aSeq = m_aRow[nColumnIndex];
                    N3SQLPutData (m_aStatementHandle, aSeq.getArray(), aSeq.getLength());
                    break;
                case SQL_WLONGVARCHAR:
                {
                    OUString const & sRet = m_aRow[nColumnIndex].getString();
                    N3SQLPutData (m_aStatementHandle, static_cast<SQLPOINTER>(const_cast<sal_Unicode *>(sRet.getStr())), sizeof(sal_Unicode)*sRet.getLength());
                    break;
                }
                case DataType::LONGVARCHAR:
                case DataType::CLOB:
                {
                    OUString sRet;
                    sRet = m_aRow[nColumnIndex].getString();
                    OString aString(OUStringToOString(sRet,m_nTextEncoding));
                    N3SQLPutData (m_aStatementHandle, static_cast<SQLPOINTER>(const_cast<char *>(aString.getStr())), aString.getLength());
                    break;
                }
                default:
                    SAL_WARN( "connectivity.odbc", "Not supported at the moment!");
            }
            nRet = N3SQLParamData(m_aStatementHandle,&pColumnIndex);
        }
        while (nRet == SQL_NEED_DATA);
    }
}

SWORD OResultSet::impl_getColumnType_nothrow(sal_Int32 columnIndex)
{
    std::map<sal_Int32,SWORD>::const_iterator aFind = m_aODBCColumnTypes.find(columnIndex);
    if ( aFind == m_aODBCColumnTypes.end() )
        aFind = m_aODBCColumnTypes.emplace(
                           columnIndex,
                           OResultSetMetaData::getColumnODBCType(m_pStatement->getOwnConnection(),m_aStatementHandle,*this,columnIndex)
                        ).first;
    return aFind->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
