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

#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>

class VclOutdevTest : public test::BootstrapFixture
{
public:
    VclOutdevTest() : BootstrapFixture(true, false) {}

    void testVirtualDevice();

    CPPUNIT_TEST_SUITE(VclOutdevTest);
    CPPUNIT_TEST(testVirtualDevice);
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

    CPPUNIT_ASSERT(pVDev->GetPixel(Point(0,0)).GetColor() == COL_WHITE);
#if defined LINUX //TODO: various failures on Mac and Windows tinderboxes
    CPPUNIT_ASSERT(pVDev->GetPixel(Point(1,2)).GetColor() == COL_BLUE);
    CPPUNIT_ASSERT(pVDev->GetPixel(Point(31,30)).GetColor() == COL_RED);
#endif
    CPPUNIT_ASSERT(pVDev->GetPixel(Point(30,31)).GetColor() == COL_WHITE);

    // Gotcha: y and x swap for BitmapReadAccess: deep joy.
    Bitmap::ScopedReadAccess pAcc(aBmp);
    CPPUNIT_ASSERT(pAcc->GetPixel(0,0).GetColor() == COL_WHITE);
#if defined LINUX //TODO: various failures on Mac and Windows tinderboxes
    CPPUNIT_ASSERT(pAcc->GetPixel(2,1).GetColor() == COL_BLUE);
    CPPUNIT_ASSERT(pAcc->GetPixel(30,31).GetColor() == COL_RED);
#endif
    CPPUNIT_ASSERT(pAcc->GetPixel(31,30).GetColor() == COL_WHITE);

#if 0
    VclPtr<vcl::Window> pWin = VclPtr<WorkWindow>::Create( (vcl::Window *)nullptr );
    CPPUNIT_ASSERT( pWin );
    OutputDevice *pOutDev = pWin.get();
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
