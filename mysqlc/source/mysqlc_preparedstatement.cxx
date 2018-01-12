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

#include "mysqlc_general.hxx"
#include "mysqlc_preparedstatement.hxx"
#include "mysqlc_propertyids.hxx"
#include "mysqlc_resultsetmetadata.hxx"

#include <boost/config.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/parameter_metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppuhelper/typeprovider.hxx>

#include <stdio.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using ::osl::MutexGuard;


static inline char * my_i_to_a(char * buf, size_t buf_size, int a)
{
    snprintf(buf, buf_size, "%d", a);
    return buf;
}

rtl::OUString OPreparedStatement::getImplementationName()
{
    return rtl::OUString("com.sun.star.sdbcx.mysqlc.PreparedStatement");
}

css::uno::Sequence<rtl::OUString> OPreparedStatement::getSupportedServiceNames()
{
    css::uno::Sequence<rtl::OUString> s(1);
    s[0] = "com.sun.star.sdbc.PreparedStatement";
    return s;
}

sal_Bool OPreparedStatement::supportsService(rtl::OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OPreparedStatement::OPreparedStatement(OConnection* _pConnection, sql::PreparedStatement * _cppPrepStmt)
    :OCommonStatement(_pConnection, _cppPrepStmt)
{
    m_xConnection = _pConnection;

    try {
        m_paramCount = static_cast<sql::PreparedStatement *>(cppStatement)->getParameterMetaData()->getParameterCount();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

OPreparedStatement::~OPreparedStatement()
{
}

void SAL_CALL OPreparedStatement::acquire()
    throw()
{
    OCommonStatement::acquire();
}

void SAL_CALL OPreparedStatement::release()
    throw()
{
    OCommonStatement::release();
}

Any SAL_CALL OPreparedStatement::queryInterface(const Type & rType)
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if (!aRet.hasValue()) {
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    }
    return aRet;
}

Sequence< Type > SAL_CALL OPreparedStatement::getTypes()
{
    return concatSequences(OPreparedStatement_BASE::getTypes(), OCommonStatement::getTypes());
}

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    try {
        if (!m_xMetaData.is()) {
            m_xMetaData = new OResultSetMetaData(
                                    static_cast<sql::PreparedStatement *>(cppStatement)->getMetaData(),
                                    getOwnConnection()->getConnectionEncoding()
                                );
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
    return m_xMetaData;
}

void SAL_CALL OPreparedStatement::close()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    try {
        clearWarnings();
        clearParameters();
        OCommonStatement::close();
    } catch (const SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}

sal_Bool SAL_CALL OPreparedStatement::execute()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    bool success = false;
    try {
        success = static_cast<sql::PreparedStatement *>(cppStatement)->execute();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
    return success;
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    sal_Int32 affectedRows = 0;
    try {
        affectedRows = static_cast<sql::PreparedStatement *>(cppStatement)->executeUpdate();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
    return affectedRows;
}

void SAL_CALL OPreparedStatement::setString(sal_Int32 parameter, const rtl::OUString& x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        std::string stringie(rtl::OUStringToOString(x, m_xConnection->getConnectionEncoding()).getStr());
        static_cast<sql::PreparedStatement *>(cppStatement)->setString(parameter, stringie);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearParameters", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    return m_xConnection.get();
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    Reference< XResultSet > xResultSet;
    try {
        sql::ResultSet * res = static_cast<sql::PreparedStatement *>(cppStatement)->executeQuery();
        xResultSet = new OResultSet(this, res, getOwnConnection()->getConnectionEncoding());
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
    return xResultSet;
}

void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 parameter, sal_Bool x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setBoolean(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBoolean", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setByte(sal_Int32 parameter, sal_Int8 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setByte", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setDate(sal_Int32 parameter, const Date& aData)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    std::string dateStr;
    char buf[20];
    dateStr.append(my_i_to_a(buf, sizeof(buf)-1, aData.Year));
    dateStr.append("-", 1);
    dateStr.append(my_i_to_a(buf, sizeof(buf)-1, aData.Month));
    dateStr.append("-", 1);
    dateStr.append(my_i_to_a(buf, sizeof(buf)-1, aData.Day));

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setDateTime(parameter, dateStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setDate", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setTime(sal_Int32 parameter, const Time& aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    std::string timeStr;
    char buf[20];
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Hours));
    timeStr.append(":", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Minutes));
    timeStr.append(":", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Seconds));

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setDateTime(parameter, timeStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setTime", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setTimestamp(sal_Int32 parameter, const DateTime& aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    std::string timeStr;
    char buf[20];
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Year));
    timeStr.append("-", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Month));
    timeStr.append("-", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Day));

    timeStr.append(" ", 1);

    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Hours));
    timeStr.append(":", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Minutes));
    timeStr.append(":", 1);
    timeStr.append(my_i_to_a(buf, sizeof(buf)-1, aVal.Seconds));

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setDateTime(parameter, timeStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setTimestamp", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setDouble(sal_Int32 parameter, double x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setDouble(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setDouble", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setFloat(sal_Int32 parameter, float x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setDouble(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setFloat", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setInt(sal_Int32 parameter, sal_Int32 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setInt", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setLong(sal_Int32 parameter, sal_Int64 aVal)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setInt64(parameter, aVal);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setLong", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setNull(sal_Int32 parameter, sal_Int32 sqlType)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setNull(parameter, sqlType);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setNull", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setClob(sal_Int32 parameter, const Reference< XClob >& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setClob", *this);
}

void SAL_CALL OPreparedStatement::setBlob(sal_Int32 parameter, const Reference< XBlob >& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBlob", *this);
}

void SAL_CALL OPreparedStatement::setArray(sal_Int32 parameter, const Reference< XArray >& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setArray", *this);
}

void SAL_CALL OPreparedStatement::setRef(sal_Int32 parameter, const Reference< XRef >& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setRef", *this);
}

namespace
{
    template < class COMPLEXTYPE >
    bool impl_setObject( const Reference< XParameters >& _rxParam, sal_Int32 _parameterIndex, const Any& _value,
        void ( SAL_CALL XParameters::*Setter )( sal_Int32, const COMPLEXTYPE& ), bool _throwIfNotExtractable )
    {
        COMPLEXTYPE aValue;
        if ( _value >>= aValue )
        {
            (_rxParam.get()->*Setter)( _parameterIndex, aValue );
            return true;
        }

        if ( _throwIfNotExtractable )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", _rxParam );
        return false;
    }

    template < class INTTYPE >
    void impl_setObject( const Reference< XParameters >& _rxParam, sal_Int32 _parameterIndex, const Any& _value,
        void ( SAL_CALL XParameters::*Setter )( sal_Int32, INTTYPE ) )
    {
        sal_Int32 nValue(0);
        if ( !( _value >>= nValue ) )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", _rxParam );
        (_rxParam.get()->*Setter)( _parameterIndex, static_cast<INTTYPE>(nValue) );
    }
}

void SAL_CALL OPreparedStatement::setObjectWithInfo(sal_Int32 _parameterIndex, const Any& _value, sal_Int32 _targetSqlType, sal_Int32 /* scale */)
{
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkParameterIndex( _parameterIndex );

    if ( !_value.hasValue() )
    {
        setNull( _parameterIndex, _targetSqlType );
        return;
    }

    switch ( _targetSqlType )
    {
    case DataType::DECIMAL:
    case DataType::NUMERIC:
    {
        double nValue(0);
        if ( _value >>= nValue )
        {
            setDouble( _parameterIndex, nValue );
            break;
        }
#if defined __GNUC__ && __GNUC__ >= 7
        [[fallthrough]];
#else
        BOOST_FALLTHROUGH;
#endif
    }

    case DataType::CHAR:
    case DataType::VARCHAR:
    case DataType::LONGVARCHAR:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setString, true );
        break;

    case DataType::BIGINT:
    {
        sal_Int64 nValue = 0;
        if ( !( _value >>= nValue ) )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", *this );
        setLong( _parameterIndex, nValue );
    }
    break;

    case DataType::FLOAT:
    case DataType::REAL:
    {
        float nValue = 0;
        if ( _value >>= nValue )
        {
            setFloat(_parameterIndex,nValue);
            break;
        }
#if defined __GNUC__ && __GNUC__ >= 7
        [[fallthrough]];
#else
        BOOST_FALLTHROUGH;
#endif
    }

    case DataType::DOUBLE:
    {
        double nValue(0);
        if ( !( _value >>= nValue ) )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", *this );
        setDouble( _parameterIndex, nValue );
    }
    break;

    case DataType::DATE:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setDate, true );
        break;

    case DataType::TIME:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setTime, true );
        break;

    case DataType::TIMESTAMP:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setTimestamp, true );
        break;

    case DataType::BINARY:
    case DataType::VARBINARY:
    case DataType::LONGVARBINARY:
    {
        if  (   impl_setObject( this, _parameterIndex, _value, &XParameters::setBytes, false )
            ||  impl_setObject( this, _parameterIndex, _value, &XParameters::setBlob, false )
            ||  impl_setObject( this, _parameterIndex, _value, &XParameters::setClob, false )
            )
            break;

        Reference< css::io::XInputStream > xBinStream;
        if ( _value >>= xBinStream )
        {
            setBinaryStream( _parameterIndex, xBinStream, xBinStream->available() );
            break;
        }

        mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", *this );
    }
    break;

    case DataType::BIT:
    case DataType::BOOLEAN:
    {
        bool bValue( false );
        if ( _value >>= bValue )
        {
            setBoolean( _parameterIndex, bValue );
            break;
        }
        sal_Int32 nValue( 0 );
        if ( _value >>= nValue )
        {
            setBoolean( _parameterIndex, ( nValue != 0 ) );
            break;
        }
        mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", *this );
    }
    break;

    case DataType::TINYINT:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setByte );
        break;

    case DataType::SMALLINT:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setShort );
        break;

    case DataType::INTEGER:
        impl_setObject( this, _parameterIndex, _value, &XParameters::setInt );
        break;

    default:
        mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", *this );
        break;
    }
}

void SAL_CALL OPreparedStatement::setObjectNull(sal_Int32 parameter, sal_Int32 /* sqlType */, const rtl::OUString& /* typeName */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObjectNull", *this);
}

void SAL_CALL OPreparedStatement::setObject(sal_Int32 parameter, const Any& /* x */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObject", *this);
}

void SAL_CALL OPreparedStatement::setShort(sal_Int32 parameter, sal_Int16 x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setShort", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setBytes(sal_Int32 parameter, const Sequence< sal_Int8 >& x)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    std::string blobby(reinterpret_cast<char const *>(x.getConstArray()), x.getLength());
    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->setString(parameter, blobby);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBytes", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::setCharacterStream(sal_Int32 parameter,
                                                    const Reference< XInputStream >& /* x */,
                                                    sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setCharacterStream", *this);
}

void SAL_CALL OPreparedStatement::setBinaryStream(sal_Int32 parameter,
                                                const Reference< XInputStream >& /* x */,
                                                sal_Int32 /* length */)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBinaryStream", *this);
}

void SAL_CALL OPreparedStatement::clearParameters()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    try {
        static_cast<sql::PreparedStatement *>(cppStatement)->clearParameters();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearParameters", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_xConnection->getConnectionEncoding());
    }
}

void SAL_CALL OPreparedStatement::clearBatch()
{
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearBatch", *this);
}

void SAL_CALL OPreparedStatement::addBatch()
{
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::addBatch", *this);
}

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch()
{
    Sequence< sal_Int32 > aRet= Sequence< sal_Int32 > ();
    return aRet;
}

void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
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
            /* XXX: Recursion ?? */
            OPreparedStatement::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}

void OPreparedStatement::checkParameterIndex(sal_Int32 column)
{
    if (column < 1 || column > static_cast<sal_Int32>(m_paramCount)) {
        throw SQLException("Parameter index out of range", *this, rtl::OUString(), 1, Any ());
    }
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
