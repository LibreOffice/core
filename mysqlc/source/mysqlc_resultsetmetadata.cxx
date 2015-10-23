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

#include "mysqlc_resultsetmetadata.hxx"
#include "mysqlc_general.hxx"
#include "cppconn/exception.h"

#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;


OResultSetMetaData::~OResultSetMetaData()
{
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnDisplaySize");

    try {
        meta->getColumnDisplaySize(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getColumnDisplaySize", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnType");
    checkColumnIndex(column);

    try {
        return mysqlc_sdbc_driver::mysqlToOOOType(meta->getColumnType(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}

/*
  XXX: This method doesn't throw exceptions at all.
  Should it declare that it throws ?? What if throw() is removed?
  Does it change the API, the open-close principle?
*/
sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount()
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnCount");
    try {
        return meta->getColumnCount();
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isCaseSensitive");
    checkColumnIndex(column);

    try {
        return meta->isCaseSensitive(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getSchemaName");
    checkColumnIndex(column);

    try {
        return convert(meta->getSchemaName(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnName");
    checkColumnIndex(column);

    try {
        return convert( meta->getColumnName( column ) );
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getTableName");
    checkColumnIndex(column);

    try {
        return convert(meta->getTableName(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getCatalogName");
    checkColumnIndex(column);

    try {
        return convert(meta->getCatalogName(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnTypeName");
    checkColumnIndex(column);

    try {
        return convert(meta->getColumnTypeName(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnLabel");
    checkColumnIndex(column);

    try {
        return convert(meta->getColumnLabel(column));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return rtl::OUString(); // fool compiler
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getColumnServiceName");
    checkColumnIndex(column);

    rtl::OUString aRet = rtl::OUString();
    return aRet;
}

sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isCurrency");
    checkColumnIndex(column);

    try {
        return meta->isCurrency(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isAutoIncrement");
    checkColumnIndex(column);

    try {
        return meta->isAutoIncrement(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isSigned");
    checkColumnIndex(column);

    try {
        return meta->isSigned(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getPrecision");
    checkColumnIndex(column);

    try {
        return meta->getPrecision(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getPrecision", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
    throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::getScale");
    checkColumnIndex(column);
    try {
        return meta->getScale(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getScale", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return 0; // fool compiler
}

sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isNullable");
    checkColumnIndex(column);

    try {
        return sal_Int32(bool(meta->isNullable(column)));
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_Int32(false); // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isSearchable");
    checkColumnIndex(column);

    try {
        return meta->isSearchable(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isReadOnly");
    checkColumnIndex(column);

    try {
        return meta->isReadOnly(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isDefinitelyWritable");
    checkColumnIndex(column);

    try {
        return meta->isDefinitelyWritable(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

sal_Bool SAL_CALL OResultSetMetaData::isWritable(sal_Int32 column)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("OResultSetMetaData::isWritable");
    checkColumnIndex(column);

    try {
        return meta->isWritable(column);
    } catch (const sql::MethodNotImplementedException &) {
        mysqlc_sdbc_driver::throwFeatureNotImplementedException("OResultSetMetaData::getMetaData", *this);
    } catch (const sql::SQLException &e) {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, m_encoding);
    }
    return sal_False; // fool compiler
}

void OResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
    throw (SQLException, RuntimeException)
{
    OSL_TRACE("OResultSetMetaData::checkColumnIndex");
    if (columnIndex < 1 || columnIndex > (sal_Int32) meta->getColumnCount()) {

        rtl::OUStringBuffer buf;
        buf.appendAscii( "Column index out of range (expected 1 to " );
        buf.append( sal_Int32( meta->getColumnCount() ) );
        buf.appendAscii( ", got " );
        buf.append( sal_Int32( columnIndex ) );
        buf.append( '.' );
        throw SQLException( buf.makeStringAndClear(), *this, rtl::OUString(), 1, Any() );
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
