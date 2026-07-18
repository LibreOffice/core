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


#include "MacabDatabaseMetaData.hxx"
#include "MacabAddressBook.hxx"
#include "MacabHeader.hxx"
#include "MacabGroup.hxx"
#include "macabutilities.hxx"

#include "MacabDriver.hxx"
#include <FDatabaseMetaDataResultSet.hxx>
#include <OTypeInfo.hxx>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <rtl/ref.hxx>

#include <vector>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

MacabDatabaseMetaData::MacabDatabaseMetaData(MacabConnection* _pCon)
        : m_xConnection(_pCon),
          m_bUseCatalog(true)
{
    OSL_ENSURE(_pCon,"MacabDatabaseMetaData::MacabDatabaseMetaData: No connection set!");

    osl_atomic_increment( &m_refCount );
    m_bUseCatalog   = !(usesLocalFiles() || usesLocalFilePerTable());
    osl_atomic_decrement( &m_refCount );
}

MacabDatabaseMetaData::~MacabDatabaseMetaData()
{
}

OUString MacabDatabaseMetaData::getCatalogSeparator(  )
{
    if (m_bUseCatalog)
    { // do some special here for you database
    }

    return OUString();
}

sal_Int32 MacabDatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxRowSize(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxConnections(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxStatementLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxTableNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxTablesInSelect(  )
{
    // MaxTablesInSelect describes how many tables can participate in the FROM part of a given SELECT statement,
    // currently, the resultset/statement implementations can cope with one table only
    return 1;
}

bool MacabDatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesMixedCaseQuotedIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsAlterTableWithAddColumn(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsAlterTableWithDropColumn(  )
{
    return false;
}

sal_Int32 MacabDatabaseMetaData::getMaxIndexLength(  )
{
    return 0; // 0 means no limit
}

bool MacabDatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString MacabDatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getIdentifierQuoteString(  )
{
    // normally this is "
    return "\"";
}

OUString MacabDatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

bool MacabDatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return false;
}

bool MacabDatabaseMetaData::isCatalogAtStart(  )
{
    return false;
}

bool MacabDatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

bool MacabDatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

bool MacabDatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return true;
}

bool MacabDatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return true;
}

bool MacabDatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSchemasInDataManipulation(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return true; // should be supported at least
}

bool MacabDatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSchemasInTableDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCatalogsInTableDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCatalogsInDataManipulation(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

sal_Int32 MacabDatabaseMetaData::getMaxStatements(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0; // 0 means no limit
}

bool MacabDatabaseMetaData::supportsTransactions(  )
{
    return false;
}

bool MacabDatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

bool MacabDatabaseMetaData::allTablesAreSelectable(  )
{
    return false;
}

bool MacabDatabaseMetaData::isReadOnly(  )
{
    // for the moment, we have read-only addresses, but this might change in the future
    return true;
}

bool MacabDatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

bool MacabDatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

bool MacabDatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsColumnAliasing(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsTableCorrelationNames(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsConvert( sal_Int32, sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsOrderByUnrelated(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsUnion(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  )
{
    return false;
}

bool MacabDatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

bool MacabDatabaseMetaData::nullsAreSortedAtStart(  )
{
    return false;
}

bool MacabDatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

bool MacabDatabaseMetaData::nullsAreSortedLow(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString MacabDatabaseMetaData::getURL(  )
{
    // if someday we support more than the default address book,
    // this method should return the URL which was used to create it
    return "sdbc:address:macab:";
}

OUString MacabDatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getDriverName(  )
{
    return "macab";
}

OUString MacabDatabaseMetaData::getDriverVersion()
{
    return MACAB_DRIVER_VERSION;
}

OUString MacabDatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
}

sal_Int32 MacabDatabaseMetaData::getDriverMajorVersion(  )
{
    return MACAB_DRIVER_VERSION_MAJOR;
}

sal_Int32 MacabDatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return TransactionIsolation::NONE;
}

sal_Int32 MacabDatabaseMetaData::getDriverMinorVersion(  )
{
    return MACAB_DRIVER_VERSION_MINOR;
}

OUString MacabDatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString MacabDatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

bool MacabDatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

bool MacabDatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

bool MacabDatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0; // 0 means no limit
}

sal_Int32 MacabDatabaseMetaData::getMaxUserNameLength(  )
{
    return 0; // 0 means no limit
}

bool MacabDatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return true;
    }
    return false;
}

bool MacabDatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 )
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return true;
    }
    return false;
}

bool MacabDatabaseMetaData::ownUpdatesAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::ownDeletesAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::ownInsertsAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::othersUpdatesAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::othersDeletesAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::othersInsertsAreVisible( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::updatesAreDetected( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::deletesAreDetected( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::insertsAreDetected( sal_Int32 )
{
    return false;
}

bool MacabDatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

Reference< XConnection > MacabDatabaseMetaData::getConnection(  )
{
    return m_xConnection;
}

Reference< XResultSet > MacabDatabaseMetaData::getTableTypes(  )
{
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);

    static ODatabaseMetaDataResultSet::ORows aRows = []
    {
        static constexpr OUStringLiteral aTable = u"TABLE";
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow(2);
        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = new ORowSetValueDecorator(OUString(aTable));
        tmp.push_back(aRow);
        return tmp;
    }();
    pResult->setRows(std::vector(aRows));
    return pResult;
}

Reference< XResultSet > MacabDatabaseMetaData::getTypeInfo(  )
{
    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);

    static ODatabaseMetaDataResultSet::ORows aRows = []()
    {
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow(19);

        // We support four types: char, timestamp, integer, float
        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = new ORowSetValueDecorator(OUString("CHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::CHAR);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(254));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[6] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[7] = new ORowSetValueDecorator(sal_Int32(ColumnValue::NULLABLE));
        aRow[8] = ODatabaseMetaDataResultSet::get1Value();
        aRow[9] = new ORowSetValueDecorator(sal_Int32(ColumnSearch::CHAR));
        aRow[10] = ODatabaseMetaDataResultSet::get1Value();
        aRow[11] = ODatabaseMetaDataResultSet::get0Value();
        aRow[12] = ODatabaseMetaDataResultSet::get0Value();
        aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[14] = ODatabaseMetaDataResultSet::get0Value();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRow[16] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[17] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[18] = new ORowSetValueDecorator(sal_Int32(10));

        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(19));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("INTEGER"));
        aRow[2] = new ORowSetValueDecorator(DataType::INTEGER);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[15] = new ORowSetValueDecorator(sal_Int32(20));
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("FLOAT"));
        aRow[2] = new ORowSetValueDecorator(DataType::FLOAT);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[15] = new ORowSetValueDecorator(sal_Int32(15));
        tmp.push_back(aRow);

        return tmp;
    }();
    pResult->setRows(std::vector(aRows));
    return pResult;
}

Reference< XResultSet > MacabDatabaseMetaData::getCatalogs(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCatalogs );
}

Reference< XResultSet > MacabDatabaseMetaData::getSchemas(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eSchemas );
}

Reference< XResultSet > MacabDatabaseMetaData::getColumnPrivileges(
    const Any&, const OUString&, const OUString&,
    const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumnPrivileges );
}

Reference< XResultSet > MacabDatabaseMetaData::getColumns(
    const Any&,
    const OUString&,
    const OUString& tableNamePattern,
    const OUString& columnNamePattern)
{
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    MacabRecords *aRecords;
    OUString sTableName;

    aRecords = m_xConnection->getAddressBook()->getMacabRecordsMatch(tableNamePattern);

    ODatabaseMetaDataResultSet::ORows aRows;
    if(aRecords != nullptr)
    {
        MacabHeader *aHeader = aRecords->getHeader();
        sTableName = aRecords->getName();

        ODatabaseMetaDataResultSet::ORow aRow(19);

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[3] = new ORowSetValueDecorator(sTableName);
        aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[9] = ODatabaseMetaDataResultSet::get0Value();
        aRow[10] = new ORowSetValueDecorator(sal_Int32(10));
        aRow[11] = ODatabaseMetaDataResultSet::get1Value();
        aRow[12] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[13] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[14] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[15] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[16] = new ORowSetValueDecorator(sal_Int32(254));
        aRow[18] = new ORowSetValueDecorator(OUString("YES"));

        sal_Int32 nPosition = 1;
        OUString sName;

        MacabHeader::iterator aField;

        for (   aField = aHeader->begin();
                aField != aHeader->end();
                ++aField, ++nPosition)
        {

            sName = CFStringToOUString(static_cast<CFStringRef>((*aField)->value));
            if (match(columnNamePattern, sName, '\0'))
            {
                aRow[4] = new ORowSetValueDecorator(sName);
                aRow[17] = new ORowSetValueDecorator(nPosition);
                switch((*aField)->type)
                {
                    case kABStringProperty:
                        aRow[5] = new ORowSetValueDecorator(DataType::CHAR);
                        aRow[6] = new ORowSetValueDecorator(OUString("CHAR"));
                        aRow[7] = new ORowSetValueDecorator(sal_Int32(256));
                        aRows.push_back(aRow);
                        break;
                    case kABDateProperty:
                        aRow[5] = new ORowSetValueDecorator(DataType::TIMESTAMP);
                        aRow[6] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
                        aRows.push_back(aRow);
                        break;
                    case kABIntegerProperty:
                        aRow[5] = new ORowSetValueDecorator(DataType::INTEGER);
                        aRow[6] = new ORowSetValueDecorator(OUString("INTEGER"));
                        aRow[7] = new ORowSetValueDecorator(sal_Int32(20));
                        aRows.push_back(aRow);
                        break;
                    case kABRealProperty:
                        aRow[5] = new ORowSetValueDecorator(DataType::FLOAT);
                        aRow[6] = new ORowSetValueDecorator(OUString("FLOAT"));
                        aRow[7] = new ORowSetValueDecorator(sal_Int32(15));
                        aRows.push_back(aRow);
                        break;
                    default:
                        ;
                        // shouldn't happen -- throw an error...?
                }
            }
        }
    }
    pResult->setRows(std::move(aRows));
    return pResult;
}

Reference< XResultSet > MacabDatabaseMetaData::getTables(
    const Any&,
    const OUString&,
    const OUString&,
    const Sequence< OUString >& types)
{
    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTables);

    // check whether we have tables in the requested types
    // for the moment, we answer only the "TABLE" table type
    // when no types are given at all, we return all the tables
    static constexpr OUString aTable = u"TABLE"_ustr;
    bool bTableFound = false;
    const OUString* p = types.getConstArray(),
                         * pEnd = p + types.getLength();

    if (p == pEnd)
    {
        bTableFound = true;
    }
    else while (p < pEnd)
    {
        if (match(*p, aTable, '\0'))
        {
            bTableFound = true;
            break;
        }
        p++;
    }
    if (!bTableFound)
        return pResult;

    static ODatabaseMetaDataResultSet::ORows aRows = [&]()
    {
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow(6);

        MacabRecords *xRecords = m_xConnection->getAddressBook()->getMacabRecords();
        std::vector<MacabGroup *> xGroups = m_xConnection->getAddressBook()->getMacabGroups();
        sal_Int32 i, nGroups;
        nGroups = xGroups.size();

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[3] = new ORowSetValueDecorator(xRecords->getName());
        aRow[4] = new ORowSetValueDecorator(aTable);
        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        tmp.push_back(aRow);

        for(i = 0; i < nGroups; i++)
        {
            aRow[3] = new ORowSetValueDecorator(xGroups[i]->getName());
            tmp.push_back(aRow);
        }
        return tmp;
    }();
    pResult->setRows(std::vector(aRows));
    return pResult;
}

Reference< XResultSet > MacabDatabaseMetaData::getProcedureColumns(
    const Any&, const OUString&,
    const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedureColumns );
}

Reference< XResultSet > MacabDatabaseMetaData::getProcedures(
    const Any&, const OUString&,
    const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedures );
}

Reference< XResultSet > MacabDatabaseMetaData::getVersionColumns(
    const Any&, const OUString&, const OUString& table )
{
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eVersionColumns);

    ODatabaseMetaDataResultSet::ORows aRows;

    if (m_xConnection->getAddressBook()->getMacabRecords(table) != nullptr)
    {
        ODatabaseMetaDataResultSet::ORow aRow( 9 );

        OUString sName = CFStringToOUString(kABModificationDateProperty);

        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[2] = new ORowSetValueDecorator(sName);
        aRow[3] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[4] = new ORowSetValueDecorator(OUString("TIMESTAMP"));

        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[6] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[7] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[8] = ODatabaseMetaDataResultSet::getEmptyValue();

        aRows.push_back(aRow);
    }
    pResult->setRows(std::move(aRows));
    return pResult;
}

Reference< XResultSet > MacabDatabaseMetaData::getExportedKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eExportedKeys );
}

Reference< XResultSet > MacabDatabaseMetaData::getImportedKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eImportedKeys );
}

Reference< XResultSet > MacabDatabaseMetaData::getPrimaryKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::ePrimaryKeys );
}

Reference< XResultSet > MacabDatabaseMetaData::getIndexInfo(
    const Any&, const OUString&, const OUString&,
    bool, bool )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eIndexInfo );
}

Reference< XResultSet > MacabDatabaseMetaData::getBestRowIdentifier(
    const Any&, const OUString&, const OUString&, sal_Int32,
    bool )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eBestRowIdentifier );
}

Reference< XResultSet > MacabDatabaseMetaData::getTablePrivileges(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
}

Reference< XResultSet > MacabDatabaseMetaData::getCrossReference(
    const Any&, const OUString&,
    const OUString&, const Any&,
    const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCrossReference );
}

Reference< XResultSet > MacabDatabaseMetaData::getUDTs( const Any&, const OUString&, const OUString&, const Sequence< sal_Int32 >& )
{
    OSL_FAIL("Not implemented yet!");
    throw SQLException();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
