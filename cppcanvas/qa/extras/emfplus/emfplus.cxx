/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/stream.hxx>

using namespace com::sun::star;

class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/cppcanvas/qa/extras/emfplus/data/"_ustr)
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
        // If this is set, the metafile will be dumped as a PNG one for debug purposes.
        char* pEnv = getenv("CPPCANVAS_DEBUG_EMFPLUS_DUMP_TO");
        if (pEnv)
        {
            SvFileStream aStream(OUString::fromUtf8(pEnv), StreamMode::WRITE);
            vcl::PngImageWriter aWriter(aStream);
            CPPUNIT_ASSERT(aWriter.write(aResultBitmap));
        }

        return aResultBitmap.GetBitmap();
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFdo77229)
{
    Bitmap aBitmap = load("fdo77229.emf");
    BitmapScopedReadAccess pAccess(aBitmap);
    // The green star was missing.
    Color aColor(pAccess->GetPixel(284, 280));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), aColor.GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), aColor.GetBlue());
    CPPUNIT_ASSERT(aColor.GetGreen() == 0xfe || aColor.GetGreen() == 0xff);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
