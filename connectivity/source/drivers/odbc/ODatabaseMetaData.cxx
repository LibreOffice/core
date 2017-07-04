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

#include "odbc/ODatabaseMetaData.hxx"
#include "odbc/OTools.hxx"
#include "odbc/ODatabaseMetaDataResultSet.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <connectivity/odbc.hxx>
#include "TPrivilegesResultSet.hxx"
#include <connectivity/dbexception.hxx>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(const SQLHANDLE _pHandle,OConnection* _pCon)
                        : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_aConnectionHandle(_pHandle)
                        ,m_pConnection(_pCon)
                        ,m_bUseCatalog(true)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
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
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openTypeInfo();
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    }

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  )
{
    Reference< XResultSet > xRef;
    if(!m_bUseCatalog)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eCatalogs);
    }
    else
    {
        try
        {
            ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
            xRef = pResult;
            pResult->openCatalogs();
        }
        catch(SQLException&)
        {
            xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eCatalogs);
        }
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openSchemas();
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eSchemas);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openColumnPrivileges(m_bUseCatalog ? catalog : Any(),schema,table,columnNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumnPrivileges);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openColumns(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern,columnNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openTables(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern,types);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTables);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openProcedureColumns(m_bUseCatalog ? catalog : Any(),schemaPattern,procedureNamePattern,columnNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eProcedureColumns);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openProcedures(m_bUseCatalog ? catalog : Any(),schemaPattern,procedureNamePattern);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eProcedures);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    bool bSuccess = false;
    try
    {
        if ( !m_pConnection->preventGetVersionColumns() )
        {
            ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_BINARY_LITERAL_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_ROW_SIZE,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_CATALOG_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_CHAR_LITERAL_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMN_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMNS_IN_INDEX,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_CURSOR_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_DRIVER_CONNECTIONS/*SQL_ACTIVE_CONNECTIONS*/,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMNS_IN_TABLE,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_STATEMENT_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  )
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openExportedKeys(m_bUseCatalog ? catalog : Any(),schema,table);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eExportedKeys);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openImportedKeys(m_bUseCatalog ? catalog : Any(),schema,table);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eImportedKeys);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openPrimaryKeys(m_bUseCatalog ? catalog : Any(),schema,table);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::ePrimaryKeys);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    sal_Bool unique, sal_Bool approximate )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openIndexInfo(m_bUseCatalog ? catalog : Any(),schema,table,unique,approximate);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eIndexInfo);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope,
    sal_Bool nullable )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openBestRowIdentifier(m_bUseCatalog ? catalog : Any(),schema,table,scope,nullable);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eBestRowIdentifier);
    }
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern )
{
    if ( m_pConnection->isIgnoreDriverPrivilegesEnabled() )
    {
        return new OResultSetPrivileges(this,catalog,schemaPattern,tableNamePattern);
    }
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
    Reference< XResultSet > xRef = pResult;
    pResult->openTablePrivileges(m_bUseCatalog ? catalog : Any(),schemaPattern,tableNamePattern);
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const OUString& primarySchema,
    const OUString& primaryTable, const Any& foreignCatalog,
    const OUString& foreignSchema, const OUString& foreignTable )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
        xRef = pResult;
        pResult->openForeignKeys(m_bUseCatalog ? primaryCatalog : Any(),primarySchema.toChar() == '%' ? &primarySchema : nullptr,&primaryTable,
            m_bUseCatalog ? foreignCatalog : Any(), foreignSchema.toChar() == '%' ? &foreignSchema : nullptr,&foreignTable);
    }
    catch(SQLException&)
    {
        xRef = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eCrossReference);
    }
    return xRef;
}

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    OUString aVal;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_ROW_SIZE_INCLUDES_LONG,aVal,*this,m_pConnection->getTextEncoding());
    return aVal.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_LOWER;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_LOWER;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_UPPER;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_UPPER;
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_INDEX_SIZE,nValue,*this);
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NON_NULLABLE_COLUMNS,nValue,*this);
    return nValue == SQL_NNC_NON_NULL;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  )
{
    OUString aVal;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_TERM,aVal,*this,m_pConnection->getTextEncoding());
    return aVal;
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    OUString aVal;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_QUOTE_CHAR,aVal,*this,m_pConnection->getTextEncoding());
    return aVal;
}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    OUString aVal;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SPECIAL_CHARACTERS,aVal,*this,m_pConnection->getTextEncoding());
    return aVal;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
    return nValue != SQL_CN_NONE;
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    SQLUSMALLINT nValue=0;
    if ( m_bUseCatalog )
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_LOCATION,nValue,*this);
    return nValue == SQL_CL_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DDL_IGNORE;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DDL_COMMIT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_DML;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue == SQL_TC_ALL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POS_DELETE) == SQL_CA1_POS_DELETE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POS_UPDATE) == SQL_CA1_POS_UPDATE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_ROLLBACK_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE || nValue == SQL_CB_CLOSE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_COMMIT_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE || nValue == SQL_CB_CLOSE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_COMMIT_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_ROLLBACK_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_ISOLATION_OPTION,nValue,*this);
    return (nValue & static_cast<SQLUINTEGER>(level)) == static_cast<SQLUINTEGER>(level);
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_DML_STATEMENTS) == SQL_SU_DML_STATEMENTS;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return static_cast<bool>(nValue & SQL_SC_SQL92_FULL);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return static_cast<bool>(nValue &SQL_SC_SQL92_ENTRY);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    OUString aStr;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_INTEGRITY,aStr,*this,m_pConnection->getTextEncoding());
    return aStr.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_OJ_CAPABILITIES,nValue,*this);
    return ((nValue & (SQL_OJ_FULL|SQL_OJ_LEFT|SQL_OJ_RIGHT|SQL_OJ_NESTED|SQL_OJ_NOT_ORDERED|SQL_OJ_ALL_COMPARISON_OPS|SQL_OJ_INNER)) != 0);
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    Reference< XResultSet > xRef;
    try
    {
        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(m_pConnection);
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_PROCEDURE_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_SCHEMA_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TXN_CAPABLE,nValue,*this);
    return nValue != SQL_TC_NONE;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ACCESSIBLE_PROCEDURES,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_PROCEDURES,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nValue,*this);
    return (nValue & SQL_CA1_POSITIONED_UPDATE) == SQL_CA1_POSITIONED_UPDATE;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ACCESSIBLE_TABLES,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    return m_pConnection->isReadOnly();
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_CATALOG;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_FILE_USAGE,nValue,*this);
    return nValue == SQL_FILE_TABLE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_FUNCTIONS,nValue,*this);
    return (nValue & SQL_FN_CVT_CONVERT) == SQL_FN_CVT_CONVERT;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONCAT_NULL_BEHAVIOR,nValue,*this);
    return nValue == SQL_CB_NULL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_COLUMN_ALIAS,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
    return nValue != SQL_CN_NONE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType )
{
    if(fromType == toType)
        return true;

    SQLUINTEGER nValue=0;
    switch(fromType)
    {
        case DataType::BIT:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_BIT,nValue,*this);
            break;
        case DataType::TINYINT:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_TINYINT,nValue,*this);
            break;
        case DataType::SMALLINT:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_SMALLINT,nValue,*this);
            break;
        case DataType::INTEGER:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_INTEGER,nValue,*this);
            break;
        case DataType::BIGINT:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_BIGINT,nValue,*this);
            break;
        case DataType::FLOAT:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_FLOAT,nValue,*this);
            break;
        case DataType::REAL:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_REAL,nValue,*this);
            break;
        case DataType::DOUBLE:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_DOUBLE,nValue,*this);
            break;
        case DataType::NUMERIC:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_NUMERIC,nValue,*this);
            break;
        case DataType::DECIMAL:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_DECIMAL,nValue,*this);
            break;
        case DataType::CHAR:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_CHAR,nValue,*this);
            break;
        case DataType::VARCHAR:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_VARCHAR,nValue,*this);
            break;
        case DataType::LONGVARCHAR:
        case DataType::CLOB:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_LONGVARCHAR,nValue,*this);
            break;
        case DataType::DATE:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_DATE,nValue,*this);
            break;
        case DataType::TIME:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_TIME,nValue,*this);
            break;
        case DataType::TIMESTAMP:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_TIMESTAMP,nValue,*this);
            break;
        case DataType::BINARY:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_BINARY,nValue,*this);
            break;
        case DataType::VARBINARY:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_VARBINARY,nValue,*this);
            break;
        case DataType::LONGVARBINARY:
        case DataType::BLOB:
            OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CONVERT_LONGVARBINARY,nValue,*this);
            break;
        case DataType::SQLNULL:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::OTHER:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::OBJECT:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::DISTINCT:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::STRUCT:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::ARRAY:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
        case DataType::REF:
            //  OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CORRELATION_NAME,nValue,*this);
            break;
    }
    bool bConvert = false;
    switch(toType)
    {
        case DataType::BIT:
            bConvert = (nValue & SQL_CVT_BIT) == SQL_CVT_BIT;
            break;
        case DataType::TINYINT:
            bConvert = (nValue & SQL_CVT_TINYINT) == SQL_CVT_TINYINT;
            break;
        case DataType::SMALLINT:
            bConvert = (nValue & SQL_CVT_SMALLINT) == SQL_CVT_SMALLINT;
            break;
        case DataType::INTEGER:
            bConvert = (nValue & SQL_CVT_INTEGER) == SQL_CVT_INTEGER;
            break;
        case DataType::BIGINT:
            bConvert = (nValue & SQL_CVT_BIGINT) == SQL_CVT_BIGINT;
            break;
        case DataType::FLOAT:
            bConvert = (nValue & SQL_CVT_FLOAT) == SQL_CVT_FLOAT;
            break;
        case DataType::REAL:
            bConvert = (nValue & SQL_CVT_REAL) == SQL_CVT_REAL;
            break;
        case DataType::DOUBLE:
            bConvert = (nValue & SQL_CVT_DOUBLE) == SQL_CVT_DOUBLE;
            break;
        case DataType::NUMERIC:
            bConvert = (nValue & SQL_CVT_NUMERIC) == SQL_CVT_NUMERIC;
            break;
        case DataType::DECIMAL:
            bConvert = (nValue & SQL_CVT_DECIMAL) == SQL_CVT_DECIMAL;
            break;
        case DataType::CHAR:
            bConvert = (nValue & SQL_CVT_CHAR) == SQL_CVT_CHAR;
            break;
        case DataType::VARCHAR:
            bConvert = (nValue & SQL_CVT_VARCHAR) == SQL_CVT_VARCHAR;
            break;
        case DataType::LONGVARCHAR:
        case DataType::CLOB:
            bConvert = (nValue & SQL_CVT_LONGVARCHAR) == SQL_CVT_LONGVARCHAR;
            break;
        case DataType::DATE:
            bConvert = (nValue & SQL_CVT_DATE) == SQL_CVT_DATE;
            break;
        case DataType::TIME:
            bConvert = (nValue & SQL_CVT_TIME) == SQL_CVT_TIME;
            break;
        case DataType::TIMESTAMP:
            bConvert = (nValue & SQL_CVT_TIMESTAMP) == SQL_CVT_TIMESTAMP;
            break;
        case DataType::BINARY:
            bConvert = (nValue & SQL_CVT_BINARY) == SQL_CVT_BINARY;
            break;
        case DataType::VARBINARY:
            bConvert = (nValue & SQL_CVT_VARBINARY) == SQL_CVT_VARBINARY;
            break;
        case DataType::LONGVARBINARY:
        case DataType::BLOB:
            bConvert = (nValue & SQL_CVT_LONGVARBINARY) == SQL_CVT_LONGVARBINARY;
            break;
    }

    return bConvert;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_EXPRESSIONS_IN_ORDERBY,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue != SQL_GB_NOT_SUPPORTED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue != SQL_GB_GROUP_BY_CONTAINS_SELECT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_GROUP_BY,nValue,*this);
    return nValue == SQL_GB_NO_RELATION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MULTIPLE_ACTIVE_TXN,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MULT_RESULT_SETS,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_LIKE_ESCAPE_CLAUSE,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'Y';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ORDER_BY_COLUMNS_IN_SELECT,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.toChar() == 'N';
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_UNION,nValue,*this);
    return (nValue & SQL_U_UNION) == SQL_U_UNION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_UNION,nValue,*this);
    return (nValue & SQL_U_UNION_ALL) == SQL_U_UNION_ALL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_QUOTED_IDENTIFIER_CASE,nValue,*this);
    return nValue == SQL_IC_MIXED;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_END;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_HIGH;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NULL_COLLATION,nValue,*this);
    return nValue == SQL_NC_LOW;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_PROCEDURE_INVOCATION) == SQL_SU_PROCEDURE_INVOCATION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_USAGE,nValue,*this);
    return (nValue & SQL_SU_PRIVILEGE_DEFINITION) == SQL_SU_PRIVILEGE_DEFINITION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_PROCEDURE_INVOCATION) == SQL_CU_PROCEDURE_INVOCATION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    SQLUINTEGER nValue=0;
    if(m_bUseCatalog)
        OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CATALOG_USAGE,nValue,*this);
    return (nValue & SQL_CU_PRIVILEGE_DEFINITION) == SQL_CU_PRIVILEGE_DEFINITION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_CORRELATED_SUBQUERIES) == SQL_SQ_CORRELATED_SUBQUERIES;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_COMPARISON) == SQL_SQ_COMPARISON;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_EXISTS) == SQL_SQ_EXISTS;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_IN) == SQL_SQ_IN;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SUBQUERIES,nValue,*this);
    return (nValue & SQL_SQ_QUANTIFIED) == SQL_SQ_QUANTIFIED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SQL_CONFORMANCE,nValue,*this);
    return static_cast<bool>(nValue & SQL_SC_SQL92_INTERMEDIATE);
}

OUString ODatabaseMetaData::getURLImpl()
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DATA_SOURCE_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getURL(  )
{
    OUString aValue = m_pConnection->getURL();
    if ( aValue.isEmpty() )
    {
        aValue = "sdbc:odbc:" + getURLImpl();
    }
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_USER_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DRIVER_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DRIVER_ODBC_VER,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DBMS_NAME,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_PROCEDURE_TERM,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SCHEMA_TERM,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) try
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.copy(0,aValue.indexOf('.')).toInt32();
}
catch (const SQLException &)
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  )
{
    SQLUINTEGER nValue;
    sal_Int32 nValueTranslated;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DEFAULT_TXN_ISOLATION,nValue,*this);
    switch(nValue)
    {
        case SQL_TXN_READ_UNCOMMITTED:
            nValueTranslated = css::sdbc::TransactionIsolation::READ_UNCOMMITTED;
            break;
        case SQL_TXN_READ_COMMITTED:
            nValueTranslated = css::sdbc::TransactionIsolation::READ_COMMITTED;
            break;
        case SQL_TXN_REPEATABLE_READ:
            nValueTranslated = css::sdbc::TransactionIsolation::REPEATABLE_READ;
            break;
        case  SQL_TXN_SERIALIZABLE:
            nValueTranslated = css::sdbc::TransactionIsolation::SERIALIZABLE;
            break;
        default:
            nValueTranslated = 0;
    }
    return nValueTranslated;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) try
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_DRIVER_VER,aValue,*this,m_pConnection->getTextEncoding());
    return aValue.copy(0,aValue.lastIndexOf('.')).toInt32();
}
catch (const SQLException &)
{
    return 0;
}

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_KEYWORDS,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  )
{
    OUString aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SEARCH_PATTERN_ESCAPE,aValue,*this,m_pConnection->getTextEncoding());
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  )
{
    SQLUINTEGER nValue;
    OUStringBuffer aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_STRING_FUNCTIONS,nValue,*this);
    if(nValue & SQL_FN_STR_ASCII)
        aValue.append("ASCII,");
    if(nValue & SQL_FN_STR_BIT_LENGTH)
        aValue.append("BIT_LENGTH,");
    if(nValue & SQL_FN_STR_CHAR)
        aValue.append("CHAR,");
    if(nValue & SQL_FN_STR_CHAR_LENGTH)
        aValue.append("CHAR_LENGTH,");
    if(nValue & SQL_FN_STR_CHARACTER_LENGTH)
        aValue.append("CHARACTER_LENGTH,");
    if(nValue & SQL_FN_STR_CONCAT)
        aValue.append("CONCAT,");
    if(nValue & SQL_FN_STR_DIFFERENCE)
        aValue.append("DIFFERENCE,");
    if(nValue & SQL_FN_STR_INSERT)
        aValue.append("INSERT,");
    if(nValue & SQL_FN_STR_LCASE)
        aValue.append("LCASE,");
    if(nValue & SQL_FN_STR_LEFT)
        aValue.append("LEFT,");
    if(nValue & SQL_FN_STR_LENGTH)
        aValue.append("LENGTH,");
    if(nValue & SQL_FN_STR_LOCATE)
        aValue.append("LOCATE,");
    if(nValue & SQL_FN_STR_LOCATE_2)
        aValue.append("LOCATE_2,");
    if(nValue & SQL_FN_STR_LTRIM)
        aValue.append("LTRIM,");
    if(nValue & SQL_FN_STR_OCTET_LENGTH)
        aValue.append("OCTET_LENGTH,");
    if(nValue & SQL_FN_STR_POSITION)
        aValue.append("POSITION,");
    if(nValue & SQL_FN_STR_REPEAT)
        aValue.append("REPEAT,");
    if(nValue & SQL_FN_STR_REPLACE)
        aValue.append("REPLACE,");
    if(nValue & SQL_FN_STR_RIGHT)
        aValue.append("RIGHT,");
    if(nValue & SQL_FN_STR_RTRIM)
        aValue.append("RTRIM,");
    if(nValue & SQL_FN_STR_SOUNDEX)
        aValue.append("SOUNDEX,");
    if(nValue & SQL_FN_STR_SPACE)
        aValue.append("SPACE,");
    if(nValue & SQL_FN_STR_SUBSTRING)
        aValue.append("SUBSTRING,");
    if(nValue & SQL_FN_STR_UCASE)
        aValue.append("UCASE,");


    if ( !aValue.isEmpty() )
        aValue.setLength(aValue.getLength()-1);

    return aValue.makeStringAndClear();
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  )
{
    SQLUINTEGER nValue;
    OUStringBuffer aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_TIMEDATE_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_TD_CURRENT_DATE)
        aValue.append("CURRENT_DATE,");
    if(nValue & SQL_FN_TD_CURRENT_TIME)
        aValue.append("CURRENT_TIME,");
    if(nValue & SQL_FN_TD_CURRENT_TIMESTAMP)
        aValue.append("CURRENT_TIMESTAMP,");
    if(nValue & SQL_FN_TD_CURDATE)
        aValue.append("CURDATE,");
    if(nValue & SQL_FN_TD_CURTIME)
        aValue.append("CURTIME,");
    if(nValue & SQL_FN_TD_DAYNAME)
        aValue.append("DAYNAME,");
    if(nValue & SQL_FN_TD_DAYOFMONTH)
        aValue.append("DAYOFMONTH,");
    if(nValue & SQL_FN_TD_DAYOFWEEK)
        aValue.append("DAYOFWEEK,");
    if(nValue & SQL_FN_TD_DAYOFYEAR)
        aValue.append("DAYOFYEAR,");
    if(nValue & SQL_FN_TD_EXTRACT)
        aValue.append("EXTRACT,");
    if(nValue & SQL_FN_TD_HOUR)
        aValue.append("HOUR,");
    if(nValue & SQL_FN_TD_MINUTE)
        aValue.append("MINUTE,");
    if(nValue & SQL_FN_TD_MONTH)
        aValue.append("MONTH,");
    if(nValue & SQL_FN_TD_MONTHNAME)
        aValue.append("MONTHNAME,");
    if(nValue & SQL_FN_TD_NOW)
        aValue.append("NOW,");
    if(nValue & SQL_FN_TD_QUARTER)
        aValue.append("QUARTER,");
    if(nValue & SQL_FN_TD_SECOND)
        aValue.append("SECOND,");
    if(nValue & SQL_FN_TD_TIMESTAMPADD)
        aValue.append("TIMESTAMPADD,");
    if(nValue & SQL_FN_TD_TIMESTAMPDIFF)
        aValue.append("TIMESTAMPDIFF,");
    if(nValue & SQL_FN_TD_WEEK)
        aValue.append("WEEK,");
    if(nValue & SQL_FN_TD_YEAR)
        aValue.append("YEAR,");

    if ( !aValue.isEmpty() )
        aValue.setLength(aValue.getLength()-1);

    return aValue.makeStringAndClear();
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  )
{
    SQLUINTEGER nValue;
    OUStringBuffer aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_SYSTEM_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_SYS_DBNAME)
        aValue.append("DBNAME,");
    if(nValue & SQL_FN_SYS_IFNULL)
        aValue.append("IFNULL,");
    if(nValue & SQL_FN_SYS_USERNAME)
        aValue.append("USERNAME,");

    if ( !aValue.isEmpty() )
        aValue.setLength(aValue.getLength()-1);

    return aValue.makeStringAndClear();
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  )
{
    SQLUINTEGER nValue;
    OUStringBuffer aValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_NUMERIC_FUNCTIONS,nValue,*this);

    if(nValue & SQL_FN_NUM_ABS)
        aValue.append("ABS,");
    if(nValue & SQL_FN_NUM_ACOS)
        aValue.append("ACOS,");
    if(nValue & SQL_FN_NUM_ASIN)
        aValue.append("ASIN,");
    if(nValue & SQL_FN_NUM_ATAN)
        aValue.append("ATAN,");
    if(nValue & SQL_FN_NUM_ATAN2)
        aValue.append("ATAN2,");
    if(nValue & SQL_FN_NUM_CEILING)
        aValue.append("CEILING,");
    if(nValue & SQL_FN_NUM_COS)
        aValue.append("COS,");
    if(nValue & SQL_FN_NUM_COT)
        aValue.append("COT,");
    if(nValue & SQL_FN_NUM_DEGREES)
        aValue.append("DEGREES,");
    if(nValue & SQL_FN_NUM_EXP)
        aValue.append("EXP,");
    if(nValue & SQL_FN_NUM_FLOOR)
        aValue.append("FLOOR,");
    if(nValue & SQL_FN_NUM_LOG)
        aValue.append("LOGF,");
    if(nValue & SQL_FN_NUM_LOG10)
        aValue.append("LOG10,");
    if(nValue & SQL_FN_NUM_MOD)
        aValue.append("MOD,");
    if(nValue & SQL_FN_NUM_PI)
        aValue.append("PI,");
    if(nValue & SQL_FN_NUM_POWER)
        aValue.append("POWER,");
    if(nValue & SQL_FN_NUM_RADIANS)
        aValue.append("RADIANS,");
    if(nValue & SQL_FN_NUM_RAND)
        aValue.append("RAND,");
    if(nValue & SQL_FN_NUM_ROUND)
        aValue.append("ROUND,");
    if(nValue & SQL_FN_NUM_SIGN)
        aValue.append("SIGN,");
    if(nValue & SQL_FN_NUM_SIN)
        aValue.append("SIN,");
    if(nValue & SQL_FN_NUM_SQRT)
        aValue.append("SQRT,");
    if(nValue & SQL_FN_NUM_TAN)
        aValue.append("TAN,");
    if(nValue & SQL_FN_NUM_TRUNCATE)
        aValue.append("TRUNCATE,");

    if ( !aValue.isEmpty() )
        aValue.setLength(aValue.getLength()-1);

    return aValue.makeStringAndClear();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    SQLUINTEGER nValue;
    // SQL_ODBC_SQL_CONFORMANCE is deprecated in ODBC 3.x, but there does not seem te be any equivalent.
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ODBC_SQL_CONFORMANCE,nValue,*this);
    SAL_WARN_IF(! (nValue == SQL_OSC_MINIMUM || nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED),
                "connectivity.odbc",
                "SQL_ODBC_SQL_CONFORMANCE is neither MINIMAL nor CORE nor EXTENDED");
    return nValue == SQL_OSC_EXTENDED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    SQLUINTEGER nValue;
    // SQL_ODBC_SQL_CONFORMANCE is deprecated in ODBC 3.x, but there does not seem te be any equivalent.
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ODBC_SQL_CONFORMANCE,nValue,*this);
    SAL_WARN_IF(! (nValue == SQL_OSC_MINIMUM || nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED),
                "connectivity.odbc",
                "SQL_ODBC_SQL_CONFORMANCE is neither MINIMAL nor CORE nor EXTENDED");
    return nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    SQLUINTEGER nValue;
    // SQL_ODBC_SQL_CONFORMANCE is deprecated in ODBC 3.x, but there does not seem te be any equivalent.
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_ODBC_SQL_CONFORMANCE,nValue,*this);
    SAL_WARN_IF(! (nValue == SQL_OSC_MINIMUM || nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED),
                "connectivity.odbc",
                "SQL_ODBC_SQL_CONFORMANCE is neither MINIMAL nor CORE nor EXTENDED");
    return nValue == SQL_OSC_MINIMUM || nValue == SQL_OSC_CORE || nValue == SQL_OSC_EXTENDED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_OJ_CAPABILITIES,nValue,*this);
    return (nValue & SQL_OJ_FULL) == SQL_OJ_FULL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return supportsFullOuterJoins(  );
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMNS_IN_GROUP_BY,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMNS_IN_ORDER_BY,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_COLUMNS_IN_SELECT,nValue,*this);
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  )
{
    SQLUSMALLINT nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_MAX_USER_NAME_LEN,nValue,*this);
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    SQLUINTEGER nValue;
    OTools::GetInfo(m_pConnection,m_aConnectionHandle,SQL_CURSOR_SENSITIVITY,nValue,*this);
    return (nValue & static_cast<SQLUINTEGER>(setType)) == static_cast<SQLUINTEGER>(setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency )
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

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType )
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
    return (nValue & SQL_CA2_SENSITIVITY_UPDATES) == SQL_CA2_SENSITIVITY_UPDATES;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType )
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
    return (nValue & SQL_CA2_SENSITIVITY_DELETIONS) != SQL_CA2_SENSITIVITY_DELETIONS;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType )
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
    return (nValue & SQL_CA2_SENSITIVITY_ADDITIONS) == SQL_CA2_SENSITIVITY_ADDITIONS;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType )
{
    return ownUpdatesAreVisible(setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType )
{
    return ownDeletesAreVisible(setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType )
{
    return ownInsertsAreVisible(setType);
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
