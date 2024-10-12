/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class CRMDBTest : public DBTestBase
{
public:
    void testCRMDatabase();
    void testRegistrationName();
    uno::Reference<XConnection> setUpDBConnection();
    void testQueryColumns();

    CPPUNIT_TEST_SUITE(CRMDBTest);
    CPPUNIT_TEST(testCRMDatabase);
    CPPUNIT_TEST(testRegistrationName);
    CPPUNIT_TEST(testQueryColumns);
    CPPUNIT_TEST_SUITE_END();
};

uno::Reference<XConnection> CRMDBTest::setUpDBConnection()
{
    createDBDocument(u"sdbc:embedded:hsqldb"_ustr);
    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    uno::Reference<XDataSource> xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    // create queries before establishing connection to database
    createQueries(xDataSource);

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    return xConnection;
}

void CRMDBTest::testCRMDatabase()
{
    uno::Reference<XConnection> xConnection = setUpDBConnection();
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

void CRMDBTest::testRegistrationName()
{
    // 1. check the existing "Bibliography" data source whether it has the proper name
    Reference<container::XNameAccess> xNameAccess(
        getMultiServiceFactory()->createInstance(u"com.sun.star.sdb.DatabaseContext"_ustr),
        UNO_QUERY);
    Reference<sdbc::XDataSource> xBiblioDataSource(xNameAccess->getByName(u"Bibliography"_ustr),
                                                   UNO_QUERY);
    CPPUNIT_ASSERT(xBiblioDataSource.is());

    Reference<beans::XPropertySet> xBiblioProp(xBiblioDataSource, UNO_QUERY);
    OUString sBiblioDataSourceName;
    xBiblioProp->getPropertyValue(u"Name"_ustr) >>= sBiblioDataSourceName;

    CPPUNIT_ASSERT_EQUAL(u"Bibliography"_ustr, sBiblioDataSourceName);

    // 2. register a newly created data source, and verify it has the proper name
    // Setup the CRMDatabase
    createDBDocument(u"sdbc:embedded:hsqldb"_ustr);
    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    createTables(xConnection);

    uno::Reference<XDataSource> xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    // Get a XDatabaseContext from XSingleService factory in order to register the DataSource
    Reference<sdb::XDatabaseContext> xDatabaseContext(xNameAccess, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDatabaseContext.is());

    // Register the datasource
    xDatabaseContext->registerObject(u"SomeNewHsqlDataSource"_ustr, xDataSource);

    // Check the newly created data source to see if it has the proper name
    Reference<beans::XPropertySet> xProp(xDataSource, UNO_QUERY);
    OUString sDataSourceName;
    xProp->getPropertyValue(u"Name"_ustr) >>= sDataSourceName;

    CPPUNIT_ASSERT_EQUAL(u"SomeNewHsqlDataSource"_ustr, sDataSourceName);

    CPPUNIT_ASSERT_THROW_MESSAGE(
        "Bibliography already exists",
        xDatabaseContext->registerObject(u"Bibliography"_ustr, xDataSource),
        container::ElementExistException);
}

void CRMDBTest::testQueryColumns()
{
    // Ported from dbaccess/Query.java
    // Test prepared queries return the expected column names

    createDBDocument(u"sdbc:embedded:hsqldb"_ustr);
    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    uno::Reference<XDataSource> xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    // Create queries before establishing connection to database
    createQueries(xDataSource);

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    createTables(xConnection);

    uno::Reference<XQueriesSupplier> xQuerySupplier(xConnection, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueries();
    CPPUNIT_ASSERT(xQueryAccess->hasElements());

    Sequence<OUString> queryNames{ u"parseable"_ustr, u"parseable native"_ustr };

    Sequence<Sequence<OUString>> expectedColumnNames{
        { u"ID"_ustr, u"NAME"_ustr, u"ADDRESS"_ustr, u"CITY"_ustr, u"POSTAL"_ustr,
          u"COMMENT"_ustr },
        { u"TABLE_CATALOG"_ustr, u"TABLE_SCHEMA"_ustr, u"TABLE_NAME"_ustr, u"VIEW_DEFINITION"_ustr,
          u"CHECK_OPTION"_ustr, u"IS_UPDATABLE"_ustr, u"VALID"_ustr }
    };

    for (sal_uInt32 i = 0; i < queryNames.size(); ++i)
    {
        uno::Reference<sdbcx::XColumnsSupplier> xColumns(xQueryAccess->getByName(queryNames[i]),
                                                         UNO_QUERY);
        uno::Reference<container::XNameAccess> xColumnAccess(xColumns->getColumns());
        CPPUNIT_ASSERT(xColumnAccess->hasElements());
        Sequence<OUString> ColumnNames = xColumnAccess->getElementNames();

        for (sal_uInt32 j = 0; j < expectedColumnNames[i].size(); ++j)
        {
            CPPUNIT_ASSERT_EQUAL(expectedColumnNames[i][j], ColumnNames[j]);
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(CRMDBTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
