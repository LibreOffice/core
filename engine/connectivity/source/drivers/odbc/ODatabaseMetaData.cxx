/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <odbc/ODatabaseMetaData.hxx>
#include <odbc/OTools.hxx>
#include <odbc/ODatabaseMetaDataResultSet.hxx>
#include <FDatabaseMetaDataResultSet.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <TPrivilegesResultSet.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

using namespace connectivity::odbc;
using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(const SQLHANDLE _pHandle,OConnection* _pCon)
                        : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_aConnectionHandle(_pHandle)
                        ,m_pConnection(_pCon)
                        ,m_bUseCatalog(true)
{
    assert(m_pConnection && "ODatabaseMetaData::ODatabaseMetaData: No connection set!");
    if(!m_pConnection->isCatalogUsed())
    {
        osl_atomic_increment( &m_refCount );
        try
        {
            m_bUseCatalog   = !(usesLocalFiles() || usesLocalFilePerTable());
        }
        catch(SQLException& )
        { // doesn't matter here
        }
        osl_atomic_decrement( &m_refCount );
    }
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openTypeInfo();
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    }

    return xRef;
}

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    OUString aVal;
    if ( m_bUseCatalog )
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_NAME_SEPARATOR,aVal,*this,m_pConnection->getTextEncoding());

    return aVal;
}

Reference< XResultSet > ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openColumnPrivileges(m_bUseCatalog ? catalog : Any(),schema,table,columnNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumnPrivileges);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getColumns(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openColumns(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern,columnNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getTables(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openTables(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern,types);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTables);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    bool bSuccess = false;
    try
    {
        if ( !m_pConnection->preventGetVersionColumns() )
        {
            rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
            xRef = pResult;
            pResult->openVersionColumns(m_bUseCatalog ? catalog : Any(),schema,table);
            bSuccess = true;
        }
    }
    catch(SQLException&)
    {
    }

    if ( !bSuccess )
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eVersionColumns);
    }

    return xRef;
}



sal_Int32 ODatabaseMetaData::getMaxColumnNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMN_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 ODatabaseMetaData::getMaxTableNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_TABLE_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_TABLES_IN_SELECT,nValue,*this);
    return nValue;
}

Reference< XResultSet > ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openImportedKeys(m_bUseCatalog ? catalog : Any(),schema,table);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eImportedKeys);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openPrimaryKeys(m_bUseCatalog ? catalog : Any(),schema,table);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::ePrimaryKeys);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    bool unique, bool approximate )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openIndexInfo(m_bUseCatalog ? catalog : Any(),schema,table,unique,approximate);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eIndexInfo);
    }
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern )
{
    if ( m_pConnection->isIgnoreDriverPrivilegesEnabled() )
    {
        return new OResultSetPrivileges(this,catalog,schemaPattern,tableNamePattern);
    }
    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
    pResult->openTablePrivileges(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern);
    return pResult;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ALTER_TABLE,nValue,*this);
    return (nValue & SQL_AT_ADD_COLUMN) == SQL_AT_ADD_COLUMN;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ALTER_TABLE,nValue,*this);
    return  ((nValue & SQL_AT_DROP_COLUMN)          == SQL_AT_DROP_COLUMN)          ||
            ((nValue & SQL_AT_DROP_COLUMN_CASCADE)  == SQL_AT_DROP_COLUMN_CASCADE)  ||
            ((nValue & SQL_AT_DROP_COLUMN_RESTRICT) == SQL_AT_DROP_COLUMN_RESTRICT);
}

bool ODatabaseMetaData::supportsNonNullableColumns(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NON_NULLABLE_COLUMNS,nValue,*this);
    return nValue == SQL_NNC_NON_NULL;
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    OUString aVal;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_QUOTE_CHAR,aVal,*this,m_pConnection->getTextEncoding());
    return aVal;
}

OUString ODatabaseMetaData::getExtraNameCharacters(  )
{
    OUString aVal;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SPECIAL_CHARACTERS,aVal,*this,m_pConnection->getTextEncoding());
    return aVal;
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    SQLUSMALLINT nValue=0;
    if ( m_bUseCatalog )
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_LOCATION,nValue,*this);
    return nValue == SQL_CL_START;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_DML_STATEMENTS) == SQL_SU_DML_STATEMENTS;
}

bool ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return static_cast<bool>(nValue &SQL_SC_SQL92_ENTRY);
}

bool ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    OUString aStr;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_INTEGRITY,aStr,*this,m_pConnection->getTextEncoding());
    return aStr.toChar() == 'Y';
}

bool ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_INDEX_DEFINITION) == SQL_SU_INDEX_DEFINITION;
}

bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_TABLE_DEFINITION) == SQL_SU_TABLE_DEFINITION;
}

bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_TABLE_DEFINITION) == SQL_CU_TABLE_DEFINITION;
}

bool ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_INDEX_DEFINITION) == SQL_CU_INDEX_DEFINITION;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_DML_STATEMENTS) == SQL_CU_DML_STATEMENTS;
}

Reference< XResultSet > ODatabaseMetaData::getTableTypes(  )
{
    Reference< XResultSet > xRef;
    try
    {
        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openTablesTypes();
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
    }
    return xRef;
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_CONCURRENT_ACTIVITIES,nValue,*this);
    return nValue;
}

bool ODatabaseMetaData::isReadOnly(  )
{
    return m_pConnection->isReadOnly();
}

bool ODatabaseMetaData::usesLocalFiles(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_CATALOG;
}

bool ODatabaseMetaData::usesLocalFilePerTable(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_TABLE;
}

bool ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue == SQL_GB_NO_RELATION;
}

bool ODatabaseMetaData::supportsMultipleResultSets(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MULT_RESULT_SETS,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

bool ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ORDER_BY_COLUMNS_IN_SELECT,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'N';
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

bool ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_PROCEDURE_INVOCATION) == SQL_SU_PROCEDURE_INVOCATION;
}

bool ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_PRIVILEGE_DEFINITION) == SQL_SU_PRIVILEGE_DEFINITION;
}

bool ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_PROCEDURE_INVOCATION) == SQL_CU_PROCEDURE_INVOCATION;
}

bool ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_PRIVILEGE_DEFINITION) == SQL_CU_PRIVILEGE_DEFINITION;
}

OUString ODatabaseMetaData::getURLImpl()
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DATA_SOURCE_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString ODatabaseMetaData::getURL(  )
{
    OUString aValue = m_pConnection->getURL();
    if ( aValue.isEmpty() )
    {
        aValue = "sdbc:odbc:" + getURLImpl();
    }
    return aValue;
}

OUString ODatabaseMetaData::getUserName(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_USER_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString ODatabaseMetaData::getDatabaseProductName(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DBMS_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

bool ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    SQLUINTEGER nValue;
    // SQL_ODBC_SQL_CONFORMANCE is deprecated in ODBC 3.x, but there does not seem te be any equivalent.
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ODBC_SQL_CONFORMANCE,nValue,*this);
    SAL_WARN_IF(! (nValue == SQL_OSC_MINIMUM || nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED),
                "connectivity.odbc",
                "SQL_ODBC_SQL_CONFORMANCE is neither MINIMAL nor CORE nor EXTENDED");
    return nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED;
}

bool ODatabaseMetaData::supportsFullOuterJoins(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_OJ_CAPABILITIES,nValue,*this);
    return (nValue & SQL_OJ_FULL) == SQL_OJ_FULL;
}

bool ODatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_SENSITIVITY,nValue,*this);
    return (nValue & static_cast<SQLUINTEGER>(setType)) == static_cast<SQLUINTEGER>(setType);
}

bool ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency )
{
    SQLUINTEGER nValue;
    SQLUSMALLINT nAskFor( SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2 );
    switch(setType)
    {
        default:
        case ResultSetType::FORWARD_ONLY:
            nAskFor = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_INSENSITIVE:
            nAskFor = SQL_STATIC_CURSOR_ATTRIBUTES2;
            break;
        case ResultSetType::SCROLL_SENSITIVE:
            nAskFor = SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
            break;
    }

    OTools::GetInfo(m_pConnection,m_aConnectionHandle,nAskFor,nValue,*this);
    bool bRet = false;
    switch(concurrency)
    {
        case ResultSetConcurrency::READ_ONLY:
            bRet = (nValue & SQL_CA2_READ_ONLY_CONCURRENCY) == SQL_CA2_READ_ONLY_CONCURRENCY;
            break;
        case ResultSetConcurrency::UPDATABLE:
            bRet = (nValue & SQL_CA2_OPT_VALUES_CONCURRENCY) == SQL_CA2_OPT_VALUES_CONCURRENCY;
            break;
    }
    return bRet;
}

bool ODatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
