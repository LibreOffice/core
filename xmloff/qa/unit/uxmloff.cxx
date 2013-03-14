/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <xmloff/xmlexp.hxx>
#include "SchXMLExport.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/MeasureUnit.hpp>

using namespace ::com::sun::star;

namespace {

class Test : public test::BootstrapFixture {
public:
    Test();

    virtual void setUp();
    virtual void tearDown();

    void testSomething();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSomething);
    CPPUNIT_TEST_SUITE_END();
private:
    SvXMLExport *pExport;
};


Test::Test()
    : pExport( NULL )
{
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    pExport = new SchXMLExport(/* util::MeasureUnit::CM, */
                               comphelper::getProcessComponentContext());
}

void Test::tearDown()
{
    delete pExport;
    BootstrapFixture::tearDown();
}

void Test::testSomething()
{
    OUString s1("A");
    OUString s2("B");
    CPPUNIT_ASSERT_MESSAGE("these strings are supposed to be different!", s1 != s2);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
