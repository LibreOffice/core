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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OTOOLS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OTOOLS_HXX

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
    AllocHandle         = 1,
    Connect             = 2,
    DriverConnect       = 3,
    BrowseConnect       = 4,
    DataSources         = 5,
    Drivers             = 6,
    GetInfo             = 7,
    GetFunctions        = 8,
    GetTypeInfo         = 9,
    SetConnectAttr      = 10,
    GetConnectAttr      = 11,
    SetEnvAttr          = 12,
    GetEnvAttr          = 13,
    SetStmtAttr         = 14,
    GetStmtAttr         = 15,
    Prepare             = 16,
    BindParameter       = 17,
    SetCursorName       = 18,
    Execute             = 19,
    ExecDirect          = 20,
    DescribeParam       = 21,
    NumParams           = 22,
    ParamData           = 23,
    PutData             = 24,
    RowCount            = 25,
    NumResultCols       = 26,
    DescribeCol         = 27,
    ColAttribute        = 28,
    BindCol             = 29,
    Fetch               = 30,
    FetchScroll         = 31,
    GetData             = 32,
    SetPos              = 33,
    BulkOperations      = 34,
    MoreResults         = 35,
    GetDiagRec          = 36,
    ColumnPrivileges    = 37,
    Columns             = 38,
    ForeignKeys         = 39,
    PrimaryKeys         = 40,
    ProcedureColumns    = 41,
    Procedures          = 42,
    SpecialColumns      = 43,
    Statistics          = 44,
    TablePrivileges     = 45,
    Tables              = 46,
    FreeStmt            = 47,
    CloseCursor         = 48,
    Cancel              = 49,
    EndTran             = 50,
    Disconnect          = 51,
    FreeHandle          = 52,
    GetCursorName       = 53,
    NativeSql           = 54,
};

namespace connectivity
{
    namespace odbc
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
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
