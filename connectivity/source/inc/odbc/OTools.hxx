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
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/textenc.h>

enum class ODBC3SQLFunctionId
{
    FIRST,
    AllocHandle,
    DriverConnect,
    GetInfo,
    GetFunctions,
    GetTypeInfo,
    SetConnectAttr,
    GetConnectAttr,
    SetEnvAttr,
    GetEnvAttr,
    SetStmtAttr,
    GetStmtAttr,
    Prepare,
    BindParameter,
    SetCursorName,
    Execute,
    ExecDirect,
    DescribeParam,
    NumParams,
    ParamData,
    PutData,
    RowCount,
    NumResultCols,
    ColAttribute,
    BindCol,
    Fetch,
    FetchScroll,
    GetData,
    SetPos,
    BulkOperations,
    MoreResults,
    GetDiagRec,
    ColumnPrivileges,
    Columns,
    ForeignKeys,
    PrimaryKeys,
    ProcedureColumns,
    Procedures,
    SpecialColumns,
    Statistics,
    TablePrivileges,
    Tables,
    FreeStmt,
    CloseCursor,
    Cancel,
    EndTran,
    Disconnect,
    FreeHandle,
    GetCursorName,
    NativeSql,

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
                @param  _bUseWChar          true when Unicode should be used
                @param  _bUseOldTimeDate    true when the old datetime format should be used
                @param  _nOdbcType          the ODBC sql type
                @param  fCType              the C type for the ODBC type
                @param  fSqlType            the SQL type for the ODBC type
            */
            static void getBindTypes(bool _bUseWChar,
                                     bool _bUseOldTimeDate,
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

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void bindValue(  OConnection const * _pConnection,
                                    SQLHANDLE _aStatementHandle,
                                    sal_Int32 columnIndex,
                                    SQLSMALLINT _nType,
                                    SQLSMALLINT _nMaxLen,
                                    const void* _pValue,
                                    void*       _pData,
                                    SQLLEN *pLen,
                                    const css::uno::Reference< css::uno::XInterface >& _xInterface,
                                    rtl_TextEncoding _nTextEncoding,
                                    bool _bUseOldTimeDate);
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


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
