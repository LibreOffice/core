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
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>

class XOutdevTest : public CppUnit::TestFixture
{
public:
    void testPdfGraphicExport();

    CPPUNIT_TEST_SUITE(XOutdevTest);
    CPPUNIT_TEST(testPdfGraphicExport);
    CPPUNIT_TEST_SUITE_END();
};

void XOutdevTest::testPdfGraphicExport()
{
#if HAVE_FEATURE_PDFIMPORT
    // Import the graphic.
    Graphic aGraphic;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc("svx/qa/unit/data/graphic.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(GRFILTER_OK), GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    XOutFlags eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = aTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "pdf", eFlags);

    // Assert that the output looks like a PDF.
    SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
    pStream->Seek(STREAM_SEEK_TO_END);
    CPPUNIT_ASSERT(pStream->Tell() > 5);
    pStream->Seek(STREAM_SEEK_TO_BEGIN);
    sal_uInt8 sFirstBytes[5];
    pStream->ReadBytes(sFirstBytes, 5);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('%'), sFirstBytes[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('D'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('F'), sFirstBytes[3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('-'), sFirstBytes[4]);
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(XOutdevTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
