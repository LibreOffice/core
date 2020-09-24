/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>

#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <test/unoapi_test.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>

using namespace com::sun::star;

/// This test uses the gen backend (i.e. intentionally not the svp one, which is the default.)
class GenTest : public UnoApiTest
{
public:
    GenTest()
        : UnoApiTest("/vcl/qa/cppunit/gen/data/")
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(
            frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
        SfxApplication::GetOrCreate();
    };

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            closeDocument(mxComponent);
            mxComponent->dispose();
        }
        UnoApiTest::tearDown();
    };

    Bitmap load(const char* pName)
    {
        OUString aFileURL;
        createFileURL(OUString::createFromAscii(pName), aFileURL);
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.drawing.DrawingDocument");
        SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
        CPPUNIT_ASSERT(pModel);
        SfxObjectShell* pShell = pModel->GetObjectShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        BitmapEx aResultBitmap;
        CPPUNIT_ASSERT(xMetaFile->CreateThumbnail(aResultBitmap));
        return aResultBitmap.GetBitmap();
    }

    uno::Reference<lang::XComponent> mxComponent;
};

CPPUNIT_TEST_FIXTURE(GenTest, testTdf121120)
{
    Bitmap aBitmap = load("tdf121120.png");
    Bitmap::ScopedReadAccess pAccess(aBitmap);
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
    pVirtualDevice->SetFillColor(Color(255, 255, 255));
    pVirtualDevice->DrawRect(tools::Rectangle(Point(), Size(1350, 225)));
    pVirtualDevice->SetFillColor(Color(0, 0, 0));
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
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    Color aPixel(pAccess->GetPixel(0, 0));
    // Without the accompanying fix in place, this test would have failed with 'Expected: 000000;
    // Actual: ffffff', i.e. the top left pixel was white, not black.
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aPixel.AsRGBHexString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
