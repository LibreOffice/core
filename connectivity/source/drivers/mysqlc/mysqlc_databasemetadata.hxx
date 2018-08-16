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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_DATABASEMETADATA_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_DATABASEMETADATA_HXX

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <cppuhelper/implbase1.hxx>

namespace connectivity
{
namespace mysqlc
{
using ::com::sun::star::uno::Any;

//************ Class: ODatabaseMetaData

typedef ::cppu::WeakImplHelper1<css::sdbc::XDatabaseMetaData> ODatabaseMetaData_BASE;

class ODatabaseMetaData final : public ODatabaseMetaData_BASE
{
    OConnection& m_rConnection;
    MYSQL* m_pMySql;
    rtl::OUString identifier_quote_string;

public:
    const OConnection& getOwnConnection() const { return m_rConnection; }

    explicit ODatabaseMetaData(OConnection& _rCon, MYSQL* pMySql);
    virtual ~ODatabaseMetaData() override;

    // as I mentioned before this interface is really BIG
    // XDatabaseMetaData
    sal_Bool SAL_CALL allProceduresAreCallable() SAL_OVERRIDE;
    sal_Bool SAL_CALL allTablesAreSelectable() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getURL() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getUserName() SAL_OVERRIDE;
    sal_Bool SAL_CALL isReadOnly() SAL_OVERRIDE;
    sal_Bool SAL_CALL nullsAreSortedHigh() SAL_OVERRIDE;
    sal_Bool SAL_CALL nullsAreSortedLow() SAL_OVERRIDE;
    sal_Bool SAL_CALL nullsAreSortedAtStart() SAL_OVERRIDE;
    sal_Bool SAL_CALL nullsAreSortedAtEnd() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getDatabaseProductName() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getDatabaseProductVersion() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getDriverName() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getDriverVersion() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getDriverMajorVersion() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getDriverMinorVersion() SAL_OVERRIDE;
    sal_Bool SAL_CALL usesLocalFiles() SAL_OVERRIDE;
    sal_Bool SAL_CALL usesLocalFilePerTable() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsMixedCaseIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesUpperCaseIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesLowerCaseIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesMixedCaseIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers() SAL_OVERRIDE;
    sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getIdentifierQuoteString() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getSQLKeywords() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getNumericFunctions() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getStringFunctions() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getSystemFunctions() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getTimeDateFunctions() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getSearchStringEscape() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getExtraNameCharacters() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsAlterTableWithAddColumn() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsAlterTableWithDropColumn() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsColumnAliasing() SAL_OVERRIDE;
    sal_Bool SAL_CALL nullPlusNonNullIsNull() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsTypeConversion() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsConvert(sal_Int32 fromType, sal_Int32 toType) SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsTableCorrelationNames() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsDifferentTableCorrelationNames() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsExpressionsInOrderBy() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOrderByUnrelated() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsGroupBy() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsGroupByUnrelated() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsGroupByBeyondSelect() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsLikeEscapeClause() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsMultipleResultSets() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsMultipleTransactions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsNonNullableColumns() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsMinimumSQLGrammar() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCoreSQLGrammar() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsExtendedSQLGrammar() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsANSI92EntryLevelSQL() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsANSI92IntermediateSQL() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsANSI92FullSQL() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsIntegrityEnhancementFacility() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOuterJoins() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsFullOuterJoins() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsLimitedOuterJoins() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getSchemaTerm() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getProcedureTerm() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getCatalogTerm() SAL_OVERRIDE;
    sal_Bool SAL_CALL isCatalogAtStart() SAL_OVERRIDE;
    rtl::OUString SAL_CALL getCatalogSeparator() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSchemasInDataManipulation() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSchemasInProcedureCalls() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSchemasInTableDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSchemasInIndexDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCatalogsInDataManipulation() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCatalogsInProcedureCalls() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCatalogsInTableDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsPositionedDelete() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsPositionedUpdate() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSelectForUpdate() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsStoredProcedures() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSubqueriesInComparisons() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSubqueriesInExists() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSubqueriesInIns() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsSubqueriesInQuantifieds() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsCorrelatedSubqueries() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsUnion() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsUnionAll() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxBinaryLiteralLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxCharLiteralLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnsInGroupBy() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnsInIndex() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnsInOrderBy() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnsInSelect() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxColumnsInTable() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxConnections() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxCursorNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxIndexLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxSchemaNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxProcedureNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxCatalogNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxRowSize() SAL_OVERRIDE;
    sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxStatementLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxStatements() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxTableNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxTablesInSelect() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getMaxUserNameLength() SAL_OVERRIDE;
    sal_Int32 SAL_CALL getDefaultTransactionIsolation() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsTransactions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsTransactionIsolationLevel(sal_Int32 level) SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions() SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly() SAL_OVERRIDE;
    sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit() SAL_OVERRIDE;
    sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getProcedures(const Any& catalog, const rtl::OUString& schemaPattern,
                               const rtl::OUString& procedureNamePattern) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getProcedureColumns(const Any& catalog, const rtl::OUString& schemaPattern,
                                     const rtl::OUString& procedureNamePattern,
                                     const rtl::OUString& columnNamePattern) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getTables(const Any& catalog, const rtl::OUString& schemaPattern,
                           const rtl::OUString& tableNamePattern,
                           const css::uno::Sequence<rtl::OUString>& types) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getSchemas() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getCatalogs() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getTableTypes() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getColumns(const Any& catalog, const rtl::OUString& schemaPattern,
                            const rtl::OUString& tableNamePattern,
                            const rtl::OUString& columnNamePattern) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getColumnPrivileges(const Any& catalog, const rtl::OUString& schema,
                                     const rtl::OUString& table,
                                     const rtl::OUString& columnNamePattern) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getTablePrivileges(const Any& catalog, const rtl::OUString& schemaPattern,
                                    const rtl::OUString& tableNamePattern) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getBestRowIdentifier(const Any& catalog, const rtl::OUString& schema,
                                      const rtl::OUString& table, sal_Int32 scope,
                                      sal_Bool nullable) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getVersionColumns(const Any& catalog, const rtl::OUString& schema,
                                   const rtl::OUString& table) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getPrimaryKeys(const Any& catalog, const rtl::OUString& schema,
                                const rtl::OUString& table) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getImportedKeys(const Any& catalog, const rtl::OUString& schema,
                                 const rtl::OUString& table) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getExportedKeys(const Any& catalog, const rtl::OUString& schema,
                                 const rtl::OUString& table) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getCrossReference(const Any& primaryCatalog, const rtl::OUString& primarySchema,
                                   const rtl::OUString& primaryTable, const Any& foreignCatalog,
                                   const rtl::OUString& foreignSchema,
                                   const rtl::OUString& foreignTable) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getTypeInfo() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getIndexInfo(const Any& catalog, const rtl::OUString& schema,
                              const rtl::OUString& table, sal_Bool unique,
                              sal_Bool approximate) SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsResultSetType(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsResultSetConcurrency(sal_Int32 setType,
                                                   sal_Int32 concurrency) SAL_OVERRIDE;
    sal_Bool SAL_CALL ownUpdatesAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL ownDeletesAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL ownInsertsAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL othersUpdatesAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL othersDeletesAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL othersInsertsAreVisible(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL updatesAreDetected(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL deletesAreDetected(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL insertsAreDetected(sal_Int32 setType) SAL_OVERRIDE;
    sal_Bool SAL_CALL supportsBatchUpdates() SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getUDTs(const Any& catalog, const rtl::OUString& schemaPattern,
                         const rtl::OUString& typeNamePattern,
                         const css::uno::Sequence<sal_Int32>& types) SAL_OVERRIDE;
    css::uno::Reference<css::sdbc::XConnection> SAL_CALL getConnection() SAL_OVERRIDE;
};
}
}

#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_DATABASEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
