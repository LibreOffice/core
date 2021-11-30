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

#include <odbc/OTools.hxx>
#include <odbc/OFunctions.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <osl/endian.h>
#include <odbc/OConnection.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <string.h>

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

namespace {
size_t sqlTypeLen ( SQLSMALLINT _nType )
{
    switch (_nType)
    {
    case SQL_C_SSHORT:
    case SQL_C_SHORT:
        return sizeof(SQLSMALLINT);
    case SQL_C_USHORT:
        return sizeof(SQLUSMALLINT);
    case SQL_C_SLONG:
    case SQL_C_LONG:
        return sizeof(SQLINTEGER);
    case SQL_C_ULONG:
        return sizeof(SQLUINTEGER);
    case SQL_C_FLOAT:
        return sizeof(SQLREAL);
    case SQL_C_DOUBLE:
        static_assert(sizeof(SQLDOUBLE) == sizeof(SQLFLOAT), "SQLDOUBLE/SQLFLOAT confusion");
        return sizeof(SQLDOUBLE);
    case SQL_C_BIT:
        return sizeof(SQLCHAR);
    case SQL_C_STINYINT:
    case SQL_C_TINYINT:
        return sizeof(SQLSCHAR);
    case SQL_C_UTINYINT:
        return sizeof(SQLCHAR);
    case SQL_C_SBIGINT:
        return sizeof(SQLBIGINT);
    case SQL_C_UBIGINT:
        return sizeof(SQLUBIGINT);
    /* iODBC gives this the same value as SQL_C_UBIGINT
    case SQL_C_BOOKMARK:
        return sizeof(BOOKMARK); */
    case SQL_C_TYPE_DATE:
    case SQL_C_DATE:
        return sizeof(SQL_DATE_STRUCT);
    case SQL_C_TYPE_TIME:
    case SQL_C_TIME:
        return sizeof(SQL_TIME_STRUCT);
    case SQL_C_TYPE_TIMESTAMP:
    case SQL_C_TIMESTAMP:
        return sizeof(SQL_TIMESTAMP_STRUCT);
    case SQL_C_NUMERIC:
        return sizeof(SQL_NUMERIC_STRUCT);
    case SQL_C_GUID:
        return sizeof(SQLGUID);
    case SQL_C_INTERVAL_YEAR:
    case SQL_C_INTERVAL_MONTH:
    case SQL_C_INTERVAL_DAY:
    case SQL_C_INTERVAL_HOUR:
    case SQL_C_INTERVAL_MINUTE:
    case SQL_C_INTERVAL_SECOND:
    case SQL_C_INTERVAL_YEAR_TO_MONTH:
    case SQL_C_INTERVAL_DAY_TO_HOUR:
    case SQL_C_INTERVAL_DAY_TO_MINUTE:
    case SQL_C_INTERVAL_DAY_TO_SECOND:
    case SQL_C_INTERVAL_HOUR_TO_MINUTE:
    case SQL_C_INTERVAL_HOUR_TO_SECOND:
    case SQL_C_INTERVAL_MINUTE_TO_SECOND:
        return sizeof(SQL_INTERVAL_STRUCT);
    // ** Variable-sized datatypes -> cannot predict length
    case SQL_C_CHAR:
    case SQL_C_WCHAR:
    case SQL_C_BINARY:
    // iODBC gives this the same value as SQL_C_BINARY
    //case SQL_C_VARBOOKMARK:
    // Unknown datatype -> cannot predict length
    default:
        return static_cast<size_t>(-1);
    }
}

void appendSQLWCHARs(OUStringBuffer & s, SQLWCHAR const * d, sal_Int32 n)
{
    static_assert(
        sizeof (SQLWCHAR) == sizeof (sal_Unicode) || sizeof (SQLWCHAR) == 4,
        "bad SQLWCHAR");
    if (sizeof (SQLWCHAR) == sizeof (sal_Unicode)) {
        s.append(reinterpret_cast<sal_Unicode const *>(d), n);
    } else {
        for (sal_Int32 i = 0; i < n; ++i) {
            s.appendUtf32(d[i]);
        }
    }
}
}


void OTools::getValue(  OConnection const * _pConnection,
                        SQLHANDLE _aStatementHandle,
                        sal_Int32 columnIndex,
                        SQLSMALLINT _nType,
                        bool &_bWasNull,
                        const css::uno::Reference< css::uno::XInterface >& _xInterface,
                        void* _pValue,
                        SQLLEN _nSize)
{
    const size_t properSize = sqlTypeLen(_nType);
    if ( properSize == static_cast<size_t>(-1) )
        SAL_WARN( "connectivity.drivers", "connectivity::odbc::OTools::getValue: unknown SQL type - cannot check buffer size");
    else
    {
        OSL_ENSURE(static_cast<size_t>(_nSize) == properSize, "connectivity::odbc::OTools::getValue got wrongly sized memory region to write result to");
        if ( o3tl::make_unsigned(_nSize) > properSize )
        {
            SAL_WARN( "connectivity.drivers", "memory region is too big - trying to fudge it");
            memset(_pValue, 0, _nSize);
#ifdef OSL_BIGENDIAN
            // This is skewed in favour of integer types
            _pValue = static_cast<char*>(_pValue) + _nSize - properSize;
#endif
        }
    }
    OSL_ENSURE(o3tl::make_unsigned(_nSize) >= properSize, "memory region is too small");
    SQLLEN pcbValue = SQL_NULL_DATA;
    OTools::ThrowException(_pConnection,
                            _pConnection->functions().GetData(_aStatementHandle,
                                        static_cast<SQLUSMALLINT>(columnIndex),
                                        _nType,
                                        _pValue,
                                        _nSize,
                                        &pcbValue),
                            _aStatementHandle,SQL_HANDLE_STMT,_xInterface,false);
    _bWasNull = pcbValue == SQL_NULL_DATA;
}

void OTools::ThrowException(const OConnection* _pConnection,
                            const SQLRETURN _rRetCode,
                            const SQLHANDLE _pContext,
                            const SQLSMALLINT _nHandleType,
                            const Reference< XInterface >& _xInterface,
                            const bool _bNoFound)
{
    switch(_rRetCode)
    {
        case SQL_NEED_DATA:
        case SQL_STILL_EXECUTING:
        case SQL_SUCCESS:

        case SQL_SUCCESS_WITH_INFO:
                        return;
        case SQL_NO_DATA_FOUND:
                                if(_bNoFound)
                                    return; // no need to throw an exception
                                break;
        case SQL_ERROR:             break;


        case SQL_INVALID_HANDLE:    SAL_WARN( "connectivity.drivers", "SdbODBC3_SetStatus: SQL_INVALID_HANDLE");
                                    throw SQLException();
    }

    // Additional Information on the latest ODBC-functioncall available
    // SQLError provides this Information.

    OUString errorMessage;
    OUString sqlState;
    SQLINTEGER pfNativeError;
    SQLRETURN n;

    // Information for latest operation:
    // when hstmt != SQL_NULL_HSTMT is (Used from SetStatus in SdbCursor, SdbTable, ...),
    // then the status of the latest statements will be fetched, without the Status of the last
    // statements of this connection [what in this case will probably be the same, but the Reference
    // Manual isn't totally clear in this...].
    // corresponding for hdbc.
    if (bUseWChar && _pConnection->functions().has(ODBC3SQLFunctionId::GetDiagRecW))
    {
        SQLWCHAR szSqlState[6];
        SQLWCHAR szErrorMessage[SQL_MAX_MESSAGE_LENGTH];
        szErrorMessage[0] = '\0';
        SQLSMALLINT cchErrorMsg = 0;

        n = _pConnection->functions().GetDiagRecW(_nHandleType,_pContext,1,
                         szSqlState,
                         &pfNativeError,
                         szErrorMessage, std::size(szErrorMessage) - 1, &cchErrorMsg);
        if (SQL_SUCCEEDED(n))
        {
            errorMessage = toUString(szErrorMessage, cchErrorMsg);
            sqlState = toUString(szSqlState, 5);
        }
    }
    else
    {
        SQLCHAR szSqlState[6];
        SQLCHAR szErrorMessage[SQL_MAX_MESSAGE_LENGTH];
        szErrorMessage[0] = '\0';
        SQLSMALLINT pcbErrorMsg = 0;

        n = _pConnection->functions().GetDiagRec(_nHandleType,_pContext,1,
                         szSqlState,
                         &pfNativeError,
                         szErrorMessage,sizeof szErrorMessage - 1,&pcbErrorMsg);
        if (SQL_SUCCEEDED(n))
        {
            rtl_TextEncoding _nTextEncoding = osl_getThreadTextEncoding();
            errorMessage = toUString(szErrorMessage, pcbErrorMsg, _nTextEncoding);
            sqlState = toUString(szSqlState, 5, _nTextEncoding);
        }
    }
    OSL_ENSURE(n != SQL_INVALID_HANDLE,"SdbODBC3_SetStatus: SQLError returned SQL_INVALID_HANDLE");
    OSL_ENSURE(n == SQL_SUCCESS || n == SQL_SUCCESS_WITH_INFO || n == SQL_NO_DATA_FOUND || n == SQL_ERROR,"SdbODBC3_SetStatus: SQLError failed");

    // For the Return Code of SQLError see ODBC 2.0 Programmer's Reference Page 287ff
    throw SQLException(errorMessage, _xInterface, sqlState, pfNativeError, Any());
}

Sequence<sal_Int8> OTools::getBytesValue(const OConnection* _pConnection,
                                         const SQLHANDLE _aStatementHandle,
                                         const sal_Int32 columnIndex,
                                         const SQLSMALLINT _fSqlType,
                                         bool &_bWasNull,
                                         const Reference< XInterface >& _xInterface)
{
    sal_Int8 aCharArray[2048];
    // First try to fetch the data with the little Buffer:
    const SQLLEN nMaxLen = sizeof aCharArray;
    SQLLEN pcbValue = SQL_NO_TOTAL;
    Sequence<sal_Int8> aData;

    OSL_ENSURE( _fSqlType != SQL_CHAR  && _fSqlType != SQL_VARCHAR  && _fSqlType != SQL_LONGVARCHAR &&
                _fSqlType != SQL_WCHAR && _fSqlType != SQL_WVARCHAR && _fSqlType != SQL_WLONGVARCHAR,
                "connectivity::odbc::OTools::getBytesValue called with character _fSqlType");

    while (pcbValue == SQL_NO_TOTAL || pcbValue > nMaxLen)
    {
        OTools::ThrowException(_pConnection,
                               _pConnection->functions().GetData(
                                   _aStatementHandle,
                                   static_cast<SQLUSMALLINT>(columnIndex),
                                   _fSqlType,
                                   static_cast<SQLPOINTER>(aCharArray),
                                   nMaxLen,
                                   &pcbValue),
                               _aStatementHandle,SQL_HANDLE_STMT,_xInterface);

        _bWasNull = pcbValue == SQL_NULL_DATA;
        if(_bWasNull)
            return Sequence<sal_Int8>();

        SQLLEN nReadBytes;
        // After the SQLGetData that wrote out to aCharArray the last byte of the data,
        // pcbValue will not be SQL_NO_TOTAL -> we have a reliable count
        if ( (pcbValue == SQL_NO_TOTAL) || (pcbValue >= nMaxLen) )
        {
            // we filled the buffer
            nReadBytes = nMaxLen;
        }
        else
        {
            nReadBytes = pcbValue;
        }
        const sal_Int32 nLen = aData.getLength();
        aData.realloc(nLen + nReadBytes);
        memcpy(aData.getArray() + nLen, aCharArray, nReadBytes);
    }
    return aData;
}

OUString OTools::getStringValue(OConnection const * _pConnection,
                                       SQLHANDLE _aStatementHandle,
                                       sal_Int32 columnIndex,
                                       SQLSMALLINT _fSqlType,
                                       bool &_bWasNull,
                                       const Reference< XInterface >& _xInterface,
                                       const rtl_TextEncoding _nTextEncoding)
{
    OUStringBuffer aData;
    switch(_fSqlType)
    {
    case SQL_WVARCHAR:
    case SQL_WCHAR:
    case SQL_WLONGVARCHAR:
    {
        SQLWCHAR waCharArray[2048];
        static_assert(sizeof(waCharArray) % sizeof(SQLWCHAR) == 0, "must fit in evenly");
        static_assert(sizeof(SQLWCHAR) == 2 || sizeof(SQLWCHAR) == 4, "must be 2 or 4");
        // Size == number of bytes, Len == number of UTF-16 or UCS4 code units
        const SQLLEN nMaxSize = sizeof(waCharArray);
        const SQLLEN nMaxLen  = sizeof(waCharArray) / sizeof(SQLWCHAR);
        static_assert(nMaxLen * sizeof(SQLWCHAR) == nMaxSize, "sizes must match");

        // read the unicode data
        SQLLEN pcbValue = SQL_NO_TOTAL;
        while ((pcbValue == SQL_NO_TOTAL ) || (pcbValue >= nMaxSize) )
        {
            OTools::ThrowException(_pConnection,
                                   _pConnection->functions().GetData(
                                       _aStatementHandle,
                                       static_cast<SQLUSMALLINT>(columnIndex),
                                       SQL_C_WCHAR,
                                       &waCharArray,
                                       SQLLEN(nMaxLen)*sizeof(sal_Unicode),
                                       &pcbValue),
                                   _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            if(_bWasNull)
                return OUString();

            SQLLEN nReadChars;
            OSL_ENSURE( (pcbValue < 0) || (pcbValue % 2 == 0),
                        "ODBC: SQLGetData of SQL_C_WCHAR returned odd number of bytes");
            if ( (pcbValue == SQL_NO_TOTAL) || (pcbValue >= nMaxSize) )
            {
                // we filled the buffer; remove the terminating null character
                nReadChars = nMaxLen-1;
                if ( waCharArray[nReadChars] != 0)
                {
                    SAL_WARN( "connectivity.drivers", "Buggy ODBC driver? Did not null-terminate (variable length) data!");
                    ++nReadChars;
                }
            }
            else
            {
                nReadChars = pcbValue/sizeof(SQLWCHAR);
            }

            appendSQLWCHARs(aData, waCharArray, nReadChars);
        }
        break;
    }
    default:
    {
        char aCharArray[2048];
        // read the unicode data
        const SQLLEN nMaxLen = sizeof(aCharArray);
        SQLLEN pcbValue = SQL_NO_TOTAL;

        while ((pcbValue == SQL_NO_TOTAL ) || (pcbValue >= nMaxLen) )
        {
            OTools::ThrowException(_pConnection,
                                   _pConnection->functions().GetData(
                                       _aStatementHandle,
                                       static_cast<SQLUSMALLINT>(columnIndex),
                                       SQL_C_CHAR,
                                       &aCharArray,
                                       nMaxLen,
                                       &pcbValue),
                                   _aStatementHandle,SQL_HANDLE_STMT,_xInterface);
            _bWasNull = pcbValue == SQL_NULL_DATA;
            if(_bWasNull)
                return OUString();

            SQLLEN nReadChars;
            if ( (pcbValue == SQL_NO_TOTAL) || (pcbValue >= nMaxLen) )
            {
                // we filled the buffer; remove the terminating null character
                nReadChars = nMaxLen-1;
                if ( aCharArray[nReadChars] != 0)
                {
                    SAL_WARN( "connectivity.drivers", "Buggy ODBC driver? Did not null-terminate (variable length) data!");
                    ++nReadChars;
                }
            }
            else
            {
                nReadChars = pcbValue;
            }

            aData.append(OUString(aCharArray, nReadChars, _nTextEncoding));

        }
        break;
    }
    }

    return aData.makeStringAndClear();
}

void OTools::GetInfo(OConnection const * _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     OUString &_rValue,
                     const Reference< XInterface >& _xInterface,
                     rtl_TextEncoding _nTextEncoding)
{
    if (bUseWChar && _pConnection->functions().has(ODBC3SQLFunctionId::GetInfoW))
    {
        SQLWCHAR aValue[512];
        SQLSMALLINT nValueLen=0;
        // SQLGetInfoW takes / outputs count of bytes, not characters
        OTools::ThrowException(_pConnection,
            _pConnection->functions().GetInfoW(_aConnectionHandle,_nInfo,aValue,sizeof(aValue)-sizeof(SQLWCHAR),&nValueLen),
            _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);

        _rValue = toUString(aValue, nValueLen / sizeof(SQLWCHAR));
    }
    else
    {
        SQLCHAR aValue[512];
        SQLSMALLINT nValueLen=0;
        OTools::ThrowException(_pConnection,
            _pConnection->functions().GetInfo(_aConnectionHandle,_nInfo,aValue,sizeof(aValue)-1,&nValueLen),
            _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);

        _rValue = toUString(aValue, nValueLen, _nTextEncoding);
    }
}

void OTools::GetInfo(OConnection const * _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     sal_Int32 &_rValue,
                     const Reference< XInterface >& _xInterface)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        _pConnection->functions().GetInfo(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}

void OTools::GetInfo(OConnection const * _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     SQLUINTEGER &_rValue,
                     const Reference< XInterface >& _xInterface)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        _pConnection->functions().GetInfo(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}

void OTools::GetInfo(OConnection const * _pConnection,
                     SQLHANDLE _aConnectionHandle,
                     SQLUSMALLINT _nInfo,
                     SQLUSMALLINT &_rValue,
                     const Reference< XInterface >& _xInterface)
{
    SQLSMALLINT nValueLen;
    _rValue = 0;    // in case the driver uses only 16 of the 32 bits (as it does, for example, for SQL_CATALOG_LOCATION)
    OTools::ThrowException(_pConnection,
        _pConnection->functions().GetInfo(_aConnectionHandle,_nInfo,&_rValue,sizeof _rValue,&nValueLen),
        _aConnectionHandle,SQL_HANDLE_DBC,_xInterface);
}

sal_Int32 OTools::MapOdbcType2Jdbc(SQLSMALLINT _nType)
{
    sal_Int32 nValue = DataType::VARCHAR;
    switch(_nType)
    {
        case SQL_BIT:
            nValue = DataType::BIT;
            break;
        case SQL_TINYINT:
            nValue = DataType::TINYINT;
            break;
        case SQL_SMALLINT:
            nValue = DataType::SMALLINT;
            break;
        case SQL_INTEGER:
            nValue = DataType::INTEGER;
            break;
        case SQL_BIGINT:
            nValue = DataType::BIGINT;
            break;
        case SQL_FLOAT:
            nValue = DataType::FLOAT;
            break;
        case SQL_REAL:
            nValue = DataType::REAL;
            break;
        case SQL_DOUBLE:
            nValue = DataType::DOUBLE;
            break;
        case SQL_NUMERIC:
            nValue = DataType::NUMERIC;
            break;
        case SQL_DECIMAL:
            nValue = DataType::DECIMAL;
            break;
        case SQL_WCHAR:
        case SQL_CHAR:
            nValue = DataType::CHAR;
            break;
        case SQL_WVARCHAR:
        case SQL_VARCHAR:
            nValue = DataType::VARCHAR;
            break;
        case SQL_WLONGVARCHAR:
        case SQL_LONGVARCHAR:
            nValue = DataType::LONGVARCHAR;
            break;
        case SQL_TYPE_DATE:
        case SQL_DATE:
            nValue = DataType::DATE;
            break;
        case SQL_TYPE_TIME:
        case SQL_TIME:
            nValue = DataType::TIME;
            break;
        case SQL_TYPE_TIMESTAMP:
        case SQL_TIMESTAMP:
            nValue = DataType::TIMESTAMP;
            break;
        case SQL_BINARY:
            nValue = DataType::BINARY;
            break;
        case SQL_VARBINARY:
        case SQL_GUID:
            nValue = DataType::VARBINARY;
            break;
        case SQL_LONGVARBINARY:
            nValue = DataType::LONGVARBINARY;
            break;
        default:
            OSL_FAIL("Invalid type");
    }
    return nValue;
}

// jdbcTypeToOdbc
// Convert the JDBC SQL type to the correct ODBC type

SQLSMALLINT OTools::jdbcTypeToOdbc(sal_Int32 jdbcType)
{
    // For the most part, JDBC types match ODBC types.  We'll
    // just convert the ones that we know are different

    sal_Int32 odbcType = jdbcType;

    switch (jdbcType)
    {
    case DataType::DATE:
        odbcType = SQL_DATE;
        break;
    case DataType::TIME:
        odbcType = SQL_TIME;
        break;
    case DataType::TIMESTAMP:
        odbcType = SQL_TIMESTAMP;
        break;
    // ODBC doesn't have any notion of CLOB or BLOB
    case DataType::CLOB:
        odbcType = SQL_LONGVARCHAR;
        break;
    case DataType::BLOB:
        odbcType = SQL_LONGVARBINARY;
        break;
    }

    return odbcType;
}

void OTools::getBindTypes(bool _bUseOldTimeDate,
                          SQLSMALLINT _nOdbcType,
                          SQLSMALLINT& fCType,
                          SQLSMALLINT& fSqlType
                          )
{
    switch(_nOdbcType)
    {
        case SQL_CHAR:              if (!bUseWChar)
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_CHAR;
                                        break;
                                    }
                                    [[fallthrough]];
        case SQL_WCHAR:             fCType   = SQL_C_WCHAR;
                                    fSqlType = SQL_WCHAR; break;
        case SQL_VARCHAR:           if (!bUseWChar)
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_VARCHAR;
                                        break;
                                    }
                                    [[fallthrough]];
        case SQL_WVARCHAR:          fCType   = SQL_C_WCHAR;
                                    fSqlType = SQL_WVARCHAR; break;
        case SQL_LONGVARCHAR:       if (!bUseWChar)
                                    {
                                        fCType   = SQL_C_CHAR;
                                        fSqlType = SQL_LONGVARCHAR;
                                        break;
                                    }
                                    [[fallthrough]];
        case SQL_WLONGVARCHAR:      fCType   = SQL_C_WCHAR;
                                    fSqlType = SQL_WLONGVARCHAR; break;
        case SQL_DECIMAL:           fCType      = bUseWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                    fSqlType    = SQL_DECIMAL; break;
        case SQL_NUMERIC:           fCType      = bUseWChar ? SQL_C_WCHAR : SQL_C_CHAR;
                                    fSqlType    = SQL_NUMERIC; break;
        case SQL_BIT:               fCType      = SQL_C_TINYINT;
                                    fSqlType    = SQL_INTEGER; break;
        case SQL_TINYINT:           fCType      = SQL_C_TINYINT;
                                    fSqlType    = SQL_TINYINT; break;
        case SQL_SMALLINT:          fCType      = SQL_C_SHORT;
                                    fSqlType    = SQL_SMALLINT; break;
        case SQL_INTEGER:           fCType      = SQL_C_LONG;
                                    fSqlType    = SQL_INTEGER; break;
        case SQL_BIGINT:            fCType      = SQL_C_SBIGINT;
                                    fSqlType    = SQL_BIGINT; break;
        case SQL_FLOAT:             fCType      = SQL_C_FLOAT;
                                    fSqlType    = SQL_FLOAT; break;
        case SQL_REAL:              fCType      = SQL_C_DOUBLE;
                                    fSqlType    = SQL_REAL; break;
        case SQL_DOUBLE:            fCType      = SQL_C_DOUBLE;
                                    fSqlType    = SQL_DOUBLE; break;
        case SQL_BINARY:            fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_BINARY; break;
        case SQL_VARBINARY:
                                    fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_VARBINARY; break;
        case SQL_LONGVARBINARY:     fCType      = SQL_C_BINARY;
                                    fSqlType    = SQL_LONGVARBINARY; break;
        case SQL_DATE:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_DATE;
                                        fSqlType    = SQL_DATE;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_DATE;
                                        fSqlType    = SQL_TYPE_DATE;
                                    }
                                    break;
        case SQL_TIME:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_TIME;
                                        fSqlType    = SQL_TIME;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_TIME;
                                        fSqlType    = SQL_TYPE_TIME;
                                    }
                                    break;
        case SQL_TIMESTAMP:
                                    if(_bUseOldTimeDate)
                                    {
                                        fCType      = SQL_C_TIMESTAMP;
                                        fSqlType    = SQL_TIMESTAMP;
                                    }
                                    else
                                    {
                                        fCType      = SQL_C_TYPE_TIMESTAMP;
                                        fSqlType    = SQL_TYPE_TIMESTAMP;
                                    }
                                    break;
        default:                        fCType      = SQL_C_BINARY;
                                        fSqlType    = SQL_LONGVARBINARY; break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
