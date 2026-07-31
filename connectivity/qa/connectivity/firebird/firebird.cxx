/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <osl/file.hxx>

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

class FirebirdTestDriver : public UnoApiTest
{
private:
    Reference<XInterface> m_xFirebirdComponent;
    Reference<XDriver> m_xDriver;
    Sequence<PropertyValue> m_infos;

public:
    FirebirdTestDriver()
        : UnoApiTest(u"/connectivity/qa/connectivity/firebird/data"_ustr)
    {
    }
    virtual void setUp() override;
    void testDBConnection();

    CPPUNIT_TEST_SUITE(FirebirdTestDriver);
    CPPUNIT_TEST(testDBConnection);
    CPPUNIT_TEST_SUITE_END();
};

void FirebirdTestDriver::setUp()
{
    UnoApiTest::setUp();

    m_xFirebirdComponent
        = getMultiServiceFactory()->createInstance(u"com.sun.star.comp.sdbc.firebird.Driver"_ustr);
    CPPUNIT_ASSERT_MESSAGE("no firebird component!", m_xFirebirdComponent.is());

    m_xDriver.set(m_xFirebirdComponent, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("cannot connect to firebird driver!", m_xDriver.is());
}

void FirebirdTestDriver::testDBConnection()
{
    // Use existing database inside the firebird tarball
    const OUString sDocPath(
        m_directories.getURLFromWorkdir(u"/UnpackedTarball/firebird/gen/examples/employee.fdb"));

    // and create a copy of it
    auto const e = osl::File::copy(sDocPath, maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);

    Reference<XConnection> xConnection
        = m_xDriver->connect("sdbc:firebird:" + maTempFile.GetURL(), m_infos);
    CPPUNIT_ASSERT_MESSAGE("cannot connect to data source!", xConnection.is());

    uno::Reference<XStatement> xStatement = xConnection->createStatement();
    CPPUNIT_ASSERT(xStatement.is());

    uno::Reference<XResultSet> xRes
        = xStatement->executeQuery(u"SELECT COUNTRY, CURRENCY FROM COUNTRY ORDER BY COUNTRY"_ustr);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // assert first row
    CPPUNIT_ASSERT(xRes->next());
    CPPUNIT_ASSERT_EQUAL(u"Australia"_ustr, xRow->getString(1));
    CPPUNIT_ASSERT_EQUAL(u"ADollar"_ustr, xRow->getString(2));
}

CPPUNIT_TEST_SUITE_REGISTRATION(FirebirdTestDriver);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
