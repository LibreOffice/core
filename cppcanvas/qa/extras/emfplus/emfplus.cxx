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

class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/cppcanvas/qa/extras/emfplus/data/")
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
        // If this is set, the metafile will be dumped as a PNG one for debug purposes.
        char* pEnv = getenv("CPPCANVAS_DEBUG_EMFPLUS_DUMP_TO");
        if (pEnv)
        {
            SvFileStream aStream(OUString::fromUtf8(pEnv), StreamMode::WRITE);
            vcl::PNGWriter aWriter(aResultBitmap);
            CPPUNIT_ASSERT(aWriter.Write(aStream));
        }

        return aResultBitmap.GetBitmap();
    }

    uno::Reference<lang::XComponent> mxComponent;
};

CPPUNIT_TEST_FIXTURE(Test, testFdo77229)
{
    Bitmap aBitmap = load("fdo77229.emf");
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    // The green star was missing.
    Color aColor(pAccess->GetPixel(142, 140));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), aColor.GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), aColor.GetBlue());
    CPPUNIT_ASSERT(aColor.GetGreen() == 0xfe || aColor.GetGreen() == 0xff);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
