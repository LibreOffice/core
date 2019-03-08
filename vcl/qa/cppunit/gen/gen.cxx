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
#include <vcl/pngwrite.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>

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
    const Size& rSize = aBitmap.GetPrefSize();
    Color aColor(pAccess->GetPixel(rSize.getWidth() / 2, rSize.getHeight() / 2).GetColor());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 255;
    // Actual  : 1'. I.e. center if the preview (which has the background color) was ~black, not
    // white.
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetRed()));
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetBlue()));
    CPPUNIT_ASSERT_EQUAL(0xff, int(aColor.GetGreen()));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
