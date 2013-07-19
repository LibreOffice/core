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

#include "TConnection.hxx"

#include "odbc/ODatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include "odbc/OResultSetMetaData.hxx"
#include "odbc/OTools.hxx"
#include <comphelper/types.hxx>
#include "FDatabaseMetaDataResultSetMetaData.hxx"
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;


using namespace connectivity::odbc;
using namespace cppu;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(OConnection* _pConnection)
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)

    ,m_aStatementHandle(_pConnection->createStatementHandle())
    ,m_aStatement(NULL)
    ,m_xMetaData(NULL)
    ,m_pRowStatusArray(NULL)
    ,m_pConnection(_pConnection)
    ,m_nTextEncoding(_pConnection->getTextEncoding())
    ,m_nRowPos(-1)
    ,m_nDriverColumnCount(0)
    ,m_nCurrentFetchState(0)
    ,m_bWasNull(sal_True)
    ,m_bEOF(sal_False)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet: No parent set!");
    if( SQL_NULL_HANDLE == m_aStatementHandle )
        throw RuntimeException();

    osl_atomic_increment( &m_refCount );
    m_pConnection->acquire();
    m_pRowStatusArray = new SQLUSMALLINT[1]; // the default value
    osl_atomic_decrement( &m_refCount );
    //  allocBuffer();
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    OSL_ENSURE(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed,"Object wasn't disposed!");
    if(!ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
    {
        osl_atomic_increment( &m_refCount );
        dispose();
    }
    delete [] m_pRowStatusArray;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_pConnection->freeStatementHandle(m_aStatementHandle);

    m_aStatement    = NULL;
m_xMetaData.clear();
    m_pConnection->release();
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}
// -----------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::acquire() throw()
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::release() throw()
{
    ODatabaseMetaDataResultSet_BASE::release();
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}
// -----------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::mapColumn (sal_Int32  column)
{
    sal_Int32   map = column;

    if (!m_aColMapping.empty())
    {
        // Validate column number
        map = m_aColMapping[column];
    }

    return map;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    /* FIXME should throw in case of not found ? or at least return -1 */
    return i;
}

template < typename T, SQLSMALLINT sqlTypeId > T ODatabaseMetaDataResultSet::getInteger ( sal_Int32 columnIndex )
{
    ::cppu::OBroadcastHelper& rBHelper(ODatabaseMetaDataResultSet_BASE::rBHelper);
    checkDisposed(rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    columnIndex = mapColumn(columnIndex);
    T nVal = 0;
    if(columnIndex <= m_nDriverColumnCount)
    {
        getValue<T>(m_pConnection, m_aStatementHandle, columnIndex, sqlTypeId, m_bWasNull, **this, nVal);

        if ( !m_aValueRange.empty() )
        {
            ::std::map<sal_Int32, ::connectivity::TInt2IntMap >::iterator aValueRangeIter (m_aValueRange.find(columnIndex));
            if ( aValueRangeIter != m_aValueRange.end() )
                return static_cast<T>(aValueRangeIter->second[nVal]);
        }
    }
    else
        m_bWasNull = sal_True;
    return nVal;
}

// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getBinaryStream", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getCharacterStream", *this );
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    columnIndex = mapColumn(columnIndex);

    sal_Bool bRet = sal_False;
    if(columnIndex <= m_nDriverColumnCount)
    {
        sal_Int32 nType = getMetaData()->getColumnType(columnIndex);
        switch(nType)
        {
            case DataType::BIT:
                {
                    sal_Int8 nValue = 0;
                    OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,SQL_C_BIT,m_bWasNull,**this,&nValue,sizeof nValue);
                    bRet = nValue != 0;
                }
                break;
            default:
                bRet = getInt(columnIndex) != 0;
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getInteger<sal_Int8, SQL_C_STINYINT>( columnIndex );
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    if(columnIndex <= m_nDriverColumnCount)
    {
        sal_Int32 nType = getMetaData()->getColumnType(columnIndex);
        switch(nType)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            {
                OUString aRet = OTools::getStringValue(m_pConnection,m_aStatementHandle,columnIndex,SQL_C_BINARY,m_bWasNull,**this,m_nTextEncoding);
                return Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),sizeof(sal_Unicode)*aRet.getLength());
            }
        }
        return OTools::getBytesValue(m_pConnection,m_aStatementHandle,columnIndex,SQL_C_BINARY,m_bWasNull,**this);
    }
    else
        m_bWasNull = sal_True;
    return Sequence<sal_Int8>();
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    if(columnIndex <= m_nDriverColumnCount)
    {
        DATE_STRUCT aDate;
        aDate.day = 0;
        aDate.month = 0;
        aDate.year = 0;
        OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_DATE : SQL_C_TYPE_DATE,m_bWasNull,**this,&aDate,sizeof aDate);
        return Date(aDate.day,aDate.month,aDate.year);
    }
    else
        m_bWasNull = sal_True;
    return Date();
}
// -------------------------------------------------------------------------

double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    double nValue(0.0);
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,SQL_C_DOUBLE,m_bWasNull,**this,&nValue,sizeof nValue);
    else
        m_bWasNull = sal_True;
    return nValue;
}
// -------------------------------------------------------------------------

float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    float nVal(0);
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,SQL_C_FLOAT,m_bWasNull,**this,&nVal,sizeof nVal);
    else
        m_bWasNull = sal_True;
    return nVal;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getInteger<sal_Int32, SQL_C_SLONG>( columnIndex );
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getInteger<sal_Int64, SQL_C_SBIGINT>( columnIndex );
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_xMetaData.is() ? m_xMetaData :  (m_xMetaData = new OResultSetMetaData(m_pConnection,m_aStatementHandle));
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getArray", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getClob", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getBlob", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getRef", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 /*columnIndex*/, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException( "XRow::getObject", *this );
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getInteger<sal_Int16, SQL_C_SSHORT>( columnIndex );
}
// -------------------------------------------------------------------------

OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    OUString aVal;
    if(columnIndex <= m_nDriverColumnCount)
        aVal = OTools::getStringValue(m_pConnection,m_aStatementHandle,columnIndex,impl_getColumnType_nothrow(columnIndex),m_bWasNull,**this,m_nTextEncoding);
    else
        m_bWasNull = sal_True;

    return aVal;
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    TIME_STRUCT aTime={0,0,0};
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_TIME : SQL_C_TYPE_TIME,m_bWasNull,**this,&aTime,sizeof aTime);
    else
        m_bWasNull = sal_True;
    return Time(0, aTime.second,aTime.minute,aTime.hour, false);
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    TIMESTAMP_STRUCT aTime={0,0,0,0,0,0,0};
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection,m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_TIMESTAMP : SQL_C_TYPE_TIMESTAMP,m_bWasNull,**this,&aTime,sizeof aTime);
    else
        m_bWasNull = sal_True;
    return DateTime(aTime.fraction, aTime.second, aTime.minute, aTime.hour,
            aTime.day, aTime.month, aTime.year, false);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nCurrentFetchState == SQL_NO_DATA;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_bEOF && m_nCurrentFetchState != SQL_NO_DATA;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    if(first())
        previous();
    m_nCurrentFetchState = SQL_SUCCESS;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    if(last())
        next();
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {

        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
        ::osl::MutexGuard aGuard( m_aMutex );

    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = sal_False;

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_FIRST,0);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = ( m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO );
    if( bRet )
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException)
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );
    ::osl::MutexGuard aGuard( m_aMutex );


    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_LAST,0);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // here I know definitely that I stand on the last record
    sal_Bool bRet = ( m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO );
    if( bRet )
        m_bEOF = sal_True;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = sal_False;

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_ABSOLUTE,row);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos = row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = sal_False;

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,row);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = sal_False;

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        --m_nRowPos;
    else if ( m_nCurrentFetchState == SQL_NO_DATA )
        m_nRowPos = 0;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_DELETED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_ADDED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_UPDATED;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nRowPos == 0;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = sal_False;

    SQLRETURN nOldFetchStatus = m_nCurrentFetchState;
    //  m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_NEXT,0);
    m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    OTools::ThrowException(m_pConnection,m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet || ( m_nCurrentFetchState == SQL_NO_DATA && nOldFetchStatus != SQL_NO_DATA ) )
        ++m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_bWasNull;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException)
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    OTools::ThrowException(m_pConnection,N3SQLCancel(m_aStatementHandle),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetConcurrency() const throw(SQLException, RuntimeException)
{
    return ResultSetConcurrency::READ_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetType() const throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchDirection() const throw(SQLException, RuntimeException)
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchSize() const throw(SQLException, RuntimeException)
{
    sal_Int32 nValue=1;
    return nValue;
}
//------------------------------------------------------------------------------
OUString ODatabaseMetaDataResultSet::getCursorName() const throw(SQLException, RuntimeException)
{
    return OUString();
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,          OUString);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,       sal_Int32);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *const_cast<ODatabaseMetaDataResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& /*rValue*/ ) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception();
        default:
            OSL_FAIL("setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch(nHandle)
    {
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
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTypeInfo() throw(SQLException, RuntimeException)
{
    TInt2IntMap aMap;
    aMap[SQL_BIT]               = DataType::BIT;
    aMap[SQL_TINYINT]           = DataType::TINYINT;
    aMap[SQL_SMALLINT]          = DataType::SMALLINT;
    aMap[SQL_INTEGER]           = DataType::INTEGER;
    aMap[SQL_FLOAT]             = DataType::FLOAT;
    aMap[SQL_REAL]              = DataType::REAL;
    aMap[SQL_DOUBLE]            = DataType::DOUBLE;
    aMap[SQL_BIGINT]            = DataType::BIGINT;

    aMap[SQL_CHAR]              = DataType::CHAR;
    aMap[SQL_WCHAR]             = DataType::CHAR;
    aMap[SQL_VARCHAR]           = DataType::VARCHAR;
    aMap[SQL_WVARCHAR]          = DataType::VARCHAR;
    aMap[SQL_LONGVARCHAR]       = DataType::LONGVARCHAR;
    aMap[SQL_WLONGVARCHAR]      = DataType::LONGVARCHAR;

    aMap[SQL_TYPE_DATE]         = DataType::DATE;
    aMap[SQL_DATE]              = DataType::DATE;
    aMap[SQL_TYPE_TIME]         = DataType::TIME;
    aMap[SQL_TIME]              = DataType::TIME;
    aMap[SQL_TYPE_TIMESTAMP]    = DataType::TIMESTAMP;
    aMap[SQL_TIMESTAMP]         = DataType::TIMESTAMP;

    aMap[SQL_DECIMAL]           = DataType::DECIMAL;
    aMap[SQL_NUMERIC]           = DataType::NUMERIC;

    aMap[SQL_BINARY]            = DataType::BINARY;
    aMap[SQL_VARBINARY]         = DataType::VARBINARY;
    aMap[SQL_LONGVARBINARY]     = DataType::LONGVARBINARY;

    aMap[SQL_GUID]              = DataType::VARBINARY;


    m_aValueRange[2] = aMap;

    OTools::ThrowException(m_pConnection,N3SQLGetTypeInfo(m_aStatementHandle, SQL_ALL_TYPES),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
//-----------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTables(const Any& catalog, const OUString& schemaPattern,
                            const OUString& tableNamePattern,
                            const Sequence< OUString >& types )  throw(SQLException, RuntimeException)
{
    OString aPKQ,aPKO,aPKN,aCOL;
    const OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schemaPattern,m_nTextEncoding);
    aPKN = OUStringToOString(tableNamePattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr();


    const char  *pCOL = NULL;
    const char* pComma = ",";
    const OUString* pBegin = types.getConstArray();
    const OUString* pEnd = pBegin + types.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        aCOL += OUStringToOString(*pBegin,m_nTextEncoding);
        aCOL += pComma;
    }
    if ( !aCOL.isEmpty() )
    {
        aCOL = aCOL.replaceAt(aCOL.getLength()-1,1,pComma);
        pCOL = aCOL.getStr();
    }
    else
        pCOL = SQL_ALL_TABLE_TYPES;

    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, pCOL ? SQL_NTS : 0);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();

}
//-----------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTablesTypes( ) throw(SQLException, RuntimeException)
{
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            0,0,
                            0,0,
                            0,0,
                            (SDB_ODBC_CHAR *) SQL_ALL_TABLE_TYPES,SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(4);
    m_xMetaData = new OResultSetMetaData(m_pConnection,m_aStatementHandle,m_aColMapping);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openCatalogs() throw(SQLException, RuntimeException)
{
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) SQL_ALL_CATALOGS,SQL_NTS,
                            (SDB_ODBC_CHAR *) "",SQL_NTS,
                            (SDB_ODBC_CHAR *) "",SQL_NTS,
                            (SDB_ODBC_CHAR *) "",SQL_NTS);

    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(1);
    m_xMetaData = new OResultSetMetaData(m_pConnection,m_aStatementHandle,m_aColMapping);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openSchemas() throw(SQLException, RuntimeException)
{
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) "",SQL_NTS,
                            (SDB_ODBC_CHAR *) SQL_ALL_SCHEMAS,SQL_NTS,
                            (SDB_ODBC_CHAR *) "",SQL_NTS,
                            (SDB_ODBC_CHAR *) "",SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(2);
    m_xMetaData = new OResultSetMetaData(m_pConnection,m_aStatementHandle,m_aColMapping);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openColumnPrivileges(  const Any& catalog, const OUString& schema,
                                        const OUString& table,   const OUString& columnNamePattern )
                                        throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN,aCOL;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schema,m_nTextEncoding);
    aPKN = OUStringToOString(table,m_nTextEncoding);
    aCOL = OUStringToOString(columnNamePattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr(),
                *pCOL = aCOL.getStr();


    SQLRETURN nRetcode = N3SQLColumnPrivileges(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openColumns(   const Any& catalog,             const OUString& schemaPattern,
                                const OUString& tableNamePattern,    const OUString& columnNamePattern )
                                throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN,aCOL;
    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schemaPattern,m_nTextEncoding);
    aPKN = OUStringToOString(tableNamePattern,m_nTextEncoding);
    aCOL = OUStringToOString(columnNamePattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr(),
                *pCOL = aCOL.getStr();


    SQLRETURN nRetcode = N3SQLColumns(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);

    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    TInt2IntMap aMap;
    aMap[SQL_BIT]               = DataType::BIT;
    aMap[SQL_TINYINT]           = DataType::TINYINT;
    aMap[SQL_SMALLINT]          = DataType::SMALLINT;
    aMap[SQL_INTEGER]           = DataType::INTEGER;
    aMap[SQL_FLOAT]             = DataType::FLOAT;
    aMap[SQL_REAL]              = DataType::REAL;
    aMap[SQL_DOUBLE]            = DataType::DOUBLE;
    aMap[SQL_BIGINT]            = DataType::BIGINT;

    aMap[SQL_CHAR]              = DataType::CHAR;
    aMap[SQL_WCHAR]             = DataType::CHAR;
    aMap[SQL_VARCHAR]           = DataType::VARCHAR;
    aMap[SQL_WVARCHAR]          = DataType::VARCHAR;
    aMap[SQL_LONGVARCHAR]       = DataType::LONGVARCHAR;
    aMap[SQL_WLONGVARCHAR]      = DataType::LONGVARCHAR;

    aMap[SQL_TYPE_DATE]         = DataType::DATE;
    aMap[SQL_DATE]              = DataType::DATE;
    aMap[SQL_TYPE_TIME]         = DataType::TIME;
    aMap[SQL_TIME]              = DataType::TIME;
    aMap[SQL_TYPE_TIMESTAMP]    = DataType::TIMESTAMP;
    aMap[SQL_TIMESTAMP]         = DataType::TIMESTAMP;

    aMap[SQL_DECIMAL]           = DataType::DECIMAL;
    aMap[SQL_NUMERIC]           = DataType::NUMERIC;

    aMap[SQL_BINARY]            = DataType::BINARY;
    aMap[SQL_VARBINARY]         = DataType::VARBINARY;
    aMap[SQL_LONGVARBINARY]     = DataType::LONGVARBINARY;

    aMap[SQL_GUID]              = DataType::VARBINARY;

    m_aValueRange[5] = aMap;
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openProcedureColumns(  const Any& catalog,     const OUString& schemaPattern,
                                const OUString& procedureNamePattern,const OUString& columnNamePattern )
                                throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN,aCOL;
    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schemaPattern,m_nTextEncoding);
    aPKN = OUStringToOString(procedureNamePattern,m_nTextEncoding);
    aCOL = OUStringToOString(columnNamePattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr(),
                *pCOL = aCOL.getStr();


    SQLRETURN nRetcode = N3SQLProcedureColumns(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);

    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openProcedures(const Any& catalog, const OUString& schemaPattern,
                                const OUString& procedureNamePattern)
                                throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schemaPattern,m_nTextEncoding);
    aPKN = OUStringToOString(procedureNamePattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr();


    SQLRETURN nRetcode = N3SQLProcedures(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openSpecialColumns(sal_Bool _bRowVer,const Any& catalog, const OUString& schema,
                                    const OUString& table,sal_Int32 scope,   sal_Bool nullable )
                                    throw(SQLException, RuntimeException)
{
    // Some ODBC drivers really don't like getting an empty string as tableName
    // E.g. psqlodbc up to at least version 09.01.0100 segfaults
    if (table.isEmpty())
    {
        const char errMsg[] = "ODBC: Trying to get special columns of empty table name";
        const char SQLState[] = "HY009";
        throw SQLException( OUString(errMsg, sizeof(errMsg) - sizeof(errMsg[0]), RTL_TEXTENCODING_ASCII_US),
                            *this,
                            OUString(SQLState, sizeof(SQLState) - sizeof(SQLState[0]), RTL_TEXTENCODING_ASCII_US),
                            -1,
                            Any() );
    }

    const OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN;
    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schema,m_nTextEncoding);
    aPKN = OUStringToOString(table,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = aPKN.getStr();


    SQLRETURN nRetcode = N3SQLSpecialColumns(m_aStatementHandle,_bRowVer ? SQL_ROWVER : SQL_BEST_ROWID,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SQLSMALLINT)scope,
                            nullable ? SQL_NULLABLE : SQL_NO_NULLS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openVersionColumns(const Any& catalog, const OUString& schema,
                                    const OUString& table)  throw(SQLException, RuntimeException)
{
    openSpecialColumns(sal_True,catalog,schema,table,SQL_SCOPE_TRANSACTION,sal_False);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openBestRowIdentifier( const Any& catalog, const OUString& schema,
                                        const OUString& table,sal_Int32 scope,sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    openSpecialColumns(sal_False,catalog,schema,table,scope,nullable);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openForeignKeys( const Any& catalog, const OUString* schema,
                                  const OUString* table,
                                  const Any& catalog2, const OUString* schema2,
                                  const OUString* table2) throw(SQLException, RuntimeException)
{
    OString aPKQ, aPKN, aFKQ, aFKO, aFKN;
    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    if ( catalog2.hasValue() )
        aFKQ = OUStringToOString(comphelper::getString(catalog2),m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = schema && !schema->isEmpty() ? OUStringToOString(*schema,m_nTextEncoding).getStr() : NULL,
                *pPKN = table   ? (aPKN = OUStringToOString(*table,m_nTextEncoding)).getStr(): NULL,
                *pFKQ = catalog2.hasValue() && !aFKQ.isEmpty() ? aFKQ.getStr() : NULL,
                *pFKO = schema2 && !schema2->isEmpty() ? (aFKO = OUStringToOString(*schema2,m_nTextEncoding)).getStr() : NULL,
                *pFKN = table2  ? (aFKN = OUStringToOString(*table2,m_nTextEncoding)).getStr() : NULL;


    SQLRETURN nRetcode = N3SQLForeignKeys(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, pPKN ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKQ, (catalog2.hasValue() && !aFKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKO, pFKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKN, SQL_NTS
                            );
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openImportedKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table) throw(SQLException, RuntimeException)
{

    openForeignKeys(Any(),NULL,NULL,catalog,!schema.compareToAscii("%") ? &schema : NULL,&table);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openExportedKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table) throw(SQLException, RuntimeException)
{
    openForeignKeys(catalog,!schema.compareToAscii("%") ? &schema : NULL,&table,Any(),NULL,NULL);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openPrimaryKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table) throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = OUStringToOString(table,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLPrimaryKeys(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTablePrivileges(const Any& catalog, const OUString& schemaPattern,
                                  const OUString& tableNamePattern) throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = OUStringToOString(tableNamePattern,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLTablePrivileges(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openIndexInfo( const Any& catalog, const OUString& schema,
                                const OUString& table,sal_Bool unique,sal_Bool approximate )
                                throw(SQLException, RuntimeException)
{
    const OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    OString aPKQ,aPKO,aPKN;

    if ( catalog.hasValue() )
        aPKQ = OUStringToOString(comphelper::getString(catalog),m_nTextEncoding);
    aPKO = OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && !aPKQ.isEmpty() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && !pSchemaPat->isEmpty() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = OUStringToOString(table,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLStatistics(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && !aPKQ.isEmpty()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            unique ? SQL_INDEX_UNIQUE : SQL_INDEX_ALL,
                            approximate);
    OTools::ThrowException(m_pConnection,nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::checkColumnCount()
{
    sal_Int16 nNumResultCols=0;
    OTools::ThrowException(m_pConnection,N3SQLNumResultCols(m_aStatementHandle,&nNumResultCols),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    m_nDriverColumnCount = nNumResultCols;
}
// -----------------------------------------------------------------------------

SWORD ODatabaseMetaDataResultSet::impl_getColumnType_nothrow(sal_Int32 columnIndex)
{
    ::std::map<sal_Int32,SWORD>::iterator aFind = m_aODBCColumnTypes.find(columnIndex);
    if ( aFind == m_aODBCColumnTypes.end() )
        aFind = m_aODBCColumnTypes.insert(::std::map<sal_Int32,SWORD>::value_type(columnIndex,OResultSetMetaData::getColumnODBCType(m_pConnection,m_aStatementHandle,*this,columnIndex))).first;
    return aFind->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
