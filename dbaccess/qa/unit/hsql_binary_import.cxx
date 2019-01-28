/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <hsqlimport.hxx>

#include <osl/process.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <test/unoapi_test.hxx>
#include <svtools/miscopt.hxx>

using namespace dbahsql;

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
    SvtMiscOptions aMiscOptions;
    osl_setEnvironment(OUString{ "DBACCESS_HSQL_MIGRATION" }.pData, OUString{ "1" }.pData);
}

void HsqlBinaryImportTest::testBinaryImport()
{
    // the migration requires the file to be writable
    utl::TempFile const temp(createTempCopy("hsqldb_migration_test.odb"));
    uno::Reference<XOfficeDatabaseDocument> const xDocument = getDocumentForUrl(temp.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    // at this point migration is already done

    uno::Reference<XStatement> statement = xConnection->createStatement();
    OUString sql{ "SELECT \"ID\", \"Power_value\", \"Power_name\", \"Retired\", "
                  "\"Birth_date\" FROM \"TestTable\" ORDER BY \"ID\"" };

    uno::Reference<XResultSet> xRes = statement->executeQuery(sql);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // assert first row
    CPPUNIT_ASSERT(xRes->next());
    constexpr sal_Int16 idExpected = 1;
    CPPUNIT_ASSERT_EQUAL(idExpected, xRow->getShort(1));
    CPPUNIT_ASSERT_EQUAL(OUString{ "45.32" }, xRow->getString(2)); // numeric
    CPPUNIT_ASSERT_EQUAL(OUString{ "laser eye" }, xRow->getString(3)); // varchar
    CPPUNIT_ASSERT(xRow->getBoolean(4)); // boolean

    css::util::Date date = xRow->getDate(5);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 15 }, date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 1 }, date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16{ 1996 }, date.Year);

    // assert second row
    CPPUNIT_ASSERT(xRes->next());
    constexpr sal_Int16 secondIdExpected = 2;
    CPPUNIT_ASSERT_EQUAL(secondIdExpected, xRow->getShort(1)); // ID
    CPPUNIT_ASSERT_EQUAL(OUString{ "54.12" }, xRow->getString(2)); // numeric
    CPPUNIT_ASSERT_EQUAL(OUString{ "telekinesis" }, xRow->getString(3)); // varchar
    CPPUNIT_ASSERT(!xRow->getBoolean(4)); // boolean

    date = xRow->getDate(5);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 26 }, date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16{ 2 }, date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16{ 1998 }, date.Year);

    closeDocument(uno::Reference<lang::XComponent>(xDocument, uno::UNO_QUERY));
}

CPPUNIT_TEST_SUITE_REGISTRATION(HsqlBinaryImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();
