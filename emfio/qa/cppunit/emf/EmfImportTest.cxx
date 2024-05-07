/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_fonts.h>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/seqstream.hxx>

#include <com/sun/star/graphic/EmfTools.hpp>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <memory>
#include <string_view>

using namespace css;
using namespace css::uno;
using namespace css::io;
using namespace css::graphic;
using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DContainer;

class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest(u"/emfio/qa/cppunit/emf/data/"_ustr)
    {
    }

    const OString aXPathPrefix = "/primitive2D/metafile/transform/"_ostr;

    Primitive2DSequence parseEmf(std::u16string_view aSource);
};

Primitive2DSequence Test::parseEmf(std::u16string_view aSource)
{
    const Reference<XEmfParser> xEmfParser = EmfTools::create(m_xContext);

    OUString aUrl = m_directories.getURLFromSrc(aSource);
    OUString aPath = m_directories.getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    CPPUNIT_ASSERT_MESSAGE("Unable to open file", nSize);
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));
    css::uno::Sequence<css::beans::PropertyValue> aEmptyValues;

    return xEmfParser->getDecomposition(aInputStream, aPath, aEmptyValues);
}

CPPUNIT_TEST_FIXTURE(Test, testPolyPolygon)
{
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/fdo79679-2.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT(pDocument);

    // Chart axis
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h19746v14817h-19746z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", 2);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]", "color"_ostr,
                u"#ffffff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m0 0h19781v14852h-19781z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m2574 13194v-12065h15303v12065z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke", 44);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[1]/polygon",
                       u"2574,13194 2574,1129"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "color"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "width"_ostr, u"0"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[2]/polygon",
                       u"2574,1129 2574,1129"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "color"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "width"_ostr, u"0"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[10]/polygon",
                       u"8674,1129 8674,1129"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[10]/line", "color"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[10]/line", "width"_ostr, u"0"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion", 28);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[6]", "width"_ostr, u"459"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[6]", "x"_ostr, u"9908"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[6]", "text"_ostr, u"0.5"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[6]", "fontcolor"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray", 98);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]", "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]/point", "x"_ostr, u"2574"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]/point", "y"_ostr, u"1129"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawImagePointsTypeBitmap)
{
    // tdf#142941 EMF+ file with ObjectTypeImage, FillRects, DrawImagePoints ,records
    // The test is checking the position of displaying bitmap with too large SrcRect

    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawImagePointsTypeBitmap.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#0080ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy11"_ostr, u"5347"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy13"_ostr, u"5558"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy22"_ostr, u"4717"_ustr);
    assertXPath(pDocument, aXPathPrefix + "bitmap", "xy23"_ostr, u"5565"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "bitmap/data[2]", "row"_ostr,
        u"020202,ffffff,ffffff,ffffff,fefefe,ffffff,ffffff,fefefe,ffffff,ffffff,f8f8f8,ffffff,"
        "fdfdfd,ffffff,ffffff,fdfdfd,ffffff,ffffff,ffffff,fbfbfb,010101,ffffff,fefefe,ffffff,"
        "ffffff,fbfbfb,ffffff,fdfdfd,fcfcfc,fdfdfd,ffffff,ffffff,ffffff,ffffff,ffffff,ffffff,"
        "ffffff,ffffff,ffffff,ffffff,020202,fdfdfd,ffffff,ffffff,fefefe,ffffff,ffffff,ffffff,"
        "ffffff,fbfbfb,fefefe,ffffff,fcfcfc,ffffff,fdfdfd,ffffff,ffffff,ffffff,ffffff,fbfbfb,"
        "010101,ffffff,fefefe,ffffff,ffffff,ffffff,fcfcfc,ffffff,fafafa,ffffff,ffffff,fefefe,"
        "ffffff,fdfdfd,fefefe,fefefe,ffffff,ffffff,fdfdfd,fffbfb,1e0000,8f4347,b13a3e,b82d32,"
        "bb3438,b73237,b63338,b33035,b63338"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawString)
{
#if HAVE_MORE_FONTS
    // EMF+ file with only one DrawString Record
    // Since the text is undecorated the optimal choice is a simpletextportion primitive

    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawString.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    // check correct import of the DrawString: height, position, text, color and font
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "height"_ostr,
                u"120"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "x"_ostr, u"817"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "y"_ostr, u"1138"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "text"_ostr, u"TEST"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textsimpleportion", "familyname"_ostr,
                u"CALIBRI"_ustr);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testDrawStringAlign)
{
#if HAVE_MORE_FONTS
    // EMF+ DrawString with alignment (StringAlignmentNear, StringAlignmentFar, StringAlignmentCenter)
    // It seems Arial font is replaced with Liberation Sans. These numbers are valid for Liberation Sans.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawStringAlign.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/transform", 9);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "width"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "height"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "x"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "y"_ostr,
                u"22"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "text"_ostr,
                u"HLVT"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[1]/textsimpleportion", "familyname"_ostr,
                u"ARIAL"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/transform[2]/textsimpleportion", "width"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[2]/textsimpleportion", "height"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[2]/textsimpleportion", "x"_ostr,
                u"144"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[2]/textsimpleportion", "y"_ostr,
                u"22"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[2]/textsimpleportion", "text"_ostr,
                u"HCVT"_ustr);

    // TODO Make the position of the text the same across the platforms (Arial vs Liberation Sans).
    // This is usually 276, but can be 275 as well; depends on what fonts are installed?
    sal_Int32 nX
        = getXPath(pDocument, aXPathPrefix + "mask/transform[3]/textsimpleportion", "x"_ostr)
              .toInt32();
    CPPUNIT_ASSERT(nX >= 275);
    CPPUNIT_ASSERT(nX <= 276);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[3]/textsimpleportion", "y"_ostr,
                u"22"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[3]/textsimpleportion", "text"_ostr,
                u"HRVT"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/transform[4]/textsimpleportion", "x"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[4]/textsimpleportion", "y"_ostr,
                u"66"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[4]/textsimpleportion", "text"_ostr,
                u"HLVC"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/transform[5]/textsimpleportion", "x"_ostr,
                u"143"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[5]/textsimpleportion", "y"_ostr,
                u"66"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[5]/textsimpleportion", "text"_ostr,
                u"HCVC"_ustr);

    // This is usually 274, but can be 273 as well; depends on what fonts are installed?
    nX = getXPath(pDocument, aXPathPrefix + "mask/transform[6]/textsimpleportion", "x"_ostr)
             .toInt32();
    CPPUNIT_ASSERT(nX >= 273);
    CPPUNIT_ASSERT(nX <= 274);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[6]/textsimpleportion", "y"_ostr,
                u"66"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[6]/textsimpleportion", "text"_ostr,
                u"HRVC"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/transform[7]/textsimpleportion", "x"_ostr,
                u"12"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[7]/textsimpleportion", "y"_ostr,
                u"110"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[7]/textsimpleportion", "text"_ostr,
                u"HLVB"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/transform[8]/textsimpleportion", "x"_ostr,
                u"143"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[8]/textsimpleportion", "y"_ostr,
                u"110"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[8]/textsimpleportion", "text"_ostr,
                u"HCVB"_ustr);

    // This is usually 275, but can be 274 as well; depends on what fonts are installed?
    nX = getXPath(pDocument, aXPathPrefix + "mask/transform[9]/textsimpleportion", "x"_ostr)
             .toInt32();
    CPPUNIT_ASSERT(nX >= 274);
    CPPUNIT_ASSERT(nX <= 275);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[9]/textsimpleportion", "y"_ostr,
                u"110"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform[9]/textsimpleportion", "text"_ostr,
                u"HRVB"_ustr);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testDrawStringTransparent)
{
#if HAVE_MORE_FONTS
    // EMF+ file with one DrawString Record with transparency

    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawStringTransparent.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence", "transparence"_ostr,
                u"50"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "height"_ostr, u"24"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "x"_ostr, u"66"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "y"_ostr, u"74"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "text"_ostr, u"Transparent Text"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/transform/unifiedtransparence/textsimpleportion",
                "familyname"_ostr, u"ARIAL"_ustr);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testDrawStringWithBrush)
{
    // tdf#142975 EMF+ with records: DrawString, Brush and Font
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawStringWithBrush.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "xy11"_ostr,
                u"20"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "xy13"_ostr,
                u"16"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "xy22"_ostr,
                u"20"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "xy33"_ostr, u"1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "text"_ostr,
                u"0123456789ABCDEF"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "fontcolor"_ostr,
                u"#a50021"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform/textdecoratedportion", "familyname"_ostr,
                u"TIMES NEW ROMAN"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusDrawBeziers)
{
    // tdf#107019 tdf#154789 EMF+ records: DrawBeziers
    // Check if DrawBeziers is displayed correctly and text is rotated
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawBeziers.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 4);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow", 9);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[9]/line", "color"_ostr,
                u"#00ff00"_ustr);

    assertXPath(pDocument, aXPathPrefix + "transform", 5);
    assertXPath(pDocument, aXPathPrefix + "transform[1]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]/textsimpleportion", "text"_ostr,
                u"% Efficiency"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy11"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy12"_ostr, u"4"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy13"_ostr, u"800"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy21"_ostr, u"-4"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy22"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "transform[1]", "xy23"_ostr, u"3196"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawLine)
{
    // EMF+ with records: DrawLine
    // The line is colored and has a specified width, therefore a polypolygonstroke primitive is the optimal choice
    Primitive2DSequence aSequence = parseEmf(u"emfio/qa/cppunit/emf/data/TestDrawLine.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    // check correct import of the DrawLine: color and width of the line
    assertXPath(pDocument, aXPathPrefix + "mask/unifiedtransparence", "transparence"_ostr,
                u"14"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/unifiedtransparence/polypolygonstroke/line",
                "color"_ostr, u"#c01002"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/unifiedtransparence/polypolygonstroke/line",
                "width"_ostr, u"115"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/unifiedtransparence/polypolygonstroke/line",
                "linecap"_ostr, u"BUTT"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/unifiedtransparence/polypolygonstroke/polypolygon",
                "path"_ostr,
                u"m89.1506452315894 403.573503917507 895.170581035125-345.821325648415"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawLineWithCaps)
{
    // EMF+ with records: DrawLine
    // Test lines with different caps styles and arrows
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawLineWithCaps.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow", 3);

    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[1]/line", "width"_ostr, u"212"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[1]/stroke", 0);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[1]/linestartattribute/polypolygon",
                "path"_ostr, u"m0-1 1 2h-2z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[1]/lineendattribute", 0);

    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[2]/line", "width"_ostr, u"212"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[2]/stroke", 0);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[2]/linestartattribute/polypolygon",
                "path"_ostr, u"m0-1 1 2h-2z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[2]/lineendattribute", 0);

    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[3]/line", "width"_ostr, u"423"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[3]/stroke", 0);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[3]/linestartattribute", 0);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow[3]/lineendattribute/polypolygon",
                "path"_ostr, u"m-1-1h2v2h-2z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence", 3);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]", "transparence"_ostr,
                u"39"_ustr);
    assertXPath(pDocument,
                aXPathPrefix
                    + "unifiedtransparence[1]/polygonstrokearrow/linestartattribute/polypolygon",
                "path"_ostr,
                u"m-1 1h2v-1l-0.0764-0.3827-0.2165-0.3244-0.3244-0.2165-0.3827-0.0764-0.3827 "
                "0.0764-0.3244 0.2165-0.2165 0.3244-0.0764 0.3827z"_ustr);
    assertXPath(pDocument,
                aXPathPrefix
                    + "unifiedtransparence[1]/polygonstrokearrow/lineendattribute/polypolygon",
                "path"_ostr, u"m-1 1h2v-1l-1-1-1 1z"_ustr);
    assertXPath(pDocument,
                aXPathPrefix + "unifiedtransparence[2]/polygonstrokearrow/linestartattribute", 0);
    assertXPath(pDocument,
                aXPathPrefix
                    + "unifiedtransparence[2]/polygonstrokearrow/lineendattribute/polypolygon",
                "path"_ostr, u"m-1-1h2v2h-2z"_ustr);
    assertXPath(pDocument,
                aXPathPrefix
                    + "unifiedtransparence[3]/polygonstrokearrow/lineendattribute/polypolygon",
                "path"_ostr, u"m0-1 1 1-0.5 0.5v0.5h-1v-0.5l-0.5-0.5z"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawLineWithDash)
{
    // EMF+ with records: DrawLine, ScaleWorldTransform, RotateWorldTransform
    // Test lines with different dash styles, different line arrows and different World Rotation
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawLineWithDash.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    // check correct import of the DrawLine: color and width of the line
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke", 10);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[1]/line", "color"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[1]/line", "width"_ostr,
                u"185"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[1]/stroke", 0);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[2]/line", "width"_ostr,
                u"185"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[2]/stroke", "dotDashArray"_ostr,
                u"185 185 "_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[3]/line", "width"_ostr,
                u"185"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[3]/stroke", "dotDashArray"_ostr,
                u"556 185 "_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[4]/line", "width"_ostr,
                u"185"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[4]/stroke", "dotDashArray"_ostr,
                u"556 185 185 185 "_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[5]/line", "width"_ostr,
                u"370"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[5]/stroke", "dotDashArray"_ostr,
                u"556 185 185 185 185 185 "_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow", 2);
    //TODO polypolygonstroke[6-9]/stroke add support for PenDataDashedLineOffset
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[1]/line", "width"_ostr,
                u"370"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[1]/stroke", "dotDashArray"_ostr,
                u"1851 741 5554 1481 "_ustr);
    // Arrows on both ends
    assertXPath(pDocument,
                aXPathPrefix + "mask/polygonstrokearrow[1]/linestartattribute/polypolygon",
                "path"_ostr, u"m0-1 1 2h-2z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[1]/lineendattribute/polypolygon",
                "path"_ostr, u"m0-1 1 2h-2z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[2]/line", "width"_ostr,
                u"370"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[2]/stroke", "dotDashArray"_ostr,
                u"1852 741 5555 1481 "_ustr);
    assertXPath(pDocument,
                aXPathPrefix + "mask/polygonstrokearrow[2]/linestartattribute/polypolygon",
                "path"_ostr,
                u"m-1 1h2v-1l-0.0764-0.3827-0.2165-0.3244-0.3244-0.2165-0.3827-0.0764-0.3827 "
                "0.0764-0.3244 0.2165-0.2165 0.3244-0.0764 0.3827z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow[2]/lineendattribute/polypolygon",
                "path"_ostr,
                u"m-1 1h2v-1l-0.0764-0.3827-0.2165-0.3244-0.3244-0.2165-0.3827-0.0764-0.3827 "
                "0.0764-0.3244 0.2165-0.2165 0.3244-0.0764 0.3827z"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testLinearGradient)
{
    // EMF+ file with LinearGradient brush
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestLinearGradient.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/metafile/transform"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "height"_ostr, u"7610"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "width"_ostr, u"15232"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h15232v7610h-15232z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "spreadmethod"_ostr,
                u"repeat"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "startx"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "starty"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "endx"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "endy"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]", "opacity"_ostr,
                u"0.392156862745098"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[1]/polypolygon", "path"_ostr,
                u"m0 0.216110019646294h7615.75822989746v7610.21611001965h-7615.75822989746z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "spreadmethod"_ostr,
                u"repeat"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "startx"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "starty"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "endx"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "endy"_ostr, u"-1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/svglineargradient[2]", "opacity"_ostr, u"1"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/svglineargradient[2]/polypolygon", "path"_ostr,
        u"m7615.75822989746 0.216110019646294h7615.75822989746v7610.21611001965h-7615.75822989746z"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTextMapMode)
{
    // EMF with records: SETMAPMODE with MM_TEXT MapMode, POLYLINE16, EXTCREATEPEN, EXTTEXTOUTW
    // MM_TEXT is mapped to one device pixel. Positive x is to the right; positive y is down.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TextMapMode.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 2);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ffffff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m0 0h3542v4647h-3542z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "textsimpleportion", 20);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "text"_ostr, u"N"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "fontcolor"_ostr,
                u"#4a70e3"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "x"_ostr, u"2099"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "y"_ostr, u"1859"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polygonstroke", 138);
    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[1]/polygon",
                       u"2142,1638 2142,1489"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[1]/line", "color"_ostr, u"#4a70e3"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[1]/line", "width"_ostr, u"11"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[10]/polygon",
                       u"1967,1029 1869,952"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[10]/line", "color"_ostr, u"#4a70e3"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[10]/line", "width"_ostr, u"11"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[20]/polygon",
                       u"2710,1113 2873,1330"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[20]/line", "color"_ostr, u"#666666"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[20]/line", "width"_ostr, u"11"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEnglishMapMode)
{
    // EMF image with records: SETMAPMODE with MM_ENGLISH MapMode, STROKEANDFILLPATH, EXTTEXTOUTW, SETTEXTALIGN, STRETCHDIBITS
    // MM_LOENGLISH is mapped to 0.01 inch. Positive x is to the right; positive y is up.M
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/test_mm_hienglish_ref.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", 1);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon[1]", "path"_ostr,
                u"m0 0h29699v20999h-29699z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", 3);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]", "color"_ostr,
                u"#ffffad"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m-1-1h29700v21001h-29700z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m1058 7937v5293h3175v-1059h-2118v-4234z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[3]/polypolygon", "path"_ostr,
                u"m12699 1058h4234v1060h-1587v4231h-1059v-4231h-1588z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion", 4);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "text"_ostr, u"UL"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "fontcolor"_ostr,
                u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "x"_ostr, u"106"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "y"_ostr, u"459"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "width"_ostr, u"424"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "height"_ostr, u"424"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline", 3);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[1]/polygon",
                       u"-1,-1 29699,-1 29699,21000 -1,21000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[2]/polygon",
                       u"1058,7937 1058,13230 4233,13230 4233,12171 2115,12171 2115,7937"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "mask/polygonhairline[3]/polygon",
        u"12699,1058 16933,1058 16933,2118 15346,2118 15346,6349 14287,6349 14287,2118 12699,2118"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRectangleWithModifyWorldTransform)
{
    // EMF image with records: EXTCREATEPEN, SELECTOBJECT, MODIFYWORLDTRANSFORM, RECTANGLE

    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestRectangleWithModifyWorldTransform.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 1);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ffffff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m1042 417 918 529 353 610-918-528z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polygonstroke", 1);
    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[1]/polygon",
                       u"1042,417 1960,946 2313,1556 1395,1028"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testChordWithModifyWorldTransform)
{
    // EMF import test with records: CHORD, MODIFYWORLDTRANSFORM, EXTCREATEPEN, SELECTOBJECT
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestChordWithModifyWorldTransform.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#ffffff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor/polypolygon", "path"_ostr,
                u"m590 448-21 45-66 24-34 12-33 12-21 45-33 12 12 33-33 12 12 33 12 34 33-12 12 33 "
                "34-12 33-12 45 21 33-12 33-12 46 21 66-25 33-12 66-24 34-12 66-24z"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke/polygon",
                       u"590,448 569,493 503,517 469,529 436,541 415,586 382,598 394,631 361,643 "
                       "361,643 373,676 373,676 385,710 418,698 430,731 464,719 497,707 542,728 "
                       "575,716 608,704 654,725 720,700 753,688 819,664 853,652 919,628"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testArcStartPointEqualEndPoint)
{
    // i73608 EMF import test where StartPoint == EndPoint. It should draw full circle
    // Records: SETMAPMODE, SETWINDOWEXTEX, SETWINDOWORGEX, EXTSELECTCLIPRGN, INTERSECTCLIPRECT, MOVETOEX, ARC
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestArcStartPointEqualEndPoint.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/group/mask/polygonhairline", "color"_ostr,
                u"#000000"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "mask/group/mask/polygonhairline/polygon",
        u"11886,23133 11970,23223 12051,23316 12131,23410 12208,23506 12282,23604 12354,23704 "
        "12424,23805 12491,23909 12556,24014 12618,24120 12677,24228 12734,24337 12788,24448 "
        "12839,24560 12888,24673 12933,24788 12976,24903 13016,25020 13053,25137 13087,25256 "
        "13119,25375 13147,25495 13172,25615 13195,25736 13214,25858 13230,25980 13244,26103 "
        "13254,26225 13261,26348 13266,26472 13267,26595 13265,26718 13260,26841 13253,26964 "
        "13242,27087 13228,27209 13211,27331 13191,27453 13168,27574 13142,27694 13113,27814 "
        "13082,27933 13047,28051 13009,28169 12969,28285 12926,28400 12879,28514 12830,28628 "
        "12779,28739 12724,28850 12667,28959 12607,29067 12545,29173 12480,29277 12412,29380 "
        "12342,29482 12269,29581 12194,29679 12117,29775 12037,29869 11955,29960 11871,30050 "
        "11784,30138 11696,30224 11605,30307 11512,30388 11418,30467 11321,30543 11223,30617 "
        "11122,30689 11020,30758 10917,30825 10811,30888 10705,30950 10596,31009 10487,31065 "
        "10376,31118 10263,31168 10150,31216 10035,31261 9919,31303 9803,31343 9685,31379 "
        "9566,31412 9447,31443 9327,31471 9206,31495 9085,31517 8963,31535 8841,31551 8719,31564 "
        "8596,31573 8473,31580 8350,31583 8226,31584 8103,31581 7980,31576 7857,31567 7735,31555 "
        "7612,31541 7491,31523 7369,31503 7248,31479 7128,31452 7008,31423 6890,31390 6772,31355 "
        "6655,31316 6538,31275 6423,31231 6310,31184 6197,31135 6085,31082 5975,31027 5866,30969 "
        "5759,30909 5653,30846 5549,30780 5447,30712 5346,30641 5247,30568 5150,30492 5054,30414 "
        "4961,30334 4870,30251 4780,30166 4693,30079 4608,29990 4525,29899 4445,29805 4367,29710 "
        "4291,29613 4217,29514 4146,29414 4078,29311 4012,29207 3949,29101 3888,28994 3830,28885 "
        "3775,28775 3722,28664 3672,28551 3625,28438 3581,28323 3540,28207 3501,28090 3465,27972 "
        "3433,27853 3403,27733 3376,27613 3352,27492 3331,27371 3313,27249 3299,27127 3287,27004 "
        "3278,26881 3272,26758 3269,26635 3270,26512 3273,26388 3279,26265 3289,26143 3301,26020 "
        "3316,25898 3335,25776 3356,25655 3380,25534 3408,25414 3438,25294 3471,25176 3508,25058 "
        "3547,24941 3588,24825 3633,24711 3681,24597 3731,24484 3784,24373 3840,24263 3899,24155 "
        "3960,24048 4023,23943 4090,23839 4159,23737 4230,23636 4304,23538 4380,23441 4459,23346 "
        "4540,23253 4623,23162 4708,23074 4796,22987 4885,22902 4977,22820 5071,22740 5166,22663 "
        "5264,22587 5363,22515 5465,22444 5567,22376 5672,22311 5778,22249 5885,22188 5994,22131 "
        "6105,22076 6216,22024 6329,21975 6443,21929 6559,21885 6675,21845 6792,21807 6910,21772 "
        "7029,21740 7149,21711 7269,21685 7390,21662 7512,21642 7634,21624 7756,21610 7879,21599 "
        "8002,21591 8125,21586 8248,21584 8371,21585 8494,21589 8617,21596 8740,21606 8862,21619 "
        "8985,21636 9106,21655 9227,21677 9348,21702 9468,21730 9587,21761 9705,21795 9823,21832 "
        "9940,21872 10055,21914 10170,21960 10283,22008 10395,22059 10506,22113 10615,22169 "
        "10723,22229 10830,22291 10935,22355 11038,22422 11140,22491 11240,22563 11338,22638 "
        "11434,22715 11529,22794 11621,22875 11711,22959 11800,23045"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testArcInsideWronglyDefinedRectangle)
{
    // tdf#142268 EMF import test with records: ARC
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestArcInsideWronglyDefinedRectangle.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polygonhairline", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "polygonhairline/polygon",
        u"1630,1460 1650,1470 1670,1480 1700,1490 1720,1500 1750,1510 1770,1520 1800,1530 "
        u"1820,1530 "
        "1850,1540 1870,1540 1900,1540 1930,1540 1950,1540 1980,1540 2000,1530 2030,1530 2050,1520 "
        "2080,1510 2100,1500 2130,1490 2150,1480 2170,1470 2200,1450 2220,1440 2240,1420 2260,1400 "
        "2280,1390 2290,1370 2310,1350 2330,1330 2340,1300 2360,1280 2370,1260 2380,1240 2390,1210 "
        "2400,1190 2410,1160 2420,1140 2420,1110 2420,1080 2430,1060 2430,1030 2430,1000 2430,980 "
        "2430,950 2420,930 2420,900 2410,870 2410,850 2400,820 2390,800 2380,770 2360,750 2350,730 "
        "2340,710 2320,680 2300,660 2290,640 2270,630 2250,610 2230,590 2210,580 2190,560 2160,550 "
        "2140,540 2120,520 2090,510 2070,510 2040,500 2020,490 1990,490 1970,480 1940,480 1920,480 "
        "1890,480 1860,480 1840,490 1810,490 1790,500 1760,500 1740,510 1710,520 1690,530 1670,540 "
        "1640,560 1620,570 1600,580 1580,600 1560,620 1540,640 1520,660 1510,680 1490,700 1480,720 "
        "1460,740 1450,760"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEllipseWithSelectClipPath)
{
    // EMF import test with records: RECTANGLE, BEGINPATH, ENDPATH, ELLIPSE
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEllipseWithSelectClipPath.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "group/mask/polypolygon", 1);
    assertXPath(
        pDocument, aXPathPrefix + "group/mask/polypolygon[1]", "path"_ostr,
        u"m2790 "
        "776v-36-35h-36v-35l-35-35-35-36h-36l-35-35-35-35h-35-36l-35-35h-35-36l-35-36h-35-"
        "36l-35-35h-35-71-35l-36-35h-70-35-36-70l-36-35h-35-71-35-71-35-71-35-35-71-35-71-"
        "35-71-35l-35 35h-71-35-36-70l-35 35h-36-70-36l-35 35h-35-36l-35 36h-35-36l-35 "
        "35h-35-36l-35 35-35 35h-35l-36 36-35 35v35h-35v35 36 35 35h35v35l35 36 36 "
        "35h35l35 35 35 35h36 35l35 36h36 35l35 35h36 35l35 35h36 70 36l35 35h70 36 35 "
        "71l35 36h35 71 35 71 35 71 35 35 71 35 71 35 71 35l36-36h70 36 35 70l36-35h35 71 "
        "35l35-35h36 35l35-35h36 35l35-36h36 35l35-35 35-35h36l35-35 35-36v-35h36v-35z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group/mask/polypolygoncolor", 1);
    assertXPath(pDocument, aXPathPrefix + "group/mask/polypolygoncolor[1]", "color"_ostr,
                u"#ffff00"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group/mask/polypolygoncolor[1]/polypolygon[1]",
                "path"_ostr, u"m353 353h2472v1057h-2472z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group/mask/polygonstroke", 1);
    assertXPathContent(pDocument, aXPathPrefix + "group/mask/polygonstroke[1]/polygon",
                       u"353,353 2825,353 2825,1410 353,1410"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEllipseXformIntersectClipRect)
{
    // EMF import test with records: EXTCREATEPEN, CREATEBRUSHINDIRECT, MODIFYWORLDTRANSFORM, INTERSECTCLIPRECT, ELLIPSE
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEllipseXformIntersectClipRect.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h3000v2000h-3000z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/group/mask/polypolygon", "path"_ostr,
                u"m370 152 1128-409 592 1623-1128 410z"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/group/mask/polypolygoncolor/polypolygon", "path"_ostr,
        u"m3613 287-12-33-12-33-12-33-12-33-33 12-12-34-13-33-45-21-12-33-33 "
        "12-12-33-45-21-46-21-12-33-33 12-12-33-45-21-33 12-46-21-45-21-33 12-45-21-34 12-45-21-33 "
        "12-45-21-34 12-45-21-33 12-45-21-33 12-34 12-45-21-33 12-33 12-45-21-34 12-66 24-45-21-33 "
        "12-34 12-66 24-33 12-45-21-34 12-66 24-33 12-33 12-34 12-66 24-33 12-33 12-67 25-33 12-33 "
        "12-33 12-67 24-33 12-21 45-33 12-66 24-34 12-33 12-21 46-66 24-33 12-22 45-33 12-33 12-21 "
        "45-33 12-33 12-21 46-34 12-21 45-33 12-21 45-33 12-21 45-34 12-21 45-33 13-21 45-21 45-33 "
        "12-21 45 12 33-33 12 12 33-21 46-22 45 13 33-34 12 12 33-21 45 12 33 12 34-33 12 12 33 12 "
        "33 13 33 12 33 12 33 12 33 12 33 12 34 33-12 12 33 12 33 46 21 12 33 33-12 12 33 45 21 45 "
        "21 12 33 34-12 12 33 45 21 33-12 45 21 46 22 33-13 45 22 33-13 46 22 33-13 45 22 33-13 45 "
        "22 34-13 45 22 33-12 33-13 46 22 33-13 33-12 45 21 33-12 67-24 45 21 33-12 33-12 67-24 "
        "33-12 45 21 33-12 67-24 33-12 33-12 33-12 67-24 33-12 33-12 66-25 34-12 33-12 33-12 66-24 "
        "33-12 22-45 33-12 66-24 33-12 33-12 22-45 66-25 33-12 21-45 33-12 34-12 21-45 33-12 33-12 "
        "21-45 33-12 21-46 34-12 21-45 33-12 21-45 33-12 21-45 33-12 22-46 21-45 33-12 21-45-12-33 "
        "33-12-12-33 21-46 21-45-12-33 33-12-12-33 21-45-12-33-12-33 33-12-12-34-12-33-12-33z"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "mask/group/mask/polygonstroke/polygon",
        u"3613,287 3601,254 3601,254 3589,221 3577,188 3565,155 3532,167 3520,133 3507,100 "
        u"3507,100 "
        "3462,79 3450,46 3417,58 3405,25 3360,4 3360,4 3314,-17 3302,-50 3269,-38 3257,-71 "
        "3212,-92 3179,-80 3133,-101 3133,-101 3088,-122 3055,-110 3010,-131 2976,-119 2931,-140 "
        "2898,-128 2853,-149 2819,-137 2774,-158 2741,-146 2696,-167 2663,-155 2629,-143 2584,-164 "
        "2551,-152 2518,-140 2473,-161 2439,-149 2373,-125 2328,-146 2295,-134 2261,-122 2195,-98 "
        "2162,-86 2117,-107 2083,-95 2017,-71 1984,-59 1951,-47 1917,-35 1851,-11 1818,1 1818,1 "
        "1785,13 1718,38 1685,50 1652,62 1619,74 1552,98 1519,110 1498,155 1465,167 1399,191 "
        "1365,203 1332,215 1311,261 1245,285 1212,297 1190,342 1157,354 1124,366 1103,411 1070,423 "
        "1037,435 1016,481 982,493 961,538 928,550 907,595 874,607 853,652 819,664 798,709 765,722 "
        "744,767 744,767 723,812 690,824 669,869 681,902 648,914 660,947 639,993 639,993 617,1038 "
        "630,1071 596,1083 608,1116 587,1161 587,1161 599,1194 611,1228 578,1240 590,1273 602,1306 "
        "615,1339 615,1339 627,1372 627,1372 639,1405 639,1405 651,1438 663,1471 675,1505 708,1493 "
        "720,1526 732,1559 732,1559 778,1580 790,1613 823,1601 835,1634 880,1655 880,1655 925,1676 "
        "937,1709 971,1697 983,1730 1028,1751 1061,1739 1106,1760 1106,1760 1152,1782 1185,1769 "
        "1230,1791 1263,1778 1309,1800 1342,1787 1387,1809 1420,1796 1465,1818 1499,1805 1544,1827 "
        "1577,1815 1610,1802 1656,1824 1689,1811 1722,1799 1767,1820 1800,1808 1867,1784 1912,1805 "
        "1945,1793 1978,1781 2045,1757 2078,1745 2123,1766 2156,1754 2223,1730 2256,1718 2289,1706 "
        "2322,1694 2389,1670 2422,1658 2422,1658 2455,1646 2521,1621 2555,1609 2588,1597 2621,1585 "
        "2687,1561 2720,1549 2742,1504 2775,1492 2841,1468 2874,1456 2907,1444 2929,1399 2995,1374 "
        "3028,1362 3049,1317 3082,1305 3116,1293 3137,1248 3170,1236 3203,1224 3224,1179 3257,1167 "
        "3278,1121 3312,1109 3333,1064 3366,1052 3387,1007 3420,995 3441,950 3474,938 3496,892 "
        "3496,892 3517,847 3550,835 3571,790 3559,757 3592,745 3580,712 3601,666 3601,666 3622,621 "
        "3610,588 3643,576 3631,543 3652,498 3652,498 3640,465 3628,432 3661,420 3649,386 3637,353 "
        "3625,320 3625,320"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSetArcDirection)
{
    // EMF import test with records: SETARCDIRECTION, ARC, PIE
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestSetArcDirection.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#ffffff"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "polypolygoncolor/polypolygon", "path"_ostr,
        u"m1640 1570-1000-950 50-50 50-50 50-50 50-40 60-40 50-40 60-30 60-40 60-20 60-30 70-20 "
        "60-20 70-10 60-20h70l70-10h60 70l70 10 60 10 70 10 70 20 60 20 60 20 70 30 60 30 60 30 50 "
        "40 60 40 50 40 50 40 50 50 50 50 50 50 40 60 40 60 40 60 30 60 30 60 30 60 20 70 30 70 10 "
        "60 20 70 10 70 10 70 10 70v80 70l-10 70v70l-10 70-20 70-20 70z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline", 2);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[1]", "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[2]", "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline", 2);
    assertXPathContent(
        pDocument, aXPathPrefix + "polygonhairline[1]/polygon",
        u"1070,1570 1110,1560 1160,1540 1200,1530 1250,1520 1300,1510 1350,1510 1400,1500 "
        u"1440,1500 "
        "1490,1500 1540,1500 1590,1500 1640,1510 1690,1510 1740,1520 1780,1530 1830,1540 1880,1560 "
        "1920,1570 1960,1590 2010,1610 2050,1630 2090,1650 2130,1670 2160,1700 2200,1720 2230,1750 "
        "2260,1780 2290,1810 2320,1840 2350,1870 2370,1900 2390,1930 2410,1970 2430,2000 2440,2030 "
        "2450,2070 2460,2110 2470,2140 2480,2180 2480,2220 2480,2250 2480,2290 2470,2320 2470,2360 "
        "2460,2400 2450,2430 2430,2470 2420,2500 2400,2540 2380,2570 2350,2600 2330,2630 2300,2660 "
        "2270,2690 2240,2720 2210,2750 2170,2770 2140,2800 2100,2820 2060,2840 2020,2860 1980,2880 "
        "1940,2900 1890,2920 1850,2930 1800,2940 1750,2950 1700,2960 1660,2970 1610,2970 1560,2980 "
        "1510,2980 1460,2980 1410,2980 1360,2970 1320,2970 1270,2960 1220,2950 1170,2940 1130,2930 "
        "1080,2910 1040,2900 1000,2880 950,2860 910,2840 870,2820 840,2800 800,2770 770,2740 "
        "730,2720 700,2690 670,2660 650,2630 620,2600 600,2560 580,2530 560,2500 550,2460 530,2430 "
        "520,2390 510,2360 510,2320 500,2280 500,2250 500,2210 500,2170 510,2140 520,2100 530,2070 "
        "540,2030 560,1990 570,1960 590,1930 610,1890 640,1860 660,1830 690,1800 720,1770 750,1740 "
        "790,1720 820,1690 860,1670 900,1650 940,1630 980,1610 1020,1590"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "polygonhairline[2]/polygon",
        u"1640,1570 640,620 690,570 740,520 790,470 840,430 900,390 950,350 1010,320 1070,280 "
        "1130,260 1190,230 1260,210 1320,190 1390,180 1450,160 1520,160 1590,150 1650,150 1720,150 "
        "1790,160 1850,170 1920,180 1990,200 2050,220 2110,240 2180,270 2240,300 2300,330 2350,370 "
        "2410,410 2460,450 2510,490 2560,540 2610,590 2660,640 2700,700 2740,760 2780,820 2810,880 "
        "2840,940 2870,1000 2890,1070 2920,1140 2930,1200 2950,1270 2960,1340 2970,1410 2980,1480 "
        "2980,1560 2980,1630 2970,1700 2970,1770 2960,1840 2940,1910 2920,1980"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawPolyLine16WithClip)
{
    // EMF image with records:
    // CREATEBRUSHINDIRECT, FILLRGN, BEGINPATH, POLYGON16, SELECTCLIPPATH, MODIFYWORLDTRANSFORM, SELECTOBJECT
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestDrawPolyLine16WithClip.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h3943v3939h-3943z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", 1);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m1323 0h1323v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]", "color"_ostr,
                u"#b4ffff"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline", 1);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[1]/polygon",
                       u"1323,0 2646,0 2646,1322 3969,1322 3969,2644 2646,2644 2646,3966 1323,3966 "
                       "1323,2644 0,2644 0,1322 1323,1322"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline[1]", "color"_ostr, u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", "path"_ostr,
                u"m2646 0v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323v-1322"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/group[1]/mask/polygonstroke/polygon",
                       u"0,793 3969,4230"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFillRegion)
{
    // EMF import with records: CREATEBRUSHINDIRECT, FILLRGN. The SETICMMODE is also used.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestFillRegion.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h3943v3939h-3943z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", 1);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m1323 0h1323v1322h1323v1322h-1323v1322h-1323v-1322h-1323v-1322h1323z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]", "color"_ostr,
                u"#ff0000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline", 1);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[1]/polygon",
                       u"1323,0 2646,0 2646,1322 3969,1322 3969,2644 2646,2644 2646,3966 1323,3966 "
                       "1323,2644 0,2644 0,1322 1323,1322"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline[1]", "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPolylinetoCloseStroke)
{
    // EMF import with records: BEGINPATH, ARC, ENDPATH, STROKEPATH, EXTCREATEPEN.
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestPolylinetoCloseStroke.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polygonhairline", 2);
    assertXPathContent(
        pDocument, aXPathPrefix + "polygonhairline[1]/polygon",
        u"1080,150 1010,170 940,190 870,210 810,230 750,260 690,280 630,310 570,340 520,380 "
        u"470,410 "
        "420,450 370,490 330,530 290,570 260,610 230,660 200,700 170,750 150,790 130,840 120,890 "
        "110,930 100,980 100,1030 100,1080 110,1130 120,1180 130,1220 140,1270 160,1320 190,1360 "
        "210,1410 250,1450 280,1490 320,1540 360,1580 400,1620 450,1650 500,1690"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[1]", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(
        pDocument, aXPathPrefix + "polygonhairline[2]/polygon",
        u"1760,1120 1710,1130 1670,1140 1620,1150 1580,1160 1540,1170 1500,1180 1460,1200 "
        u"1420,1210 "
        "1380,1230 1350,1240 1320,1260 1290,1280 1260,1300 1230,1310 1210,1330 1190,1360 1170,1380 "
        "1150,1400 1140,1420 1120,1440 1110,1460 1110,1490 1100,1510 1100,1530 1100,1550 1100,1580 "
        "1110,1600 1120,1620 1130,1650 1140,1670 1160,1690 1170,1710 1190,1730"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[2]", "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPolydraw)
{
    // tdf#142249 EMF import with records: POLYDRAW, CREATEPEN.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEmfPolydraw.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polygonhairline", 2);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[1]", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "polygonhairline[1]/polygon",
                       u"2000,200 3000,200 3000,1000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[2]", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "polygonhairline[2]/polygon",
                       u"3000,2000 3000,2000 4000,2000 3000,3000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polygonstroke", 1);
    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[1]/polygon",
                       u"50,50 50,50 1000,1000 1000,1000 2000,2500 2000,1000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[1]/line", "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[1]/stroke", "dotDashArray"_ostr,
                u"30 10 "_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusBrushPathGradientWithBlendColors)
{
    // tdf#131506 EMF+ records: FillRects, Brush with PathGradient and BlendColor, FillRects
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusBrushPathGradientWithBlendColors.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "svgradialgradient", "radius"_ostr, u"0.7"_ustr);
    assertXPath(pDocument, aXPathPrefix + "svgradialgradient/focalx", 0);
    assertXPath(pDocument, aXPathPrefix + "svgradialgradient/focaly", 0);
    assertXPath(pDocument, aXPathPrefix + "svgradialgradient", "startx"_ostr, u"1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "svgradialgradient", "starty"_ostr, u"1"_ustr);
    assertXPath(pDocument, aXPathPrefix + "svgradialgradient", "spreadmethod"_ostr, u"pad"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusDrawCurve)
{
    // tdf#143877 EMF+ records: DrawCurve, DrawClosedCurve
    Primitive2DSequence aSequence = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawCurve.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke", 26);

    // Verify DrawCurve with tension=0.5, offset=0, segments=2
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[3]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/polypolygonstroke[3]/polypolygon", "path"_ostr,
        u"m4121.5678588927 597.698391624308 124.350808427242 119.857180790532 168.799608035277 "
        "163.249184432137 191.024007839294 186.003527805174 191.024007839294 188.120210909643 "
        "168.799608035277 169.599233745543 124.350808427242 130.440596312875 57.677609015188 "
        "70.6442986116379 14.8162665360123 69.7182497534329 10.5830475257226 128.191620514377 "
        "6.34982851543373 166.821087170928 2.11660950514397 185.606649723086-2.11660950514397 "
        "184.548308170852-6.34982851543373 163.646062514225-10.5830475257226 "
        "122.899912753206-14.8162665360123 62.3098588877929"_ustr);

    // Verify DrawCurve with tension=1.125, offset=0, segments=3
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[9]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/polypolygonstroke[9]/polypolygon", "path"_ostr,
        u"m6593.23860852523 4960.44685532205 129.113179813817 111.125862984599 154.512493875552 "
        "132.292694029285 167.212150906418 147.109475760564 167.212150906418 155.576208178439 "
        "154.512493875552 157.692891282907 129.113179813817 153.45952507397 91.0142087212153 "
        "142.876109551627 59.2650661440466 139.171914118808 42.3321901028912 144.463621879979 "
        "25.399314061734 146.580304984447 8.46643802057861 145.521963432213-8.46643802057861 "
        "141.288597223276-25.399314061734 133.880206357636-42.3321901028912 "
        "123.296790835293-59.2650661440466 109.538350656248-83.6060754532091 "
        "99.2195205219632-111.121999020089 93.3986419846751-132.288094071533 "
        "85.9902511190348-147.104360607545 76.9943479250442-155.570798628123 "
        "66.4109324027004-157.687408133268 54.2400045520071-153.454189122979 "
        "40.4815643729608-142.871141597256 25.135611865564"_ustr);

    // Verify DrawCurve with tension=0.125, offset=0, segments=4
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[13]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/polypolygonstroke[13]/polypolygon", "path"_ostr,
        u"m4121.5678588927 9267.6323875275 124.350808427242 119.857180790532 168.799608035277 "
        "163.249184432138 191.024007839294 186.003527805173 191.024007839294 188.120210909643 "
        "168.799608035277 169.599233745543 124.350808427242 130.440596312876 57.677609015188 "
        "70.6442986116381 14.8162665360123 69.7182497534323 10.5830475257226 128.191620514377 "
        "6.34982851543373 166.821087170927 2.11660950514397 185.606649723086-2.11660950514397 "
        "184.548308170854-6.34982851543373 163.646062514224-10.5830475257226 "
        "122.899912753204-14.8162665360123 62.3098588877929-55.8255756981871 "
        "42.2675157423582-119.853013228809 69.3875180183586-163.243508084272 "
        "86.5855682421661-185.997060264576 93.861666413779-188.113669769721 "
        "91.2158125331916-169.593336599706 78.6480066004096-130.436060754531 "
        "56.1582486154312-70.6418422341994 "
        "23.7465385782562-70.6418422341985-7.93756164175647-130.436060754532-28.046051134208-169."
        "593336599706-41.8044913132544-188.113669769721-49.2128821788938-185.997060264576-50."
        "271223731128-163.243508084272-44.979515969957-119.853013228809-33.3377588953808-55."
        "8255756981871-15.3459525073959"_ustr);

    // Verify DrawCurve with tension=0.125, offset=1, segments=3
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[18]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/polypolygonstroke[18]/polypolygon", "path"_ostr,
        u"m5162.41058304753 14700.2318678401 10.5830475257226 128.191620514377 6.34982851543373 "
        "166.821087170927 2.11660950514397 185.606649723086-2.11660950514397 "
        "184.548308170854-6.34982851543373 163.646062514224-10.5830475257226 "
        "122.899912753204-14.8162665360123 62.3098588877929-55.8255756981871 "
        "42.2675157423582-119.853013228809 69.3875180183586-163.243508084272 "
        "86.5855682421661-185.997060264576 93.861666413779-188.113669769721 "
        "91.2158125331916-169.593336599706 78.6480066004096-130.436060754531 "
        "56.1582486154312-70.6418422341994 23.7465385782562"_ustr);

    // Verify DrawClosedCurve with tension=0.5
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygonstroke[22]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(
        pDocument, aXPathPrefix + "mask/polypolygonstroke[22]/polypolygon", "path"_ostr,
        u"m2709.26016658501 19012.0476443365v122.767620059174 173.568014566423 198.968211820044 "
        "198.968211820044 173.568014566423 122.767620059174 46.5670282983083l-46.5654091131796 "
        "23.283514149156-122.763351298383 61.383810029587-173.561979421852 "
        "86.7840072832114-198.961293483586 99.4841059100218-198.961293483586 "
        "99.4841059100218-173.561979421852 86.7840072832114-122.763351298383 "
        "61.383810029587-46.5654091131796 "
        "23.283514149156-46.5654091131798-11.6417570745798-122.763351298383-30.6919050147917-173."
        "561979421852-43.3920036416057-198.961293483586-49.7420529550109-198.961293483586-49."
        "7420529550109-173.561979421852-43.3920036416057-122.763351298383-30.6919050147953-46."
        "5654091131798-11.6417570745762 46.5654091131798-104.775813671193 "
        "122.763351298383-276.227145133147 173.561979421852-390.528032774448 "
        "198.961293483586-447.678476595098 198.961293483586-447.678476595102 "
        "173.561979421852-390.528032774448 122.763351298383-276.227145133147 "
        "46.5654091131798-104.775813671193 46.5654091131796 46.5670282983083 122.763351298383 "
        "122.767620059178 173.561979421852 173.568014566419 198.961293483586 198.968211820044 "
        "198.961293483586 198.968211820047 173.561979421852 173.568014566419 122.763351298383 "
        "122.767620059178 46.5654091131796 46.5670282983083z"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusGetDC)
{
    // tdf#147818 EMF+ records: GetDC, DrawPath, FillRects
    Primitive2DSequence aSequence = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusGetDC.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "textsimpleportion", "text"_ostr,
                u"sd CCCCCCCCCCCCCCC"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion", "fontcolor"_ostr, u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group", 5);
    assertXPath(
        pDocument, aXPathPrefix + "group[4]/textsimpleportion", "text"_ostr,
        u"Getttttttttttttttttttttttttttttt, uuuu: \"eeeeeeeeeeeeeeeeeeeeeee-7acd04a3953b\")"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[5]/textsimpleportion", "text"_ostr,
                u"TTTTTTTTTTTTTTTTTTTTTTTTTTTTT, trackId: 55)"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[5]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 6);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m105.78125 "
                "776.111111111111h3878.64583333333l458.385416666667-493.888888888889v-176."
                "388888888889h-4337.03125v670.277777777778"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ffffff"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]/polypolygon", "path"_ostr,
                u"m2291.92708333333 4550.83333333333h317.34375v-317.5h-317.34375z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]", "color"_ostr, u"#fcf2e3"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[6]/polypolygon", "path"_ostr,
                u"m19428.4895833333 6632.22222222222h317.34375v-2398.88888888889h-317.34375z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[6]", "color"_ostr, u"#fcf2e3"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke", 4);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow", 13);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusSave)
{
    // tdf#147818 EMF+ records: Save, Restore, SetWorldTransform, FillRects, SetClipRegion
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestEmfPlusSave.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h33544v21311h-33544z"_ustr);

    assertXPath(
        pDocument, aXPathPrefix + "mask/group/mask/polypolygoncolor/polypolygon", "path"_ostr,
        u"m327.458333333333 638.222222222222h437007.1875v295555.555555556h-437007.1875z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/group/mask/polypolygoncolor", "color"_ostr,
                u"#ff0cad"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor/polypolygon", "path"_ostr,
                u"m10853.4145539602 7321.41354709201h41952690v29630720h-41952690z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", "color"_ostr, u"#00ffad"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstrokearrow/line", "color"_ostr,
                u"#000000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstrokearrow/polygon",
                       u"10853.4145539602,7321.41354709201 10853.4145539602,4907.54325697157 "
                       "12832.6557236512,4907.54325697157"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusDrawPathWithCustomCap)
{
    // tdf#142261 EMF+ records: DrawPath, SetWorldTransform, Object (Brush, Pen, Path)
    // Check if CustomEndCap is displayed correctly
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawPathWithCustomCap.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPathContent(pDocument, aXPathPrefix + "polygonstrokearrow/polygon",
                       u"1423.297394625,1268.98481214025 705.717657763014,1304.88786195939"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/line", "color"_ostr, u"#cc0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/line", "width"_ostr, u"96"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/line", "linecap"_ostr, u"BUTT"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/stroke", 0);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/linestartattribute", 0);

    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/lineendattribute", "centered"_ostr,
                u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstrokearrow/lineendattribute/polypolygon",
                "path"_ostr, u"m-1.5 3 1.5-3 1.5 3z"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusDrawPathWithMiterLimit)
{
    // tdf#142261 EMF+ records: DrawPath, TranslateWorldTransform, Object (Brush, Pen, Path)
    // Check if Miter is correctly set for Lines
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusDrawPathWithMiterLimit.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke", 3);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "color"_ostr,
                u"#c800c8"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "width"_ostr, u"1058"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "linejoin"_ostr,
                u"Miter"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "miterangle"_ostr,
                u"5"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/stroke", 0);

    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence", 3);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]", "transparence"_ostr,
                u"85"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]/polypolygonstroke/line",
                "color"_ostr, u"#6400c8"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]/polypolygonstroke/line",
                "width"_ostr, u"1058"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]/polypolygonstroke/line",
                "linejoin"_ostr, u"Miter"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[1]/polypolygonstroke/line",
                "miterangle"_ostr, u"19"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[2]", "transparence"_ostr,
                u"69"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[2]/polypolygonstroke/line",
                "miterangle"_ostr, u"19"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[3]", "transparence"_ostr,
                u"53"_ustr);
    assertXPath(pDocument, aXPathPrefix + "unifiedtransparence[3]/polypolygonstroke/line",
                "miterangle"_ostr, u"19"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "color"_ostr,
                u"#0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "width"_ostr, u"1058"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "linejoin"_ostr,
                u"Miter"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "miterangle"_ostr,
                u"60"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/stroke", 0);

    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[3]/line", "color"_ostr,
                u"#0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[3]/line", "width"_ostr, u"1058"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[3]/line", "linejoin"_ostr,
                u"Miter"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[3]/line", "miterangle"_ostr,
                u"60"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[3]/stroke", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testEmfPlusFillClosedCurve)
{
    // tdf#143876 EMF+ records: SetWorldTransform, FillClosedCurve, DrawClosedCurve
    Primitive2DSequence aSequence
        = parseEmf(u"emfio/qa/cppunit/emf/data/TestEmfPlusFillClosedCurve.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 2);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#808080"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]", "color"_ostr, u"#808080"_ustr);

    // Validate curve generated by FillClosedCurve, tension=0.25
    assertXPath(
        pDocument, aXPathPrefix + "polypolygoncolor[2]/polypolygon", "path"_ostr,
        u"m1305.97700269476 "
        "808.572945906987-50.3314857912787-25.4001972536228-68.1076984321412-40.2169789849024-76."
        "5824044585986-48.6837114027768-75.7556038706516-50.8003945072452-65.6272966683-46."
        "5670282983082-46.1974828515433-35.9836127759654-17.4661624203823-19.050147940217 "
        "18.1896129348361-7.40839086563994 47.9544341009307-5.29170776117132 "
        "67.797648211661-3.17502465670293 77.719255267026-1.0583415522342 77.719255267026 "
        "1.0583415522342 67.7976482116608 3.17502465670293 47.9544341009309 5.29170776117132 "
        "18.1896129348359 7.40839086563994-17.4661624203823 19.050147940217-46.1974828515431 "
        "35.9836127759654-65.6272966683 46.5670282983082-75.7556038706516 "
        "50.8003945072452-76.5824044585986 48.6837114027768-68.1076984321412 "
        "40.2169789849024-50.3314857912787 25.4001972536228-23.2537665360119 4.23336620893701 "
        "2.58375183733483-29.7658561565891 18.0862628613424-64.4265419922616 "
        "29.2480707986281-87.1808853652984 36.0691756491915-98.0288862757 "
        "38.5495774130329-96.9705447234655 36.6892760901519-84.0058607085957 "
        "30.4882716805487-59.1348342310902 19.9465641842235-22.3574652909491 19.9465641842235 "
        "22.3574652909491 30.4882716805487 59.1348342310902 36.6892760901519 84.0058607085957 "
        "38.5495774130327 96.9705447234655 36.0691756491917 98.0288862757 29.2480707986281 "
        "87.1808853652984 18.0862628613424 64.4265419922616 2.5837518373346 29.7658561565891z"_ustr);

    // Validate curve generated by DrawClosedCurve, tension=0.25
    assertXPath(
        pDocument, aXPathPrefix + "polypolygonstroke[2]/polypolygon", "path"_ostr,
        u"m1305.97700269476 "
        "808.572945906987-50.3314857912787-25.4001972536228-68.1076984321412-40.2169789849024-76."
        "5824044585986-48.6837114027768-75.7556038706516-50.8003945072452-65.6272966683-46."
        "5670282983082-46.1974828515433-35.9836127759654-17.4661624203823-19.050147940217 "
        "18.1896129348361-7.40839086563994 47.9544341009307-5.29170776117132 "
        "67.797648211661-3.17502465670293 77.719255267026-1.0583415522342 77.719255267026 "
        "1.0583415522342 67.7976482116608 3.17502465670293 47.9544341009309 5.29170776117132 "
        "18.1896129348359 7.40839086563994-17.4661624203823 19.050147940217-46.1974828515431 "
        "35.9836127759654-65.6272966683 46.5670282983082-75.7556038706516 "
        "50.8003945072452-76.5824044585986 48.6837114027768-68.1076984321412 "
        "40.2169789849024-50.3314857912787 25.4001972536228-23.2537665360119 4.23336620893701 "
        "2.58375183733483-29.7658561565891 18.0862628613424-64.4265419922616 "
        "29.2480707986281-87.1808853652984 36.0691756491915-98.0288862757 "
        "38.5495774130329-96.9705447234655 36.6892760901519-84.0058607085957 "
        "30.4882716805487-59.1348342310902 19.9465641842235-22.3574652909491 19.9465641842235 "
        "22.3574652909491 30.4882716805487 59.1348342310902 36.6892760901519 84.0058607085957 "
        "38.5495774130327 96.9705447234655 36.0691756491917 98.0288862757 29.2480707986281 "
        "87.1808853652984 18.0862628613424 64.4265419922616 2.5837518373346 29.7658561565891z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke", 2);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "color"_ostr,
                u"#00ff00"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "width"_ostr, u"4"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "linejoin"_ostr,
                u"Miter"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "miterangle"_ostr,
                u"4"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[1]/line", "linecap"_ostr,
                u"BUTT"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "color"_ostr,
                u"#aaaa00"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygonstroke[2]/line", "width"_ostr, u"4"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testExtTextOutOpaqueAndClipTransform)
{
    // tdf#142495 EMF records: SETBKCOLOR, SELECTOBJECT, EXTTEXTOUTW, MODIFYWORLDTRANSFORM, CREATEFONTINDIRECT.
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/emf/data/TestExtTextOutOpaqueAndClipTransform.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "textsimpleportion", 2);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "text"_ostr,
                u"No_rect- DLP-"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[1]", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[2]", "text"_ostr,
                u"OpaqueTranspa"_ustr);
    assertXPath(pDocument, aXPathPrefix + "textsimpleportion[2]", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 3);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m966 490-477-275-84 147 476 275z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ff0000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m251 713 623 361-148 257-623-361z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]", "color"_ostr, u"#0080ff"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]/polypolygon", "path"_ostr,
                u"m972 1326-476-275-148 257 476 276z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]", "color"_ostr, u"#800080"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group", 3);
    assertXPath(pDocument, aXPathPrefix + "group[1]/polypolygoncolor", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[1]/textsimpleportion", "text"_ostr,
                u"Opaque - DLP-"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[1]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group[2]/mask/group/polypolygoncolor", "color"_ostr,
                u"#00ff00"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[2]/mask/polypolygon", "path"_ostr,
                u"m320 508 586 340-169 293-586-339z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[2]/mask/group/textsimpleportion", "text"_ostr,
                u"Clip  -  DLP-"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[2]/mask/group/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "group[3]/mask/group/polypolygoncolor", "color"_ostr,
                u"#0080ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[3]/mask/polypolygon", "path"_ostr,
                u"m251 713 623 361-148 257-623-361z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[3]/mask/group/textsimpleportion", "text"_ostr,
                u"Opaque ClipP-"_ustr);
    assertXPath(pDocument, aXPathPrefix + "group[3]/mask/group/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testNegativeWinOrg)
{
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestNegativeWinOrg.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    // The crop box (EMR_EXTSELECTCLIPRGN) would not factor in WinOrg coordinates
    // and be lower and more to the right than it actually is which would cut the
    // text in the emf above in half.
    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", 1);
    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", "minx"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", "miny"_ostr,
                u"272"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", "maxx"_ostr,
                u"6800"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/group[1]/mask/polypolygon", "maxy"_ostr,
                u"644"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testBitBltStretchBltWMF)
{
    // tdf#55058 tdf#142722 WMF records: BITBLT, STRETCHBLT.
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestBitBltStretchBlt.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", 2);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy11"_ostr, u"508"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy22"_ostr, u"508"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "xy23"_ostr, u"406"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "height"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]", "width"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]/data", 10);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]/data[1]", "row"_ostr,
                u"000000,000000,000000,000000,000000,000000,000000,000000,000000,000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]/data[4]", "row"_ostr,
                u"000000,ffffff,000000,ffffff,000000,ffffff,000000,ffffff,000000,ffffff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[1]/data[5]", "row"_ostr,
                u"ffffff,000000,ffffff,ffffff,000000,000000,000000,ffffff,ffffff,000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy11"_ostr, u"1524"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy13"_ostr, u"813"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy22"_ostr, u"1016"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "height"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]", "width"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]/data", 10);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]/data[1]", "row"_ostr,
                u"000000,00001c,000038,000055,000071,00008d,0000aa,0000c6,0000e2,0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap[2]/data[5]", "row"_ostr,
                u"720000,721c1c,723838,725555,727171,72728d,55728d,39728d,1d728d,00728d"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testExtTextOutOpaqueAndClipWMF)
{
    // tdf#53004 WMF records: SETBKCOLOR, SELECTOBJECT, EXTTEXTOUT, CREATEBRUSHINDIRECT.
    Primitive2DSequence aSequence
        = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestExtTextOutOpaqueAndClip.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    // On some operating systems (Linux on LO Jenkins CI), the `/mask/` string is not added to XPath
    // As a result tests are failing. On my Ubuntu 20.04 the `/mask/` string was added

    OString aPrefix = aXPathPrefix;
    if (countXPathNodes(pDocument, aXPathPrefix + "mask"))
        aPrefix += "mask/";

    // These values come from the fix for tdf#88163
    assertXPath(pDocument, aPrefix + "polypolygoncolor", 5);
    assertXPath(pDocument, aPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m7257 1836h320v3628h-320z"_ustr);
    assertXPath(pDocument, aPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ff0000"_ustr);

    assertXPath(pDocument, aPrefix + "polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m7257 5976h320v321h-320z"_ustr);
    assertXPath(pDocument, aPrefix + "polypolygoncolor[2]", "color"_ostr, u"#00ff00"_ustr);

    assertXPath(pDocument, aPrefix + "polypolygoncolor[3]/polypolygon", "path"_ostr,
                u"m10203 5976h320v321h-320z"_ustr);
    assertXPath(pDocument, aPrefix + "polypolygoncolor[3]", "color"_ostr, u"#8080ff"_ustr);

    assertXPath(pDocument, aPrefix + "group", 5);
    assertXPath(pDocument, aPrefix + "group[1]/polypolygoncolor", "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDocument, aPrefix + "group[1]/textsimpleportion", "text"_ostr, u"ABCD"_ustr);
    assertXPath(pDocument, aPrefix + "group[1]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aPrefix + "group[2]/polypolygoncolor", "color"_ostr, u"#8080ff"_ustr);
    assertXPath(pDocument, aPrefix + "group[2]/textsimpleportion", "text"_ostr, u"MMMM"_ustr);
    assertXPath(pDocument, aPrefix + "group[2]/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aPrefix + "group[3]/mask/group/polypolygoncolor", "color"_ostr,
                u"#ff8000"_ustr);
    assertXPath(pDocument, aPrefix + "group[3]/mask/group/polypolygoncolor/polypolygon",
                "path"_ostr, u"m1067 1067h1317v473h-1317z"_ustr);
    assertXPath(pDocument, aPrefix + "group[3]/mask/group/textsimpleportion", "text"_ostr,
                u"OOOO"_ustr);
    assertXPath(pDocument, aPrefix + "group[3]/mask/group/textsimpleportion", "fontcolor"_ostr,
                u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPaletteWMF)
{
    // WMF import with records: CREATEPALETTE, SELECTOBJECT, CREATEPENINDIRECT, CREATEBRUSHINDIRECT, ELLIPSE.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestPalette.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor", 2);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m0 0h3015v3015h-3015z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[1]", "color"_ostr,
                u"#ffff00"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m2222 2222h2698v2698h-2698z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polypolygoncolor[2]", "color"_ostr,
                u"#0080ff"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke", 2);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[1]/polygon",
                       u"0,0 3015,0 3015,3015 0,3015"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "width"_ostr, u"132"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[2]/polygon",
                       u"2222,2222 4920,2222 4920,4920 2222,4920"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "width"_ostr, u"132"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRestoreDCWMF)
{
    // WMF records: RESTOREDC, SAVEDC, CREATEBRUSHINDIRECT, RECTANGLE.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestRestoreDC.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 3);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m238 2884h1640v1110h-1640z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[1]", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "polygonhairline[1]/polygon",
                       u"238,2884 1878,2884 1878,3994 238,3994"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]", "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]/polypolygon", "path"_ostr,
                u"m238 238h1640v1110h-1640z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[2]", "color"_ostr, u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]", "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[3]/polypolygon", "path"_ostr,
                u"m238 5530h1640v1110h-1640z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline[3]", "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRoundrectWMF)
{
    // WMF records: ROUNDRECT, SETBKCOLOR, CREATEBRUSHINDIRECT
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestRoundRect.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#ffffff"_ustr);

    assertXPathContent(
        pDocument, aXPathPrefix + "polygonstroke/polygon",
        u"2858,659 2858,651 2858,643 2858,635 2858,619 2858,611 2858,603 2850,595 2850,587 "
        u"2850,580 "
        "2850,564 2850,556 2842,548 2842,540 2842,532 2834,524 2834,516 2834,508 2826,500 2826,492 "
        "2818,484 2818,476 2810,468 2810,460 2802,452 2802,445 2794,437 2794,429 2786,421 2786,421 "
        "2778,413 2770,405 2770,397 2762,389 2754,389 2754,381 2746,373 2738,373 2731,365 2731,365 "
        "2723,357 2715,349 2707,349 2707,341 2699,341 2691,341 2683,333 2675,333 2675,333 2667,325 "
        "2659,325 2651,325 2643,325 2635,318 2627,318 2627,318 2619,318 2611,318 2604,318 572,318 "
        "564,318 556,318 548,318 548,318 540,318 532,325 524,325 516,325 508,325 500,333 500,333 "
        "492,333 484,341 476,341 468,341 468,349 460,349 452,357 445,365 445,365 437,373 429,373 "
        "421,381 421,389 413,389 405,397 405,405 397,413 389,421 389,421 381,429 381,437 373,445 "
        "373,452 365,460 365,468 357,476 357,484 349,492 349,500 341,508 341,516 341,524 333,532 "
        "333,540 333,548 325,556 325,564 325,580 325,587 325,595 318,603 318,611 318,619 318,635 "
        "318,643 318,651 318,659 318,1667 318,1675 318,1683 318,1691 318,1707 318,1715 318,1723 "
        "325,1731 325,1739 325,1746 325,1762 325,1770 333,1778 333,1786 333,1794 341,1802 341,1810 "
        "341,1818 349,1826 349,1834 357,1842 357,1850 365,1858 365,1866 373,1874 373,1881 381,1889 "
        "381,1897 389,1905 389,1905 397,1913 405,1921 405,1929 413,1937 421,1937 421,1945 429,1953 "
        "437,1953 445,1961 445,1961 452,1969 460,1977 468,1977 468,1985 476,1985 484,1985 492,1993 "
        "500,1993 500,1993 508,2001 516,2001 524,2001 532,2001 540,2008 548,2008 548,2008 556,2008 "
        "564,2008 572,2008 2604,2008 2611,2008 2619,2008 2627,2008 2627,2008 2635,2008 2643,2001 "
        "2651,2001 2659,2001 2667,2001 2675,1993 2675,1993 2683,1993 2691,1985 2699,1985 2707,1985 "
        "2707,1977 2715,1977 2723,1969 2731,1961 2731,1961 2738,1953 2746,1953 2754,1945 2754,1937 "
        "2762,1937 2770,1929 2770,1921 2778,1913 2786,1905 2786,1905 2794,1897 2794,1889 2802,1881 "
        "2802,1874 2810,1866 2810,1858 2818,1850 2818,1842 2826,1834 2826,1826 2834,1818 2834,1810 "
        "2834,1802 2842,1794 2842,1786 2842,1778 2850,1770 2850,1762 2850,1746 2850,1739 2850,1731 "
        "2858,1723 2858,1715 2858,1707 2858,1691 2858,1683 2858,1675 2858,1667"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "width"_ostr, u"143"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testStretchDIBWMF)
{
    // WMF records: STRETCHDIB
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestStretchDIB.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy11"_ostr, u"12065"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy22"_ostr, u"12065"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "xy23"_ostr, u"0"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "height"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap", "width"_ostr, u"10"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/bitmap/data", 10);
    assertXPath(pDocument, aXPathPrefix + "/mask/bitmap/data[1]", "row"_ostr,
                u"000000,00001c,000038,000055,000071,00008d,0000aa,0000c6,0000e2,0000ff"_ustr);
    assertXPath(pDocument, aXPathPrefix + "/mask/bitmap/data[5]", "row"_ostr,
                u"720000,721c1c,723838,725555,727171,72728d,55728d,39728d,1d728d,00728d"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testMoveToLineToWMF)
{
    // tdf#89331 WMF records: MOTETO, LINETO, CREATEPENINDIRECT.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/wmf/data/TestLineTo.wmf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke/polygon",
                       u"5856,3586 7167,621 8625,3586"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "color"_ostr, u"#800000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "width"_ostr, u"310"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "linejoin"_ostr, u"Bevel"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "linecap"_ostr, u"ROUND"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPolyLineWidth)
{
    // EMF import with records: CREATEPEN, ROUNDRECT.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestPolyLineWidth.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor/polypolygon", "path"_ostr,
                u"m530 529 1236-176-707 352z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#ffff00"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke/polygon",
                       u"530,529 530,529 1766,353 1059,705"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke/line", "width"_ostr, u"71"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRestoreDC)
{
    // EMF records: SAVEDC, RESTOREDC, POLYGON16, MODIFYWORLDTRANSFORM
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestRestoreDC.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor/polypolygon", "path"_ostr,
                u"m1148 4354v1481h4943v-1481z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonhairline", "color"_ostr, u"#000000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "polygonhairline/polygon",
                       u"1148,4354 1148,5835 6091,5835 6091,4354"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRoundRect)
{
    // EMF import with records: CREATEPEN, ROUNDRECT.
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestRoundRect.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor", 2);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]/polypolygon", "path"_ostr,
                u"m100 100h4000v2000h-4000z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[1]", "color"_ostr, u"#ffffff"_ustr);

    assertXPath(pDocument, aXPathPrefix + "polygonstroke", 2);
    assertXPathContent(pDocument, aXPathPrefix + "polygonstroke[1]/polygon",
                       u"100,100 4100,100 4100,2100 100,2100"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[1]/line", "color"_ostr, u"#ff0000"_ustr);

    assertXPath(
        pDocument, aXPathPrefix + "polypolygoncolor[2]/polypolygon", "path"_ostr,
        u"m4090 "
        "2700v-30-20l-10-30v-20l-10-30-10-20-10-20-20-30-10-20-20-20-20-20-20-30-20-20-20-20-20-10-"
        "30-20-20-20-30-20-30-10-30-10-30-20-30-10-30-10-40-10-30-10h-30l-40-10h-30l-40-10h-30-40-"
        "2590-40-30l-40 10h-30l-40 10h-30l-30 10-40 10-30 10-30 10-30 20-30 10-30 10-30 20-20 "
        "20-30 20-20 10-20 20-20 20-20 30-20 20-20 20-10 20-20 30-10 20-10 20-10 30v20l-10 30v20 "
        "30 990 30 20l10 30v20l10 30 10 20 10 20 20 30 10 20 20 20 20 20 20 30 20 20 20 20 20 10 "
        "30 20 20 20 30 20 30 10 30 10 30 20 30 10 30 10 40 10 30 10h30l40 10h30l40 10h30 40 2590 "
        "40 30l40-10h30l40-10h30l30-10 40-10 30-10 30-10 30-20 30-10 30-10 30-20 20-20 30-20 20-10 "
        "20-20 20-20 20-30 20-20 20-20 10-20 20-30 10-20 10-20 10-30v-20l10-30v-20-30z"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polypolygoncolor[2]", "color"_ostr, u"#ffffff"_ustr);

    assertXPathContent(
        pDocument, aXPathPrefix + "polygonstroke[2]/polygon",
        u"4090,2700 4090,2670 4090,2650 4080,2620 4080,2600 4070,2570 4060,2550 4050,2530 "
        u"4030,2500 "
        "4020,2480 4000,2460 3980,2440 3960,2410 3940,2390 3920,2370 3900,2360 3870,2340 3850,2320 "
        "3820,2300 3790,2290 3760,2280 3730,2260 3700,2250 3670,2240 3630,2230 3600,2220 3570,2220 "
        "3530,2210 3500,2210 3460,2200 3430,2200 3390,2200 800,2200 760,2200 730,2200 690,2210 "
        "660,2210 620,2220 590,2220 560,2230 520,2240 490,2250 460,2260 430,2280 400,2290 370,2300 "
        "340,2320 320,2340 290,2360 270,2370 250,2390 230,2410 210,2440 190,2460 170,2480 160,2500 "
        "140,2530 130,2550 120,2570 110,2600 110,2620 100,2650 100,2670 100,2700 100,3690 100,3720 "
        "100,3740 110,3770 110,3790 120,3820 130,3840 140,3860 160,3890 170,3910 190,3930 210,3950 "
        "230,3980 250,4000 270,4020 290,4030 320,4050 340,4070 370,4090 400,4100 430,4110 460,4130 "
        "490,4140 520,4150 560,4160 590,4170 620,4170 660,4180 690,4180 730,4190 760,4190 800,4190 "
        "3390,4190 3430,4190 3460,4190 3500,4180 3530,4180 3570,4170 3600,4170 3630,4160 3670,4150 "
        "3700,4140 3730,4130 3760,4110 3790,4100 3820,4090 3850,4070 3870,4050 3900,4030 3920,4020 "
        "3940,4000 3960,3980 3980,3950 4000,3930 4020,3910 4030,3890 4050,3860 4060,3840 4070,3820 "
        "4080,3790 4080,3770 4090,3740 4090,3720 4090,3690"_ustr);
    assertXPath(pDocument, aXPathPrefix + "polygonstroke[2]/line", "color"_ostr, u"#ff0000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testCreatePen)
{
    // Check import of EMF image with records: RESTOREDC, SAVEDC, MOVETOEX, LINETO, POLYLINE16, EXTTEXTOUTW with DxBuffer
    // The CREATEPEN record is used with PS_COSMETIC line style, and in this case width must be set to 0
    Primitive2DSequence aSequence = parseEmf(u"/emfio/qa/cppunit/emf/data/TestCreatePen.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, aXPathPrefix + "mask/polypolygon", "path"_ostr,
                u"m0 0h31250v18192h-31250z"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke", 3);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[1]/polygon",
                       u"17898,5693 20172,5693"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "color"_ostr,
                u"#008000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[1]/line", "width"_ostr, u"0"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[2]/polygon",
                       u"17898,6959 20172,6959"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "color"_ostr,
                u"#000080"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[2]/line", "width"_ostr, u"0"_ustr);

    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonstroke[3]/polygon",
                       u"17898,7381 20172,7381"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[3]/line", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonstroke[3]/line", "width"_ostr, u"0"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline", 755);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline[5]", "color"_ostr, u"#ff0000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[5]/polygon",
                       u"27925,16078 27875,16078"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/polygonhairline[10]", "color"_ostr,
                u"#ff0000"_ustr);
    assertXPathContent(pDocument, aXPathPrefix + "mask/polygonhairline[10]/polygon",
                       u"27925,14180 27875,14180"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion", 69);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "width"_ostr, u"374"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "x"_ostr, u"28124"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "y"_ostr, u"16581"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "text"_ostr, u"0.0"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[1]", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[10]", "width"_ostr, u"266"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[10]", "x"_ostr, u"28000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[10]", "y"_ostr, u"428"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[10]", "text"_ostr, u"-6"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/textsimpleportion[10]", "fontcolor"_ostr,
                u"#000000"_ustr);

    assertXPath(pDocument, aXPathPrefix + "mask/pointarray", 8);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]", "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]/point", "x"_ostr, u"25844"_ustr);
    assertXPath(pDocument, aXPathPrefix + "mask/pointarray[1]/point", "y"_ostr, u"8918"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPdfInEmf)
{
    if (!vcl::pdf::PDFiumLibrary::get())
    {
        return;
    }

    // Load a PPTX file, which has a shape, with a bitmap fill, which is an EMF, containing a PDF.
    loadFromFile(u"pdf-in-emf.pptx");

    // Get the EMF.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"FillBitmap"_ustr) >>= xGraphic;
    Graphic aGraphic(xGraphic);

    // Check the size hint of the EMF, which influences the bitmap generated from the PDF.
    const std::shared_ptr<VectorGraphicData>& pVectorGraphicData = aGraphic.getVectorGraphicData();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14321
    // - Actual  : 0
    // i.e. there was no size hint, the shape with 14cm height had a bitmap-from-PDF fill, the PDF
    // height was only 5cm, so it looked blurry.
    // Tolerance was added later based on results on different systems.
    CPPUNIT_ASSERT_LESSEQUAL(1.0, abs(14321.0 - pVectorGraphicData->getSizeHint().getY()));

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 255
    // i.e. the pixel in the center was entirely opaque, while it should be transparent.
    BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
    Size size = aBitmapEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0),
                         aBitmapEx.GetAlpha(size.Width() / 2, size.Height() / 2));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
