/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/pngwrite.hxx>

using namespace com::sun::star;

class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    Test()
        : mpTestDocumentPath("/cppcanvas/qa/extras/emfplus/data/")
    {
    }

    virtual ~Test()
    {
    }

    virtual void setUp() SAL_OVERRIDE
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    };

    virtual void tearDown() SAL_OVERRIDE
    {
        if (mxComponent.is())
            mxComponent->dispose();
        test::BootstrapFixture::tearDown();
    };

    Bitmap load(const char* pName)
    {
        mxComponent = loadFromDesktop(getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(pName), "com.sun.star.drawing.DrawingDocument");
        SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
        SfxObjectShell* pShell = pModel->GetObjectShell();
        boost::shared_ptr<GDIMetaFile> pMetaFile = pShell->GetPreviewMetaFile();
        BitmapEx aResultBitmap;
        CPPUNIT_ASSERT(pMetaFile->CreateThumbnail(aResultBitmap));
        // If this is set, the metafile will be dumped as a PNG one for debug purposes.
        char* pEnv = getenv("CPPCANVAS_DEBUG_EMFPLUS_DUMP_TO");
        if (pEnv)
        {
            SvFileStream aStream(OStringToOUString(pEnv, RTL_TEXTENCODING_UTF8), STREAM_WRITE);
            vcl::PNGWriter aWriter(aResultBitmap);
            CPPUNIT_ASSERT(aWriter.Write(aStream));
        }

        return aResultBitmap.GetBitmap();
    }

    void testFdo77229();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testFdo77229);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference<lang::XComponent> mxComponent;
    const char* mpTestDocumentPath;
};

void Test::testFdo77229()
{
    Bitmap aBitmap = load("fdo77229.emf");
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    // The green star was missing.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x00fe00), Color(pAccess->GetPixel(140, 140)).GetColor());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
