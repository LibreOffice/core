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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/parameter_metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>

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
using mysqlc_sdbc_driver::getStringFromAny;


/* {{{ my_i_to_a() -I- */
static inline char * my_i_to_a(char * buf, size_t buf_size, int a)
{
    snprintf(buf, buf_size, "%d", a);
    return buf;
}
/* }}} */


IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.mysqlc.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


/* {{{ OPreparedStatement::OPreparedStatement() -I- */
OPreparedStatement::OPreparedStatement(OConnection* _pConnection, sql::PreparedStatement * _cppPrepStmt)
    :OCommonStatement(_pConnection, _cppPrepStmt)
{
    OSL_TRACE("OPreparedStatement::OPreparedStatement");
    m_pConnection = _pConnection;
    m_pConnection->acquire();

    try {
        m_paramCount = ((sql::PreparedStatement *)cppStatement)->getParameterMetaData()->getParameterCount();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::~OPreparedStatement() -I- */
OPreparedStatement::~OPreparedStatement()
{
    OSL_TRACE("OPreparedStatement::~OPreparedStatement");
}
/* }}} */


/* {{{ OPreparedStatement::acquire() -I- */
void SAL_CALL OPreparedStatement::acquire()
    throw()
{
    OSL_TRACE("OPreparedStatement::acquire");
    OCommonStatement::acquire();
}
/* }}} */


/* {{{ OPreparedStatement::release() -I- */
void SAL_CALL OPreparedStatement::release()
    throw()
{
    OSL_TRACE("OPreparedStatement::release");
    OCommonStatement::release();
}
/* }}} */


/* {{{ OPreparedStatement::queryInterface() -I- */
Any SAL_CALL OPreparedStatement::queryInterface(const Type & rType)
    throw(RuntimeException)
{
    OSL_TRACE("OPreparedStatement::queryInterface");
    Any aRet = OCommonStatement::queryInterface(rType);
    if (!aRet.hasValue()) {
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    }
    return (aRet);
}
/* }}} */


/* {{{ OPreparedStatement::getPropertySetInfo() -I- */
Sequence< Type > SAL_CALL OPreparedStatement::getTypes()
    throw(RuntimeException)
{
    OSL_TRACE("OPreparedStatement::getTypes");
    return concatSequences(OPreparedStatement_BASE::getTypes(), OCommonStatement::getTypes());
}
/* }}} */


/* {{{ OPreparedStatement::getMetaData() -I- */
Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::getMetaData");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    try {
        if (!m_xMetaData.is()) {
            m_xMetaData = new OResultSetMetaData(
                                    ((sql::PreparedStatement *)cppStatement)->getMetaData(),
                                    getOwnConnection()->getConnectionEncoding()
                                );
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return m_xMetaData;
}
/* }}} */


/* {{{ OPreparedStatement::close() -I- */
void SAL_CALL OPreparedStatement::close()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::close");

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
/* }}} */


/* {{{ OPreparedStatement::execute() -I- */
sal_Bool SAL_CALL OPreparedStatement::execute()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::execute");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    sal_Bool success = sal_False;
    try {
        success = ((sql::PreparedStatement *)cppStatement)->execute()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return success;
}
/* }}} */


/* {{{ OPreparedStatement::executeUpdate() -I- */
sal_Int32 SAL_CALL OPreparedStatement::executeUpdate()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::executeUpdate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    sal_Int32 affectedRows = sal_False;
    try {
        affectedRows = ((sql::PreparedStatement *)cppStatement)->executeUpdate();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return affectedRows;
}
/* }}} */


/* {{{ OPreparedStatement::getPropertySetInfo() -I- */
void SAL_CALL OPreparedStatement::setString(sal_Int32 parameter, const OUString& x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setString");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        std::string stringie(::rtl::OUStringToOString(x, m_pConnection->getConnectionEncoding()).getStr());
        ((sql::PreparedStatement *)cppStatement)->setString(parameter, stringie);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearParameters", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::getConnection() -I- */
Reference< XConnection > SAL_CALL OPreparedStatement::getConnection()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::getConnection");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
/* }}} */

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    return OCommonStatement::executeQuery( sql );
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    return OCommonStatement::executeUpdate( sql );
}

sal_Bool SAL_CALL OPreparedStatement::execute( const OUString& sql )
    throw(SQLException, RuntimeException)
{
    return OCommonStatement::execute( sql );
}

/* {{{ OPreparedStatement::executeQuery() -I- */
Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::executeQuery");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    Reference< XResultSet > xResultSet;
    try {
        sql::ResultSet * res = ((sql::PreparedStatement *)cppStatement)->executeQuery();
        xResultSet = new OResultSet(this, res, getOwnConnection()->getConnectionEncoding());
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
    return xResultSet;
}
/* }}} */


/* {{{ OPreparedStatement::setBoolean() -I- */
void SAL_CALL OPreparedStatement::setBoolean(sal_Int32 parameter, sal_Bool x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setBoolean");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setBoolean(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBoolean", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setByte() -I- */
void SAL_CALL OPreparedStatement::setByte(sal_Int32 parameter, sal_Int8 x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setByte");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setByte", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setDate() -I- */
void SAL_CALL OPreparedStatement::setDate(sal_Int32 parameter, const Date& aData)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setDate");
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
        ((sql::PreparedStatement *)cppStatement)->setDateTime(parameter, dateStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setDate", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setTime() -I- */
void SAL_CALL OPreparedStatement::setTime(sal_Int32 parameter, const Time& aVal)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setTime");
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
        ((sql::PreparedStatement *)cppStatement)->setDateTime(parameter, timeStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setTime", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setTimestamp() -I- */
void SAL_CALL OPreparedStatement::setTimestamp(sal_Int32 parameter, const DateTime& aVal)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setTimestamp");
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
        ((sql::PreparedStatement *)cppStatement)->setDateTime(parameter, timeStr);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setTimestamp", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setDouble() -I- */
void SAL_CALL OPreparedStatement::setDouble(sal_Int32 parameter, double x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setDouble");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setDouble(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setDouble", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setFloat() -I- */
void SAL_CALL OPreparedStatement::setFloat(sal_Int32 parameter, float x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setFloat");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setDouble(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setFloat", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setInt() -I- */
void SAL_CALL OPreparedStatement::setInt(sal_Int32 parameter, sal_Int32 x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setInt");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setInt", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setLong() -I- */
void SAL_CALL OPreparedStatement::setLong(sal_Int32 parameter, sal_Int64 aVal)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setLong");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setInt64(parameter, aVal);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setLong", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setNull() -I- */
void SAL_CALL OPreparedStatement::setNull(sal_Int32 parameter, sal_Int32 sqlType)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setNull(parameter, sqlType);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setNull", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setClob() -U- */
void SAL_CALL OPreparedStatement::setClob(sal_Int32 parameter, const Reference< XClob >& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setClob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setClob", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setBlob() -U- */
void SAL_CALL OPreparedStatement::setBlob(sal_Int32 parameter, const Reference< XBlob >& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setBlob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBlob", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setArray() -U- */
void SAL_CALL OPreparedStatement::setArray(sal_Int32 parameter, const Reference< XArray >& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setArray");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setArray", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setRef() -U- */
void SAL_CALL OPreparedStatement::setRef(sal_Int32 parameter, const Reference< XRef >& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setRef");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setRef", *this);
}
/* }}} */

namespace
{
    template < class COMPLEXTYPE >
    bool impl_setObject( const Reference< XParameters >& _rxParam, sal_Int32 _parameterIndex, const Any& _value,
        void ( SAL_CALL XParameters::*_Setter )( sal_Int32, const COMPLEXTYPE& ), bool _throwIfNotExtractable )
    {
        COMPLEXTYPE aValue;
        if ( _value >>= aValue )
        {
            (_rxParam.get()->*_Setter)( _parameterIndex, aValue );
            return true;
        }

        if ( _throwIfNotExtractable )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", _rxParam );
        return false;
    }

    template < class INTTYPE >
    void impl_setObject( const Reference< XParameters >& _rxParam, sal_Int32 _parameterIndex, const Any& _value,
        void ( SAL_CALL XParameters::*_Setter )( sal_Int32, INTTYPE ) )
    {
        sal_Int32 nValue(0);
        if ( !( _value >>= nValue ) )
            mysqlc_sdbc_driver::throwInvalidArgumentException( "OPreparedStatement::setObjectWithInfo", _rxParam );
        (_rxParam.get()->*_Setter)( _parameterIndex, (INTTYPE)nValue );
    }
}

/* {{{ OPreparedStatement::setObjectWithInfo() -U- */
void SAL_CALL OPreparedStatement::setObjectWithInfo(sal_Int32 _parameterIndex, const Any& _value, sal_Int32 _targetSqlType, sal_Int32 /* scale */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setObjectWithInfo");
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
    }
    // run through

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
    }
    // run through if we couldn't set a float value

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

        Reference< ::com::sun::star::io::XInputStream > xBinStream;
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
/* }}} */


/* {{{ OPreparedStatement::setObjectNull() -U- */
void SAL_CALL OPreparedStatement::setObjectNull(sal_Int32 parameter, sal_Int32 /* sqlType */, const OUString& /* typeName */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setObjectNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObjectNull", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setObject() -U- */
void SAL_CALL OPreparedStatement::setObject(sal_Int32 parameter, const Any& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setObject", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setShort() -I- */
void SAL_CALL OPreparedStatement::setShort(sal_Int32 parameter, sal_Int16 x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setShort");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    try {
        ((sql::PreparedStatement *)cppStatement)->setInt(parameter, x);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setShort", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setBytes() -I- */
void SAL_CALL OPreparedStatement::setBytes(sal_Int32 parameter, const Sequence< sal_Int8 >& x)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setBytes");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    std::string blobby((char *)x.getConstArray(), x.getLength());
    try {
        ((sql::PreparedStatement *)cppStatement)->setString(parameter, blobby);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBytes", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::setCharacterStream() -U- */
void SAL_CALL OPreparedStatement::setCharacterStream(sal_Int32 parameter,
                                                    const Reference< XInputStream >& /* x */,
                                                    sal_Int32 /* length */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setCharacterStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setCharacterStream", *this);
}
/* }}} */


/* {{{ OPreparedStatement::setBinaryStream() -U- */
void SAL_CALL OPreparedStatement::setBinaryStream(sal_Int32 parameter,
                                                const Reference< XInputStream >& /* x */,
                                                sal_Int32 /* length */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::setBinaryStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);
    checkParameterIndex(parameter);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::setBinaryStream", *this);
}
/* }}} */


/* {{{ OPreparedStatement::clearParameters() -I- */
void SAL_CALL OPreparedStatement::clearParameters()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::clearParameters");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OPreparedStatement::rBHelper.bDisposed);

    try {
        ((sql::PreparedStatement *)cppStatement)->clearParameters();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearParameters", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_pConnection->getConnectionEncoding());
    }
}
/* }}} */


/* {{{ OPreparedStatement::clearBatch() -U- */
void SAL_CALL OPreparedStatement::clearBatch()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::clearBatch");
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::clearBatch", *this);
}
/* }}} */


/* {{{ OPreparedStatement::addBatch() -U- */
void SAL_CALL OPreparedStatement::addBatch()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::addBatch");
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OPreparedStatement::addBatch", *this);
}
/* }}} */


/* {{{ OPreparedStatement::executeBatch() -I- */
Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OPreparedStatement::executeBatch");
    Sequence< sal_Int32 > aRet= Sequence< sal_Int32 > ();
    return aRet;
}
/* }}} */


/* {{{ OPreparedStatement::setFastPropertyValue_NoBroadcast() -I- */
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
    throw(Exception)
{
    OSL_TRACE("OPreparedStatement::setFastPropertyValue_NoBroadcast");
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
/* }}} */


/* {{{ OPreparedStatement::checkParameterIndex() -I- */
void OPreparedStatement::checkParameterIndex(sal_Int32 column)
{
    OSL_TRACE("OPreparedStatement::checkColumnIndex");
    if (column < 1 || column > (sal_Int32) m_paramCount) {
        OUString buf( RTL_CONSTASCII_USTRINGPARAM( "Parameter index out of range" ) );
        throw SQLException(buf, *this, OUString(), 1, Any ());
    }
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
