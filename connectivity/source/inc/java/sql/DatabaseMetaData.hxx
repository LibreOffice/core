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

#include <java/lang/Object.hxx>
#include <TDatabaseMetaDataBase.hxx>

#include <java/sql/ConnectionLog.hxx>

namespace connectivity
{
    class java_sql_Connection;

    //************ Class: java.sql.DatabaseMetaDataDate


    class java_sql_DatabaseMetaData :   public ODatabaseMetaDataBase,
                                        public java_lang_Object
    {
        java_sql_Connection*        m_pConnection;
        java::sql::ConnectionLog    m_aLogger;

    // Static data for the class
        static jclass theClass;

    public:
        virtual jclass getMyClass() const override;
        virtual ~java_sql_DatabaseMetaData() override;
        // A ctor that is needed for returning the object
        java_sql_DatabaseMetaData( JNIEnv * pEnv, jobject myObj, java_sql_Connection& _rConnection );

    private:

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

        virtual sal_Bool SAL_CALL allProceduresAreCallable(  ) override;
        virtual sal_Bool SAL_CALL allTablesAreSelectable(  ) override;
        virtual OUString SAL_CALL getURL(  ) override;
        virtual OUString SAL_CALL getUserName(  ) override;
        virtual sal_Bool SAL_CALL isReadOnly(  ) override;
        virtual sal_Bool SAL_CALL nullsAreSortedHigh(  ) override;
        virtual sal_Bool SAL_CALL nullsAreSortedLow(  ) override;
        virtual sal_Bool SAL_CALL nullsAreSortedAtStart(  ) override;
        virtual sal_Bool SAL_CALL nullsAreSortedAtEnd(  ) override;
        virtual OUString SAL_CALL getDatabaseProductName(  ) override;
        virtual OUString SAL_CALL getDatabaseProductVersion(  ) override;
        virtual OUString SAL_CALL getDriverName(  ) override;
        virtual OUString SAL_CALL getDriverVersion(  ) override;
        virtual sal_Int32 SAL_CALL getDriverMajorVersion(  ) override;
        virtual sal_Int32 SAL_CALL getDriverMinorVersion(  ) override;
        virtual sal_Bool SAL_CALL usesLocalFiles(  ) override;
        virtual sal_Bool SAL_CALL usesLocalFilePerTable(  ) override;
        virtual sal_Bool SAL_CALL supportsMixedCaseIdentifiers(  ) override;
        virtual sal_Bool SAL_CALL storesUpperCaseIdentifiers(  ) override;
        virtual sal_Bool SAL_CALL storesLowerCaseIdentifiers(  ) override;
        virtual sal_Bool SAL_CALL storesMixedCaseIdentifiers(  ) override;
        virtual sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers(  ) override;
        virtual sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers(  ) override;
        virtual OUString SAL_CALL getSQLKeywords(  ) override;
        virtual OUString SAL_CALL getNumericFunctions(  ) override;
        virtual OUString SAL_CALL getStringFunctions(  ) override;
        virtual OUString SAL_CALL getSystemFunctions(  ) override;
        virtual OUString SAL_CALL getTimeDateFunctions(  ) override;
        virtual OUString SAL_CALL getSearchStringEscape(  ) override;
        virtual OUString SAL_CALL getExtraNameCharacters(  ) override;
        virtual sal_Bool SAL_CALL supportsColumnAliasing(  ) override;
        virtual sal_Bool SAL_CALL nullPlusNonNullIsNull(  ) override;
        virtual sal_Bool SAL_CALL supportsTypeConversion(  ) override;
        virtual sal_Bool SAL_CALL supportsConvert( sal_Int32 fromType, sal_Int32 toType ) override;
        virtual sal_Bool SAL_CALL supportsTableCorrelationNames(  ) override;
        virtual sal_Bool SAL_CALL supportsDifferentTableCorrelationNames(  ) override;
        virtual sal_Bool SAL_CALL supportsExpressionsInOrderBy(  ) override;
        virtual sal_Bool SAL_CALL supportsOrderByUnrelated(  ) override;
        virtual sal_Bool SAL_CALL supportsGroupBy(  ) override;
        virtual sal_Bool SAL_CALL supportsGroupByUnrelated(  ) override;
        virtual sal_Bool SAL_CALL supportsGroupByBeyondSelect(  ) override;
        virtual sal_Bool SAL_CALL supportsLikeEscapeClause(  ) override;
        virtual sal_Bool SAL_CALL supportsMultipleResultSets(  ) override;
        virtual sal_Bool SAL_CALL supportsMultipleTransactions(  ) override;
        virtual sal_Bool SAL_CALL supportsNonNullableColumns(  ) override;
        virtual sal_Bool SAL_CALL supportsMinimumSQLGrammar(  ) override;
        virtual sal_Bool SAL_CALL supportsCoreSQLGrammar(  ) override;
        virtual sal_Bool SAL_CALL supportsExtendedSQLGrammar(  ) override;
        virtual sal_Bool SAL_CALL supportsANSI92EntryLevelSQL(  ) override;
        virtual sal_Bool SAL_CALL supportsANSI92IntermediateSQL(  ) override;
        virtual sal_Bool SAL_CALL supportsANSI92FullSQL(  ) override;
        virtual sal_Bool SAL_CALL supportsIntegrityEnhancementFacility(  ) override;
        virtual sal_Bool SAL_CALL supportsOuterJoins(  ) override;
        virtual sal_Bool SAL_CALL supportsFullOuterJoins(  ) override;
        virtual sal_Bool SAL_CALL supportsLimitedOuterJoins(  ) override;
        virtual OUString SAL_CALL getSchemaTerm(  ) override;
        virtual OUString SAL_CALL getProcedureTerm(  ) override;
        virtual OUString SAL_CALL getCatalogTerm(  ) override;
        virtual sal_Bool SAL_CALL supportsSchemasInProcedureCalls(  ) override;
        virtual sal_Bool SAL_CALL supportsSchemasInIndexDefinitions(  ) override;
        virtual sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions(  ) override;
        virtual sal_Bool SAL_CALL supportsCatalogsInProcedureCalls(  ) override;
        virtual sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions(  ) override;
        virtual sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions(  ) override;
        virtual sal_Bool SAL_CALL supportsPositionedDelete(  ) override;
        virtual sal_Bool SAL_CALL supportsPositionedUpdate(  ) override;
        virtual sal_Bool SAL_CALL supportsSelectForUpdate(  ) override;
        virtual sal_Bool SAL_CALL supportsStoredProcedures(  ) override;
        virtual sal_Bool SAL_CALL supportsSubqueriesInComparisons(  ) override;
        virtual sal_Bool SAL_CALL supportsSubqueriesInExists(  ) override;
        virtual sal_Bool SAL_CALL supportsSubqueriesInIns(  ) override;
        virtual sal_Bool SAL_CALL supportsSubqueriesInQuantifieds(  ) override;
        virtual sal_Bool SAL_CALL supportsCorrelatedSubqueries(  ) override;
        virtual sal_Bool SAL_CALL supportsUnion(  ) override;
        virtual sal_Bool SAL_CALL supportsUnionAll(  ) override;
        virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit(  ) override;
        virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback(  ) override;
        virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit(  ) override;
        virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback(  ) override;
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
        virtual sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs(  ) override;
        virtual sal_Int32 SAL_CALL getMaxStatementLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxTableNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getMaxUserNameLength(  ) override;
        virtual sal_Int32 SAL_CALL getDefaultTransactionIsolation(  ) override;
        virtual sal_Bool SAL_CALL supportsTransactions(  ) override;
        virtual sal_Bool SAL_CALL supportsTransactionIsolationLevel( sal_Int32 level ) override;
        virtual sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions(  ) override;
        virtual sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly(  ) override;
        virtual sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit(  ) override;
        virtual sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedures( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedureColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTables( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getSchemas(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCatalogs(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTableTypes(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const css::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTablePrivileges( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, sal_Bool nullable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getVersionColumns( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getImportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getExportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCrossReference( const css::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const css::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getIndexInfo( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate ) override;
        virtual sal_Bool SAL_CALL supportsResultSetType( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) override;
        virtual sal_Bool SAL_CALL ownUpdatesAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL ownDeletesAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL ownInsertsAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL othersUpdatesAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL othersDeletesAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL othersInsertsAreVisible( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL updatesAreDetected( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL deletesAreDetected( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL insertsAreDetected( sal_Int32 setType ) override;
        virtual sal_Bool SAL_CALL supportsBatchUpdates(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getUDTs( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const css::uno::Sequence< sal_Int32 >& types ) override;

    private:
        bool        impl_callBooleanMethod( const char* _pMethodName, jmethodID& _inout_MethodID );
        OUString impl_callStringMethod( const char* _pMethodName, jmethodID& _inout_MethodID );
        sal_Int32   impl_callIntMethod_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID );
        sal_Int32   impl_callIntMethod_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID );
        bool        impl_callBooleanMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument );
        css::uno::Reference< css::sdbc::XResultSet >
                        impl_callResultSetMethod( const char* _pMethodName, jmethodID& _inout_MethodID );
        css::uno::Reference< css::sdbc::XResultSet >
                        impl_callResultSetMethodWithStrings( const char* _pMethodName, jmethodID& _inout_MethodID, const css::uno::Any& _rCatalog,
                            const OUString& _rSchemaPattern, const OUString& _rLeastPattern,
                            const OUString* _pOptionalAdditionalString = nullptr);
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
