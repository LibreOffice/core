/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "DatabaseMetaData.hxx"
#include "Util.hxx"

#include <ibase.h>
#include <rtl/ustrbuf.hxx>
#include <FDatabaseMetaDataResultSet.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

using namespace connectivity::firebird;

using namespace ::rtl;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(Connection* _pCon)
: m_pConnection(_pCon)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}


OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator() throw(SQLException, RuntimeException)
{
    return OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength() throw(SQLException, RuntimeException)
{
    return -1;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm() throw(SQLException, RuntimeException)
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart() throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions() throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions() throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs() throw(SQLException, RuntimeException)
{
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eCatalogs);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls() throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions() throw(SQLException, RuntimeException)
{
    return sal_False;
}


sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength()
    throw(SQLException, RuntimeException)
{
    return -1;
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm()
    throw(SQLException, RuntimeException)
{
    return OUString();
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
    throw(SQLException, RuntimeException)
{
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eSchemas);
}


sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength() throw(SQLException, RuntimeException)
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize() throw(SQLException, RuntimeException)
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength() throw(SQLException, RuntimeException)
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength() throw(SQLException, RuntimeException)
{
    return 31;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex() throw(SQLException, RuntimeException)
{
    
    
    return 16;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength() throw(SQLException, RuntimeException)
{
    return 32;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections() throw(SQLException, RuntimeException)
{
    return 100; 
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable() throw(SQLException, RuntimeException)
{
    
    
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength() throw(SQLException, RuntimeException)
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength() throw(SQLException, RuntimeException)
{
    return 31;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}


sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}



OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString()
    throw(SQLException, RuntimeException)
{
    OUString aVal('"');
    return aVal;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    
    
    
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}



sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}


sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable()
    throw(SQLException, RuntimeException)
{
    
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert(sal_Int32 fromType,
                                                     sal_Int32 toType)
    throw(SQLException, RuntimeException)
{
    (void) fromType;
    (void) toType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    OUString aVal;
    return aVal;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::
        supportsDataDefinitionAndDataManipulationTransactions()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions()
    throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel(
        sal_Int32 aLevel)
    throw(SQLException, RuntimeException)
{
    return  aLevel == TransactionIsolation::READ_UNCOMMITTED
           || aLevel == TransactionIsolation::READ_COMMITTED
           || aLevel == TransactionIsolation::REPEATABLE_READ
           || aLevel == TransactionIsolation::SERIALIZABLE;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation()
    throw(SQLException, RuntimeException)
{
    return TransactionIsolation::REPEATABLE_READ;
}


sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_True; 
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 31; 
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    return m_pConnection->isReadOnly();
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return m_pConnection->isEmbedded();
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    
    return sal_False;
}


sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

OUString SAL_CALL ODatabaseMetaData::getURL() throw(SQLException, RuntimeException)
{
    return m_pConnection->getConnectionURL();
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion() throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; 
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    return 31;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType(sal_Int32 setType)
    throw(SQLException, RuntimeException)
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return sal_True;
        default:
            return sal_False;
    }
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency(
        sal_Int32 aResultSetType,
        sal_Int32 aConcurrency)
    throw(SQLException, RuntimeException)
{
    if (aResultSetType == ResultSetType::FORWARD_ONLY
        && aConcurrency == ResultSetConcurrency::READ_ONLY)
        return sal_True;
    else
        return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
    throw(SQLException, RuntimeException)
{
    
    return sal_False;
}

uno::Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection()
    throw(SQLException, RuntimeException)
{
    return (uno::Reference< XConnection >) m_pConnection;
}






uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo()
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getTypeInfo()");

    
    
    ODatabaseMetaDataResultSet* pResultSet =
            new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aResults;

    if(aResults.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow(19);

        
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue(); 
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue(); 
        aRow[7] = new ORowSetValueDecorator(sal_True); 
        aRow[8] = new ORowSetValueDecorator(sal_True); 
        aRow[10] = new ORowSetValueDecorator(sal_False); 
        
        
        
        
        aRow[11] = new ORowSetValueDecorator(sal_False);
        
        aRow[13] = new ORowSetValueDecorator();
        aRow[16] = new ORowSetValueDecorator();             
        aRow[17] = new ORowSetValueDecorator();             
        aRow[18] = new ORowSetValueDecorator(sal_Int16(10));

        
        aRow[1] = new ORowSetValueDecorator(OUString("CHAR"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_TEXT));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32767)); 
        aRow[6] = new ORowSetValueDecorator(OUString("length")); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        
        aRow[1] = new ORowSetValueDecorator(OUString("VARCHAR"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_VARYING));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32767)); 
        aRow[6] = new ORowSetValueDecorator(OUString("length")); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        
        {
            aRow[6] = new ORowSetValueDecorator(); 
            aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
            aRow[12] = new ORowSetValueDecorator(sal_True); 
            aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
            aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        }
        
        aRow[1] = new ORowSetValueDecorator(OUString("SMALLINT"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_SHORT));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(5)); 
        aResults.push_back(aRow);
        
        aRow[1] = new ORowSetValueDecorator(OUString("INTEGER"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_LONG));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(10)); 
        aResults.push_back(aRow);
        
        aRow[1] = new ORowSetValueDecorator(OUString("BIGINT"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_INT64));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(20)); 
        aResults.push_back(aRow);

        
        {
            aRow[6] = new ORowSetValueDecorator(); 
            aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
            aRow[12] = new ORowSetValueDecorator(sal_True); 
        }
        
        aRow[1] = new ORowSetValueDecorator(OUString("FLOAT"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_FLOAT));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(7)); 
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); 
        aRow[15] = new ORowSetValueDecorator(sal_Int16(7)); 
        aResults.push_back(aRow);
        
        aRow[1] = new ORowSetValueDecorator(OUString("DOUBLE PRECISION"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_DOUBLE));
        aRow[3] = new ORowSetValueDecorator(sal_Int16(15)); 
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); 
        aRow[15] = new ORowSetValueDecorator(sal_Int16(15)); 
        aResults.push_back(aRow);







        

        
        
        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_TIMESTAMP));
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); 
        aRow[6] = new ORowSetValueDecorator(); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        
        
        aRow[1] = new ORowSetValueDecorator(OUString("TIME"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_TYPE_TIME));
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); 
        aRow[6] = new ORowSetValueDecorator(); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        
        
        aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_TYPE_DATE));
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); 
        aRow[6] = new ORowSetValueDecorator(); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        
        
        aRow[1] = new ORowSetValueDecorator(OUString("BLOB"));
        aRow[2] = new ORowSetValueDecorator(getColumnTypeFromFBType(SQL_BLOB));
        aRow[3] = new ORowSetValueDecorator(sal_Int32(0)); 
        aRow[6] = new ORowSetValueDecorator(); 
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::NONE)); 
        aRow[12] = new ORowSetValueDecorator(sal_False); 
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); 
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); 
        aResults.push_back(aRow);

        



    }
    pResultSet->setRows(aResults);
    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable,
        const OUString& sColumnNamePattern)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getColumnPrivileges() with "
             "Table: " << sTable
             << " & ColumnNamePattern: " << sColumnNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumnPrivileges);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    static const OUString wld("%");
    OUStringBuffer queryBuf(
            "SELECT "
            "priv.RDB$RELATION_NAME, "  
            "priv.RDB$GRANTOR,"         
            "priv.RDB$USER, "           
            "priv.RDB$PRIVILEGE, "      
            "priv.RDB$GRANT_OPTION, "   
            "priv.RDB$FIELD_NAME "      
            "FROM RDB$USER_PRIVILEGES priv ");

    {
        OUString sAppend = "WHERE priv.RDB$RELATION_NAME = '%' ";
        queryBuf.append(sAppend.replaceAll("%", sTable));
    }
    if (!sColumnNamePattern.isEmpty())
    {
        OUString sAppend;
        if (sColumnNamePattern.match(wld))
            sAppend = "AND priv.RDB$FIELD_NAME LIKE '%' ";
        else
            sAppend = "AND priv.RDB$FIELD_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll(wld, sColumnNamePattern));
    }

    queryBuf.append(" ORDER BY priv.RDB$FIELD, "
                              "priv.RDB$PRIVILEGE");

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aCurrentRow(8);
    aCurrentRow[0] = new ORowSetValueDecorator(); 
    aCurrentRow[1] = new ORowSetValueDecorator(); 
    aCurrentRow[2] = new ORowSetValueDecorator(); 

    while( rs->next() )
    {
        
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(6)));
        aCurrentRow[5] = new ORowSetValueDecorator(xRow->getString(2)); 
        aCurrentRow[6] = new ORowSetValueDecorator(xRow->getString(3)); 
        aCurrentRow[7] = new ORowSetValueDecorator(xRow->getString(4)); 
        aCurrentRow[7] = new ORowSetValueDecorator(xRow->getBoolean(5)); 

        aResults.push_back(aCurrentRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/,
        const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern,
        const OUString& columnNamePattern)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getColumns() with "
             "TableNamePattern: " << tableNamePattern <<
             " & ColumnNamePattern: " << columnNamePattern);

    OUStringBuffer queryBuf("SELECT "
        "relfields.RDB$RELATION_NAME, " 
        "relfields.RDB$FIELD_NAME, "    
        "relfields.RDB$DESCRIPTION,"    
        "relfields.RDB$DEFAULT_VALUE, " 
        "relfields.RDB$FIELD_POSITION, "
        "fields.RDB$FIELD_TYPE, "       
        "fields.RDB$FIELD_LENGTH, "     
        "fields.RDB$FIELD_PRECISION, "  
        
        
        
        
        "relfields.RDB$NULL_FLAG "      
        "FROM RDB$RELATION_FIELDS relfields "
        "JOIN RDB$FIELDS fields "
        "on (fields.RDB$FIELD_NAME = relfields.RDB$FIELD_SOURCE) "
        "WHERE (1 = 1) ");

    if (!tableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (tableNamePattern.match("%"))
            sAppend = "AND relfields.RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "AND relfields.RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", tableNamePattern));
    }

    if (!columnNamePattern.isEmpty())
    {
        OUString sAppend;
        if (columnNamePattern.match("%"))
            sAppend = "AND relfields.RDB$FIELD_NAME LIKE '%' ";
        else
            sAppend = "AND relfields.RDB$FIELD_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", columnNamePattern));
    }

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XStatement > statement = m_pConnection->createStatement();
    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(19);

    aCurrentRow[0] =  new ORowSetValueDecorator(); 
    aCurrentRow[1] =  new ORowSetValueDecorator(); 
    aCurrentRow[2] =  new ORowSetValueDecorator(); 
    aCurrentRow[8] =  new ORowSetValueDecorator(); 
    aCurrentRow[10] = new ORowSetValueDecorator(sal_Int32(10)); 
    aCurrentRow[14] = new ORowSetValueDecorator(); 
    aCurrentRow[15] = new ORowSetValueDecorator(); 

    while( rs->next() )
    {
        
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        
        short aType = getFBTypeFromBlrType(xRow->getShort(6));
        aCurrentRow[5] = new ORowSetValueDecorator(getColumnTypeFromFBType(aType));
        
        aCurrentRow[6] = new ORowSetValueDecorator(getColumnTypeNameFromFBType(aType));

        
        {
            sal_Int32 aColumnSize = 0;
            switch (aType)
            {
                case SQL_TEXT:
                case SQL_VARYING:
                    aColumnSize = xRow->getShort(7);
                    break;
                case SQL_SHORT:
                case SQL_LONG:
                case SQL_FLOAT:
                case SQL_DOUBLE:
                case SQL_D_FLOAT:
                case SQL_INT64:
                case SQL_QUAD:
                    aColumnSize = xRow->getShort(8);
                    break;
                case SQL_TIMESTAMP:
                case SQL_BLOB:
                case SQL_ARRAY:
                case SQL_TYPE_TIME:
                case SQL_TYPE_DATE:
                case SQL_NULL:
                    
                    break;
            }
            aCurrentRow[7] = new ORowSetValueDecorator(aColumnSize);
        }

        
        
        aCurrentRow[9] = new ORowSetValueDecorator(sal_Int32(0));

        
        if (xRow->getShort(9))
        {
            aCurrentRow[11] = new ORowSetValueDecorator(ColumnValue::NO_NULLS);
        }
        else
        {
            aCurrentRow[11] = new ORowSetValueDecorator(ColumnValue::NULLABLE);
        }
        
        {
            OUString aDescription;
            uno::Reference< XBlob > xDescriptionBlob = xRow->getBlob(3);
            if (xDescriptionBlob.is())
            {
                sal_Int32 aBlobLength = (sal_Int32) xDescriptionBlob->length();
                aDescription = OUString((char*) xDescriptionBlob->getBytes(0, aBlobLength).getArray(),
                                        aBlobLength,
                                        RTL_TEXTENCODING_UTF8);
            }
            aCurrentRow[12] = new ORowSetValueDecorator(aDescription);
        }
        
        {
            uno::Reference< XBlob > xDefaultValueBlob = xRow->getBlob(4);
            if (xDefaultValueBlob.is())
            {
                
            }
            aCurrentRow[13] = new ORowSetValueDecorator();
        }

        
        if (aType == SQL_TEXT)
        {
            aCurrentRow[16] = new ORowSetValueDecorator(xRow->getShort(7));
        }
        else if (aType == SQL_VARYING)
        {
            aCurrentRow[16] = new ORowSetValueDecorator(sal_Int32(32767));
        }
        else
        {
            aCurrentRow[16] = new ORowSetValueDecorator(sal_Int32(0));
        }
        
        {
            short nColumnNumber = xRow->getShort(5);
            
            
            aCurrentRow[17] = new ORowSetValueDecorator(sal_Int32(nColumnNumber + 1));
        }
        
        if (xRow->getShort(9))
        {
            aCurrentRow[18] = new ORowSetValueDecorator(OUString("NO"));
        }
        else
        {
            aCurrentRow[18] = new ORowSetValueDecorator(OUString("YES"));
        }

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumns);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/,
        const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern,
        const Sequence< OUString >& types)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getTables() with "
             "TableNamePattern: " << tableNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    static const OUString wld("%");
    OUStringBuffer queryBuf(
            "SELECT "
            "RDB$RELATION_NAME, "
            "RDB$SYSTEM_FLAG, "
            "RDB$RELATION_TYPE, "
            "RDB$DESCRIPTION, "
            "RDB$VIEW_BLR "
            "FROM RDB$RELATIONS "
            "WHERE ");

    
    if ((types.getLength() == 0) || (types.getLength() == 1 && types[0].match(wld)))
    {
        
        queryBuf.append("(RDB$RELATION_TYPE = 0 OR RDB$RELATION_TYPE = 1) ");
    }
    else
    {
        queryBuf.append("( (0 = 1) ");
        for (int i = 0; i < types.getLength(); i++)
        {
            if (types[i] == "SYSTEM TABLE")
                queryBuf.append("OR (RDB$SYSTEM_FLAG = 1 AND RDB$VIEW_BLR IS NULL) ");
            else if (types[i] == "TABLE")
                queryBuf.append("OR (RDB$SYSTEM_FLAG IS NULL OR RDB$SYSTEM_FLAG = 0 AND RDB$VIEW_BLR IS NULL) ");
            else if (types[i] == "VIEW")
                queryBuf.append("OR (RDB$SYSTEM_FLAG IS NULL OR RDB$SYSTEM_FLAG = 0 AND RDB$VIEW_BLR IS NOT NULL) ");
            else
                throw SQLException(); 
        }
        queryBuf.append(") ");
    }

    if (!tableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (tableNamePattern.match(wld))
            sAppend = "AND RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "AND RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll(wld, tableNamePattern));
    }

    queryBuf.append(" ORDER BY RDB$RELATION_TYPE, RDB$RELATION_NAME");

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aCurrentRow(6);
    aCurrentRow[0] = new ORowSetValueDecorator(); 
    aCurrentRow[1] = new ORowSetValueDecorator(); 
    aCurrentRow[2] = new ORowSetValueDecorator(); 

    while( rs->next() )
    {
        
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        
        {
            
            sal_Int16 nSystemFlag = xRow->getShort(2);
            sal_Int16 nTableType  = xRow->getShort(3);
            xRow->getBlob(5); 
            sal_Bool aIsView      = !xRow->wasNull();
            OUString sTableType;

            if (nSystemFlag == 1)
            {
                sTableType = "SYSTEM TABLE";
            }
            else if (aIsView)
            {
                sTableType = "VIEW";
            }
            else
            {
                if (nTableType == 0)
                    sTableType = "TABLE";
            }

            aCurrentRow[4] = new ORowSetValueDecorator(sTableType);
        }
        
        {
            uno::Reference< XBlob > xBlob   = xRow->getBlob(4);
            OUString sDescription;

            if (xBlob.is())
            {
                
                
                sal_Int32 aBlobLength = (sal_Int32) xBlob->length();
                sDescription = OUString((char*) xBlob->getBytes(0, aBlobLength).getArray(),
                                        aBlobLength,
                                        RTL_TEXTENCODING_UTF8);
            }

            aCurrentRow[5] = new ORowSetValueDecorator(sDescription);
        }

        aResults.push_back(aCurrentRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    (void) columnNamePattern;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eProcedureColumns);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eProcedures);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    (void) catalog;
    (void) schema;
    (void) table;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eVersionColumns);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    
    
    
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    (void) catalog;
    (void) schema;
    (void) table;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eExportedKeys);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    
    
    
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    (void) catalog;
    (void) schema;
    (void) table;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eImportedKeys);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getPrimaryKeys() with "
             "Table: " << sTable);

    OUStringBuffer aQueryBuf("SELECT "
        "constr.RDB$RELATION_NAME, "    
        "inds.RDB$FIELD_NAME, "         
        "inds.RDB$FIELD_POSITION, "     
        "constr.RDB$CONSTRAINT_NAME "   
        "FROM RDB$RELATION_CONSTRAINTS constr "
        "JOIN RDB$INDEX_SEGMENTS inds "
        "on (constr.RDB$INDEX_NAME = inds.RDB$INDEX_NAME) ");

    OUString sAppend = "WHERE constr.RDB$RELATION_NAME = '%' ";
    aQueryBuf.append(sAppend.replaceAll("%", sTable));

    aQueryBuf.append("AND constr.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY' "
                    "ORDER BY inds.RDB$FIELD_NAME");

    OUString sQuery = aQueryBuf.makeStringAndClear();

    uno::Reference< XStatement > xStatement = m_pConnection->createStatement();
    uno::Reference< XResultSet > xRs = xStatement->executeQuery(sQuery);
    uno::Reference< XRow > xRow( xRs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(7);

    aCurrentRow[0] =  new ORowSetValueDecorator(); 
    aCurrentRow[1] =  new ORowSetValueDecorator(); 
    aCurrentRow[2] =  new ORowSetValueDecorator(); 

    while(xRs->next())
    {
        
        if (xRs->getRow() == 1) 
        {
            aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        }
        
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        
        aCurrentRow[5] = new ORowSetValueDecorator(xRow->getShort(3));
        
        aCurrentRow[6] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(4)));

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::ePrimaryKeys);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable,
        sal_Bool bIsUnique,
        sal_Bool bIsApproximate)
    throw(SQLException, RuntimeException)
{
    
    
    
    SAL_INFO("connectivity.firebird", "getPrimaryKeys() with "
             "Table: " << sTable);

    OUStringBuffer aQueryBuf("SELECT "
        "indices.RDB$RELATION_NAME, "               
        "index_segments.RDB$FIELD_NAME, "           
        "index_segments.RDB$FIELD_POSITION, "       
        "indices.RDB$INDEX_NAME, "                  
        "indices.RDB$UNIQUE_FLAG, "                 
        "indices.RDB$INDEX_TYPE "                   
        "FROM RDB$INDICES indices "
        "JOIN RDB$INDEX_SEGMENTS index_segments "
        "on (indices.RDB$INDEX_NAME = index_segments.RDB$INDEX_NAME) "
        "WHERE indices.RDB$RELATION_NAME = '" + sTable + "' "
        "AND (indices.RDB$SYSTEM_FLAG = 0) ");
    
    
    

    
    
    if (bIsUnique)
        aQueryBuf.append("AND (indices.RDB$UNIQUE_FLAG = 1) ");

    
    (void) bIsApproximate;

    OUString sQuery = aQueryBuf.makeStringAndClear();

    uno::Reference< XStatement > xStatement = m_pConnection->createStatement();
    uno::Reference< XResultSet > xRs = xStatement->executeQuery(sQuery);
    uno::Reference< XRow > xRow( xRs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(14);

    aCurrentRow[0] = new ORowSetValueDecorator(); 
    aCurrentRow[1] = new ORowSetValueDecorator(); 
    aCurrentRow[2] = new ORowSetValueDecorator(); 
    aCurrentRow[5] = new ORowSetValueDecorator(); 
    
    
    aCurrentRow[7] = new ORowSetValueDecorator(IndexType::CLUSTERED); 
    aCurrentRow[13] = new ORowSetValueDecorator(); 

    while(xRs->next())
    {
        
        if (xRs->getRow() == 1) 
        {
            aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        }

        
        aCurrentRow[4] = new ORowSetValueDecorator(!xRow->getBoolean(5));
        
        aCurrentRow[6] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(4)));

        
        aCurrentRow[8] = new ORowSetValueDecorator(xRow->getShort(3));
        
        aCurrentRow[9] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        
        if (xRow->getShort(6) == 1)
            aCurrentRow[10] = new ORowSetValueDecorator(OUString("D"));
        else
            aCurrentRow[10] = new ORowSetValueDecorator(OUString("A"));
        
        
        aCurrentRow[11] = new ORowSetValueDecorator((sal_Int32)0); 
        
        aCurrentRow[12] = new ORowSetValueDecorator((sal_Int32)0); 

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::ePrimaryKeys);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope,
    sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    (void) scope;
    (void) nullable;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eBestRowIdentifier);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*aCatalog*/,
        const OUString& /*sSchemaPattern*/,
        const OUString& sTableNamePattern)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getTablePrivileges() with "
             "TableNamePattern: " << sTableNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTablePrivileges);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    
    
    static const OUString wld("%");
    OUStringBuffer queryBuf(
            "SELECT "
            "priv.RDB$RELATION_NAME, "  
            "priv.RDB$GRANTOR,"         
            "priv.RDB$USER, "           
            "priv.RDB$PRIVILEGE, "      
            "priv.RDB$GRANT_OPTION "    
            "FROM RDB$USER_PRIVILEGES priv ");

    if (!sTableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (sTableNamePattern.match(wld))
            sAppend = "WHERE priv.RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "WHERE priv.RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll(wld, sTableNamePattern));
    }
    queryBuf.append(" ORDER BY priv.RDB$RELATION_TYPE, "
                              "priv.RDB$RELATION_NAME, "
                              "priv.RDB$PRIVILEGE");

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aRow(8);
    aRow[0] = new ORowSetValueDecorator(); 
    aRow[1] = new ORowSetValueDecorator(); 
    aRow[2] = new ORowSetValueDecorator(); 

    while( rs->next() )
    {
        
        aRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        aRow[4] = new ORowSetValueDecorator(xRow->getString(2)); 
        aRow[5] = new ORowSetValueDecorator(xRow->getString(3)); 
        aRow[6] = new ORowSetValueDecorator(xRow->getString(4)); 
        aRow[7] = new ORowSetValueDecorator(xRow->getBoolean(5)); 

        aResults.push_back(aRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const OUString& primarySchema,
    const OUString& primaryTable, const Any& foreignCatalog,
    const OUString& foreignSchema, const OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    (void) primaryCatalog;
    (void) primarySchema;
    (void) primaryTable;
    (void) foreignCatalog;
    (void) foreignSchema;
    (void) foreignTable;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eCrossReference);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) typeNamePattern;
    (void) types;
    OSL_FAIL("Not implemented yet!");
    
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eUDTs);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
