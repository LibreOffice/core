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
    void TestChordWithModifyWorldTransform();
    void TestEllipseWithSelectClipPath();
    void TestEllipseXformIntersectClipRect();
    void TestDrawPolyLine16WithClip();
    void TestFillRegion();
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
    CPPUNIT_TEST(TestChordWithModifyWorldTransform);
    CPPUNIT_TEST(TestEllipseWithSelectClipPath);
    CPPUNIT_TEST(TestEllipseXformIntersectClipRect);
    CPPUNIT_TEST(TestDrawPolyLine16WithClip);
    CPPUNIT_TEST(TestFillRegion);
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

    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor", "color", "#ffffff");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor/polypolygon", "path", "m590 448-154 93-54 57-21 45 24 67 45 21 224-6 265-97z");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke/polygon", "590,448 436,541 382,598 361,643 385,710 430,731 654,725 919,628");
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
    assertXPath(pDocument, "/primitive2D/metafile/transform/group/mask/polypolygon[1]", "path", "m2790 705-106-106-106-70-318-106-424-70h-706l-35 35-247 35h-142l-318 106-105 70-106 106v141l106 106 105 70 318 106 424 71h706l424-71 318-106 106-70 106-106");

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
    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polypolygoncolor/polypolygon", "path", "m3565 155-58-55-205-150-90-42-124-30-392-45-368 21-211 39-598 217-187 105-21 46-121 81-66 24-87 69-272 287-75 102-42 90-61 247-9 79 97 265 57 54 205 150 91 42 124 31 392 45 112-3 467-58 597-217 187-105 296-220 271-286 76-103 42-90 60-247 9-78z");
    assertXPathContent(pDocument, "/primitive2D/metafile/transform/mask/group/mask/polygonstroke/polygon", "3565,155 3507,100 3302,-50 3212,-92 3088,-122 2696,-167 2328,-146 2117,-107 1519,110 1332,215 1311,261 1190,342 1124,366 1037,435 765,722 690,824 648,914 587,1161 578,1240 675,1505 732,1559 937,1709 1028,1751 1152,1782 1544,1827 1656,1824 2123,1766 2720,1549 2907,1444 3203,1224 3474,938 3550,835 3592,745 3652,498 3661,420");
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
                "path", "m4090 2650-10-50-10-30-50-90-100-110-20-10-30-20-20-20-30-20-90-40-100-30-170-30h-2730l-140 20-130 40-90 40-30 20-20 20-30 20-20 10-100 110-50 90-10 30-10 50v1090l10 50 10 30 50 90 100 110 20 10 30 20 20 20 30 20 90 40 100 30 170 30h2730l140-20 130-40 90-40 30-20 20-20 30-20 20-10 100-110 50-90 10-30 10-50z");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygoncolor[2]",
                "color", "#ffffff");

    assertXPathContent(pDocument, "/primitive2D/metafile/transform/polygonstroke[2]/polygon",
                       "4090,2650 4080,2600 4070,2570 4020,2480 3920,2370 3900,2360 3870,2340 3850,2320 3820,2300 3730,2260 3630,2230 3460,2200 730,2200 590,2220 460,2260 370,2300 340,2320 320,2340 290,2360 270,2370 170,2480 120,2570 110,2600 100,2650 100,3740 110,3790 120,3820 170,3910 270,4020 290,4030 320,4050 340,4070 370,4090 460,4130 560,4160 730,4190 3460,4190 3600,4170 3730,4130 3820,4090 3850,4070 3870,4050 3900,4030 3920,4020 4020,3910 4070,3820 4080,3790 4090,3740");
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
