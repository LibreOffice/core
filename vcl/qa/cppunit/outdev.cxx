/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

class VclOutdevTest : public test::BootstrapFixture
{
public:
    VclOutdevTest() : BootstrapFixture(true, false) {}

    void testVirtualDevice();
    void testDrawTransformedBitmapEx();

    CPPUNIT_TEST_SUITE(VclOutdevTest);
    CPPUNIT_TEST(testVirtualDevice);
    CPPUNIT_TEST(testDrawTransformedBitmapEx);
    CPPUNIT_TEST_SUITE_END();
};

void VclOutdevTest::testVirtualDevice()
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    pVDev->SetOutputSizePixel(Size(32,32));
    pVDev->SetBackground(Wallpaper(COL_WHITE));
    pVDev->Erase();
    pVDev->DrawPixel(Point(1,2),COL_BLUE);
    pVDev->DrawPixel(Point(31,30),COL_RED);

    Size aSize = pVDev->GetOutputSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(32,32), aSize);

    Bitmap aBmp = pVDev->GetBitmap(Point(),aSize);

#if 0
    OUString rFileName("/tmp/foo-unx.png");
    try {
        vcl::PNGWriter aWriter( aBmp );
        SvFileStream sOutput( rFileName, StreamMode::WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("vcl", "Error writing png to " << rFileName);
    }
#endif

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(0,0)).GetColor());
#if defined LINUX //TODO: various failures on Mac and Windows tinderboxes
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pVDev->GetPixel(Point(1,2)).GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, pVDev->GetPixel(Point(31,30)).GetColor());
#endif
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(30,31)).GetColor());

    // Gotcha: y and x swap for BitmapReadAccess: deep joy.
    Bitmap::ScopedReadAccess pAcc(aBmp);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(pAcc->GetPixel(0,0)).GetColor());
#if defined LINUX //TODO: various failures on Mac and Windows tinderboxes
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, Color(pAcc->GetPixel(2,1)).GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, Color(pAcc->GetPixel(30,31)).GetColor());
#endif
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(pAcc->GetPixel(31,30)).GetColor());

#if 0
    VclPtr<vcl::Window> pWin = VclPtr<WorkWindow>::Create( (vcl::Window *)nullptr );
    CPPUNIT_ASSERT( pWin );
    OutputDevice *pOutDev = pWin.get();
#endif
}

void VclOutdevTest::testDrawTransformedBitmapEx()
{
    // Create a virtual device, and connect a metafile to it.
    // Also create a 16x16 bitmap.
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), 24);
    BitmapEx aBitmapEx(aBitmap);
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.scale(8, 8);
    aMatrix.rotate(M_PI / 2);
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    // Draw the rotated bitmap on the vdev.
    pVDev->DrawTransformedBitmapEx(aMatrix, aBitmapEx);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMtf.GetActionSize());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMPEXSCALE, pAction->GetType());
    auto pBitmapAction = static_cast<MetaBmpExScaleAction*>(pAction);
    const BitmapEx& rBitmapEx = pBitmapAction->GetBitmapEx();
    Size aTransformedSize = rBitmapEx.GetSizePixel();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16x16
    // - Actual  : 8x8
    // I.e. the bitmap before scaling was already scaled down, just because it was rotated.
    CPPUNIT_ASSERT_EQUAL(Size(16, 16), aTransformedSize);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
