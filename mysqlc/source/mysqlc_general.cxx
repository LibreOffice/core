/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/
#include "mysqlc_general.hxx"
#include "mysqlc_resultsetmetadata.hxx"

#include <cppconn/exception.h>
#include <cppconn/datatype.h>

using com::sun::star::sdbc::SQLException;

using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using ::rtl::OUString;

namespace mysqlc_sdbc_driver
{
// -----------------------------------------------------------------------------
void throwFeatureNotImplementedException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    const ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": feature not implemented." ) );
    throw SQLException(
        sMessage,
        _rxContext,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HYC00")),
        0,
        _pNextException ? *_pNextException : Any()
    );
}


void throwInvalidArgumentException( const sal_Char* _pAsciiFeatureName, const Reference< XInterface >& _rxContext, const Any* _pNextException )
    throw (SQLException)
{
    const ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii( _pAsciiFeatureName ) + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": invalid arguments." ) );
    throw SQLException(
        sMessage,
        _rxContext,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HYC00")),
        0,
        _pNextException ? *_pNextException : Any()
    );
}

void translateAndThrow(const ::sql::SQLException& _error, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _context, const rtl_TextEncoding encoding)
{
    throw SQLException(
            convert(_error.what(), encoding),
            _context,
            convert(_error.getSQLState(), encoding),
            _error.getErrorCode(),
            Any()
        );
}


OUString getStringFromAny(const Any& _rAny)
{
    OUString nReturn;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


int mysqlToOOOType(int cppConnType)
    throw ()
{
    switch (cppConnType) {
        case sql::DataType::BIT:
            return com::sun::star::sdbc::DataType::VARCHAR;

        case sql::DataType::TINYINT:
            return com::sun::star::sdbc::DataType::TINYINT;

        case sql::DataType::SMALLINT:
            return com::sun::star::sdbc::DataType::SMALLINT;

        case sql::DataType::INTEGER:
            return com::sun::star::sdbc::DataType::INTEGER;

        case sql::DataType::BIGINT:
            return com::sun::star::sdbc::DataType::BIGINT;

        case sql::DataType::REAL:
            return com::sun::star::sdbc::DataType::REAL;

        case sql::DataType::DOUBLE:
            return com::sun::star::sdbc::DataType::DOUBLE;

        case sql::DataType::DECIMAL:
            return com::sun::star::sdbc::DataType::DECIMAL;

        case sql::DataType::CHAR:
            return com::sun::star::sdbc::DataType::CHAR;

        case sql::DataType::BINARY:
            return com::sun::star::sdbc::DataType::BINARY;

        case sql::DataType::ENUM:
        case sql::DataType::SET:
        case sql::DataType::VARCHAR:
            return com::sun::star::sdbc::DataType::VARCHAR;

        case sql::DataType::VARBINARY:
            return com::sun::star::sdbc::DataType::VARBINARY;

        case sql::DataType::LONGVARCHAR:
            return com::sun::star::sdbc::DataType::LONGVARCHAR;

        case sql::DataType::LONGVARBINARY:
            return com::sun::star::sdbc::DataType::LONGVARBINARY;

        case sql::DataType::TIMESTAMP:
            return com::sun::star::sdbc::DataType::TIMESTAMP;

        case sql::DataType::DATE:
            return com::sun::star::sdbc::DataType::DATE;

        case sql::DataType::TIME:
            return com::sun::star::sdbc::DataType::TIME;

        case sql::DataType::GEOMETRY:
            return com::sun::star::sdbc::DataType::VARCHAR;

        case sql::DataType::SQLNULL:
            return com::sun::star::sdbc::DataType::SQLNULL;

        case sql::DataType::UNKNOWN:
            return com::sun::star::sdbc::DataType::VARCHAR;
    }

    OSL_ENSURE( false, "mysqlToOOOType: unhandled case, falling back to VARCHAR" );
    return com::sun::star::sdbc::DataType::VARCHAR;
}


::rtl::OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding)
{
    return ::rtl::OUString( _string.c_str(), _string.size(), encoding );
}

::std::string convert(const ::rtl::OUString& _string, const rtl_TextEncoding encoding)
{
    return ::std::string( ::rtl::OUStringToOString( _string, encoding ).getStr() );
}


} /* namespace */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
