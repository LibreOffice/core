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

#include "FDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"

#include <ibase.h>
#include <rtl/ustrbuf.hxx>

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
    return 0;
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
    return 32;
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
    return 32;
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
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
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString(  ) throw(SQLException, RuntimeException)
{
    // normally this is "
    ::rtl::OUString aVal("\"");
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
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
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
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
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
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
::rtl::OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion() throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
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
::rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue;
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    (void) setType;
    return sal_False;
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

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates() throw(SQLException, RuntimeException)
{
    // No batch support in firebird
    return sal_False;
}

uno::Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection() throw(SQLException, RuntimeException)
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
// -----------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    (void) columnNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) tableNamePattern;
    (void) columnNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& catalog,
        const ::rtl::OUString& schemaPattern,
        const ::rtl::OUString& tableNamePattern,
        const Sequence< ::rtl::OUString >& types)
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
    // TODO: OUStringBuf
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
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    (void) columnNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any& catalog, const ::rtl::OUString& schemaPattern,
    const ::rtl::OUString& procedureNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) procedureNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schema;
    (void) table;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
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
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
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
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    (void) catalog;
    (void) schemaPattern;
    (void) tableNamePattern;
    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
    const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
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
uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& typeNamePattern, const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException)
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
