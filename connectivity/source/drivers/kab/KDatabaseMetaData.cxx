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


#include "KDatabaseMetaData.hxx"
#include "kfields.hxx"
#include "KDEInit.h"
#include <shell/kde_headers.h>
#include "FDatabaseMetaDataResultSet.hxx"
#include "OTypeInfo.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

KabDatabaseMetaData::KabDatabaseMetaData(KabConnection* _pCon)
        : m_xConnection(_pCon),
          m_bUseCatalog(sal_True)
{
    OSL_ENSURE(_pCon,"KabDatabaseMetaData::KabDatabaseMetaData: No connection set!");

    osl_atomic_increment( &m_refCount );
    m_bUseCatalog   = !(usesLocalFiles() || usesLocalFilePerTable());
    osl_atomic_decrement( &m_refCount );
}
// -------------------------------------------------------------------------
KabDatabaseMetaData::~KabDatabaseMetaData()
{
}
// -------------------------------------------------------------------------
const ::rtl::OUString & KabDatabaseMetaData::getAddressBookTableName()
{
    static const ::rtl::OUString aAddressBookTableName
        (::rtl::OUString::createFromAscii( i18n("Address Book") ));

    return aAddressBookTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getCatalogSeparator(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    if (m_bUseCatalog)
    { // do some special here for you database
    }

    return aVal;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxTablesInSelect(  ) throw(SQLException, RuntimeException)
{
    // MaxTablesInSelect describes how many tables can participate in the FROM part of a given SELECT statement,
    // currently, the resultset/statement implementations can cope with one table only
    sal_Int32 nValue = 1;
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    if (m_bUseCatalog)
    {
    }
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    // normally this is "
    ::rtl::OUString aVal("\"");
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    return aVal;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::isCatalogAtStart(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bValue = sal_False;
    if (m_bUseCatalog)
    {
    }
    return bValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSchemasInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_True; // should be supported at least
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSchemasInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCatalogsInTableDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCatalogsInDataManipulation(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxStatements(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    // for the moment, we have read-only addresses, but this might change in the future
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsConvert( sal_Int32, sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    // if someday we support more than the default address book,
    // this method should return the URL which was used to create it
    ::rtl::OUString aValue(  "sdbc:address:kab:" );
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue(  "kab" );
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getDriverVersion() throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue(KAB_DRIVER_VERSION);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return KAB_DRIVER_VERSION_MAJOR;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return TransactionIsolation::NONE;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return KAB_DRIVER_VERSION_MINOR;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return sal_True;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 ) throw(SQLException, RuntimeException)
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return sal_True;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::ownUpdatesAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::ownDeletesAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::ownInsertsAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::othersUpdatesAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::othersDeletesAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::othersInsertsAreVisible( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::updatesAreDetected( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::deletesAreDetected( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::insertsAreDetected( sal_Int32 ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabDatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL KabDatabaseMetaData::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >) m_xConnection.get();
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    static const ::rtl::OUString aTable("TABLE");

    if (aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow(2);
        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = new ORowSetValueDecorator(aTable);
        aRows.push_back(aRow);
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    if (aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow(19);

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = new ORowSetValueDecorator(::rtl::OUString("CHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::CHAR);
        aRow[3] = new ORowSetValueDecorator((sal_Int32) 254);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[6] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[7] = new ORowSetValueDecorator((sal_Int32) ColumnValue::NULLABLE);
        aRow[8] = ODatabaseMetaDataResultSet::get1Value();
        aRow[9] = new ORowSetValueDecorator((sal_Int32) ColumnSearch::CHAR);
        aRow[10] = ODatabaseMetaDataResultSet::get1Value();
        aRow[11] = ODatabaseMetaDataResultSet::get0Value();
        aRow[12] = ODatabaseMetaDataResultSet::get0Value();
        aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[14] = ODatabaseMetaDataResultSet::get0Value();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRow[16] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[17] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[18] = new ORowSetValueDecorator((sal_Int32) 10);
        aRows.push_back(aRow);
// Much more types might appear in KDE address books
// To be completed
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCatalogs );
}
// -----------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eSchemas );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getColumnPrivileges(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString&,
    const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumnPrivileges );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getColumns(
    const Any&,
    const ::rtl::OUString&,
    const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern) throw(SQLException, RuntimeException)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xRef = pResult;

    ODatabaseMetaDataResultSet::ORows aRows;

    if (match(tableNamePattern, getAddressBookTableName(), '\0'))
    {
        ODatabaseMetaDataResultSet::ORow aRow(19);

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[3] = new ORowSetValueDecorator(getAddressBookTableName());
        aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[9] = ODatabaseMetaDataResultSet::get0Value();
        aRow[10] = new ORowSetValueDecorator((sal_Int32) 10);
        aRow[11] = ODatabaseMetaDataResultSet::get1Value();
        aRow[12] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[14] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[15] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[16] = new ORowSetValueDecorator((sal_Int32) 254);
        aRow[18] = new ORowSetValueDecorator(::rtl::OUString("YES"));

        sal_Int32 nPosition = 1;
        QString aQtName;
        ::rtl::OUString sName;

        aQtName = ::KABC::Addressee::revisionLabel();
        sName = (const sal_Unicode *) aQtName.ucs2();
        if (match(columnNamePattern, sName, '\0'))
        {
            aRow[4] = new ORowSetValueDecorator(sName);
            aRow[5] = new ORowSetValueDecorator(DataType::TIMESTAMP);
            aRow[6] = new ORowSetValueDecorator(::rtl::OUString("TIMESTAMP"));
            aRow[17] = new ORowSetValueDecorator(nPosition++);
            aRows.push_back(aRow);
        }

        ::KABC::Field::List aFields = ::KABC::Field::allFields();
        ::KABC::Field::List::iterator aField;

        for (   aField = aFields.begin();
                aField != aFields.end();
                ++aField, ++nPosition)
        {
            aQtName = (*aField)->label();
            sName = (const sal_Unicode *) aQtName.ucs2();
            if (match(columnNamePattern, sName, '\0'))
            {
                aRow[4] = new ORowSetValueDecorator(sName);
                aRow[5] = new ORowSetValueDecorator(DataType::CHAR);
                aRow[6] = new ORowSetValueDecorator(::rtl::OUString("CHAR"));
                aRow[7] = new ORowSetValueDecorator((sal_Int32) 256);
// Might be VARCHAR and not CHAR[256]...
                aRow[17] = new ORowSetValueDecorator(nPosition);
                aRows.push_back(aRow);
            }
        }
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getTables(
    const Any&,
    const ::rtl::OUString&,
    const ::rtl::OUString&,
    const Sequence< ::rtl::OUString >& types) throw(SQLException, RuntimeException)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTables);
    Reference< XResultSet > xRef = pResult;

    // check whether we have tables in the requested types
    // for the moment, we answer only the "TABLE" table type
    // when no types are given at all, we return all the tables
    static const ::rtl::OUString aTable("TABLE");
    sal_Bool bTableFound = sal_False;
    const ::rtl::OUString* p = types.getConstArray(),
                         * pEnd = p + types.getLength();

    if (p == pEnd)
    {
        bTableFound = sal_True;
    }
    else while (p < pEnd)
    {
        if (match(*p, aTable, '\0'))
        {
            bTableFound = sal_True;
            break;
        }
        p++;
    }
    if (!bTableFound)
        return xRef;

    static ODatabaseMetaDataResultSet::ORows aRows;

    if (aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow(6);

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[3] = new ORowSetValueDecorator(getAddressBookTableName());
        aRow[4] = new ORowSetValueDecorator(aTable);
        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRows.push_back(aRow);
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getProcedureColumns(
    const Any&, const ::rtl::OUString&,
    const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedureColumns );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getProcedures(
    const Any&, const ::rtl::OUString&,
    const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedures );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getVersionColumns(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eVersionColumns);

    Reference< XResultSet > xRef = pResult;

    ODatabaseMetaDataResultSet::ORows aRows;

    if (table == getAddressBookTableName())
    {
        ODatabaseMetaDataResultSet::ORow aRow( 9 );
        QString aQtName = ::KABC::Addressee::revisionLabel();
        ::rtl::OUString sName = (const sal_Unicode *) aQtName.ucs2();

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();

        aRow[2] = new ORowSetValueDecorator(sName);
        aRow[3] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[4] = new ORowSetValueDecorator(::rtl::OUString("TIMESTAMP"));

        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[6] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[7] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();

        aRows.push_back(aRow);
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getExportedKeys(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eExportedKeys );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getImportedKeys(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eImportedKeys );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getPrimaryKeys(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::ePrimaryKeys );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getIndexInfo(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString&,
    sal_Bool, sal_Bool ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eIndexInfo );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getBestRowIdentifier(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString&, sal_Int32,
    sal_Bool ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eBestRowIdentifier );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getTablePrivileges(
    const Any&, const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getCrossReference(
    const Any&, const ::rtl::OUString&,
    const ::rtl::OUString&, const Any&,
    const ::rtl::OUString&, const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCrossReference );
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL KabDatabaseMetaData::getUDTs( const Any&, const ::rtl::OUString&, const ::rtl::OUString&, const Sequence< sal_Int32 >& ) throw(SQLException, RuntimeException)
{
    OSL_FAIL("Not implemented yet!");
    throw SQLException();
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
