/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/log.hxx>

#include <vcl/drawables/Drawable.hxx>
#include <vcl/virdev.hxx>

enum EnumScaffolding
{
    Scaffolding,
    NoScaffolding
};

namespace vcl
{
class FakeDrawable : public Drawable
{
public:
    FakeDrawable(EnumScaffolding eScaffolding)
        : Drawable(eScaffolding == Scaffolding)
        , meScaffolding(eScaffolding)
        , mbDidAddAction(false)
        , mbDidInitClipRegion(false)
        , mbDidInitLineColor(false)
        , mbDidInitFillColor(false)
        , mbDidUseAlpha(false)
    {
    }

    bool DidAddAction() { return mbDidAddAction; }
    bool DidInitClipRegion() { return mbDidInitClipRegion; }
    bool DidInitLineColor() { return mbDidInitLineColor; }
    bool DidInitFillColor() { return mbDidInitFillColor; }
    bool DidUseAlpha() { return mbDidUseAlpha; }

protected:
    bool ShouldAddAction() const override;
    bool CanDraw(OutputDevice*) const override;
    bool ShouldInitClipRegion() const override;
    bool ShouldInitLineColor() const override;
    bool ShouldInitFillColor() const override;
    bool UseAlphaVirtDev() const override;

    bool DrawCommand(OutputDevice* const) const override;

private:
    EnumScaffolding meScaffolding;

    mutable bool mbDidAddAction;
    mutable bool mbDidInitClipRegion;
    mutable bool mbDidInitLineColor;
    mutable bool mbDidInitFillColor;
    mutable bool mbDidUseAlpha;
};

bool FakeDrawable::DrawCommand(OutputDevice* const) const { return true; }

bool FakeDrawable::ShouldAddAction() const
{
    mbDidAddAction = (meScaffolding == Scaffolding);
    return true;
}

bool FakeDrawable::CanDraw(OutputDevice*) const { return true; }

bool FakeDrawable::ShouldInitClipRegion() const
{
    mbDidInitClipRegion = (meScaffolding == Scaffolding);
    return true;
}

bool FakeDrawable::ShouldInitLineColor() const
{
    mbDidInitLineColor = (meScaffolding == Scaffolding);
    return true;
}

bool FakeDrawable::ShouldInitFillColor() const
{
    mbDidInitFillColor = (meScaffolding == Scaffolding);
    return true;
}

bool FakeDrawable::UseAlphaVirtDev() const
{
    mbDidUseAlpha = (meScaffolding == Scaffolding);
    return true;
}

} // namespace vcl

namespace
{
class DrawableTest : public CppUnit::TestFixture
{
    void testScaffolding();
    void testNoScaffolding();

    CPPUNIT_TEST_SUITE(DrawableTest);
    CPPUNIT_TEST(testScaffolding);
    CPPUNIT_TEST(testNoScaffolding);
    CPPUNIT_TEST_SUITE_END();
};

void DrawableTest::testScaffolding()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    vcl::FakeDrawable aFakeDrawable(EnumScaffolding::Scaffolding);

    pRenderContext->Draw(aFakeDrawable);

    CPPUNIT_ASSERT_MESSAGE("Did not add action", aFakeDrawable.DidAddAction());
    CPPUNIT_ASSERT_MESSAGE("Did not initialize line color", aFakeDrawable.DidInitLineColor());
    CPPUNIT_ASSERT_MESSAGE("Did not initialize fill color", aFakeDrawable.DidInitFillColor());
    CPPUNIT_ASSERT_MESSAGE("Did not initialize clipping region", aFakeDrawable.DidInitClipRegion());
    CPPUNIT_ASSERT_MESSAGE("Did not use alpha VirtualDevice", aFakeDrawable.DidUseAlpha());
}

void DrawableTest::testNoScaffolding()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    vcl::FakeDrawable aFakeDrawable(EnumScaffolding::NoScaffolding);

    pRenderContext->Draw(aFakeDrawable);

    CPPUNIT_ASSERT_MESSAGE("Added action", !aFakeDrawable.DidAddAction());
    CPPUNIT_ASSERT_MESSAGE("Initialized line color", !aFakeDrawable.DidInitLineColor());
    CPPUNIT_ASSERT_MESSAGE("Initialized fill color", !aFakeDrawable.DidInitFillColor());
    CPPUNIT_ASSERT_MESSAGE("Initialized clipping region", !aFakeDrawable.DidInitClipRegion());
    CPPUNIT_ASSERT_MESSAGE("Used alpha VirtualDevice", !aFakeDrawable.DidUseAlpha());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(DrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
