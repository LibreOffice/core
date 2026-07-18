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

#include <TDatabaseMetaDataBase.hxx>
#include <file/FConnection.hxx>
#include <file/filedllapi.hxx>

namespace connectivity::file
{

    //************ Class: ODatabaseMetaData


    class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE ODatabaseMetaData :
        public  ODatabaseMetaDataBase
    {
        virtual css::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() override;
        // cached database information
        virtual OUString    impl_getIdentifierQuoteString_throw(  ) override;
        virtual bool        impl_isCatalogAtStart_throw(  ) override;
        virtual OUString    impl_getCatalogSeparator_throw(  ) override;
        virtual bool        impl_supportsCatalogsInTableDefinitions_throw(  ) override;
        virtual bool        impl_supportsSchemasInTableDefinitions_throw(  ) override ;
        virtual bool        impl_supportsCatalogsInDataManipulation_throw(  ) override;
        virtual bool        impl_supportsSchemasInDataManipulation_throw(  ) override ;
        virtual bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  ) override;
        virtual bool        impl_supportsAlterTableWithAddColumn_throw(  ) override;
        virtual bool        impl_supportsAlterTableWithDropColumn_throw(  ) override;
        virtual sal_Int32   impl_getMaxStatements_throw(  ) override;
        virtual sal_Int32   impl_getMaxTablesInSelect_throw(  ) override;
        virtual bool        impl_storesMixedCaseQuotedIdentifiers_throw(  ) override;
    protected:
        OConnection* m_pConnection; // I need the native class not only the interface
        virtual ~ODatabaseMetaData() override;
    public:

        ODatabaseMetaData(OConnection* _pCon);
        // XDatabaseMetaData
        virtual bool allProceduresAreCallable(  ) override;
        virtual bool allTablesAreSelectable(  ) override;
        virtual OUString getURL(  ) override;
        virtual OUString getUserName(  ) override;
        virtual bool isReadOnly(  ) override;
        virtual bool nullsAreSortedHigh(  ) override;
        virtual bool nullsAreSortedLow(  ) override;
        virtual bool nullsAreSortedAtStart(  ) override;
        virtual bool nullsAreSortedAtEnd(  ) override;
        virtual OUString getDatabaseProductName(  ) override;
        virtual OUString getDatabaseProductVersion(  ) override;
        virtual OUString getDriverName(  ) override;
        virtual OUString getDriverVersion(  ) override;
        virtual sal_Int32 getDriverMajorVersion(  ) override;
        virtual sal_Int32 getDriverMinorVersion(  ) override;
        virtual bool usesLocalFiles(  ) override;
        virtual bool usesLocalFilePerTable(  ) override;
        virtual bool supportsMixedCaseIdentifiers(  ) override;
        virtual bool storesUpperCaseIdentifiers(  ) override;
        virtual bool storesLowerCaseIdentifiers(  ) override;
        virtual bool storesMixedCaseIdentifiers(  ) override;
        virtual bool storesUpperCaseQuotedIdentifiers(  ) override;
        virtual bool storesLowerCaseQuotedIdentifiers(  ) override;
        virtual OUString getSQLKeywords(  ) override;
        virtual OUString getNumericFunctions(  ) override;
        virtual OUString getStringFunctions(  ) override;
        virtual OUString getSystemFunctions(  ) override;
        virtual OUString getTimeDateFunctions(  ) override;
        virtual OUString getSearchStringEscape(  ) override;
        virtual OUString getExtraNameCharacters(  ) override;
        virtual bool supportsColumnAliasing(  ) override;
        virtual bool nullPlusNonNullIsNull(  ) override;
        virtual bool supportsTypeConversion(  ) override;
        virtual bool supportsConvert( sal_Int32 fromType, sal_Int32 toType ) override;
        virtual bool supportsTableCorrelationNames(  ) override;
        virtual bool supportsDifferentTableCorrelationNames(  ) override;
        virtual bool supportsExpressionsInOrderBy(  ) override;
        virtual bool supportsOrderByUnrelated(  ) override;
        virtual bool supportsGroupBy(  ) override;
        virtual bool supportsGroupByUnrelated(  ) override;
        virtual bool supportsGroupByBeyondSelect(  ) override;
        virtual bool supportsLikeEscapeClause(  ) override;
        virtual bool supportsMultipleResultSets(  ) override;
        virtual bool supportsMultipleTransactions(  ) override;
        virtual bool supportsNonNullableColumns(  ) override;
        virtual bool supportsMinimumSQLGrammar(  ) override;
        virtual bool supportsCoreSQLGrammar(  ) override;
        virtual bool supportsExtendedSQLGrammar(  ) override;
        virtual bool supportsANSI92EntryLevelSQL(  ) override;
        virtual bool supportsANSI92IntermediateSQL(  ) override;
        virtual bool supportsANSI92FullSQL(  ) override;
        virtual bool supportsIntegrityEnhancementFacility(  ) override;
        virtual bool supportsOuterJoins(  ) override;
        virtual bool supportsFullOuterJoins(  ) override;
        virtual bool supportsLimitedOuterJoins(  ) override;
        virtual OUString getSchemaTerm(  ) override;
        virtual OUString getProcedureTerm(  ) override;
        virtual OUString getCatalogTerm(  ) override;
        virtual bool supportsSchemasInProcedureCalls(  ) override;
        virtual bool supportsSchemasInIndexDefinitions(  ) override;
        virtual bool supportsSchemasInPrivilegeDefinitions(  ) override;
        virtual bool supportsCatalogsInProcedureCalls(  ) override;
        virtual bool supportsCatalogsInIndexDefinitions(  ) override;
        virtual bool supportsCatalogsInPrivilegeDefinitions(  ) override;
        virtual bool supportsPositionedDelete(  ) override;
        virtual bool supportsPositionedUpdate(  ) override;
        virtual bool supportsSelectForUpdate(  ) override;
        virtual bool supportsStoredProcedures(  ) override;
        virtual bool supportsSubqueriesInComparisons(  ) override;
        virtual bool supportsSubqueriesInExists(  ) override;
        virtual bool supportsSubqueriesInIns(  ) override;
        virtual bool supportsSubqueriesInQuantifieds(  ) override;
        virtual bool supportsCorrelatedSubqueries(  ) override;
        virtual bool supportsUnion(  ) override;
        virtual bool supportsUnionAll(  ) override;
        virtual bool supportsOpenCursorsAcrossCommit(  ) override;
        virtual bool supportsOpenCursorsAcrossRollback(  ) override;
        virtual bool supportsOpenStatementsAcrossCommit(  ) override;
        virtual bool supportsOpenStatementsAcrossRollback(  ) override;
        virtual sal_Int32 getMaxBinaryLiteralLength(  ) override;
        virtual sal_Int32 getMaxCharLiteralLength(  ) override;
        virtual sal_Int32 getMaxColumnNameLength(  ) override;
        virtual sal_Int32 getMaxColumnsInGroupBy(  ) override;
        virtual sal_Int32 getMaxColumnsInIndex(  ) override;
        virtual sal_Int32 getMaxColumnsInOrderBy(  ) override;
        virtual sal_Int32 getMaxColumnsInSelect(  ) override;
        virtual sal_Int32 getMaxColumnsInTable(  ) override;
        virtual sal_Int32 getMaxConnections(  ) override;
        virtual sal_Int32 getMaxCursorNameLength(  ) override;
        virtual sal_Int32 getMaxIndexLength(  ) override;
        virtual sal_Int32 getMaxSchemaNameLength(  ) override;
        virtual sal_Int32 getMaxProcedureNameLength(  ) override;
        virtual sal_Int32 getMaxCatalogNameLength(  ) override;
        virtual sal_Int32 getMaxRowSize(  ) override;
        virtual bool doesMaxRowSizeIncludeBlobs(  ) override;
        virtual sal_Int32 getMaxStatementLength(  ) override;
        virtual sal_Int32 getMaxTableNameLength(  ) override;
        virtual sal_Int32 getMaxUserNameLength(  ) override;
        virtual sal_Int32 getDefaultTransactionIsolation(  ) override;
        virtual bool supportsTransactions(  ) override;
        virtual bool supportsTransactionIsolationLevel( sal_Int32 level ) override;
        virtual bool supportsDataDefinitionAndDataManipulationTransactions(  ) override;
        virtual bool supportsDataManipulationTransactionsOnly(  ) override;
        virtual bool dataDefinitionCausesTransactionCommit(  ) override;
        virtual bool dataDefinitionIgnoredInTransactions(  ) override;

        virtual css::uno::Reference< css::sdbc::XResultSet > getTables( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const cpo::uno::Sequence< OUString >& types ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getTableTypes(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getTablePrivileges( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) override;

        virtual bool supportsResultSetType( sal_Int32 setType ) override;
        virtual bool supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) override;
        virtual bool ownUpdatesAreVisible( sal_Int32 setType ) override;
        virtual bool ownDeletesAreVisible( sal_Int32 setType ) override;
        virtual bool ownInsertsAreVisible( sal_Int32 setType ) override;
        virtual bool othersUpdatesAreVisible( sal_Int32 setType ) override;
        virtual bool othersDeletesAreVisible( sal_Int32 setType ) override;
        virtual bool othersInsertsAreVisible( sal_Int32 setType ) override;
        virtual bool updatesAreDetected( sal_Int32 setType ) override;
        virtual bool deletesAreDetected( sal_Int32 setType ) override;
        virtual bool insertsAreDetected( sal_Int32 setType ) override;
        virtual bool supportsBatchUpdates(  ) override;

        virtual css::uno::Reference< css::sdbc::XResultSet > getUDTs( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const cpo::uno::Sequence< sal_Int32 >& types ) override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
