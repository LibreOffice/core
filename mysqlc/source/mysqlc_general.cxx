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

using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;

namespace mysqlc_sdbc_driver
{

void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    const rtl::OUString sMessage = rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ": feature not implemented.";
    throw SQLException(
        sMessage,
        _rxContext,
        rtl::OUString("HYC00"),
        0,
        _pNextException ? *_pNextException : Any()
    );
}

void throwInvalidArgumentException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    const rtl::OUString sMessage = rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ": invalid arguments.";
    throw SQLException(
        sMessage,
        _rxContext,
        rtl::OUString("HYC00"),
        0,
        _pNextException ? *_pNextException : Any()
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
