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

class HsqlBinaryImportTest : public DBTestBase
{
public:
    void testBinaryImport();

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(HsqlBinaryImportTest);

    CPPUNIT_TEST(testBinaryImport);

    CPPUNIT_TEST_SUITE_END();
};

void HsqlBinaryImportTest::setUp()
{
    DBTestBase::setUp();
    osl_setEnvironment(u"DBACCESS_HSQL_MIGRATION"_ustr.pData, u"1"_ustr.pData);
}

void HsqlBinaryImportTest::testBinaryImport()
{
    bool oldValue = officecfg::Office::Common::Misc::ExperimentalMode::get();
    {
        std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(true, xChanges);
        xChanges->commit();
    }

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

    if (!oldValue)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, xChanges);
        xChanges->commit();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(HsqlBinaryImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();
