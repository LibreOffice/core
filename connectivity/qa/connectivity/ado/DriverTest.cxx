/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include "ado/AConnection.hxx"
#include "ado/ADatabaseMetaData.hxx"
#include "ado/ADriver.hxx"
#include "ado/AStatement.hxx"
#include "ado/ACallableStatement.hxx"
#include "ado/APreparedStatement.hxx"
#include "ado/ACatalog.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include <osl/file.hxx>
#include "strings.hrc"


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

namespace connectivity { namespace ado {


class AdoDriverTest: public test::BootstrapFixture
{
public:
    AdoDriverTest() : test::BootstrapFixture(false, false) {};

    void test_metadata();
    void test_select_default_all();

    virtual void setUp();
    virtual void tearDown();

    CPPUNIT_TEST_SUITE(AdoDriverTest);

    CPPUNIT_TEST(test_metadata);
    CPPUNIT_TEST(test_select_default_all);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<XInterface> m_xAdoComponent;
    Reference<XConnection> m_xConnection;
};

void AdoDriverTest::setUp()
{
    test::BootstrapFixture::setUp();
    m_xAdoComponent = getMultiServiceFactory()->createInstance("com.sun.star.comp.sdbc.ado.ODriver");
    CPPUNIT_ASSERT_MESSAGE("no ado component!", m_xAdoComponent.is());

    OUString url = "sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=" +
        m_directories.getPathFromWorkdir("/CppunitTest/TS001018407.mdb");

    Sequence< PropertyValue > info;
    Reference< XDriver> xDriver(m_xAdoComponent, UNO_QUERY);
    if (!xDriver.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to ado driver!", xDriver.is());
    }

    m_xConnection = xDriver->connect(url, info);
    if (!m_xConnection.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot connect to students data source!", m_xConnection.is());
    }
}

void AdoDriverTest::tearDown()
{
    m_xAdoComponent = 0;
    test::BootstrapFixture::tearDown();
}

void AdoDriverTest::test_metadata()
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
}

void AdoDriverTest::test_select_default_all()
{
    const OUString sql = "select \"FirstName\" from \"Students\" ORDER BY \"FirstName\"";
    Reference< XPreparedStatement > xStatement = m_xConnection->prepareStatement(sql);
    if (!xStatement.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot create prepared statement!", xStatement.is());
    }

    Reference< XResultSet > xResultSet = xStatement->executeQuery();
    if (!xResultSet.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot execute sql statement!", xResultSet.is());
    }

    Reference< XRow > xDelegatorRow(xResultSet, UNO_QUERY);
    if (!xDelegatorRow.is())
    {
        CPPUNIT_ASSERT_MESSAGE("cannot extract row from result set!", xDelegatorRow.is());
    }

    sal_Bool result = xResultSet->first();
    CPPUNIT_ASSERT_MESSAGE("fetch first row failed!", result);
/*
    OUString mail = xDelegatorRow->getString(1);
    CPPUNIT_ASSERT_MESSAGE("first row is not john@doe.org!", mail.equalsAscii("john@doe.org"));
*/
}

CPPUNIT_TEST_SUITE_REGISTRATION(AdoDriverTest);

}}

CPPUNIT_PLUGIN_IMPLEMENT();
