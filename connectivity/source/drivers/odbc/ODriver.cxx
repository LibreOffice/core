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

#include <sal/config.h>

#include <odbc/OConnection.hxx>
#include <odbc/ODriver.hxx>
#include <resource/sharedresources.hxx>
#include <strings.hrc>

#include <connectivity/dbexception.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/module.h>

#include <utility>

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODBCDriver::ODBCDriver(css::uno::Reference< css::uno::XComponentContext > _xContext)
    :ODriver_BASE(m_aMutex)
    ,m_xContext(std::move(_xContext))
{
}

void ODBCDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    for (auto const& connection : m_xConnections)
    {
        rtl::Reference< OConnection > xComp(connection);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo

OUString ODBCDriver::getImplementationName(  )
{
    return u"com.sun.star.comp.sdbc.ODBCDriver"_ustr;
        // this name is referenced in the configuration and in the odbc.xml
        // Please take care when changing it.
}


Sequence< OUString > ODBCDriver::getSupportedServiceNames(  )
{
    return { u"com.sun.star.sdbc.Driver"_ustr };
}


sal_Bool SAL_CALL ODBCDriver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


Reference< XConnection > SAL_CALL ODBCDriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    if ( ! acceptsURL(url) )
        return nullptr;

    rtl::Reference<OConnection> pCon = new OConnection(EnvironmentHandle(), this);
    pCon->Construct(url,info);
    m_xConnections.push_back(pCon);

    return pCon;
}

sal_Bool SAL_CALL ODBCDriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:odbc:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ )
{
    if ( acceptsURL(url) )
    {
        Sequence< OUString > aBooleanValues{ u"false"_ustr, u"true"_ustr };

        return
        {
            {
                u"CharSet"_ustr,
                u"CharSet of the database."_ustr,
                false,
                {},
                {}
            },
            {
                u"UseCatalog"_ustr,
                u"Use catalog for file-based databases."_ustr,
                false,
                u"false"_ustr,
                aBooleanValues
            },
            {
                u"SystemDriverSettings"_ustr,
                u"Driver settings."_ustr,
                false,
                {},
                {}
            },
            {
                u"ParameterNameSubstitution"_ustr,
                u"Change named parameters with '?'."_ustr,
                false,
                u"false"_ustr,
                aBooleanValues
            },
            {
                u"IgnoreDriverPrivileges"_ustr,
                u"Ignore the privileges from the database driver."_ustr,
                false,
                u"false"_ustr,
                aBooleanValues
            },
            {
                u"IsAutoRetrievingEnabled"_ustr,
                u"Retrieve generated values."_ustr,
                false,
                u"false"_ustr,
                aBooleanValues
            },
            {
                u"AutoRetrievingStatement"_ustr,
                u"Auto-increment statement."_ustr,
                false,
                {},
                {}
            },
            {
                u"GenerateASBeforeCorrelationName"_ustr,
                u"Generate AS before table correlation names."_ustr,
                false,
                u"false"_ustr,
                aBooleanValues
            },
            {
                u"EscapeDateTime"_ustr,
                u"Escape date time format."_ustr,
                false,
                u"true"_ustr,
                aBooleanValues
            }
        };
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
}

sal_Int32 SAL_CALL ODBCDriver::getMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL ODBCDriver::getMinorVersion(  )
{
    return 0;
}

// Implib definitions for ODBC-DLL/shared library:

namespace
{
constinit oslGenericFunction pODBC3SQLFunctions[static_cast<size_t>(ODBC3SQLFunctionId::LAST)]{};

bool LoadFunctions(oslModule pODBCso)
{
    auto load = [pODBCso](ODBC3SQLFunctionId id, const OUString& name)
    {
        assert(id > ODBC3SQLFunctionId::FIRST && id < ODBC3SQLFunctionId::LAST);
        auto& rpFunc = pODBC3SQLFunctions[static_cast<size_t>(id)];
        assert(rpFunc == nullptr);
        rpFunc = osl_getFunctionSymbol(pODBCso, name.pData);
        return rpFunc != nullptr;
    };

    // Optional functions for Unicode support
    if (bUseWChar)
    {
        load(ODBC3SQLFunctionId::DriverConnectW,    u"SQLDriverConnectW"_ustr);
        load(ODBC3SQLFunctionId::GetInfoW,          u"SQLGetInfoW"_ustr);
        load(ODBC3SQLFunctionId::SetConnectAttrW,   u"SQLSetConnectAttrW"_ustr);
        load(ODBC3SQLFunctionId::GetConnectAttrW,   u"SQLGetConnectAttrW"_ustr);
        load(ODBC3SQLFunctionId::PrepareW,          u"SQLPrepareW"_ustr);
        load(ODBC3SQLFunctionId::SetCursorNameW,    u"SQLSetCursorNameW"_ustr);
        load(ODBC3SQLFunctionId::ExecDirectW,       u"SQLExecDirectW"_ustr);
        load(ODBC3SQLFunctionId::ColAttributeW,     u"SQLColAttributeW"_ustr);
        load(ODBC3SQLFunctionId::GetDiagRecW,       u"SQLGetDiagRecW"_ustr);
        load(ODBC3SQLFunctionId::ColumnPrivilegesW, u"SQLColumnPrivilegesW"_ustr);
        load(ODBC3SQLFunctionId::ColumnsW,          u"SQLColumnsW"_ustr);
        load(ODBC3SQLFunctionId::ForeignKeysW,      u"SQLForeignKeysW"_ustr);
        load(ODBC3SQLFunctionId::PrimaryKeysW,      u"SQLPrimaryKeysW"_ustr);
        load(ODBC3SQLFunctionId::ProcedureColumnsW, u"SQLProcedureColumnsW"_ustr);
        load(ODBC3SQLFunctionId::ProceduresW,       u"SQLProceduresW"_ustr);
        load(ODBC3SQLFunctionId::SpecialColumnsW,   u"SQLSpecialColumnsW"_ustr);
        load(ODBC3SQLFunctionId::StatisticsW,       u"SQLStatisticsW"_ustr);
        load(ODBC3SQLFunctionId::TablePrivilegesW,  u"SQLTablePrivilegesW"_ustr);
        load(ODBC3SQLFunctionId::TablesW,           u"SQLTablesW"_ustr);
        load(ODBC3SQLFunctionId::GetCursorNameW,    u"SQLGetCursorNameW"_ustr);
        load(ODBC3SQLFunctionId::NativeSqlW,        u"SQLNativeSqlW"_ustr);
    }

    return load(ODBC3SQLFunctionId::AllocHandle,      u"SQLAllocHandle"_ustr)
        && load(ODBC3SQLFunctionId::DriverConnect,    u"SQLDriverConnect"_ustr)
        && load(ODBC3SQLFunctionId::GetInfo,          u"SQLGetInfo"_ustr)
        && load(ODBC3SQLFunctionId::GetFunctions,     u"SQLGetFunctions"_ustr)
        && load(ODBC3SQLFunctionId::GetTypeInfo,      u"SQLGetTypeInfo"_ustr)
        && load(ODBC3SQLFunctionId::SetConnectAttr,   u"SQLSetConnectAttr"_ustr)
        && load(ODBC3SQLFunctionId::GetConnectAttr,   u"SQLGetConnectAttr"_ustr)
        && load(ODBC3SQLFunctionId::SetEnvAttr,       u"SQLSetEnvAttr"_ustr)
        && load(ODBC3SQLFunctionId::GetEnvAttr,       u"SQLGetEnvAttr"_ustr)
        && load(ODBC3SQLFunctionId::SetStmtAttr,      u"SQLSetStmtAttr"_ustr)
        && load(ODBC3SQLFunctionId::GetStmtAttr,      u"SQLGetStmtAttr"_ustr)
        && load(ODBC3SQLFunctionId::Prepare,          u"SQLPrepare"_ustr)
        && load(ODBC3SQLFunctionId::BindParameter,    u"SQLBindParameter"_ustr)
        && load(ODBC3SQLFunctionId::SetCursorName,    u"SQLSetCursorName"_ustr)
        && load(ODBC3SQLFunctionId::Execute,          u"SQLExecute"_ustr)
        && load(ODBC3SQLFunctionId::ExecDirect,       u"SQLExecDirect"_ustr)
        && load(ODBC3SQLFunctionId::DescribeParam,    u"SQLDescribeParam"_ustr)
        && load(ODBC3SQLFunctionId::NumParams,        u"SQLNumParams"_ustr)
        && load(ODBC3SQLFunctionId::ParamData,        u"SQLParamData"_ustr)
        && load(ODBC3SQLFunctionId::PutData,          u"SQLPutData"_ustr)
        && load(ODBC3SQLFunctionId::RowCount,         u"SQLRowCount"_ustr)
        && load(ODBC3SQLFunctionId::NumResultCols,    u"SQLNumResultCols"_ustr)
        && load(ODBC3SQLFunctionId::ColAttribute,     u"SQLColAttribute"_ustr)
        && load(ODBC3SQLFunctionId::BindCol,          u"SQLBindCol"_ustr)
        && load(ODBC3SQLFunctionId::Fetch,            u"SQLFetch"_ustr)
        && load(ODBC3SQLFunctionId::FetchScroll,      u"SQLFetchScroll"_ustr)
        && load(ODBC3SQLFunctionId::GetData,          u"SQLGetData"_ustr)
        && load(ODBC3SQLFunctionId::SetPos,           u"SQLSetPos"_ustr)
        && load(ODBC3SQLFunctionId::BulkOperations,   u"SQLBulkOperations"_ustr)
        && load(ODBC3SQLFunctionId::MoreResults,      u"SQLMoreResults"_ustr)
        && load(ODBC3SQLFunctionId::GetDiagRec,       u"SQLGetDiagRec"_ustr)
        && load(ODBC3SQLFunctionId::ColumnPrivileges, u"SQLColumnPrivileges"_ustr)
        && load(ODBC3SQLFunctionId::Columns,          u"SQLColumns"_ustr)
        && load(ODBC3SQLFunctionId::ForeignKeys,      u"SQLForeignKeys"_ustr)
        && load(ODBC3SQLFunctionId::PrimaryKeys,      u"SQLPrimaryKeys"_ustr)
        && load(ODBC3SQLFunctionId::ProcedureColumns, u"SQLProcedureColumns"_ustr)
        && load(ODBC3SQLFunctionId::Procedures,       u"SQLProcedures"_ustr)
        && load(ODBC3SQLFunctionId::SpecialColumns,   u"SQLSpecialColumns"_ustr)
        && load(ODBC3SQLFunctionId::Statistics,       u"SQLStatistics"_ustr)
        && load(ODBC3SQLFunctionId::TablePrivileges,  u"SQLTablePrivileges"_ustr)
        && load(ODBC3SQLFunctionId::Tables,           u"SQLTables"_ustr)
        && load(ODBC3SQLFunctionId::FreeStmt,         u"SQLFreeStmt"_ustr)
        && load(ODBC3SQLFunctionId::CloseCursor,      u"SQLCloseCursor"_ustr)
        && load(ODBC3SQLFunctionId::Cancel,           u"SQLCancel"_ustr)
        && load(ODBC3SQLFunctionId::EndTran,          u"SQLEndTran"_ustr)
        && load(ODBC3SQLFunctionId::Disconnect,       u"SQLDisconnect"_ustr)
        && load(ODBC3SQLFunctionId::FreeHandle,       u"SQLFreeHandle"_ustr)
        && load(ODBC3SQLFunctionId::GetCursorName,    u"SQLGetCursorName"_ustr)
        && load(ODBC3SQLFunctionId::NativeSql,        u"SQLNativeSql"_ustr);
}

// Take care of Dynamically loading of the DLL/shared lib and Addresses:
// Returns sal_True at success
bool LoadLibrary_ODBC3(OUString &_rPath)
{
    static bool bLoaded = false;
    static oslModule pODBCso = nullptr;

    if (bLoaded)
        return true;
#ifdef DISABLE_DYNLOADING
    (void)_rPath;
#else
#ifdef _WIN32
    _rPath = "ODBC32.DLL";
#endif
#ifdef UNX
 #ifdef MACOSX
    _rPath = "libiodbc.dylib";
 #else
    _rPath = "libodbc.so.2";
    pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    if ( !pODBCso )
    {
        _rPath = "libodbc.so.1";
        pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
    }
    if ( !pODBCso )
        _rPath = "libodbc.so";

 #endif   /* MACOSX */
#endif

    if ( !pODBCso )
        pODBCso = osl_loadModule( _rPath.pData,SAL_LOADMODULE_NOW );
#endif // DISABLE_DYNLOADING
    if( !pODBCso)
        return false;

    bLoaded = LoadFunctions(pODBCso);
    return bLoaded;
}

class ORealOdbcDriver : public connectivity::odbc::ODBCDriver, public connectivity::odbc::Functions
{
public:
    explicit ORealOdbcDriver(const css::uno::Reference<css::uno::XComponentContext>& _rxContext)
        : ODBCDriver(_rxContext)
    {
    }
    const Functions& functions() const override { return *this; }

    // Functions

    bool has(ODBC3SQLFunctionId id) const override
    {
        assert(id > ODBC3SQLFunctionId::FIRST && id < ODBC3SQLFunctionId::LAST);
        return pODBC3SQLFunctions[static_cast<size_t>(id)] != nullptr;
    }

    SQLRETURN AllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle,
                          SQLHANDLE* OutputHandlePtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::AllocHandle, HandleType, InputHandle, OutputHandlePtr);
    }

    SQLRETURN DriverConnect(SQLHDBC ConnectionHandle, HWND WindowHandle,
                            SQLCHAR* InConnectionString, SQLSMALLINT StringLength1,
                            SQLCHAR* OutConnectionString, SQLSMALLINT BufferLength,
                            SQLSMALLINT* StringLength2Ptr,
                            SQLUSMALLINT DriverCompletion) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::DriverConnect, ConnectionHandle, WindowHandle,
                        InConnectionString, StringLength1, OutConnectionString, BufferLength,
                        StringLength2Ptr, DriverCompletion);
    }

    SQLRETURN DriverConnectW(SQLHDBC ConnectionHandle, HWND WindowHandle,
                             SQLWCHAR* InConnectionString, SQLSMALLINT StringLength1,
                             SQLWCHAR* OutConnectionString, SQLSMALLINT BufferLength,
                             SQLSMALLINT* StringLength2Ptr,
                             SQLUSMALLINT DriverCompletion) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::DriverConnectW, ConnectionHandle, WindowHandle,
                        InConnectionString, StringLength1, OutConnectionString, BufferLength,
                        StringLength2Ptr, DriverCompletion);
    }

    SQLRETURN GetInfo(SQLHDBC ConnectionHandle, SQLUSMALLINT InfoType, SQLPOINTER InfoValuePtr,
                      SQLSMALLINT BufferLength, SQLSMALLINT* StringLengthPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetInfo, ConnectionHandle, InfoType, InfoValuePtr,
                        BufferLength, StringLengthPtr);
    }

    SQLRETURN GetInfoW(SQLHDBC ConnectionHandle, SQLUSMALLINT InfoType, SQLPOINTER InfoValuePtr,
                       SQLSMALLINT BufferLength, SQLSMALLINT* StringLengthPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetInfoW, ConnectionHandle, InfoType, InfoValuePtr,
                        BufferLength, StringLengthPtr);
    }

    SQLRETURN GetFunctions(SQLHDBC ConnectionHandle, SQLUSMALLINT FunctionId,
                           SQLUSMALLINT* SupportedPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetFunctions, ConnectionHandle, FunctionId,
                        SupportedPtr);
    }

    SQLRETURN GetTypeInfo(SQLHSTMT StatementHandle, SQLSMALLINT DataType) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetTypeInfo, StatementHandle, DataType);
    }

    SQLRETURN SetConnectAttr(SQLHDBC ConnectionHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                             SQLINTEGER StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetConnectAttr, ConnectionHandle, Attribute, ValuePtr,
                        StringLength);
    }

    SQLRETURN SetConnectAttrW(SQLHDBC ConnectionHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                              SQLINTEGER StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetConnectAttrW, ConnectionHandle, Attribute, ValuePtr,
                        StringLength);
    }

    SQLRETURN GetConnectAttr(SQLHDBC ConnectionHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                             SQLINTEGER BufferLength, SQLINTEGER* StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetConnectAttr, ConnectionHandle, Attribute, ValuePtr,
                        BufferLength, StringLength);
    }

    SQLRETURN GetConnectAttrW(SQLHDBC ConnectionHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                              SQLINTEGER BufferLength, SQLINTEGER* StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetConnectAttrW, ConnectionHandle, Attribute, ValuePtr,
                        BufferLength, StringLength);
    }

    SQLRETURN SetEnvAttr(SQLHENV EnvironmentHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                         SQLINTEGER StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetEnvAttr, EnvironmentHandle, Attribute, ValuePtr,
                        StringLength);
    }

    SQLRETURN GetEnvAttr(SQLHENV EnvironmentHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                         SQLINTEGER BufferLength, SQLINTEGER* StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetEnvAttr, EnvironmentHandle, Attribute, ValuePtr,
                        BufferLength, StringLength);
    }

    SQLRETURN SetStmtAttr(SQLHSTMT StatementHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                          SQLINTEGER StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetStmtAttr, StatementHandle, Attribute, ValuePtr,
                        StringLength);
    }

    SQLRETURN GetStmtAttr(SQLHSTMT StatementHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                          SQLINTEGER BufferLength, SQLINTEGER* StringLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetStmtAttr, StatementHandle, Attribute, ValuePtr,
                        BufferLength, StringLength);
    }

    SQLRETURN Prepare(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                      SQLINTEGER TextLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Prepare, StatementHandle, StatementText, TextLength);
    }

    SQLRETURN PrepareW(SQLHSTMT StatementHandle, SQLWCHAR* StatementText,
                       SQLINTEGER TextLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::PrepareW, StatementHandle, StatementText, TextLength);
    }

    SQLRETURN BindParameter(SQLHSTMT StatementHandle, SQLUSMALLINT ParameterNumber,
                            SQLSMALLINT InputOutputType, SQLSMALLINT ValueType,
                            SQLSMALLINT ParameterType, SQLULEN ColumnSize,
                            SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr,
                            SQLLEN BufferLength, SQLLEN* StrLen_or_IndPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::BindParameter, StatementHandle, ParameterNumber,
                        InputOutputType, ValueType, ParameterType, ColumnSize, DecimalDigits,
                        ParameterValuePtr, BufferLength, StrLen_or_IndPtr);
    }

    SQLRETURN SetCursorName(SQLHSTMT StatementHandle, SQLCHAR* CursorName,
                            SQLSMALLINT NameLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetCursorName, StatementHandle, CursorName, NameLength);
    }

    SQLRETURN SetCursorNameW(SQLHSTMT StatementHandle, SQLWCHAR* CursorName,
                             SQLSMALLINT NameLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetCursorNameW, StatementHandle, CursorName, NameLength);
    }

    SQLRETURN Execute(SQLHSTMT StatementHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Execute, StatementHandle);
    }

    SQLRETURN ExecDirect(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                         SQLINTEGER TextLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ExecDirect, StatementHandle, StatementText, TextLength);
    }

    SQLRETURN ExecDirectW(SQLHSTMT StatementHandle, SQLWCHAR* StatementText,
                          SQLINTEGER TextLength) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ExecDirectW, StatementHandle, StatementText, TextLength);
    }

    SQLRETURN DescribeParam(SQLHSTMT StatementHandle, SQLUSMALLINT ParameterNumber,
                            SQLSMALLINT* DataTypePtr, SQLULEN* ParameterSizePtr,
                            SQLSMALLINT* DecimalDigitsPtr, SQLSMALLINT* NullablePtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::DescribeParam, StatementHandle, ParameterNumber,
                        DataTypePtr, ParameterSizePtr, DecimalDigitsPtr, NullablePtr);
    }

    SQLRETURN NumParams(SQLHSTMT StatementHandle, SQLSMALLINT* ParameterCountPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::NumParams, StatementHandle, ParameterCountPtr);
    }

    SQLRETURN ParamData(SQLHSTMT StatementHandle, SQLPOINTER* ValuePtrPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ParamData, StatementHandle, ValuePtrPtr);
    }

    SQLRETURN PutData(SQLHSTMT StatementHandle, SQLPOINTER DataPtr,
                      SQLLEN StrLen_or_Ind) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::PutData, StatementHandle, DataPtr, StrLen_or_Ind);
    }

    SQLRETURN RowCount(SQLHSTMT StatementHandle, SQLLEN* RowCountPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::RowCount, StatementHandle, RowCountPtr);
    }

    SQLRETURN NumResultCols(SQLHSTMT StatementHandle, SQLSMALLINT* ColumnCountPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::NumResultCols, StatementHandle, ColumnCountPtr);
    }

    SQLRETURN ColAttribute(SQLHSTMT StatementHandle, SQLUSMALLINT ColumnNumber,
                           SQLUSMALLINT FieldIdentifier, SQLPOINTER CharacterAttributePtr,
                           SQLSMALLINT BufferLength, SQLSMALLINT* StringLengthPtr,
                           SQLLEN* NumericAttributePtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ColAttribute, StatementHandle, ColumnNumber,
                        FieldIdentifier, CharacterAttributePtr, BufferLength, StringLengthPtr,
                        NumericAttributePtr);
    }

    SQLRETURN ColAttributeW(SQLHSTMT StatementHandle, SQLUSMALLINT ColumnNumber,
                            SQLUSMALLINT FieldIdentifier, SQLPOINTER CharacterAttributePtr,
                            SQLSMALLINT BufferLength, SQLSMALLINT* StringLengthPtr,
                            SQLLEN* NumericAttributePtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ColAttributeW, StatementHandle, ColumnNumber,
                        FieldIdentifier, CharacterAttributePtr, BufferLength, StringLengthPtr,
                        NumericAttributePtr);
    }

    SQLRETURN BindCol(SQLHSTMT StatementHandle, SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                      SQLPOINTER TargetValuePtr, SQLLEN BufferLength,
                      SQLLEN* StrLen_or_IndPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::BindCol, StatementHandle, ColumnNumber, TargetType,
                        TargetValuePtr, BufferLength, StrLen_or_IndPtr);
    }

    SQLRETURN Fetch(SQLHSTMT StatementHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Fetch, StatementHandle);
    }

    SQLRETURN FetchScroll(SQLHSTMT StatementHandle, SQLSMALLINT FetchOrientation,
                          SQLLEN FetchOffset) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::FetchScroll, StatementHandle, FetchOrientation,
                        FetchOffset);
    }

    SQLRETURN GetData(SQLHSTMT StatementHandle, SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                      SQLPOINTER TargetValuePtr, SQLLEN BufferLength,
                      SQLLEN* StrLen_or_IndPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetData, StatementHandle, ColumnNumber, TargetType,
                        TargetValuePtr, BufferLength, StrLen_or_IndPtr);
    }

    SQLRETURN SetPos(SQLHSTMT StatementHandle, SQLSETPOSIROW RowNumber, SQLUSMALLINT Operation,
                     SQLUSMALLINT LockType) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SetPos, StatementHandle, RowNumber, Operation,
                        LockType);
    }

    SQLRETURN BulkOperations(SQLHSTMT StatementHandle, SQLSMALLINT Operation) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::BulkOperations, StatementHandle, Operation);
    }

    SQLRETURN MoreResults(SQLHSTMT StatementHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::MoreResults, StatementHandle);
    }

    SQLRETURN GetDiagRec(SQLSMALLINT HandleType, SQLHANDLE Handle, SQLSMALLINT RecNumber,
                         SQLCHAR* Sqlstate, SQLINTEGER* NativeErrorPtr, SQLCHAR* MessageText,
                         SQLSMALLINT BufferLength, SQLSMALLINT* TextLengthPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetDiagRec, HandleType, Handle, RecNumber, Sqlstate,
                        NativeErrorPtr, MessageText, BufferLength, TextLengthPtr);
    }

    SQLRETURN GetDiagRecW(SQLSMALLINT HandleType, SQLHANDLE Handle, SQLSMALLINT RecNumber,
                          SQLWCHAR* Sqlstate, SQLINTEGER* NativeErrorPtr, SQLWCHAR* MessageText,
                          SQLSMALLINT BufferLength, SQLSMALLINT* TextLengthPtr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetDiagRecW, HandleType, Handle, RecNumber, Sqlstate,
                        NativeErrorPtr, MessageText, BufferLength, TextLengthPtr);
    }

    SQLRETURN ColumnPrivileges(SQLHSTMT StatementHandle, SQLCHAR* CatalogName,
                               SQLSMALLINT NameLength1, SQLCHAR* SchemaName,
                               SQLSMALLINT NameLength2, SQLCHAR* TableName, SQLSMALLINT NameLength3,
                               SQLCHAR* ColumnName, SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ColumnPrivileges, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, TableName, NameLength3, ColumnName,
                        NameLength4);
    }

    SQLRETURN ColumnPrivilegesW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName,
                                SQLSMALLINT NameLength1, SQLWCHAR* SchemaName,
                                SQLSMALLINT NameLength2, SQLWCHAR* TableName, SQLSMALLINT NameLength3,
                                SQLWCHAR* ColumnName, SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ColumnPrivilegesW, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, TableName, NameLength3, ColumnName,
                        NameLength4);
    }

    SQLRETURN Columns(SQLHSTMT StatementHandle, SQLCHAR* CatalogName, SQLSMALLINT NameLength1,
                      SQLCHAR* SchemaName, SQLSMALLINT NameLength2, SQLCHAR* TableName,
                      SQLSMALLINT NameLength3, SQLCHAR* ColumnName,
                      SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Columns, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, ColumnName, NameLength4);
    }

    SQLRETURN ColumnsW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName, SQLSMALLINT NameLength1,
                       SQLWCHAR* SchemaName, SQLSMALLINT NameLength2, SQLWCHAR* TableName,
                       SQLSMALLINT NameLength3, SQLWCHAR* ColumnName,
                       SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ColumnsW, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, ColumnName, NameLength4);
    }

    SQLRETURN ForeignKeys(SQLHSTMT StatementHandle, SQLCHAR* PKCatalogName, SQLSMALLINT NameLength1,
                          SQLCHAR* PKSchemaName, SQLSMALLINT NameLength2, SQLCHAR* PKTableName,
                          SQLSMALLINT NameLength3, SQLCHAR* FKCatalogName, SQLSMALLINT NameLength4,
                          SQLCHAR* FKSchemaName, SQLSMALLINT NameLength5, SQLCHAR* FKTableName,
                          SQLSMALLINT NameLength6) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ForeignKeys, StatementHandle, PKCatalogName,
                        NameLength1, PKSchemaName, NameLength2, PKTableName, NameLength3,
                        FKCatalogName, NameLength4, FKSchemaName, NameLength5, FKTableName,
                        NameLength6);
    }

    SQLRETURN ForeignKeysW(SQLHSTMT StatementHandle, SQLWCHAR* PKCatalogName, SQLSMALLINT NameLength1,
                           SQLWCHAR* PKSchemaName, SQLSMALLINT NameLength2, SQLWCHAR* PKTableName,
                           SQLSMALLINT NameLength3, SQLWCHAR* FKCatalogName, SQLSMALLINT NameLength4,
                           SQLWCHAR* FKSchemaName, SQLSMALLINT NameLength5, SQLWCHAR* FKTableName,
                           SQLSMALLINT NameLength6) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ForeignKeysW, StatementHandle, PKCatalogName,
                        NameLength1, PKSchemaName, NameLength2, PKTableName, NameLength3,
                        FKCatalogName, NameLength4, FKSchemaName, NameLength5, FKTableName,
                        NameLength6);
    }

    SQLRETURN PrimaryKeys(SQLHSTMT StatementHandle, SQLCHAR* CatalogName, SQLSMALLINT NameLength1,
                          SQLCHAR* SchemaName, SQLSMALLINT NameLength2, SQLCHAR* TableName,
                          SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::PrimaryKeys, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3);
    }

    SQLRETURN PrimaryKeysW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName, SQLSMALLINT NameLength1,
                           SQLWCHAR* SchemaName, SQLSMALLINT NameLength2, SQLWCHAR* TableName,
                           SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::PrimaryKeysW, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3);
    }

    SQLRETURN ProcedureColumns(SQLHSTMT StatementHandle, SQLCHAR* CatalogName,
                               SQLSMALLINT NameLength1, SQLCHAR* SchemaName,
                               SQLSMALLINT NameLength2, SQLCHAR* ProcName, SQLSMALLINT NameLength3,
                               SQLCHAR* ColumnName, SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ProcedureColumns, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, ProcName, NameLength3, ColumnName,
                        NameLength4);
    }

    SQLRETURN ProcedureColumnsW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName,
                                SQLSMALLINT NameLength1, SQLWCHAR* SchemaName,
                                SQLSMALLINT NameLength2, SQLWCHAR* ProcName, SQLSMALLINT NameLength3,
                                SQLWCHAR* ColumnName, SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ProcedureColumnsW, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, ProcName, NameLength3, ColumnName,
                        NameLength4);
    }

    SQLRETURN Procedures(SQLHSTMT StatementHandle, SQLCHAR* CatalogName, SQLSMALLINT NameLength1,
                         SQLCHAR* SchemaName, SQLSMALLINT NameLength2, SQLCHAR* ProcName,
                         SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Procedures, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, ProcName, NameLength3);
    }

    SQLRETURN ProceduresW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName, SQLSMALLINT NameLength1,
                          SQLWCHAR* SchemaName, SQLSMALLINT NameLength2, SQLWCHAR* ProcName,
                          SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::ProceduresW, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, ProcName, NameLength3);
    }

    SQLRETURN SpecialColumns(SQLHSTMT StatementHandle, SQLUSMALLINT IdentifierType,
                             SQLCHAR* CatalogName, SQLSMALLINT NameLength1, SQLCHAR* SchemaName,
                             SQLSMALLINT NameLength2, SQLCHAR* TableName, SQLSMALLINT NameLength3,
                             SQLUSMALLINT Scope, SQLUSMALLINT Nullable) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SpecialColumns, StatementHandle, IdentifierType,
                        CatalogName, NameLength1, SchemaName, NameLength2, TableName, NameLength3,
                        Scope, Nullable);
    }

    SQLRETURN SpecialColumnsW(SQLHSTMT StatementHandle, SQLUSMALLINT IdentifierType,
                              SQLWCHAR* CatalogName, SQLSMALLINT NameLength1, SQLWCHAR* SchemaName,
                              SQLSMALLINT NameLength2, SQLWCHAR* TableName, SQLSMALLINT NameLength3,
                              SQLUSMALLINT Scope, SQLUSMALLINT Nullable) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::SpecialColumnsW, StatementHandle, IdentifierType,
                        CatalogName, NameLength1, SchemaName, NameLength2, TableName, NameLength3,
                        Scope, Nullable);
    }

    SQLRETURN Statistics(SQLHSTMT StatementHandle, SQLCHAR* CatalogName, SQLSMALLINT NameLength1,
                         SQLCHAR* SchemaName, SQLSMALLINT NameLength2, SQLCHAR* TableName,
                         SQLSMALLINT NameLength3, SQLUSMALLINT Unique,
                         SQLUSMALLINT Reserved) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Statistics, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, Unique, Reserved);
    }

    SQLRETURN StatisticsW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName, SQLSMALLINT NameLength1,
                          SQLWCHAR* SchemaName, SQLSMALLINT NameLength2, SQLWCHAR* TableName,
                          SQLSMALLINT NameLength3, SQLUSMALLINT Unique,
                          SQLUSMALLINT Reserved) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::StatisticsW, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, Unique, Reserved);
    }

    SQLRETURN TablePrivileges(SQLHSTMT StatementHandle, SQLCHAR* CatalogName,
                              SQLSMALLINT NameLength1, SQLCHAR* SchemaName, SQLSMALLINT NameLength2,
                              SQLCHAR* TableName, SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::TablePrivileges, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, TableName, NameLength3);
    }

    SQLRETURN TablePrivilegesW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName,
                               SQLSMALLINT NameLength1, SQLWCHAR* SchemaName, SQLSMALLINT NameLength2,
                               SQLWCHAR* TableName, SQLSMALLINT NameLength3) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::TablePrivilegesW, StatementHandle, CatalogName,
                        NameLength1, SchemaName, NameLength2, TableName, NameLength3);
    }

    SQLRETURN Tables(SQLHSTMT StatementHandle, SQLCHAR* CatalogName, SQLSMALLINT NameLength1,
                     SQLCHAR* SchemaName, SQLSMALLINT NameLength2, SQLCHAR* TableName,
                     SQLSMALLINT NameLength3, SQLCHAR* TableType,
                     SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Tables, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, TableType, NameLength4);
    }

    SQLRETURN TablesW(SQLHSTMT StatementHandle, SQLWCHAR* CatalogName, SQLSMALLINT NameLength1,
                      SQLWCHAR* SchemaName, SQLSMALLINT NameLength2, SQLWCHAR* TableName,
                      SQLSMALLINT NameLength3, SQLWCHAR* TableType,
                      SQLSMALLINT NameLength4) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::TablesW, StatementHandle, CatalogName, NameLength1,
                        SchemaName, NameLength2, TableName, NameLength3, TableType, NameLength4);
    }

    SQLRETURN FreeStmt(SQLHSTMT StatementHandle, SQLUSMALLINT Option) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::FreeStmt, StatementHandle, Option);
    }

    SQLRETURN CloseCursor(SQLHSTMT StatementHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::CloseCursor, StatementHandle);
    }

    SQLRETURN Cancel(SQLHSTMT StatementHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Cancel, StatementHandle);
    }

    SQLRETURN EndTran(SQLSMALLINT HandleType, SQLHANDLE Handle,
                      SQLSMALLINT CompletionType) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::EndTran, HandleType, Handle, CompletionType);
    }

    SQLRETURN Disconnect(SQLHDBC ConnectionHandle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::Disconnect, ConnectionHandle);
    }

    SQLRETURN FreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::FreeHandle, HandleType, Handle);
    }

    SQLRETURN GetCursorName(SQLHSTMT StatementHandle, SQLCHAR* CursorName, SQLSMALLINT BufferLength,
                            SQLSMALLINT* NameLength2) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetCursorName, StatementHandle, CursorName,
                        BufferLength, NameLength2);
    }

    SQLRETURN GetCursorNameW(SQLHSTMT StatementHandle, SQLWCHAR* CursorName, SQLSMALLINT BufferLength,
                             SQLSMALLINT* NameLength2) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::GetCursorNameW, StatementHandle, CursorName,
                        BufferLength, NameLength2);
    }

    SQLRETURN NativeSql(SQLHDBC ConnectionHandle, SQLCHAR* InStatementText, SQLINTEGER TextLength1,
                        SQLCHAR* OutStatementText, SQLINTEGER BufferLength,
                        SQLINTEGER* TextLength2Ptr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::NativeSql, ConnectionHandle, InStatementText,
                        TextLength1, OutStatementText, BufferLength, TextLength2Ptr);
    }

    SQLRETURN NativeSqlW(SQLHDBC ConnectionHandle, SQLWCHAR* InStatementText, SQLINTEGER TextLength1,
                         SQLWCHAR* OutStatementText, SQLINTEGER BufferLength,
                         SQLINTEGER* TextLength2Ptr) const override
    {
        return ODBCFunc(ODBC3SQLFunctionId::NativeSqlW, ConnectionHandle, InStatementText,
                        TextLength1, OutStatementText, BufferLength, TextLength2Ptr);
    }

protected:
    virtual SQLHANDLE EnvironmentHandle() override;

private:
    template <typename... Args> static SQLRETURN ODBCFunc(ODBC3SQLFunctionId id, Args... args)
    {
        assert(id > ODBC3SQLFunctionId::FIRST && id < ODBC3SQLFunctionId::LAST);
        assert(pODBC3SQLFunctions[static_cast<size_t>(id)]);
        using Func_t = SQLRETURN(SQL_API*)(Args...);
        return (*reinterpret_cast<Func_t>(pODBC3SQLFunctions[static_cast<size_t>(id)]))(args...);
    }

    SQLHANDLE m_pDriverHandle = SQL_NULL_HANDLE;
};

// ODBC Environment (common for all Connections):
SQLHANDLE ORealOdbcDriver::EnvironmentHandle()
{
    // Is (for this instance) already an Environment made?
    if (m_pDriverHandle == SQL_NULL_HANDLE)
    {
        OUString aPath;
        SQLHANDLE h = SQL_NULL_HANDLE;
        // allocate Environment
        // load ODBC-DLL now:
        if (!LoadLibrary_ODBC3(aPath)
            || AllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &h) != SQL_SUCCESS)
            dbtools::throwSQLException(aPath, OUString(), *this, 1000);

        // Save in global Structure
        m_pDriverHandle = h;
        SetEnvAttr(h, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),
                   SQL_IS_UINTEGER);
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_odbc_ORealOdbcDriver_get_implementation(css::uno::XComponentContext* context,
                                                     css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORealOdbcDriver(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
