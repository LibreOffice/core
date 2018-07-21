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

#include "DatabaseMetaData.hxx"
#include "Util.hxx"

#include <ibase.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <FDatabaseMetaDataResultSet.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/Deferrability.hpp>

using namespace connectivity::firebird;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODatabaseMetaData::ODatabaseMetaData(Connection* _pCon)
: m_pConnection(_pCon)
{
    SAL_WARN_IF(!m_pConnection.is(), "connectivity.firebird",
            "ODatabaseMetaData::ODatabaseMetaData: No connection set!");
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

//----- Catalog Info -- UNSUPPORTED -------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getCatalogSeparator()
{
    return OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength()
{
    return -1;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm()
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::isCatalogAtStart()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInTableDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInDataManipulation(  )
{
    return false;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCatalogs()
{
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eCatalogs);
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions()
{
    return false;
}

//----- Schema Info -- UNSUPPORTED --------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInDataManipulation()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInTableDefinitions()
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength()
{
    return -1;
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm()
{
    return OUString();
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getSchemas()
{
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eSchemas);
}

//----- Max Sizes/Lengths -----------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength()
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize()
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength()
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength()
{
    return 31;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex()
{
    // TODO: No idea.
    // See: http://www.firebirdsql.org/en/firebird-technical-specifications/
    return 16;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength()
{
    return 32;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections()
{
    return 100; // Arbitrary
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable()
{
    // May however be smaller.
    // See: http://www.firebirdsql.org/en/firebird-technical-specifications/
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength()
{
    return 32767;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength()
{
    return 31;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTablesInSelect(  )
{
    return 0; // 0 means no limit
}


sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return false;
}

// ---- Identifiers -----------------------------------------------------------
// Only quoted identifiers are case sensitive, unquoted are case insensitive
OUString SAL_CALL ODatabaseMetaData::getIdentifierQuoteString()
{
    OUString aVal('"');
    return aVal;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseQuotedIdentifiers(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseQuotedIdentifiers()
{
    // TODO: confirm this -- the documentation is highly ambiguous
    // However it seems this should be true as quoted identifiers ARE
    // stored mixed case.
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers()
{
    return false;
}

// ---- Unquoted Identifiers -------------------------------------------------
// All unquoted identifiers are stored upper case.
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers()
{
    return true;
}

// ---- SQL Feature Support ---------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithAddColumn()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsAlterTableWithDropColumn()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable()
{
    // TODO: true if embedded, but unsure about remote server
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert(sal_Int32,
                                                     sal_Int32)
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames()
{
    return true;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    return 0; // 0 means no limit
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return true;
}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return false;
}
// ---- Data definition stuff -------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::
        supportsDataDefinitionAndDataManipulationTransactions()
{
    return false;
}
//----- Transaction Support --------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback()
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions()
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel(
        sal_Int32 aLevel)
{
    return  aLevel == TransactionIsolation::READ_UNCOMMITTED
           || aLevel == TransactionIsolation::READ_COMMITTED
           || aLevel == TransactionIsolation::REPEATABLE_READ
           || aLevel == TransactionIsolation::SERIALIZABLE;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation()
{
    return TransactionIsolation::REPEATABLE_READ;
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
    return true;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatements(  )
{
    return 0; // 0 means no limit
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 31; // TODO: confirm
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    return m_pConnection->isReadOnly();
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    return m_pConnection->isEmbedded();
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    // Unsure
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    // Unsure
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
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
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

OUString SAL_CALL ODatabaseMetaData::getURL()
{
    return m_pConnection->getConnectionURL();
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
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 1;
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
    return 31;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType(sal_Int32 setType)
{
    switch (setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return true;
        default:
            return false;
    }
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency(
        sal_Int32 aResultSetType,
        sal_Int32 aConcurrency)
{
    if (aResultSetType == ResultSetType::FORWARD_ONLY
        && aConcurrency == ResultSetConcurrency::READ_ONLY)
        return true;
    else
        return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates()
{
    // No batch support in firebird
    return false;
}

uno::Reference< XConnection > SAL_CALL ODatabaseMetaData::getConnection()
{
    return uno::Reference<XConnection>(m_pConnection.get());
}

// here follow all methods which return a resultset
// the first methods is an example implementation how to use this resultset
// of course you could implement it on your and you should do this because
// the general way is more memory expensive

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTableTypes);
    uno::Reference< XResultSet > xResultSet = pResultSet;

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aRow(2);

    aRow[0] = new ORowSetValueDecorator(); // unused

    // TODO Put these statics to one place
    // like postgreSQL's Statics class.

    aRow[1] = new ORowSetValueDecorator(OUString("TABLE"));
    aResults.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("VIEW"));
    aResults.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("SYSTEM TABLE"));
    aResults.push_back(aRow);

    pResultSet->setRows(aResults);
    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTypeInfo()
{
    SAL_INFO("connectivity.firebird", "getTypeInfo()");

    // this returns an empty resultset where the column-names are already set
    // in special the metadata of the resultset already returns the right columns
    ODatabaseMetaDataResultSet* pResultSet =
            new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTypeInfo);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    static ODatabaseMetaDataResultSet::ORows aResults;

    if(aResults.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow(19);

        // Common data
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue(); // Literal quote marks
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue(); // Literal quote marks
        aRow[7] = new ORowSetValueDecorator(true); // Nullable
        aRow[8] = new ORowSetValueDecorator(true); // Case sensitive
        aRow[10] = new ORowSetValueDecorator(false); // Is unsigned
        // FIXED_PREC_SCALE: docs state "can it be a money value? " however
        // in reality this causes Base to treat all numbers as money formatted
        // by default which is wrong (and formatting as money value is still
        // possible for all values).
        aRow[11] = new ORowSetValueDecorator(false);
        // Localised Type Name -- TODO: implement (but can be null):
        aRow[13] = new ORowSetValueDecorator();
        aRow[16] = new ORowSetValueDecorator();             // Unused
        aRow[17] = new ORowSetValueDecorator();             // Unused
        aRow[18] = new ORowSetValueDecorator(sal_Int16(10));// Radix

        // Char
        aRow[1] = new ORowSetValueDecorator(OUString("CHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::CHAR);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32765)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(OUString("length")); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // Varchar
        aRow[1] = new ORowSetValueDecorator(OUString("VARCHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::VARCHAR);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32765)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(OUString("length")); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // Binary (CHAR)
        // It is distinguished from Text type by its character set
        aRow[1] = new ORowSetValueDecorator(OUString("CHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::BINARY);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32765)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(OUString("length")); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::NONE)); // Searchable
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // Varbinary (VARCHAR)
        aRow[1] = new ORowSetValueDecorator(OUString("VARCHAR"));
        aRow[2] = new ORowSetValueDecorator(DataType::VARBINARY);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(32765)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(OUString("length")); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::NONE)); // Searchable

        // Clob (SQL_BLOB)
        aRow[1] = new ORowSetValueDecorator(OUString("BLOB")); // BLOB, with subtype 1
        aRow[2] = new ORowSetValueDecorator(DataType::CLOB);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(2147483647)); // Precision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // Longvarbinary (SQL_BLOB)
        // Distinguished from simple blob with a user-defined subtype.
        aRow[2] = new ORowSetValueDecorator(DataType::LONGVARBINARY);
        aResults.push_back(aRow);

        // Integer Types common
        {
            aRow[6] = new ORowSetValueDecorator(); // Create Params
            aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
            aRow[12] = new ORowSetValueDecorator(true); // Autoincrement
            aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
            aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        }
        // Smallint (SQL_SHORT)
        aRow[1] = new ORowSetValueDecorator(OUString("SMALLINT"));
        aRow[2] = new ORowSetValueDecorator(DataType::SMALLINT);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(5)); // Prevision
        aResults.push_back(aRow);
        // Integer (SQL_LONG)
        aRow[1] = new ORowSetValueDecorator(OUString("INTEGER"));
        aRow[2] = new ORowSetValueDecorator(DataType::INTEGER);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(10)); // Precision
        aResults.push_back(aRow);
        // Bigint (SQL_INT64)
        aRow[1] = new ORowSetValueDecorator(OUString("BIGINT"));
        aRow[2] = new ORowSetValueDecorator(DataType::BIGINT);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(20)); // Precision
        aResults.push_back(aRow);

        // Decimal Types common
        {
            aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
            aRow[12] = new ORowSetValueDecorator(true); // Autoincrement
        }

        aRow[6] = new ORowSetValueDecorator(OUString("PRECISION,SCALE")); // Create params
        // Numeric
        aRow[1] = new ORowSetValueDecorator(OUString("NUMERIC"));
        aRow[2] = new ORowSetValueDecorator(DataType::NUMERIC);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(15)); // Precision
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); // Minimum scale
        aRow[15] = new ORowSetValueDecorator(sal_Int16(15)); // Max scale
        aResults.push_back(aRow);
        // Decimal
        aRow[1] = new ORowSetValueDecorator(OUString("DECIMAL"));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(15)); // Precision
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); // Minimum scale
        aRow[15] = new ORowSetValueDecorator(sal_Int16(15)); // Max scale
        aResults.push_back(aRow);

        aRow[6] = new ORowSetValueDecorator(); // Create Params
        // Float (SQL_FLOAT)
        aRow[1] = new ORowSetValueDecorator(OUString("FLOAT"));
        aRow[2] = new ORowSetValueDecorator(DataType::FLOAT);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(7)); // Precision
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); // Minimum scale
        aRow[15] = new ORowSetValueDecorator(sal_Int16(7)); // Max scale
        aResults.push_back(aRow);
        // Double (SQL_DOUBLE)
        aRow[1] = new ORowSetValueDecorator(OUString("DOUBLE PRECISION"));
        aRow[2] = new ORowSetValueDecorator(DataType::DOUBLE);
        aRow[3] = new ORowSetValueDecorator(sal_Int16(15)); // Precision
        aRow[14] = new ORowSetValueDecorator(sal_Int16(1)); // Minimum scale
        aRow[15] = new ORowSetValueDecorator(sal_Int16(15)); // Max scale
        aResults.push_back(aRow);

        // TODO: no idea whether D_FLOAT corresponds to an sql type

        // SQL_TIMESTAMP
        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // SQL_TYPE_TIME
        aRow[1] = new ORowSetValueDecorator(OUString("TIME"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIME);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // SQL_TYPE_DATE
        aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::FULL)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // SQL_BLOB
        aRow[1] = new ORowSetValueDecorator(OUString("BLOB"));
        aRow[2] = new ORowSetValueDecorator(DataType::BLOB);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(0)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::NONE)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

        // SQL_BOOLEAN
        aRow[1] = new ORowSetValueDecorator(OUString("BOOLEAN"));
        aRow[2] = new ORowSetValueDecorator(DataType::BOOLEAN);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(1)); // Prevision = max length
        aRow[6] = new ORowSetValueDecorator(); // Create Params
        aRow[9] = new ORowSetValueDecorator(
                sal_Int16(ColumnSearch::BASIC)); // Searchable
        aRow[12] = new ORowSetValueDecorator(false); // Autoincrement
        aRow[14] = ODatabaseMetaDataResultSet::get0Value(); // Minimum scale
        aRow[15] = ODatabaseMetaDataResultSet::get0Value(); // Max scale
        aResults.push_back(aRow);

    }
    pResultSet->setRows(aResults);
    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumnPrivileges(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable,
        const OUString& sColumnNamePattern)
{
    SAL_INFO("connectivity.firebird", "getColumnPrivileges() with "
             "Table: " << sTable
             << " & ColumnNamePattern: " << sColumnNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumnPrivileges);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    static const char wld[] = "%";
    OUStringBuffer queryBuf(
            "SELECT "
            "priv.RDB$RELATION_NAME, "  // 1 Table name
            "priv.RDB$GRANTOR,"         // 2
            "priv.RDB$USER, "           // 3 Grantee
            "priv.RDB$PRIVILEGE, "      // 4
            "priv.RDB$GRANT_OPTION, "   // 5 is Grantable
            "priv.RDB$FIELD_NAME "      // 6 Column name
            "FROM RDB$USER_PRIVILEGES priv ");

    {
        OUString sAppend = "WHERE priv.RDB$RELATION_NAME = '%' ";
        queryBuf.append(sAppend.replaceAll("%", sTable));
    }
    if (!sColumnNamePattern.isEmpty())
    {
        OUString sAppend;
        if (sColumnNamePattern.match(wld))
            sAppend = "AND priv.RDB$FIELD_NAME LIKE '%' ";
        else
            sAppend = "AND priv.RDB$FIELD_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll(wld, sColumnNamePattern));
    }

    queryBuf.append(" ORDER BY priv.RDB$FIELD, "
                              "priv.RDB$PRIVILEGE");

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aCurrentRow(8);
    aCurrentRow[0] = new ORowSetValueDecorator(); // Unused
    aCurrentRow[1] = new ORowSetValueDecorator(); // 1. TABLE_CAT Unsupported
    aCurrentRow[2] = new ORowSetValueDecorator(); // 1. TABLE_SCHEM Unsupported

    while( rs->next() )
    {
        // 3. TABLE_NAME
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        // 4. COLUMN_NAME
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(6)));
        aCurrentRow[5] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2))); // 5. GRANTOR
        aCurrentRow[6] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(3))); // 6. GRANTEE
        aCurrentRow[7] = new ORowSetValueDecorator( ( xRow->getShort(5) == 1 ) ?
                    OUString("YES") : OUString("NO")); // 8. Grantable

        aResults.push_back(aCurrentRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/,
        const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern,
        const OUString& columnNamePattern)
{
    SAL_INFO("connectivity.firebird", "getColumns() with "
             "TableNamePattern: " << tableNamePattern <<
             " & ColumnNamePattern: " << columnNamePattern);

    OUStringBuffer queryBuf("SELECT "
        "relfields.RDB$RELATION_NAME, " // 1
        "relfields.RDB$FIELD_NAME, "    // 2
        "relfields.RDB$DESCRIPTION,"    // 3
        "relfields.RDB$DEFAULT_VALUE, " // 4
        "relfields.RDB$FIELD_POSITION, "// 5
        "fields.RDB$FIELD_TYPE, "       // 6
        "fields.RDB$FIELD_SUB_TYPE, "   // 7
        "fields.RDB$FIELD_LENGTH, "     // 8
        "fields.RDB$FIELD_PRECISION, "  // 9
        "fields.RDB$FIELD_SCALE, "      // 10
        // Specifically use relfields null flag -- the one in fields is used
        // for domains, whether a specific field is nullable is set in relfields,
        // this is also the one we manually fiddle when changin NULL/NOT NULL
        // (see Table.cxx)
        "relfields.RDB$NULL_FLAG, "      // 11
        "fields.RDB$CHARACTER_LENGTH, "   // 12
        "charset.RDB$CHARACTER_SET_NAME " // 13
        "FROM RDB$RELATION_FIELDS relfields "
        "JOIN RDB$FIELDS fields "
        "on (fields.RDB$FIELD_NAME = relfields.RDB$FIELD_SOURCE) "
        "LEFT JOIN RDB$CHARACTER_SETS charset "
        "on (fields.RDB$CHARACTER_SET_ID = charset.RDB$CHARACTER_SET_ID) "
        "WHERE (1 = 1) ");

    if (!tableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (tableNamePattern.match("%"))
            sAppend = "AND relfields.RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "AND relfields.RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", tableNamePattern));
    }

    if (!columnNamePattern.isEmpty())
    {
        OUString sAppend;
        if (columnNamePattern.match("%"))
            sAppend = "AND relfields.RDB$FIELD_NAME LIKE '%' ";
        else
            sAppend = "AND relfields.RDB$FIELD_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll("%", columnNamePattern));
    }

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XStatement > statement = m_pConnection->createStatement();
    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(19);

    aCurrentRow[0] =  new ORowSetValueDecorator(); // Unused -- numbering starts from 0
    aCurrentRow[1] =  new ORowSetValueDecorator(); // Catalog - can be null
    aCurrentRow[2] =  new ORowSetValueDecorator(); // Schema - can be null
    aCurrentRow[8] =  new ORowSetValueDecorator(); // Unused
    aCurrentRow[10] = new ORowSetValueDecorator(sal_Int32(10)); // Radix: fixed in FB
    aCurrentRow[14] = new ORowSetValueDecorator(); // Unused
    aCurrentRow[15] = new ORowSetValueDecorator(); // Unused

    while( rs->next() )
    {
        // 3. TABLE_NAME
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        // 4. Column Name
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        // 5. Datatype
        short aType = getFBTypeFromBlrType(xRow->getShort(6));
        short aScale = xRow->getShort(10);
        OUString sCharsetName = xRow->getString(13);
        // result field may be filled with spaces
        sCharsetName = sCharsetName.trim();
        ColumnTypeInfo aInfo(aType, xRow->getShort(7), aScale,
                xRow->getString(13));

        aCurrentRow[5] = new ORowSetValueDecorator(aInfo.getSdbcType());
        // 6. Typename (SQL_*)
        aCurrentRow[6] = new ORowSetValueDecorator(aInfo.getColumnTypeName());

        // 7. Column Sizes
        {
            sal_Int32 aColumnSize = 0;
            switch (aType)
            {
                case SQL_TEXT:
                case SQL_VARYING:
                    aColumnSize = xRow->getShort(12);
                    break;
                case SQL_SHORT:
                case SQL_LONG:
                case SQL_FLOAT:
                case SQL_DOUBLE:
                case SQL_D_FLOAT:
                case SQL_INT64:
                case SQL_QUAD:
                    aColumnSize = xRow->getShort(9);
                    break;
                case SQL_TIMESTAMP:
                case SQL_BLOB:
                case SQL_ARRAY:
                case SQL_TYPE_TIME:
                case SQL_TYPE_DATE:
                case SQL_NULL:
                    // TODO: implement.
                    break;
            }
            aCurrentRow[7] = new ORowSetValueDecorator(aColumnSize);
        }

        // 9. Decimal digits (scale)
        // fb stores a negative number
        aCurrentRow[9] = new ORowSetValueDecorator( static_cast<sal_Int16>(-aScale) );

        // 11. Nullable
        if (xRow->getShort(11))
        {
            aCurrentRow[11] = new ORowSetValueDecorator(ColumnValue::NO_NULLS);
        }
        else
        {
            aCurrentRow[11] = new ORowSetValueDecorator(ColumnValue::NULLABLE);
        }
        // 12. Comments -- may be omitted
        {
            OUString aDescription;
            uno::Reference< XBlob > xDescriptionBlob = xRow->getBlob(3);
            if (xDescriptionBlob.is())
            {
                sal_Int32 aBlobLength = static_cast<sal_Int32>(xDescriptionBlob->length());
                aDescription = OUString(reinterpret_cast<char*>(xDescriptionBlob->getBytes(0, aBlobLength).getArray()),
                                        aBlobLength,
                                        RTL_TEXTENCODING_UTF8);
            }
            aCurrentRow[12] = new ORowSetValueDecorator(aDescription);
        }
        // 13. Default --  may be omitted.
        {
            uno::Reference< XBlob > xDefaultValueBlob = xRow->getBlob(4);
            if (xDefaultValueBlob.is())
            {
                // TODO: Implement
            }
            aCurrentRow[13] = new ORowSetValueDecorator();
        }

        // 16. Bytes in Column for char
        if (aType == SQL_TEXT)
        {
            aCurrentRow[16] = new ORowSetValueDecorator(xRow->getShort(8));
        }
        else if (aType == SQL_VARYING)
        {
            aCurrentRow[16] = new ORowSetValueDecorator(sal_Int32(32767));
        }
        else
        {
            aCurrentRow[16] = new ORowSetValueDecorator(sal_Int32(0));
        }
        // 17. Index of column
        {
            short nColumnNumber = xRow->getShort(5);
            // Firebird stores column numbers beginning with 0 internally
            // SDBC expects column numbering to begin with 1.
            aCurrentRow[17] = new ORowSetValueDecorator(sal_Int32(nColumnNumber + 1));
        }
        // 18. Is nullable
        if (xRow->getShort(9))
        {
            aCurrentRow[18] = new ORowSetValueDecorator(OUString("NO"));
        }
        else
        {
            aCurrentRow[18] = new ORowSetValueDecorator(OUString("YES"));
        }

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eColumns);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/,
        const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern,
        const Sequence< OUString >& types)
{
    SAL_INFO("connectivity.firebird", "getTables() with "
             "TableNamePattern: " << tableNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    static const char wld[] = "%";
    OUStringBuffer queryBuf(
            "SELECT "
            "RDB$RELATION_NAME, "
            "RDB$SYSTEM_FLAG, "
            "RDB$RELATION_TYPE, "
            "RDB$DESCRIPTION, "
            "RDB$VIEW_BLR "
            "FROM RDB$RELATIONS "
            "WHERE ");

    // TODO: GLOBAL TEMPORARY, LOCAL TEMPORARY, ALIAS, SYNONYM
    if ((types.getLength() == 0) || (types.getLength() == 1 && types[0].match(wld)))
    {
        // All table types? I.e. includes system tables.
        queryBuf.append("(RDB$RELATION_TYPE = 0 OR RDB$RELATION_TYPE = 1) ");
    }
    else
    {
        queryBuf.append("( (0 = 1) ");
        for (int i = 0; i < types.getLength(); i++)
        {
            if (types[i] == "SYSTEM TABLE")
                queryBuf.append("OR (RDB$SYSTEM_FLAG = 1 AND RDB$VIEW_BLR IS NULL) ");
            else if (types[i] == "TABLE")
                queryBuf.append("OR (RDB$SYSTEM_FLAG IS NULL OR RDB$SYSTEM_FLAG = 0 AND RDB$VIEW_BLR IS NULL) ");
            else if (types[i] == "VIEW")
                queryBuf.append("OR (RDB$SYSTEM_FLAG IS NULL OR RDB$SYSTEM_FLAG = 0 AND RDB$VIEW_BLR IS NOT NULL) ");
            else
                throw SQLException(); // TODO: implement other types, see above.
        }
        queryBuf.append(") ");
    }

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
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aCurrentRow(6);
    aCurrentRow[0] = new ORowSetValueDecorator(); // 0. Unused
    aCurrentRow[1] = new ORowSetValueDecorator(); // 1. Table_Cat Unsupported
    aCurrentRow[2] = new ORowSetValueDecorator(); // 2. Table_Schem Unsupported

    while( rs->next() )
    {
        // 3. TABLE_NAME
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        // 4. TABLE_TYPE
        {
            // TODO: check this as the docs are a bit unclear.
            sal_Int16 nSystemFlag = xRow->getShort(2);
            sal_Int16 nTableType  = xRow->getShort(3);
            xRow->getBlob(5); // We have to retrieve a column to verify it is null.
            bool aIsView      = !xRow->wasNull();
            OUString sTableType;

            if (nSystemFlag == 1)
            {
                sTableType = "SYSTEM TABLE";
            }
            else if (aIsView)
            {
                sTableType = "VIEW";
            }
            else
            {
                if (nTableType == 0)
                    sTableType = "TABLE";
            }

            aCurrentRow[4] = new ORowSetValueDecorator(sTableType);
        }
        // 5. REMARKS
        {
            uno::Reference< XBlob > xBlob   = xRow->getBlob(4);
            OUString sDescription;

            if (xBlob.is())
            {
                // TODO: we should actually be using CLOB here instead.
                // However we haven't implemented CLOB yet, so use BLOB.
                sal_Int32 aBlobLength = static_cast<sal_Int32>(xBlob->length());
                sDescription = OUString(reinterpret_cast<char*>(xBlob->getBytes(0, aBlobLength).getArray()),
                                        aBlobLength,
                                        RTL_TEXTENCODING_UTF8);
            }

            aCurrentRow[5] = new ORowSetValueDecorator(sDescription);
        }

        aResults.push_back(aCurrentRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedureColumns(
    const Any&, const OUString&,
    const OUString&, const OUString& )
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eProcedureColumns);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getProcedures(
    const Any&, const OUString&,
    const OUString& )
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eProcedures);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getVersionColumns(
    const Any&, const OUString&, const OUString& )
{
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eVersionColumns);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getExportedKeys(
    const Any&, const OUString&, const OUString& )
{
    // List the columns in a table which are foreign keys. This is actually
    // never used anywhere in the LO codebase currently. Retrieval from firebird
    // requires using a 5-table join.
    SAL_WARN("connectivity.firebird", "Not yet implemented");
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eExportedKeys);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getImportedKeys(
    const Any&, const OUString&, const OUString& table )
{
    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eImportedKeys);
    uno::Reference< XResultSet > xResultSet = pResultSet;

    uno::Reference< XStatement > statement = m_pConnection->createStatement();
    OUString sSQL;

    sSQL = "SELECT "
           "RDB$REF_CONSTRAINTS.RDB$UPDATE_RULE, " // 1 update rule
           "RDB$REF_CONSTRAINTS.RDB$DELETE_RULE, " // 2 delete rule
           "RDB$REF_CONSTRAINTS.RDB$CONST_NAME_UQ, " // 3 primary or unique key name
           "RDB$REF_CONSTRAINTS.RDB$CONSTRAINT_NAME, " // 4 foreign key name
           "PRIM.RDB$DEFERRABLE, " // 5 deferrability
           "PRIM.RDB$INITIALLY_DEFERRED, " // 6 deferrability
           "PRIM.RDB$RELATION_NAME, " // 7 PK table name
           "PRIMARY_INDEX.RDB$FIELD_NAME, " // 8 PK column name
           "PRIMARY_INDEX.RDB$FIELD_POSITION, " // 9 PK sequence number
           "FOREI.RDB$RELATION_NAME, " // 10 FK table name
           "FOREIGN_INDEX.RDB$FIELD_NAME " // 11 FK column name
           "FROM RDB$REF_CONSTRAINTS "
           "INNER JOIN RDB$RELATION_CONSTRAINTS AS PRIM "
           "ON RDB$REF_CONSTRAINTS.RDB$CONST_NAME_UQ = PRIM.RDB$CONSTRAINT_NAME "
           "INNER JOIN RDB$RELATION_CONSTRAINTS AS FOREI "
           "ON RDB$REF_CONSTRAINTS.RDB$CONSTRAINT_NAME = FOREI.RDB$CONSTRAINT_NAME "
           "INNER JOIN RDB$INDEX_SEGMENTS AS PRIMARY_INDEX "
           "ON PRIM.RDB$INDEX_NAME = PRIMARY_INDEX.RDB$INDEX_NAME "
           "INNER JOIN RDB$INDEX_SEGMENTS AS FOREIGN_INDEX "
           "ON FOREI.RDB$INDEX_NAME = FOREIGN_INDEX.RDB$INDEX_NAME "
           "WHERE FOREI.RDB$CONSTRAINT_TYPE = 'FOREIGN KEY' "
           "AND FOREI.RDB$RELATION_NAME = '"+ table +"'";

    uno::Reference< XResultSet > rs = statement->executeQuery(sSQL);
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(15);

    // TODO is it necessary to initialize these?
    aCurrentRow[0] = new ORowSetValueDecorator(); // Unused
    aCurrentRow[1] = new ORowSetValueDecorator(); // PKTABLE_CAT unsupported
    aCurrentRow[2] = new ORowSetValueDecorator(); // PKTABLE_SCHEM unsupported
    aCurrentRow[5] = new ORowSetValueDecorator(); // FKTABLE_CAT unsupported
    aCurrentRow[6] = new ORowSetValueDecorator(); // FKTABLE_SCHEM unsupported

    std::map< OUString,sal_Int32> aRuleMap;
    aRuleMap[ OUString("CASCADE")] = KeyRule::CASCADE;
    aRuleMap[ OUString("RESTRICT")] = KeyRule::RESTRICT;
    aRuleMap[ OUString("SET NULL")] = KeyRule::SET_NULL;
    aRuleMap[ OUString("SET DEFAULT")] = KeyRule::SET_DEFAULT;
    aRuleMap[ OUString("NO ACTION")] = KeyRule::NO_ACTION;

    while(rs->next())
    {
        aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(7))); // PK table
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(8))); // PK column
        aCurrentRow[7] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(10))); // FK table
        aCurrentRow[8] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(11))); // FK column

        aCurrentRow[9] = new ORowSetValueDecorator(xRow->getShort(9)); // PK sequence number
        aCurrentRow[10] = new ORowSetValueDecorator(aRuleMap[sanitizeIdentifier(xRow->getString(1))]); // update role
        aCurrentRow[11] = new ORowSetValueDecorator(aRuleMap[sanitizeIdentifier(xRow->getString(2))]); // delete role

        aCurrentRow[12] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(4))); // FK name
        aCurrentRow[13] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(3))); // PK name

        aCurrentRow[14] = new ORowSetValueDecorator(Deferrability::NONE); // deferrability

        // deferrability is currently not supported, but may be supported in the future.
        /*
        aCurrentRow[14] = (xRow->getString(5) == "NO" ?
                          new ORowSetValueDecorator(Deferrability::NONE)
                        : (xRow->getString(6) == "NO" ?
                            new ORowSetValueDecorator(Deferrability::INITIALLY_IMMEDIATE)
                          : new ORowSetValueDecorator(Deferrability::INITIALLY_DEFERRED));
        */

        aResults.push_back(aCurrentRow);
    }

    pResultSet->setRows( aResults );
    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getPrimaryKeys(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable)
{
    SAL_INFO("connectivity.firebird", "getPrimaryKeys() with "
             "Table: " << sTable);

    OUStringBuffer aQueryBuf("SELECT "
        "constr.RDB$RELATION_NAME, "    // 1. Table Name
        "inds.RDB$FIELD_NAME, "         // 2. Column Name
        "inds.RDB$FIELD_POSITION, "     // 3. Sequence Number
        "constr.RDB$CONSTRAINT_NAME "   // 4 Constraint name
        "FROM RDB$RELATION_CONSTRAINTS constr "
        "JOIN RDB$INDEX_SEGMENTS inds "
        "on (constr.RDB$INDEX_NAME = inds.RDB$INDEX_NAME) ");

    OUString sAppend = "WHERE constr.RDB$RELATION_NAME = '%' ";
    aQueryBuf.append(sAppend.replaceAll("%", sTable));

    aQueryBuf.append("AND constr.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY' "
                    "ORDER BY inds.RDB$FIELD_NAME");

    OUString sQuery = aQueryBuf.makeStringAndClear();

    uno::Reference< XStatement > xStatement = m_pConnection->createStatement();
    uno::Reference< XResultSet > xRs = xStatement->executeQuery(sQuery);
    uno::Reference< XRow > xRow( xRs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(7);

    aCurrentRow[0] =  new ORowSetValueDecorator(); // Unused -- numbering starts from 0
    aCurrentRow[1] =  new ORowSetValueDecorator(); // Catalog - can be null
    aCurrentRow[2] =  new ORowSetValueDecorator(); // Schema - can be null

    while(xRs->next())
    {
        // 3. Table Name
        if (xRs->getRow() == 1) // Table name doesn't change, so only retrieve once
        {
            aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        }
        // 4. Column Name
        aCurrentRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        // 5. KEY_SEQ (which key in the sequence)
        aCurrentRow[5] = new ORowSetValueDecorator(xRow->getShort(3));
        // 6. Primary Key Name
        aCurrentRow[6] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(4)));

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::ePrimaryKeys);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getIndexInfo(
        const Any& /*aCatalog*/,
        const OUString& /*sSchema*/,
        const OUString& sTable,
        sal_Bool bIsUnique,
        sal_Bool) // TODO: what is bIsApproximate?

{
    // Apparently this method can also return a "tableIndexStatistic"
    // However this is only mentioned in XDatabaseMetaData.idl (whose comments
    // are duplicated in the postgresql driver), and is otherwise undocumented.
    SAL_INFO("connectivity.firebird", "getPrimaryKeys() with "
             "Table: " << sTable);

    OUStringBuffer aQueryBuf("SELECT "
        "indices.RDB$RELATION_NAME, "               // 1. Table Name
        "index_segments.RDB$FIELD_NAME, "           // 2. Column Name
        "index_segments.RDB$FIELD_POSITION, "       // 3. Sequence Number
        "indices.RDB$INDEX_NAME, "                  // 4. Index name
        "indices.RDB$UNIQUE_FLAG, "                 // 5. Unique Flag
        "indices.RDB$INDEX_TYPE "                   // 6. Index Type
        "FROM RDB$INDICES indices "
        "JOIN RDB$INDEX_SEGMENTS index_segments "
        "on (indices.RDB$INDEX_NAME = index_segments.RDB$INDEX_NAME) "
        "WHERE indices.RDB$RELATION_NAME = '" + sTable + "' "
        "AND (indices.RDB$SYSTEM_FLAG = 0) ");
    // Not sure whether we should exclude system indices, but otoh. we never
    // actually deal with system tables (system indices only apply to system
    // tables) within the GUI.

    // Only filter if true (according to the docs), i.e.:
    // If false we return all indices, if true we return only unique indices
    if (bIsUnique)
        aQueryBuf.append("AND (indices.RDB$UNIQUE_FLAG = 1) ");

    OUString sQuery = aQueryBuf.makeStringAndClear();

    uno::Reference< XStatement > xStatement = m_pConnection->createStatement();
    uno::Reference< XResultSet > xRs = xStatement->executeQuery(sQuery);
    uno::Reference< XRow > xRow( xRs, UNO_QUERY_THROW );

    ODatabaseMetaDataResultSet::ORows aResults;
    ODatabaseMetaDataResultSet::ORow aCurrentRow(14);

    aCurrentRow[0] = new ORowSetValueDecorator(); // Unused -- numbering starts from 0
    aCurrentRow[1] = new ORowSetValueDecorator(); // Catalog - can be null
    aCurrentRow[2] = new ORowSetValueDecorator(); // Schema - can be null
    aCurrentRow[5] = new ORowSetValueDecorator(); // Index Catalog -- can be null
    // According to wikipedia firebird uses clustered indices.
    // The documentation does not specifically seem to specify this.
    aCurrentRow[7] = new ORowSetValueDecorator(IndexType::CLUSTERED); // 7. INDEX TYPE
    aCurrentRow[13] = new ORowSetValueDecorator(); // Filter Condition -- can be null

    while(xRs->next())
    {
        // 3. Table Name
        if (xRs->getRow() == 1) // Table name doesn't change, so only retrieve once
        {
            aCurrentRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        }

        // 4. NON_UNIQUE -- i.e. specifically negate here.
        aCurrentRow[4] = new ORowSetValueDecorator(xRow->getShort(5) == 0);
        // 6. INDEX NAME
        aCurrentRow[6] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(4)));

        // 8. ORDINAL POSITION
        aCurrentRow[8] = new ORowSetValueDecorator(xRow->getShort(3));
        // 9. COLUMN NAME
        aCurrentRow[9] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2)));
        // 10. ASC(ending)/DESC(ending)
        if (xRow->getShort(6) == 1)
            aCurrentRow[10] = new ORowSetValueDecorator(OUString("D"));
        else
            aCurrentRow[10] = new ORowSetValueDecorator(OUString("A"));
        // TODO: double check this^^^, doesn't seem to be officially documented anywhere.
        // 11. CARDINALITY
        aCurrentRow[11] = new ORowSetValueDecorator(sal_Int32(0)); // TODO: determine how to do this
        // 12. PAGES
        aCurrentRow[12] = new ORowSetValueDecorator(sal_Int32(0)); // TODO: determine how to do this

        aResults.push_back(aCurrentRow);
    }
    ODatabaseMetaDataResultSet* pResultSet = new
            ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::ePrimaryKeys);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getBestRowIdentifier(
    const Any&, const OUString&, const OUString&, sal_Int32,
    sal_Bool )
{
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eBestRowIdentifier);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*aCatalog*/,
        const OUString& /*sSchemaPattern*/,
        const OUString& sTableNamePattern)
{
    SAL_INFO("connectivity.firebird", "getTablePrivileges() with "
             "TableNamePattern: " << sTableNamePattern);

    ODatabaseMetaDataResultSet* pResultSet = new
        ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTablePrivileges);
    uno::Reference< XResultSet > xResultSet = pResultSet;
    uno::Reference< XStatement > statement = m_pConnection->createStatement();

    // TODO: column specific privileges are included, we may need
    // to have WHERE RDB$FIELD_NAME = NULL or similar.
    static const char wld[] = "%";
    OUStringBuffer queryBuf(
            "SELECT "
            "priv.RDB$RELATION_NAME, "  // 1
            "priv.RDB$GRANTOR,"         // 2
            "priv.RDB$USER, "           // 3 Grantee
            "priv.RDB$PRIVILEGE, "      // 4
            "priv.RDB$GRANT_OPTION "    // 5 is Grantable
            "FROM RDB$USER_PRIVILEGES priv ");

    if (!sTableNamePattern.isEmpty())
    {
        OUString sAppend;
        if (sTableNamePattern.match(wld))
            sAppend = "WHERE priv.RDB$RELATION_NAME LIKE '%' ";
        else
            sAppend = "WHERE priv.RDB$RELATION_NAME = '%' ";

        queryBuf.append(sAppend.replaceAll(wld, sTableNamePattern));
    }
    queryBuf.append(" ORDER BY priv.RDB$RELATION_TYPE, "
                              "priv.RDB$RELATION_NAME, "
                              "priv.RDB$PRIVILEGE");

    OUString query = queryBuf.makeStringAndClear();

    uno::Reference< XResultSet > rs = statement->executeQuery(query.getStr());
    uno::Reference< XRow > xRow( rs, UNO_QUERY_THROW );
    ODatabaseMetaDataResultSet::ORows aResults;

    ODatabaseMetaDataResultSet::ORow aRow(8);
    aRow[0] = new ORowSetValueDecorator(); // Unused
    aRow[1] = new ORowSetValueDecorator(); // TABLE_CAT unsupported
    aRow[2] = new ORowSetValueDecorator(); // TABLE_SCHEM unsupported.

    while( rs->next() )
    {
        // 3. TABLE_NAME
        aRow[3] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(1)));
        aRow[4] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(2))); // 4. GRANTOR
        aRow[5] = new ORowSetValueDecorator(sanitizeIdentifier(xRow->getString(3))); // 5. GRANTEE
        aRow[6] = new ORowSetValueDecorator(xRow->getString(4)); // 6. Privilege
        aRow[7] = new ORowSetValueDecorator(bool(xRow->getBoolean(5))); // 7. Is Grantable

        aResults.push_back(aRow);
    }

    pResultSet->setRows( aResults );

    return xResultSet;
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getCrossReference(
    const Any&, const OUString&,
    const OUString&, const Any&,
    const OUString&, const OUString& )
{
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eCrossReference);
}

uno::Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any&, const OUString&, const OUString&, const Sequence< sal_Int32 >& )
{
    OSL_FAIL("Not implemented yet!");
    // TODO implement
    return new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eUDTs);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
