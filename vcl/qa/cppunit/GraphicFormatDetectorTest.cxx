/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <graphic/GraphicFormatDetector.hxx>
#include <graphic/DetectorTools.hxx>

#include <tools/stream.hxx>

using namespace css;

namespace
{
class GraphicFormatDetectorTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/data/") + sFileName;
    }

    void testDetectMET();
    void testDetectBMP();
    void testDetectWMF();
    void testDetectPCX();
    void testDetectJPG();
    void testDetectPNG();
    void testDetectGIF();
    void testDetectPSD();
    void testDetectTGA();
    void testDetectTIF();
    void testDetectXBM();
    void testDetectXPM();
    void testDetectSVG();
    void testDetectSVGZ();
    void testDetectPDF();
    void testDetectEPS();
    void testMatchArray();
    void testCheckArrayForMatchingStrings();

    CPPUNIT_TEST_SUITE(GraphicFormatDetectorTest);
    CPPUNIT_TEST(testDetectMET);
    CPPUNIT_TEST(testDetectBMP);
    CPPUNIT_TEST(testDetectWMF);
    CPPUNIT_TEST(testDetectPCX);
    CPPUNIT_TEST(testDetectJPG);
    CPPUNIT_TEST(testDetectPNG);
    CPPUNIT_TEST(testDetectGIF);
    CPPUNIT_TEST(testDetectPSD);
    CPPUNIT_TEST(testDetectTGA);
    CPPUNIT_TEST(testDetectTIF);
    CPPUNIT_TEST(testDetectXBM);
    CPPUNIT_TEST(testDetectXPM);
    CPPUNIT_TEST(testDetectSVG);
    CPPUNIT_TEST(testDetectSVGZ);
    CPPUNIT_TEST(testDetectPDF);
    CPPUNIT_TEST(testDetectEPS);
    CPPUNIT_TEST(testMatchArray);
    CPPUNIT_TEST(testCheckArrayForMatchingStrings);
    CPPUNIT_TEST_SUITE_END();
};

void GraphicFormatDetectorTest::testDetectMET()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.met"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "MET");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkMET());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("MET"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectBMP()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.bmp"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "BMP");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkBMP());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("BMP"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectWMF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.wmf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "WMF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMForEMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("WMF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPCX()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.pcx"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PCX");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPCX());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PCX"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectJPG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.jpg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "JPG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkJPG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("JPG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPNG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.png"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PNG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPNG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PNG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectGIF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.gif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "GIF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkGIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("GIF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPSD()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.psd"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PSD");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPSD());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PSD"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTGA()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.tga"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "TGA");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTGA());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension("TGA"); // detection is based on extension only
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("TGA"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTIF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.tif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "TIF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("TIF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXBM()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.xbm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "XBM");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXBM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("XBM"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXPM()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.xpm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "XPM");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXPM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("XPM"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.svg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "SVG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("SVG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVGZ()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.svgz"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "SVG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("SVG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPDF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.pdf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PDF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPDF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PDF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectEPS()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.eps"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "EPS");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkEPS());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("EPS"), rFormatExtension);
}

void GraphicFormatDetectorTest::testMatchArray()
{
    std::string aString("<?xml version=\"1.0\" standalone=\"no\"?>\n"
                        "<svg width=\"5cm\" height=\"4cm\" version=\"1.1\"\n"
                        "xmlns=\"http://www.w3.org/2000/svg\">\n"
                        "</svg>");

    const char* pCompleteStringPointer = aString.c_str();
    const char* pMatchPointer;
    int nCheckSize = aString.size();

    // Check beginning of the input string
    pMatchPointer = vcl::matchArrayWithString(pCompleteStringPointer, nCheckSize, "<?xml");
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(0, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, OString(pMatchPointer).startsWith("<?xml"));

    // Check middle of the input string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "version");
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(6, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, OString(pMatchPointer).startsWith("version"));

    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "<svg");
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(38, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, OString(pMatchPointer).startsWith("<svg"));

    // Check end of the input string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "/svg>");
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(119, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, OString(pMatchPointer).startsWith("/svg>"));

    // Check that non-existing search string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "none");
    CPPUNIT_ASSERT(pMatchPointer == nullptr);
}

void GraphicFormatDetectorTest::testCheckArrayForMatchingStrings()
{
    std::string aString("<?xml version=\"1.0\" standalone=\"no\"?>\n"
                        "<svg width=\"5cm\" height=\"4cm\" version=\"1.1\"\n"
                        "xmlns=\"http://www.w3.org/2000/svg\">\n"
                        "</svg>");
    const char* pCompleteStringPointer = aString.c_str();
    int nCheckSize = aString.size();
    bool bResult;

    // check beginning string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "<?xml" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check ending string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "/svg>" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check middle string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "version" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check beginning and then ending string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml", "/svg>" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check ending and then beginning string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "/svg>", "<?xml" });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check middle strings
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "version", "<svg" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check beginning, middle and ending strings
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml", "version", "<svg", "/svg>" });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check non-existing
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "none" });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing on the beginning
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "none", "version", "<svg", "/svg>" });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing on the end
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml", "version", "<svg", "none" });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing after the end
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml", "/svg>", "none" });
    CPPUNIT_ASSERT_EQUAL(false, bResult);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicFormatDetectorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
