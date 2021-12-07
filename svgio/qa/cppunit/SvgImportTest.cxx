/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>

#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>

#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>

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
using drawinglayer::primitive2d::Primitive2DReference;

class Test : public test::BootstrapFixture, public XmlTestTools
{
    void checkRectPrimitive(Primitive2DSequence const & rPrimitive);

    void testStyles();
    void testTdf87309();
    void testFontsizeKeywords();
    void testFontsizePercentage();
    void testFontsizeRelative();
    void testTdf45771();
    void testTdf97941();
    void testTdf104339();
    void testTdf85770();
    void testTdf79163();
    void testTdf97542_1();
    void testTdf97542_2();
    void testTdf97543();
    void testRGBColor();
    void testRGBAColor();
    void testNoneColor();
    void testTdf97936();
    void testClipPathAndParentStyle();
    void testClipPathAndStyle();
    void testi125329();
    void testMaskingPath07b();
    void test123926();
    void test47446();
    void test47446b();
    void testMaskText();
    void testTdf99994();
    void testTdf99115();
    void testTdf101237();
    void testTdf94765();
    void testBehaviourWhenWidthAndHeightIsOrIsNotSet();

    Primitive2DSequence parseSvg(std::u16string_view aSource);

public:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStyles);
    CPPUNIT_TEST(testTdf87309);
    CPPUNIT_TEST(testFontsizeKeywords);
    CPPUNIT_TEST(testFontsizePercentage);
    CPPUNIT_TEST(testFontsizeRelative);
    CPPUNIT_TEST(testTdf45771);
    CPPUNIT_TEST(testTdf97941);
    CPPUNIT_TEST(testTdf104339);
    CPPUNIT_TEST(testTdf85770);
    CPPUNIT_TEST(testTdf79163);
    CPPUNIT_TEST(testTdf97542_1);
    CPPUNIT_TEST(testTdf97542_2);
    CPPUNIT_TEST(testTdf97543);
    CPPUNIT_TEST(testRGBColor);
    CPPUNIT_TEST(testRGBAColor);
    CPPUNIT_TEST(testNoneColor);
    CPPUNIT_TEST(testTdf97936);
    CPPUNIT_TEST(testClipPathAndParentStyle);
    CPPUNIT_TEST(testClipPathAndStyle);
    CPPUNIT_TEST(testi125329);
    CPPUNIT_TEST(testMaskingPath07b);
    CPPUNIT_TEST(test123926);
    CPPUNIT_TEST(test47446);
    CPPUNIT_TEST(test47446b);
    CPPUNIT_TEST(testMaskText);
    CPPUNIT_TEST(testTdf99994);
    CPPUNIT_TEST(testTdf99115);
    CPPUNIT_TEST(testTdf101237);
    CPPUNIT_TEST(testTdf94765);
    CPPUNIT_TEST(testBehaviourWhenWidthAndHeightIsOrIsNotSet);
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseSvg(std::u16string_view aSource)
{
    const Reference<XSvgParser> xSvgParser = SvgTools::create(m_xContext);

    OUString aUrl  = m_directories.getURLFromSrc(aSource);
    OUString aPath = m_directories.getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    return xSvgParser->getDecomposition(aInputStream, aPath);
}

void Test::checkRectPrimitive(Primitive2DSequence const & rPrimitive)
{
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", "100"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", "100"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "3"); // rect stroke width


}

bool arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB)
{
    return std::equal(rA.begin(), rA.end(), rB.begin(), rB.end(),
        [](const css::uno::Reference<css::graphic::XPrimitive2D>& a,
           const css::uno::Reference<css::graphic::XPrimitive2D>& b)
        {
            return drawinglayer::primitive2d::arePrimitive2DReferencesEqual(a, b);
        });
}

// Attributes for an object (like rect as in this case) can be defined
// in different ways (directly with xml attributes, or with CSS styles),
// however the end result should be the same.
void Test::testStyles()
{
    Primitive2DSequence aSequenceRect = parseSvg(u"/svgio/qa/cppunit/data/Rect.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRect.getLength()));
    checkRectPrimitive(aSequenceRect);

    Primitive2DSequence aSequenceRectWithStyle = parseSvg(u"/svgio/qa/cppunit/data/RectWithStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithStyle.getLength()));
    checkRectPrimitive(aSequenceRectWithStyle);

    Primitive2DSequence aSequenceRectWithParentStyle = parseSvg(u"/svgio/qa/cppunit/data/RectWithParentStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithParentStyle.getLength()));
    checkRectPrimitive(aSequenceRectWithParentStyle);

    Primitive2DSequence aSequenceRectWithStylesByGroup = parseSvg(u"/svgio/qa/cppunit/data/RectWithStylesByGroup.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithStylesByGroup.getLength()));
    checkRectPrimitive(aSequenceRectWithStylesByGroup);

    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithParentStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStylesByGroup));
}

void Test::testTdf87309()
{
    Primitive2DSequence aSequenceTdf87309 = parseSvg(u"/svgio/qa/cppunit/data/tdf87309.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf87309.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf87309);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", "110");
}

void Test::testFontsizeKeywords()
{
    Primitive2DSequence aSequenceFontsizeKeywords = parseSvg(u"/svgio/qa/cppunit/data/FontsizeKeywords.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizeKeywords.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizeKeywords);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", "#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", "#ffd700");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "fontcolor", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "height", "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "fontcolor", "#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "height", "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "height", "23");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "fontcolor", "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "height", "27");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "fontcolor", "#ff7f50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "height", "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "fontcolor", "#ffc0cb");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "height", "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "familyname", "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "fontcolor", "#fffff0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "height", "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "familyname", "Times New Roman");
}


void Test::testFontsizePercentage()
{
    //Check when font-size uses percentage and defined globally
    Primitive2DSequence aSequenceFontsizePercentage = parseSvg(u"/svgio/qa/cppunit/data/FontsizePercentage.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizePercentage.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizePercentage);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Times New Roman");
}

void Test::testFontsizeRelative()
{
    //Check when font-size uses relative units (em,ex) and it's based on its parent's font-size
    Primitive2DSequence aSequenceFontsizeRelative = parseSvg(u"/svgio/qa/cppunit/data/FontsizeRelative.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizeRelative.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizeRelative);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "serif");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", "serif");
}

void Test::testTdf45771()
{
    //Check text fontsize when using relative units
    Primitive2DSequence aSequenceTdf45771 = parseSvg(u"/svgio/qa/cppunit/data/tdf45771.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf45771.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf45771);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Times New Roman");
}

void Test::testTdf97941()
{
    //Check tspan fontsize when using relative units
    Primitive2DSequence aSequenceTdf97941 = parseSvg(u"/svgio/qa/cppunit/data/tdf97941.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97941.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf97941);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "48");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Times New Roman");
}

void Test::testTdf104339()
{
    Primitive2DSequence aSequenceTdf104339 = parseSvg(u"/svgio/qa/cppunit/data/tdf104339.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf104339.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf104339);

    CPPUNIT_ASSERT (pDocument);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/transform/transform/polypolygoncolor", "color", "#000000");
}

void Test::testTdf85770()
{
    Primitive2DSequence aSequenceTdf85770 = parseSvg(u"/svgio/qa/cppunit/data/tdf85770.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf85770.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf85770));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Start Middle End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", "Start ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", "Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", "End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "familyname", "Times New Roman");

}

void Test::testTdf79163()
{
    //Check Opacity
    Primitive2DSequence aSequenceTdf79163 = parseSvg(u"/svgio/qa/cppunit/data/tdf79163.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf79163.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf79163));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", "50");
}

void Test::testTdf97542_1()
{
    Primitive2DSequence aSequenceTdf97542_1 = parseSvg(u"/svgio/qa/cppunit/data/tdf97542_1.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97542_1.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97542_1));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "fontcolor", "#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "text", "Text");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "height", "48");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "familyname", "serif");
}

void Test::testTdf97542_2()
{
    Primitive2DSequence aSequenceTdf97542_2 = parseSvg(u"/svgio/qa/cppunit/data/tdf97542_2.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97542_2.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97542_2));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "focusx", "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "focusy", "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "radius", "3");
}

void Test::testTdf97543()
{
    // check visibility="inherit"
    Primitive2DSequence aSequenceTdf97543 = parseSvg(u"/svgio/qa/cppunit/data/tdf97543.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97543.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97543));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", "110");
}

void Test::testRGBColor()
{
    Primitive2DSequence aSequenceRGBColor = parseSvg(u"/svgio/qa/cppunit/data/RGBColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#646464");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", "110");
}

void Test::testRGBAColor()
{
    Primitive2DSequence aSequenceRGBAColor = parseSvg(u"/svgio/qa/cppunit/data/RGBAColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBAColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBAColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", "50");
}

void Test::testNoneColor()
{
    Primitive2DSequence aSequenceRGBAColor = parseSvg(u"/svgio/qa/cppunit/data/noneColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBAColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBAColor));

    CPPUNIT_ASSERT (pDocument);

    //No polypolygoncolor exists
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor", 0);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "width", "3");
}

void Test::testTdf97936()
{
    // check that both rectangles are rendered in the viewBox
    Primitive2DSequence aSequenceTdf97936 = parseSvg(u"/svgio/qa/cppunit/data/tdf97936.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97936.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97936));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "width", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "minx", "70");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "miny", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "maxx", "120");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "maxy", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "width", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "miny", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "maxx", "60");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "maxy", "100");
}

void Test::testClipPathAndParentStyle()
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndParentStyle = parseSvg(u"/svgio/qa/cppunit/data/ClipPathAndParentStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndParentStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndParentStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "5");

}

void Test::testClipPathAndStyle()
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg(u"/svgio/qa/cppunit/data/ClipPathAndStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#0000cc");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "2");

}

void Test::testi125329()
{
    //Check style inherit from * css element
    Primitive2DSequence aSequencei125329 = parseSvg(u"/svgio/qa/cppunit/data/i125329.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequencei125329.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequencei125329));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor", "color", "#c0c0c0"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "height", "30"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "width", "50"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "minx", "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "miny", "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "maxx", "65");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "maxy", "45");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line", "color", "#008000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line", "width", "1"); // rect stroke width
}

void Test::testMaskingPath07b()
{
    //For the time being, check that masking-path-07-b.svg can be imported and it doesn't hang on loading
    //it used to hang after d5649ae7b76278cb3155f951d6327157c7c92b65
    Primitive2DSequence aSequenceMaskingPath07b = parseSvg(u"/svgio/qa/cppunit/data/masking-path-07-b.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceMaskingPath07b.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceMaskingPath07b));

    CPPUNIT_ASSERT (pDocument);

}

void Test::test123926()
{
    Primitive2DSequence aSequence123926 = parseSvg(u"/svgio/qa/cppunit/data/tdf123926.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence123926.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence123926));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/unifiedtransparence/polypolygoncolor", "color", "#7cb5ec");
}

void Test::test47446()
{
    //Check that marker's fill attribute is black is not set
    Primitive2DSequence aSequence47446 = parseSvg(u"/svgio/qa/cppunit/data/47446.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence47446.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence47446));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", "#000000");

}

void Test::test47446b()
{
    //Check that marker's fill attribute is inherit from def
    Primitive2DSequence aSequence47446b = parseSvg(u"/svgio/qa/cppunit/data/47446b.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence47446b.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence47446b));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", "#ffff00");

}

void Test::testMaskText()
{
    //Check that mask is applied on text
    Primitive2DSequence aSequenceMaskText = parseSvg(u"/svgio/qa/cppunit/data/maskText.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceMaskText.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceMaskText));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion", "fontcolor", "#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion", "text", "Black White");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion", "height", "26");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion", "familyname", "Times New Roman");
}

void Test::testTdf99994()
{
    //Check text fontsize when using relative units
    Primitive2DSequence aSequenceTdf99994 = parseSvg(u"/svgio/qa/cppunit/data/tdf99994.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf99994.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf99994));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "test");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", "Sans");
}

void Test::testTdf99115()
{
    //Check that styles are resolved correctly where there is a * css selector
    Primitive2DSequence aSequenceTdf99115 = parseSvg(u"/svgio/qa/cppunit/data/tdf99115.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf99115.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf99115) );

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "red 1");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", "red 2");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", "red 3");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", "blue 4");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", "blue 5");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", "blue 6");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "fontcolor", "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", "green 7");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "fontcolor", "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", "green 8");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "fontcolor", "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "height", "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", "green 9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "fontcolor", "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "height", "18");
}

void Test::testTdf101237()
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceTdf101237 = parseSvg(u"/svgio/qa/cppunit/data/tdf101237.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf101237.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf101237));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "5");
}

void Test::testTdf94765()
{
    Primitive2DSequence aSequenceTdf94765 = parseSvg(u"/svgio/qa/cppunit/data/tdf94765.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf94765.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf94765));

    CPPUNIT_ASSERT (pDocument);

    //Check that both rectangles use the gradient as fill
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "endx", "2");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "endy", "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "endx", "0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "endy", "0");
}

void Test::testBehaviourWhenWidthAndHeightIsOrIsNotSet()
{
    // This test checks the behaviour when width and height attributes
    // are and are not set. In both cases the result must be the same,
    // however if the width / height are set, then the size of the image
    // is enforced, but this isn't really possible in LibreOffice (or
    // maybe we could lock the size in this case).
    // The behaviour in browsers is that when a SVG image has width / height
    // attributes set, then the image is shown with that size, but if it
    // isn't set then it is shown as scalable image which is the size of
    // the container.

    {
        const Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_WithWidthHeight.svg");
        CPPUNIT_ASSERT(aSequence.hasElements());

        geometry::RealRectangle2D aRealRect;
        basegfx::B2DRange aRange;
        uno::Sequence<beans::PropertyValue> aViewParameters;

        for (css::uno::Reference<css::graphic::XPrimitive2D> const & xReference : aSequence)
        {
            if (xReference.is())
            {
                aRealRect = xReference->getRange(aViewParameters);
                aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
            }
        }

        double fWidth = (aRange.getWidth() / 2540.0) * 96.0;
        double fHeight = (aRange.getHeight() / 2540.0) * 96.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fWidth, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fHeight, 1E-12);
    }

    {
        const Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_NoWidthHeight.svg");
        CPPUNIT_ASSERT(aSequence.hasElements());


        geometry::RealRectangle2D aRealRect;
        basegfx::B2DRange aRange;
        uno::Sequence<beans::PropertyValue> aViewParameters;

        for (css::uno::Reference<css::graphic::XPrimitive2D> const & xReference : aSequence)
        {
            if (xReference.is())
            {
                aRealRect = xReference->getRange(aViewParameters);
                aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
            }
        }

        double fWidth = (aRange.getWidth() / 2540.0) * 96.0;
        double fHeight = (aRange.getHeight() / 2540.0) * 96.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fWidth, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fHeight, 1E-12);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
