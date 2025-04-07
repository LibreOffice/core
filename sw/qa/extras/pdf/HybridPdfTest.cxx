/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <config_poppler.h>

namespace
{
class HybridPdfTest : public SwModelTestBase
{
public:
    HybridPdfTest()
        : SwModelTestBase(u"/sw/qa/extras/pdf/data/"_ustr)
    {
    }

    void testNoHybridDataInPDF();
    void testHybridWithAdditionalStreams();
    void testHybridWithAdditionalStreamsAndAttachedFile();

    CPPUNIT_TEST_SUITE(HybridPdfTest);
    CPPUNIT_TEST(testNoHybridDataInPDF);
    CPPUNIT_TEST(testHybridWithAdditionalStreams);
    CPPUNIT_TEST(testHybridWithAdditionalStreamsAndAttachedFile);
    CPPUNIT_TEST_SUITE_END();
};

void HybridPdfTest::testNoHybridDataInPDF()
{
#if ENABLE_PDFIMPORT
    // Load PDF document without attached ODT document
    loadFromFile(u"PDFOnly.pdf");
    uno::Reference<lang::XServiceInfo> xServiceInfo(mxComponent, uno::UNO_QUERY_THROW);
    // Draw document is expected in this case - default when importing PDF
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr));
#endif
}

void HybridPdfTest::testHybridWithAdditionalStreams()
{
#if ENABLE_PDFIMPORT
    // Load PDF document with an embedded ODT document
    // The ODT document is embedded in "/AdditionalStreams" structure that is in the PDF trailer
    createSwDoc("Hybrid_AdditionalStreamsOnly.pdf");

    // We can access the document text in a single paragraph that spans multiple rows
    // This wouldn't be possible with a PDF, so the opened document has to be ODT
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. \nThat didn't bode well."_ustr,
                         getParagraph(1)->getString());
#endif
}

void HybridPdfTest::testHybridWithAdditionalStreamsAndAttachedFile()
{
#if ENABLE_PDFIMPORT
    // Load PDF document with an embedded ODT document
    // The ODT document is embedded in "/AdditionalStreams" structure that is in the PDF trailer
    // and is included as an attached file conforming to the PDF specs
    createSwDoc("Hybrid_AdditionalStreamsAndPDFAttachedFile.pdf");

    // We can access the document text in a single paragraph that spans multiple rows
    // This wouldn't be possible with a PDF, so the opened document has to be ODT
    CPPUNIT_ASSERT_EQUAL(u"He heard quiet steps behind him. \nThat didn't bode well."_ustr,
                         getParagraph(1)->getString());
#endif
}

} // end of anonymous namespace
CPPUNIT_TEST_SUITE_REGISTRATION(HybridPdfTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
