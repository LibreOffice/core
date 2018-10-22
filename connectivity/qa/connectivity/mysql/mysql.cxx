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
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
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
    void testDBConnection();
    void testCreateAndDropTable();
    void testIntegerInsertAndQuery();

    CPPUNIT_TEST_SUITE(MysqlTestDriver);
    CPPUNIT_TEST(testDBConnection);
    CPPUNIT_TEST(testCreateAndDropTable);
    CPPUNIT_TEST(testIntegerInsertAndQuery);
    CPPUNIT_TEST_SUITE_END();
};

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
    CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xRow.is());

    for (long i = 0; i < ROW_COUNT; ++i)
    {
        bool hasRow = xResultSet->next();
        CPPUNIT_ASSERT_MESSAGE("not enough result after query", hasRow);
        CPPUNIT_ASSERT_EQUAL(i, xRow->getLong(1)); // first and only column
    }

    nUpdateCount = xStatement->executeUpdate("DROP TABLE myTestTable");
    CPPUNIT_ASSERT_EQUAL(0, nUpdateCount); // it's a DDL statement
}

CPPUNIT_TEST_SUITE_REGISTRATION(MysqlTestDriver);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
