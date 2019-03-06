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
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapaccess.hxx>
#include <bitmapwriteaccess.hxx>

using namespace css;

class SvmTest : public test::BootstrapFixture, public XmlTestTools
{
    /*OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }*/

    xmlDocPtr dumpMeta(const GDIMetaFile& rMetaFile);

    void checkVirtualDevice(xmlDocPtr pDoc);
    void checkErase(xmlDocPtr pDoc);

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

    void checkStrechText(const GDIMetaFile& rMetaFile);
    void testStrechText();

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

    void checkPushPop(const GDIMetaFile& rMetaFile);
    void testPushPop();

    void checkTextColor(const GDIMetaFile& rMetaFile);
    void testTextColor();

    void checkTextFillColor(const GDIMetaFile& rMetaFile);
    void testTextFillColor();

    void checkTextLineColor(const GDIMetaFile& rMetaFile);
    void testTextLineColor();

    void checkGradient(const GDIMetaFile& rMetaFile);
    void testGradient();

public:
    SvmTest()
        : BootstrapFixture(true, false)
//      , maDataUrl("/vcl/qa/cppunit/svm/data/")
    {}

    CPPUNIT_TEST_SUITE(SvmTest);
    CPPUNIT_TEST(testPoint);
    CPPUNIT_TEST(testPixel);
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
    CPPUNIT_TEST(testStrechText);
    CPPUNIT_TEST(testTextRect);
    CPPUNIT_TEST(testTextLine);
    CPPUNIT_TEST(testBitmaps); // BMP, BMPSCALE, BMPSCALEPART
    CPPUNIT_TEST(testBitmapExs);
    CPPUNIT_TEST(testMasks);
    CPPUNIT_TEST(testPushPop);
    CPPUNIT_TEST(testTextColor);
    CPPUNIT_TEST(testTextFillColor);
    CPPUNIT_TEST(testTextLineColor);
    CPPUNIT_TEST(testGradient);
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

static GDIMetaFile readMetafile(const OUString& rUrl)
{
    SvFileStream aFileStream(rUrl, StreamMode::READ);
    GDIMetaFile aResultMetafile;
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

static GDIMetaFile writeAndRead(GDIMetaFile& rMetaFile, const OUString& sUrl)
{
    // Turn on to output the SVM bitstreams to files (using the input URL)
    // to inspect the content or to create a reference file, otherwise leave
    // disabled for normal test runs.
    static const bool bOutputToFile = false;

    if (bOutputToFile)
    {
        writeMetaFile(rMetaFile, sUrl);
        return readMetafile(sUrl);
    }
    else
    {
        SvMemoryStream aStream;
        rMetaFile.Write(aStream);
        aStream.Seek(STREAM_SEEK_TO_BEGIN);

        GDIMetaFile aResultMetafile;
        aResultMetafile.Read(aStream);
        return aResultMetafile;
    }
}

xmlDocPtr SvmTest::dumpMeta(const GDIMetaFile& rMetaFile)
{
    MetafileXmlDump dumper;
    xmlDocPtr pDoc = dumpAndParse(dumper, rMetaFile);
    CPPUNIT_ASSERT (pDoc);

    checkVirtualDevice(pDoc);
    checkErase(pDoc);

    return pDoc;
}

void SvmTest::checkVirtualDevice(xmlDocPtr pDoc)
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

void SvmTest::checkErase(xmlDocPtr pDoc)
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
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPixel(writeAndRead(aGDIMetaFile, "pixel.svm"));
}

void SvmTest::checkPoint(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPoint(writeAndRead(aGDIMetaFile, "point.svm"));
}

void SvmTest::checkLine(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkLine(writeAndRead(aGDIMetaFile, "line.svm"));
}

void SvmTest::checkRect(const GDIMetaFile& rMetaFile)
{

    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkRect(writeAndRead(aGDIMetaFile, "rect.svm"));
}

void SvmTest::checkRoundRect(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkRoundRect(writeAndRead(aGDIMetaFile, "roundrect.svm"));
}

void SvmTest::checkEllipse(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkEllipse(writeAndRead(aGDIMetaFile, "ellipse.svm"));
}

void SvmTest::checkArc(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkArc(writeAndRead(aGDIMetaFile, "arc.svm"));
}

void SvmTest::checkPie(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPie(writeAndRead(aGDIMetaFile, "pie.svm"));
}

void SvmTest::checkChord(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkChord(writeAndRead(aGDIMetaFile, "chord.svm"));
}

void SvmTest::checkPolyLine(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPolyLine(writeAndRead(aGDIMetaFile, "polyline.svm"));
}

void SvmTest::checkPolygon(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPolygon(writeAndRead(aGDIMetaFile, "polygon.svm"));
}

void SvmTest::checkPolyPolygon(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPolyPolygon(writeAndRead(aGDIMetaFile, "polypolygon.svm"));
}

void SvmTest::checkText(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkText(writeAndRead(aGDIMetaFile, "text.svm"));
}

void SvmTest::checkTextArray(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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
    long const aDX[] = { 10, 15, 20, 25, 30, 35 };
    pVirtualDev->DrawTextArray(Point(4,6), "123456", aDX, 1, 4);

    checkTextArray(writeAndRead(aGDIMetaFile, "textarray.svm"));
}

void SvmTest::checkStrechText(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

    assertXPathAttrs(pDoc, "/metafile/stretchtext[1]", {
        {"x", "4"}, {"y", "6"}, {"index", "1"}, {"length", "4"}, {"width", "10"}
    });

    assertXPathContent(pDoc, "/metafile/stretchtext[1]/textcontent", "123456");
}

void SvmTest::testStrechText()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);
    pVirtualDev->DrawStretchText(Point(4,6), 10, "123456", 1, 4);

    checkStrechText(writeAndRead(aGDIMetaFile, "strecthtext.svm"));
}

void SvmTest::checkTextRect(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkTextRect(writeAndRead(aGDIMetaFile, "textrectangle.svm"));
}

void SvmTest::checkTextLine(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkTextLine(writeAndRead(aGDIMetaFile, "textline.svm"));
}

void SvmTest::checkBitmaps(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

#ifdef LINUX
    assertXPathAttrs(pDoc, "/metafile/bmp[1]", {{"x", "1"}, {"y", "2"}, {"crc", "b8dee5da"}});
    assertXPathAttrs(pDoc, "/metafile/bmpscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"}, {"crc", "281fc589"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"crc", "5e01ddcc"}
    });
#else
    assertXPathAttrs(pDoc, "/metafile/bmp[1]", {{"x", "1"}, {"y", "2"}, {"crc", "b8dee5da"}});
    assertXPathAttrs(pDoc, "/metafile/bmpscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"}, {"crc", "281fc589"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"crc", "5e01ddcc"}
    });
#endif
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

    checkBitmaps(writeAndRead(aGDIMetaFile, "bitmaps.svm"));
}

void SvmTest::checkBitmapExs(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

#ifdef LINUX
    assertXPathAttrs(pDoc, "/metafile/bmpex[1]", {
        {"x", "1"}, {"y", "2"}, {"crc", "b8dee5da"}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"},
        {"crc", "281fc589"}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"crc", "5e01ddcc"}, {"transparenttype", "bitmap"}
    });
#else
    assertXPathAttrs(pDoc, "/metafile/bmpex[1]", {
        {"x", "1"}, {"y", "2"}, {"crc", "b8dee5da"}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscale[1]", {
        {"x", "1"}, {"y", "2"}, {"width", "3"}, {"height", "4"},
        {"crc", "281fc589"}, {"transparenttype", "bitmap"}
    });
    assertXPathAttrs(pDoc, "/metafile/bmpexscalepart[1]", {
        {"destx", "1"}, {"desty", "2"}, {"destwidth", "3"}, {"destheight", "4"},
        {"srcx", "2"},  {"srcy", "1"},  {"srcwidth", "4"},  {"srcheight", "3"},
        {"crc", "5e01ddcc"}, {"transparenttype", "bitmap"}
    });
#endif
}

void SvmTest::testBitmapExs()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev, aGDIMetaFile);

    Bitmap aBitmap1(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap1);
        pAccess->Erase(COL_RED);
    }
    BitmapEx aBitmapEx1(aBitmap1, COL_YELLOW);

    Bitmap aBitmap2(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap2);
        pAccess->Erase(COL_GREEN);
    }
    BitmapEx aBitmapEx2(aBitmap2, COL_YELLOW);

    Bitmap aBitmap3(Size(4,4), 24);
    {
        BitmapScopedWriteAccess pAccess(aBitmap3);
        pAccess->Erase(COL_BLUE);
    }
    BitmapEx aBitmapEx3(aBitmap3, COL_YELLOW);

    pVirtualDev->DrawBitmapEx(Point(1, 2), aBitmapEx1);
    pVirtualDev->DrawBitmapEx(Point(1, 2), Size(3, 4), aBitmapEx2);
    pVirtualDev->DrawBitmapEx(Point(1, 2), Size(3, 4), Point(2, 1), Size(4, 3), aBitmapEx3);

    checkBitmapExs(writeAndRead(aGDIMetaFile, "bitmapexs.svm"));
}

void SvmTest::checkMasks(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkMasks(writeAndRead(aGDIMetaFile, "masks.svm"));
}

void SvmTest::checkPushPop(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkPushPop(writeAndRead(aGDIMetaFile, "pushpop.svm"));
}

void SvmTest::checkTextColor(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkTextColor(writeAndRead(aGDIMetaFile, "textcolor.svm"));
}

void SvmTest::checkTextFillColor(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkTextFillColor(writeAndRead(aGDIMetaFile, "textfillecolor.svm"));
}

void SvmTest::checkTextLineColor(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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

    checkTextLineColor(writeAndRead(aGDIMetaFile, "textlinecolor.svm"));
}

void SvmTest::checkGradient(const GDIMetaFile& rMetaFile)
{
    xmlDocPtr pDoc = dumpMeta(rMetaFile);

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
}

void SvmTest::testGradient()
{
    GDIMetaFile aGDIMetaFile;
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    setupBaseVirtualDevice(*pVirtualDev.get(), aGDIMetaFile);

    tools::Rectangle aRectangle(Point(1, 2), Size(4,5));

    Gradient aGradient(GradientStyle::Linear, COL_WHITE, COL_BLACK);
    pVirtualDev->DrawGradient(aRectangle, aGradient);

    checkGradient(writeAndRead(aGDIMetaFile, "gradient.svm"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SvmTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
