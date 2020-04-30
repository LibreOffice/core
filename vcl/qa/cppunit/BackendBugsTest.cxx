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

#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <test/outputdevice.hxx>

class BackendBugsTest : public test::BootstrapFixture
{
    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        BitmapEx aBitmapEx(device->GetBitmap(Point(0, 0), device->GetOutputSizePixel()));
        SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
    }

public:
    BackendBugsTest()
        : BootstrapFixture(true, false)
    {
    }

    void testTdf124848()
    {
        ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(100, 100));
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        device->SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
        device->SetLineColor(COL_BLACK);
        basegfx::B2DHomMatrix matrix;
        // DrawPolyLine() would apply the whole matrix to the line width, making it negative
        // in case of a larger rotation.
        matrix.rotate(M_PI); //180 degrees
        matrix.translate(100, 100);
        CPPUNIT_ASSERT(device->DrawPolyLineDirect(
            matrix, basegfx::B2DPolygon({ { 50, 50 }, { 50, 100 } }), 100));
        // exportDevice("/tmp/tdf124848.png", device);
        // 100px wide line should fill the entire width of the upper half
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(2, 2)));
    }

    CPPUNIT_TEST_SUITE(BackendBugsTest);

    CPPUNIT_TEST(testTdf124848);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BackendBugsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
