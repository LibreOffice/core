/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class CRMDBTest : public DBTestBase
{
public:
    void testCRMDatabase();

    CPPUNIT_TEST_SUITE(CRMDBTest);
    CPPUNIT_TEST(testCRMDatabase);
    CPPUNIT_TEST_SUITE_END();
};

void CRMDBTest::testCRMDatabase()
{
    createDBDocument(u"sdbc:embedded:hsqldb"_ustr);
    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    uno::Reference<XDataSource> xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    // create queries before establishing connection to database
    createQueries(xDataSource);

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    createTables(xConnection);

    // test selection
    Reference<XStatement> xStatement = xConnection->createStatement();
    Reference<XResultSet> xResults
        = xStatement->executeQuery(u"SELECT \"NAME\" FROM \"CATEGORIES\" ORDER BY \"ID\""_ustr);
    CPPUNIT_ASSERT(xResults.is());

    Reference<XRow> xRow(xResults, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xResults->next());
    CPPUNIT_ASSERT_EQUAL(u"Food"_ustr, xRow->getString(1));
    CPPUNIT_ASSERT(xResults->next());
    CPPUNIT_ASSERT_EQUAL(u"Furniture"_ustr, xRow->getString(1));

    // test if the queries have been created and can be used
    uno::Reference<XQueriesSupplier> xQuerySupplier(xConnection, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueries();
    CPPUNIT_ASSERT(xQueryAccess->hasElements());

    // the unshipped orders query depends on the all orders query, so we'll test
    // to see if both work
    uno::Reference<sdbcx::XColumnsSupplier> xColumns(
        xQueryAccess->getByName(u"unshipped orders"_ustr), UNO_QUERY);
    uno::Reference<container::XNameAccess> xColumnAccess(xColumns->getColumns());
    CPPUNIT_ASSERT(xColumnAccess->hasElements());

    Sequence<OUString> ColumnNames = xColumnAccess->getElementNames();
    CPPUNIT_ASSERT(ColumnNames.hasElements());
    // first column returned should be from the all orders query
    CPPUNIT_ASSERT_EQUAL(ColumnNames[0], u"Order No."_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(CRMDBTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
