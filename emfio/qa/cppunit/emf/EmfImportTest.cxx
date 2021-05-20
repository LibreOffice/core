/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_features.h>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/graphic/EmfTools.hpp>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <memory>
#include <string_view>

namespace
{

using namespace css;
using namespace css::uno;
using namespace css::io;
using namespace css::graphic;
using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DContainer;

class Test : public test::BootstrapFixture, public XmlTestTools, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;

    void testPolyPolygon();
    void TestDrawString();
    void TestDrawStringTransparent();
    void TestDrawLine();
    void TestLinearGradient();
    void TestTextMapMode();
    void TestEnglishMapMode();
    void TestRectangleWithModifyWorldTransform();
    void TestArcStartPointEqualEndPoint();
    void TestArcInsideWronglyDefinedRectangle();
    void TestChordWithModifyWorldTransform();
    void TestEllipseWithSelectClipPath();
    void TestEllipseXformIntersectClipRect();
    void TestDrawPolyLine16WithClip();
    void TestFillRegion();
    void TestPalette();
    void TestPolylinetoCloseStroke();
    void TestPolyLineWidth();
    void TestRoundRect();
    void TestCreatePen();
    void TestPdfInEmf();

    Primitive2DSequence parseEmf(std::u16string_view aSource);

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testPolyPolygon);
    CPPUNIT_TEST(TestDrawString);
    CPPUNIT_TEST(TestDrawStringTransparent);
    CPPUNIT_TEST(TestDrawLine);
    CPPUNIT_TEST(TestLinearGradient);
    CPPUNIT_TEST(TestTextMapMode);
    CPPUNIT_TEST(TestEnglishMapMode);
    CPPUNIT_TEST(TestRectangleWithModifyWorldTransform);
    CPPUNIT_TEST(TestArcStartPointEqualEndPoint);
    CPPUNIT_TEST(TestArcInsideWronglyDefinedRectangle);
    CPPUNIT_TEST(TestChordWithModifyWorldTransform);
    CPPUNIT_TEST(TestEllipseWithSelectClipPath);
    CPPUNIT_TEST(TestEllipseXformIntersectClipRect);
    CPPUNIT_TEST(TestDrawPolyLine16WithClip);
    CPPUNIT_TEST(TestFillRegion);
    CPPUNIT_TEST(TestPalette);
    CPPUNIT_TEST(TestPolylinetoCloseStroke);
    CPPUNIT_TEST(TestPolyLineWidth);
    CPPUNIT_TEST(TestRoundRect);
    CPPUNIT_TEST(TestCreatePen);
    CPPUNIT_TEST(TestPdfInEmf);
    CPPUNIT_TEST_SUITE_END();
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

Primitive2DSequence Test::parseEmf(std::u16string_view aSource)
{
    const Reference<XEmfParser> xEmfParser = EmfTools::create(m_xContext);

    OUString aUrl  = m_directories.getURLFromSrc(aSource);
    OUString aPath = m_directories.getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));
    css::uno::Sequence< css::beans::PropertyValue > aEmptyValues;

    return xEmfParser->getDecomposition(aInputStream, aPath, aEmptyValues);
}

void Test::testPolyPolygon()
{
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/fdo79679-2.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Chart axis
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h19746v14817h-19746z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor", 2);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]", "color", "#ffffff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]/polypolygon", "path", "m0 0h19781v14852h-19781z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[2]/polypolygon", "path", "m2574 13194v-12065h15303v12065z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke", 116);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/polygon", "2574,13194 2574,1129 17877,1129 17877,13194");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/line", "color", "#ffffff");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[10]/polygon", "8674,13194 8674,1129");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[10]/line", "color", "#000000");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion", 28);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[6]", "width", "459");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[6]", "x", "9908");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[6]", "text", "0.5");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[6]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray", 98);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]/point", "x", "2574");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]/point", "y", "1129");

}

void Test::TestDrawString()
{
#if HAVE_MORE_FONTS
    // This unit checks for a correct import of an EMF+ file with only one DrawString Record
    // Since the text is undecorated the optimal choice is a simpletextportion primitive

    // first, get the sequence of primitives and dump it
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawString.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    // check correct import of the DrawString: height, position, text, color and font
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "height", "120");
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "x", "817");
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "y", "1137");
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "text", "TEST");
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/transform/textsimpleportion", "familyname", "CALIBRI");
#endif
}

void Test::TestDrawStringTransparent()
{
#if HAVE_MORE_FONTS
    // This unit checks for a correct import of an EMF+ file with one DrawString Record with transparency

    // first, get the sequence of primitives and dump it
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawStringTransparent.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence", "transparence", "0.498039215686275");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "height", "24");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "x", "66");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "y", "74");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "text", "Transparent Text");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence/textsimpleportion", "familyname", "ARIAL");
#endif
}

void Test::TestDrawLine()
{
    // This unit checks for a correct import of an EMF+ file with only one DrawLine Record
    // The line is colored and has a specified width, therefore a polypolygonstroke primitive is the optimal choice

    // first, get the sequence of primitives and dump it
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawLine.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    // check correct import of the DrawLine: color and width of the line
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke/line", "width", "33");
}

void Test::TestLinearGradient()
{
    // This unit checks for a correct import of an EMF+ file with LinearGradient brush
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestLinearGradient.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy11", "1.0000656512605");
    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy12", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy13", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy21", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy22", "1.00013140604468");
    assertXPath(pDocument, "/primitive2D/metafile/transform", "xy23", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "height", "7610");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "width", "15232");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h15232v7610h-15232z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]", "startx", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]", "starty", "-1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]", "endx", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]", "endy", "-1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]", "opacity", "0.392156862745098");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[1]/polypolygon", "path", "m0 0.216110019646294h7615.75822989746v7610.21611001965h-7615.75822989746z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]", "startx", "-1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]", "starty", "-1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]", "endx", "0");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]", "endy", "-1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]", "opacity", "1");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/svglineargradient[2]/polypolygon", "path", "m7615.75822989746 0.216110019646294h7615.75822989746v7610.21611001965h-7615.75822989746z");
}

void Test::TestTextMapMode()
{
    // Check import of EMF image with records: SETMAPMODE with MM_TEXT MapMode, POLYLINE16, EXTCREATEPEN, EXTTEXTOUTW
    // MM_TEXT is mapped to one device pixel. Positive x is to the right; positive y is down.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TextMapMode.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor", 2);
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]", "color", "#ffffff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]/polypolygon", "path", "m0 0h3542v4647h-3542z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion", 20);
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "text", "N");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "fontcolor", "#4a70e3");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "x", "2099");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "y", "1859");

    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke", 138);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/polygon", "2142,1638 2142,1489");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/line", "color", "#4a70e3");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/line", "width", "11");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[10]/polygon", "1967,1029 1869,952");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[10]/line", "color", "#4a70e3");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[10]/line", "width", "11");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[20]/polygon", "2710,1113 2873,1330");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[20]/line", "color", "#666666");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[20]/line", "width", "11");
}

void Test::TestEnglishMapMode()
{
    // Check import of EMF image with records: SETMAPMODE with MM_ENGLISH MapMode, STROKEANDFILLPATH, EXTTEXTOUTW, SETTEXTALIGN, STRETCHDIBITS
    // MM_LOENGLISH is mapped to 0.01 inch. Positive x is to the right; positive y is up.M
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/test_mm_hienglish_ref.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon[1]", "path", "m0 0h29699v20999h-29699z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]", "color", "#ffffad");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]/polypolygon", "path", "m-1-1h29700v21001h-29700z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[2]/polypolygon", "path", "m1058 7937v5293h3175v-1059h-2118v-4234z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[3]/polypolygon", "path", "m12699 1058h4234v1060h-1587v4231h-1059v-4231h-1588z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion", 4);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "text", "UL");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "x", "106");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "y", "459");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "width", "424");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "height", "424");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline", 3);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[1]/polygon", "-1,-1 29699,-1 29699,21000 -1,21000");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[2]/polygon", "1058,7937 1058,13230 4233,13230 4233,12171 2115,12171 2115,7937");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[3]/polygon", "12699,1058 16933,1058 16933,2118 15346,2118 15346,6349 14287,6349 14287,2118 12699,2118");
}

void Test::TestRectangleWithModifyWorldTransform()
{
    // Check import of EMF image with records: EXTCREATEPEN, SELECTOBJECT, MODIFYWORLDTRANSFORM, RECTANGLE

    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestRectangleWithModifyWorldTransform.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]", "color", "#ffffff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]/polypolygon", "path", "m1042 417 918 529 353 610-918-528z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke", 1);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/polygon", "1042,417 1960,946 2313,1556 1395,1028");
}

void Test::TestChordWithModifyWorldTransform()
{
    // EMF import test with records: CHORD, MODIFYWORLDTRANSFORM, EXTCREATEPEN, SELECTOBJECT
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestChordWithModifyWorldTransform.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor",
                "color", "#ffffff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor/polypolygon",
                "path", "m590 448-21 45-66 24-34 12-33 12-21 45-33 12 12 33-33 12 12 33 12 34 33-12 12 33 34-12 33-12 45 21 33-12 33-12 46 21 66-25 33-12 66-24 34-12 66-24z");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke/polygon",
                       "590,448 569,493 503,517 469,529 436,541 415,586 382,598 394,631 361,643 361,643 373,676 373,676 385,710 418,698 430,731 464,719 497,707 542,728 575,716 608,704 654,725 720,700 753,688 819,664 853,652 919,628");
}

void Test::TestArcStartPointEqualEndPoint()
{
    // i73608 EMF import test where StartPoint == EndPoint. It should draw full circle
    // Records: SETMAPMODE, SETWINDOWEXTEX, SETWINDOWORGEX, EXTSELECTCLIPRGN, INTERSECTCLIPRECT, MOVETOEX, ARC
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestArcStartPointEqualEndPoint.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polygonhairline",
                "color", "#000000");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polygonhairline/polygon",
                       "11886,23133 11970,23223 12051,23316 12131,23410 12208,23506 12282,23604 12354,23704 12424,23805 12491,23909 12556,24014 12618,24120 12677,24228 12734,24337 12788,24448 12839,24560 12888,24673 12933,24788 12976,24903 13016,25020 13053,25137 13087,25256 13119,25375 13147,25495 13172,25615 13195,25736 13214,25858 13230,25980 13244,26103 13254,26225 13261,26348 13266,26472 13267,26595 13265,26718 13260,26841 13253,26964 13242,27087 13228,27209 13211,27331 13191,27453 13168,27574 13142,27694 13113,27814 13082,27933 13047,28051 13009,28169 12969,28285 12926,28400 12879,28514 12830,28628 12779,28739 12724,28850 12667,28959 12607,29067 12545,29173 12480,29277 12412,29380 12342,29482 12269,29581 12194,29679 12117,29775 12037,29869 11955,29960 11871,30050 11784,30138 11696,30224 11605,30307 11512,30388 11418,30467 11321,30543 11223,30617 11122,30689 11020,30758 10917,30825 10811,30888 10705,30950 10596,31009 10487,31065 10376,31118 10263,31168 10150,31216 10035,31261 9919,31303 9803,31343 9685,31379 9566,31412 9447,31443 9327,31471 9206,31495 9085,31517 8963,31535 8841,31551 8719,31564 8596,31573 8473,31580 8350,31583 8226,31584 8103,31581 7980,31576 7857,31567 7735,31555 7612,31541 7491,31523 7369,31503 7248,31479 7128,31452 7008,31423 6890,31390 6772,31355 6655,31316 6538,31275 6423,31231 6310,31184 6197,31135 6085,31082 5975,31027 5866,30969 5759,30909 5653,30846 5549,30780 5447,30712 5346,30641 5247,30568 5150,30492 5054,30414 4961,30334 4870,30251 4780,30166 4693,30079 4608,29990 4525,29899 4445,29805 4367,29710 4291,29613 4217,29514 4146,29414 4078,29311 4012,29207 3949,29101 3888,28994 3830,28885 3775,28775 3722,28664 3672,28551 3625,28438 3581,28323 3540,28207 3501,28090 3465,27972 3433,27853 3403,27733 3376,27613 3352,27492 3331,27371 3313,27249 3299,27127 3287,27004 3278,26881 3272,26758 3269,26635 3270,26512 3273,26388 3279,26265 3289,26143 3301,26020 3316,25898 3335,25776 3356,25655 3380,25534 3408,25414 3438,25294 3471,25176 3508,25058 3547,24941 3588,24825 3633,24711 3681,24597 3731,24484 3784,24373 3840,24263 3899,24155 3960,24048 4023,23943 4090,23839 4159,23737 4230,23636 4304,23538 4380,23441 4459,23346 4540,23253 4623,23162 4708,23074 4796,22987 4885,22902 4977,22820 5071,22740 5166,22663 5264,22587 5363,22515 5465,22444 5567,22376 5672,22311 5778,22249 5885,22188 5994,22131 6105,22076 6216,22024 6329,21975 6443,21929 6559,21885 6675,21845 6792,21807 6910,21772 7029,21740 7149,21711 7269,21685 7390,21662 7512,21642 7634,21624 7756,21610 7879,21599 8002,21591 8125,21586 8248,21584 8371,21585 8494,21589 8617,21596 8740,21606 8862,21619 8985,21636 9106,21655 9227,21677 9348,21702 9468,21730 9587,21761 9705,21795 9823,21832 9940,21872 10055,21914 10170,21960 10283,22008 10395,22059 10506,22113 10615,22169 10723,22229 10830,22291 10935,22355 11038,22422 11140,22491 11240,22563 11338,22638 11434,22715 11529,22794 11621,22875 11711,22959 11800,23045");
}

void Test::TestArcInsideWronglyDefinedRectangle()
{
    // tdf#142268 EMF import test with records: ARC
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestArcInsideWronglyDefinedRectangle.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonhairline",
                "color", "#000000");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonhairline/polygon",
                       "1630,1460 1650,1470 1670,1480 1700,1490 1720,1500 1750,1510 1770,1520 1800,1530 1820,1530 1850,1540 1870,1540 1900,1540 1930,1540 1950,1540 1980,1540 2000,1530 2030,1530 2050,1520 2080,1510 2100,1500 2130,1490 2150,1480 2170,1470 2200,1450 2220,1440 2240,1420 2260,1400 2280,1390 2290,1370 2310,1350 2330,1330 2340,1300 2360,1280 2370,1260 2380,1240 2390,1210 2400,1190 2410,1160 2420,1140 2420,1110 2420,1080 2430,1060 2430,1030 2430,1000 2430,980 2430,950 2420,930 2420,900 2410,870 2410,850 2400,820 2390,800 2380,770 2360,750 2350,730 2340,710 2320,680 2300,660 2290,640 2270,630 2250,610 2230,590 2210,580 2190,560 2160,550 2140,540 2120,520 2090,510 2070,510 2040,500 2020,490 1990,490 1970,480 1940,480 1920,480 1890,480 1860,480 1840,490 1810,490 1790,500 1760,500 1740,510 1710,520 1690,530 1670,540 1640,560 1620,570 1600,580 1580,600 1560,620 1540,640 1520,660 1510,680 1490,700 1480,720 1460,740 1450,760");
}

void Test::TestEllipseWithSelectClipPath()
{
    // EMF import test with records: RECTANGLE, BEGINPATH, ENDPATH, ELLIPSE
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEllipseWithSelectClipPath.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygon", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygon[1]", "path", "m2790 776v-36-35h-36v-35l-35-35-35-36h-36l-35-35-35-35h-35-36l-35-35h-35-36l-35-36h-35-36l-35-35h-35-71-35l-36-35h-70-35-36-70l-36-35h-35-71-35-71-35-71-35-35-71-35-71-35-71-35l-35 35h-71-35-36-70l-35 35h-36-70-36l-35 35h-35-36l-35 36h-35-36l-35 35h-35-36l-35 35-35 35h-35l-36 36-35 35v35h-35v35 36 35 35h35v35l35 36 36 35h35l35 35 35 35h36 35l35 36h36 35l35 35h36 35l35 35h36 70 36l35 35h70 36 35 71l35 36h35 71 35 71 35 71 35 35 71 35 71 35 71 35l36-36h70 36 35 70l36-35h35 71 35l35-35h36 35l35-35h36 35l35-36h36 35l35-35 35-35h36l35-35 35-36v-35h36v-35z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygoncolor[1]", "color", "#ffff00");
    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygoncolor[1]/polypolygon[1]", "path", "m353 353h2472v1057h-2472z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polygonstroke", 1);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/group/mask/polygonstroke[1]/polygon", "353,353 2825,353 2825,1410 353,1410");
}

void Test::TestEllipseXformIntersectClipRect()
{
    // EMF import test with records: EXTCREATEPEN, CREATEBRUSHINDIRECT, MODIFYWORLDTRANSFORM, INTERSECTCLIPRECT, ELLIPSE
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEllipseXformIntersectClipRect.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h3000v2000h-3000z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polypolygon", "path", "m370 152 1128-409 592 1623-1128 410z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polypolygoncolor/polypolygon", "path", "m3613 287-12-33-12-33-12-33-12-33-33 12-12-34-13-33-45-21-12-33-33 12-12-33-45-21-46-21-12-33-33 12-12-33-45-21-33 12-46-21-45-21-33 12-45-21-34 12-45-21-33 12-45-21-34 12-45-21-33 12-45-21-33 12-34 12-45-21-33 12-33 12-45-21-34 12-66 24-45-21-33 12-34 12-66 24-33 12-45-21-34 12-66 24-33 12-33 12-34 12-66 24-33 12-33 12-67 25-33 12-33 12-33 12-67 24-33 12-21 45-33 12-66 24-34 12-33 12-21 46-66 24-33 12-22 45-33 12-33 12-21 45-33 12-33 12-21 46-34 12-21 45-33 12-21 45-33 12-21 45-34 12-21 45-33 13-21 45-21 45-33 12-21 45 12 33-33 12 12 33-21 46-22 45 13 33-34 12 12 33-21 45 12 33 12 34-33 12 12 33 12 33 13 33 12 33 12 33 12 33 12 33 12 34 33-12 12 33 12 33 46 21 12 33 33-12 12 33 45 21 45 21 12 33 34-12 12 33 45 21 33-12 45 21 46 22 33-13 45 22 33-13 46 22 33-13 45 22 33-13 45 22 34-13 45 22 33-12 33-13 46 22 33-13 33-12 45 21 33-12 67-24 45 21 33-12 33-12 67-24 33-12 45 21 33-12 67-24 33-12 33-12 33-12 67-24 33-12 33-12 66-25 34-12 33-12 33-12 66-24 33-12 22-45 33-12 66-24 33-12 33-12 22-45 66-25 33-12 21-45 33-12 34-12 21-45 33-12 33-12 21-45 33-12 21-46 34-12 21-45 33-12 21-45 33-12 21-45 33-12 22-46 21-45 33-12 21-45-12-33 33-12-12-33 21-46 21-45-12-33 33-12-12-33 21-45-12-33-12-33 33-12-12-34-12-33-12-33z");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polygonstroke/polygon", "3613,287 3601,254 3601,254 3589,221 3577,188 3565,155 3532,167 3520,133 3507,100 3507,100 3462,79 3450,46 3417,58 3405,25 3360,4 3360,4 3314,-17 3302,-50 3269,-38 3257,-71 3212,-92 3179,-80 3133,-101 3133,-101 3088,-122 3055,-110 3010,-131 2976,-119 2931,-140 2898,-128 2853,-149 2819,-137 2774,-158 2741,-146 2696,-167 2663,-155 2629,-143 2584,-164 2551,-152 2518,-140 2473,-161 2439,-149 2373,-125 2328,-146 2295,-134 2261,-122 2195,-98 2162,-86 2117,-107 2083,-95 2017,-71 1984,-59 1951,-47 1917,-35 1851,-11 1818,1 1818,1 1785,13 1718,38 1685,50 1652,62 1619,74 1552,98 1519,110 1498,155 1465,167 1399,191 1365,203 1332,215 1311,261 1245,285 1212,297 1190,342 1157,354 1124,366 1103,411 1070,423 1037,435 1016,481 982,493 961,538 928,550 907,595 874,607 853,652 819,664 798,709 765,722 744,767 744,767 723,812 690,824 669,869 681,902 648,914 660,947 639,993 639,993 617,1038 630,1071 596,1083 608,1116 587,1161 587,1161 599,1194 611,1228 578,1240 590,1273 602,1306 615,1339 615,1339 627,1372 627,1372 639,1405 639,1405 651,1438 663,1471 675,1505 708,1493 720,1526 732,1559 732,1559 778,1580 790,1613 823,1601 835,1634 880,1655 880,1655 925,1676 937,1709 971,1697 983,1730 1028,1751 1061,1739 1106,1760 1106,1760 1152,1782 1185,1769 1230,1791 1263,1778 1309,1800 1342,1787 1387,1809 1420,1796 1465,1818 1499,1805 1544,1827 1577,1815 1610,1802 1656,1824 1689,1811 1722,1799 1767,1820 1800,1808 1867,1784 1912,1805 1945,1793 1978,1781 2045,1757 2078,1745 2123,1766 2156,1754 2223,1730 2256,1718 2289,1706 2322,1694 2389,1670 2422,1658 2422,1658 2455,1646 2521,1621 2555,1609 2588,1597 2621,1585 2687,1561 2720,1549 2742,1504 2775,1492 2841,1468 2874,1456 2907,1444 2929,1399 2995,1374 3028,1362 3049,1317 3082,1305 3116,1293 3137,1248 3170,1236 3203,1224 3224,1179 3257,1167 3278,1121 3312,1109 3333,1064 3366,1052 3387,1007 3420,995 3441,950 3474,938 3496,892 3496,892 3517,847 3550,835 3571,790 3559,757 3592,745 3580,712 3601,666 3601,666 3622,621 3610,588 3643,576 3631,543 3652,498 3652,498 3640,465 3628,432 3661,420 3649,386 3637,353 3625,320 3625,320");
}

void Test::TestDrawPolyLine16WithClip()
{
    // Check import of EMF image with records:
    // CREATEBRUSHINDIRECT, FILLRGN, BEGINPATH, POLYGON16, SELECTCLIPPATH, MODIFYWORLDTRANSFORM, SELECTOBJECT
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawPolyLine16WithClip.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h3943v3939h-3943z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]/polypolygon",
                "path", "m1323 0h1323v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]", "color", "#b4ffff");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline", 1);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[1]/polygon",
                       "1323,0 2646,0 2646,1322 3969,1322 3969,2644 2646,2644 2646,3966 1323,3966 1323,2644 0,2644 0,1322 1323,1322");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[1]", "color", "#000000");


    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/group[1]/mask/polypolygon", "path", "m2646 0v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323v-1322");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/group[1]/mask/polygonstroke/polygon", "0,793 3969,4230");
}

void Test::TestFillRegion()
{
    // EMF import with records: CREATEBRUSHINDIRECT, FILLRGN. The SETICMMODE is also used.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestFillRegion.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h3943v3939h-3943z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]/polypolygon",
                "path", "m1323 0h1323v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]", "color", "#ff0000");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline", 1);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[1]/polygon",
                       "1323,0 2646,0 2646,1322 3969,1322 3969,2644 2646,2644 2646,3966 1323,3966 1323,2644 0,2644 0,1322 1323,1322");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[1]", "color", "#000000");
}

void Test::TestPolylinetoCloseStroke()
{
    // EMF import with records: BEGINPATH, ARC, ENDPATH, STROKEPATH, EXTCREATEPEN.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestPolylinetoCloseStroke.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonhairline", 2);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonhairline[1]/polygon",
                       "1080,150 1010,170 940,190 870,210 810,230 750,260 690,280 630,310 570,340 520,380 470,410 420,450 370,490 330,530 290,570 260,610 230,660 200,700 170,750 150,790 130,840 120,890 110,930 100,980 100,1030 100,1080 110,1130 120,1180 130,1220 140,1270 160,1320 190,1360 210,1410 250,1450 280,1490 320,1540 360,1580 400,1620 450,1650 500,1690");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonhairline[1]",
                "color", "#000000");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonhairline[2]/polygon",
                       "1760,1120 1710,1130 1670,1140 1620,1150 1580,1160 1540,1170 1500,1180 1460,1200 1420,1210 1380,1230 1350,1240 1320,1260 1290,1280 1260,1300 1230,1310 1210,1330 1190,1360 1170,1380 1150,1400 1140,1420 1120,1440 1110,1460 1110,1490 1100,1510 1100,1530 1100,1550 1100,1580 1110,1600 1120,1620 1130,1650 1140,1670 1160,1690 1170,1710 1190,1730");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonhairline[2]",
                "color", "#000000");
}


void Test::TestPalette()
{
    // WMF import with records: CREATEPALETTE, SELECTOBJECT, CREATEPENINDIRECT, CREATEBRUSHINDIRECT, ELLIPSE.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestPalette.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor", 2);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]/polypolygon",
                "path", "m2433 1216.5c0 213.540351086756-56.2099207528522 423.318631225821-162.98009629623 608.25s-260.338534929591 338.499728160392-445.269903703769 445.26990370377-394.709648913244 162.98009629623-608.25 162.98009629623-423.318631225821-56.2099207528518-608.25-162.98009629623-338.499728160392-260.338534929591-445.26990370377-445.26990370377-162.980096296231-394.709648913244-162.980096296231-608.25c-2.70117261891301E-013-213.540351086756 56.2099207528519-423.318631225821 162.98009629623-608.25 106.770175543378-184.931368774179 260.338534929591-338.499728160392 445.269903703769-445.26990370377s394.709648913244-162.980096296231 608.25-162.980096296231c213.540351086756-5.40234523782601E-013 423.318631225821 56.2099207528518 608.25 162.98009629623s338.499728160392 260.338534929591 445.26990370377 445.269903703769c106.770175543379 184.931368774179 162.980096296231 394.709648913244 162.980096296231 608.25z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[1]",
                "color", "#ffff00");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[2]/polypolygon",
                "path", "m5079 3862.5c0 213.540351086756-56.2099207528527 423.318631225821-162.980096296231 608.25s-260.33853492959 338.499728160391-445.269903703769 445.269903703769-394.709648913244 162.980096296231-608.25 162.980096296231-423.318631225821-56.2099207528518-608.25-162.98009629623-338.499728160391-260.33853492959-445.26990370377-445.269903703769-162.980096296231-394.709648913245-162.980096296231-608.250000000001c-4.54747350886464E-013-213.540351086756 56.2099207528518-423.318631225821 162.98009629623-608.25 106.770175543378-184.931368774179 260.33853492959-338.499728160392 445.269903703769-445.26990370377s394.709648913244-162.980096296231 608.25-162.980096296231c213.540351086756-4.54747350886464E-013 423.318631225821 56.2099207528522 608.25 162.98009629623s338.499728160392 260.33853492959 445.26990370377 445.269903703769c106.770175543378 184.931368774178 162.980096296231 394.709648913244 162.980096296231 608.25z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygoncolor[2]",
                "color", "#0080ff");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke", 2);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/polygon",
                       "2408,1191 2406,1129 2402,1068 2394,1007 2383,946 2369,886 2352,827 2332,768 2309,711 2284,655 2255,600 2224,547 2190,496 2153,446 2114,398 2073,353 2029,309 1984,268 1936,229 1886,192 1835,158 1782,127 1727,98 1671,73 1614,50 1555,30 1496,13 1436,-1 1375,-12 1314,-20 1253,-24 1191,-26 1191,-26 1129,-24 1068,-20 1007,-12 946,-1 886,13 827,30 768,50 711,73 655,98 600,127 547,158 496,192 446,229 398,268 353,309 309,353 268,398 229,446 192,496 158,547 127,600 98,655 73,711 50,768 30,827 13,886 -1,946 -12,1007 -20,1068 -24,1129 -26,1191 -26,1191 -24,1253 -20,1314 -12,1375 -1,1436 13,1496 30,1555 50,1614 73,1671 98,1727 127,1782 158,1835 192,1886 229,1936 268,1984 309,2029 353,2073 398,2114 446,2153 496,2190 547,2224 600,2255 655,2284 711,2309 768,2332 827,2352 886,2369 946,2383 1007,2394 1068,2402 1129,2406 1191,2408 1191,2408 1253,2406 1314,2402 1375,2394 1436,2383 1496,2369 1555,2352 1614,2332 1671,2309 1727,2284 1782,2255 1835,2224 1886,2190 1936,2153 1984,2114 2029,2073 2073,2029 2114,1984 2153,1936 2190,1886 2224,1835 2255,1782 2284,1727 2309,1671 2332,1614 2352,1555 2369,1496 2383,1436 2394,1375 2402,1314 2406,1253");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/line",
                "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/line",
                "width", "132");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/polygon",
                       "5053,3836 5051,3774 5047,3713 5039,3652 5028,3591 5014,3531 4997,3472 4977,3413 4954,3356 4929,3300 4900,3245 4869,3192 4835,3141 4798,3091 4759,3043 4718,2998 4674,2954 4629,2913 4581,2874 4531,2837 4480,2803 4427,2772 4372,2743 4316,2718 4259,2695 4200,2675 4141,2658 4081,2644 4020,2633 3959,2625 3898,2621 3836,2619 3836,2619 3774,2621 3713,2625 3652,2633 3591,2644 3531,2658 3472,2675 3413,2695 3356,2718 3300,2743 3245,2772 3192,2803 3141,2837 3091,2874 3043,2913 2998,2954 2954,2998 2913,3043 2874,3091 2837,3141 2803,3192 2772,3245 2743,3300 2718,3356 2695,3413 2675,3472 2658,3531 2644,3591 2633,3652 2625,3713 2621,3774 2619,3836 2619,3836 2621,3898 2625,3959 2633,4020 2644,4081 2658,4141 2675,4200 2695,4259 2718,4316 2743,4372 2772,4427 2803,4480 2837,4531 2874,4581 2913,4629 2954,4674 2998,4718 3043,4759 3091,4798 3141,4835 3192,4869 3245,4900 3300,4929 3356,4954 3413,4977 3472,4997 3531,5014 3591,5028 3652,5039 3713,5047 3774,5051 3836,5053 3836,5053 3898,5051 3959,5047 4020,5039 4081,5028 4141,5014 4200,4997 4259,4977 4316,4954 4372,4929 4427,4900 4480,4869 4531,4835 4581,4798 4629,4759 4674,4718 4718,4674 4759,4629 4798,4581 4835,4531 4869,4480 4900,4427 4929,4372 4954,4316 4977,4259 4997,4200 5014,4141 5028,4081 5039,4020 5047,3959 5051,3898");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/line",
                "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/line",
                "width", "132");
}

void Test::TestPolyLineWidth()
{
    // EMF import with records: CREATEPEN, ROUNDRECT.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestPolyLineWidth.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor/polypolygon",
                "path", "m530 529 1236-176-707 352z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor",
                "color", "#ffff00");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke/polygon",
                       "530,529 530,529 1766,353 1059,705");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke/line",
                "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke/line",
                "width", "71");
}

void Test::TestRoundRect()
{
    // EMF import with records: CREATEPEN, ROUNDRECT.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestRoundRect.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor", 2);
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]/polypolygon",
                "path", "m100 100h4000v2000h-4000z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[1]",
                "color", "#ffffff");

    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke", 2);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/polygon",
                       "100,100 4100,100 4100,2100 100,2100");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[1]/line",
                "color", "#ff0000");

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[2]/polypolygon",
                "path", "m4090 2700v-30-20l-10-30v-20l-10-30-10-20-10-20-20-30-10-20-20-20-20-20-20-30-20-20-20-20-20-10-30-20-20-20-30-20-30-10-30-10-30-20-30-10-30-10-40-10-30-10h-30l-40-10h-30l-40-10h-30-40-2590-40-30l-40 10h-30l-40 10h-30l-30 10-40 10-30 10-30 10-30 20-30 10-30 10-30 20-20 20-30 20-20 10-20 20-20 20-20 30-20 20-20 20-10 20-20 30-10 20-10 20-10 30v20l-10 30v20 30 990 30 20l10 30v20l10 30 10 20 10 20 20 30 10 20 20 20 20 20 20 30 20 20 20 20 20 10 30 20 20 20 30 20 30 10 30 10 30 20 30 10 30 10 40 10 30 10h30l40 10h30l40 10h30 40 2590 40 30l40-10h30l40-10h30l30-10 40-10 30-10 30-10 30-20 30-10 30-10 30-20 20-20 30-20 20-10 20-20 20-20 20-30 20-20 20-20 10-20 20-30 10-20 10-20 10-30v-20l10-30v-20-30z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[2]",
                "color", "#ffffff");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[2]/polygon",
                       "4090,2700 4090,2670 4090,2650 4080,2620 4080,2600 4070,2570 4060,2550 4050,2530 4030,2500 4020,2480 4000,2460 3980,2440 3960,2410 3940,2390 3920,2370 3900,2360 3870,2340 3850,2320 3820,2300 3790,2290 3760,2280 3730,2260 3700,2250 3670,2240 3630,2230 3600,2220 3570,2220 3530,2210 3500,2210 3460,2200 3430,2200 3390,2200 800,2200 760,2200 730,2200 690,2210 660,2210 620,2220 590,2220 560,2230 520,2240 490,2250 460,2260 430,2280 400,2290 370,2300 340,2320 320,2340 290,2360 270,2370 250,2390 230,2410 210,2440 190,2460 170,2480 160,2500 140,2530 130,2550 120,2570 110,2600 110,2620 100,2650 100,2670 100,2700 100,3690 100,3720 100,3740 110,3770 110,3790 120,3820 130,3840 140,3860 160,3890 170,3910 190,3930 210,3950 230,3980 250,4000 270,4020 290,4030 320,4050 340,4070 370,4090 400,4100 430,4110 460,4130 490,4140 520,4150 560,4160 590,4170 620,4170 660,4180 690,4180 730,4190 760,4190 800,4190 3390,4190 3430,4190 3460,4190 3500,4180 3530,4180 3570,4170 3600,4170 3630,4160 3670,4150 3700,4140 3730,4130 3760,4110 3790,4100 3820,4090 3850,4070 3870,4050 3900,4030 3920,4020 3940,4000 3960,3980 3980,3950 4000,3930 4020,3910 4030,3890 4050,3860 4060,3840 4070,3820 4080,3790 4080,3770 4090,3740 4090,3720 4090,3690");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polygonstroke[2]/line",
                "color", "#ff0000");
}

void Test::TestCreatePen()
{
    // Check import of EMF image with records: RESTOREDC, SAVEDC, MOVETOEX, LINETO, POLYLINE16, EXTTEXTOUTW with DxBuffer
    // The CREATEPEN record is used with PS_COSMETIC line style, which sometimes will be displayed as solid hairline
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestCreatePen.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence));
    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polypolygon", "path", "m0 0h31250v18192h-31250z");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke", 748);
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/polygon", "27875,16523 27875,1453");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/line", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[1]/line", "width", "6");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/polygon", "27975,16453 27875,16453");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/line", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[2]/line", "width", "6");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[3]/polygon", "27925,16078 27875,16078");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[3]/line", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonstroke[3]/line", "width", "6");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline", 10);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[5]", "color", "#008000");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[5]/polygon", "25850,2179 25844,1958");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[10]", "color", "#000080");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/polygonhairline[10]/polygon", "2025,1642 2025,1501");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion", 69);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "width", "374");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "x", "28124");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "y", "16581");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "text", "0.0");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[1]", "fontcolor", "#000000");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[10]", "width", "266");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[10]", "x", "28000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[10]", "y", "428");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[10]", "text", "-6");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/textsimpleportion[10]", "fontcolor", "#000000");

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray", 8);
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]", "color", "#008000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]/point", "x", "25844");
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/pointarray[1]/point", "y", "8918");
}

void Test::TestPdfInEmf()
{
    if (!vcl::pdf::PDFiumLibrary::get())
    {
        return;
    }

    // Load a PPTX file, which has a shape, with a bitmap fill, which is an EMF, containing a PDF.
    OUString aURL = m_directories.getURLFromSrc(u"emfio/qa/cppunit/emf/data/pdf-in-emf.pptx");
    getComponent() = loadFromDesktop(aURL);

    // Get the EMF.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("FillBitmap") >>= xGraphic;
    Graphic aGraphic(xGraphic);

    // Check the size hint of the EMF, which influences the bitmap generated from the PDF.
    const std::shared_ptr<VectorGraphicData>& pVectorGraphicData = aGraphic.getVectorGraphicData();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14321
    // - Actual  : 0
    // i.e. there was no size hint, the shape with 14cm height had a bitmap-from-PDF fill, the PDF
    // height was only 5cm, so it looked blurry.
    CPPUNIT_ASSERT_EQUAL(14321.0, pVectorGraphicData->getSizeHint().getY());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 255
    // i.e. the pixel in the center was entirely opaque, while it should be transparent.
    BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
    Size size = aBitmapEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0), aBitmapEx.GetAlpha(size.Width() / 2, size.Height() / 2));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
