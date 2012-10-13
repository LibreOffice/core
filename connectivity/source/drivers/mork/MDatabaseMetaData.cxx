/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include "resource/common_res.hrc"
#include <vector>

#include "MDatabaseMetaDataHelper.hxx"
#include "MorkParser.hxx"
#include "connectivity/dbtools.hxx"

using namespace dbtools;
using namespace connectivity::mork;
using namespace connectivity;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


namespace connectivity
{
    namespace mork
    {
        static sal_Int32 const s_nCOLUMN_SIZE = 256;
        static sal_Int32 const s_nDECIMAL_DIGITS = 0;
        static sal_Int32 const s_nNULLABLE = 1;
        static sal_Int32 const s_nCHAR_OCTET_LENGTH = 65535;
    }
}

ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon)
                        : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_pConnection(_pCon)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
    m_pMetaDataHelper = new MDatabaseMetaDataHelper();
}
// -------------------------------------------------------------------------
ODatabaseMetaData::~ODatabaseMetaData()
{
    delete m_pMetaDataHelper;
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ORows& SAL_CALL ODatabaseMetaData::getColumnRows(
        const ::rtl::OUString& tableNamePattern,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException)
{
    SAL_INFO("connectivity.mork", "=> ODatabaseMetaData::getColumnRows()" );
    SAL_INFO("connectivity.mork", "tableNamePattern: " << tableNamePattern);
    SAL_INFO("connectivity.mork", "columnNamePattern: " << columnNamePattern);

    static ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(19);
    aRows.clear();

    ::osl::MutexGuard aGuard( m_aMutex );
    ::std::vector< ::rtl::OUString > tables;
    if (!m_pMetaDataHelper->getTableStrings(m_pConnection, tables))
    {
        ::connectivity::SharedResources aResources;
        // TODO:
        // get better message here?
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_UNKNOWN_COLUMN_TYPE);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }

    // ****************************************************
    // Some entries in a row never change, so set them now
    // ****************************************************

    // Catalog
    aRow[1] = new ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("")));
    // Schema
    aRow[2] = new ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("")));
    // DATA_TYPE
    aRow[5] = new ORowSetValueDecorator(static_cast<sal_Int16>(DataType::VARCHAR));
    // TYPE_NAME, not used
    aRow[6] = new ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("VARCHAR")));
    // COLUMN_SIZE
    aRow[7] = new ORowSetValueDecorator(s_nCOLUMN_SIZE);
    // BUFFER_LENGTH, not used
    aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
    // DECIMAL_DIGITS.
    aRow[9] = new ORowSetValueDecorator(s_nDECIMAL_DIGITS);
    // NUM_PREC_RADIX
    aRow[10] = new ORowSetValueDecorator((sal_Int32)10);
    // NULLABLE
    aRow[11] = new ORowSetValueDecorator(s_nNULLABLE);
    // REMARKS
    aRow[12] = ODatabaseMetaDataResultSet::getEmptyValue();
    // COULUMN_DEF, not used
    aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
    // SQL_DATA_TYPE, not used
    aRow[14] = ODatabaseMetaDataResultSet::getEmptyValue();
    // SQL_DATETIME_SUB, not used
    aRow[15] = ODatabaseMetaDataResultSet::getEmptyValue();
    // CHAR_OCTET_LENGTH, refer to [5]
    aRow[16] = new ORowSetValueDecorator(s_nCHAR_OCTET_LENGTH);
    // IS_NULLABLE
    aRow[18] = new ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("YES")));

    // Iterate over all tables
    for(size_t j = 0; j < tables.size(); j++ ) {
        if(match(tableNamePattern, tables[j],'\0')) {
            // TABLE_NAME
            aRow[3] = new ORowSetValueDecorator( tables[j] );

            const OColumnAlias& colNames = m_pConnection->getColumnAlias();

            SAL_INFO("connectivity.mork", "\tTableName = : " << tables[j]);
            // Iterate over all collumns in the table.
            for (   OColumnAlias::AliasMap::const_iterator compare = colNames.begin();
                    compare != colNames.end();
                    ++compare
                )
            {
                if ( match( columnNamePattern, compare->first, '\0' ) )
                {
                    SAL_INFO("connectivity.mork", "\t\tColumnNam : " << compare->first);

                    // COLUMN_NAME
                    aRow[4] = new ORowSetValueDecorator( compare->first );
                    // ORDINAL_POSITION
                    aRow[17] = new ORowSetValueDecorator( static_cast< sal_Int32 >( compare->second.columnPosition ) + 1 );
                    aRows.push_back(aRow);
                }
            }
        }
    }
    return( aRows );
}
// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 65535; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 254; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 20; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    // We only support a single table
    return 1;
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
sal_Bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
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
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
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
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aVal;
    return aVal;
}
// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    // normally this is "
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("\""));
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
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return sal_True;
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
    //We support create table
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
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
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
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
sal_Bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    return 0;// 0 means no limit
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    // We allow you to select from any table.
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    //we support insert/update/delete now
    //But we have to set this to return sal_True otherwise the UI will add create "table/edit table"
    //entry to the popup menu. We should avoid them.
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
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
    // Support added for this.
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    // Any case may be used
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
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
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
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
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
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pConnection->getURL();
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
    ::rtl::OUString aValue = ::rtl::OUString::valueOf((sal_Int32)1);
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue = ::rtl::OUString::valueOf((sal_Int32)0);
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
    sal_Int32 nValue = 0; // 0 means no limit
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    // there exists no possibility to get table types so we have to check
    static ::rtl::OUString sTableTypes[] =
    {
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TABLE")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("VIEW"))
        // Currently we only support a 'TABLE' and 'VIEW' nothing more complex
        //
        // ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SYSTEM TABLE")),
        // ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("GLOBAL TEMPORARY")),
        // ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LOCAL TEMPORARY")),
        // ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ALIAS")),
        // ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SYNONYM"))
    };
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // here we fill the rows which should be visible when ask for data from the resultset returned here
    const sal_Int32  nSize = sizeof(sTableTypes) / sizeof(::rtl::OUString);
    ODatabaseMetaDataResultSet::ORows aRows;
    for(sal_Int32 i=0;i < nSize;++i)
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(sTableTypes[i]));
        // bound row
        aRows.push_back(aRow);
    }
    // here we set the rows at the resultset
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aRows;

    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(19);
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("VARCHAR"))));
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
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& tableNamePattern,
    const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( getColumnRows( tableNamePattern, columnNamePattern ));
    return xResultSet;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/,
    const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& /*types*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> ODatabaseMetaData::getTables()" );
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    Reference< XResultSet > xResultSet = pResultSet;

    // ODatabaseMetaDataResultSet::ORows aRows;
    // aRows = m_pDbMetaDataHelper->getTables( m_pConnection, tableNamePattern );
    // pResultSet->setRows( aRows );
    ODatabaseMetaDataResultSet::ORows _rRows;
    if ( !m_pMetaDataHelper->getTables( m_pConnection, tableNamePattern, _rRows ) ) {
        ::connectivity::SharedResources aResources;
        // TODO:
        // get better message here?
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_UNKNOWN_COLUMN_TYPE);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }
    pResultSet->setRows( _rRows );

    return xResultSet;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> ODatabaseMetaData::getTablePrivileges()" );
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTablePrivileges);
    Reference< XResultSet > xRef = pResult;

    ::std::vector< ::rtl::OUString > tables;
    if ( !m_pMetaDataHelper->getTableStrings( m_pConnection, tables) )
    {
        ::connectivity::SharedResources aResources;
        // TODO:
        // get better message here?
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_UNKNOWN_COLUMN_TYPE);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }

    ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
    ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(8);
    aRows.reserve(8);
    aRow[0] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[1] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[3] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[4] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[5] = new ::connectivity::ORowSetValueDecorator(getUserName());
    aRow[7] = new ::connectivity::ORowSetValueDecorator(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NO")));


    // Iterate over all tables
    for(size_t j = 0; j < tables.size(); j++ ) {
       if(match(tableNamePattern, tables[j],'\0'))
           {
            // TABLE_NAME
            aRow[2] = new ORowSetValueDecorator( tables[j] );

            SAL_INFO("connectivity.mork", "\tTableName = : " << tables[j]);

                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getSelectValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getInsertValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDeleteValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getUpdateValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getCreateValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getReadValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getAlterValue();
                aRows.push_back(aRow);
                aRow[6] = ::connectivity::ODatabaseMetaDataResultSet::getDropValue();
                aRows.push_back(aRow);
        }
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
