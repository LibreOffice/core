/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include "MColumnAlias.hxx"
#include "MQueryHelper.hxx"
#include "MConnection.hxx"

#include <com/sun/star/sdbc/XDriver.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

namespace connectivity { namespace mork {


class MorkDriverTest: public test::BootstrapFixture
{
public:
    MorkDriverTest() : test::BootstrapFixture(false, false) {};

    void test_metadata();
    void test_select_default_all();
    void test_select_list_table_joe_doe_5();

    virtual void setUp();
    virtual void tearDown();

    CPPUNIT_TEST_SUITE(MorkDriverTest);

    CPPUNIT_TEST(test_metadata);
    CPPUNIT_TEST(test_select_default_all);
    CPPUNIT_TEST(test_select_list_table_joe_doe_5);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<XInterface> m_xMorkComponent;
    Reference<XConnection> m_xConnection;
};

void MorkDriverTest::setUp()
{
    test::BootstrapFixture::setUp();
    m_xMorkComponent = getMultiServiceFactory()->createInstance("com.sun.star.comp.sdbc.MorkDriver");
    CPPUNIT_ASSERT_MESSAGE("no mork component!", m_xMorkComponent.is());

    // is this the best way to pass test file through URL?
    // may be take a custom Sequence< PropertyValue > route?
    OUString url = OUString("sdbc:address:thunderbird:unittest:") +
        getPathFromSrc("/connectivity/qa/connectivity/mork/abook_10_john_does.mab");

    Sequence< PropertyValue > info;
    Reference< XDriver> xDriver(m_xMorkComponent, UNO_QUERY);
    if (!xDriver.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to mork driver!", xDriver.is());
    }

    m_xConnection = xDriver->connect(url, info);
    if (!m_xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to address book data soure!", m_xConnection.is());
    }
}

void MorkDriverTest::tearDown()
{
// how to make dispose() work?
// Reference< com::sun::star::lang::XComponent >( m_xMorkComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

void MorkDriverTest::test_metadata()
{
    Reference< XDatabaseMetaData > xDatabaseMetaData = m_xConnection->getMetaData();
    if (!xDatabaseMetaData.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot retrieve meta data!", xDatabaseMetaData.is());
    }

    const Any catalog;
    const OUString schemaPattern = "%";
    const OUString tableNamePattern = "%";
    const Sequence< OUString > types;

    Reference< XResultSet > xResultSet =
        xDatabaseMetaData->getTables(catalog, schemaPattern, tableNamePattern, types);
    if (!xResultSet.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot retrieve tables!", xResultSet.is());
    }

    // TODO: how to access that result set and check the tables?
    // it should be 3 tables inside: AddressBook, does_5 and does_10
}

void MorkDriverTest::test_select_default_all()
{
    const OUString sql = "select \"E-mail\" from \"AddressBook\" ORDER BY \"E-mail\"";
    Reference< XPreparedStatement > xStatement = m_xConnection->prepareStatement(sql);
    if (!xStatement.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot create prepared statement!", xStatement.is());
    }

    Reference< XResultSet > xResultSet = xStatement->executeQuery();
    if (!xResultSet.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot execure sql statement!", xResultSet.is());
    }

    Reference< XRow > xDelegatorRow(xResultSet, UNO_QUERY);
    if (!xDelegatorRow.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xDelegatorRow.is());
    }

    sal_Bool result = xResultSet->first();
    CPPUNIT_ASSERT_MESSAGE("fetch first row failed!", result);
    OUString mail = xDelegatorRow->getString(1);
    CPPUNIT_ASSERT_MESSAGE("first row is not john@doe.org!", mail.equalsAscii("john@doe.org"));

    result = xResultSet->next();
    CPPUNIT_ASSERT_MESSAGE("fetch second row failed!", result);
    mail = xDelegatorRow->getString(1);
    CPPUNIT_ASSERT_MESSAGE("second row is not john@doe10.org!", mail.equalsAscii("john@doe10.org"));

    result = xResultSet->last();
    CPPUNIT_ASSERT_MESSAGE("fetch last row failed!", result);
    mail = xDelegatorRow->getString(1);
    CPPUNIT_ASSERT_MESSAGE("last row is not john@doe9.org!", mail.equalsAscii("john@doe9.org"));
}

void MorkDriverTest::test_select_list_table_joe_doe_5()
{
    const OUString sql = "select \"E-mail\" from \"does_5\" where \"E-mail\" LIKE '%doe5.org' ";
    Reference< XPreparedStatement > xStatement = m_xConnection->prepareStatement(sql);
    if (!xStatement.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot create prepared statement!", xStatement.is());
    }

    Reference< XResultSet > xResultSet = xStatement->executeQuery();
    if (!xResultSet.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot execure sql statement!", xResultSet.is());
    }

    Reference< XRow > xDelegatorRow(xResultSet, UNO_QUERY);
    if (!xDelegatorRow.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xDelegatorRow.is());
    }

    sal_Bool result = xResultSet->first();
    CPPUNIT_ASSERT_MESSAGE("fetch first row failed!", result);
    OUString mail = xDelegatorRow->getString(1);
    CPPUNIT_ASSERT_MESSAGE("last row is not john@doe5.org!", mail.equalsAscii("john@doe5.org"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(MorkDriverTest);

}}

CPPUNIT_PLUGIN_IMPLEMENT();
