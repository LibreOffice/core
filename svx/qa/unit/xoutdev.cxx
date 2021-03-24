/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <sal/types.h>
#include <sfx2/app.hxx>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

class XOutdevTest : public CppUnit::TestFixture
{
public:
    virtual void setUp() override
    {
        CppUnit::TestFixture::setUp();
        SfxApplication::GetOrCreate();
    }
};

CPPUNIT_TEST_FIXTURE(XOutdevTest, testPdfGraphicExport)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // Import the graphic.
    Graphic aGraphic;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"svx/qa/unit/data/graphic.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = aTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "pdf", eFlags);

    // Assert that the output looks like a PDF.
    SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 5);
    sal_uInt8 sFirstBytes[5];
    pStream->ReadBytes(sFirstBytes, 5);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('%'), sFirstBytes[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('D'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('F'), sFirstBytes[3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('-'), sFirstBytes[4]);
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testTdf60684)
{
    Graphic aGraphic;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"svx/qa/unit/data/tdf60684.jpg");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = aTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "png", eFlags);

    SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 4);
    sal_uInt8 sFirstBytes[4];
    pStream->ReadBytes(sFirstBytes, 4);

    //Checks if the file's header matches a PNG's expected header
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('N'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('G'), sFirstBytes[3]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
