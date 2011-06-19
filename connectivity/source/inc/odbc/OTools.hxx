/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#define _CONNECTIVITY_OTOOLS_HXX_

#include "odbc/OFunctiondefs.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/textenc.h>

#define ODBC3SQLAllocHandle         1
#define ODBC3SQLConnect             2
#define ODBC3SQLDriverConnect       3
#define ODBC3SQLBrowseConnect       4
#define ODBC3SQLDataSources         5
#define ODBC3SQLDrivers             6
#define ODBC3SQLGetInfo             7
#define ODBC3SQLGetFunctions        8
#define ODBC3SQLGetTypeInfo         9
#define ODBC3SQLSetConnectAttr      10
#define ODBC3SQLGetConnectAttr      11
#define ODBC3SQLSetEnvAttr          12
#define ODBC3SQLGetEnvAttr          13
#define ODBC3SQLSetStmtAttr         14
#define ODBC3SQLGetStmtAttr         15
#define ODBC3SQLPrepare             16
#define ODBC3SQLBindParameter       17
#define ODBC3SQLSetCursorName       18
#define ODBC3SQLExecute             19
#define ODBC3SQLExecDirect          20
#define ODBC3SQLDescribeParam       21
#define ODBC3SQLNumParams           22
#define ODBC3SQLParamData           23
#define ODBC3SQLPutData             24
#define ODBC3SQLRowCount            25
#define ODBC3SQLNumResultCols       26
#define ODBC3SQLDescribeCol         27
#define ODBC3SQLColAttribute        28
#define ODBC3SQLBindCol             29
#define ODBC3SQLFetch               30
#define ODBC3SQLFetchScroll         31
#define ODBC3SQLGetData             32
#define ODBC3SQLSetPos              33
#define ODBC3SQLBulkOperations      34
#define ODBC3SQLMoreResults         35
#define ODBC3SQLGetDiagRec          36
#define ODBC3SQLColumnPrivileges    37
#define ODBC3SQLColumns             38
#define ODBC3SQLForeignKeys         39
#define ODBC3SQLPrimaryKeys         40
#define ODBC3SQLProcedureColumns    41
#define ODBC3SQLProcedures          42
#define ODBC3SQLSpecialColumns      43
#define ODBC3SQLStatistics          44
#define ODBC3SQLTablePrivileges     45
#define ODBC3SQLTables              46
#define ODBC3SQLFreeStmt            47
#define ODBC3SQLCloseCursor         48
#define ODBC3SQLCancel              49
#define ODBC3SQLEndTran             50
#define ODBC3SQLDisconnect          51
#define ODBC3SQLFreeHandle          52
#define ODBC3SQLGetCursorName       53
#define ODBC3SQLNativeSql           54

namespace connectivity
{
    namespace odbc
    {
        class OConnection;

        const sal_uInt32 ODBC_FRACTION_UNITS_PER_HSECOND = 10000000L;
        const sal_Int32 MAX_PUT_DATA_LENGTH = 2000;

        class OOO_DLLPUBLIC_ODBCBASE OTools
        {
        public:
            static void ThrowException( OConnection* _pConnection,
                                        SQLRETURN _rRetCode,
                                        SQLHANDLE _pContext,
                                        SQLSMALLINT _nHandleType,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                        sal_Bool _bNoFound=sal_True,
                                        rtl_TextEncoding _nTextEncoding = RTL_TEXTENCODING_MS_1252)
                                        throw(::com::sun::star::sdbc::SQLException);

            static void GetInfo(OConnection* _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                ::rtl::OUString &_rValue,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                rtl_TextEncoding _nTextEncoding)
                                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(OConnection* _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                sal_Int32 &_rValue,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(OConnection* _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                SQLUSMALLINT &_rValue,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(OConnection* _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                SQLUINTEGER &_rValue,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void GetInfo(OConnection* _pConnection,
                                SQLHANDLE _aConnectionHandle,
                                SQLUSMALLINT _nInfo,
                                sal_Bool &_rValue,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static sal_Int32 MapOdbcType2Jdbc(sal_Int32 _nType);
            static sal_Int32 jdbcTypeToOdbc(sal_Int32 jdbcType);

            static DATE_STRUCT DateToOdbcDate(const ::com::sun::star::util::Date& x)
            {
                DATE_STRUCT aVal;
                aVal.year   = x.Year;
                aVal.month  = x.Month;
                aVal.day    = x.Day;
                return aVal;
            }
            static TIME_STRUCT TimeToOdbcTime(const ::com::sun::star::util::Time& x)
            {
                TIME_STRUCT aVal;
                aVal.hour   = x.Hours;
                aVal.minute = x.Minutes;
                aVal.second = x.Seconds;
                return aVal;
            }
            static TIMESTAMP_STRUCT DateTimeToTimestamp(const ::com::sun::star::util::DateTime& x)
            {
                TIMESTAMP_STRUCT aVal;
                aVal.year       = x.Year;
                aVal.month      = x.Month;
                aVal.day        = x.Day;
                aVal.hour       = x.Hours;
                aVal.minute     = x.Minutes;
                aVal.second     = x.Seconds;
                aVal.fraction   = x.HundredthSeconds * ODBC_FRACTION_UNITS_PER_HSECOND;
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
            static void getBindTypes(sal_Bool _bUseWChar,
                                     sal_Bool _bUseOldTimeDate,
                                     SQLSMALLINT _nOdbcType,
                                     SQLSMALLINT& fCType,
                                     SQLSMALLINT& fSqlType);

            static ::rtl::OUString getStringValue(  OConnection* _pConnection,
                                                    SQLHANDLE _aStatementHandle,
                                                    sal_Int32 columnIndex,
                                                    SQLSMALLINT _fSqlType,
                                                    sal_Bool &_bWasNull,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                                    rtl_TextEncoding _nTextEncoding) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static  ::com::sun::star::uno::Sequence<sal_Int8> getBytesValue(OConnection* _pConnection,
                                                                            SQLHANDLE _aStatementHandle,
                                                                            sal_Int32 columnIndex,
                                                                            SQLSMALLINT _fSqlType,
                                                                            sal_Bool &_bWasNull,
                                                                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static void getValue(   OConnection* _pConnection,
                                    SQLHANDLE _aStatementHandle,
                                    sal_Int32 columnIndex,
                                    SQLSMALLINT _nType,
                                    sal_Bool &_bWasNull,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                    void* _pValue,
                                    SQLLEN _nSize) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            /**
                bindData copies the from pValue to pData
                @param  _nOdbcType          the ODBC sql type
                @param  _bUseWChar          true when Unicode should be used
                @param  _pData              contains a copy of the data to be set
                @param  _pValue             contains the data to be copied
                @param  _nTextEncoding      the text encoding
                @param  _nColumnSize        the columnsize which is a out param
            */
            static void bindData(   SQLSMALLINT _nOdbcType,
                                    sal_Bool _bUseWChar,
                                    sal_Int8 *&_pData,
                                    SQLLEN*& pLen,
                                    const void* _pValue,
                                    rtl_TextEncoding _nTextEncoding,
                                    SQLULEN& _nColumnSize);

            static void bindParameter(  OConnection* _pConnection,
                                        SQLHANDLE _hStmt,
                                        sal_Int32 nPos,
                                        sal_Int8*& pDataBuffer,
                                        sal_Int8* pLenBuffer,
                                        SQLSMALLINT _nJDBCtype,
                                        sal_Bool _bUseWChar,
                                        sal_Bool _bUseOldTimeDate,
                                        const void* _pValue,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                        rtl_TextEncoding _nTextEncoding)
                                         throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static void bindValue(  OConnection* _pConnection,
                                    SQLHANDLE _aStatementHandle,
                                    sal_Int32 columnIndex,
                                    SQLSMALLINT _nType,
                                    SQLSMALLINT _nMaxLen,
                                    const void* _pValue,
                                    void*       _pData,
                                    SQLLEN *pLen,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                    rtl_TextEncoding _nTextEncoding,
                                    sal_Bool _bUseOldTimeDate) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };

        template <class T> void getValue(   OConnection* _pConnection,
                                            SQLHANDLE _aStatementHandle,
                                            sal_Int32 columnIndex,
                                            SQLSMALLINT _nType,
                                            sal_Bool &_bWasNull,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                                            T& _rValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            OTools::getValue(_pConnection,_aStatementHandle,columnIndex,_nType,_bWasNull,_xInterface,&_rValue,sizeof _rValue);
        }
        //-----------------------------------------------------------------------------


    }
}
#endif // _CONNECTIVITY_OTOOLS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
