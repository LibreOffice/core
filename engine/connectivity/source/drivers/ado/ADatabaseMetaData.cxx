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

#include <ado/ADatabaseMetaData.hxx>
#include <ado/ADatabaseMetaDataResultSet.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <ado/AConnection.hxx>
#include <ado/adoimp.hxx>
#include <FDatabaseMetaDataResultSet.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <rtl/ref.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
    : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
    ,m_rADOConnection(_pCon->getConnection())
    ,m_pConnection(_pCon)
{
}

sal_Int32 ODatabaseMetaData::getInt32Property(const OUString& _aProperty)
{
    connectivity::ado::WpADOProperties aProps(m_rADOConnection.get_Properties());
    //  ADOS::ThrowException(m_rADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    sal_Int32 nValue(0);
    if(!aVar.isNull() && !aVar.isEmpty())
        nValue = aVar.getInt32();
    return nValue;
}


bool ODatabaseMetaData::getBoolProperty(const OUString& _aProperty)
{
    connectivity::ado::WpADOProperties aProps(m_rADOConnection.get_Properties());
    ADOS::ThrowException(m_rADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    return !aVar.isNull() && !aVar.isEmpty() && aVar.getBool();
}

OUString ODatabaseMetaData::getStringProperty(const OUString& _aProperty)
{
    connectivity::ado::WpADOProperties aProps(m_rADOConnection.get_Properties());
    ADOS::ThrowException(m_rADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");

    ADO_PROP(_aProperty);
    OUString aValue;
    if(!aVar.isNull() && !aVar.isEmpty() && aVar.getType() == VT_BSTR)
        aValue = aVar.getString();

    return aValue;
}

Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    ADORecordset *pRecordset = m_rADOConnection.getTypeInfo();

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTypeInfoMap(ADOS::isJetEngine(m_pConnection->getEngineType()));
    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getCatalogs(  )
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = nullptr;
    m_rADOConnection.OpenSchema(adSchemaCatalogs,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCatalogsMap();

    return pResult;
}

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return getLiteral(DBLITERAL_CATALOG_SEPARATOR);
}

Reference< XResultSet > ODatabaseMetaData::getSchemas(  )
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = nullptr;
    m_rADOConnection.OpenSchema(adSchemaSchemata,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setSchemasMap();
    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getColumnPrivileges(catalog,schema,table,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnPrivilegesMap();
    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getColumns(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getColumns(catalog,schemaPattern,tableNamePattern,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnsMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getTables(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    ADORecordset *pRecordset = m_rADOConnection.getTables(catalog,schemaPattern,tableNamePattern,types);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTablesMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getProcedureColumns(catalog,schemaPattern,procedureNamePattern,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProcedureColumnsMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getProcedures(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern )
{
    // Create elements used in the array
    ADORecordset *pRecordset = m_rADOConnection.getProcedures(catalog,schemaPattern,procedureNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProceduresMap();

    return pResult;
}

sal_Int32 ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return getMaxSize(DBLITERAL_BINARY_LITERAL);
}

sal_Int32 ODatabaseMetaData::getMaxRowSize(  )
{
    return getInt32Property("Maximum Row Size");
}

sal_Int32 ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    return getMaxSize(DBLITERAL_CATALOG_NAME);
}

sal_Int32 ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    return getMaxSize(DBLITERAL_CHAR_LITERAL);
}

sal_Int32 ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return getMaxSize(DBLITERAL_COLUMN_NAME);
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxCursorNameLength(  )
{
    return getMaxSize(DBLITERAL_CURSOR_NAME);
}

sal_Int32 ODatabaseMetaData::getMaxConnections(  )
{
    return getInt32Property("Active Sessions");
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInTable(  )
{
    return getInt32Property("Max Columns in Table");
}

sal_Int32 ODatabaseMetaData::getMaxStatementLength(  )
{
    return getMaxSize(DBLITERAL_TEXT_COMMAND);
}

sal_Int32 ODatabaseMetaData::getMaxTableNameLength(  )
{
    return getMaxSize(DBLITERAL_TABLE_NAME);
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return getInt32Property("Maximum Tables in SELECT");
}

Reference< XResultSet > ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getExportedKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getImportedKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getPrimaryKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setPrimaryKeysMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    bool unique, bool approximate )
{
    ADORecordset *pRecordset = m_rADOConnection.getIndexInfo(catalog,schema,table,unique,approximate);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setIndexInfoMap();

    return pResult;
}

Reference< XResultSet > ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern )
{
    Reference< XResultSet > xRef;
    if(!ADOS::isJetEngine(m_pConnection->getEngineType()))
    {   // the jet provider doesn't support this method
        // Create elements used in the array

        ADORecordset *pRecordset = m_rADOConnection.getTablePrivileges(catalog,schemaPattern,tableNamePattern);
        ADOS::ThrowException(m_rADOConnection,*this);

        rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
        pResult->setTablePrivilegesMap();
        xRef = pResult;
    }
    else
    {
        rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTablePrivileges);
        xRef = pResult;
        ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
        ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(8);
        aRows.reserve(8);

        aRow[0] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = new ::connectivity::ORowSetValueDecorator(tableNamePattern);
        aRow[3] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[4] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[5] = new ::connectivity::ORowSetValueDecorator(getUserName());
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getSelectValue();
        aRow[7] = new ::connectivity::ORowSetValueDecorator(OUString("NO"));

        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getInsertValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDeleteValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getUpdateValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getCreateValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getReadValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getAlterValue();
        aRows.push_back(aRow);
        aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDropValue();
        aRows.push_back(aRow);
        pResult->setRows(std::move(aRows));
    }

    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const OUString& primarySchema,
    const OUString& primaryTable, const Any& foreignCatalog,
    const OUString& foreignSchema, const OUString& foreignTable )
{
    ADORecordset *pRecordset = m_rADOConnection.getCrossReference(primaryCatalog,primarySchema,primaryTable,foreignCatalog,foreignSchema,foreignTable);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();

    return pResult;
}

bool ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return getBoolProperty("Maximum Row Size Includes BLOB");
}

bool ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

bool ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

bool ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

bool ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}

bool ODatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return true;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return true;
}

sal_Int32 ODatabaseMetaData::getMaxIndexLength(  )
{
    return getInt32Property("Maximum Index Size");
}

bool ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NON_NULL;
}

OUString ODatabaseMetaData::getCatalogTerm(  )
{
    return getStringProperty("Catalog Term");
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return getLiteral(DBLITERAL_QUOTE_PREFIX);

}

OUString ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

bool ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return getInt32Property("Catalog Location") == DBPROPVAL_CL_START;
}

bool ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_IGNORE;
}

bool ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_COMMIT;
}

bool ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DML;
}

bool ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_ALL;
}

bool ODatabaseMetaData::supportsPositionedDelete(  )
{
    return true;
}

bool ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return true;
}

bool ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return getInt32Property("Prepare Abort Behavior") == DBPROPVAL_CB_PRESERVE;
}

bool ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return getInt32Property("Prepare Commit Behavior") == DBPROPVAL_CB_PRESERVE;
}

bool ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_COMMIT) == DBPROPVAL_TR_COMMIT;
}

bool ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_ABORT) == DBPROPVAL_TR_ABORT;
}

bool ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level )
{
    bool bValue(false);

    sal_Int32 nTxn = getInt32Property("Isolation Levels");
    if(level == TransactionIsolation::NONE)
        bValue = true;
    else if(level == TransactionIsolation::READ_UNCOMMITTED)
        bValue = (nTxn & DBPROPVAL_TI_READUNCOMMITTED) == DBPROPVAL_TI_READUNCOMMITTED;
    else if(level == TransactionIsolation::READ_COMMITTED)
        bValue = (nTxn & DBPROPVAL_TI_READCOMMITTED) == DBPROPVAL_TI_READCOMMITTED;
    else if(level == TransactionIsolation::REPEATABLE_READ)
        bValue = (nTxn & DBPROPVAL_TI_REPEATABLEREAD) == DBPROPVAL_TI_REPEATABLEREAD;
    else if(level == TransactionIsolation::SERIALIZABLE)
        bValue = (nTxn & DBPROPVAL_TI_SERIALIZABLE) == DBPROPVAL_TI_SERIALIZABLE;

    return bValue;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_DML_STATEMENTS) == DBPROPVAL_SU_DML_STATEMENTS;
}

bool ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_FULL) == DBPROPVAL_SQL_ANSI92_FULL);
}

bool ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_ENTRY) == DBPROPVAL_SQL_ANSI92_ENTRY);
}

bool ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI89_IEF) == DBPROPVAL_SQL_ANSI89_IEF);
}

bool ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_INDEX_DEFINITION) == DBPROPVAL_SU_INDEX_DEFINITION;
}

bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_TABLE_DEFINITION) == DBPROPVAL_SU_TABLE_DEFINITION;
}

bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOuterJoins(  )
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return getBoolProperty("Outer Join Capabilities");
}

Reference< XResultSet > ODatabaseMetaData::getTableTypes(  )
{
    return new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return getMaxSize(DBLITERAL_PROCEDURE_NAME);
}

sal_Int32 ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    return getMaxSize(DBLITERAL_SCHEMA_NAME);
}

bool ODatabaseMetaData::supportsTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_NONE;
}

bool ODatabaseMetaData::allProceduresAreCallable(  )
{
    return true;
}

bool ODatabaseMetaData::supportsStoredProcedures(  )
{
    return true;
}

bool ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return true;
}

bool ODatabaseMetaData::allTablesAreSelectable(  )
{
    return true;
}

bool ODatabaseMetaData::isReadOnly(  )
{
    return getBoolProperty("Read-Only Data Source");
}

bool ODatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

bool ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

bool ODatabaseMetaData::supportsTypeConversion(  )
{
    return true;
}

bool ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NULL;
}

bool ODatabaseMetaData::supportsColumnAliasing(  )
{
    return isCapable(DBLITERAL_COLUMN_ALIAS);
}

bool ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

bool ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return getBoolProperty("Rowset Conversions on Command");
}

bool ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

bool ODatabaseMetaData::supportsGroupBy(  )
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_NOT_SUPPORTED;
}

bool ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_CONTAINS_SELECT;
}

bool ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return getInt32Property("GROUP BY Support") == DBPROPVAL_GB_NO_RELATION;
}

bool ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return true;
}

bool ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

bool ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return isCapable(DBLITERAL_ESCAPE_PERCENT);
}

bool ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

bool ODatabaseMetaData::supportsUnion(  )
{
    return true;
}

bool ODatabaseMetaData::supportsUnionAll(  )
{
    return true;
}

bool ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

bool ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_END) == DBPROPVAL_NC_END;
}

bool ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_START) == DBPROPVAL_NC_START;
}

bool ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_HIGH) == DBPROPVAL_NC_HIGH;
}

bool ODatabaseMetaData::nullsAreSortedLow(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_LOW) == DBPROPVAL_NC_LOW;
}

bool ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_PRIVILEGE_DEFINITION) == DBPROPVAL_SU_PRIVILEGE_DEFINITION;
}

bool ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_CORRELATEDSUBQUERIES) == DBPROPVAL_SQ_CORRELATEDSUBQUERIES;
}

bool ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_COMPARISON) == DBPROPVAL_SQ_COMPARISON;
}

bool ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_EXISTS) == DBPROPVAL_SQ_EXISTS;
}

bool ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_IN) == DBPROPVAL_SQ_IN;
}

bool ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_QUANTIFIED) == DBPROPVAL_SQ_QUANTIFIED;
}

bool ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_INTERMEDIATE) == DBPROPVAL_SQL_ANSI92_INTERMEDIATE);
}

OUString ODatabaseMetaData::getURL(  )
{
    return "sdbc:ado:"+ m_rADOConnection.GetConnectionString();
}

OUString ODatabaseMetaData::getUserName(  )
{
    return getStringProperty("User Name");
}

OUString ODatabaseMetaData::getDriverName(  )
{
    return getStringProperty("Provider Friendly Name");
}

OUString ODatabaseMetaData::getDriverVersion(  )
{
    return getStringProperty("Provider Version");
}

OUString ODatabaseMetaData::getDatabaseProductVersion(  )
{
    return getStringProperty("DBMS Version");
}

OUString ODatabaseMetaData::getDatabaseProductName(  )
{
    return getStringProperty("DBMS Name");
}

OUString ODatabaseMetaData::getProcedureTerm(  )
{
    return getStringProperty("Procedure Term");
}

OUString ODatabaseMetaData::getSchemaTerm(  )
{
    return getStringProperty("Schema Term");
}

sal_Int32 ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 1;
}

sal_Int32 ODatabaseMetaData::getDefaultTransactionIsolation(  )
{
    sal_Int32 nRet = TransactionIsolation::NONE;
    switch(m_rADOConnection.get_IsolationLevel())
    {
        case adXactReadCommitted:
            nRet = TransactionIsolation::READ_COMMITTED;
            break;
        case adXactRepeatableRead:
            nRet = TransactionIsolation::REPEATABLE_READ;
            break;
        case adXactSerializable:
            nRet = TransactionIsolation::SERIALIZABLE;
            break;
        case adXactReadUncommitted:
            nRet = TransactionIsolation::READ_UNCOMMITTED;
            break;
        default:
            ;
    }
    return nRet;
}

sal_Int32 ODatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

OUString ODatabaseMetaData::getSQLKeywords(  )
{
    WpADORecordset aRecordset;
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();
    m_rADOConnection.OpenSchema(adSchemaDBInfoKeywords,vtEmpty,vtEmpty,&aRecordset);
    OSL_ENSURE(aRecordset,"getSQLKeywords: no resultset!");
    ADOS::ThrowException(m_rADOConnection,*this);
    if ( aRecordset )
    {
        aRecordset.MoveFirst();
        OLEVariant  aValue;
        OUString aRet;
        while(!aRecordset.IsAtEOF())
        {
            WpOLEAppendCollection<ADOFields, WpADOField>  aFields(aRecordset.GetFields());
            WpADOField aField(aFields.GetItem(0));
            aField.get_Value(aValue);
            aRet += aValue.getString() + ",";
            aRecordset.MoveNext();
        }
        aRecordset.Close();
        if ( !aRet.isEmpty() )
            return aRet.copy(0,aRet.lastIndexOf(','));
    }
    return OUString();
}

OUString ODatabaseMetaData::getSearchStringEscape(  )
{
    return getLiteral(DBLITERAL_ESCAPE_PERCENT);
}

OUString ODatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

bool ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_EXTENDED) == DBPROPVAL_SQL_ODBC_EXTENDED);
}

bool ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_CORE) == DBPROPVAL_SQL_ODBC_CORE);
}

bool ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_MINIMUM) == DBPROPVAL_SQL_ODBC_MINIMUM);
}

bool ODatabaseMetaData::supportsFullOuterJoins(  )
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return (getInt32Property("Outer Join Capabilities") & 0x00000004L) == 0x00000004L;
}

bool ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return supportsFullOuterJoins(  );
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return getInt32Property("Max Columns in GROUP BY");
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return getInt32Property("Max Columns in ORDER BY");
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxUserNameLength(  )
{
    return getMaxSize(DBLITERAL_USER_NAME);
}

bool ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ )
{
    return true;
}

bool ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::updatesAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::deletesAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::insertsAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

bool ODatabaseMetaData::supportsBatchUpdates(  )
{
    return true;
}

Reference< XResultSet > ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XDatabaseMetaData::getUDTs", *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
