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
#pragma once

#include <connectivity/odbc.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/textenc.h>

enum class ODBC3SQLFunctionId
{
    FIRST,
    AllocHandle,
    DriverConnect,
    DriverConnectW,
    GetInfo,
    GetInfoW,
    GetFunctions,
    GetTypeInfo,
    SetConnectAttr,
    SetConnectAttrW,
    GetConnectAttr,
    GetConnectAttrW,
    SetEnvAttr,
    GetEnvAttr,
    SetStmtAttr,
    GetStmtAttr,
    Prepare,
    PrepareW,
    BindParameter,
    SetCursorName,
    SetCursorNameW,
    Execute,
    ExecDirect,
    ExecDirectW,
    DescribeParam,
    NumParams,
    ParamData,
    PutData,
    RowCount,
    NumResultCols,
    ColAttribute,
    ColAttributeW,
    BindCol,
    Fetch,
    FetchScroll,
    GetData,
    SetPos,
    BulkOperations,
    MoreResults,
    GetDiagRec,
    GetDiagRecW,
    ColumnPrivileges,
    ColumnPrivilegesW,
    Columns,
    ColumnsW,
    ForeignKeys,
    ForeignKeysW,
    PrimaryKeys,
    PrimaryKeysW,
    ProcedureColumns,
    ProcedureColumnsW,
    Procedures,
    ProceduresW,
    SpecialColumns,
    SpecialColumnsW,
    Statistics,
    StatisticsW,
    TablePrivileges,
    TablePrivilegesW,
    Tables,
    TablesW,
    FreeStmt,
    CloseCursor,
    Cancel,
    EndTran,
    Disconnect,
    FreeHandle,
    GetCursorName,
    GetCursorNameW,
    NativeSql,
    NativeSqlW,

    LAST
};

namespace connectivity::odbc
    {
        class OConnection;

        const sal_Int32 MAX_PUT_DATA_LENGTH = 2000;

        class OOO_DLLPUBLIC_ODBCBASE OTools
        {
        public:
            /// @throws css::sdbc::SQLException
            static void ThrowException( const OConnection* _pConnection,
                                        SQLRETURN _rRetCode,
                                        SQLHANDLE _pContext,
                                        SQLSMALLINT _nHandleType,
                                        const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                        bool _bNoFound=true);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void GetInfo(OConnection const * _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                OUString &_rValue,
                                const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                rtl_TextEncoding _nTextEncoding);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void GetInfo(OConnection const * _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                sal_Int32 &_rValue,
                                const css::uno::Reference< css::uno::XInterface >& _xInterface);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void GetInfo(OConnection const * _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                SQLUSMALLINT &_rValue,
                                const css::uno::Reference< css::uno::XInterface >& _xInterface);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void GetInfo(OConnection const * _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                SQLUINTEGER &_rValue,
                                const css::uno::Reference< css::uno::XInterface >& _xInterface);

            static sal_Int32 MapOdbcType2Jdbc(SQLSMALLINT _nType);
            static SQLSMALLINT jdbcTypeToOdbc(sal_Int32 jdbcType);

            static DATE_STRUCT DateToOdbcDate(const css::util::Date& x)
            {
                DATE_STRUCT aVal;
                aVal.year   = x.Year;
                aVal.month  = x.Month;
                aVal.day    = x.Day;
                return aVal;
            }
            static TIME_STRUCT TimeToOdbcTime(const css::util::Time& x)
            {
                TIME_STRUCT aVal;
                aVal.hour   = x.Hours;
                aVal.minute = x.Minutes;
                aVal.second = x.Seconds;
                return aVal;
            }
            static TIMESTAMP_STRUCT DateTimeToTimestamp(const css::util::DateTime& x)
            {
                TIMESTAMP_STRUCT aVal;
                aVal.year       = x.Year;
                aVal.month      = x.Month;
                aVal.day        = x.Day;
                aVal.hour       = x.Hours;
                aVal.minute     = x.Minutes;
                aVal.second     = x.Seconds;
                aVal.fraction   = x.NanoSeconds;
                return aVal;
            }
            /**
                getBindTypes set the ODBC type for C
                @param  _bUseOldTimeDate    true when the old datetime format should be used
                @param  _nOdbcType          the ODBC sql type
                @param  fCType              the C type for the ODBC type
                @param  fSqlType            the SQL type for the ODBC type
            */
            static void getBindTypes(bool _bUseOldTimeDate,
                                     SQLSMALLINT _nOdbcType,
                                     SQLSMALLINT& fCType,
                                     SQLSMALLINT& fSqlType);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static OUString getStringValue(  OConnection const * _pConnection,
                                                    SQLHANDLE _aStatementHandle,
                                                    sal_Int32 columnIndex,
                                                    SQLSMALLINT _fSqlType,
                                                    bool &_bWasNull,
                                                    const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                                    rtl_TextEncoding _nTextEncoding);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static  css::uno::Sequence<sal_Int8> getBytesValue(const OConnection* _pConnection,
                                                                            SQLHANDLE _aStatementHandle,
                                                                            sal_Int32 columnIndex,
                                                                            SQLSMALLINT _fSqlType,
                                                                            bool &_bWasNull,
                                                                            const css::uno::Reference< css::uno::XInterface >& _xInterface);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void getValue(   OConnection const * _pConnection,
                                    SQLHANDLE _aStatementHandle,
                                    sal_Int32 columnIndex,
                                    SQLSMALLINT _nType,
                                    bool &_bWasNull,
                                    const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                    void* _pValue,
                                    SQLLEN _nSize);
        };

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
        template <class T> void getValue(   const OConnection* _pConnection,
                                            SQLHANDLE _aStatementHandle,
                                            sal_Int32 columnIndex,
                                            SQLSMALLINT _nType,
                                            bool &_bWasNull,
                                            const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                            T& _rValue)
        {
            OTools::getValue(_pConnection,_aStatementHandle,columnIndex,_nType,_bWasNull,_xInterface,&_rValue,sizeof _rValue);
        }

// Keep const_ and reinterpret_cast required to convert strings in one place

inline OUString toUString(const SQLCHAR* str, sal_Int32 len, rtl_TextEncoding enc)
{
    return OUString(reinterpret_cast<const char*>(str), len, enc);
}

inline OUString toUString(const SQLCHAR* str)
{
    return OUString::createFromAscii(reinterpret_cast<const char*>(str));
}

inline OUString toUString(const SQLWCHAR* str, sal_Int32 len)
{
    static_assert(sizeof(SQLWCHAR) == sizeof(sal_Unicode) || sizeof(SQLWCHAR) == sizeof(sal_uInt32));
    if constexpr (sizeof(SQLWCHAR) == sizeof(sal_Unicode))
        return OUString(reinterpret_cast<const sal_Unicode*>(str), len);
    else
    {
        auto* codepoints = reinterpret_cast<const sal_uInt32*>(str);
        return OUString(codepoints, len);
    }
}

// A templated class to encapsulate conversion from our string types into arrays of
// SQLCHAR / SQLWCHAR (non-const, even if used as input balues, and not modified),
// that ODBC functions take. It owns its buffer (important for delayed reads/writes)

template <typename C, size_t CHAR_SIZE = sizeof(C)> class CHARS {};

template <size_t CHAR_SIZE> class SIZED
{
public:
    SQLSMALLINT cch() const { return m_len; } // count of characters
    bool empty() const { return m_len == 0; }
    SQLSMALLINT cb() const { return m_len * CHAR_SIZE; } // count of bytes

protected:
    SQLSMALLINT m_len = 0;
};

template <typename C> class CHARS<C, sizeof(char)> : public SIZED<sizeof(char)>
{
public:
    CHARS() = default;
    CHARS(std::u16string_view str, rtl_TextEncoding encoding)
        : CHARS(OUStringToOString(str, encoding))
    {
    }
    CHARS(const OString& str)
        : m_string(str)
    {
        m_len = std::min(m_string.getLength(), sal_Int32(std::numeric_limits<SQLSMALLINT>::max()));
    }
    C* get() { return reinterpret_cast<C*>(const_cast<char*>(m_string.getStr())); }

private:
    OString m_string; // ref-counted CoW, but in practice always  created ad-hoc
};

template <typename C> class CHARS<C, sizeof(sal_Unicode)> : public SIZED<sizeof(sal_Unicode)>
{
public:
    CHARS() = default;
    CHARS(const OUString& str)
        : m_string(str)
    {
        m_len = std::min(m_string.getLength(), sal_Int32(std::numeric_limits<SQLSMALLINT>::max()));
    }
    C* get() { return reinterpret_cast<C*>(const_cast<sal_Unicode*>(m_string.getStr())); }

private:
    OUString m_string; // ref-counted CoW
};

template <typename C> class CHARS<C, sizeof(sal_uInt32)> : public SIZED<sizeof(sal_uInt32)>
{
public:
    CHARS() = default;
    CHARS(std::u16string_view str)
    {
        auto size = std::min(str.size(), size_t(std::numeric_limits<SQLSMALLINT>::max()));
        m_buf = std::make_unique<sal_uInt32[]>(size + 1);
        auto p = m_buf.get();
        for (size_t i = 0; i < str.size() && o3tl::make_unsigned(p - m_buf.get()) < size; ++p)
            *p = o3tl::iterateCodePoints(str, &i);
        m_len = p - m_buf.get();
        *p = 0;
    }
    C* get() { return reinterpret_cast<C*>(m_buf.get()); }

private:
    std::unique_ptr<sal_uInt32[]> m_buf;
};

using SQLChars = CHARS<SQLCHAR>;
using SQLWChars = CHARS<SQLWCHAR>;

// for now, use wchar only on Windows (see comment in OPreparedStatement::setParameter)
#ifdef _WIN32
const bool bUseWChar = true;
#else
const volatile inline bool bUseWChar = false; // volatile to avoid "unreachabe code" warnings
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
