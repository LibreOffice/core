/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_DATABASEMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_DATABASEMETADATA_HXX

#include "pq_connection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

#include <cppuhelper/implbase.hxx>

namespace pq_sdbc_driver
{

class DatabaseMetaData :
        public ::cppu::WeakImplHelper< css::sdbc::XDatabaseMetaData >
{
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ConnectionSettings *m_pSettings;
    css::uno::Reference< css::sdbc::XConnection > m_origin;
    css::uno::Reference< css::sdbc::XPreparedStatement > m_getIntSetting_stmt;
    css::uno::Reference< css::sdbc::XPreparedStatement > m_getReferences_stmt[16];
    css::uno::Reference< css::sdbc::XPreparedStatement > m_getTablePrivs_stmt;
    css::uno::Reference< css::sdbc::XPreparedStatement > m_getColumnPrivs_stmt;

    sal_Int32 getIntSetting(const OUString& settingName) throw (css::sdbc::SQLException, css::uno::RuntimeException);
    sal_Int32 getMaxIndexKeys()  throw (css::sdbc::SQLException, css::uno::RuntimeException);
    sal_Int32 getMaxNameLength() throw (css::sdbc::SQLException, css::uno::RuntimeException);
    css::uno::Reference< css::sdbc::XResultSet > getImportedExportedKeys(
        const css::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable,
        const css::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable )
        throw (css::sdbc::SQLException, css::uno::RuntimeException);
    void init_getReferences_stmt ();
    void init_getPrivs_stmt ();

public:
    DatabaseMetaData(
        const ::rtl::Reference< RefCountedMutex > & reMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings
        );

public:
    // Methods
    virtual sal_Bool SAL_CALL allProceduresAreCallable(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL allTablesAreSelectable(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getURL(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getUserName(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL nullsAreSortedHigh(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL nullsAreSortedLow(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL nullsAreSortedAtStart(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL nullsAreSortedAtEnd(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDatabaseProductName(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDatabaseProductVersion(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDriverName(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDriverVersion(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDriverMajorVersion(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDriverMinorVersion(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL usesLocalFiles(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL usesLocalFilePerTable(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMixedCaseIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesUpperCaseIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesLowerCaseIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesMixedCaseIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesUpperCaseQuotedIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesLowerCaseQuotedIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getIdentifierQuoteString(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSQLKeywords(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNumericFunctions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getStringFunctions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSystemFunctions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTimeDateFunctions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSearchStringEscape(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getExtraNameCharacters(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsColumnAliasing(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL nullPlusNonNullIsNull(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsTypeConversion(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsTableCorrelationNames(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsDifferentTableCorrelationNames(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsExpressionsInOrderBy(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOrderByUnrelated(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsGroupBy(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsGroupByUnrelated(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsGroupByBeyondSelect(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsLikeEscapeClause(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMultipleResultSets(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMultipleTransactions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsNonNullableColumns(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMinimumSQLGrammar(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCoreSQLGrammar(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsExtendedSQLGrammar(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsANSI92EntryLevelSQL(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsANSI92IntermediateSQL(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsANSI92FullSQL(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsIntegrityEnhancementFacility(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOuterJoins(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsFullOuterJoins(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsLimitedOuterJoins(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSchemaTerm(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getProcedureTerm(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCatalogTerm(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isCatalogAtStart(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCatalogSeparator(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSchemasInDataManipulation(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSchemasInProcedureCalls(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSchemasInTableDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSchemasInIndexDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSchemasInPrivilegeDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCatalogsInDataManipulation(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCatalogsInProcedureCalls(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCatalogsInTableDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCatalogsInIndexDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCatalogsInPrivilegeDefinitions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsPositionedDelete(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsPositionedUpdate(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSelectForUpdate(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsStoredProcedures(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSubqueriesInComparisons(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSubqueriesInExists(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSubqueriesInIns(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsSubqueriesInQuantifieds(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsCorrelatedSubqueries(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsUnion(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsUnionAll(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossCommit(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOpenCursorsAcrossRollback(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossCommit(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsOpenStatementsAcrossRollback(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxBinaryLiteralLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxCharLiteralLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInGroupBy(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInIndex(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInOrderBy(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInSelect(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInTable(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxConnections(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxCursorNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxIndexLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxSchemaNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxProcedureNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxCatalogNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxRowSize(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL doesMaxRowSizeIncludeBlobs(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxStatementLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxStatements(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxTableNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxTablesInSelect(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxUserNameLength(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDefaultTransactionIsolation(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsTransactions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsTransactionIsolationLevel( sal_Int32 level ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsDataDefinitionAndDataManipulationTransactions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsDataManipulationTransactionsOnly(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL dataDefinitionCausesTransactionCommit(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL dataDefinitionIgnoredInTransactions(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedures( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedureColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTables( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getSchemas(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCatalogs(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTableTypes(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const css::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTablePrivileges( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, sal_Bool nullable ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getVersionColumns( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getImportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getExportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCrossReference( const css::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const css::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTypeInfo(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getIndexInfo( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsResultSetType( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL ownUpdatesAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL ownDeletesAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL ownInsertsAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL othersUpdatesAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL othersDeletesAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL othersInsertsAreVisible( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL updatesAreDetected( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL deletesAreDetected( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL insertsAreDetected( sal_Int32 setType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsBatchUpdates(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getUDTs( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern, const css::uno::Sequence< sal_Int32 >& types ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
