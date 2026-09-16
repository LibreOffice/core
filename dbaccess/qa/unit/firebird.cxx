/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <connectivity/predicateinput.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class FirebirdTest
    : public DBTestBase
{
public:
    void testEmptyDBConnection();
    void testIntegerDatabase();
    void testTdf132924();
    void testTdf153057();
    void testTdf153057_RangeFilter();
    void testTdf153057_FractionalFilter();
    void testTdf173589_TableStandardFilter();

    CPPUNIT_TEST_SUITE(FirebirdTest);
    CPPUNIT_TEST(testEmptyDBConnection);
    CPPUNIT_TEST(testIntegerDatabase);
    CPPUNIT_TEST(testTdf132924);
    CPPUNIT_TEST(testTdf153057);
    CPPUNIT_TEST(testTdf153057_RangeFilter);
    CPPUNIT_TEST(testTdf153057_FractionalFilter);
    CPPUNIT_TEST(testTdf173589_TableStandardFilter);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * Test the loading of an "empty" file, i.e. the embedded database has not yet
 * been initialised (as occurs when a new .odb is created and opened by base).
 */
void FirebirdTest::testEmptyDBConnection()
{
    createTempCopy(u"firebird_empty.odb");
    uno::Reference< XOfficeDatabaseDocument > xDocument =
        getDocumentForUrl(maTempFile.GetURL());

    getConnectionForDocument(xDocument);
}

/**
 * Test reading of integers from a known .odb to verify that the data
 * can still be read on all systems.
 */
void FirebirdTest::testIntegerDatabase()
{
    loadFromFile(u"firebird_integer_ods12.odb");
    uno::Reference< XOfficeDatabaseDocument > xDocument(mxComponent, UNO_QUERY_THROW);

    uno::Reference< XConnection > xConnection =
        getConnectionForDocument(xDocument);

    uno::Reference< XStatement > xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());

    uno::Reference< XResultSet > xResultSet = xStatement->executeQuery(
        u"SELECT * FROM TESTTABLE"_ustr);
    CPPUNIT_ASSERT(xResultSet.is());
    CPPUNIT_ASSERT(xResultSet->next());

    uno::Reference< XRow > xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT(xRow.is());
    uno::Reference< XColumnLocate > xColumnLocate(xRow, UNO_QUERY);
    CPPUNIT_ASSERT(xColumnLocate.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int16(-30000),
        xRow->getShort(xColumnLocate->findColumn(u"_SMALLINT"_ustr)));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2100000000),
        xRow->getInt(xColumnLocate->findColumn(u"_INT"_ustr)));
    CPPUNIT_ASSERT_EQUAL(SAL_CONST_INT64(-9000000000000000000),
        xRow->getLong(xColumnLocate->findColumn(u"_BIGINT"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr,
        xRow->getString(xColumnLocate->findColumn(u"_CHAR"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr,
        xRow->getString(xColumnLocate->findColumn(u"_VARCHAR"_ustr)));

    CPPUNIT_ASSERT(!xResultSet->next()); // Should only be one row
}

void FirebirdTest::testTdf132924()
{
    loadFromFile(u"tdf132924.odb");
    uno::Reference< XOfficeDatabaseDocument > xDocument(mxComponent, UNO_QUERY_THROW);
    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);

    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());

    uno::Reference<XResultSet> xResultSet = xStatement->executeQuery(u"SELECT * FROM AliasTest"_ustr);
    CPPUNIT_ASSERT(xResultSet.is());
    CPPUNIT_ASSERT(xResultSet->next());

    uno::Reference<XRow> xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT(xRow.is());
    uno::Reference<XColumnLocate> xColumnLocate(xRow, UNO_QUERY);
    CPPUNIT_ASSERT(xColumnLocate.is());

    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : The column name 'TestId' is not valid
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xRow->getShort(xColumnLocate->findColumn(u"TestId"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"TestName"_ustr, xRow->getString(xColumnLocate->findColumn(u"TestName"_ustr)));
}

/**
 * Test for tdf#153057: filtering on DATE, TIME and TIMESTAMP columns via
 * a string value.  Before the fix, OPreparedStatement::setString() had no
 * cases for DataType::DATE / TIME / TIMESTAMP and would throw
 * "Incorrect type for setString" when a filter string was applied to such
 * a column in Base.
 */
void FirebirdTest::testTdf153057()
{
    // Create a fresh embedded Firebird database in a temp file.
    createTempCopy(u"firebird_empty.odb");
    uno::Reference<XOfficeDatabaseDocument> xDocument = getDocumentForUrl(maTempFile.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    CPPUNIT_ASSERT(xConnection.is());

    // Create a table with DATE, TIME and TIMESTAMP columns and two rows.
    uno::Reference<XStatement> xStmt = xConnection->createStatement();
    xStmt->execute(u"CREATE TABLE \"TEMPORAL\" ("
                   " \"ID\"   INTEGER NOT NULL PRIMARY KEY,"
                   " \"D\"    DATE,"
                   " \"T\"    TIME,"
                   " \"TS\"   TIMESTAMP)"_ustr);

    xStmt->execute(
        u"INSERT INTO \"TEMPORAL\" VALUES(1, '2024-07-15', '14:30:00', '2024-07-15 14:30:00')"_ustr);
    xStmt->execute(
        u"INSERT INTO \"TEMPORAL\" VALUES(2, '2025-01-01', '09:00:00', '2025-01-01 09:00:00')"_ustr);
    xConnection->commit();

    // --- Test DATE filtering via setString ---
    // Before the fix this would throw "Incorrect type for setString".
    {
        uno::Reference<XPreparedStatement> xPS = xConnection->prepareStatement(
            u"SELECT \"ID\" FROM \"TEMPORAL\" WHERE \"D\" = ?"_ustr);
        uno::Reference<XParameters> xParams(xPS, UNO_QUERY_THROW);
        xParams->setString(1, u"2024-07-15"_ustr);

        uno::Reference<XResultSet> xRS = xPS->executeQuery();
        CPPUNIT_ASSERT_MESSAGE("DATE filter: expected at least one row", xRS->next());
        uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("DATE filter: wrong row returned", sal_Int32(1),
                                     xRow->getInt(1));
        CPPUNIT_ASSERT_MESSAGE("DATE filter: expected exactly one row", !xRS->next());
    }

    // --- Test TIME filtering via setString ---
    {
        uno::Reference<XPreparedStatement> xPS = xConnection->prepareStatement(
            u"SELECT \"ID\" FROM \"TEMPORAL\" WHERE \"T\" = ?"_ustr);
        uno::Reference<XParameters> xParams(xPS, UNO_QUERY_THROW);
        xParams->setString(1, u"09:00:00"_ustr);

        uno::Reference<XResultSet> xRS = xPS->executeQuery();
        CPPUNIT_ASSERT_MESSAGE("TIME filter: expected at least one row", xRS->next());
        uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("TIME filter: wrong row returned", sal_Int32(2),
                                     xRow->getInt(1));
        CPPUNIT_ASSERT_MESSAGE("TIME filter: expected exactly one row", !xRS->next());
    }

    // --- Test TIMESTAMP filtering via setString ---
    {
        uno::Reference<XPreparedStatement> xPS = xConnection->prepareStatement(
            u"SELECT \"ID\" FROM \"TEMPORAL\" WHERE \"TS\" = ?"_ustr);
        uno::Reference<XParameters> xParams(xPS, UNO_QUERY_THROW);
        xParams->setString(1, u"2025-01-01 09:00:00"_ustr);

        uno::Reference<XResultSet> xRS = xPS->executeQuery();
        CPPUNIT_ASSERT_MESSAGE("TIMESTAMP filter: expected at least one row", xRS->next());
        uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("TIMESTAMP filter: wrong row returned", sal_Int32(2),
                                     xRow->getInt(1));
        CPPUNIT_ASSERT_MESSAGE("TIMESTAMP filter: expected exactly one row", !xRS->next());
    }
}

/**
 * Test for tdf#153057: a range filter on a TIME column built via
 * XSingleSelectQueryComposer::setStructuredFilter(). This exercises
 * OPredicateInputController -> OSQLParser::buildDate() ->
 * DBTypeConversion::toTimeString(), a different code path from
 * testTdf153057()'s setString() test above. Before the fix,
 * a whole-second TIME value got ".000000000" into
 * the generated SQL, which Firebird rejects.
 */
void FirebirdTest::testTdf153057_RangeFilter()
{
    createTempCopy(u"firebird_empty.odb");
    uno::Reference<XOfficeDatabaseDocument> xDocument = getDocumentForUrl(maTempFile.GetURL());
    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    CPPUNIT_ASSERT(xConnection.is());

    uno::Reference<XStatement> xStmt = xConnection->createStatement();
    xStmt->execute(u"CREATE TABLE \"DURATIONS\" ("
                   " \"ID\" INTEGER NOT NULL PRIMARY KEY,"
                   " \"T\"  TIME)"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS\" VALUES(1, '00:01:00')"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS\" VALUES(2, '00:03:00')"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS\" VALUES(3, '00:05:00')"_ustr);
    xConnection->commit();

    // A table created directly via SQL isn't visible to the connection's
    // cached table container until it's explicitly refreshed (matches the
    // "Refresh Tables" UI command). Without this, the composer below can't
    // resolve "T" as a typed select column and silently falls back to an
    // untyped/unquoted filter-value path.
    uno::Reference<sdbcx::XTablesSupplier> xTablesSupplier(xConnection, UNO_QUERY_THROW);
    uno::Reference<util::XRefreshable> xTablesRefresh(xTablesSupplier->getTables(), UNO_QUERY_THROW);
    xTablesRefresh->refresh();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY_THROW);
    uno::Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);
    xComposer->setQuery(u"SELECT \"ID\", \"T\" FROM \"DURATIONS\""_ustr);

    beans::PropertyValue aLower, aUpper;
    aLower.Name = u"\"T\""_ustr;
    aLower.Handle = SQLFilterOperator::GREATER_EQUAL;
    aLower.Value <<= u"00:02:00"_ustr;
    aUpper.Name = u"\"T\""_ustr;
    aUpper.Handle = SQLFilterOperator::LESS_EQUAL;
    aUpper.Value <<= u"00:04:00"_ustr;

    Sequence<Sequence<beans::PropertyValue>> aFilter{ { aLower, aUpper } };

    // Before the fix: SQLException "conversion error from string ..." here,
    // because the generated literal was "... AND \"T\" <= '00:04:00.000000000'".
    xComposer->setStructuredFilter(aFilter);

    uno::Reference<XStatement> xQueryStmt = xConnection->createStatement();
    uno::Reference<XResultSet> xRS = xQueryStmt->executeQuery(xComposer->getQuery());
    uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("range filter: expected a row", xRS->next());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range filter: wrong row returned", sal_Int32(2), xRow->getInt(1));
    CPPUNIT_ASSERT_MESSAGE("range filter: expected exactly one row", !xRS->next());
}

/**
 * Regression test for a fractional-seconds TIME filter, distinct from
 * testTdf153057_RangeFilter() above.
 */
void FirebirdTest::testTdf153057_FractionalFilter()
{
    createTempCopy(u"firebird_empty.odb");
    uno::Reference<XOfficeDatabaseDocument> xDocument = getDocumentForUrl(maTempFile.GetURL());
    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    CPPUNIT_ASSERT(xConnection.is());

    uno::Reference<XStatement> xStmt = xConnection->createStatement();
    xStmt->execute(u"CREATE TABLE \"DURATIONS2\" ("
                   " \"ID\" INTEGER NOT NULL PRIMARY KEY,"
                   " \"T\"  TIME)"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS2\" VALUES(1, '00:01:00')"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS2\" VALUES(2, '00:03:00.1235')"_ustr);
    xStmt->execute(u"INSERT INTO \"DURATIONS2\" VALUES(3, '00:05:00')"_ustr);
    xConnection->commit();

    uno::Reference<sdbcx::XTablesSupplier> xTablesSupplier(xConnection, UNO_QUERY_THROW);
    uno::Reference<util::XRefreshable> xTablesRefresh(xTablesSupplier->getTables(), UNO_QUERY_THROW);
    xTablesRefresh->refresh();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY_THROW);
    uno::Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);
    xComposer->setQuery(u"SELECT \"ID\", \"T\" FROM \"DURATIONS2\""_ustr);

    // A value with genuine sub-decisecond precision (6 significant fractional
    // digits) should be rounded to 4 digits for Firebird
    beans::PropertyValue aEquals;
    aEquals.Name = u"\"T\""_ustr;
    aEquals.Handle = SQLFilterOperator::EQUAL;
    aEquals.Value <<= u"00:03:00.123456"_ustr;

    Sequence<Sequence<beans::PropertyValue>> aFilter{ { aEquals } };

    xComposer->setStructuredFilter(aFilter);

    uno::Reference<XStatement> xQueryStmt = xConnection->createStatement();
    uno::Reference<XResultSet> xRS = xQueryStmt->executeQuery(xComposer->getQuery());
    uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("fractional filter: expected a row", xRS->next());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("fractional filter: wrong row returned", sal_Int32(2), xRow->getInt(1));
    CPPUNIT_ASSERT_MESSAGE("fractional filter: expected exactly one row", !xRS->next());
}

/**
 * Regression test for tdf#173589: filtering a TIME column via the Standard
 * Filter dialog on a raw table (Table Data View) returned an
 * empty result.
 */
void FirebirdTest::testTdf173589_TableStandardFilter()
{
    createTempCopy(u"firebird_empty.odb");
    uno::Reference<XOfficeDatabaseDocument> xDocument = getDocumentForUrl(maTempFile.GetURL());
    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    CPPUNIT_ASSERT(xConnection.is());

    uno::Reference<XStatement> xStmt = xConnection->createStatement();
    xStmt->execute(u"CREATE TABLE \"TITLES\" ("
                   " \"ID\" INTEGER NOT NULL PRIMARY KEY,"
                   " \"Duration\"  TIME)"_ustr);
    xStmt->execute(u"INSERT INTO \"TITLES\" VALUES(1, '00:01:00')"_ustr);
    xStmt->execute(u"INSERT INTO \"TITLES\" VALUES(2, '00:05:00')"_ustr);
    xStmt->execute(u"INSERT INTO \"TITLES\" VALUES(3, '00:09:00')"_ustr);
    xConnection->commit();

    uno::Reference<sdbcx::XTablesSupplier> xTablesSupplier(xConnection, UNO_QUERY_THROW);
    uno::Reference<util::XRefreshable> xTablesRefresh(xTablesSupplier->getTables(), UNO_QUERY_THROW);
    xTablesRefresh->refresh();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY_THROW);
    uno::Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);
    xComposer->setQuery(u"SELECT \"ID\", \"Duration\" FROM \"TITLES\""_ustr);

    uno::Reference<container::XNameAccess> xColumns(
        uno::Reference<sdbcx::XColumnsSupplier>(xComposer, UNO_QUERY_THROW)->getColumns());
    uno::Reference<beans::XPropertySet> xColumn(xColumns->getByName(u"Duration"_ustr), UNO_QUERY_THROW);

    ::dbtools::OPredicateInputController aPredicateInput(getComponentContext(), xConnection);
    OUString sPredicateValue;
    aPredicateInput.getPredicateValue(u"00:05:00"_ustr, xColumn) >>= sPredicateValue;
    CPPUNIT_ASSERT_MESSAGE("tdf173589: predicate value must not be empty", !sPredicateValue.isEmpty());

    beans::PropertyValue aFilter;
    aFilter.Name = u"\"Duration\""_ustr;
    aFilter.Handle = SQLFilterOperator::LESS;
    aFilter.Value <<= sPredicateValue;

    Sequence<Sequence<beans::PropertyValue>> aFilterSeq{ { aFilter } };

    xComposer->setStructuredFilter(aFilterSeq);

    uno::Reference<XStatement> xQueryStmt = xConnection->createStatement();
    uno::Reference<XResultSet> xRS = xQueryStmt->executeQuery(xComposer->getQuery());
    uno::Reference<XRow> xRow(xRS, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("tdf173589: expected a row", xRS->next());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf173589: wrong row returned", sal_Int32(1), xRow->getInt(1));
    CPPUNIT_ASSERT_MESSAGE("tdf173589: expected exactly one row", !xRS->next());
}

CPPUNIT_TEST_SUITE_REGISTRATION(FirebirdTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
