/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <graphic/GraphicFormatDetector.hxx>

#include <tools/stream.hxx>

using namespace css;

namespace
{
class GraphicFormatDetectorTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc("/vcl/qa/cppunit/data/") + sFileName;
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
    CPPUNIT_TEST_SUITE_END();
};

void GraphicFormatDetectorTest::testDetectMET()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.met"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "MET");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkMET());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("MET"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectBMP()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.bmp"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "BMP");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkBMP());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("BMP"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectWMF()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.wmf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "WMF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMForEMF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("WMF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPCX()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.pcx"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PCX");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPCX());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PCX"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectJPG()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.jpg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "JPG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkJPG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("JPG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPNG()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.png"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PNG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPNG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PNG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectGIF()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.gif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "GIF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkGIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("GIF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPSD()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.psd"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PSD");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPSD());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PSD"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTGA()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.tga"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "TGA");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTGA());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension("TGA"); // detection is based on extension only
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("TGA"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectTIF()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.tif"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "TIF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkTIF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("TIF"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXBM()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.xbm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "XBM");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXBM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("XBM"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectXPM()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.xpm"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "XPM");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkXPM());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("XPM"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVG()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.svg"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "SVG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("SVG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectSVGZ()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.svgz"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "SVG");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkSVG());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("SVG"), rFormatExtension);
}

void GraphicFormatDetectorTest::testDetectPDF()
{
    SvFileStream aFileStream(getFullUrl("TypeDetectionExample.pdf"), StreamMode::READ);
    vcl::GraphicFormatDetector aDetector(aFileStream, "PDF");

    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkPDF());

    aFileStream.Seek(aDetector.mnStreamPosition);

    OUString rFormatExtension;
    CPPUNIT_ASSERT(ImpPeekGraphicFormat(aFileStream, rFormatExtension, false));
    CPPUNIT_ASSERT_EQUAL(OUString("PDF"), rFormatExtension);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicFormatDetectorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
