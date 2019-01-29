/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>

#include <com/sun/star/util/DateTime.hpp>
#include <svtools/miscopt.hxx>
#include <osl/process.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

class MysqlTestDriver : public test::BootstrapFixture
{
private:
    OUString m_sUrl;
    Reference<XInterface> m_xMysqlcComponent;
    Reference<XDriver> m_xDriver;
    Sequence<PropertyValue> m_infos;

public:
    MysqlTestDriver()
        : test::BootstrapFixture(false, false)
    {
    }
    virtual void setUp() override;
    virtual void tearDown() override;
    void testDBConnection();
    void testCreateAndDropTable();
    void testIntegerInsertAndQuery();
    void testDBPositionChange();
    void testMultipleResultsets();
    void testDBMetaData();
    void testTimestampField();

    CPPUNIT_TEST_SUITE(MysqlTestDriver);
    CPPUNIT_TEST(testDBConnection);
    CPPUNIT_TEST(testCreateAndDropTable);
    CPPUNIT_TEST(testIntegerInsertAndQuery);
    CPPUNIT_TEST(testMultipleResultsets);
    CPPUNIT_TEST(testDBMetaData);
    CPPUNIT_TEST(testTimestampField);
    CPPUNIT_TEST_SUITE_END();
};

void MysqlTestDriver::tearDown()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    }
    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");
    xStatement->executeUpdate("DROP TABLE IF EXISTS otherTable");
    test::BootstrapFixture::tearDown();
}

void MysqlTestDriver::setUp()
{
    test::BootstrapFixture::setUp();

    /* Get URL from environment variable. This test suite should run only when
     * there is an URL given. This is because it can be used for testing connection to
     * external databases as well.
     *
     * Example URL:
     * username/password@sdbc:mysql:mysqlc:localhost:3306/testdatabase
     */
    osl_getEnvironment(OUString("CONNECTIVITY_TEST_MYSQL_DRIVER").pData, &m_sUrl.pData);
    m_xMysqlcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.sdbc.mysqlc.MysqlCDriver");
    CPPUNIT_ASSERT_MESSAGE("no mysqlc component!", m_xMysqlcComponent.is());

    // set user name and password
    m_infos = Sequence<PropertyValue>{ 2 };
    m_infos[0].Name = OUString{ "user" };
    sal_Int32 nPer = m_sUrl.indexOf("/");
    m_infos[0].Value = makeAny(m_sUrl.copy(0, nPer));
    m_sUrl = m_sUrl.copy(nPer + 1);
    m_infos[1].Name = OUString{ "password" };
    sal_Int32 nAt = m_sUrl.indexOf("@");
    m_infos[1].Value = makeAny(m_sUrl.copy(0, nAt));
    m_sUrl = m_sUrl.copy(nAt + 1);

    m_xDriver.set(m_xMysqlcComponent, UNO_QUERY);
    if (!m_xDriver.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to mysqlc driver!", m_xDriver.is());
    }
}

/**
 * Test database connection. It is assumed that the given URL is correct and
 * there is a server running at the location.
 */
void MysqlTestDriver::testDBConnection()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    }

    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());

    Reference<XResultSet> xResultSet = xStatement->executeQuery("SELECT 1");
    CPPUNIT_ASSERT(xResultSet.is());
    Reference<XRow> xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xRow.is());

    sal_Bool result = xResultSet->first();
    CPPUNIT_ASSERT_MESSAGE("fetch first row failed!", result);
}

/**
 * Test creation and removal of a table
 */
void MysqlTestDriver::testCreateAndDropTable()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    }

    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");

    auto nUpdateCount
        = xStatement->executeUpdate("CREATE TABLE myTestTable (id INTEGER PRIMARY KEY)");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement

    // we can use the same xStatement instance here
    nUpdateCount = xStatement->executeUpdate("DROP TABLE myTestTable");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement
}

void MysqlTestDriver::testIntegerInsertAndQuery()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    }

    Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");

    auto nUpdateCount
        = xStatement->executeUpdate("CREATE TABLE myTestTable (id INTEGER PRIMARY KEY)");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement

    Reference<XPreparedStatement> xPrepared
        = xConnection->prepareStatement(OUString{ "INSERT INTO myTestTable VALUES (?)" });
    Reference<XParameters> xParams(xPrepared, UNO_QUERY);
    constexpr int ROW_COUNT = 3;
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        xParams->setLong(1, i); // first and only column
        nUpdateCount = xPrepared->executeUpdate();
        CPPUNIT_ASSERT_EQUAL(1, nUpdateCount); // one row is inserted at a time
    }

    // now let's query the existing data
    Reference<XResultSet> xResultSet = xStatement->executeQuery("SELECT id from myTestTable");
    CPPUNIT_ASSERT_MESSAGE("result set cannot be instantiated after query", xResultSet.is());
    Reference<XRow> xRow(xResultSet, UNO_QUERY);
    Reference<XColumnLocate> xColumnLocate(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xRow.is());

    for (long i = 0; i < ROW_COUNT; ++i)
    {
        bool hasRow = xResultSet->next();
        CPPUNIT_ASSERT_MESSAGE("not enough result after query", hasRow);
        CPPUNIT_ASSERT_EQUAL(i, xRow->getLong(1)); // first and only column
        CPPUNIT_ASSERT_EQUAL(i, xRow->getLong(xColumnLocate->findColumn("id"))); // test findColumn
    }
    CPPUNIT_ASSERT_MESSAGE("Cursor is not on last position.",
                           xResultSet->isLast()); // cursor is on last position
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT, xResultSet->getRow()); // which is the last position

    bool hasRow = xResultSet->next(); // go to afterlast
    // no more rows, next should return false
    CPPUNIT_ASSERT_MESSAGE("next returns true after last row", !hasRow);
    // cursor should be in afterlast position
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT + 1, xResultSet->getRow());
    CPPUNIT_ASSERT_MESSAGE("Cursor is not on after-last position.", xResultSet->isAfterLast());

    nUpdateCount = xStatement->executeUpdate("DROP TABLE myTestTable");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement
}

void MysqlTestDriver::testDBPositionChange()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    }

    Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");

    auto nUpdateCount
        = xStatement->executeUpdate("CREATE TABLE myTestTable (id INTEGER PRIMARY KEY)");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement
    Reference<XPreparedStatement> xPrepared
        = xConnection->prepareStatement(OUString{ "INSERT INTO myTestTable VALUES (?)" });
    Reference<XParameters> xParams(xPrepared, UNO_QUERY);
    constexpr int ROW_COUNT = 3;
    for (int i = 1; i <= ROW_COUNT; ++i)
    {
        xParams->setLong(1, i); // first and only column
        nUpdateCount = xPrepared->executeUpdate();
        CPPUNIT_ASSERT_EQUAL(1, nUpdateCount); // one row is inserted at a time
    }
    Reference<XResultSet> xResultSet = xStatement->executeQuery("SELECT id from myTestTable");
    CPPUNIT_ASSERT_MESSAGE("result set cannot be instantiated after query", xResultSet.is());
    Reference<XRow> xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xRow.is());

    xResultSet->afterLast();
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT + 1, xResultSet->getRow());
    xResultSet->last();
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT, nUpdateCount);
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT, xResultSet->getRow());
    bool successPrevious = xResultSet->previous();
    CPPUNIT_ASSERT(successPrevious);
    CPPUNIT_ASSERT_EQUAL(ROW_COUNT - 1, nUpdateCount);
    xResultSet->beforeFirst();
    xResultSet->next();
    CPPUNIT_ASSERT_EQUAL(1, xResultSet->getRow());
    xResultSet->first();
    CPPUNIT_ASSERT_EQUAL(1, xResultSet->getRow());

    // Now previous should put the cursor to before-first position, but it
    // should return with false.
    successPrevious = xResultSet->previous();
    CPPUNIT_ASSERT(!successPrevious);
    CPPUNIT_ASSERT_EQUAL(0, xResultSet->getRow());

    nUpdateCount = xStatement->executeUpdate("DROP TABLE myTestTable");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement
}

void MysqlTestDriver::testMultipleResultsets()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    CPPUNIT_ASSERT(xConnection.is());
    Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    // create two tables
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");
    xStatement->executeUpdate("DROP TABLE IF EXISTS otherTable");
    xStatement->executeUpdate("CREATE TABLE myTestTable (id INTEGER PRIMARY KEY)");
    xStatement->executeUpdate("INSERT INTO myTestTable VALUES (1)");
    xStatement->executeUpdate("CREATE TABLE otherTable (id INTEGER PRIMARY KEY)");
    xStatement->executeUpdate("INSERT INTO otherTable VALUES (2)");

    // create first result set
    Reference<XResultSet> xResultSet = xStatement->executeQuery("SELECT id from myTestTable");
    CPPUNIT_ASSERT_MESSAGE("result set cannot be instantiated after query", xResultSet.is());
    // use it
    xResultSet->next();
    Reference<XRow> xRowFirst(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(1l, xRowFirst->getLong(1));
    // create second result set
    Reference<XResultSet> xResultSet2 = xStatement->executeQuery("SELECT id from otherTable");
    // use second result set
    xResultSet2->next();
    Reference<XRow> xRowSecond(xResultSet2, UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(2l, xRowSecond->getLong(1));
    // now use the first result set again
    CPPUNIT_ASSERT_EQUAL(1l, xRowFirst->getLong(1));

    xStatement->executeUpdate("DROP TABLE myTestTable");
    xStatement->executeUpdate("DROP TABLE otherTable");
}

void MysqlTestDriver::testDBMetaData()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");

    auto nUpdateCount = xStatement->executeUpdate(
        "CREATE TABLE myTestTable (id INTEGER PRIMARY KEY, name VARCHAR(20))");
    Reference<XPreparedStatement> xPrepared
        = xConnection->prepareStatement(OUString{ "INSERT INTO myTestTable VALUES (?, ?)" });
    Reference<XParameters> xParams(xPrepared, UNO_QUERY);
    constexpr int ROW_COUNT = 3;
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        xParams->setLong(1, i);
        xParams->setString(2, "lorem");
        xPrepared->executeUpdate();
    }

    Reference<XResultSet> xResultSet = xStatement->executeQuery("SELECT * from myTestTable");
    Reference<XResultSetMetaDataSupplier> xMetaDataSupplier(xResultSet, UNO_QUERY);
    Reference<XResultSetMetaData> xMetaData = xMetaDataSupplier->getMetaData();
    CPPUNIT_ASSERT_EQUAL(OUString{ "id" }, xMetaData->getColumnName(1));
    CPPUNIT_ASSERT_EQUAL(OUString{ "name" }, xMetaData->getColumnName(2));
    CPPUNIT_ASSERT(!xMetaData->isAutoIncrement(1));
    CPPUNIT_ASSERT(!xMetaData->isCaseSensitive(2)); // default collation should be case insensitive
    xResultSet->next(); // use it
    // test that meta data is usable even after fetching result set
    CPPUNIT_ASSERT_EQUAL(OUString{ "name" }, xMetaData->getColumnName(2));
    CPPUNIT_ASSERT_THROW_MESSAGE("exception expected when indexing out of range",
                                 xMetaData->getColumnName(3), sdbc::SQLException);
    nUpdateCount = xStatement->executeUpdate("DROP TABLE myTestTable");
}

void MysqlTestDriver::testTimestampField()
{
    Reference<XConnection> xConnection = m_xDriver->connect(m_sUrl, m_infos);
    if (!xConnection.is())
        CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());
    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());
    xStatement->executeUpdate("DROP TABLE IF EXISTS myTestTable");

    xStatement->executeUpdate(
        "CREATE TABLE myTestTable (id INTEGER PRIMARY KEY, mytimestamp timestamp)");
    xStatement->executeUpdate("INSERT INTO myTestTable VALUES (1, '2008-02-16 20:15:03')");

    // now let's query
    Reference<XResultSet> xResultSet
        = xStatement->executeQuery("SELECT mytimestamp from myTestTable");

    xResultSet->next(); // use it
    Reference<XRow> xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xRow.is());
    util::DateTime dt = xRow->getTimestamp(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<short>(2008), dt.Year);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(2), dt.Month);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(16), dt.Day);

    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(20), dt.Hours);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(15), dt.Minutes);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned short>(3), dt.Seconds);

    xStatement->executeUpdate("DROP TABLE myTestTable");
}

CPPUNIT_TEST_SUITE_REGISTRATION(MysqlTestDriver);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
