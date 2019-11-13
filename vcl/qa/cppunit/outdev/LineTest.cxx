/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

class LineTest : public test::BootstrapFixture
{
public:
    LineTest()
        : BootstrapFixture(true, false)
    {
    }
    void testLine();

    CPPUNIT_TEST_SUITE(LineTest);
    CPPUNIT_TEST(testLine);

    CPPUNIT_TEST_SUITE_END();
};

void LineTest::testLine()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(10, 10));

    pRenderContext->DrawLine(Point(0, 0), Point(9, 9));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pRenderContext->GetPixel(Point(5, 5)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(LineTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
