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

#include <cppconn/exception.h>
#include <cppconn/datatype.h>
#include <rtl/ustring.hxx>

using com::sun::star::sdbc::SQLException;

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;

using namespace rtl;

namespace mysqlc_sdbc_driver
{

void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const css::uno::Reference< XInterface >& _rxContext )
{
    const rtl::OUString sMessage = rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ": feature not implemented.";
    throw SQLException(
        sMessage,
        _rxContext,
        rtl::OUString("HYC00"),
        0,
        Any()
    );
}

void throwInvalidArgumentException( const sal_Char* _pAsciiFeatureName, const css::uno::Reference< XInterface >& _rxContext )
{
    const rtl::OUString sMessage = rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ": invalid arguments.";
    throw SQLException(
        sMessage,
        _rxContext,
        rtl::OUString("HYC00"),
        0,
        Any()
    );
}

void translateAndThrow(const ::sql::SQLException& _error, const css::uno::Reference< css::uno::XInterface >& _context, const rtl_TextEncoding encoding)
{
    throw SQLException(
            convert(_error.what(), encoding),
            _context,
            convert(_error.getSQLState(), encoding),
            _error.getErrorCode(),
            Any()
        );
}

void throwSQLExceptionWithMsg(const char* msg, unsigned int errorNum, const css::uno::Reference< css::uno::XInterface >& _context, const rtl_TextEncoding encoding)
{
    rtl::OString errorMsg{msg};
    // TODO error code?
    throw SQLException( rtl::OStringToOUString(errorMsg, encoding),
            _context, rtl::OUString(), errorNum, Any());
}

rtl::OUString getStringFromAny(const Any& _rAny)
{
    rtl::OUString nReturn;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}

int mysqlToOOOType(int cppConnType)
    throw ()
{
    switch (cppConnType) {
        case sql::DataType::BIT:
            return css::sdbc::DataType::VARCHAR;

        case sql::DataType::TINYINT:
            return css::sdbc::DataType::TINYINT;

        case sql::DataType::SMALLINT:
            return css::sdbc::DataType::SMALLINT;

        case sql::DataType::INTEGER:
            return css::sdbc::DataType::INTEGER;

        case sql::DataType::BIGINT:
            return css::sdbc::DataType::BIGINT;

        case sql::DataType::REAL:
            return css::sdbc::DataType::REAL;

        case sql::DataType::DOUBLE:
            return css::sdbc::DataType::DOUBLE;

        case sql::DataType::DECIMAL:
            return css::sdbc::DataType::DECIMAL;

        case sql::DataType::CHAR:
            return css::sdbc::DataType::CHAR;

        case sql::DataType::BINARY:
            return css::sdbc::DataType::BINARY;

        case sql::DataType::ENUM:
        case sql::DataType::SET:
        case sql::DataType::VARCHAR:
            return css::sdbc::DataType::VARCHAR;

        case sql::DataType::VARBINARY:
            return css::sdbc::DataType::VARBINARY;

        case sql::DataType::LONGVARCHAR:
            return css::sdbc::DataType::LONGVARCHAR;

        case sql::DataType::LONGVARBINARY:
            return css::sdbc::DataType::LONGVARBINARY;

        case sql::DataType::TIMESTAMP:
            return css::sdbc::DataType::TIMESTAMP;

        case sql::DataType::DATE:
            return css::sdbc::DataType::DATE;

        case sql::DataType::TIME:
            return css::sdbc::DataType::TIME;

        case sql::DataType::GEOMETRY:
            return css::sdbc::DataType::VARCHAR;

        case sql::DataType::SQLNULL:
            return css::sdbc::DataType::SQLNULL;

        case sql::DataType::UNKNOWN:
            return css::sdbc::DataType::VARCHAR;
    }

    OSL_FAIL( "mysqlToOOOType: unhandled case, falling back to VARCHAR" );
    return css::sdbc::DataType::VARCHAR;
}

rtl::OUString mysqlTypeToStr(MYSQL_FIELD* field)
{
    bool isUnsigned = (field->flags & UNSIGNED_FLAG) != 0;
    bool isZerofill = (field->flags & ZEROFILL_FLAG) != 0;
    switch (field->type)
    {
            case MYSQL_TYPE_BIT:
                    return OUString{"BIT"};
            case MYSQL_TYPE_DECIMAL:
            case MYSQL_TYPE_NEWDECIMAL:
                    return isUnsigned ? (isZerofill? OUString{"DECIMAL UNSIGNED ZEROFILL"} : OUString{"DECIMAL UNSIGNED"}): OUString{"DECIMAL"};
            case MYSQL_TYPE_TINY:
                    return isUnsigned ? (isZerofill? OUString{"TINYINT UNSIGNED ZEROFILL"} : OUString{"TINYINT UNSIGNED"}): OUString{"TINYINT"};
            case MYSQL_TYPE_SHORT:
                    return isUnsigned ? (isZerofill? OUString{"SMALLINT UNSIGNED ZEROFILL"} : OUString{"SMALLINT UNSIGNED"}): OUString{"SMALLINT"};
            case MYSQL_TYPE_LONG:
                    return isUnsigned ? (isZerofill? OUString{"INT UNSIGNED ZEROFILL"} : OUString{"INT UNSIGNED"}): OUString{"INT"};
            case MYSQL_TYPE_FLOAT:
                    return isUnsigned ? (isZerofill? OUString{"FLOAT UNSIGNED ZEROFILL"} : OUString{"FLOAT UNSIGNED"}): OUString{"FLOAT"};
            case MYSQL_TYPE_DOUBLE:
                    return isUnsigned ? (isZerofill? OUString{"DOUBLE UNSIGNED ZEROFILL"} : OUString{"DOUBLE UNSIGNED"}): OUString{"DOUBLE"};
            case MYSQL_TYPE_NULL:
                    return OUString{"NULL"};
            case MYSQL_TYPE_TIMESTAMP:
                    return OUString{"TIMESTAMP"};
            case MYSQL_TYPE_LONGLONG:
                    return isUnsigned ? (isZerofill? OUString{"BIGINT UNSIGNED ZEROFILL"} : OUString{"BIGINT UNSIGNED"}) : OUString{"BIGINT"};
            case MYSQL_TYPE_INT24:
                    return isUnsigned ? (isZerofill? OUString{"MEDIUMINT UNSIGNED ZEROFILL"} : OUString{"MEDIUMINT UNSIGNED"}) : OUString{"MEDIUMINT"};
            case MYSQL_TYPE_DATE:
                    return OUString{"DATE"};
            case MYSQL_TYPE_TIME:
                    return OUString{"TIME"};
            case MYSQL_TYPE_DATETIME:
                    return OUString{"DATETIME"};
            case MYSQL_TYPE_TINY_BLOB:
            {
                    return OUString{"TINYBLOB"};
            }
            case MYSQL_TYPE_MEDIUM_BLOB:
            {
                    return OUString{"MEDIUMBLOB"};
            }
            case MYSQL_TYPE_LONG_BLOB:
            {
                    return OUString{"LONGBLOB"};
            }
            case MYSQL_TYPE_BLOB:
            {
                    return OUString{"BLOB"};
            }
            case MYSQL_TYPE_VARCHAR:
            case MYSQL_TYPE_VAR_STRING:
                    if (field->flags & ENUM_FLAG) {
                            return OUString{"ENUM"};
                    }
                    if (field->flags & SET_FLAG) {
                            return OUString{"SET"};
                    }
                    return OUString{"VARCHAR"};
            case MYSQL_TYPE_STRING:
                    if (field->flags & ENUM_FLAG) {
                            return OUString{"ENUM"};
                    }
                    if (field->flags & SET_FLAG) {
                            return OUString{"SET"};
                    }
                    return OUString{"CHAR"};
            case MYSQL_TYPE_YEAR:
                    return OUString{"YEAR"};
            case MYSQL_TYPE_GEOMETRY:
                    return OUString{"GEOMETRY"};
            default:
                    return OUString{"UNKNOWN"};
    }

}

rtl::OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding)
{
    return rtl::OUString( _string.c_str(), _string.size(), encoding );
}

::std::string convert(const rtl::OUString& _string, const rtl_TextEncoding encoding)
{
    return ::std::string( rtl::OUStringToOString( _string, encoding ).getStr() );
}


} /* namespace */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
