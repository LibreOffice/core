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
#include <vcl/bmpacc.hxx>
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
    VirtualDevice aVDev;
    aVDev.SetOutputSizePixel(Size(32,32));
    aVDev.SetBackground(Wallpaper(COL_WHITE));
    aVDev.Erase();
    aVDev.DrawPixel(Point(1,2),COL_GREEN);
    aVDev.DrawPixel(Point(31,30),COL_RED);

    Size aSize = aVDev.GetOutputSizePixel();
    CPPUNIT_ASSERT(aSize == Size(32,32));

    Bitmap aBmp = aVDev.GetBitmap(Point(),aSize);

#if 0
    OUString rFileName("/tmp/foo-unx.png");
    try {
        vcl::PNGWriter aWriter( aBmp );
        SvFileStream sOutput( rFileName, STREAM_WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("vcl", "Error writing png to " << rFileName);
    }
#endif

    CPPUNIT_ASSERT(aVDev.GetPixel(Point(0,0)) == COL_WHITE);
    CPPUNIT_ASSERT(aVDev.GetPixel(Point(1,2)) == COL_GREEN);
    CPPUNIT_ASSERT(aVDev.GetPixel(Point(31,30)) == COL_RED);
    CPPUNIT_ASSERT(aVDev.GetPixel(Point(30,31)) == COL_WHITE);

    // Gotcha: y and x swap for BitmapReadAccess: deep joy.
    Bitmap::ScopedReadAccess pAcc(aBmp);
    CPPUNIT_ASSERT(pAcc->GetPixel(0,0) == Color(COL_WHITE));
    CPPUNIT_ASSERT(pAcc->GetPixel(2,1) == Color(COL_GREEN));
    CPPUNIT_ASSERT(pAcc->GetPixel(30,31) == Color(COL_RED));
    CPPUNIT_ASSERT(pAcc->GetPixel(31,30) == Color(COL_WHITE));

#if 0
    vcl::Window* pWin = new WorkWindow( (vcl::Window *)NULL );
    CPPUNIT_ASSERT( pWin != NULL );
    OutputDevice *pOutDev = static_cast< OutputDevice * >( pWin );
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
