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
using namespace ::cpo::uno;
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

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return getLiteral(DBLITERAL_CATALOG_SEPARATOR);
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

sal_Int32 ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return getMaxSize(DBLITERAL_COLUMN_NAME);
}

sal_Int32 ODatabaseMetaData::getMaxTableNameLength(  )
{
    return getMaxSize(DBLITERAL_TABLE_NAME);
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return getInt32Property("Maximum Tables in SELECT");
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

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED ;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return true;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return true;
}

bool ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return getInt32Property("NULL Concatenation Behavior") == DBPROPVAL_CB_NON_NULL;
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return getLiteral(DBLITERAL_QUOTE_PREFIX);

}

OUString ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return getInt32Property("Catalog Location") == DBPROPVAL_CL_START;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return (getInt32Property("Schema Usage") & DBPROPVAL_SU_DML_STATEMENTS) == DBPROPVAL_SU_DML_STATEMENTS;
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

Reference< XResultSet > ODatabaseMetaData::getTableTypes(  )
{
    return new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
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

bool ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return getInt32Property("GROUP BY Support") == DBPROPVAL_GB_NO_RELATION;
}

bool ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return getBoolProperty("ORDER BY Columns in Select List");
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return (getInt32Property("Identifier Case Sensitivity") & DBPROPVAL_IC_MIXED) == DBPROPVAL_IC_MIXED;
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

OUString ODatabaseMetaData::getURL(  )
{
    return "sdbc:ado:"+ m_rADOConnection.GetConnectionString();
}

OUString ODatabaseMetaData::getUserName(  )
{
    return getStringProperty("User Name");
}

OUString ODatabaseMetaData::getDatabaseProductName(  )
{
    return getStringProperty("DBMS Name");
}

bool ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    sal_Int32 nProp = getInt32Property("SQL Support");
    return (nProp == 512) || ((nProp & DBPROPVAL_SQL_ODBC_CORE) == DBPROPVAL_SQL_ODBC_CORE);
}

bool ODatabaseMetaData::supportsFullOuterJoins(  )
{
    if ( ADOS::isJetEngine(m_pConnection->getEngineType()) )
        return true;
    return (getInt32Property("Outer Join Capabilities") & 0x00000004L) == 0x00000004L;
}

bool ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ )
{
    return true;
}

bool ODatabaseMetaData::supportsBatchUpdates(  )
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
