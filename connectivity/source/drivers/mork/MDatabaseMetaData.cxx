/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MConnection.hxx"
#include "MDatabaseMetaData.hxx"

#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <sal/log.hxx>
#include <strings.hrc>
#include <vector>

#include "MDatabaseMetaDataHelper.hxx"
#include <connectivity/dbtools.hxx>

using namespace connectivity::mork;
using namespace connectivity;

using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;


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
                        ,m_pMetaDataHelper(new MDatabaseMetaDataHelper)
{
    OSL_ENSURE(m_pConnection,"ODatabaseMetaData::ODatabaseMetaData: No connection set!");
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}


ODatabaseMetaDataResultSet::ORows ODatabaseMetaData::getColumnRows(
        const OUString& tableNamePattern,
        const OUString& columnNamePattern )
{
    SAL_INFO("connectivity.mork", "=> ODatabaseMetaData::getColumnRows()" );
    SAL_INFO("connectivity.mork", "tableNamePattern: " << tableNamePattern);
    SAL_INFO("connectivity.mork", "columnNamePattern: " << columnNamePattern);

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(19);

    ::osl::MutexGuard aGuard( m_aMutex );
    std::vector< OUString > tables;
    connectivity::mork::MDatabaseMetaDataHelper::getTableStrings(m_pConnection, tables);

    // ****************************************************
    // Some entries in a row never change, so set them now
    // ****************************************************

    // Catalog
    aRow[1] = new ORowSetValueDecorator(OUString());
    // Schema
    aRow[2] = new ORowSetValueDecorator(OUString());
    // DATA_TYPE
    aRow[5] = new ORowSetValueDecorator(static_cast<sal_Int16>(DataType::VARCHAR));
    // TYPE_NAME, not used
    aRow[6] = new ORowSetValueDecorator(OUString("VARCHAR"));
    // COLUMN_SIZE
    aRow[7] = new ORowSetValueDecorator(s_nCOLUMN_SIZE);
    // BUFFER_LENGTH, not used
    aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
    // DECIMAL_DIGITS.
    aRow[9] = new ORowSetValueDecorator(s_nDECIMAL_DIGITS);
    // NUM_PREC_RADIX
    aRow[10] = new ORowSetValueDecorator(sal_Int32(10));
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
    aRow[18] = new ORowSetValueDecorator(OUString("YES"));

    // Iterate over all tables
    for(OUString & table : tables) {
        if(match(tableNamePattern, table,'\0')) {
            // TABLE_NAME
            aRow[3] = new ORowSetValueDecorator( table );

            const OColumnAlias& colNames = m_pConnection->getColumnAlias();

            SAL_INFO("connectivity.mork", "\tTableName = : " << table);
            // Iterate over all columns in the table.
            for (const auto& [rName, rAlias] : colNames)
            {
                if ( match( columnNamePattern, rName, '\0' ) )
                {
                    SAL_INFO("connectivity.mork", "\t\tColumnNam : " << rName);

                    // COLUMN_NAME
                    aRow[4] = new ORowSetValueDecorator( rName );
                    // ORDINAL_POSITION
                    aRow[17] = new ORowSetValueDecorator( static_cast< sal_Int32 >( rAlias.columnPosition ) + 1 );
                    aRows.push_back(aRow);
                }
            }
        }
    }
    return aRows;
}

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 65535; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 254; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return 20; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0; // 0 means no limit
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    // We only support a single table
    return 1;
}


sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    // normally this is "
    return OUString( "\"");
}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return true;
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    //We support create table
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return true; // should be supported at least
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  )
{
    return 0;// 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  )
{
    // We allow you to select from any table.
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    //we support insert/update/delete now
    //But we have to set this to return sal_True otherwise the UI will add create "table/edit table"
    //entry to the popup menu. We should avoid them.
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  )
{
    // Support added for this.
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return true;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    // Any case may be used
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString SAL_CALL ODatabaseMetaData::getURL(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pConnection->getURL();
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion()
{
    OUString aValue = OUString::number(1);
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    OUString aValue = OUString::number(0);
    return aValue;
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
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

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 /*setType*/, sal_Int32 /*concurrency*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    // there exists no possibility to get table types so we have to check
    static const OUStringLiteral sTableTypes[] =
    {
        "TABLE",
        "VIEW"
        // Currently we only support a 'TABLE' and 'VIEW' nothing more complex

        // OUString("SYSTEM TABLE"),
        // OUString("GLOBAL TEMPORARY"),
        // OUString("LOCAL TEMPORARY"),
        // OUString("ALIAS"),
        // OUString("SYNONYM")
    };
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    // here we fill the rows which should be visible when ask for data from the resultset returned here
    ODatabaseMetaDataResultSet::ORows aRows;
    for(const auto & sTableType : sTableTypes)
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString(sTableType)));
        // bound row
        aRows.push_back(aRow);
    }
    // here we set the rows at the resultset
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aRows = [&]()
    {
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(19);
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR")));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(s_nCHAR_OCTET_LENGTH)));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        // aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(ColumnSearch::CHAR)));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(10)));

        tmp.push_back(aRow);
        return tmp;
    }();
    pResultSet->setRows(aRows);
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern,
    const OUString& columnNamePattern )
{
    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( getColumnRows( tableNamePattern, columnNamePattern ));
    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/,
    const OUString& tableNamePattern, const Sequence< OUString >& /*types*/ )
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
    connectivity::mork::MDatabaseMetaDataHelper::getTables( m_pConnection, tableNamePattern, _rRows );
    pResultSet->setRows( _rRows );

    return xResultSet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern )
{
    SAL_INFO("connectivity.mork", "=> ODatabaseMetaData::getTablePrivileges()" );
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTablePrivileges);
    Reference< XResultSet > xRef = pResult;

    std::vector< OUString > tables;
    connectivity::mork::MDatabaseMetaDataHelper::getTableStrings( m_pConnection, tables);

    ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
    ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(8);
    aRows.reserve(8);
    aRow[0] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[1] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[3] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[4] = ::connectivity::ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[5] = new ::connectivity::ORowSetValueDecorator(getUserName());
    aRow[7] = new ::connectivity::ORowSetValueDecorator(OUString("NO"));


    // Iterate over all tables
    for(OUString & table : tables) {
       if(match(tableNamePattern, table,'\0'))
           {
            // TABLE_NAME
                aRow[2] = new ORowSetValueDecorator( table );

                SAL_INFO("connectivity.mork", "\tTableName = : " << table);

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

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
