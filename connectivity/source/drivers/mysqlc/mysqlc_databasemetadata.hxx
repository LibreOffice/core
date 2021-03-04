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

#pragma once

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <cppuhelper/implbase1.hxx>

namespace connectivity::mysqlc
{
using ::com::sun::star::uno::Any;

//************ Class: ODatabaseMetaData

typedef ::cppu::WeakImplHelper1<css::sdbc::XDatabaseMetaData> ODatabaseMetaData_BASE;

class ODatabaseMetaData final : public ODatabaseMetaData_BASE
{
    OConnection& m_rConnection;
    MYSQL* m_pMySql;

public:
    const OConnection& getOwnConnection() const { return m_rConnection; }

    explicit ODatabaseMetaData(OConnection& _rCon, MYSQL* pMySql);
    virtual ~ODatabaseMetaData() override;

    // as I mentioned before this interface is really BIG
    // XDatabaseMetaData
    sal_Bool SAL_CALL allProceduresAreCallable() override;
    sal_Bool SAL_CALL allTablesAreSelectable() override;
    OUString SAL_CALL getURL() override;
    OUString SAL_CALL getUserName() override;
    sal_Bool SAL_CALL isReadOnly() override;
    sal_Bool SAL_CALL nullsAreSortedHigh() override;
    sal_Bool SAL_CALL nullsAreSortedLow() override;
    sal_Bool SAL_CALL nullsAreSortedAtStart() override;
    sal_Bool SAL_CALL nullsAreSortedAtEnd() override;
    OUString SAL_CALL getDatabaseProductName() override;
    OUString SAL_CALL getDatabaseProductVersion() override;
    OUString SAL_CALL getDriverName() override;
    OUString SAL_CALL getDriverVersion() override;
    sal_Int32 SAL_CALL getDriverMajorVersion() override;
    sal_Int32 SAL_CALL getDriverMinorVersion() override;
    sal_Bool SAL_CALL usesLocalFiles() override;
    sal_Bool SAL_CALL usesLocalFilePerTable() override;
    sal_Bool SAL_CALL supportsMixedCaseIdentifiers() override;
    sal_Bool SAL_CALL storesUpperCaseIdentifiers() override;
    sal_Bool SAL_CALL storesLowerCaseIdentifiers() override;
    sal_Bool SAL_CALL storesMixedCaseIdentifiers() override;
    sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers() override;
    sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers() override;
    sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers() override;
    sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers() override;
    OUString SAL_CALL getIdentifierQuoteString() override;
    OUString SAL_CALL getSQLKeywords() override;
    OUString SAL_CALL getNumericFunctions() override;
    OUString SAL_CALL getStringFunctions() override;
    OUString SAL_CALL getSystemFunctions() override;
    OUString SAL_CALL getTimeDateFunctions() override;
    OUString SAL_CALL getSearchStringEscape() override;
    OUString SAL_CALL getExtraNameCharacters() override;
    sal_Bool SAL_CALL supportsAlterTableWithAddColumn() override;
    sal_Bool SAL_CALL supportsAlterTableWithDropColumn() override;
    sal_Bool SAL_CALL supportsColumnAliasing() override;
    sal_Bool SAL_CALL nullPlusNonNullIsNull() override;
    sal_Bool SAL_CALL supportsTypeConversion() override;
    sal_Bool SAL_CALL supportsConvert(sal_Int32 fromType, sal_Int32 toType) override;
    sal_Bool SAL_CALL supportsTableCorrelationNames() override;
    sal_Bool SAL_CALL supportsDifferentTableCorrelationNames() override;
    sal_Bool SAL_CALL supportsExpressionsInOrderBy() override;
    sal_Bool SAL_CALL supportsOrderByUnrelated() override;
    sal_Bool SAL_CALL supportsGroupBy() override;
    sal_Bool SAL_CALL supportsGroupByUnrelated() override;
    sal_Bool SAL_CALL supportsGroupByBeyondSelect() override;
    sal_Bool SAL_CALL supportsLikeEscapeClause() override;
    sal_Bool SAL_CALL supportsMultipleResultSets() override;
    sal_Bool SAL_CALL supportsMultipleTransactions() override;
    sal_Bool SAL_CALL supportsNonNullableColumns() override;
    sal_Bool SAL_CALL supportsMinimumSQLGrammar() override;
    sal_Bool SAL_CALL supportsCoreSQLGrammar() override;
    sal_Bool SAL_CALL supportsExtendedSQLGrammar() override;
    sal_Bool SAL_CALL supportsANSI92EntryLevelSQL() override;
    sal_Bool SAL_CALL supportsANSI92IntermediateSQL() override;
    sal_Bool SAL_CALL supportsANSI92FullSQL() override;
    sal_Bool SAL_CALL supportsIntegrityEnhancementFacility() override;
    sal_Bool SAL_CALL supportsOuterJoins() override;
    sal_Bool SAL_CALL supportsFullOuterJoins() override;
    sal_Bool SAL_CALL supportsLimitedOuterJoins() override;
    OUString SAL_CALL getSchemaTerm() override;
    OUString SAL_CALL getProcedureTerm() override;
    OUString SAL_CALL getCatalogTerm() override;
    sal_Bool SAL_CALL isCatalogAtStart() override;
    OUString SAL_CALL getCatalogSeparator() override;
    sal_Bool SAL_CALL supportsSchemasInDataManipulation() override;
    sal_Bool SAL_CALL supportsSchemasInProcedureCalls() override;
    sal_Bool SAL_CALL supportsSchemasInTableDefinitions() override;
    sal_Bool SAL_CALL supportsSchemasInIndexDefinitions() override;
    sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions() override;
    sal_Bool SAL_CALL supportsCatalogsInDataManipulation() override;
    sal_Bool SAL_CALL supportsCatalogsInProcedureCalls() override;
    sal_Bool SAL_CALL supportsCatalogsInTableDefinitions() override;
    sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions() override;
    sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions() override;
    sal_Bool SAL_CALL supportsPositionedDelete() override;
    sal_Bool SAL_CALL supportsPositionedUpdate() override;
    sal_Bool SAL_CALL supportsSelectForUpdate() override;
    sal_Bool SAL_CALL supportsStoredProcedures() override;
    sal_Bool SAL_CALL supportsSubqueriesInComparisons() override;
    sal_Bool SAL_CALL supportsSubqueriesInExists() override;
    sal_Bool SAL_CALL supportsSubqueriesInIns() override;
    sal_Bool SAL_CALL supportsSubqueriesInQuantifieds() override;
    sal_Bool SAL_CALL supportsCorrelatedSubqueries() override;
    sal_Bool SAL_CALL supportsUnion() override;
    sal_Bool SAL_CALL supportsUnionAll() override;
    sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit() override;
    sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback() override;
    sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit() override;
    sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback() override;
    sal_Int32 SAL_CALL getMaxBinaryLiteralLength() override;
    sal_Int32 SAL_CALL getMaxCharLiteralLength() override;
    sal_Int32 SAL_CALL getMaxColumnNameLength() override;
    sal_Int32 SAL_CALL getMaxColumnsInGroupBy() override;
    sal_Int32 SAL_CALL getMaxColumnsInIndex() override;
    sal_Int32 SAL_CALL getMaxColumnsInOrderBy() override;
    sal_Int32 SAL_CALL getMaxColumnsInSelect() override;
    sal_Int32 SAL_CALL getMaxColumnsInTable() override;
    sal_Int32 SAL_CALL getMaxConnections() override;
    sal_Int32 SAL_CALL getMaxCursorNameLength() override;
    sal_Int32 SAL_CALL getMaxIndexLength() override;
    sal_Int32 SAL_CALL getMaxSchemaNameLength() override;
    sal_Int32 SAL_CALL getMaxProcedureNameLength() override;
    sal_Int32 SAL_CALL getMaxCatalogNameLength() override;
    sal_Int32 SAL_CALL getMaxRowSize() override;
    sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs() override;
    sal_Int32 SAL_CALL getMaxStatementLength() override;
    sal_Int32 SAL_CALL getMaxStatements() override;
    sal_Int32 SAL_CALL getMaxTableNameLength() override;
    sal_Int32 SAL_CALL getMaxTablesInSelect() override;
    sal_Int32 SAL_CALL getMaxUserNameLength() override;
    sal_Int32 SAL_CALL getDefaultTransactionIsolation() override;
    sal_Bool SAL_CALL supportsTransactions() override;
    sal_Bool SAL_CALL supportsTransactionIsolationLevel(sal_Int32 level) override;
    sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions() override;
    sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly() override;
    sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit() override;
    sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions() override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getProcedures(const Any& catalog, const OUString& schemaPattern,
                               const OUString& procedureNamePattern) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getProcedureColumns(const Any& catalog, const OUString& schemaPattern,
                                     const OUString& procedureNamePattern,
                                     const OUString& columnNamePattern) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getTables(const Any& catalog, const OUString& schemaPattern,
                           const OUString& tableNamePattern,
                           const css::uno::Sequence<OUString>& types) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getSchemas() override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getCatalogs() override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getTableTypes() override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getColumns(const Any& catalog, const OUString& schemaPattern,
                            const OUString& tableNamePattern,
                            const OUString& columnNamePattern) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getColumnPrivileges(const Any& catalog, const OUString& schema,
                                     const OUString& table,
                                     const OUString& columnNamePattern) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getTablePrivileges(const Any& catalog, const OUString& schemaPattern,
                                    const OUString& tableNamePattern) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getBestRowIdentifier(const Any& catalog, const OUString& schema,
                                      const OUString& table, sal_Int32 scope,
                                      sal_Bool nullable) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL
    getVersionColumns(const Any& catalog, const OUString& schema, const OUString& table) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL
    getPrimaryKeys(const Any& catalog, const OUString& schema, const OUString& table) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL
    getImportedKeys(const Any& catalog, const OUString& schema, const OUString& table) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL
    getExportedKeys(const Any& catalog, const OUString& schema, const OUString& table) override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getCrossReference(const Any& primaryCatalog, const OUString& primarySchema,
                                   const OUString& primaryTable, const Any& foreignCatalog,
                                   const OUString& foreignSchema,
                                   const OUString& foreignTable) override;
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getTypeInfo() override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getIndexInfo(const Any& catalog, const OUString& schema, const OUString& table,
                              sal_Bool unique, sal_Bool approximate) override;
    sal_Bool SAL_CALL supportsResultSetType(sal_Int32 setType) override;
    sal_Bool SAL_CALL supportsResultSetConcurrency(sal_Int32 setType,
                                                   sal_Int32 concurrency) override;
    sal_Bool SAL_CALL ownUpdatesAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL ownDeletesAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL ownInsertsAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL othersUpdatesAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL othersDeletesAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL othersInsertsAreVisible(sal_Int32 setType) override;
    sal_Bool SAL_CALL updatesAreDetected(sal_Int32 setType) override;
    sal_Bool SAL_CALL deletesAreDetected(sal_Int32 setType) override;
    sal_Bool SAL_CALL insertsAreDetected(sal_Int32 setType) override;
    sal_Bool SAL_CALL supportsBatchUpdates() override;
    css::uno::Reference<css::sdbc::XResultSet>
        SAL_CALL getUDTs(const Any& catalog, const OUString& schemaPattern,
                         const OUString& typeNamePattern,
                         const css::uno::Sequence<sal_Int32>& types) override;
    css::uno::Reference<css::sdbc::XConnection> SAL_CALL getConnection() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
