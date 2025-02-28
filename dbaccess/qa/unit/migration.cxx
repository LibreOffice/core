/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <osl/process.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <officecfg/Office/Common.hxx>

class MigrationTest : public DBTestBase
{
public:
    void testBinaryImport();
    void testNumbers();
    void testTime();

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(MigrationTest);

    CPPUNIT_TEST(testBinaryImport);
    CPPUNIT_TEST(testNumbers);
    CPPUNIT_TEST(testTime);

    CPPUNIT_TEST_SUITE_END();
};

void MigrationTest::setUp()
{
    DBTestBase::setUp();
    osl_setEnvironment(u"DBACCESS_HSQL_MIGRATION"_ustr.pData, u"1"_ustr.pData);

    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, xChanges);
    xChanges->commit();
}

void MigrationTest::tearDown()
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(false, xChanges);
    xChanges->commit();

    DBTestBase::tearDown();
}

void MigrationTest::testBinaryImport()
{
    // the migration requires the file to be writable
    createTempCopy(u"hsqldb_migration_test.odb");
    uno::Reference<XOfficeDatabaseDocument> const xDocument
        = getDocumentForUrl(maTempFile.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    // at this point migration is already done

    uno::Reference<XStatement> statement = xConnection->createStatement();

    uno::Reference<XResultSet> xRes
        = statement->executeQuery(u"SELECT \"ID\", \"Power_value\", \"Power_name\", \"Retired\", "
                                  "\"Birth_date\" FROM \"TestTable\" ORDER BY \"ID\""_ustr);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // assert first row
    CPPUNIT_ASSERT(xRes->next());
    constexpr sal_Int16 idExpected = 1;
    CPPUNIT_ASSERT_EQUAL(idExpected, xRow->getShort(1));
    CPPUNIT_ASSERT_EQUAL(u"45.32"_ustr, xRow->getString(2)); // numeric
    CPPUNIT_ASSERT_EQUAL(u"laser eye"_ustr, xRow->getString(3)); // varchar
    CPPUNIT_ASSERT(xRow->getBoolean(4)); // boolean

    css::util::Date date = xRow->getDate(5);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 15 }, date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 1 }, date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16{ 1996 }, date.Year);

    // assert second row
    CPPUNIT_ASSERT(xRes->next());
    constexpr sal_Int16 secondIdExpected = 2;
    CPPUNIT_ASSERT_EQUAL(secondIdExpected, xRow->getShort(1)); // ID
    CPPUNIT_ASSERT_EQUAL(u"54.12"_ustr, xRow->getString(2)); // numeric
    CPPUNIT_ASSERT_EQUAL(u"telekinesis"_ustr, xRow->getString(3)); // varchar
    CPPUNIT_ASSERT(!xRow->getBoolean(4)); // boolean

    date = xRow->getDate(5);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 26 }, date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 2 }, date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16{ 1998 }, date.Year);

    // assert third row
    CPPUNIT_ASSERT(xRes->next());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xRow->getShort(1)); // ID
    CPPUNIT_ASSERT_EQUAL(u"12.00"_ustr, xRow->getString(2)); // numeric
    CPPUNIT_ASSERT_EQUAL(u"mind-reading"_ustr, xRow->getString(3)); // varchar
    CPPUNIT_ASSERT(xRow->getBoolean(4)); // boolean

    date = xRow->getDate(5);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 20 }, date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 5 }, date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16{ 1967 }, date.Year);
}

void MigrationTest::testNumbers()
{
    struct expect_t
    {
        sal_Int16 id;
        OUString number;
    };

    const expect_t expect[] = {
        { 1, u"0.0"_ustr },   { 2, u"25.0"_ustr },  { 3, u"26.0"_ustr },
        { 4, u"30.4"_ustr },  { 5, u"45.8"_ustr },  { 6, u"-25.0"_ustr },
        { 7, u"-26.0"_ustr }, { 8, u"-30.4"_ustr }, { 9, u"-45.8"_ustr },
    };

    // the migration requires the file to be writable
    createTempCopy(u"tdf126268.odb");
    uno::Reference<XOfficeDatabaseDocument> const xDocument
        = getDocumentForUrl(maTempFile.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);

    // select basically everything from the .odb
    uno::Reference<XStatement> statement = xConnection->createStatement();

    uno::Reference<XResultSet> xRes
        = statement->executeQuery(u"SELECT ID, Column1, Column2 FROM tableTest ORDER BY ID"_ustr);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // check result
    for (auto& e : expect)
    {
        CPPUNIT_ASSERT(xRes->next());
        CPPUNIT_ASSERT_EQUAL(e.id, xRow->getShort(1));
        CPPUNIT_ASSERT_EQUAL(e.number, xRow->getString(2)); //decimal
        CPPUNIT_ASSERT_EQUAL(e.number, xRow->getString(3)); //numeric
    }
    CPPUNIT_ASSERT(!xRes->next());
}

void MigrationTest::testTime()
{
    struct expect_t
    {
        sal_Int16 id;
        sal_Int16 h, m, s;
    };

    /* The values here assume that our results are in UTC.  However,
       tdf#119675 "Firebird: Migration: User dialog to set treatment of
       datetime and time values during migration" is going to change the
       final result of migration. If that change is implemented below
       the level we are testing, this test will have to allow for or set
       the destination timezone.
     */
    const expect_t expect[]
        = { { 0, 15, 10, 10 }, { 1, 23, 30, 30 }, { 2, 5, 0, 0 },  { 3, 4, 30, 0 },
            { 4, 3, 15, 10 },  { 5, 5, 0, 0 },    { 6, 3, 22, 22 } };

    // the migration requires the file to be writable
    createTempCopy(u"tdf119625.odb");
    uno::Reference<XOfficeDatabaseDocument> const xDocument
        = getDocumentForUrl(maTempFile.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    // at this point migration is already done
    /* In the presence of tdf#119625, terminal already has messages

           *value exceeds the range for a valid time
           caused by
           'isc_dsql_execute'

           warn:dbaccess:22435:22435:dbaccess/source/filter/hsqldb/hsqlimport.cxx:373: Error during migration

       In this case, we do not expect anything good from the following
       code, but I (tje, 2018-09-04) do not know how to detect this
       situation.  In particular, the migration has been observed to
       create the destination table (but truncated after the first
       row), and xConnection.is() returns true.
    */

    // select basically everything from the .odb
    uno::Reference<XStatement> statement = xConnection->createStatement();

    uno::Reference<XResultSet> xRes = statement->executeQuery(u"  SELECT id, tst_dt, tst_d, tst_t "
                                                              "    FROM tst_data "
                                                              "ORDER BY id"_ustr);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // check result
    for (auto& e : expect)
    {
        CPPUNIT_ASSERT(xRes->next());
        CPPUNIT_ASSERT_EQUAL(xRow->getShort(1), e.id);
        auto time_got = xRow->getTime(4);
        auto time_expected = css::util::Time(0, e.s, e.m, e.h, false);
        auto equal_times = time_got == time_expected;
        CPPUNIT_ASSERT(equal_times);
    }
    CPPUNIT_ASSERT(!xRes->next());
}

CPPUNIT_TEST_SUITE_REGISTRATION(MigrationTest);

CPPUNIT_PLUGIN_IMPLEMENT();
