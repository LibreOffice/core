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

#pragma once

#include <odbc/OConnection.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <TDatabaseMetaDataBase.hxx>

namespace connectivity::odbc
{

    //************ Class: ODatabaseMetaData


    class ODatabaseMetaData final :
        public ODatabaseMetaDataBase
    {
        SQLHANDLE       m_aConnectionHandle;
        OConnection*    m_pConnection;
        bool            m_bUseCatalog;

        // cached database information
        virtual OUString    impl_getIdentifierQuoteString_throw(  ) override;
        virtual bool        impl_isCatalogAtStart_throw(  ) override;
        virtual OUString    impl_getCatalogSeparator_throw(  ) override;
        virtual bool        impl_supportsCatalogsInTableDefinitions_throw(  ) override;
        virtual bool        impl_supportsSchemasInTableDefinitions_throw(  ) override ;
        virtual bool        impl_supportsCatalogsInDataManipulation_throw(  ) override;
        virtual bool        impl_supportsSchemasInDataManipulation_throw(  ) override ;
        virtual bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  ) override ;
        virtual bool        impl_supportsAlterTableWithAddColumn_throw(  ) override;
        virtual bool        impl_supportsAlterTableWithDropColumn_throw(  ) override;
        virtual sal_Int32   impl_getMaxStatements_throw(  ) override;
        virtual sal_Int32   impl_getMaxTablesInSelect_throw(  ) override;
        virtual bool        impl_storesMixedCaseQuotedIdentifiers_throw(  ) override;
        OUString getURLImpl();
        virtual css::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() override;
        virtual ~ODatabaseMetaData() override;

    public:
        ODatabaseMetaData(const SQLHANDLE _pHandle,OConnection* _pCon);

        // XDatabaseMetaData
        virtual bool SAL_CALL allProceduresAreCallable(  ) override;
        virtual bool SAL_CALL allTablesAreSelectable(  ) override;
        virtual OUString SAL_CALL getURL(  ) override;
        virtual OUString SAL_CALL getUserName(  ) override;
        virtual bool SAL_CALL isReadOnly(  ) override;
        virtual bool SAL_CALL nullsAreSortedHigh(  ) override;
        virtual bool SAL_CALL nullsAreSortedLow(  ) override;
        virtual bool SAL_CALL nullsAreSortedAtStart(  ) override;
        virtual bool SAL_CALL nullsAreSortedAtEnd(  ) override;
        virtual OUString SAL_CALL getDatabaseProductName(  ) override;
        virtual OUString SAL_CALL getDatabaseProductVersion(  ) override;
        virtual OUString SAL_CALL getDriverName(  ) override;
        virtual OUString SAL_CALL getDriverVersion(  ) override;
        virtual sal_Int32 SAL_CALL getDriverMajorVersion(  ) override;
        virtual sal_Int32 SAL_CALL getDriverMinorVersion(  ) override;
        virtual bool SAL_CALL usesLocalFiles(  ) override;
        virtual bool SAL_CALL usesLocalFilePerTable(  ) override;
        virtual bool SAL_CALL supportsMixedCaseIdentifiers(  ) override;
        virtual bool SAL_CALL storesUpperCaseIdentifiers(  ) override;
        virtual bool SAL_CALL storesLowerCaseIdentifiers(  ) override;
        virtual bool SAL_CALL storesMixedCaseIdentifiers(  ) override;

        virtual bool SAL_CALL storesUpperCaseQuotedIdentifiers(  ) override;
        virtual bool SAL_CALL storesLowerCaseQuotedIdentifiers(  ) override;

        virtual OUString SAL_CALL getSQLKeywords(  ) override;
        virtual OUString SAL_CALL getNumericFunctions(  ) override;
        virtual OUString SAL_CALL getStringFunctions(  ) override;
        virtual OUString SAL_CALL getSystemFunctions(  ) override;
        virtual OUString SAL_CALL getTimeDateFunctions(  ) override;
        virtual OUString SAL_CALL getSearchStringEscape(  ) override;
        virtual OUString SAL_CALL getExtraNameCharacters(  ) override;
        virtual bool SAL_CALL supportsColumnAliasing(  ) override;
        virtual bool SAL_CALL nullPlusNonNullIsNull(  ) override;
        virtual bool SAL_CALL supportsTypeConversion(  ) override;
        virtual bool SAL_CALL supportsConvert( sal_Int32 fromType, sal_Int32 toType ) override;
        virtual bool SAL_CALL supportsTableCorrelationNames(  ) override;
        virtual bool SAL_CALL supportsDifferentTableCorrelationNames(  ) override;
        virtual bool SAL_CALL supportsExpressionsInOrderBy(  ) override;
        virtual bool SAL_CALL supportsOrderByUnrelated(  ) override;
        virtual bool SAL_CALL supportsGroupBy(  ) override;
        virtual bool SAL_CALL supportsGroupByUnrelated(  ) override;
        virtual bool SAL_CALL supportsGroupByBeyondSelect(  ) override;
        virtual bool SAL_CALL supportsLikeEscapeClause(  ) override;
        virtual bool SAL_CALL supportsMultipleResultSets(  ) override;
        virtual bool SAL_CALL supportsMultipleTransactions(  ) override;
        virtual bool SAL_CALL supportsNonNullableColumns(  ) override;
        virtual bool SAL_CALL supportsMinimumSQLGrammar(  ) override;
        virtual bool SAL_CALL supportsCoreSQLGrammar(  ) override;
        virtual bool SAL_CALL supportsExtendedSQLGrammar(  ) override;
        virtual bool SAL_CALL supportsANSI92EntryLevelSQL(  ) override;
        virtual bool SAL_CALL supportsANSI92IntermediateSQL(  ) override;
        virtual bool SAL_CALL supportsANSI92FullSQL(  ) override;
        virtual bool SAL_CALL supportsIntegrityEnhancementFacility(  ) override;
        virtual bool SAL_CALL supportsOuterJoins(  ) override;
        virtual bool SAL_CALL supportsFullOuterJoins(  ) override;
        virtual bool SAL_CALL supportsLimitedOuterJoins(  ) override;
        virtual OUString SAL_CALL getSchemaTerm(  ) override;
        virtual OUString SAL_CALL getProcedureTerm(  ) override;
        virtual OUString SAL_CALL getCatalogTerm(  ) override;

        virtual bool SAL_CALL supportsSchemasInProcedureCalls(  ) override;
        virtual bool SAL_CALL supportsSchemasInIndexDefinitions(  ) override;
        virtual bool SAL_CALL supportsSchemasInPrivilegeDefinitions(  ) override;
        virtual bool SAL_CALL supportsCatalogsInProcedureCalls(  ) override;

        virtual bool SAL_CALL supportsCatalogsInIndexDefinitions(  ) override;
        virtual bool SAL_CALL supportsCatalogsInPrivilegeDefinitions(  ) override;
        virtual bool SAL_CALL supportsPositionedDelete(  ) override;
        virtual bool SAL_CALL supportsPositionedUpdate(  ) override;
        virtual bool SAL_CALL supportsSelectForUpdate(  ) override;
        virtual bool SAL_CALL supportsStoredProcedures(  ) override;
        virtual bool SAL_CALL supportsSubqueriesInComparisons(  ) override;
        virtual bool SAL_CALL supportsSubqueriesInExists(  ) override;
        virtual bool SAL_CALL supportsSubqueriesInIns(  ) override;
        virtual bool SAL_CALL supportsSubqueriesInQuantifieds(  ) override;
        virtual bool SAL_CALL supportsCorrelatedSubqueries(  ) override;
        virtual bool SAL_CALL supportsUnion(  ) override;
        virtual bool SAL_CALL supportsUnionAll(  ) override;
        virtual bool SAL_CALL supportsOpenCursorsAcrossCommit(  ) override;
        virtual bool SAL_CALL supportsOpenCursorsAcrossRollback(  ) override;
        virtual bool SAL_CALL supportsOpenStatementsAcrossCommit(  ) override;
        virtual bool SAL_CALL supportsOpenStatementsAcrossRollback(  ) override;
        virtual sal_Int32 SAL_CALL getMaxBinaryLiteralLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxCharLiteralLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnsInGroupBy(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnsInIndex(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnsInOrderBy(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnsInSelect(  ) override;
        virtual sal_Int32 SAL_CALL getMaxColumnsInTable(  ) override;
        virtual sal_Int32 SAL_CALL getMaxConnections(  ) override;
        virtual sal_Int32 SAL_CALL getMaxCursorNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxIndexLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxSchemaNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxProcedureNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxCatalogNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxRowSize(  ) override;
        virtual bool SAL_CALL doesMaxRowSizeIncludeBlobs(  ) override;
        virtual sal_Int32 SAL_CALL getMaxStatementLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxTableNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxUserNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getDefaultTransactionIsolation(  ) override;
        virtual bool SAL_CALL supportsTransactions(  ) override;
        virtual bool SAL_CALL supportsTransactionIsolationLevel( sal_Int32 level ) override;
        virtual bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions(  ) override;
        virtual bool SAL_CALL supportsDataManipulationTransactionsOnly(  ) override;
        virtual bool SAL_CALL dataDefinitionCausesTransactionCommit(  ) override;
        virtual bool SAL_CALL dataDefinitionIgnoredInTransactions(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedures( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedureColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTables( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getSchemas(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCatalogs(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTableTypes(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTablePrivileges( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, bool nullable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getVersionColumns( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getImportedKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getExportedKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCrossReference( const cpo::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const cpo::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getIndexInfo( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, bool unique, bool approximate ) override;
        virtual bool SAL_CALL supportsResultSetType( sal_Int32 setType ) override;
        virtual bool SAL_CALL supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) override;
        virtual bool SAL_CALL ownUpdatesAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL ownDeletesAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL ownInsertsAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL othersUpdatesAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL othersDeletesAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL othersInsertsAreVisible( sal_Int32 setType ) override;
        virtual bool SAL_CALL updatesAreDetected( sal_Int32 setType ) override;
        virtual bool SAL_CALL deletesAreDetected( sal_Int32 setType ) override;
        virtual bool SAL_CALL insertsAreDetected( sal_Int32 setType ) override;
        virtual bool SAL_CALL supportsBatchUpdates(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getUDTs( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const css::uno::Sequence< sal_Int32 >& types ) override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
