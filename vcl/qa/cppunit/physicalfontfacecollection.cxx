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
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/virdev.hxx>

#include <PhysicalFontFace.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <fontattributes.hxx>
#include <fontinstance.hxx>
#include <fontselect.hxx>

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

namespace
{
class TestFontInstance : public LogicalFontInstance
{
public:
    TestFontInstance(PhysicalFontFace const& rFontFace, FontSelectPattern const& rFontSelectPattern)
        : LogicalFontInstance(rFontFace, rFontSelectPattern)
    {
    }

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override
    {
        return true;
    }

protected:
    bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override { return true; }
};

class TestFontFace : public PhysicalFontFace
{
public:
    TestFontFace(sal_uIntPtr nId)
        : PhysicalFontFace(FontAttributes())
        , mnFontId(nId)
    {
    }

    rtl::Reference<LogicalFontInstance>
    CreateFontInstance(FontSelectPattern const& rFontSelectPattern) const override
    {
        return new TestFontInstance(*this, rFontSelectPattern);
    }

    sal_IntPtr GetFontId() const override { return mnFontId; }
    FontCharMapRef GetFontCharMap() const override { return FontCharMap::GetDefaultMap(false); }
    bool GetFontCapabilities(vcl::FontCapabilities&) const override { return true; }

private:
    sal_IntPtr mnFontId;
};
}

void VclPhysicalFontFaceCollectionTest::testShouldGetFontId()
{
    vcl::font::PhysicalFontFaceCollection aCollection;
    aCollection.Add(new TestFontFace(1988756));

    rtl::Reference<PhysicalFontFace> pActual(new TestFontFace(1988756));
    rtl::Reference<PhysicalFontFace> pExpected = aCollection.Get(0);

    CPPUNIT_ASSERT_EQUAL(pExpected->GetFontId(), pActual->GetFontId());
    CPPUNIT_ASSERT_EQUAL(1, aCollection.Count());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFaceCollectionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
