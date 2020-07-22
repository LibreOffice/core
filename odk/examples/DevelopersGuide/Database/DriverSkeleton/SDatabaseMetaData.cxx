/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "SDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>

using namespace connectivity::skeleton;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
: m_pConnection(_pCon)
, m_bUseCatalog(sal_True)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
    if(!m_pConnection->isCatalogUsed())
    {
        osl_atomic_increment( &m_refCount );
        m_bUseCatalog   = !(usesLocalFiles() || usesLocalFilePerTable());
        osl_atomic_decrement( &m_refCount );
    }
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator(  )
{
    ::rtl::OUString aVal;
    if(m_bUseCatalog)
    { // do some special here for you database
    }

    return aVal;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}


sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn(  )
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return sal_False;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  )
{
    ::rtl::OUString aVal;
    if(m_bUseCatalog)
    {
    }
    return aVal;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString(  )
{
    // normally this is "
    ::rtl::OUString aVal("\"");
    return aVal;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    ::rtl::OUString aVal;
    return aVal;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart(  )
{
    sal_Bool bValue = sal_False;
    if(m_bUseCatalog)
    {
    }
    return bValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return sal_True; // should be supported at least
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  )
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return sal_False;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getURL(  )
{
    ::rtl::OUString aValue("sdbc:skeleton:");
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getUserName(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return TransactionIsolation::NONE;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  )
{
    ::rtl::OUString aValue;
    return aValue;
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  )
{
    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  )
{
    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  )
{
    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  )
{
    return ::rtl::OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return sal_True;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return sal_False;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  )
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 setType )
{
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  )
{
    return sal_False;
}

Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection(  )
{
    return (Reference< XConnection >)m_pConnection;//new OConnection(m_aConnectionHandle);
}

// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo(  )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs(  )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    const ::rtl::OUString& columnNamePattern )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    sal_Bool unique, sal_Bool approximate )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
    sal_Bool nullable )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
    const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable )
{
    return NULL;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern, const Sequence< sal_Int32 >& types )
{
    OSL_FAIL("Not implemented yet!");
    throw SQLException();
    return NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
