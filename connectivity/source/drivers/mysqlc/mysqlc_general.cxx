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

#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/string_view.hxx>

#include <com/sun/star/sdbc/DataType.hpp>

using com::sun::star::sdbc::SQLException;

using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;

using namespace rtl;

namespace mysqlc_sdbc_driver
{
void allocateSqlVar(void** mem, enum_field_types eType, unsigned nSize)
{
    assert(mem);
    switch (eType)
    {
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_INT24:
            *mem = malloc(sizeof(sal_Int32));
            break;
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_YEAR:
            *mem = malloc(sizeof(sal_Int16));
            break;
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_TINY:
            *mem = malloc(sizeof(sal_Int8));
            break;
        case MYSQL_TYPE_LONGLONG:
            *mem = malloc(sizeof(sal_Int64));
            break;
        case MYSQL_TYPE_FLOAT:
            *mem = malloc(sizeof(float));
            break;
        case MYSQL_TYPE_DOUBLE:
            *mem = malloc(sizeof(double));
            break;
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_GEOMETRY:
            *mem = malloc(sizeof(MYSQL_TIME));
            break;
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            *mem = malloc(sizeof(char) * nSize);
            break;
        case MYSQL_TYPE_NULL:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
            *mem = nullptr;
            break;
        default:
            SAL_WARN("connectivity.mysqlc", "unknown enum_field_type");
    }
}

void throwFeatureNotImplementedException(const char* _pAsciiFeatureName,
                                         const css::uno::Reference<XInterface>& _rxContext)
{
    const OUString sMessage
        = OUString::createFromAscii(_pAsciiFeatureName) + ": feature not implemented.";
    throw SQLException(sMessage, _rxContext, u"HYC00"_ustr, 0, Any());
}

void throwInvalidArgumentException(const char* _pAsciiFeatureName,
                                   const css::uno::Reference<XInterface>& _rxContext)
{
    const OUString sMessage
        = OUString::createFromAscii(_pAsciiFeatureName) + ": invalid arguments.";
    throw SQLException(sMessage, _rxContext, u"HYC00"_ustr, 0, Any());
}

void throwSQLExceptionWithMsg(const char* msg, const char* SQLSTATE, unsigned int errorNum,
                              const css::uno::Reference<css::uno::XInterface>& _context,
                              const rtl_TextEncoding encoding)
{
    OString errorMsg{ msg };
    throwSQLExceptionWithMsg(OStringToOUString(errorMsg, encoding), SQLSTATE, errorNum, _context);
}

void throwSQLExceptionWithMsg(const OUString& msg, const char* SQLSTATE, unsigned int errorNum,
                              const css::uno::Reference<css::uno::XInterface>& _context)
{
    throw SQLException(msg, _context, OStringToOUString(SQLSTATE, RTL_TEXTENCODING_ASCII_US),
                       errorNum, Any());
}

sal_Int32 mysqlToOOOType(int eType, int charsetnr) noexcept
{
    // charset number 63 indicates binary
    switch (eType)
    {
        case MYSQL_TYPE_BIT:
            return css::sdbc::DataType::BIT;

        case MYSQL_TYPE_TINY:
            return css::sdbc::DataType::TINYINT;

        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_YEAR:
            return css::sdbc::DataType::SMALLINT;

        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
            return css::sdbc::DataType::INTEGER;

        case MYSQL_TYPE_LONGLONG:
            return css::sdbc::DataType::BIGINT;

        case MYSQL_TYPE_FLOAT:
            return css::sdbc::DataType::REAL;

        case MYSQL_TYPE_DOUBLE:
            return css::sdbc::DataType::DOUBLE;

        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return css::sdbc::DataType::DECIMAL;

        case MYSQL_TYPE_STRING:
            if (charsetnr == 63)
                return css::sdbc::DataType::BINARY;
            return css::sdbc::DataType::CHAR;

        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_VAR_STRING:
            if (charsetnr == 63)
                return css::sdbc::DataType::VARBINARY;
            return css::sdbc::DataType::VARCHAR;

        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
            if (charsetnr == 63)
                return css::sdbc::DataType::LONGVARBINARY;
            return css::sdbc::DataType::LONGVARCHAR;

        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATETIME:
            return css::sdbc::DataType::TIMESTAMP;

        case MYSQL_TYPE_DATE:
            return css::sdbc::DataType::DATE;

        case MYSQL_TYPE_TIME:
            return css::sdbc::DataType::TIME;

        case MYSQL_TYPE_GEOMETRY:
            return css::sdbc::DataType::VARCHAR;

        case MYSQL_TYPE_NULL:
            return css::sdbc::DataType::SQLNULL;
    }

    OSL_FAIL("mysqlToOOOType: unhandled case, falling back to VARCHAR");
    return css::sdbc::DataType::VARCHAR;
}

sal_Int32 mysqlStrToOOOType(std::u16string_view sType)
{
    // TODO other types.
    if (o3tl::equalsIgnoreAsciiCase(sType, u"tiny")
        || o3tl::equalsIgnoreAsciiCase(sType, u"tinyint"))
        return css::sdbc::DataType::TINYINT;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"smallint")
        || o3tl::equalsIgnoreAsciiCase(sType, u"year"))
        return css::sdbc::DataType::SMALLINT;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"int")
        || o3tl::equalsIgnoreAsciiCase(sType, u"mediumint"))
        return css::sdbc::DataType::INTEGER;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"varchar") || o3tl::equalsIgnoreAsciiCase(sType, u"set")
        || o3tl::equalsIgnoreAsciiCase(sType, u"enum"))
        return css::sdbc::DataType::VARCHAR;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"bigint"))
        return css::sdbc::DataType::BIGINT;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"blob")
        || o3tl::equalsIgnoreAsciiCase(sType, u"longblob")
        || o3tl::equalsIgnoreAsciiCase(sType, u"tinyblob")
        || o3tl::equalsIgnoreAsciiCase(sType, u"mediumblob"))
        return css::sdbc::DataType::BLOB;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"varbinary"))
        return css::sdbc::DataType::VARBINARY;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"char"))
        return css::sdbc::DataType::CHAR;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"tinytext"))
        return css::sdbc::DataType::VARCHAR;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"text"))
        return css::sdbc::DataType::LONGVARCHAR;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"mediumtext")
        || o3tl::equalsIgnoreAsciiCase(sType, u"longtext"))
        return css::sdbc::DataType::CLOB;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"binary"))
        return css::sdbc::DataType::BINARY;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"time"))
        return css::sdbc::DataType::TIME;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"date"))
        return css::sdbc::DataType::DATE;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"datetime")
        || o3tl::equalsIgnoreAsciiCase(sType, u"timestamp"))
        return css::sdbc::DataType::TIMESTAMP;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"decimal"))
        return css::sdbc::DataType::DECIMAL;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"real") || o3tl::equalsIgnoreAsciiCase(sType, u"float"))
        return css::sdbc::DataType::REAL;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"double"))
        return css::sdbc::DataType::DOUBLE;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"bit"))
        return css::sdbc::DataType::BIT;
    if (o3tl::equalsIgnoreAsciiCase(sType, u"bool")
        || o3tl::equalsIgnoreAsciiCase(sType, u"boolean"))
        return css::sdbc::DataType::BOOLEAN;
    OSL_FAIL("Unknown type name from string, failing back to varchar.");
    return css::sdbc::DataType::VARCHAR;
}

OUString mysqlTypeToStr(unsigned type, unsigned flags)
{
    bool isUnsigned = (flags & UNSIGNED_FLAG) != 0;
    bool isZerofill = (flags & ZEROFILL_FLAG) != 0;
    switch (type)
    {
        case MYSQL_TYPE_BIT:
            return u"BIT"_ustr;
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return isUnsigned
                       ? (isZerofill ? u"DECIMAL UNSIGNED ZEROFILL"_ustr : u"DECIMAL UNSIGNED"_ustr)
                       : u"DECIMAL"_ustr;
        case MYSQL_TYPE_TINY:
            return isUnsigned
                       ? (isZerofill ? u"TINYINT UNSIGNED ZEROFILL"_ustr : u"TINYINT UNSIGNED"_ustr)
                       : u"TINYINT"_ustr;
        case MYSQL_TYPE_SHORT:
            return isUnsigned ? (isZerofill ? u"SMALLINT UNSIGNED ZEROFILL"_ustr
                                            : u"SMALLINT UNSIGNED"_ustr)
                              : u"SMALLINT"_ustr;
        case MYSQL_TYPE_LONG:
            return isUnsigned ? (isZerofill ? u"INT UNSIGNED ZEROFILL"_ustr : u"INT UNSIGNED"_ustr)
                              : u"INT"_ustr;
        case MYSQL_TYPE_FLOAT:
            return isUnsigned
                       ? (isZerofill ? u"FLOAT UNSIGNED ZEROFILL"_ustr : u"FLOAT UNSIGNED"_ustr)
                       : u"FLOAT"_ustr;
        case MYSQL_TYPE_DOUBLE:
            return isUnsigned
                       ? (isZerofill ? u"DOUBLE UNSIGNED ZEROFILL"_ustr : u"DOUBLE UNSIGNED"_ustr)
                       : u"DOUBLE"_ustr;
        case MYSQL_TYPE_NULL:
            return u"NULL"_ustr;
        case MYSQL_TYPE_TIMESTAMP:
            return u"TIMESTAMP"_ustr;
        case MYSQL_TYPE_LONGLONG:
            return isUnsigned
                       ? (isZerofill ? u"BIGINT UNSIGNED ZEROFILL"_ustr : u"BIGINT UNSIGNED"_ustr)
                       : u"BIGINT"_ustr;
        case MYSQL_TYPE_INT24:
            return isUnsigned ? (isZerofill ? u"MEDIUMINT UNSIGNED ZEROFILL"_ustr
                                            : u"MEDIUMINT UNSIGNED"_ustr)
                              : u"MEDIUMINT"_ustr;
        case MYSQL_TYPE_DATE:
            return u"DATE"_ustr;
        case MYSQL_TYPE_TIME:
            return u"TIME"_ustr;
        case MYSQL_TYPE_DATETIME:
            return u"DATETIME"_ustr;
        case MYSQL_TYPE_TINY_BLOB:
        {
            return u"TINYBLOB"_ustr;
        }
        case MYSQL_TYPE_MEDIUM_BLOB:
        {
            return u"MEDIUMBLOB"_ustr;
        }
        case MYSQL_TYPE_LONG_BLOB:
        {
            return u"LONGBLOB"_ustr;
        }
        case MYSQL_TYPE_BLOB:
        {
            return u"BLOB"_ustr;
        }
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
            if (flags & ENUM_FLAG)
            {
                return u"ENUM"_ustr;
            }
            if (flags & SET_FLAG)
            {
                return u"SET"_ustr;
            }
            return u"VARCHAR"_ustr;
        case MYSQL_TYPE_STRING:
            if (flags & ENUM_FLAG)
            {
                return u"ENUM"_ustr;
            }
            if (flags & SET_FLAG)
            {
                return u"SET"_ustr;
            }
            return u"CHAR"_ustr;
        case MYSQL_TYPE_YEAR:
            return u"YEAR"_ustr;
        case MYSQL_TYPE_GEOMETRY:
            return u"GEOMETRY"_ustr;
        default:
            return u"UNKNOWN"_ustr;
    }
}

OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding)
{
    return OUString(_string.c_str(), _string.size(), encoding);
}

} /* namespace */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
