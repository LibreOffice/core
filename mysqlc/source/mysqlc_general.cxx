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

using com::sun::star::sdbc::SQLException;

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;

namespace mysqlc_sdbc_driver
{

template<typename T>
void resetSqlVar(void* target, T* pValue, enum_field_types type, sal_Int32 nSize)
{
    if(target)
    {
        free(target);
        target = nullptr;
    }
    constexpr nUnitSize = sizeof(T);
    switch(type)
    {
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
            target = malloc(nUnitSize);
            memcpy(target, pValue, nUnitSize);
            break;
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_BLOB:
            target = malloc(nUnitSize*nSize);
            memcpy(target, pValue, nUnitSize*nSize);
            break;
        case MYSQL_TYPE_NULL:
            // nothing I guess
            break;
        default:
            SAL_WARN("connectivity","unknown enum_field_type");
    }

}

void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext )
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

void throwInvalidArgumentException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext )
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
        switch (field->type) {
                case MYSQL_TYPE_BIT:
                        return "BIT";
                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_NEWDECIMAL:
                        return isUnsigned ? (isZerofill? "DECIMAL UNSIGNED ZEROFILL" : "DECIMAL UNSIGNED"): "DECIMAL";
                case MYSQL_TYPE_TINY:
                        return isUnsigned ? (isZerofill? "TINYINT UNSIGNED ZEROFILL" : "TINYINT UNSIGNED"): "TINYINT";
                case MYSQL_TYPE_SHORT:
                        return isUnsigned ? (isZerofill? "SMALLINT UNSIGNED ZEROFILL" : "SMALLINT UNSIGNED"): "SMALLINT";
                case MYSQL_TYPE_LONG:
                        return isUnsigned ? (isZerofill? "INT UNSIGNED ZEROFILL" : "INT UNSIGNED"): "INT";
                case MYSQL_TYPE_FLOAT:
                        return isUnsigned ? (isZerofill? "FLOAT UNSIGNED ZEROFILL" : "FLOAT UNSIGNED"): "FLOAT";
                case MYSQL_TYPE_DOUBLE:
                        return isUnsigned ? (isZerofill? "DOUBLE UNSIGNED ZEROFILL" : "DOUBLE UNSIGNED"): "DOUBLE";
                case MYSQL_TYPE_NULL:
                        return "NULL";
                case MYSQL_TYPE_TIMESTAMP:
                        return "TIMESTAMP";
                case MYSQL_TYPE_LONGLONG:
                        return isUnsigned ? (isZerofill? "BIGINT UNSIGNED ZEROFILL" : "BIGINT UNSIGNED") : "BIGINT";
                case MYSQL_TYPE_INT24:
                        return isUnsigned ? (isZerofill? "MEDIUMINT UNSIGNED ZEROFILL" : "MEDIUMINT UNSIGNED") : "MEDIUMINT";
                case MYSQL_TYPE_DATE:
                        return "DATE";
                case MYSQL_TYPE_TIME:
                        return "TIME";
                case MYSQL_TYPE_DATETIME:
                        return "DATETIME";
                case MYSQL_TYPE_TINY_BLOB:
                {
                        bool isBinary = field->charsetnr == MAGIC_BINARY_CHARSET_NR;
                        return isBinary? "TINYBLOB":"TINYTEXT";
                }
                case MYSQL_TYPE_MEDIUM_BLOB:// should no appear over the wire
                {
                        bool isBinary = field->charsetnr == MAGIC_BINARY_CHARSET_NR;
                        return isBinary? "MEDIUMBLOB":"MEDIUMTEXT";
                }
                case MYSQL_TYPE_LONG_BLOB:// should no appear over the wire
                {
                        bool isBinary = field->charsetnr == MAGIC_BINARY_CHARSET_NR;
                        return isBinary? "LONGBLOB":"LONGTEXT";
                }
                case MYSQL_TYPE_BLOB:
                {
                        bool isBinary= field->charsetnr == MAGIC_BINARY_CHARSET_NR;
                        return isBinary? "BLOB":"TEXT";
                }
                case MYSQL_TYPE_VARCHAR:
                case MYSQL_TYPE_VAR_STRING:
                        if (field->flags & ENUM_FLAG) {
                                return "ENUM";
                        }
                        if (field->flags & SET_FLAG) {
                                return "SET";
                        }
                        if (field->charsetnr == MAGIC_BINARY_CHARSET_NR) {
                                return "VARBINARY";
                        }
                        return "VARCHAR";
                case MYSQL_TYPE_STRING:
                        if (field->flags & ENUM_FLAG) {
                                return "ENUM";
                        }
                        if (field->flags & SET_FLAG) {
                                return "SET";
                        }
                        if ((field->flags & BINARY_FLAG) && field->charsetnr == MAGIC_BINARY_CHARSET_NR) {
                                return "BINARY";
                        }
                        return "CHAR";
                case MYSQL_TYPE_YEAR:
                        return "YEAR";
                case MYSQL_TYPE_GEOMETRY:
                        return "GEOMETRY";
                default:
                        return "UNKNOWN";

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
