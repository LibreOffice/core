/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <test/unoapi_test.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>

using namespace com::sun::star;

/// This test uses the gen backend (i.e. intentionally not the svp one, which is the default.)
class GenTest : public UnoApiTest
{
public:
    GenTest()
        : UnoApiTest(u"/vcl/qa/cppunit/gen/data/"_ustr)
    {
    }

    Bitmap load(const char* pName)
    {
        loadFromFile(OUString::createFromAscii(pName));
        SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
        CPPUNIT_ASSERT(pModel);
        SfxObjectShell* pShell = pModel->GetObjectShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        BitmapEx aResultBitmap;
        CPPUNIT_ASSERT(xMetaFile->CreateThumbnail(aResultBitmap));
        return aResultBitmap.GetBitmap();
    }
};

CPPUNIT_TEST_FIXTURE(GenTest, testTdf121120)
{
    Bitmap aBitmap = load("tdf121120.png");
    BitmapScopedReadAccess pAccess(aBitmap);
    const Size& rSize = aBitmap.GetSizePixel();
    Color aColor(pAccess->GetPixel(rSize.getHeight() / 2, rSize.getWidth() / 2));
    // Without the accompanying fix in place, this test would have failed with 'Expected: 255;
    // Actual  : 1'. I.e. center of the preview (which has the background color) was ~black, not
    // white.
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetRed()));
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetBlue()));
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetGreen()));
}

/// Test that drawing a line preview to a bitmap is not lost.
CPPUNIT_TEST_FIXTURE(GenTest, testTdf107966)
{
    // Set up the virtual device: white background.
    ScopedVclPtr<VirtualDevice> pVirtualDevice(VclPtr<VirtualDevice>::Create());
    pVirtualDevice->SetLineColor();
    MapMode aMapMode;
    aMapMode.SetMapUnit(MapUnit::MapTwip);
    pVirtualDevice->SetMapMode(aMapMode);
    pVirtualDevice->SetOutputSizePixel(Size(90, 15));
    pVirtualDevice->SetFillColor(COL_WHITE);
    pVirtualDevice->DrawRect(tools::Rectangle(Point(), Size(1350, 225)));
    pVirtualDevice->SetFillColor(COL_BLACK);
    AntialiasingFlags nOldAA = pVirtualDevice->GetAntialiasing();
    pVirtualDevice->SetAntialiasing(nOldAA & ~AntialiasingFlags::Enable);

    // Paint a black polygon on it.
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(0, 15));
    aPolygon.append(basegfx::B2DPoint(1350, 15));
    aPolygon.append(basegfx::B2DPoint(1350, 0));
    aPolygon.append(basegfx::B2DPoint(0, 0));
    pVirtualDevice->DrawPolygon(aPolygon);

    // Make sure that the polygon is visible.
    Bitmap aBitmap = pVirtualDevice->GetBitmap(Point(), Size(1350, 15));
    BitmapScopedReadAccess pAccess(aBitmap);
    Color aPixel(pAccess->GetPixel(0, 0));
    // Without the accompanying fix in place, this test would have failed with 'Expected: 000000;
    // Actual: ffffff', i.e. the top left pixel was white, not black.
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixel.AsRGBHexString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
