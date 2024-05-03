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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  )
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  )
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getColumnPrivileges(catalog,schema,table,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnPrivilegesMap();
    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getColumns(catalog,schemaPattern,tableNamePattern,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnsMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    ADORecordset *pRecordset = m_rADOConnection.getTables(catalog,schemaPattern,tableNamePattern,types);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTablesMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern )
{
    ADORecordset *pRecordset = m_rADOConnection.getProcedureColumns(catalog,schemaPattern,procedureNamePattern,columnNamePattern);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProcedureColumnsMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return getMaxSize(DBLITERAL_BINARY_LITERAL);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  )
{
    return getInt32Property("Maximum Row Size");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    return getMaxSize(DBLITERAL_CATALOG_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    return getMaxSize(DBLITERAL_CHAR_LITERAL);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return getMaxSize(DBLITERAL_COLUMN_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  )
{
    return getMaxSize(DBLITERAL_CURSOR_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  )
{
    return getInt32Property("Active Sessions");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  )
{
    return getInt32Property("Max Columns in Table");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  )
{
    return getMaxSize(DBLITERAL_TEXT_COMMAND);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  )
{
    return getMaxSize(DBLITERAL_TABLE_NAME);
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return getInt32Property("Maximum Tables in SELECT");
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getExportedKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getImportedKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table )
{
    ADORecordset *pRecordset = m_rADOConnection.getPrimaryKeys(catalog,schema,table);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setPrimaryKeysMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    sal_Bool unique, sal_Bool approximate )
{
    ADORecordset *pRecordset = m_rADOConnection.getIndexInfo(catalog,schema,table,unique,approximate);
    ADOS::ThrowException(m_rADOConnection,*this);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setIndexInfoMap();

    return pResult;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
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

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return getBoolProperty("Maximum Row Size Includes BLOB");
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  )
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    return getInt32Property("Maximum Index Size");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NON_NULL;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  )
{
    return getStringProperty("Catalog Term");
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return getLiteral(DBLITERAL_QUOTE_PREFIX);

}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return getInt32Property("Catalog Location") == DBPROPVAL_CL_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_IGNORE;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_COMMIT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DML;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_ALL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return getInt32Property("Prepare Abort Behavior") == DBPROPVAL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return getInt32Property("Prepare Commit Behavior") == DBPROPVAL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_COMMIT) == DBPROPVAL_TR_COMMIT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_ABORT) == DBPROPVAL_TR_ABORT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level )
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_FULL) == DBPROPVAL_SQL_ANSI92_FULL);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_ENTRY) == DBPROPVAL_SQL_ANSI92_ENTRY);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI89_IEF) == DBPROPVAL_SQL_ANSI89_IEF);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  )
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return getBoolProperty("Outer Join Capabilities");
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    return new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return getMaxSize(DBLITERAL_PROCEDURE_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    return getMaxSize(DBLITERAL_SCHEMA_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  )
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_NONE;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    return getBoolProperty("Read-Only Data Source");
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NULL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  )
{
    return isCapable(DBLITERAL_COLUMN_ALIAS);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return getBoolProperty("Rowset Conversions on Command");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_NOT_SUPPORTED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_CONTAINS_SELECT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return getInt32Property("GROUP BY Support") == DBPROPVAL_GB_NO_RELATION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return isCapable(DBLITERAL_ESCAPE_PERCENT);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_END) == DBPROPVAL_NC_END;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_START) == DBPROPVAL_NC_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_HIGH) == DBPROPVAL_NC_HIGH;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_LOW) == DBPROPVAL_NC_LOW;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_PRIVILEGE_DEFINITION) == DBPROPVAL_SU_PRIVILEGE_DEFINITION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_CORRELATEDSUBQUERIES) == DBPROPVAL_SQ_CORRELATEDSUBQUERIES;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_COMPARISON) == DBPROPVAL_SQ_COMPARISON;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_EXISTS) == DBPROPVAL_SQ_EXISTS;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_IN) == DBPROPVAL_SQ_IN;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_QUANTIFIED) == DBPROPVAL_SQ_QUANTIFIED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_INTERMEDIATE) == DBPROPVAL_SQL_ANSI92_INTERMEDIATE);
}

OUString SAL_CALL ODatabaseMetaData::getURL(  )
{
    return "sdbc:ado:"+ m_rADOConnection.GetConnectionString();
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  )
{
    return getStringProperty("User Name");
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  )
{
    return getStringProperty("Provider Friendly Name");
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  )
{
    return getStringProperty("Provider Version");
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    return getStringProperty("DBMS Version");
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    return getStringProperty("DBMS Name");
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    return getStringProperty("Procedure Term");
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  )
{
    return getStringProperty("Schema Term");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  )
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

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  )
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

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  )
{
    return getLiteral(DBLITERAL_ESCAPE_PERCENT);
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_EXTENDED) == DBPROPVAL_SQL_ODBC_EXTENDED);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_CORE) == DBPROPVAL_SQL_ODBC_CORE);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_MINIMUM) == DBPROPVAL_SQL_ODBC_MINIMUM);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  )
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return (getInt32Property("Outer Join Capabilities") & 0x00000004L) == 0x00000004L;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return supportsFullOuterJoins(  );
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return getInt32Property("Max Columns in GROUP BY");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return getInt32Property("Max Columns in ORDER BY");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  )
{
    return getMaxSize(DBLITERAL_USER_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType )
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  )
{
    return true;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XDatabaseMetaData::getUDTs", *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
