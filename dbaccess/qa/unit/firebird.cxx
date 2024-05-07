/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCloseable.hpp>

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

    CPPUNIT_TEST_SUITE(FirebirdTest);
    CPPUNIT_TEST(testEmptyDBConnection);
    CPPUNIT_TEST(testIntegerDatabase);
    CPPUNIT_TEST(testTdf132924);
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

    css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(false);
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

    css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(false);
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

    css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(FirebirdTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
