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

#include "mysqlc_propertyids.hxx"
#include "mysqlc_general.hxx"
#include "mysqlc_resultset.hxx"
#include "mysqlc_resultsetmetadata.hxx"

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::mysqlc;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using ::osl::MutexGuard;

#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>

#include <stdio.h>


//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
/* {{{ OResultSet::getImplementationName() -I- */
OUString SAL_CALL OResultSet::getImplementationName()
    throw (RuntimeException)
{
    OSL_TRACE("OResultSet::getImplementationName");
    return OUString( "com.sun.star.sdbcx.mysqlc.ResultSet" );
}
/* }}} */


/* {{{ OResultSet::getSupportedServiceNames() -I- */
Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames()
    throw(RuntimeException)
{
    OSL_TRACE("OResultSet::getSupportedServiceNames");
    Sequence< OUString > aSupported(2);
    aSupported[0] = OUString( "com.sun.star.sdbc.ResultSet" );
    aSupported[1] = OUString( "com.sun.star.sdbcx.ResultSet" );
    return (aSupported);
}
/* }}} */


/* {{{ OResultSet::supportsService() -I- */
sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
    throw(RuntimeException)
{
    OSL_TRACE("OResultSet::supportsService");
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported) {}

    return (pSupported != pEnd);
}
/* }}} */


/* {{{ OResultSet::OResultSet() -I- */
OResultSet::OResultSet(OCommonStatement * pStmt, sql::ResultSet * result, rtl_TextEncoding _encoding )
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_aStatement((OWeakObject*)pStmt)
    ,m_xMetaData(NULL)
    ,m_result(result)
    ,fieldCount( 0 )
    ,m_encoding( _encoding )
{
    OSL_TRACE("OResultSet::OResultSet");
    try {
        sql::ResultSetMetaData * rs_meta = m_result->getMetaData();
        fieldCount = rs_meta->getColumnCount();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
}
/* }}} */


/* {{{ OResultSet::~OResultSet() -I- */
OResultSet::~OResultSet()
{
    OSL_TRACE("OResultSet::~OResultSet");
}
/* }}} */


/* {{{ OResultSet::disposing() -I- */
void OResultSet::disposing()
{
    OSL_TRACE("OResultSet::disposing");
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    m_aStatement = NULL;
    m_xMetaData  = NULL;
}
/* }}} */


/* {{{ OResultSet::queryInterface() -I- */
Any SAL_CALL OResultSet::queryInterface(const Type & rType)
    throw(RuntimeException)
{
    OSL_TRACE("OResultSet::queryInterface");
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue()) {
        aRet = OResultSet_BASE::queryInterface(rType);
    }
    return aRet;
}
/* }}} */


/* {{{ OResultSet::getTypes() -I- */
Sequence< Type > SAL_CALL OResultSet::getTypes()
    throw(RuntimeException)
{
    OSL_TRACE("OResultSet::getTypes");
    OTypeCollection aTypes( ::getCppuType((const  Reference< XMultiPropertySet > *) NULL),
                                                ::getCppuType((const Reference< XFastPropertySet > *) NULL),
                                                ::getCppuType((const Reference< XPropertySet > *) NULL));

    return concatSequences(aTypes.getTypes(), OResultSet_BASE::getTypes());
}
/* }}} */


/* {{{ OResultSet::findColumn() -I- */
sal_Int32 SAL_CALL OResultSet::findColumn(const OUString& columnName)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::findColumn");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        // find the first column with the name columnName
        sql::ResultSetMetaData * meta = m_result->getMetaData();
        for (sal_uInt32 i = 1; i <= fieldCount; i++) {
            if (columnName.equalsIgnoreAsciiCaseAscii(meta->getColumnName(i).c_str())) {
                /* SDBC knows them indexed from 1 */
                return i;
            }
        }
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    throw SQLException(
        "The column name '" + columnName + "' is not valid.",
        *this,
        OUString("42S22"),
        0,
        Any()
    );
}
/* }}} */


/* {{{ OResultSet::getBinaryStream() -U- */
Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getBinaryStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBinaryStream", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getCharacterStream() -U- */
Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getCharacterStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getCharacterStream", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getBoolean() -I- */
sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getBoolean");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getBoolean(column)? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False;
}
/* }}} */


/* {{{ OResultSet::getByte() -I- */
sal_Int8 SAL_CALL OResultSet::getByte(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getByte");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getInt(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getBytes() -I- */
Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getBytes");

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);


    sql::SQLString val = m_result->getString(column);
    if (!val.length()) {
        return Sequence< sal_Int8>();
    } else {
        return Sequence< sal_Int8 > ((sal_Int8*)val.c_str(), val.length());
    }
}
/* }}} */


/* {{{ OResultSet::getDate() -I- */
Date SAL_CALL OResultSet::getDate(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getDate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Date d;
    try {
        OUString dateString = getString(column);
        OUString token;
        sal_Int32 nIndex = 0, i=0;

        do {
            token = dateString.getToken (0, '-', nIndex);
            switch (i) {
                case 0:
                    d.Year =  static_cast<sal_uInt16>(token.toUInt32());
                    break;
                case 1:
                    d.Month =  static_cast<sal_uInt16>(token.toUInt32());
                    break;
                case 2:
                    d.Day =  static_cast<sal_uInt16>(token.toUInt32());
                    break;
                default:;
            }
            i++;
        } while (nIndex >= 0);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return d;
}
/* }}} */


/* {{{ OResultSet::getDouble() -I- */
double SAL_CALL OResultSet::getDouble(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getDouble");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getDouble(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0.0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getFloat() -I- */
float SAL_CALL OResultSet::getFloat(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getFloat");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getDouble(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0.0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getInt() -I- */
sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getInt");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getInt(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getRow() -I- */
sal_Int32 SAL_CALL OResultSet::getRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->getRow();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getLong() -I- */
sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getLong");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getInt64(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getMetaData() -I- */
Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getMetaData");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    try {
        if (!m_xMetaData.is()) {
            m_xMetaData = new OResultSetMetaData(m_result->getMetaData(), m_encoding);
        }
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return m_xMetaData;
}
/* }}} */


/* {{{ OResultSet::getArray() -U- */
Reference< XArray > SAL_CALL OResultSet::getArray(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getArray");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getArray", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getClob() -U- */
Reference< XClob > SAL_CALL OResultSet::getClob(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getClob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getClob", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getBlob() -U- */
Reference< XBlob > SAL_CALL OResultSet::getBlob(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getBlob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBlob", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getRef() -U- */
Reference< XRef > SAL_CALL OResultSet::getRef(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getRef");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getRef", *this);
    return NULL;
}
/* }}} */


/* {{{ OResultSet::getObject() -U- */
Any SAL_CALL OResultSet::getObject(sal_Int32 column, const Reference< XNameAccess >& /* typeMap */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Any aRet= Any();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getObject", *this);
    return aRet;
}
/* }}} */


/* {{{ OResultSet::getShort() -I- */
sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getShort");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return (sal_Int16) m_result->getInt(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}
/* }}} */


/* {{{ OResultSet::getString() -I- */
OUString SAL_CALL OResultSet::getString(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getString");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);

    try {
        sql::SQLString val = m_result->getString(column);
        if (!m_result->wasNull()) {
            return OUString( val.c_str(), val.length(), m_encoding );
        } else {
            return OUString();
        }
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return OUString(); // fool compiler
}
/* }}} */


/* {{{ OResultSet::getTime() -I- */
Time SAL_CALL OResultSet::getTime(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getTime");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    checkColumnIndex(column);
    Time t;
    OUString timeString = getString(column);
    OUString token;
    sal_Int32 nIndex, i=0;

    nIndex = timeString.indexOf(' ') + 1;

    do {
        token = timeString.getToken (0, ':', nIndex);
        switch (i) {
            case 0:
                t.Hours =  static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 1:
                t.Minutes =  static_cast<sal_uInt16>(token.toUInt32());
                break;
            case 2:
                t.Seconds =  static_cast<sal_uInt16>(token.toUInt32());
                break;
        }
        i++;
    } while (nIndex >= 0);

    return t;
}
/* }}} */


/* {{{ OResultSet::getTimestamp() -I- */
DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getTimestamp");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    checkColumnIndex(column);
    DateTime dt;
    Date d = getDate(column);
    Time t = getTime(column);

    dt.Year = d.Year;
    dt.Month = d.Month;
    dt.Day = d.Day;
    dt.Hours = t.Hours;
    dt.Minutes = t.Minutes;
    dt.Seconds = t.Seconds;
    return dt;
}
/* }}} */


/* {{{ OResultSet::isBeforeFirst() -I- */
sal_Bool SAL_CALL OResultSet::isBeforeFirst()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::isBeforeFirst");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isBeforeFirst()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::isAfterLast() -I- */
sal_Bool SAL_CALL OResultSet::isAfterLast()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::isAfterLast");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isAfterLast()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::isFirst() -I- */
sal_Bool SAL_CALL OResultSet::isFirst()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::isFirst");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isFirst();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::isLast() -I- */
sal_Bool SAL_CALL OResultSet::isLast()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::isLast");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isLast()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::beforeFirst() -I- */
void SAL_CALL OResultSet::beforeFirst()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::beforeFirst");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        m_result->beforeFirst();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
}
/* }}} */


/* {{{ OResultSet::afterLast() -I- */
void SAL_CALL OResultSet::afterLast()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::afterLast");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        m_result->afterLast();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
}
/* }}} */


/* {{{ OResultSet::close() -I- */
void SAL_CALL OResultSet::close() throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::close");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        m_result->close();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }

    dispose();
}
/* }}} */


/* {{{ OResultSet::first() -I- */
sal_Bool SAL_CALL OResultSet::first() throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::first");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->first()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::last() -I- */
sal_Bool SAL_CALL OResultSet::last()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::last");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->last()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::absolute() -I- */
sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 row)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::absolute");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->absolute(row)? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::relative() -I- */
sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::relative");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->relative(row)? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::previous() -I- */
sal_Bool SAL_CALL OResultSet::previous()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::previous");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->previous()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::getStatement() -I- */
Reference< XInterface > SAL_CALL OResultSet::getStatement()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getStatement");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_aStatement.get();
}
/* }}} */


/* {{{ OResultSet::rowDeleted() -I- */
sal_Bool SAL_CALL OResultSet::rowDeleted()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::rowDeleted");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
/* }}} */


/* {{{ OResultSet::rowInserted() -I- */
sal_Bool SAL_CALL OResultSet::rowInserted()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::rowInserted");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
/* }}} */


/* {{{ OResultSet::rowUpdated() -I- */
sal_Bool SAL_CALL OResultSet::rowUpdated()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::rowUpdated");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
/* }}} */


/* {{{ OResultSet::next() -I- */
sal_Bool SAL_CALL OResultSet::next()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::next");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->next()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::wasNull() -I- */
sal_Bool SAL_CALL OResultSet::wasNull()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::wasNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->wasNull()? sal_True:sal_False;
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}
/* }}} */


/* {{{ OResultSet::cancel() -I- */
void SAL_CALL OResultSet::cancel()
    throw(RuntimeException)
{
    OSL_TRACE("OResultSet::cancel");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
/* }}} */


/* {{{ OResultSet::clearWarnings() -I- */
void SAL_CALL OResultSet::clearWarnings()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::clearWarnings");
}
/* }}} */


/* {{{ OResultSet::getWarnings() -I- */
Any SAL_CALL OResultSet::getWarnings()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getWarnings");
    Any aRet= Any();
    return aRet;
}
/* }}} */


/* {{{ OResultSet::insertRow() -I- */
void SAL_CALL OResultSet::insertRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::insertRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    // you only have to implement this if you want to insert new rows
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::insertRow", *this);
}
/* }}} */


/* {{{ OResultSet::updateRow() -I- */
void SAL_CALL OResultSet::updateRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateRow", *this);
}
/* }}} */


/* {{{ OResultSet::deleteRow() -I- */
void SAL_CALL OResultSet::deleteRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::deleteRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRow", *this);
}
/* }}} */


/* {{{ OResultSet::cancelRowUpdates() -I- */
void SAL_CALL OResultSet::cancelRowUpdates()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::cancelRowUpdates");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::cancelRowUpdates", *this);
}
/* }}} */


/* {{{ OResultSet::moveToInsertRow() -I- */
void SAL_CALL OResultSet::moveToInsertRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::moveToInsertRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow insert's
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveToInsertRow", *this);
}
/* }}} */


/* {{{ OResultSet::moveToCurrentRow() -I- */
void SAL_CALL OResultSet::moveToCurrentRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::moveToCurrentRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
/* }}} */


/* {{{ OResultSet::updateNull() -U- */
void SAL_CALL OResultSet::updateNull(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNull", *this);
}
/* }}} */


/* {{{ OResultSet::updateBoolean() -U- */
void SAL_CALL OResultSet::updateBoolean(sal_Int32 column, sal_Bool /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateBoolean");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBoolean", *this);
}
/* }}} */


/* {{{ OResultSet::updateByte() -U- */
void SAL_CALL OResultSet::updateByte(sal_Int32 column, sal_Int8 /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateByte");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateByte", *this);
}
/* }}} */


/* {{{ OResultSet::updateShort() -U- */
void SAL_CALL OResultSet::updateShort(sal_Int32 column, sal_Int16 /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateShort");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateShort", *this);
}
/* }}} */


/* {{{ OResultSet::updateInt() -U- */
void SAL_CALL OResultSet::updateInt(sal_Int32 column, sal_Int32 /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateInt");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateInt", *this);
}
/* }}} */


/* {{{ OResultSet::updateLong() -U- */
void SAL_CALL OResultSet::updateLong(sal_Int32 column, sal_Int64 /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateLong");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateLong", *this);
}
/* }}} */


/* {{{ OResultSet::updateFloat() -U- */
void SAL_CALL OResultSet::updateFloat(sal_Int32 column, float /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateFloat");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateFloat", *this);
}
/* }}} */


/* {{{ OResultSet::updateDouble() -U- */
void SAL_CALL OResultSet::updateDouble(sal_Int32 column, double /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateDouble");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDouble", *this);
}
/* }}} */


/* {{{ OResultSet::updateString() -U- */
void SAL_CALL OResultSet::updateString(sal_Int32 column, const OUString& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateString");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateString", *this);
}
/* }}} */


/* {{{ OResultSet::updateBytes() -U- */
void SAL_CALL OResultSet::updateBytes(sal_Int32 column, const Sequence< sal_Int8 >& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateBytes");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBytes", *this);
}
/* }}} */


/* {{{ OResultSet::updateDate() -U- */
void SAL_CALL OResultSet::updateDate(sal_Int32 column, const Date& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateDate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDate", *this);
}
/* }}} */


/* {{{ OResultSet::updateTime() -U- */
void SAL_CALL OResultSet::updateTime(sal_Int32 column, const Time& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateTime");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTime", *this);
}
/* }}} */


/* {{{ OResultSet::updateTimestamp() -U- */
void SAL_CALL OResultSet::updateTimestamp(sal_Int32 column, const DateTime& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateTimestamp");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTimestamp", *this);
}
/* }}} */


/* {{{ OResultSet::updateBinaryStream() -U- */
void SAL_CALL OResultSet::updateBinaryStream(sal_Int32 column, const Reference< XInputStream >& /* x */,
                                            sal_Int32 /* length */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateBinaryStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBinaryStream", *this);
}
/* }}} */


/* {{{ OResultSet::updateCharacterStream() -U- */
void SAL_CALL OResultSet::updateCharacterStream(sal_Int32 column, const Reference< XInputStream >& /* x */,
                                                sal_Int32 /* length */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateCharacterStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateCharacterStream", *this);
}
/* }}} */


/* {{{ OResultSet::refreshRow() -U- */
void SAL_CALL OResultSet::refreshRow()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::refreshRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::refreshRow", *this);
}
/* }}} */


/* {{{ OResultSet::updateObject() -U- */
void SAL_CALL OResultSet::updateObject(sal_Int32 column, const Any& /* x */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateObject", *this);
}
/* }}} */


/* {{{ OResultSet::updateNumericObject() -U- */
void SAL_CALL OResultSet::updateNumericObject(sal_Int32 column, const Any& /* x */, sal_Int32 /* scale */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::updateNumericObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNumericObject", *this);
}
/* }}} */


// XRowLocate
/* {{{ OResultSet::getBookmark() -U- */
Any SAL_CALL OResultSet::getBookmark()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::getBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    Any aRet = Any();

    // if you don't want to support bookmark you must remove the XRowLocate interface
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBookmark", *this);

    return aRet;
}
/* }}} */


/* {{{ OResultSet::moveToBookmark() -U- */
sal_Bool SAL_CALL OResultSet::moveToBookmark(const Any& /* bookmark */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::moveToBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
/* }}} */


/* {{{ OResultSet::moveRelativeToBookmark() -U- */
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark(const Any& /* bookmark */, sal_Int32 /* rows */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::moveRelativeToBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveRelativeToBookmark", *this);
    return sal_False;
}
/* }}} */


/* {{{ OResultSet::compareBookmarks() -I- */
sal_Int32 SAL_CALL OResultSet::compareBookmarks(const Any& /* n1 */, const Any& /* n2 */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::compareBookmarks");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::compareBookmarks", *this);

    return CompareBookmark::NOT_EQUAL;
}
/* }}} */


/* {{{ OResultSet::hasOrderedBookmarks() -I- */
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks()
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::hasOrderedBookmarks");
    return sal_False;
}
/* }}} */


/* {{{ OResultSet::hashBookmark() -U- */
sal_Int32 SAL_CALL OResultSet::hashBookmark(const Any& /* bookmark */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::hashBookmark");
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::hashBookmark", *this);
    return 0;
}
/* }}} */


// XDeleteRows
/* {{{ OResultSet::deleteRows() -U- */
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows(const Sequence< Any >& /* rows */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::deleteRows");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    Sequence< sal_Int32 > aRet = Sequence< sal_Int32 >();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRows", *this);
    return aRet;
}
/* }}} */


/* {{{ OResultSet::createArrayHelper() -I- */
IPropertyArrayHelper * OResultSet::createArrayHelper() const
{
    OSL_TRACE("OResultSet::createArrayHelper");
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
/* }}} */


/* {{{ OResultSet::getInfoHelper() -I- */
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    OSL_TRACE("OResultSet::getInfoHelper");
    return (*const_cast<OResultSet*>(this)->getArrayHelper());
}
/* }}} */


/* {{{ OResultSet::convertFastPropertyValue() -I- */
sal_Bool OResultSet::convertFastPropertyValue(Any & /* rConvertedValue */,
                                            Any & /* rOldValue */,
                                            sal_Int32 nHandle,
                                            const Any& /* rValue */)
    throw (::com::sun::star::lang::IllegalArgumentException)
{
    OSL_TRACE("OResultSet::convertFastPropertyValue");
    switch (nHandle) {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}
/* }}} */


/* {{{ OResultSet::setFastPropertyValue_NoBroadcast() -I- */
void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& /* rValue */)
    throw (Exception)
{
    OSL_TRACE("OResultSet::setFastPropertyValue_NoBroadcast");
    switch (nHandle) {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:
            ;
    }
}
/* }}} */


/* {{{ OResultSet::getFastPropertyValue() -I- */
void OResultSet::getFastPropertyValue(Any& _rValue, sal_Int32 nHandle) const
{
    OSL_TRACE("OResultSet::getFastPropertyValue");
    switch (nHandle) {
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rValue <<= sal_False;
            break;
        case PROPERTY_ID_CURSORNAME:
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            _rValue <<= ResultSetConcurrency::READ_ONLY;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            _rValue <<= ResultSetType::SCROLL_INSENSITIVE;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            _rValue <<= FetchDirection::FORWARD;
            break;
        case PROPERTY_ID_FETCHSIZE:
            _rValue <<= sal_Int32(50);
            break;
            ;
        default:
            ;
    }
}
/* }}} */


/* {{{ OResultSet::acquire() -I- */
void SAL_CALL OResultSet::acquire()
    throw()
{
    OSL_TRACE("OResultSet::acquire");
    OResultSet_BASE::acquire();
}
/* }}} */


/* {{{ OResultSet::release() -I- */
void SAL_CALL OResultSet::release()
    throw()
{
    OSL_TRACE("OResultSet::release");
    OResultSet_BASE::release();
}
/* }}} */


/* {{{ OResultSet::getPropertySetInfo() -I- */
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE("OResultSet::getPropertySetInfo");
    return (::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper()));
}
/* }}} */


/* {{{ OResultSet::checkColumnIndex() -I- */
void OResultSet::checkColumnIndex(sal_Int32 index)
    throw (SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::checkColumnIndex");
    if ((index < 1 || index > (int) fieldCount)) {
        /* static object for efficiency or thread safety is a problem ? */
        OUString buf( "index out of range" );
        throw SQLException(buf, *this, OUString(), 1, Any());
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
