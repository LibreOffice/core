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
#include <o3tl/string_view.hxx>

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
    void testDetectWMZ();
    void testDetectPCX();
    void testDetectJPG();
    void testDetectPNG();
    void testDetectAPNG();
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
    void testDetectWEBP();
    void testDetectEMF();
    void testDetectEMZ();
    void testMatchArray();
    void testCheckArrayForMatchingStrings();

    CPPUNIT_TEST_SUITE(GraphicFormatDetectorTest);
    CPPUNIT_TEST(testDetectMET);
    CPPUNIT_TEST(testDetectBMP);
    CPPUNIT_TEST(testDetectWMF);
    CPPUNIT_TEST(testDetectWMZ);
    CPPUNIT_TEST(testDetectPCX);
    CPPUNIT_TEST(testDetectJPG);
    CPPUNIT_TEST(testDetectPNG);
    CPPUNIT_TEST(testDetectAPNG);
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
    CPPUNIT_TEST(testDetectWEBP);
    CPPUNIT_TEST(testDetectEMF);
    CPPUNIT_TEST(testDetectEMZ);
    CPPUNIT_TEST(testMatchArray);
    CPPUNIT_TEST(testCheckArrayForMatchingStrings);
    CPPUNIT_TEST_SUITE_END();
};

void GraphicFormatDetectorTest::testDetectMET()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.met"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"MET"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkMET());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"MET"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectBMP()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.bmp"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"BMP"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkBMP());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"BMP"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectWMF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.wmf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"WMF"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"WMF"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectWMZ()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.wmz"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"WMZ"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"WMZ"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPCX()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.pcx"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"PCX"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPCX());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"PCX"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectJPG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.jpg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"JPG"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkJPG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"JPG"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPNG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.png"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"PNG"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPNG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"PNG"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectAPNG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.apng"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"APNG"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkAPNG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"APNG"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectGIF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.gif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"GIF"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkGIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"GIF"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPSD()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.psd"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"PSD"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPSD());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"PSD"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTGA()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.tga"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"TGA"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTGA());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension(u"TGA"_ustr); // detection is based on extension only
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"TGA"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTIF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.tif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"TIF"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"TIF"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXBM()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.xbm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"XBM"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXBM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"XBM"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXPM()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.xpm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"XPM"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXPM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"XPM"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVG()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.svg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"SVG"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"SVG"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVGZ()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.svgz"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"SVGZ"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"SVGZ"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPDF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.pdf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"PDF"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPDF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"PDF"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectEPS()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.eps"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"EPS"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkEPS());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"EPS"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectWEBP()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.webp"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"WEBP"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWEBP());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"WEBP"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectEMF()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.emf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"EMF"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkEMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"EMF"_ustr, rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectEMZ()
{
    SvFileStream aFileStream(getFullUrl(u"TypeDetectionExample.emz"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, u"EMZ"_ustr);

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkEMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(vcl::peekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(u"EMZ"_ustr, rFormatExtension);
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
    pMatchPointer = vcl::matchArrayWithString(pCompleteStringPointer, nCheckSize, "<?xml"_ostr);
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(0, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, o3tl::starts_with(pMatchPointer, "<?xml"));

    // Check middle of the input string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "version"_ostr);
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(6, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, o3tl::starts_with(pMatchPointer, "version"));

    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "<svg"_ostr);
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(38, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, o3tl::starts_with(pMatchPointer, "<svg"));

    // Check end of the input string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "/svg>"_ostr);
    CPPUNIT_ASSERT(pMatchPointer != nullptr);
    CPPUNIT_ASSERT_EQUAL(119, int(pMatchPointer - pCompleteStringPointer));
    CPPUNIT_ASSERT_EQUAL(true, o3tl::starts_with(pMatchPointer, "/svg>"));

    // Check that non-existing search string
    pMatchPointer = vcl::matchArrayWithString(aString.c_str(), nCheckSize, "none"_ostr);
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
    bResult
        = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "<?xml"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check ending string
    bResult
        = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "/svg>"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check middle string
    bResult
        = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "version"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check beginning and then ending string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml"_ostr, "/svg>"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check ending and then beginning string
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "/svg>"_ostr, "<?xml"_ostr });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check middle strings
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "version"_ostr, "<svg"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check beginning, middle and ending strings
    bResult = vcl::checkArrayForMatchingStrings(
        pCompleteStringPointer, nCheckSize,
        { "<?xml"_ostr, "version"_ostr, "<svg"_ostr, "/svg>"_ostr });
    CPPUNIT_ASSERT_EQUAL(true, bResult);

    // check non-existing
    bResult
        = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize, { "none"_ostr });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing on the beginning
    bResult = vcl::checkArrayForMatchingStrings(
        pCompleteStringPointer, nCheckSize,
        { "none"_ostr, "version"_ostr, "<svg"_ostr, "/svg>"_ostr });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing on the end
    bResult = vcl::checkArrayForMatchingStrings(
        pCompleteStringPointer, nCheckSize,
        { "<?xml"_ostr, "version"_ostr, "<svg"_ostr, "none"_ostr });
    CPPUNIT_ASSERT_EQUAL(false, bResult);

    // check non-existing after the end
    bResult = vcl::checkArrayForMatchingStrings(pCompleteStringPointer, nCheckSize,
                                                { "<?xml"_ostr, "/svg>"_ostr, "none"_ostr });
    CPPUNIT_ASSERT_EQUAL(false, bResult);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicFormatDetectorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
