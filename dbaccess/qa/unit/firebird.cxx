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

    CPPUNIT_TEST_SUITE(FirebirdTest);
    CPPUNIT_TEST(testEmptyDBConnection);
    CPPUNIT_TEST(testIntegerDatabase);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * Test the loading of an "empty" file, i.e. the embedded database has not yet
 * been initialised (as occurs when a new .odb is created and opened by base).
 */
void FirebirdTest::testEmptyDBConnection()
{
    uno::Reference< XOfficeDatabaseDocument > xDocument =
        getDocumentForFileName("firebird_empty.odb");

    getConnectionForDocument(xDocument);
}

/**
 * Test reading of integers from a known .odb to verify that the data
 * can still be read on all systems.
 */
void FirebirdTest::testIntegerDatabase()
{
    uno::Reference< XOfficeDatabaseDocument > xDocument =
        getDocumentForFileName("firebird_integer_x64le.odb");

    uno::Reference< XConnection > xConnection =
        getConnectionForDocument(xDocument);

    uno::Reference< XStatement > xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());

    uno::Reference< XResultSet > xResultSet = xStatement->executeQuery(
        "SELECT * FROM TESTTABLE");
    CPPUNIT_ASSERT(xResultSet.is());
    CPPUNIT_ASSERT(xResultSet->next());

    uno::Reference< XRow > xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT(xRow.is());
    uno::Reference< XColumnLocate > xColumnLocate(xRow, UNO_QUERY);
    CPPUNIT_ASSERT(xColumnLocate.is());

    CPPUNIT_ASSERT(sal_Int16(-30000) ==
        xRow->getShort(xColumnLocate->findColumn("_SMALLINT")));
    CPPUNIT_ASSERT(sal_Int32(-2100000000) ==
        xRow->getInt(xColumnLocate->findColumn("_INT")));
    CPPUNIT_ASSERT(SAL_CONST_INT64(-9000000000000000000) ==
        xRow->getLong(xColumnLocate->findColumn("_BIGINT")));
    CPPUNIT_ASSERT(OUString("5") ==
        xRow->getString(xColumnLocate->findColumn("_CHAR")));
    CPPUNIT_ASSERT(OUString("5") ==
        xRow->getString(xColumnLocate->findColumn("_VARCHAR")));

    CPPUNIT_ASSERT(!xResultSet->next()); // Should only be one row
}

CPPUNIT_TEST_SUITE_REGISTRATION(FirebirdTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
