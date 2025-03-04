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

#include <TConnection.hxx>

#include <odbc/ODatabaseMetaDataResultSet.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/stl_types.hxx>
#include <odbc/OResultSetMetaData.hxx>
#include <odbc/OTools.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;


using namespace connectivity::odbc;
using namespace cppu;

using namespace ::com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;


ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(OConnection* _pConnection)
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)

    ,m_aStatementHandle(_pConnection->createStatementHandle())
    ,m_pConnection(_pConnection)
    ,m_nTextEncoding(_pConnection->getTextEncoding())
    ,m_nRowPos(-1)
    ,m_nDriverColumnCount(0)
    ,m_nCurrentFetchState(0)
    ,m_bWasNull(true)
    ,m_bEOF(false)
{
    OSL_ENSURE(m_pConnection.is(),"ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet: No parent set!");
    if( SQL_NULL_HANDLE == m_aStatementHandle )
        throw RuntimeException();

    osl_atomic_increment( &m_refCount );
    m_pRowStatusArray.reset( new SQLUSMALLINT[1] ); // the default value
    osl_atomic_decrement( &m_refCount );
}


ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    OSL_ENSURE(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed,"Object wasn't disposed!");
    if(!ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
    {
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void ODatabaseMetaDataResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_pConnection->freeStatementHandle(m_aStatementHandle);

    m_xMetaData.clear();
    m_pConnection.clear();
}

Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}

Reference< XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void SAL_CALL ODatabaseMetaDataResultSet::acquire() noexcept
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}

void SAL_CALL ODatabaseMetaDataResultSet::release() noexcept
{
    ODatabaseMetaDataResultSet_BASE::release();
}

Sequence< Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                    cppu::UnoType<XFastPropertySet>::get(),
                                    cppu::UnoType<XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}

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


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
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
        getValue<T>(m_pConnection.get(), m_aStatementHandle, columnIndex, sqlTypeId, m_bWasNull, **this, nVal);

        if ( !m_aValueRange.empty() )
        {
            auto aValueRangeIter = m_aValueRange.find(columnIndex);
            if ( aValueRangeIter != m_aValueRange.end() )
                return static_cast<T>(aValueRangeIter->second[nVal]);
        }
    }
    else
        m_bWasNull = true;
    return nVal;
}


Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getBinaryStream"_ustr, *this );
}

Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getCharacterStream"_ustr, *this );
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    columnIndex = mapColumn(columnIndex);

    bool bRet = false;
    if(columnIndex <= m_nDriverColumnCount)
    {
        sal_Int32 nType = getMetaData()->getColumnType(columnIndex);
        switch(nType)
        {
            case DataType::BIT:
                {
                    sal_Int8 nValue = 0;
                    OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,SQL_C_BIT,m_bWasNull,**this,&nValue,sizeof nValue);
                    bRet = nValue != 0;
                }
                break;
            default:
                bRet = getInt(columnIndex) != 0;
        }
    }
    return bRet;
}


sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex )
{
    return getInteger<sal_Int8, SQL_C_STINYINT>( columnIndex );
}


Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex )
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
                OUString const aRet = OTools::getStringValue(m_pConnection.get(),m_aStatementHandle,columnIndex,SQL_C_BINARY,m_bWasNull,**this,m_nTextEncoding);
                return Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),sizeof(sal_Unicode)*aRet.getLength());
            }
        }
        return OTools::getBytesValue(m_pConnection.get(),m_aStatementHandle,columnIndex,SQL_C_BINARY,m_bWasNull,**this);
    }
    else
        m_bWasNull = true;
    return Sequence<sal_Int8>();
}


css::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex )
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
        OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_DATE : SQL_C_TYPE_DATE,m_bWasNull,**this,&aDate,sizeof aDate);
        return Date(aDate.day,aDate.month,aDate.year);
    }
    else
        m_bWasNull = true;
    return Date();
}


double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    double nValue(0.0);
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,SQL_C_DOUBLE,m_bWasNull,**this,&nValue,sizeof nValue);
    else
        m_bWasNull = true;
    return nValue;
}


float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    float nVal(0);
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,SQL_C_FLOAT,m_bWasNull,**this,&nVal,sizeof nVal);
    else
        m_bWasNull = true;
    return nVal;
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex )
{
    return getInteger<sal_Int32, SQL_C_SLONG>( columnIndex );
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  )
{
    return 0;
}


sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex )
{
    return getInteger<sal_Int64, SQL_C_SBIGINT>( columnIndex );
}


Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  )
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection.get(),m_aStatementHandle);
    return m_xMetaData;
}

Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getArray"_ustr, *this );
}

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getClob"_ustr, *this );
}

Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getBlob"_ustr, *this );
}


Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getRef"_ustr, *this );
}


Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 /*columnIndex*/, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::dbtools::throwFunctionNotSupportedSQLException( u"XRow::getObject"_ustr, *this );
}


sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex )
{
    return getInteger<sal_Int16, SQL_C_SSHORT>( columnIndex );
}


OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    OUString aVal;
    if(columnIndex <= m_nDriverColumnCount)
        aVal = OTools::getStringValue(m_pConnection.get(),m_aStatementHandle,columnIndex,impl_getColumnType_nothrow(columnIndex),m_bWasNull,**this,m_nTextEncoding);
    else
        m_bWasNull = true;

    return aVal;
}


css::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    TIME_STRUCT aTime={0,0,0};
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_TIME : SQL_C_TYPE_TIME,m_bWasNull,**this,&aTime,sizeof aTime);
    else
        m_bWasNull = true;
    return Time(0, aTime.second,aTime.minute,aTime.hour, false);
}


css::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    TIMESTAMP_STRUCT aTime={0,0,0,0,0,0,0};
    if(columnIndex <= m_nDriverColumnCount)
        OTools::getValue(m_pConnection.get(),m_aStatementHandle,columnIndex,m_pConnection->useOldDateFormat() ? SQL_C_TIMESTAMP : SQL_C_TYPE_TIMESTAMP, m_bWasNull, **this, &aTime, sizeof aTime);
    else
        m_bWasNull = true;
    return DateTime(aTime.fraction, aTime.second, aTime.minute, aTime.hour,
            aTime.day, aTime.month, aTime.year, false);
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nCurrentFetchState == SQL_NO_DATA;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nRowPos == 1;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_bEOF && m_nCurrentFetchState != SQL_NO_DATA;
}

void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    if(first())
        previous();
    m_nCurrentFetchState = SQL_SUCCESS;
}

void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    if(last())
        next();
}


void SAL_CALL ODatabaseMetaDataResultSet::close(  )
{
    {

        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
        ::osl::MutexGuard aGuard( m_aMutex );

    }
    dispose();
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = false;

    m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_FIRST,0);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    bool bRet = ( m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO );
    if( bRet )
        m_nRowPos = 1;
    return bRet;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  )
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );
    ::osl::MutexGuard aGuard( m_aMutex );


    m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_LAST,0);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // here I know definitely that I stand on the last record
    bool bRet = ( m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO );
    if( bRet )
        m_bEOF = true;
    return bRet;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = false;

    m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_ABSOLUTE,row);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos = row;
    return bRet;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = false;

    m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,row);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos += row;
    return bRet;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = false;

    m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        --m_nRowPos;
    else if ( m_nCurrentFetchState == SQL_NO_DATA )
        m_nRowPos = 0;
    return bRet;
}

Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  )
{
    return nullptr;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_DELETED;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  )
{
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_ADDED;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_pRowStatusArray[0] == SQL_ROW_UPDATED;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_nRowPos == 0;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bEOF = false;

    SQLRETURN nOldFetchStatus = m_nCurrentFetchState;
    //  m_nCurrentFetchState = functions().FetchScroll(m_aStatementHandle,SQL_FETCH_NEXT,0);
    m_nCurrentFetchState = functions().Fetch(m_aStatementHandle);
    OTools::ThrowException(m_pConnection.get(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet || ( m_nCurrentFetchState == SQL_NO_DATA && nOldFetchStatus != SQL_NO_DATA ) )
        ++m_nRowPos;
    return bRet;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    return m_bWasNull;
}

void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}


void SAL_CALL ODatabaseMetaDataResultSet::cancel(  )
{

    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    functions().Cancel(m_aStatementHandle);
}

void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  )
{
}

Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  )
{
    return Any();
}

sal_Int32 ODatabaseMetaDataResultSet::getFetchSize()
{
    return 1;
}

const OUString & ODatabaseMetaDataResultSet::getCursorName()
{
    return EMPTY_OUSTRING;
}


::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    return new ::cppu::OPropertyArrayHelper
    {
        {
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
                PROPERTY_ID_CURSORNAME,
                cppu::UnoType<OUString>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                PROPERTY_ID_FETCHDIRECTION,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                PROPERTY_ID_FETCHSIZE,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                PROPERTY_ID_RESULTSETCONCURRENCY,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                PROPERTY_ID_RESULTSETTYPE,
                cppu::UnoType<sal_Int32>::get(),
                0
            }
        }
    };
}

::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    switch(nHandle)
    {
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

void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& /*rValue*/ )
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
        default:
            OSL_FAIL("setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}

void ODatabaseMetaDataResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= sal_Int32(css::sdbc::ResultSetConcurrency::READ_ONLY);
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= sal_Int32(css::sdbc::ResultSetType::FORWARD_ONLY);
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}

void ODatabaseMetaDataResultSet::openTypeInfo()
{
    ::std::map<sal_Int32,sal_Int32> aMap;
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


    m_aValueRange[2] = std::move(aMap);

    OTools::ThrowException(m_pConnection.get(),functions().GetTypeInfo(m_aStatementHandle, SQL_ALL_TYPES),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openTables(const Any& catalog, const OUString& schemaPattern,
                            const OUString& tableNamePattern,
                            const Sequence< OUString >& types )
{
    OUString uPKQ;
    catalog >>= uPKQ;
    OUStringBuffer uCOL;
    comphelper::intersperse(types.begin(), types.end(), comphelper::OUStringBufferAppender(uCOL), u",");
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::TablesW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLWChars(schemaPattern);
        SQLWChars aPKN(tableNamePattern);
        SQLWChars aCOL = !uCOL.isEmpty() ? SQLWChars(uCOL.makeStringAndClear()) : SQLWChars(u"" SQL_ALL_TABLE_TYPES ""_ustr);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().TablesW(m_aStatementHandle,
                                       pPKQ, pPKQ ? SQL_NTSL : 0,
                                       pPKO, pPKO ? SQL_NTSL : 0,
                                       aPKN.get(), SQL_NTSL,
                                       aCOL.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLChars(schemaPattern, m_nTextEncoding);
        SQLChars aPKN(tableNamePattern, m_nTextEncoding);
        SQLChars aCOL = !uCOL.isEmpty() ? SQLChars(uCOL, m_nTextEncoding) : SQLChars(SQL_ALL_TABLE_TYPES ""_ostr);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().Tables(m_aStatementHandle,
                                      pPKQ, pPKQ ? SQL_NTS : 0,
                                      pPKO, pPKO ? SQL_NTS : 0,
                                      aPKN.get(), SQL_NTS,
                                      aCOL.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();

}

void ODatabaseMetaDataResultSet::openTablesTypes( )
{
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::TablesW))
    {
        nRetcode = functions().TablesW(m_aStatementHandle,
                                       nullptr, 0,
                                       nullptr, 0,
                                       nullptr, 0,
                                       SQLWChars(u"" SQL_ALL_TABLE_TYPES ""_ustr).get(), SQL_NTSL);
    }
    else
    {
        nRetcode = functions().Tables(m_aStatementHandle,
                                      nullptr,0,
                                      nullptr,0,
                                      nullptr,0,
                                      SQLChars(SQL_ALL_TABLE_TYPES ""_ostr).get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(4);
    m_xMetaData = new OResultSetMetaData(m_pConnection.get(),m_aStatementHandle,std::vector(m_aColMapping));
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openCatalogs()
{
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::TablesW))
    {
        nRetcode = functions().TablesW(m_aStatementHandle,
                                       SQLWChars(u"" SQL_ALL_CATALOGS ""_ustr).get(), SQL_NTSL,
                                       SQLWChars(u""_ustr).get(), SQL_NTSL,
                                       SQLWChars(u""_ustr).get(), SQL_NTSL,
                                       SQLWChars(u""_ustr).get(), SQL_NTSL);
    }
    else
    {
        nRetcode = functions().Tables(m_aStatementHandle,
                                      SQLChars(SQL_ALL_CATALOGS ""_ostr).get(), SQL_NTS,
                                      SQLChars(""_ostr).get(), SQL_NTS,
                                      SQLChars(""_ostr).get(), SQL_NTS,
                                      SQLChars(""_ostr).get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(1);
    m_xMetaData = new OResultSetMetaData(m_pConnection.get(),m_aStatementHandle,std::vector(m_aColMapping));
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openSchemas()
{
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::TablesW))
    {
        nRetcode = functions().TablesW(m_aStatementHandle,
                                       SQLWChars(u""_ustr).get(), SQL_NTSL,
                                       SQLWChars(u"" SQL_ALL_SCHEMAS ""_ustr).get(), SQL_NTSL,
                                       SQLWChars(u""_ustr).get(),SQL_NTSL,
                                       SQLWChars(u""_ustr).get(),SQL_NTSL);
    }
    else
    {
        nRetcode = functions().Tables(m_aStatementHandle,
                                      SQLChars(""_ostr).get(), SQL_NTS,
                                      SQLChars(SQL_ALL_SCHEMAS ""_ostr).get(), SQL_NTS,
                                      SQLChars(""_ostr).get(), SQL_NTS,
                                      SQLChars(""_ostr).get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(2);
    m_xMetaData = new OResultSetMetaData(m_pConnection.get(),m_aStatementHandle,std::vector(m_aColMapping));
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openColumnPrivileges(  const Any& catalog, const OUString& schema,
                                        const OUString& table,
                                        const OUString& columnNamePattern )
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::ColumnPrivilegesW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLWChars(schema);
        SQLWChars aPKN(table);
        SQLWChars aCOL(columnNamePattern);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ColumnPrivilegesW(m_aStatementHandle,
                                                 pPKQ, pPKQ ? SQL_NTSL : 0,
                                                 pPKO, pPKO ? SQL_NTSL : 0,
                                                 aPKN.get(), SQL_NTSL,
                                                 aCOL.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLChars(schema, m_nTextEncoding);
        SQLChars aPKN(table, m_nTextEncoding);
        SQLChars aCOL(columnNamePattern, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ColumnPrivileges(m_aStatementHandle,
                                                pPKQ, pPKQ ? SQL_NTS : 0,
                                                pPKO, pPKO ? SQL_NTS : 0,
                                                aPKN.get(), SQL_NTS,
                                                aCOL.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openColumns(   const Any& catalog,             const OUString& schemaPattern,
                                const OUString& tableNamePattern,    const OUString& columnNamePattern )
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::ColumnsW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLWChars(schemaPattern);
        SQLWChars aPKN(tableNamePattern);
        SQLWChars aCOL(columnNamePattern);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ColumnsW(m_aStatementHandle,
                                        pPKQ, pPKQ ? SQL_NTSL : 0,
                                        pPKO, pPKO ? SQL_NTSL : 0,
                                        aPKN.get(), SQL_NTSL,
                                        aCOL.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLChars(schemaPattern, m_nTextEncoding);
        SQLChars aPKN(tableNamePattern, m_nTextEncoding);
        SQLChars aCOL(columnNamePattern, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().Columns(m_aStatementHandle,
                                       pPKQ, pPKQ ? SQL_NTS : 0,
                                       pPKO, pPKO ? SQL_NTS : 0,
                                       aPKN.get(), SQL_NTS,
                                       aCOL.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    ::std::map<sal_Int32,sal_Int32> aMap;
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

    m_aValueRange[5] = std::move(aMap);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openProcedureColumns(  const Any& catalog,     const OUString& schemaPattern,
                                const OUString& procedureNamePattern,const OUString& columnNamePattern )
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::ProcedureColumnsW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLWChars(schemaPattern);
        SQLWChars aPKN(procedureNamePattern);
        SQLWChars aCOL(columnNamePattern);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ProcedureColumnsW(m_aStatementHandle,
                                                 pPKQ, pPKQ ? SQL_NTSL : 0,
                                                 pPKO, pPKO ? SQL_NTSL : 0,
                                                 aPKN.get(), SQL_NTSL,
                                                 aCOL.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLChars(schemaPattern, m_nTextEncoding);
        SQLChars aPKN(procedureNamePattern, m_nTextEncoding);
        SQLChars aCOL(columnNamePattern, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ProcedureColumns(m_aStatementHandle,
                                                pPKQ, pPKQ ? SQL_NTS : 0,
                                                pPKO, pPKO ? SQL_NTS : 0,
                                                aPKN.get(), SQL_NTS,
                                                aCOL.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openProcedures(const Any& catalog, const OUString& schemaPattern,
                                const OUString& procedureNamePattern)
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::ProceduresW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLWChars(schemaPattern);
        SQLWChars aPKN(procedureNamePattern);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().ProceduresW(m_aStatementHandle,
                                           pPKQ, pPKQ ? SQL_NTSL : 0,
                                           pPKO, pPKO ? SQL_NTSL : 0,
                                           aPKN.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLChars(schemaPattern, m_nTextEncoding);
        SQLChars aPKN(procedureNamePattern, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().Procedures(m_aStatementHandle,
                                          pPKQ, pPKQ ? SQL_NTS : 0,
                                          pPKO, pPKO ? SQL_NTS : 0,
                                          aPKN.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openSpecialColumns(bool _bRowVer,const Any& catalog, const OUString& schema,
                                    const OUString& table,sal_Int32 scope,   bool nullable )
{
    // Some ODBC drivers really don't like getting an empty string as tableName
    // E.g. psqlodbc up to at least version 09.01.0100 segfaults
    if (table.isEmpty())
    {
        static constexpr OUStringLiteral errMsg
            = u"ODBC: Trying to get special columns of empty table name";
        static constexpr OUStringLiteral SQLState = u"HY009";
        throw SQLException( errMsg, *this, SQLState, -1, Any() );
    }

    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::SpecialColumnsW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLWChars(schema);
        SQLWChars aPKN(table);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().SpecialColumnsW(m_aStatementHandle, _bRowVer ? SQL_ROWVER : SQL_BEST_ROWID,
                                               pPKQ, pPKQ ? SQL_NTSL : 0,
                                               pPKO, pPKO ? SQL_NTSL : 0,
                                               aPKN.get(), SQL_NTSL,
                                               scope,
                                               nullable ? SQL_NULLABLE : SQL_NO_NULLS);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLChars(schema, m_nTextEncoding);
        SQLChars aPKN(table, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().SpecialColumns(m_aStatementHandle, _bRowVer ? SQL_ROWVER : SQL_BEST_ROWID,
                                              pPKQ, pPKQ ? SQL_NTS : 0,
                                              pPKO, pPKO ? SQL_NTS : 0,
                                              aPKN.get(), SQL_NTS,
                                              static_cast<SQLSMALLINT>(scope),
                                              nullable ? SQL_NULLABLE : SQL_NO_NULLS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openVersionColumns(const Any& catalog, const OUString& schema,
                                    const OUString& table)
{
    openSpecialColumns(true,catalog,schema,table,SQL_SCOPE_TRANSACTION,false);
}

void ODatabaseMetaDataResultSet::openBestRowIdentifier( const Any& catalog, const OUString& schema,
                                        const OUString& table,sal_Int32 scope,bool nullable )
{
    openSpecialColumns(false,catalog,schema,table,scope,nullable);
}

void ODatabaseMetaDataResultSet::openForeignKeys( const Any& catalog, const OUString* schema,
                                  const OUString* table,
                                  const Any& catalog2, const OUString* schema2,
                                  const OUString* table2)
{
    OUString uPKQ, uFKQ;
    catalog >>= uPKQ;
    catalog2 >>= uFKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::ForeignKeysW))
    {
        SQLWChars aPKQ, aPKO, aPKN, aFKQ, aFKO, aFKN;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!uFKQ.isEmpty())
            aFKQ = SQLWChars(uFKQ);
        if (schema && !schema->isEmpty())
            aPKO = SQLWChars(*schema);
        if (table)
            aPKN = SQLWChars(*table);
        if (schema2 && !schema2->isEmpty())
            aFKO = SQLWChars(*schema2);
        if (table2)
            aFKN = SQLWChars(*table2);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr;
        auto pPKO = !aPKO.empty() ? aPKO.get() : nullptr;
        auto pPKN = !aPKN.empty() ? aPKN.get() : nullptr;
        auto pFKQ = !aFKQ.empty() ? aFKQ.get() : nullptr;
        auto pFKO = !aFKO.empty() ? aFKO.get() : nullptr;
        auto pFKN = !aFKN.empty() ? aFKN.get() : nullptr;

        nRetcode = functions().ForeignKeysW(m_aStatementHandle,
                                            pPKQ, pPKQ ? SQL_NTSL : 0,
                                            pPKO, pPKO ? SQL_NTSL : 0,
                                            pPKN, pPKN ? SQL_NTSL : 0,
                                            pFKQ, pFKQ ? SQL_NTSL : 0,
                                            pFKO, pFKO ? SQL_NTSL : 0,
                                            pFKN, pFKN ? SQL_NTSL : 0);
    }
    else
    {
        SQLChars aPKQ, aPKO, aPKN, aFKQ, aFKO, aFKN;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!uFKQ.isEmpty())
            aFKQ = SQLChars(uFKQ, m_nTextEncoding);
        if (schema && !schema->isEmpty())
            aPKO = SQLChars(*schema, m_nTextEncoding);
        if (table)
            aPKN = SQLChars(*table, m_nTextEncoding);
        if (schema2 && !schema2->isEmpty())
            aFKO = SQLChars(*schema2, m_nTextEncoding);
        if (table2)
            aFKN = SQLChars(*table2, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr;
        auto pPKO = !aPKO.empty() ? aPKO.get() : nullptr;
        auto pPKN = !aPKN.empty() ? aPKN.get() : nullptr;
        auto pFKQ = !aFKQ.empty() ? aFKQ.get() : nullptr;
        auto pFKO = !aFKO.empty() ? aFKO.get() : nullptr;
        auto pFKN = !aFKN.empty() ? aFKN.get() : nullptr;

        nRetcode = functions().ForeignKeys(m_aStatementHandle,
                                           pPKQ, pPKQ ? SQL_NTS : 0,
                                           pPKO, pPKO ? SQL_NTS : 0,
                                           pPKN, pPKN ? SQL_NTS : 0,
                                           pFKQ, pFKQ ? SQL_NTS : 0,
                                           pFKO, pFKO ? SQL_NTS : 0,
                                           pFKN, pFKN ? SQL_NTS : 0);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openImportedKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table)
{

    openForeignKeys(Any(),nullptr,nullptr,catalog, schema == "%" ? &schema : nullptr, &table);
}

void ODatabaseMetaDataResultSet::openExportedKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table)
{
    openForeignKeys(catalog, schema == "%" ? &schema : nullptr, &table,Any(),nullptr,nullptr);
}

void ODatabaseMetaDataResultSet::openPrimaryKeys(const Any& catalog, const OUString& schema,
                                  const OUString& table)
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::PrimaryKeysW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLWChars(schema);
        SQLWChars aPKN(table);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().PrimaryKeysW(m_aStatementHandle,
                                            pPKQ, pPKQ ? SQL_NTSL : 0,
                                            pPKO, pPKO ? SQL_NTSL : 0,
                                            aPKN.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLChars(schema, m_nTextEncoding);
        SQLChars aPKN(table, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().PrimaryKeys(m_aStatementHandle,
                                           pPKQ, pPKQ ? SQL_NTS : 0,
                                           pPKO, pPKO ? SQL_NTS : 0,
                                           aPKN.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openTablePrivileges(const Any& catalog, const OUString& schemaPattern,
                                  const OUString& tableNamePattern)
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::TablePrivilegesW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLWChars(schemaPattern);
        SQLWChars aPKN(tableNamePattern);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().TablePrivilegesW(m_aStatementHandle,
                                                pPKQ, pPKQ ? SQL_NTSL : 0,
                                                pPKO, pPKO ? SQL_NTSL : 0,
                                                aPKN.get(), SQL_NTSL);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schemaPattern.isEmpty() && schemaPattern != "%")
            aPKO = SQLChars(schemaPattern, m_nTextEncoding);
        SQLChars aPKN(tableNamePattern, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().TablePrivileges(m_aStatementHandle,
                                               pPKQ, pPKQ ? SQL_NTS : 0,
                                               pPKO, pPKO ? SQL_NTS : 0,
                                               aPKN.get(), SQL_NTS);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::openIndexInfo( const Any& catalog, const OUString& schema,
                                const OUString& table, bool unique, bool approximate )
{
    OUString uPKQ;
    catalog >>= uPKQ;
    SQLRETURN nRetcode;
    if (bUseWChar && functions().has(ODBC3SQLFunctionId::StatisticsW))
    {
        SQLWChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLWChars(uPKQ);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLWChars(schema);
        SQLWChars aPKN(table);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().StatisticsW(m_aStatementHandle,
                                           pPKQ, pPKQ ? SQL_NTSL : 0,
                                           pPKO, pPKO ? SQL_NTSL : 0,
                                           aPKN.get(), SQL_NTSL,
                                           unique ? SQL_INDEX_UNIQUE : SQL_INDEX_ALL,
                                           approximate ? 1 : 0);
    }
    else
    {
        SQLChars aPKQ, aPKO;
        if (!uPKQ.isEmpty())
            aPKQ = SQLChars(uPKQ, m_nTextEncoding);
        if (!schema.isEmpty() && schema != "%")
            aPKO = SQLChars(schema, m_nTextEncoding);
        SQLChars aPKN(table, m_nTextEncoding);

        auto pPKQ = !aPKQ.empty() ? aPKQ.get() : nullptr,
             pPKO = !aPKO.empty() ? aPKO.get() : nullptr;

        nRetcode = functions().Statistics(m_aStatementHandle,
                                          pPKQ, pPKQ ? SQL_NTS : 0,
                                          pPKO, pPKO ? SQL_NTS : 0,
                                          aPKN.get(), SQL_NTS,
                                          unique ? SQL_INDEX_UNIQUE : SQL_INDEX_ALL,
                                          approximate ? 1 : 0);
    }
    OTools::ThrowException(m_pConnection.get(),nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    checkColumnCount();
}

void ODatabaseMetaDataResultSet::checkColumnCount()
{
    sal_Int16 nNumResultCols=0;
    OTools::ThrowException(m_pConnection.get(),functions().NumResultCols(m_aStatementHandle,&nNumResultCols),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    m_nDriverColumnCount = nNumResultCols;
}


SWORD ODatabaseMetaDataResultSet::impl_getColumnType_nothrow(sal_Int32 columnIndex)
{
    std::map<sal_Int32,SWORD>::iterator aFind = m_aODBCColumnTypes.find(columnIndex);
    if ( aFind == m_aODBCColumnTypes.end() )
        aFind = m_aODBCColumnTypes.emplace(
                        columnIndex,
                        OResultSetMetaData::getColumnODBCType(m_pConnection.get(),m_aStatementHandle,*this,columnIndex)
                    ).first;
    return aFind->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
