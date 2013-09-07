/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class FirebirdTest
    : public ::test::BootstrapFixture
    , public ::unotest::MacrosTest
{
public:
    virtual void setUp();
    virtual void tearDown();

    /**
     * Test that we connect to the database with an "empty" .odb file.
     */
    void testEmptyDBConnection();

    CPPUNIT_TEST_SUITE(FirebirdTest);
    CPPUNIT_TEST(testEmptyDBConnection);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * Test the loading of an "empty" file, i.e. the embedded database has not yet
 * been initialised (as occurs when a new .odb is created and opened by base).
 */
void FirebirdTest::testEmptyDBConnection()
{
    const OUString sFileName("/dbaccess/qa/unit/data/firebird_empty.odb");

    uno::Reference< lang::XComponent > xComponent = loadFromDesktop(
                                                    getSrcRootURL() + sFileName);
    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference< XOfficeDatabaseDocument > xDocument(xComponent, UNO_QUERY);
    CPPUNIT_ASSERT(xDocument.is());

    uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    uno::Reference< XConnection > xConnection = xDataSource->getConnection("","");
    CPPUNIT_ASSERT(xConnection.is());
}

void FirebirdTest::setUp()
{
    ::test::BootstrapFixture::setUp();

    mxDesktop = ::com::sun::star::frame::Desktop::create(
                    ::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(mxDesktop.is());
}

void FirebirdTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(FirebirdTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
