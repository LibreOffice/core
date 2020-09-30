/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/skia/SkiaHelper.hxx>
#include <skia/salbmp.hxx>
#include <vcl/bitmapaccess.hxx>

// This tests backends that use Skia (i.e. intentionally not the svp one, which is the default.)
// Note that you still may need to actually set for Skia to be used (see vcl/README.vars).
// If Skia is not enabled, all tests will be silently skipped.
namespace
{
class SkiaTest : public test::BootstrapFixture
{
public:
    SkiaTest()
        : test::BootstrapFixture(true, false)
    {
    }

    void testBitmapErase();

    CPPUNIT_TEST_SUITE(SkiaTest);
    CPPUNIT_TEST(testBitmapErase);
    CPPUNIT_TEST_SUITE_END();
};

void SkiaTest::testBitmapErase()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    Bitmap bitmap(Size(10, 10), 24);
    SkiaSalBitmap* skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap);
    // Uninitialized bitmap.
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
    // Test that Bitmap.Erase() just sets erase color and doesn't allocate pixels.
    bitmap.Erase(COL_RED);
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasEraseColor());
    // Reading a pixel will create pixel data.
    BitmapReadAccess access(bitmap);
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), access.GetColor(0, 0));
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(SkiaTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
