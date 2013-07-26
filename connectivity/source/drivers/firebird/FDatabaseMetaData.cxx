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

#include "FDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include "Util.hxx"

#include <ibase.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
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

namespace connectivity
{
    namespace firebird
    {
        static sal_Int32    const s_nCOLUMN_SIZE = 256;
        static sal_Int32    const s_nDECIMAL_DIGITS = 0;
        static sal_Int32    const s_nNULLABLE = 1;
        static sal_Int32 const s_nCHAR_OCTET_LENGTH = 65535;
    }
}

ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
: m_pConnection(_pCon)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

//----- Catalog Info -- UNSUPPORTED -------------------------------------------
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
    return 0;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls() throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions() throw(SQLException, RuntimeException)
{
    return sal_False;
}

//----- Max Sizes/Lengths -----------------------------------------------------
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
    // TODO: No idea.
    // See: http://www.firebirdsql.org/en/firebird-technical-specifications/
    return 16;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength() throw(SQLException, RuntimeException)
{
    return 32;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections() throw(SQLException, RuntimeException)
{
    return 100; // Arbitrary
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable() throw(SQLException, RuntimeException)
{
    // May however be smaller.
    // See: http://www.firebirdsql.org/en/firebird-technical-specifications/
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
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

// ---- Identifiers -----------------------------------------------------------
// Only quoted identifiers are case sensitive, unquoted are case insensitive
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
    // TODO: confirm this -- the documentation is highly ambiguous
    // However it seems this should be true as quoted identifiers ARE
    // stored mixed case.
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
    throw(SQLException, RuntimeException)
{
    return sal_False;
}

// ---- Unquoted Identifiers -------------------------------------------------
// All unquoted identifers are stored upper case.
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

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    OUString aVal;
    return aVal;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    (void) level;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_True; // should be supported at least
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 31; // TODO: confirm
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = -1; // 0 means no limit
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException)
{
    (void) fromType;
    (void) toType;
    return sal_False;
}
// -------------------------------------------------------------------------
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
    // Unsure
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    // Unsure
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}

OUString SAL_CALL ODatabaseMetaData::getURL() throw(SQLException, RuntimeException)
{
    return m_pConnection->getConnectionURL();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDriverVersion() throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return TransactionIsolation::NONE;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException)
{
    (void) setType;
    (void) concurrency;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
    throw(SQLException, RuntimeException)
{
    // No batch support in firebird
    return sal_False;
}

uno::Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection()
    throw(SQLException, RuntimeException)
{
    return (uno::Reference< XConnection >) m_pConnection;
}
// -------------------------------------------------------------------------
// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "getTypeInfo().");

    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aRows;

    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(19);
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR(100)")));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)s_nCHAR_OCTET_LENGTH));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        // aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnSearch::CHAR));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));

        aRows.push_back(aRow);

    }
    pResultSet->setRows(aRows);
    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
    throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const OUString& schema, const OUString& table,
    const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    (void) columnNamePattern;
    return NULL;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& tableNamePattern,
        const OUString& columnNamePattern)
    throw(SQLException, RuntimeException)
{
    (void) catalog;         // Unsupported in firebird
    (void) schemaPattern;   // Unsupported in firebird
    SAL_INFO("connectivity.firebird", "getColumns() with "
             "TableNamePattern: " << tableNamePattern <<
             " & ColumnNamePattern: " << columnNamePattern);

    OUStringBuffer queryBuf("SELECT "
        "relfields.RDB$RELATION_NAME, " // 1
        "relfields.RDB$FIELD_NAME, "    // 2
        "relfields.RDB$DESCRIPTION,"    // 3
        "relfields.RDB$DEFAULT_VALUE, " // 4
        "relfields.RDB$FIELD_POSITION, "// 5
        "fields.RDB$FIELD_TYPE, "       // 6
        "fields.RDB$FIELD_LENGTH, "     // 7
        "fields.RDB$FIELD_PRECISION, "  // 8
        "fields.RDB$NULL_FLAG "         // 9
        "FROM RDB$RELATION_FIELDS relfields "
        "JOIN RDB$FIELDS fields "
        "on (fields.RDB$FIELD_NAME = relfields.RDB$FIELD_SOURCE) ");

    if (!tableNamePattern.isEmpty() && !columnNamePattern.isEmpty())
    {
        queryBuf.append("WHERE ");
    }
    if (!tableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (tableNamePattern.match("%"))
            sAppend = "relfields.RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "relfields.RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", tableNamePattern));
    }

    if (!columnNamePattern.isEmpty())
    {
        if (!tableNamePattern.isEmpty())
            queryBuf.append("AND ");

        OUString sAppend;
        if (columnNamePattern.match("%"))
            sAppend = "relfields.RDB$FIELD_NAME LIKE '%' ";
        else
            sAppend = "relfields.RDB$FIELD_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", columnNamePattern));
    }

    OUString query = queryBuf.makeStringAndClear();

    SAL_INFO("connectivity.firebird", "Retrieving columns with " <<
                OUStringToOString(query,RTL_TEXTENCODING_UTF8).getStr());

    uno::Reference< XStatement > statement = m_pConnection->createStatement();
    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;

    while( rs->next() )
    {
        ODatabaseMetaDataResultSet::ORow aCurrentRow(18);

        // 1. TABLE_CAT (catalog) may be null
        aCurrentRow.push_back(new ORowSetValueDecorator());
        // 2. TABLE_SCHEM (schema) may be null
        aCurrentRow.push_back(new ORowSetValueDecorator());
        // 3. TABLE_NAME
        {
            OUString aTableName = xRow->getString(1);
            aCurrentRow.push_back(new ORowSetValueDecorator(aTableName));
        }
        // 4. Column Name
        {
            OUString aColumnName = xRow->getString(2);
            aCurrentRow.push_back(new ORowSetValueDecorator(aColumnName));
        }

        // 5. Datatype
        short aType = getFBTypeFromBlrType(xRow->getShort(6));
        aCurrentRow.push_back(new ORowSetValueDecorator(getColumnTypeFromFBType(aType)));
        // 6. Typename (SQL_*)
        aCurrentRow.push_back(new ORowSetValueDecorator(getColumnTypeNameFromFBType(aType)));

        // 7. Column Sizes
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
                    // TODO: implement.
                    break;
            }
            aCurrentRow.push_back(new ORowSetValueDecorator(aColumnSize));
        }
        // 8. Unused
        aCurrentRow.push_back(new ORowSetValueDecorator());
        // 9. Decimal Digits
        // TODO: implement
        aCurrentRow.push_back(new ORowSetValueDecorator(sal_Int32(0)));
        // 10. Radix
        aCurrentRow.push_back(new ORowSetValueDecorator(sal_Int32(10)));
        // 11. Nullable
        if (xRow->getShort(9))
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(ColumnValue::NO_NULLS));
        }
        else
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(ColumnValue::NULLABLE));
        }
        // 12. Comments -- may be omitted
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
            aCurrentRow.push_back(new ORowSetValueDecorator(aDescription));
        }
        // 13. Default --  may be omitted.
        {
            uno::Reference< XBlob > xDefaultValueBlob = xRow->getBlob(4);
            if (xDefaultValueBlob.is())
            {
                // TODO: Implement
            }
            aCurrentRow.push_back(new ORowSetValueDecorator());
        }
        // 14. Unused
        aCurrentRow.push_back(new ORowSetValueDecorator());
        // 15. Unused
        aCurrentRow.push_back(new ORowSetValueDecorator());
        // 16. Bytes in Column for char
        if (aType == SQL_TEXT)
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(xRow->getShort(7)));
        }
        else if (aType == SQL_VARYING)
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(sal_Int32(32767)));
        }
        else
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(sal_Int32(0)));
        }
        // 17. Index of column
        {
            short aColumnNumber = xRow->getShort(5);
            // Firebird stores column numbers beginning with 0 internally
            // SDBC expects column numbering to begin with 1.
            aCurrentRow.push_back(new ORowSetValueDecorator(sal_Int32(aColumnNumber + 1)));
        }
        // 18. Is nullable
        if (xRow->getShort(9))
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(OUString("NO")));
        }
        else
        {
            aCurrentRow.push_back(new ORowSetValueDecorator(OUString("YES")));
        }

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& catalog,
        const OUString& schemaPattern,
        const OUString& tableNamePattern,
        const Sequence< OUString >& types)
    throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) types;
    // TODO: implement types
    SAL_INFO("connectivity.firebird", "getTables() with "
             "TableNamePattern: " << tableNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    static const OUString wld("%");
    OUStringBuffer queryBuf(
            "SELECT "
            "RDB$RELATION_NAME, RDB$SYSTEM_FLAG, RDB$RELATION_TYPE, "
            "RDB$DESCRIPTION "
            "FROM RDB$RELATIONS "
            "WHERE (RDB$RELATION_TYPE = 0 OR RDB$RELATION_TYPE = 1)");

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
    ODatabaseMetaDataResultSet::ORows aRows;

    while( rs->next() )
    {
        ODatabaseMetaDataResultSet::ORow aCurrentRow(3);

        OUString aTableName             = xRow->getString(1);
        sal_Int16 systemFlag            = xRow->getShort(2);
        sal_Int16 tableType             = xRow->getShort(3);
        uno::Reference< XBlob > xBlob   = xRow->getBlob(4);

        OUString aDescription;
        if (xBlob.is())
        {
            sal_Int32 aBlobLength = (sal_Int32) xBlob->length();
            aDescription = OUString((char*) xBlob->getBytes(0, aBlobLength).getArray(),
                                    aBlobLength,
                                    RTL_TEXTENCODING_UTF8);
        }

        OUString aTableType;
        if( 1 == systemFlag )
        {
            aTableType = OUString::createFromAscii("SYSTEM TABLE");

        }
        else
        {
            if( 0 == tableType )
            {
                aTableType = OUString::createFromAscii("TABLE");
            }
            else
            {
                aTableType = OUString::createFromAscii("VIEW");
            }
        }

        // TABLE_CAT (catalog) may be null -- thus we omit it.
        // TABLE_SCHEM (schema) may be null -- thus we omit it.
        // TABLE_NAME
        aCurrentRow.push_back(new ORowSetValueDecorator(aTableName));
        // TABLE_TYPE
        aCurrentRow.push_back(new ORowSetValueDecorator(aTableType));
        // REMARKS
        aCurrentRow.push_back(new ORowSetValueDecorator(aDescription));

        aRows.push_back(aCurrentRow);
    }

    pResultSet->setRows( aRows );

    return xResultSet;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    (void) columnNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const OUString& schemaPattern,
    const OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const OUString& schema, const OUString& table,
    sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    (void) unique;
    (void) approximate;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope,
    sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    (void) scope;
    (void) nullable;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) tableNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
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
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) typeNamePattern;
    (void) types;
    OSL_FAIL("Not implemented yet!");
    throw SQLException();
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
