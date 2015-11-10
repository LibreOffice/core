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

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

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

rtl::OUString SAL_CALL OResultSet::getImplementationName()
    throw (RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getImplementationName");
    return rtl::OUString( "com.sun.star.sdbcx.mysqlc.ResultSet" );
}

Sequence< rtl::OUString > SAL_CALL OResultSet::getSupportedServiceNames()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getSupportedServiceNames");
    Sequence< rtl::OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const rtl::OUString& _rServiceName)
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

OResultSet::OResultSet(OCommonStatement * pStmt, sql::ResultSet * result, rtl_TextEncoding _encoding )
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_aStatement(static_cast<OWeakObject*>(pStmt))
    ,m_xMetaData(nullptr)
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

OResultSet::~OResultSet()
{
    OSL_TRACE("OResultSet::~OResultSet");
}

void OResultSet::disposing()
{
    OSL_TRACE("OResultSet::disposing");
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    m_aStatement = nullptr;
    m_xMetaData  = nullptr;
}

Any SAL_CALL OResultSet::queryInterface(const Type & rType)
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::queryInterface");
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue()) {
        aRet = OResultSet_BASE::queryInterface(rType);
    }
    return aRet;
}

Sequence< Type > SAL_CALL OResultSet::getTypes()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getTypes");
    OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                                cppu::UnoType<XFastPropertySet>::get(),
                                                cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(), OResultSet_BASE::getTypes());
}

sal_Int32 SAL_CALL OResultSet::findColumn(const rtl::OUString& columnName)
    throw(SQLException, RuntimeException, std::exception)
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
        rtl::OUString("42S22"),
        0,
        Any()
    );
}

Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getBinaryStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBinaryStream", *this);
    return nullptr;
}

Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getCharacterStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getCharacterStream", *this);
    return nullptr;
}

sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getBoolean");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);
    try {
        return m_result->getBoolean(column);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False;
}

sal_Int8 SAL_CALL OResultSet::getByte(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getBytes");

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    sql::SQLString val = m_result->getString(column);
    if (!val.length()) {
        return Sequence< sal_Int8>();
    } else {
        return Sequence< sal_Int8 > (reinterpret_cast<sal_Int8 const *>(val.c_str()), val.length());
    }
}

Date SAL_CALL OResultSet::getDate(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getDate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Date d;
    try {
        rtl::OUString dateString = getString(column);
        rtl::OUString token;
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

double SAL_CALL OResultSet::getDouble(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

float SAL_CALL OResultSet::getFloat(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

sal_Int32 SAL_CALL OResultSet::getRow()
    throw(SQLException, RuntimeException, std::exception)
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

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData()
    throw(SQLException, RuntimeException, std::exception)
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

Reference< XArray > SAL_CALL OResultSet::getArray(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getArray");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getArray", *this);
    return nullptr;
}

Reference< XClob > SAL_CALL OResultSet::getClob(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getClob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getClob", *this);
    return nullptr;
}

Reference< XBlob > SAL_CALL OResultSet::getBlob(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getBlob");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBlob", *this);
    return nullptr;
}

Reference< XRef > SAL_CALL OResultSet::getRef(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getRef");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getRef", *this);
    return nullptr;
}

Any SAL_CALL OResultSet::getObject(sal_Int32 column, const Reference< XNameAccess >& /* typeMap */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);

    Any aRet= Any();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getObject", *this);
    return aRet;
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

rtl::OUString SAL_CALL OResultSet::getString(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getString");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(column);

    try {
        sql::SQLString val = m_result->getString(column);
        if (!m_result->wasNull()) {
            return rtl::OUString( val.c_str(), val.length(), m_encoding );
        } else {
            return rtl::OUString();
        }
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

Time SAL_CALL OResultSet::getTime(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getTime");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);

    checkColumnIndex(column);
    Time t;
    rtl::OUString timeString = getString(column);
    rtl::OUString token;
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

DateTime SAL_CALL OResultSet::getTimestamp(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
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

sal_Bool SAL_CALL OResultSet::isBeforeFirst()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::isBeforeFirst");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isBeforeFirst();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::isAfterLast()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::isAfterLast");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isAfterLast();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::isFirst()
    throw(SQLException, RuntimeException, std::exception)
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

sal_Bool SAL_CALL OResultSet::isLast()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::isLast");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->isLast();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

void SAL_CALL OResultSet::beforeFirst()
    throw(SQLException, RuntimeException, std::exception)
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

void SAL_CALL OResultSet::afterLast()
    throw(SQLException, RuntimeException, std::exception)
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

void SAL_CALL OResultSet::close() throw(SQLException, RuntimeException, std::exception)
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

sal_Bool SAL_CALL OResultSet::first() throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::first");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->first();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::last()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::last");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->last();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 row)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::absolute");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->absolute(row);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::relative");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->relative(row);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::previous()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::previous");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->previous();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

Reference< XInterface > SAL_CALL OResultSet::getStatement()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getStatement");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_aStatement.get();
}

sal_Bool SAL_CALL OResultSet::rowDeleted()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::rowDeleted");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}

sal_Bool SAL_CALL OResultSet::rowInserted()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::rowInserted");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}

sal_Bool SAL_CALL OResultSet::rowUpdated()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::rowUpdated");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}

sal_Bool SAL_CALL OResultSet::next()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::next");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->next();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

sal_Bool SAL_CALL OResultSet::wasNull()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::wasNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    try {
        return m_result->wasNull();
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; //fool
}

void SAL_CALL OResultSet::cancel()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::cancel");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OResultSet::clearWarnings()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::clearWarnings");
}

Any SAL_CALL OResultSet::getWarnings()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getWarnings");
    Any aRet= Any();
    return aRet;
}

void SAL_CALL OResultSet::insertRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::insertRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    // you only have to implement this if you want to insert new rows
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::insertRow", *this);
}

void SAL_CALL OResultSet::updateRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateRow", *this);
}

void SAL_CALL OResultSet::deleteRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::deleteRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRow", *this);
}

void SAL_CALL OResultSet::cancelRowUpdates()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::cancelRowUpdates");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::cancelRowUpdates", *this);
}

void SAL_CALL OResultSet::moveToInsertRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::moveToInsertRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // only when you allow insert's
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveToInsertRow", *this);
}

void SAL_CALL OResultSet::moveToCurrentRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::moveToCurrentRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL OResultSet::updateNull(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateNull");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNull", *this);
}

void SAL_CALL OResultSet::updateBoolean(sal_Int32 column, sal_Bool /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateBoolean");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBoolean", *this);
}

void SAL_CALL OResultSet::updateByte(sal_Int32 column, sal_Int8 /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateByte");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateByte", *this);
}

void SAL_CALL OResultSet::updateShort(sal_Int32 column, sal_Int16 /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateShort");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateShort", *this);
}

void SAL_CALL OResultSet::updateInt(sal_Int32 column, sal_Int32 /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateInt");
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateInt", *this);
}

void SAL_CALL OResultSet::updateLong(sal_Int32 column, sal_Int64 /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateLong");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateLong", *this);
}

void SAL_CALL OResultSet::updateFloat(sal_Int32 column, float /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateFloat");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateFloat", *this);
}

void SAL_CALL OResultSet::updateDouble(sal_Int32 column, double /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateDouble");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDouble", *this);
}

void SAL_CALL OResultSet::updateString(sal_Int32 column, const rtl::OUString& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateString");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateString", *this);
}

void SAL_CALL OResultSet::updateBytes(sal_Int32 column, const Sequence< sal_Int8 >& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateBytes");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBytes", *this);
}

void SAL_CALL OResultSet::updateDate(sal_Int32 column, const Date& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateDate");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateDate", *this);
}

void SAL_CALL OResultSet::updateTime(sal_Int32 column, const Time& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateTime");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTime", *this);
}

void SAL_CALL OResultSet::updateTimestamp(sal_Int32 column, const DateTime& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateTimestamp");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateTimestamp", *this);
}

void SAL_CALL OResultSet::updateBinaryStream(sal_Int32 column, const Reference< XInputStream >& /* x */,
                                            sal_Int32 /* length */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateBinaryStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateBinaryStream", *this);
}

void SAL_CALL OResultSet::updateCharacterStream(sal_Int32 column, const Reference< XInputStream >& /* x */,
                                                sal_Int32 /* length */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateCharacterStream");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateCharacterStream", *this);
}

void SAL_CALL OResultSet::refreshRow()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::refreshRow");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::refreshRow", *this);
}

void SAL_CALL OResultSet::updateObject(sal_Int32 column, const Any& /* x */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateObject", *this);
}

void SAL_CALL OResultSet::updateNumericObject(sal_Int32 column, const Any& /* x */, sal_Int32 /* scale */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::updateNumericObject");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    checkColumnIndex(column);
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::updateNumericObject", *this);
}

// XRowLocate
Any SAL_CALL OResultSet::getBookmark()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    Any aRet = Any();

    // if you don't want to support bookmark you must remove the XRowLocate interface
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::getBookmark", *this);

    return aRet;
}

sal_Bool SAL_CALL OResultSet::moveToBookmark(const Any& /* bookmark */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::moveToBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}

sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark(const Any& /* bookmark */, sal_Int32 /* rows */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::moveRelativeToBookmark");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::moveRelativeToBookmark", *this);
    return sal_False;
}

sal_Int32 SAL_CALL OResultSet::compareBookmarks(const Any& /* n1 */, const Any& /* n2 */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::compareBookmarks");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::compareBookmarks", *this);

    return CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::hasOrderedBookmarks");
    return sal_False;
}

sal_Int32 SAL_CALL OResultSet::hashBookmark(const Any& /* bookmark */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::hashBookmark");
    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::hashBookmark", *this);
    return 0;
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows(const Sequence< Any >& /* rows */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::deleteRows");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    Sequence< sal_Int32 > aRet = Sequence< sal_Int32 >();

    mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSet::deleteRows", *this);
    return aRet;
}

IPropertyArrayHelper * OResultSet::createArrayHelper() const
{
    OSL_TRACE("OResultSet::createArrayHelper");
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = Property("FetchDirection", PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("FetchSize", PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = Property("IsBookmarkable",
        PROPERTY_ID_ISBOOKMARKABLE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY);
    pProperties[nPos++] = Property("ResultSetConcurrency",
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);
    pProperties[nPos++] = Property("ResultSetType",
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    OSL_TRACE("OResultSet::getInfoHelper");
    return *getArrayHelper();
}

sal_Bool OResultSet::convertFastPropertyValue(Any & /* rConvertedValue */,
                                            Any & /* rOldValue */,
                                            sal_Int32 nHandle,
                                            const Any& /* rValue */)
    throw (css::lang::IllegalArgumentException)
{
    OSL_TRACE("OResultSet::convertFastPropertyValue");
    switch (nHandle) {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}

void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& /* rValue */)
    throw (Exception, std::exception)
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

void SAL_CALL OResultSet::acquire()
    throw()
{
    OSL_TRACE("OResultSet::acquire");
    OResultSet_BASE::acquire();
}

void SAL_CALL OResultSet::release()
    throw()
{
    OSL_TRACE("OResultSet::release");
    OResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo() throw(css::uno::RuntimeException, std::exception)
{
    OSL_TRACE("OResultSet::getPropertySetInfo");
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void OResultSet::checkColumnIndex(sal_Int32 index)
    throw (SQLException, RuntimeException)
{
    OSL_TRACE("OResultSet::checkColumnIndex");
    if ((index < 1 || index > (int) fieldCount)) {
        /* static object for efficiency or thread safety is a problem ? */
        rtl::OUString buf( "index out of range" );
        throw SQLException(buf, *this, rtl::OUString(), 1, Any());
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
