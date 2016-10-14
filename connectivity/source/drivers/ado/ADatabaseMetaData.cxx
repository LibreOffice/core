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

#include "ado/ADatabaseMetaData.hxx"
#include "ado/ADatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include "ado/AConnection.hxx"
#include "ado/adoimp.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
    : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
    ,m_pADOConnection(_pCon->getConnection())
    ,m_pConnection(_pCon)
{
}

sal_Int32 ODatabaseMetaData::getInt32Property(const OUString& _aProperty)  throw(css::sdbc::SQLException, css::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    //  ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    sal_Int32 nValue(0);
    if(!aVar.isNull() && !aVar.isEmpty())
        nValue = aVar.getInt32();
    return nValue;
}


bool ODatabaseMetaData::getBoolProperty(const OUString& _aProperty)  throw(css::sdbc::SQLException, css::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");
    ADO_PROP(_aProperty);
    return !aVar.isNull() && !aVar.isEmpty() && aVar.getBool();
}

OUString ODatabaseMetaData::getStringProperty(const OUString& _aProperty)  throw(css::sdbc::SQLException, css::uno::RuntimeException)
{
    connectivity::ado::WpADOProperties aProps(m_pADOConnection->get_Properties());
    ADOS::ThrowException(*m_pADOConnection,*this);
    OSL_ENSURE(aProps.IsValid(),"There are no properties at the connection");

    ADO_PROP(_aProperty);
    OUString aValue;
    if(!aVar.isNull() && !aVar.isEmpty() && aVar.getType() == VT_BSTR)
        aValue = aVar.getString();

    return aValue;
}

Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    ADORecordset *pRecordset = m_pADOConnection->getTypeInfo();

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTypeInfoMap(ADOS::isJetEngine(m_pConnection->getEngineType()));
    Reference< XResultSet > xRef = pResult;
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException)
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = nullptr;
    m_pADOConnection->OpenSchema(adSchemaCatalogs,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCatalogsMap();
    xRef = pResult;

    return xRef;
}

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return getLiteral(DBLITERAL_CATALOG_SEPARATOR);
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();

    ADORecordset *pRecordset = nullptr;
    m_pADOConnection->OpenSchema(adSchemaSchemata,vtEmpty,vtEmpty,&pRecordset);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setSchemasMap();
    xRef = pResult;
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getColumnPrivileges(catalog,schema,table,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnPrivilegesMap();
    xRef = pResult;
    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern,
    const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getColumns(catalog,schemaPattern,tableNamePattern,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setColumnsMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& tableNamePattern, const Sequence< OUString >& types ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getTables(catalog,schemaPattern,tableNamePattern,types);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setTablesMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getProcedureColumns(catalog,schemaPattern,procedureNamePattern,columnNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProcedureColumnsMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    // Create elements used in the array
    ADORecordset *pRecordset = m_pADOConnection->getProcedures(catalog,schemaPattern,procedureNamePattern);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setProceduresMap();
    xRef = pResult;

    return xRef;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_BINARY_LITERAL);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Maximum Row Size");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CATALOG_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CHAR_LITERAL);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_COLUMN_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_CURSOR_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Active Sessions");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Max Columns in Table");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_TEXT_COMMAND);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_TABLE_NAME);
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return getInt32Property("Maximum Tables in SELECT");
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getExportedKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getImportedKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getPrimaryKeys(catalog,schema,table);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setPrimaryKeysMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getIndexInfo(catalog,schema,table,unique,approximate);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setIndexInfoMap();
    xRef = pResult;

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    Reference< XResultSet > xRef;
    if(!ADOS::isJetEngine(m_pConnection->getEngineType()))
    {   // the jet provider doesn't support this method
        // Create elements used in the array

        ADORecordset *pRecordset = m_pADOConnection->getTablePrivileges(catalog,schemaPattern,tableNamePattern);
        ADOS::ThrowException(*m_pADOConnection,*this);

        ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
        pResult->setTablePrivilegesMap();
        xRef = pResult;
    }
    else
    {
        ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTablePrivileges);
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
        pResult->setRows(aRows);
    }

    return xRef;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const OUString& primarySchema,
    const OUString& primaryTable, const Any& foreignCatalog,
    const OUString& foreignSchema, const OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = m_pADOConnection->getCrossReference(primaryCatalog,primarySchema,primaryTable,foreignCatalog,foreignSchema,foreignTable);
    ADOS::ThrowException(*m_pADOConnection,*this);

    Reference< XResultSet > xRef;

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(pRecordset);
    pResult->setCrossReferenceMap();
    xRef = pResult;

    return xRef;
}

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty("Maximum Row Size Includes BLOB");
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_LOWER) == DBPROPVAL_IC_LOWER ;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_UPPER) == DBPROPVAL_IC_UPPER ;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
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

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Maximum Index Size");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NON_NULL;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("Catalog Term");
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return getLiteral(DBLITERAL_QUOTE_PREFIX);

}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return getInt32Property("Catalog Location") == DBPROPVAL_CL_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_IGNORE;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DDL_COMMIT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_DML;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_ALL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Prepare Abort Behavior") == DBPROPVAL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Prepare Commit Behavior") == DBPROPVAL_CB_PRESERVE;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_COMMIT) == DBPROPVAL_TR_COMMIT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Isolation Retention") & DBPROPVAL_TR_ABORT) == DBPROPVAL_TR_ABORT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException)
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_FULL) == DBPROPVAL_SQL_ANSI92_FULL);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_ENTRY) == DBPROPVAL_SQL_ANSI92_ENTRY);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI89_IEF) == DBPROPVAL_SQL_ANSI89_IEF);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return getBoolProperty("Outer Join Capabilities");
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    return new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_PROCEDURE_NAME);
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_SCHEMA_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Transaction DDL") == DBPROPVAL_TC_NONE;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty("Read-Only Data Source");
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NULL;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_COLUMN_ALIAS);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_CORRELATION_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ ) throw(SQLException, RuntimeException)
{
    return getBoolProperty("Rowset Conversions on Command");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_NOT_SUPPORTED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("GROUP BY Support") != DBPROPVAL_GB_CONTAINS_SELECT;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("GROUP BY Support") == DBPROPVAL_GB_NO_RELATION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return isCapable(DBLITERAL_ESCAPE_PERCENT);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_END) == DBPROPVAL_NC_END;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_START) == DBPROPVAL_NC_START;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_HIGH) == DBPROPVAL_NC_HIGH;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("NULL Collation Order") & DBPROPVAL_NC_LOW) == DBPROPVAL_NC_LOW;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_PRIVILEGE_DEFINITION) == DBPROPVAL_SU_PRIVILEGE_DEFINITION;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_CORRELATEDSUBQUERIES) == DBPROPVAL_SQ_CORRELATEDSUBQUERIES;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_COMPARISON) == DBPROPVAL_SQ_COMPARISON;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_EXISTS) == DBPROPVAL_SQ_EXISTS;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_IN) == DBPROPVAL_SQ_IN;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return (getInt32Property("Subquery Support") & DBPROPVAL_SQ_QUANTIFIED) == DBPROPVAL_SQ_QUANTIFIED;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ANSI92_INTERMEDIATE) == DBPROPVAL_SQL_ANSI92_INTERMEDIATE);
}

OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    return "sdbc:ado:"+ m_pADOConnection->GetConnectionString();
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("User Name");
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("Provider Friendly Name");
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("Provider Version");
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("DBMS Version");
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("DBMS Name");
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("Procedure Term");
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    return getStringProperty("Schema Term");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nRet = TransactionIsolation::NONE;
    switch(m_pADOConnection->get_IsolationLevel())
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

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ADORecordset *pRecordset = nullptr;
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();
    m_pADOConnection->OpenSchema(adSchemaDBInfoKeywords,vtEmpty,vtEmpty,&pRecordset);
    OSL_ENSURE(pRecordset,"getSQLKeywords: no resultset!");
    ADOS::ThrowException(*m_pADOConnection,*this);
    if ( pRecordset )
    {
        WpADORecordset aRecordset(pRecordset);

        aRecordset.MoveFirst();
        OLEVariant  aValue;
        OUString aRet, aComma(",");
        while(!aRecordset.IsAtEOF())
        {
            WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(aRecordset.GetFields());
            WpADOField aField(aFields.GetItem(0));
            aField.get_Value(aValue);
            aRet = aRet + aValue.getString() + aComma;
            aRecordset.MoveNext();
        }
        aRecordset.Close();
        if ( !aRet.isEmpty() )
            return aRet.copy(0,aRet.lastIndexOf(','));
    }
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    return getLiteral(DBLITERAL_ESCAPE_PERCENT);
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue.copy(0,aValue.lastIndexOf(','));
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue.copy(0,aValue.lastIndexOf(','));
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_EXTENDED) == DBPROPVAL_SQL_ODBC_EXTENDED);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_CORE) == DBPROPVAL_SQL_ODBC_CORE);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_MINIMUM) == DBPROPVAL_SQL_ODBC_MINIMUM);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return (getInt32Property("Outer Join Capabilities") & 0x00000004L) == 0x00000004L;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return supportsFullOuterJoins(  );
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Max Columns in GROUP BY");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return getInt32Property("Max Columns in ORDER BY");
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    return getMaxSize(DBLITERAL_USER_NAME);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ ) throw(SQLException, RuntimeException)
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY != setType;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return true;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XDatabaseMetaData::getUDTs", *this );
    return Reference< XResultSet >();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
