/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <unotest/macros_test.hxx>
#include <comphelper/processfactory.hxx>

using namespace css;

namespace
{

class GraphicObjectTest: public test::BootstrapFixture, public unotest::MacrosTest
{

public:
    void testTdf88836();

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

private:
    CPPUNIT_TEST_SUITE(GraphicObjectTest);
    CPPUNIT_TEST(testTdf88836);
    CPPUNIT_TEST_SUITE_END();
};


void GraphicObjectTest::testTdf88836()
{
    // Construction with empty bitmap -> type should be GraphicType::NONE
    Graphic aGraphic = BitmapEx(Bitmap());
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::NONE), int(aGraphic.GetType()));
    aGraphic = Graphic(BitmapEx());
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::NONE), int(aGraphic.GetType()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicObjectTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
