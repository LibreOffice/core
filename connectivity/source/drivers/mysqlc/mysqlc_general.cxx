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
#include "mysqlc_resultsetmetadata.hxx"

#include <sal/log.hxx>
#include <rtl/ustring.hxx>

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
        case MYSQL_TYPE_YEAR: // FIXME below
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

void throwFeatureNotImplementedException(const sal_Char* _pAsciiFeatureName,
                                         const css::uno::Reference<XInterface>& _rxContext)
{
    const OUString sMessage
        = OUString::createFromAscii(_pAsciiFeatureName) + ": feature not implemented.";
    throw SQLException(sMessage, _rxContext, "HYC00", 0, Any());
}

void throwInvalidArgumentException(const sal_Char* _pAsciiFeatureName,
                                   const css::uno::Reference<XInterface>& _rxContext)
{
    const OUString sMessage
        = OUString::createFromAscii(_pAsciiFeatureName) + ": invalid arguments.";
    throw SQLException(sMessage, _rxContext, "HYC00", 0, Any());
}

void throwSQLExceptionWithMsg(const char* msg, unsigned int errorNum,
                              const css::uno::Reference<css::uno::XInterface>& _context,
                              const rtl_TextEncoding encoding)
{
    OString errorMsg{ msg };
    // TODO error code?
    throw SQLException(OStringToOUString(errorMsg, encoding), _context, OUString(), errorNum,
                       Any());
}

sal_Int32 mysqlToOOOType(int eType, int charsetnr) noexcept
{
    // charset number 63 indicates binary
    switch (eType)
    {
        case MYSQL_TYPE_BIT:
            return css::sdbc::DataType::VARCHAR;

        case MYSQL_TYPE_TINY:
            return css::sdbc::DataType::TINYINT;

        case MYSQL_TYPE_SHORT:
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

sal_Int32 mysqlStrToOOOType(const OUString& sType)
{
    // TODO other types.
    if (sType.equalsIgnoreAsciiCase("tiny") || sType.equalsIgnoreAsciiCase("tinyint"))
        return css::sdbc::DataType::TINYINT;
    if (sType.equalsIgnoreAsciiCase("smallint") || sType.equalsIgnoreAsciiCase("mediumint"))
        return css::sdbc::DataType::SMALLINT;
    if (sType.equalsIgnoreAsciiCase("longtext"))
        return css::sdbc::DataType::LONGVARCHAR;
    if (sType.equalsIgnoreAsciiCase("int"))
        return css::sdbc::DataType::INTEGER;
    if (sType.equalsIgnoreAsciiCase("varchar") || sType.equalsIgnoreAsciiCase("set")
        || sType.equalsIgnoreAsciiCase("enum"))
        return css::sdbc::DataType::VARCHAR;
    if (sType.equalsIgnoreAsciiCase("bigint"))
        return css::sdbc::DataType::BIGINT;
    if (sType.equalsIgnoreAsciiCase("blob") || sType.equalsIgnoreAsciiCase("longblob"))
        return css::sdbc::DataType::BLOB;
    if (sType.equalsIgnoreAsciiCase("varbinary"))
        return css::sdbc::DataType::VARBINARY;
    if (sType.equalsIgnoreAsciiCase("text") || sType.equalsIgnoreAsciiCase("char"))
        return css::sdbc::DataType::CHAR;
    if (sType.equalsIgnoreAsciiCase("binary"))
        return css::sdbc::DataType::BINARY;
    if (sType.equalsIgnoreAsciiCase("time"))
        return css::sdbc::DataType::TIME;
    if (sType.equalsIgnoreAsciiCase("date"))
        return css::sdbc::DataType::DATE;
    if (sType.equalsIgnoreAsciiCase("datetime") || sType.equalsIgnoreAsciiCase("timestamp"))
        return css::sdbc::DataType::TIMESTAMP;
    if (sType.equalsIgnoreAsciiCase("decimal"))
        return css::sdbc::DataType::DECIMAL;
    if (sType.equalsIgnoreAsciiCase("real") || sType.equalsIgnoreAsciiCase("float"))
        return css::sdbc::DataType::REAL;
    if (sType.equalsIgnoreAsciiCase("double"))
        return css::sdbc::DataType::DOUBLE;
    if (sType.equalsIgnoreAsciiCase("bit") || sType.equalsIgnoreAsciiCase("bool")
        || sType.equalsIgnoreAsciiCase("boolean"))
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
            return OUString{ "BIT" };
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return isUnsigned ? (isZerofill ? OUString{ "DECIMAL UNSIGNED ZEROFILL" }
                                            : OUString{ "DECIMAL UNSIGNED" })
                              : OUString{ "DECIMAL" };
        case MYSQL_TYPE_TINY:
            return isUnsigned ? (isZerofill ? OUString{ "TINYINT UNSIGNED ZEROFILL" }
                                            : OUString{ "TINYINT UNSIGNED" })
                              : OUString{ "TINYINT" };
        case MYSQL_TYPE_SHORT:
            return isUnsigned ? (isZerofill ? OUString{ "SMALLINT UNSIGNED ZEROFILL" }
                                            : OUString{ "SMALLINT UNSIGNED" })
                              : OUString{ "SMALLINT" };
        case MYSQL_TYPE_LONG:
            return isUnsigned ? (isZerofill ? OUString{ "INT UNSIGNED ZEROFILL" }
                                            : OUString{ "INT UNSIGNED" })
                              : OUString{ "INT" };
        case MYSQL_TYPE_FLOAT:
            return isUnsigned ? (isZerofill ? OUString{ "FLOAT UNSIGNED ZEROFILL" }
                                            : OUString{ "FLOAT UNSIGNED" })
                              : OUString{ "FLOAT" };
        case MYSQL_TYPE_DOUBLE:
            return isUnsigned ? (isZerofill ? OUString{ "DOUBLE UNSIGNED ZEROFILL" }
                                            : OUString{ "DOUBLE UNSIGNED" })
                              : OUString{ "DOUBLE" };
        case MYSQL_TYPE_NULL:
            return OUString{ "NULL" };
        case MYSQL_TYPE_TIMESTAMP:
            return OUString{ "TIMESTAMP" };
        case MYSQL_TYPE_LONGLONG:
            return isUnsigned ? (isZerofill ? OUString{ "BIGINT UNSIGNED ZEROFILL" }
                                            : OUString{ "BIGINT UNSIGNED" })
                              : OUString{ "BIGINT" };
        case MYSQL_TYPE_INT24:
            return isUnsigned ? (isZerofill ? OUString{ "MEDIUMINT UNSIGNED ZEROFILL" }
                                            : OUString{ "MEDIUMINT UNSIGNED" })
                              : OUString{ "MEDIUMINT" };
        case MYSQL_TYPE_DATE:
            return OUString{ "DATE" };
        case MYSQL_TYPE_TIME:
            return OUString{ "TIME" };
        case MYSQL_TYPE_DATETIME:
            return OUString{ "DATETIME" };
        case MYSQL_TYPE_TINY_BLOB:
        {
            return OUString{ "TINYBLOB" };
        }
        case MYSQL_TYPE_MEDIUM_BLOB:
        {
            return OUString{ "MEDIUMBLOB" };
        }
        case MYSQL_TYPE_LONG_BLOB:
        {
            return OUString{ "LONGBLOB" };
        }
        case MYSQL_TYPE_BLOB:
        {
            return OUString{ "BLOB" };
        }
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
            if (flags & ENUM_FLAG)
            {
                return OUString{ "ENUM" };
            }
            if (flags & SET_FLAG)
            {
                return OUString{ "SET" };
            }
            return OUString{ "VARCHAR" };
        case MYSQL_TYPE_STRING:
            if (flags & ENUM_FLAG)
            {
                return OUString{ "ENUM" };
            }
            if (flags & SET_FLAG)
            {
                return OUString{ "SET" };
            }
            return OUString{ "CHAR" };
        case MYSQL_TYPE_YEAR:
            return OUString{ "YEAR" };
        case MYSQL_TYPE_GEOMETRY:
            return OUString{ "GEOMETRY" };
        default:
            return OUString{ "UNKNOWN" };
    }
}

OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding)
{
    return OUString(_string.c_str(), _string.size(), encoding);
}

} /* namespace */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
