/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>

namespace
{
class Test : public test::BootstrapFixture
{
public:
    virtual void setUp() override;

protected:
    css::uno::Reference<css::sheet::addin::XAnalysis> mxAnalysis;
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();
    auto xFactory(comphelper::getProcessServiceFactory());
    mxAnalysis.set(xFactory->createInstance(u"com.sun.star.sheet.addin.Analysis"_ustr),
                   css::uno::UNO_QUERY_THROW);
}

CPPUNIT_TEST_FIXTURE(Test, test_getDec2Hex)
{
    // Test that 'Places' argument accepts different numeric types
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_Int8(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_Int16(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_uInt16(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_Int32(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_uInt32(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_Int64(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(sal_uInt64(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(double(10))));
    CPPUNIT_ASSERT_EQUAL(u"000000006E"_ustr,
                         mxAnalysis->getDec2Hex({}, 110, css::uno::Any(float(10))));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
