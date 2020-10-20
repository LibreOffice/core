/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/virdev.hxx>
#include <bitmapwriteaccess.hxx>
#include <vcl/pngwrite.hxx>

#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLHelper.hxx>
#endif
#include <vcl/skia/SkiaHelper.hxx>

using namespace css;

class SvmTest : public test::BootstrapFixture, public XmlTestTools
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

    void checkRendering(ScopedVclPtrInstance<VirtualDevice> const & pVirtualDev, const GDIMetaFile& rMetaFile);

    // write GDI Metafile to a file in data directory
    // only use this for new tests to create the svm file
    void writeToFile(GDIMetaFile& rMetaFile, OUString const & rName);

    GDIMetaFile writeAndReadStream(GDIMetaFile& rMetaFile, OUString const & rName = OUString());

    GDIMetaFile readFile(const OUString& sName);

    xmlDocUniquePtr dumpMeta(const GDIMetaFile& rMetaFile);

    void checkVirtualDevice(const xmlDocUniquePtr& pDoc);
    void checkErase(const xmlDocUniquePtr& pDoc);

    void checkPixel(const GDIMetaFile& rMetaFile);
    void testPixel();

    void checkPoint(const GDIMetaFile& rMetaFile);
    void testPoint();

    void checkLine(const GDIMetaFile& rMetaFile);
    void testLine();

    void checkRect(const GDIMetaFile& rMetaFile);
    void testRect();

    void checkRoundRect(const GDIMetaFile& rMetaFile);
    void testRoundRect();

    void checkEllipse(const GDIMetaFile& rMetaFile);
    void testEllipse();

    void checkArc(const GDIMetaFile& rMetaFile);
    void testArc();

    void checkPie(const GDIMetaFile& rMetaFile);
    void testPie();

    void checkChord(const GDIMetaFile& rMetaFile);
    void testChord();

    void checkPolyLine(const GDIMetaFile& rMetaFile);
    void testPolyLine();

    void checkPolygon(const GDIMetaFile& rMetaFile);
    void testPolygon();

    void checkPolyPolygon(const GDIMetaFile& rMetaFile);
    void testPolyPolygon();

    void checkText(const GDIMetaFile& rMetaFile);
    void testText();

    void checkTextArray(const GDIMetaFile& rMetaFile);
    void testTextArray();

    void checkstretchText(const GDIMetaFile& rMetaFile);
    void teststretchText();

    void checkTextRect(const GDIMetaFile& rMetaFile);
    void testTextRect();

    void checkTextLine(const GDIMetaFile& rMetaFile);
    void testTextLine();

    void checkBitmaps(const GDIMetaFile& rMetaFile);
    void testBitmaps();

    void checkBitmapExs(const GDIMetaFile& rMetaFile);
    void testBitmapExs();

    void checkMasks(const GDIMetaFile& rMetaFile);
    void testMasks();

    void checkGradient(const GDIMetaFile& rMetaFile);
    void testGradient();

    //void checkGradientEx(const GDIMetaFile& rMetaFile);
    void testGradientEx();

    void checkHatch(const GDIMetaFile& rMetaFile);
    void testHatch();

    void checkWallpaper(const GDIMetaFile& rMetaFile);
    void testWallpaper();

    void checkClipRegion(const GDIMetaFile& rMetaFile);
    void testClipRegion();

    //void checkIntersectRectClipRegion(const GDIMetaFile& rMetaFile);
    void testIntersectRectClipRegion();

    //void checkIntersectRegionClipRegion(const GDIMetaFile& rMetaFile);
    void testIntersectRegionClipRegion();

    //void checkMoveClipRegion(const GDIMetaFile& rMetaFile);
    void testMoveClipRegion();

    void checkLineColor(const GDIMetaFile& rMetaFile);
    void testLineColor();

    void checkFillColor(const GDIMetaFile& rMetaFile);
    void testFillColor();

    void checkTextColor(const GDIMetaFile& rMetaFile);
    void testTextColor();

    void checkTextFillColor(const GDIMetaFile& rMetaFile);
    void testTextFillColor();

    void checkTextLineColor(const GDIMetaFile& rMetaFile);
    void testTextLineColor();

    void checkOverLineColor(const GDIMetaFile& rMetaFile);
    void testOverLineColor();

    void checkTextAlign(const GDIMetaFile& rMetaFile);
    void testTextAlign();

    //void checkMapMode(const GDIMetaFile& rMetaFile);
    void testMapMode();

    //void checkFont(const GDIMetaFile& rMetaFile);
    void testFont();

    void checkPushPop(const GDIMetaFile& rMetaFile);
    void testPushPop();

    void checkRasterOp(const GDIMetaFile& rMetaFile);
    void testRasterOp();

    void checkTransparent(const GDIMetaFile& rMetaFile);
    void testTransparent();

    //void checkFloatTransparent(const GDIMetaFile& rMetaFile);
    void testFloatTransparent();

    //void checkEPS(const GDIMetaFile& rMetaFile);
    void testEPS();

    //void checkRefPoint(const GDIMetaFile& rMetaFile);
    void testRefPoint();

    //void checkComment(const GDIMetaFile& rMetaFile);
    void testComment();

    //void checkLayoutMode(const GDIMetaFile& rMetaFile);
    void testLayoutMode();

    //void checkTextLanguage(const GDIMetaFile& rMetaFile);
    void testTextLanguage();

public:
    SvmTest()
        : BootstrapFixture(true, false)
        , maDataUrl("/vcl/qa/cppunit/svm/data/")
    {}

    CPPUNIT_TEST_SUITE(SvmTest);
    CPPUNIT_TEST(testPixel);
    CPPUNIT_TEST(testPoint);
    CPPUNIT_TEST(testLine);
    CPPUNIT_TEST(testRect);
    CPPUNIT_TEST(testRoundRect);
    CPPUNIT_TEST(testEllipse);
    CPPUNIT_TEST(testArc);
    CPPUNIT_TEST(testPie);
    CPPUNIT_TEST(testChord);
    CPPUNIT_TEST(testPolyLine);
    CPPUNIT_TEST(testPolygon);
    CPPUNIT_TEST(testPolyPolygon);
    CPPUNIT_TEST(testText);
    CPPUNIT_TEST(testTextArray);
    CPPUNIT_TEST(teststretchText);
    CPPUNIT_TEST(testTextRect);
    CPPUNIT_TEST(testTextLine);
    CPPUNIT_TEST(testBitmaps); // BMP, BMPSCALE, BMPSCALEPART
    CPPUNIT_TEST(testBitmapExs); // BMPEX, BMPEXSCALE, BMPEXSCALEPART
    CPPUNIT_TEST(testMasks); // MASK, MASKSCALE, MASKSCALEPART
    CPPUNIT_TEST(testGradient);
    CPPUNIT_TEST(testGradientEx);
    CPPUNIT_TEST(testHatch);
    CPPUNIT_TEST(testWallpaper);
    CPPUNIT_TEST(testClipRegion);
    CPPUNIT_TEST(testIntersectRectClipRegion);
    CPPUNIT_TEST(testIntersectRegionClipRegion);
    CPPUNIT_TEST(testMoveClipRegion);
    CPPUNIT_TEST(testLineColor);
    CPPUNIT_TEST(testFillColor);
    CPPUNIT_TEST(testTextColor);
    CPPUNIT_TEST(testTextFillColor);
    CPPUNIT_TEST(testTextLineColor);
    CPPUNIT_TEST(testOverLineColor);
    CPPUNIT_TEST(testTextAlign);
    CPPUNIT_TEST(testMapMode);
    CPPUNIT_TEST(testFont);
    CPPUNIT_TEST(testPushPop);
    CPPUNIT_TEST(testRasterOp);
    CPPUNIT_TEST(testTransparent);
    CPPUNIT_TEST(testFloatTransparent);
    CPPUNIT_TEST(testEPS);
    CPPUNIT_TEST(testRefPoint);
    CPPUNIT_TEST(testComment);
    CPPUNIT_TEST(testLayoutMode);
    CPPUNIT_TEST(testTextLanguage);

    CPPUNIT_TEST_SUITE_END();
};

static void setupBaseVirtualDevice(VirtualDevice& rDevice, GDIMetaFile& rMeta)
{
    rDevice.SetConnectMetaFile(&rMeta);
    Size aVDSize(10, 10);
    rDevice.SetOutputSizePixel(aVDSize);
    rDevice.SetBackground(Wallpaper(COL_LIGHTRED));
    rDevice.Erase();
}

void SvmTest::checkRendering(ScopedVclPtrInstance<VirtualDevice> const & pVirtualDev, const GDIMetaFile& rMetaFile)
{
    BitmapEx aSourceBitmapEx = pVirtualDev->GetBitmapEx(Point(), Size(10, 10));
    ScopedVclPtrInstance<VirtualDevice> pVirtualDevResult;
    pVirtualDevResult->SetOutputSizePixel(Size(10, 10));
    const_cast<GDIMetaFile&>(rMetaFile).Play(pVirtualDevResult.get());
    BitmapEx aResultBitmapEx = pVirtualDevResult->GetBitmapEx(Point(), Size(10, 10));

    const bool bWriteCompareBitmap = false;

    if (bWriteCompareBitmap)
    {
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();

        {
            SvFileStream aStream(aTempFile.GetURL() + ".source.png", StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PNGWriter aPNGWriter(aSourceBitmapEx);
            aPNGWriter.Write(aStream);
        }
        {
            SvFileStream aStream(aTempFile.GetURL() + ".result.png", StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PNGWriter aPNGWriter(aResultBitmapEx);
            aPNGWriter.Write(aStream);
        }
    }
    CPPUNIT_ASSERT_EQUAL(aSourceBitmapEx.GetChecksum(), aResultBitmapEx.GetChecksum());
}

static GDIMetaFile readMetafile(const OUString& rUrl)
{
    GDIMetaFile aResultMetafile;
    SvFileStream aFileStream(rUrl, StreamMode::READ);
    aFileStream.Seek(STREAM_SEEK_TO_BEGIN);
    aResultMetafile.Read(aFileStream);
    return aResultMetafile;
}

static void writeMetaFile(GDIMetaFile& rInputMetafile, const OUString& rUrl)
{
    SvFileStream aFileStream(rUrl, StreamMode::WRITE);
    aFileStream.Seek(STREAM_SEEK_TO_BEGIN);
    rInputMetafile.Write(aFileStream);
    aFileStream.Close();
}

void SvmTest::writeToFile(GDIMetaFile& rMetaFile, OUString const & rName)
{
    if (rName.isEmpty())
        return;
    OUString sFilePath = getFullUrl(rName);
    writeMetaFile(rMetaFile, sFilePath);
}

GDIMetaFile SvmTest::writeAndReadStream(GDIMetaFile& rMetaFile, OUString const & rName)
{
    if (!rName.isEmpty())
        writeToFile(rMetaFile, rName);

    SvMemoryStream aStream;
    rMetaFile.Write(aStream);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    GDIMetaFile aResultMetafile;
    aResultMetafile.Read(aStream);
    return aResultMetafile;
}

GDIMetaFile SvmTest::readFile(const OUString& sName)
{
    OUString sFilePath = getFullUrl(sName);
    return readMetafile(sFilePath);
}

xmlDocUniquePtr SvmTest::dumpMeta(const GDIMetaFile& rMetaFile)
{
    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, rMetaFile);
    CPPUNIT_ASSERT (pDoc);

    checkVirtualDevice(pDoc);
    checkErase(pDoc);

    return pDoc;
}

void SvmTest::checkVirtualDevice(const xmlDocUniquePtr& pDoc)
{
    assertXPath(pDoc, "/metafile/linecolor[1]", "color", "#000000");
    assertXPath(pDoc, "/metafile/fillcolor[1]", "color", "#ffffff");

    assertXPathAttrs(pDoc, "/metafile/rect[1]", {
        {"left", "0"},  {"top", "0"},
        {"right", "9"}, {"bottom", "9"}
    });

    assertXPath(pDoc, "/metafile/linecolor[2]", "color", "#000000");
    assertXPath(pDoc, "/metafile/fillcolor[2]", "color", "#ffffff");
}

void SvmTest::checkErase(const xmlDocUniquePtr& pDoc)
{
    assertXPath(pDoc, "/metafile/linecolor[3]", "color", "#000000");
    assertXPath(pDoc, "/metafile/fillcolor[3]", "color", "#ff0000");

    assertXPathAttrs(pDoc, "/metafile/rect[2]", {
        {"left", "0"},  {"top", "0"},
        {"right", "9"}, {"bottom", "9"}
    });
}

void SvmTest::checkPixel(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/pixel[1]", {
        {"x", "8"}, {"y", "1"}, {"color", "#008000"},
    });

    assertXPathAttrs(pDoc, "/metafile/pixel[2]", {
        {"x", "1"}, {"y", "8"}, {"color", "#000080"},
    });
}

void SvmTest::testPixel()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->DrawPixel(Point(8, 1), COL_GREEN);
    pVirtualDev->DrawPixel(Point(1, 8), COL_BLUE);

    checkPixel(writeAndReadStream(aGDIMetaFile));
    checkPixel(readFile("pixel.svm"));
}

void SvmTest::checkPoint(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/point[1]", {
        {"x", "4"}, {"y", "4"}
    });
}

void SvmTest::testPoint()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->DrawPixel(Point(4, 4));

    checkPoint(writeAndReadStream(aGDIMetaFile));
    checkPoint(readFile("point.svm"));
}

void SvmTest::checkLine(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/line[1]", {
        {"startx", "1"}, {"starty", "1"},
        {"endx", "8"},   {"endy", "8"},
    });

    assertXPathAttrs(pDoc, "/metafile/line[1]", {
        {"style", "solid"}, {"width", "0"},
        {"dashlen", "0"},   {"dashcount", "0"},
        {"dotlen", "0"},    {"dotcount", "0"},
        {"distance", "0"},
        {"join", "round"},  {"cap", "butt"}
    });

    assertXPathAttrs(pDoc, "/metafile/line[2]", {
        {"startx", "1"}, {"starty", "8"},
        {"endx", "8"},   {"endy", "1"},
    });

    assertXPathAttrs(pDoc, "/metafile/line[2]", {
        {"style", "dash"},  {"width", "7"},
        {"dashlen", "5"},   {"dashcount", "4"},
        {"dotlen", "3"},    {"dotcount", "2"},
        {"distance", "1"},
        {"join", "miter"},  {"cap", "round"}
    });
}

void SvmTest::testLine()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->DrawLine(Point(1, 1), Point(8, 8));
    LineInfo aLineInfo(LineStyle::Dash, 7);
    aLineInfo.SetDashLen(5);
    aLineInfo.SetDashCount(4);
    aLineInfo.SetDotLen(3);
    aLineInfo.SetDotCount(2);
    aLineInfo.SetDistance(1);
    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Miter);
    aLineInfo.SetLineCap(css::drawing::LineCap_ROUND);
    pVirtualDev->DrawLine(Point(1, 8), Point(8, 1), aLineInfo);

    checkLine(writeAndReadStream(aGDIMetaFile));
    checkLine(readFile("line.svm"));
}

void SvmTest::checkRect(const GDIMetaFile& rMetaFile)
{

    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/rect[3]", {
        {"left", "1"}, {"top", "2"},
        {"right", "4"},   {"bottom", "5"},
    });
}

void SvmTest::testRect()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawRect(tools::Rectangle(Point(1, 2), Size(4, 4)));

    checkRect(writeAndReadStream(aGDIMetaFile));
    checkRect(readFile("rect.svm"));
}

void SvmTest::checkRoundRect(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/roundrect[1]", {
        {"left", "1"}, {"top", "2"},
        {"right", "4"},   {"bottom", "5"},
        {"horizontalround", "1"}, {"verticalround", "2"}
    });
}

void SvmTest::testRoundRect()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawRect(tools::Rectangle(Point(1, 2), Size(4, 4)), 1, 2);

    checkRoundRect(writeAndReadStream(aGDIMetaFile));
    checkRoundRect(readFile("roundrect.svm"));
}

void SvmTest::checkEllipse(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/ellipse[1]", {
        {"left", "1"}, {"top", "2"},
        {"right", "4"},   {"bottom", "5"},
    });
}

void SvmTest::testEllipse()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawEllipse(tools::Rectangle(Point(1, 2), Size(4, 4)));

    checkEllipse(writeAndReadStream(aGDIMetaFile));
    checkEllipse(readFile("ellipse.svm"));
}

void SvmTest::checkArc(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/arc[1]", {
        {"left",  "1"}, {"top",    "2"},
        {"right", "4"}, {"bottom", "5"},

        {"startx", "10"}, {"starty", "11"},
        {"endx",   "12"}, {"endy",   "13"},
    });
}

void SvmTest::testArc()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawArc(tools::Rectangle(Point(1, 2), Size(4, 4)), Point(10, 11), Point(12, 13));

    checkArc(writeAndReadStream(aGDIMetaFile));
    checkArc(readFile("arc.svm"));
}

void SvmTest::checkPie(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/pie[1]", {
        {"left",  "11"}, {"top",    "12"},
        {"right", "14"}, {"bottom", "15"},

        {"startx", "20"}, {"starty", "21"},
        {"endx",   "22"}, {"endy",   "23"},
    });
}

void SvmTest::testPie()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawPie(tools::Rectangle(Point(11, 12), Size(4, 4)), Point(20, 21), Point(22, 23));

    checkPie(writeAndReadStream(aGDIMetaFile));
    checkPie(readFile("pie.svm"));
}

void SvmTest::checkChord(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#123456");
    assertXPath(pDoc, "/metafile/fillcolor[5]", "color", "#654321");

    assertXPathAttrs(pDoc, "/metafile/chord[1]", {
        {"left",  "21"}, {"top",    "22"},
        {"right", "24"}, {"bottom", "25"},

        {"startx", "30"}, {"starty", "31"},
        {"endx",   "32"}, {"endy",   "33"},
    });
}

void SvmTest::testChord()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(Color(0x123456));
    pVirtualDev->SetFillColor(Color(0x654321));

    pVirtualDev->DrawChord(tools::Rectangle(Point(21, 22), Size(4, 4)), Point(30, 31), Point(32, 33));

    checkChord(writeAndReadStream(aGDIMetaFile));
    checkChord(readFile("chord.svm"));
}

void SvmTest::checkPolyLine(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/polyline[1]", {
        {"style", "solid"}, {"width", "0"},
        {"dashlen", "0"},   {"dashcount", "0"},
        {"dotlen", "0"},    {"dotcount", "0"},
        {"distance", "0"},
        {"join", "round"},  {"cap", "butt"}
    });

    assertXPathAttrs(pDoc, "/metafile/polyline[1]/point[1]", {{"x", "1"}, {"y", "8"}});
    assertXPathAttrs(pDoc, "/metafile/polyline[1]/point[2]", {{"x", "2"}, {"y", "7"}});
    assertXPathAttrs(pDoc, "/metafile/polyline[1]/point[3]", {{"x", "3"}, {"y", "6"}});

    assertXPathAttrs(pDoc, "/metafile/polyline[2]", {
        {"style", "dash"},  {"width", "7"},
        {"dashlen", "5"},   {"dashcount", "4"},
        {"dotlen", "3"},    {"dotcount", "2"},
        {"distance", "1"},
        {"join", "miter"},  {"cap", "round"}
    });
    assertXPathAttrs(pDoc, "/metafile/polyline[2]/point[1]", {{"x", "8"}, {"y", "1"}, {"flags", "normal"}});
    assertXPathAttrs(pDoc, "/metafile/polyline[2]/point[2]", {{"x", "7"}, {"y", "2"}, {"flags", "control"}});
    assertXPathAttrs(pDoc, "/metafile/polyline[2]/point[3]", {{"x", "6"}, {"y", "3"}, {"flags", "smooth"}});
    assertXPathAttrs(pDoc, "/metafile/polyline[2]/point[4]", {{"x", "5"}, {"y", "4"}, {"flags", "symmetric"}});
}

void SvmTest::testPolyLine()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    pVirtualDev->DrawPolyLine(aPolygon);

    tools::Polygon aPolygonWithControl(4);
    aPolygonWithControl.SetPoint(Point(8, 1), 0);
    aPolygonWithControl.SetPoint(Point(7, 2), 1);
    aPolygonWithControl.SetPoint(Point(6, 3), 2);
    aPolygonWithControl.SetPoint(Point(5, 4), 3);

    aPolygonWithControl.SetFlags(0, PolyFlags::Normal);
    aPolygonWithControl.SetFlags(1, PolyFlags::Control);
    aPolygonWithControl.SetFlags(2, PolyFlags::Smooth);
    aPolygonWithControl.SetFlags(3, PolyFlags::Symmetric);

    LineInfo aLineInfo(LineStyle::Dash, 7);
    aLineInfo.SetDashLen(5);
    aLineInfo.SetDashCount(4);
    aLineInfo.SetDotLen(3);
    aLineInfo.SetDotCount(2);
    aLineInfo.SetDistance(1);
    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Miter);
    aLineInfo.SetLineCap(css::drawing::LineCap_ROUND);

    pVirtualDev->DrawPolyLine(aPolygonWithControl, aLineInfo);

    checkPolyLine(writeAndReadStream(aGDIMetaFile));
    checkPolyLine(readFile("polyline.svm"));
}

void SvmTest::checkPolygon(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/polygon[1]/point[1]", {{"x", "1"}, {"y", "8"}});
    assertXPathAttrs(pDoc, "/metafile/polygon[1]/point[2]", {{"x", "2"}, {"y", "7"}});
    assertXPathAttrs(pDoc, "/metafile/polygon[1]/point[3]", {{"x", "3"}, {"y", "6"}});

    assertXPathAttrs(pDoc, "/metafile/polygon[2]/point[1]", {{"x", "8"}, {"y", "1"}, {"flags", "normal"}});
    assertXPathAttrs(pDoc, "/metafile/polygon[2]/point[2]", {{"x", "7"}, {"y", "2"}, {"flags", "control"}});
    assertXPathAttrs(pDoc, "/metafile/polygon[2]/point[3]", {{"x", "6"}, {"y", "3"}, {"flags", "smooth"}});
    assertXPathAttrs(pDoc, "/metafile/polygon[2]/point[4]", {{"x", "5"}, {"y", "4"}, {"flags", "symmetric"}});
}

void SvmTest::testPolygon()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    pVirtualDev->DrawPolygon(aPolygon);

    tools::Polygon aPolygonWithControl(4);
    aPolygonWithControl.SetPoint(Point(8, 1), 0);
    aPolygonWithControl.SetPoint(Point(7, 2), 1);
    aPolygonWithControl.SetPoint(Point(6, 3), 2);
    aPolygonWithControl.SetPoint(Point(5, 4), 3);

    aPolygonWithControl.SetFlags(0, PolyFlags::Normal);
    aPolygonWithControl.SetFlags(1, PolyFlags::Control);
    aPolygonWithControl.SetFlags(2, PolyFlags::Smooth);
    aPolygonWithControl.SetFlags(3, PolyFlags::Symmetric);

    pVirtualDev->DrawPolygon(aPolygonWithControl);

    checkPolygon(writeAndReadStream(aGDIMetaFile));
    checkPolygon(readFile("polygon.svm"));
}

void SvmTest::checkPolyPolygon(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[1]/point[1]", {{"x", "1"}, {"y", "8"}});
    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[1]/point[2]", {{"x", "2"}, {"y", "7"}});
    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[1]/point[3]", {{"x", "3"}, {"y", "6"}});

    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[2]/point[1]", {{"x", "8"}, {"y", "1"}, {"flags", "normal"}});
    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[2]/point[2]", {{"x", "7"}, {"y", "2"}, {"flags", "control"}});
    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[2]/point[3]", {{"x", "6"}, {"y", "3"}, {"flags", "smooth"}});
    assertXPathAttrs(pDoc, "/metafile/polypolygon[1]/polygon[2]/point[4]", {{"x", "5"}, {"y", "4"}, {"flags", "symmetric"}});
}

void SvmTest::testPolyPolygon()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    tools::Polygon aPolygonWithControl(4);
    aPolygonWithControl.SetPoint(Point(8, 1), 0);
    aPolygonWithControl.SetPoint(Point(7, 2), 1);
    aPolygonWithControl.SetPoint(Point(6, 3), 2);
    aPolygonWithControl.SetPoint(Point(5, 4), 3);

    aPolygonWithControl.SetFlags(0, PolyFlags::Normal);
    aPolygonWithControl.SetFlags(1, PolyFlags::Control);
    aPolygonWithControl.SetFlags(2, PolyFlags::Smooth);
    aPolygonWithControl.SetFlags(3, PolyFlags::Symmetric);

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(aPolygon);
    aPolyPolygon.Insert(aPolygonWithControl);

    pVirtualDev->DrawPolyPolygon(aPolyPolygon);

    checkPolyPolygon(writeAndReadStream(aGDIMetaFile));
    checkPolyPolygon(readFile("polypolygon.svm"));
}

void SvmTest::checkText(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/text[1]", {
        {"x", "4"}, {"y", "6"}, {"index", "1"}, {"length", "2"},
    });

    assertXPathContent(pDoc, "/metafile/text[1]/textcontent", "xABC");
}

void SvmTest::testText()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->DrawText(Point(4,6), "xABC", 1, 2);

    checkText(writeAndReadStream(aGDIMetaFile));
    checkText(readFile("text.svm"));
}

void SvmTest::checkTextArray(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textarray[1]", {
        {"x", "4"}, {"y", "6"}, {"index", "1"}, {"length", "4"},
    });
    assertXPathContent(pDoc, "/metafile/textarray[1]/dxarray", "15 20 25 ");
    assertXPathContent(pDoc, "/metafile/textarray[1]/text", "123456");
}

void SvmTest::testTextArray()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);
    tools::Long const aDX[] = { 10, 15, 20, 25, 30, 35 };
    pVirtualDev->DrawTextArray(Point(4,6), "123456", aDX, 1, 4);

    checkTextArray(writeAndReadStream(aGDIMetaFile));
    checkTextArray(readFile("textarray.svm"));
}

void SvmTest::checkstretchText(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/stretchtext[1]", {
        {"x", "4"}, {"y", "6"}, {"index", "1"}, {"length", "4"}, {"width", "10"}
    });

    assertXPathContent(pDoc, "/metafile/stretchtext[1]/textcontent", "123456");
}

void SvmTest::teststretchText()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);
    pVirtualDev->DrawStretchText(Point(4,6), 10, "123456", 1, 4);

    checkstretchText(writeAndReadStream(aGDIMetaFile));
    checkstretchText(readFile("strecthtext.svm"));
}

void SvmTest::checkTextRect(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textrect[1]", {
        {"left", "0"}, {"top", "0"}, {"right", "4"}, {"bottom", "4"}
    });
    assertXPathContent(pDoc, "/metafile/textrect[1]/textcontent", "123456");
    assertXPathContent(pDoc, "/metafile/textrect[1]/style", "Center");
}

void SvmTest::testTextRect()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);
    pVirtualDev->DrawText(tools::Rectangle(Point(0,0), Size(5,5)), "123456", DrawTextFlags::Center);

    checkTextRect(writeAndReadStream(aGDIMetaFile));
    checkTextRect(readFile("textrectangle.svm"));
}

void SvmTest::checkTextLine(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textline[1]", {
        {"x", "4"}, {"y", "6"}, {"width", "10"},
        {"strikeout", "single"}, {"underline", "single"}, {"overline", "single"}
    });
}

void SvmTest::testTextLine()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);
    pVirtualDev->DrawTextLine(Point(4,6), 10, STRIKEOUT_SINGLE, LINESTYLE_SINGLE, LINESTYLE_SINGLE);

    checkTextLine(writeAndReadStream(aGDIMetaFile));
    checkTextLine(readFile("textline.svm"));
}

void SvmTest::checkBitmaps(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    if (SkiaHelper::isVCLSkiaEnabled())
        return; // TODO SKIA using CRCs is broken (the idea of it)

    OUString crc1 = "b8dee5da";
    OUString crc2 = "281fc589";
    OUString crc3 = "5e01ddcc";
#if HAVE_FEATURE_OPENGL
    if (OpenGLHelper::isVCLOpenGLEnabled())
    {
        // OpenGL uses a different scaling algorithm and also a different RGB order.
        crc1 = "5e01ddcc";
        crc2 = "281fc589";
        crc3 = "b8dee5da";
    }
#endif

    assertXPathAttrs(pDoc, "/metafile/bmp[1]", {{"x", "1"}, {"y", "2"}, {"crc", crc1}});
    assertXPathAttrs(pDoc, "/metafile/bmpscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"}, {"crc", crc2}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"crc", crc3}
    });
}

void SvmTest::testBitmaps()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    Bitmap aBitmap1(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap1);
        pAccess->Erase(COL_RED);
    }
    Bitmap aBitmap2(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap2);
        pAccess->Erase(COL_GREEN);
    }
    Bitmap aBitmap3(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap3);
        pAccess->Erase(COL_BLUE);
    }
    pVirtualDev->DrawBitmap(Point(1, 2), aBitmap1);
    pVirtualDev->DrawBitmap(Point(1, 2), Size(3, 4), aBitmap2);
    pVirtualDev->DrawBitmap(Point(1, 2), Size(3, 4), Point(2, 1), Size(4, 3), aBitmap3);

    {
        GDIMetaFile aReloadedGDIMetaFile = writeAndReadStream(aGDIMetaFile);
        checkBitmaps(aReloadedGDIMetaFile);
        checkRendering(pVirtualDev, aReloadedGDIMetaFile);
    }
    {
        GDIMetaFile aFileGDIMetaFile = readFile("bitmaps.svm");
        checkBitmaps(aFileGDIMetaFile);
        checkRendering(pVirtualDev, aFileGDIMetaFile);
    }
}

void SvmTest::checkBitmapExs(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    if (SkiaHelper::isVCLSkiaEnabled())
        return; // TODO SKIA using CRCs is broken (the idea of it)

    std::vector<OUString> aExpectedCRC;

#if HAVE_FEATURE_OPENGL
    if (OpenGLHelper::isVCLOpenGLEnabled())
    {
        aExpectedCRC.insert(aExpectedCRC.end(),
        {
            "08feb5d3",
            "281fc589",
            "b8dee5da",
            "4df0e464",
            "4322ee3a",
            "1426653b",
            "4fd547df",
            "71efc447",
        });
    }
    else
#endif
    {
        aExpectedCRC.insert(aExpectedCRC.end(),
        {
            "d8377d4f",
            "281fc589",
            "5e01ddcc",
            "4df0e464",
            "4322ee3a",
            "794c92a9",
            "3c80d829",
            "71efc447",
        });
    }

    assertXPathAttrs(pDoc, "/metafile/bmpex[1]", {
        {"x", "1"}, {"y", "1"}, {"crc", aExpectedCRC[0]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscale[1]", {
        {"x", "5"}, {"y", "0"}, {"width", "2"}, {"height", "3"},
        {"crc", aExpectedCRC[1]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscalepart[1]", {
        {"destx", "7"}, {"desty", "1"}, {"destwidth", "2"}, {"destheight", "2"},
        {"srcx", "0"},  {"srcy", "0"},  {"srcwidth", "3"},  {"srcheight", "4"},
        {"crc", aExpectedCRC[2]}, {"transparenttype", "bitmap"}
    });

#ifndef MACOSX
    assertXPathAttrs(pDoc, "/metafile/bmpex[2]", {
        {"x", "6"}, {"y", "6"}, {"crc", aExpectedCRC[3]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpex[3]", {
        {"x", "0"}, {"y", "6"}, {"crc", aExpectedCRC[4]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpex[4]", {
        {"x", "2"}, {"y", "6"}, {"crc", aExpectedCRC[5]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpex[5]", {
        {"x", "0"}, {"y", "8"}, {"crc", aExpectedCRC[6]}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpex[6]", {
        {"x", "2"}, {"y", "8"}, {"crc", aExpectedCRC[7]}, {"transparenttype", "bitmap"}
    });
#endif
}

void SvmTest::testBitmapExs()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    // DrawBitmapEx
    {
        Bitmap aBitmap(Size(4,4), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_YELLOW);
        }

        pVirtualDev->DrawBitmapEx(Point(1, 1), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - Scale
    {
        Bitmap aBitmap(Size(4,4), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_GREEN);
        }
        pVirtualDev->DrawBitmapEx(Point(5, 0), Size(2, 3), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - Scale - Part
    {
        Bitmap aBitmap(Size(4,4), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_BLUE);
        }
        pVirtualDev->DrawBitmapEx(Point(7, 1), Size(2, 2), Point(0, 0), Size(3, 4), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - 50% transparent
    {
        Bitmap aBitmap(Size(4, 4), 24);
        AlphaMask aAlpha(Size(4, 4));
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_MAGENTA);

            AlphaScopedWriteAccess pAlphaAccess(aAlpha);
            pAlphaAccess->Erase(Color(128, 128, 128));
        }
        pVirtualDev->DrawBitmapEx(Point(6, 6), BitmapEx(aBitmap, aAlpha));
    }

    // DrawBitmapEx - 1-bit
    {
        Bitmap aBitmap(Size(2, 2), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_MAGENTA);
        }
        aBitmap.Convert(BmpConversion::N1BitThreshold);
        pVirtualDev->DrawBitmapEx(Point(0, 6), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - 4-bit
    {
        Bitmap aBitmap(Size(2, 2), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_MAGENTA);
        }
        aBitmap.Convert(BmpConversion::N4BitColors);
        pVirtualDev->DrawBitmapEx(Point(2, 6), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - 8-bit Color
    {
        Bitmap aBitmap(Size(2, 2), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_MAGENTA);
        }
        aBitmap.Convert(BmpConversion::N8BitColors);
        pVirtualDev->DrawBitmapEx(Point(0, 8), BitmapEx(aBitmap, COL_WHITE));
    }

    // DrawBitmapEx - 8-bit Grey
    {
        Bitmap aBitmap(Size(2, 2), 24);
        {
            BitmapScopedWriteAccess pAccess(aBitmap);
            pAccess->Erase(COL_MAGENTA);
        }
        aBitmap.Convert(BmpConversion::N8BitGreys);
        pVirtualDev->DrawBitmapEx(Point(2, 8), BitmapEx(aBitmap, COL_WHITE));
    }

    {
        GDIMetaFile aReloadedGDIMetaFile = writeAndReadStream(aGDIMetaFile);
        checkBitmapExs(aReloadedGDIMetaFile);
        checkRendering(pVirtualDev, aReloadedGDIMetaFile);
    }
    {
        GDIMetaFile aFileGDIMetaFile = readFile("bitmapexs.svm");
        checkBitmapExs(aFileGDIMetaFile);
        checkRendering(pVirtualDev, aFileGDIMetaFile);
    }
}

void SvmTest::checkMasks(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/mask[1]", {
        {"x", "1"}, {"y", "2"},
        {"color", "#000000"}
    });
    assertXPathAttrs(pDoc, "/metafile/maskscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"},
        {"color", "#000000"}
    });
    assertXPathAttrs(pDoc, "/metafile/maskscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"color", "#ff0000"}
    });
}

// TODO: Masks are kind-of special - we don't persist the color attribute (it is
// always #000000) of the meta-action (which is wrong), but rely on alpha to do
// the right thing.
void SvmTest::testMasks()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    Bitmap aBitmap1(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap1);
        pAccess->Erase(COL_RED);
    }
    Bitmap aBitmap2(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap2);
        pAccess->Erase(COL_GREEN);
    }
    Bitmap aBitmap3(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap3);
        pAccess->Erase(COL_BLUE);
    }

    pVirtualDev->DrawMask(Point(1, 2), aBitmap1, COL_LIGHTRED);
    pVirtualDev->DrawMask(Point(1, 2), Size(3, 4), aBitmap2, COL_LIGHTRED);
    pVirtualDev->DrawMask(Point(1, 2), Size(3, 4), Point(2, 1), Size(4, 3), aBitmap3, COL_LIGHTRED, MetaActionType::MASKSCALEPART);

    checkMasks(writeAndReadStream(aGDIMetaFile));
    checkMasks(readFile("masks.svm"));
}

void SvmTest::checkGradient(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/gradient[1]", {
        {"style", "Linear"},
        {"startcolor", "#ffffff"},
        {"endcolor", "#000000"},
        {"angle", "0"},
        {"border", "0"},
        {"offsetx", "50"},
        {"offsety", "50"},
        {"startintensity", "100"},
        {"endintensity", "100"},
        {"steps", "0"},
    });
    assertXPathAttrs(pDoc, "/metafile/gradient[1]/rectangle", {
        {"left", "1"},
        {"top", "2"},
        {"right", "4"},
        {"bottom", "6"},
    });

    assertXPathAttrs(pDoc, "/metafile/gradient[2]", {
        {"style", "Radial"},
        {"startcolor", "#ff0000"},
        {"endcolor", "#00ff00"},
        {"angle", "55"},
        {"border", "10"},
        {"offsetx", "22"},
        {"offsety", "24"},
        {"startintensity", "4"},
        {"endintensity", "14"},
        {"steps", "64"},
    });
    assertXPathAttrs(pDoc, "/metafile/gradient[2]/rectangle", {
        {"left", "3"},
        {"top", "4"},
        {"right", "3"},
        {"bottom", "5"},
    });
}

void SvmTest::testGradient()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Rectangle aRectangle(Point(1, 2), Size(4,5));

    Gradient aGradient(GradientStyle::Linear, COL_WHITE, COL_BLACK);
    pVirtualDev->DrawGradient(aRectangle, aGradient);

    tools::Rectangle aRectangle2(Point(3, 4), Size(1,2));

    Gradient aGradient2;
    aGradient2.SetStyle(GradientStyle::Radial);
    aGradient2.SetStartColor(COL_LIGHTRED);
    aGradient2.SetEndColor(COL_LIGHTGREEN);
    aGradient2.SetAngle(Degree10(55));
    aGradient2.SetBorder(10);
    aGradient2.SetOfsX(22);
    aGradient2.SetOfsY(24);
    aGradient2.SetStartIntensity(4);
    aGradient2.SetEndIntensity(14);
    aGradient2.SetSteps(64);
    pVirtualDev->DrawGradient(aRectangle2, aGradient2);

    checkGradient(writeAndReadStream(aGDIMetaFile));
    checkGradient(readFile("gradient.svm"));
}

void SvmTest::testGradientEx()
{}

void SvmTest::checkHatch(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/hatch[1]/polygon/point[1]", {
        {"x", "1"}, {"y", "8"},
    });
    assertXPathAttrs(pDoc, "/metafile/hatch[1]/polygon/point[2]", {
        {"x", "2"}, {"y", "7"},
    });
    assertXPathAttrs(pDoc, "/metafile/hatch[1]/polygon/point[3]", {
        {"x", "3"}, {"y", "6"},
    });

    assertXPathAttrs(pDoc, "/metafile/hatch[1]/hatch", {
        {"style", "Single"},
        {"color", "#ffff00"},
        {"distance", "15"},
        {"angle", "900"},
    });
}

void SvmTest::testHatch()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    tools::PolyPolygon aPolyPolygon(1);
    aPolyPolygon.Insert(aPolygon);

    Hatch aHatch(HatchStyle::Single, COL_YELLOW, 15, Degree10(900));

    pVirtualDev->DrawHatch(aPolyPolygon, aHatch);

    checkHatch(writeAndReadStream(aGDIMetaFile));
    checkHatch(readFile("hatch.svm"));
}

void SvmTest::checkWallpaper(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    // Funny enough - we don't serialize the rectangle of the wallpaper so it's always EMPTY
    assertXPathAttrs(pDoc, "/metafile/wallpaper[1]",
    {
        {"left", "0"},
        {"top", "0"},
        {"right", "empty"},
        {"bottom", "empty"},
    });

    assertXPathAttrs(pDoc, "/metafile/wallpaper[1]/wallpaper",
    {
        {"color", "#00ff00"},
        {"style", "Tile"},
        {"fixed", "true"},
        {"scrollable", "true"},
    });
}

void SvmTest::testWallpaper()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    Wallpaper aWallpaper(COL_LIGHTGREEN);
    pVirtualDev->DrawWallpaper(tools::Rectangle(Point(1, 1), Size(3, 3)), aWallpaper);

    checkWallpaper(writeAndReadStream(aGDIMetaFile));
    checkWallpaper(readFile("wallpaper.svm"));
}

void SvmTest::checkClipRegion(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/clipregion[1]", {
        {"left", "2"},
        {"top", "2"},
        {"right", "5"},
        {"bottom", "5"},
    });
}

void SvmTest::testClipRegion()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    vcl::Region aRegion(tools::Rectangle(Point(2, 2), Size(4, 4)));

    // TODO
    // explicit Region(const tools::Polygon& rPolygon);
    // explicit Region(const tools::PolyPolygon& rPolyPoly);
    // explicit Region(const basegfx::B2DPolyPolygon&);
    pVirtualDev->SetClipRegion(aRegion);

    checkClipRegion(writeAndReadStream(aGDIMetaFile));
    checkClipRegion(readFile("clipregion.svm"));
}

void SvmTest::testIntersectRectClipRegion()
{}
void SvmTest::testIntersectRegionClipRegion()
{}
void SvmTest::testMoveClipRegion()
{}

void SvmTest::checkLineColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/push/linecolor[1]", {
        {"color", "#654321"},
    });
}

void SvmTest::testLineColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->Push();
    pVirtualDev->SetLineColor(Color(0x654321));
    pVirtualDev->Pop();

    checkLineColor(writeAndReadStream(aGDIMetaFile));
    checkLineColor(readFile("linecolor.svm"));
}

void SvmTest::checkFillColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/push/fillcolor[1]", {
        {"color", "#456789"},
    });
}

void SvmTest::testFillColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->Push();
    pVirtualDev->SetFillColor(Color(0x456789));
    pVirtualDev->Pop();

    checkFillColor(writeAndReadStream(aGDIMetaFile));
    checkFillColor(readFile("fillcolor.svm"));
}

void SvmTest::checkTextColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textcolor[1]", {
        {"color", "#123456"},
    });
}

void SvmTest::testTextColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetTextColor(Color(0x123456));

    checkTextColor(writeAndReadStream(aGDIMetaFile));
    checkTextColor(readFile("textcolor.svm"));
}

void SvmTest::checkTextFillColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textfillcolor[1]", {
        {"color", "#234567"},
    });
}

void SvmTest::testTextFillColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetTextFillColor(Color(0x234567));

    checkTextFillColor(writeAndReadStream(aGDIMetaFile));
    checkTextFillColor(readFile("textfillecolor.svm"));
}

void SvmTest::checkTextLineColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textlinecolor[1]", {
        {"color", "#345678"},
    });
}

void SvmTest::testTextLineColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetTextLineColor(Color(0x345678));

    checkTextLineColor(writeAndReadStream(aGDIMetaFile));
    checkTextLineColor(readFile("textlinecolor.svm"));
}

void SvmTest::checkOverLineColor(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/push/overlinecolor[1]", {
        {"color", "#345678"},
    });
}

void SvmTest::testOverLineColor()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->Push();
    pVirtualDev->SetOverlineColor(Color(0x345678));
    pVirtualDev->Pop();

    checkOverLineColor(writeAndReadStream(aGDIMetaFile));
    checkOverLineColor(readFile("overlinecolor.svm"));
}

void SvmTest::checkTextAlign(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/textalign[1]", {
        {"align", "bottom"},
    });
}

void SvmTest::testTextAlign()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetTextAlign(TextAlign::ALIGN_BOTTOM);

    checkTextAlign(writeAndReadStream(aGDIMetaFile));
    checkTextAlign(readFile("textalign.svm"));
}

void SvmTest::testMapMode()
{}
void SvmTest::testFont()
{}

void SvmTest::checkPushPop(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/push[1]", {{"flags", "PushAll"}});
    assertXPathAttrs(pDoc, "/metafile/push[1]/linecolor[1]", {{"color", "#800000"}});
    assertXPathAttrs(pDoc, "/metafile/push[1]/line[1]", {
        {"startx", "4"}, {"starty", "4"},
        {"endx", "6"},   {"endy", "6"},
    });
    assertXPathAttrs(pDoc, "/metafile/push[1]/push[1]", {{"flags", "PushLineColor, PushFillColor"}});
    assertXPathAttrs(pDoc, "/metafile/push[1]/push[1]/line[1]", {
        {"startx", "5"}, {"starty", "5"},
        {"endx", "7"},   {"endy", "7"},
    });
}

void SvmTest::testPushPop()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetLineColor(COL_YELLOW);
    pVirtualDev->Push();
    pVirtualDev->SetLineColor(COL_RED);
    pVirtualDev->DrawLine(Point(4,4), Point(6,6));
    pVirtualDev->Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
    pVirtualDev->SetLineColor(COL_LIGHTRED);
    pVirtualDev->DrawLine(Point(5,5), Point(7,7));
    pVirtualDev->Pop();
    pVirtualDev->Pop();
    pVirtualDev->DrawLine(Point(1,1), Point(8,8));

    checkPushPop(writeAndReadStream(aGDIMetaFile));
    checkPushPop(readFile("pushpop.svm"));
}

void SvmTest::checkRasterOp(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/rasterop[1]", {
        {"operation", "xor"},
    });
}

void SvmTest::testRasterOp()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    pVirtualDev->SetRasterOp(RasterOp::Xor);

    checkRasterOp(writeAndReadStream(aGDIMetaFile));
    checkRasterOp(readFile("rasterop.svm"));
}

void SvmTest::checkTransparent(const GDIMetaFile& rMetaFile)
{
    xmlDocUniquePtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/transparent[1]", {
        {"transparence", "50"},
    });

    assertXPathAttrs(pDoc, "/metafile/transparent[1]/polygon/point[1]", {
        {"x", "1"}, {"y", "8"},
    });
    assertXPathAttrs(pDoc, "/metafile/transparent[1]/polygon/point[2]", {
        {"x", "2"}, {"y", "7"},
    });
    assertXPathAttrs(pDoc, "/metafile/transparent[1]/polygon/point[3]", {
        {"x", "3"}, {"y", "6"},
    });
}

void SvmTest::testTransparent()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    tools::PolyPolygon aPolyPolygon(1);
    aPolyPolygon.Insert(aPolygon);

    pVirtualDev->DrawTransparent(aPolygon, 50);

    CPPUNIT_ASSERT(aGDIMetaFile.HasTransparentActions());
    checkTransparent(writeAndReadStream(aGDIMetaFile));
    checkTransparent(readFile("transparent.svm"));
}

void SvmTest::testFloatTransparent()
{}

void SvmTest::testEPS()
{}

void SvmTest::testRefPoint()
{}

void SvmTest::testComment()
{}

void SvmTest::testLayoutMode()
{}

void SvmTest::testTextLanguage()
{}

CPPUNIT_TEST_SUITE_REGISTRATION(SvmTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
