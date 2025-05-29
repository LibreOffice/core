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
#include <cppunit/TestAssert.h>

#include <rtl/ref.hxx>

#include <font/PhysicalFontFaceCollection.hxx>
#include "fontmocks.hxx"

class VclPhysicalFontFaceCollectionTest : public test::BootstrapFixture
{
public:
    VclPhysicalFontFaceCollectionTest()
        : BootstrapFixture(true, false)
    {
    }

    void testShouldGetFontId();

    CPPUNIT_TEST_SUITE(VclPhysicalFontFaceCollectionTest);
    CPPUNIT_TEST(testShouldGetFontId);
    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontFaceCollectionTest::testShouldGetFontId()
{
    vcl::font::PhysicalFontFaceCollection aCollection;
    aCollection.Add(new TestFontFace(1988756));

    rtl::Reference<vcl::font::PhysicalFontFace> pActual(new TestFontFace(1988756));
    rtl::Reference<vcl::font::PhysicalFontFace> pExpected = aCollection.Get(0);

    CPPUNIT_ASSERT_EQUAL(pExpected->GetFontId(), pActual->GetFontId());
    CPPUNIT_ASSERT_EQUAL(1, aCollection.Count());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFaceCollectionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
