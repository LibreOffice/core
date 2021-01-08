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
#include <com/sun/star/sdbc/TransactionIsolation.hpp>

#include <vector>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
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

OUString SAL_CALL MacabDatabaseMetaData::getCatalogSeparator(  )
{
    if (m_bUseCatalog)
    { // do some special here for you database
    }

    return OUString();
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxRowSize(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxConnections(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxStatementLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxTableNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxTablesInSelect(  )
{
    // MaxTablesInSelect describes how many tables can participate in the FROM part of a given SELECT statement,
    // currently, the resultset/statement implementations can cope with one table only
    return 1;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesMixedCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsAlterTableWithAddColumn(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsAlterTableWithDropColumn(  )
{
    return false;
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxIndexLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString SAL_CALL MacabDatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getIdentifierQuoteString(  )
{
    // normally this is "
    return "\"";
}

OUString SAL_CALL MacabDatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::isCatalogAtStart(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSchemasInDataManipulation(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return true; // should be supported at least
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSchemasInTableDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCatalogsInTableDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCatalogsInDataManipulation(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxStatements(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::allTablesAreSelectable(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::isReadOnly(  )
{
    // for the moment, we have read-only addresses, but this might change in the future
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::usesLocalFiles(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsColumnAliasing(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsTableCorrelationNames(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsConvert( sal_Int32, sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsOrderByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsUnion(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::nullsAreSortedAtStart(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::nullsAreSortedLow(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString SAL_CALL MacabDatabaseMetaData::getURL(  )
{
    // if someday we support more than the default address book,
    // this method should return the URL which was used to create it
    return "sdbc:address:macab:";
}

OUString SAL_CALL MacabDatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getDriverName(  )
{
    return "macab";
}

OUString SAL_CALL MacabDatabaseMetaData::getDriverVersion()
{
    return MACAB_DRIVER_VERSION;
}

OUString SAL_CALL MacabDatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getDriverMajorVersion(  )
{
    return MACAB_DRIVER_VERSION_MAJOR;
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return TransactionIsolation::NONE;
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getDriverMinorVersion(  )
{
    return MACAB_DRIVER_VERSION_MINOR;
}

OUString SAL_CALL MacabDatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getStringFunctions(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString SAL_CALL MacabDatabaseMetaData::getNumericFunctions(  )
{
    return OUString();
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL MacabDatabaseMetaData::getMaxUserNameLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return true;
    }
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 )
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
        case ResultSetType::SCROLL_INSENSITIVE:
            return true;
    }
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::ownUpdatesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::ownDeletesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::ownInsertsAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::othersUpdatesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::othersDeletesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::othersInsertsAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::updatesAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::deletesAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::insertsAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL MacabDatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

Reference< XConnection > SAL_CALL MacabDatabaseMetaData::getConnection(  )
{
    return m_xConnection.get();
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getTableTypes(  )
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows = [&]
    {
        static constexpr OUStringLiteral aTable = u"TABLE";
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow(2);
        aRow[0] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[1] = new ORowSetValueDecorator(OUString(aTable));
        tmp.push_back(aRow);
        return tmp;
    }();
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getTypeInfo(  )
{
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows = [&]()
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
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getCatalogs(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCatalogs );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getSchemas(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eSchemas );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getColumnPrivileges(
    const Any&, const OUString&, const OUString&,
    const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumnPrivileges );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getColumns(
    const Any&,
    const OUString&,
    const OUString& tableNamePattern,
    const OUString& columnNamePattern)
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xRef = pResult;
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
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getTables(
    const Any&,
    const OUString&,
    const OUString&,
    const Sequence< OUString >& types)
{
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTables);
    Reference< XResultSet > xRef = pResult;

    // check whether we have tables in the requested types
    // for the moment, we answer only the "TABLE" table type
    // when no types are given at all, we return all the tables
    static constexpr OUStringLiteral aTable = u"TABLE";
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
        return xRef;

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
        aRow[4] = new ORowSetValueDecorator(OUString(aTable));
        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        tmp.push_back(aRow);

        for(i = 0; i < nGroups; i++)
        {
            aRow[3] = new ORowSetValueDecorator(xGroups[i]->getName());
            tmp.push_back(aRow);
        }
        return tmp;
    }();
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getProcedureColumns(
    const Any&, const OUString&,
    const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedureColumns );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getProcedures(
    const Any&, const OUString&,
    const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedures );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getVersionColumns(
    const Any&, const OUString&, const OUString& table )
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eVersionColumns);
    Reference< XResultSet > xRef = pResult;

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
    pResult->setRows(aRows);
    return xRef;
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getExportedKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eExportedKeys );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getImportedKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eImportedKeys );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getPrimaryKeys(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::ePrimaryKeys );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getIndexInfo(
    const Any&, const OUString&, const OUString&,
    sal_Bool, sal_Bool )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eIndexInfo );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getBestRowIdentifier(
    const Any&, const OUString&, const OUString&, sal_Int32,
    sal_Bool )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eBestRowIdentifier );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getTablePrivileges(
    const Any&, const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getCrossReference(
    const Any&, const OUString&,
    const OUString&, const Any&,
    const OUString&, const OUString& )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCrossReference );
}

Reference< XResultSet > SAL_CALL MacabDatabaseMetaData::getUDTs( const Any&, const OUString&, const OUString&, const Sequence< sal_Int32 >& )
{
    OSL_FAIL("Not implemented yet!");
    throw SQLException();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
