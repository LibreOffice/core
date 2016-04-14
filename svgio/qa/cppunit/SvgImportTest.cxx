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
#include <test/primitive2dxmldump.hxx>
#include <test/xmltesttools.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <memory>

namespace
{

using namespace css::uno;
using namespace css::io;
using namespace css::graphic;
using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DContainer;

class Test : public test::BootstrapFixture, public XmlTestTools
{
    void checkRectPrimitive(Primitive2DSequence& rPrimitive);

    void testStyles();
    void testTdf87309();
    void testFontsizeKeywords();
    void testFontsizePercentage();
    void testFontsizeRelative();
    void testTdf45771();
    void testTdf97941();
    void testTdf85770();
    void testTdf79163();
    void testTdf97542_1();
    void testTdf97542_2();
    void testTdf97543();
    void testRGBColor();
    void testRGBAColor();
    void testTdf97936();
    void testClipPathAndParentStyle();
    void testClipPathAndStyle();
    void testi125329();
    void testMaskingPath07b();
    void test47446();
    void test47446b();
    void testMaskText();

    Primitive2DSequence parseSvg(const char* aSource);

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStyles);
    CPPUNIT_TEST(testTdf87309);
    CPPUNIT_TEST(testFontsizeKeywords);
    CPPUNIT_TEST(testFontsizePercentage);
    CPPUNIT_TEST(testFontsizeRelative);
    CPPUNIT_TEST(testTdf45771);
    CPPUNIT_TEST(testTdf97941);
    CPPUNIT_TEST(testTdf85770);
    CPPUNIT_TEST(testTdf79163);
    CPPUNIT_TEST(testTdf97542_1);
    CPPUNIT_TEST(testTdf97542_2);
    CPPUNIT_TEST(testTdf97543);
    CPPUNIT_TEST(testRGBColor);
    CPPUNIT_TEST(testRGBAColor);
    CPPUNIT_TEST(testTdf97936);
    CPPUNIT_TEST(testClipPathAndParentStyle);
    CPPUNIT_TEST(testClipPathAndStyle);
    CPPUNIT_TEST(testi125329);
    CPPUNIT_TEST(testMaskingPath07b);
    CPPUNIT_TEST(test47446);
    CPPUNIT_TEST(test47446b);
    CPPUNIT_TEST(testMaskText);
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseSvg(const char* aSource)
{
    const Reference<XSvgParser> xSvgParser = SvgTools::create(m_xContext);

    OUString aUrl  = m_directories.getURLFromSrc(aSource);
    OUString aPath = m_directories.getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, StreamMode::READ);
    sal_Size nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.Read(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    return xSvgParser->getDecomposition(aInputStream, aPath);
}

void Test::setUp()
{
    BootstrapFixture::setUp();
}

void Test::tearDown()
{
    BootstrapFixture::tearDown();
}

void Test::checkRectPrimitive(Primitive2DSequence& rPrimitive)
{
    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxy", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "3"); // rect stroke width


}


bool arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB)
{
    const sal_Int32 nCount(rA.getLength());

    if(nCount != rB.getLength())
        return false;

    for(sal_Int32 a(0L); a < nCount; a++) {
        if(!drawinglayer::primitive2d::arePrimitive2DReferencesEqual(rA[a], rB[a]))
            return false;
    }

    return true;
}

// Attributes for an object (like rect as in this case) can be defined
// in different ways (directly with xml attributes, or with CSS styles),
// however the end result should be the same.
void Test::testStyles()
{
    Primitive2DSequence aSequenceRect = parseSvg("/svgio/qa/cppunit/data/Rect.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRect.getLength());
    checkRectPrimitive(aSequenceRect);

    Primitive2DSequence aSequenceRectWithStyle = parseSvg("/svgio/qa/cppunit/data/RectWithStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithStyle.getLength());
    checkRectPrimitive(aSequenceRectWithStyle);

    Primitive2DSequence aSequenceRectWithParentStyle = parseSvg("/svgio/qa/cppunit/data/RectWithParentStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithParentStyle.getLength());
    checkRectPrimitive(aSequenceRectWithParentStyle);

    Primitive2DSequence aSequenceRectWithStylesByGroup = parseSvg("/svgio/qa/cppunit/data/RectWithStylesByGroup.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithStylesByGroup.getLength());
    checkRectPrimitive(aSequenceRectWithStylesByGroup);

    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithParentStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStylesByGroup));
}

void Test::testTdf87309()
{
    Primitive2DSequence aSequenceTdf87309 = parseSvg("/svgio/qa/cppunit/data/tdf87309.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf87309.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf87309));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxy", "110");
}

void Test::testFontsizeKeywords()
{
    Primitive2DSequence aSequenceFontsizeKeywords = parseSvg("/svgio/qa/cppunit/data/FontsizeKeywords.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceFontsizeKeywords.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceFontsizeKeywords));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "height", "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "height", "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "height", "23");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "height", "27");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "height", "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "height", "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "height", "16");
}


void Test::testFontsizePercentage()
{
    //Check when font-size uses percentage and defined globally
    Primitive2DSequence aSequenceFontsizePercentage = parseSvg("/svgio/qa/cppunit/data/FontsizePercentage.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceFontsizePercentage.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceFontsizePercentage));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "16");
}

void Test::testFontsizeRelative()
{
    //Check when font-size uses relative units (em,ex) and it's based on its parent's font-size
    Primitive2DSequence aSequenceFontsizeRelative = parseSvg("/svgio/qa/cppunit/data/FontsizeRelative.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceFontsizeRelative.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceFontsizeRelative));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "50");
}

void Test::testTdf45771()
{
    //Check text fontsize when using relative units
    Primitive2DSequence aSequenceTdf45771 = parseSvg("/svgio/qa/cppunit/data/tdf45771.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf45771.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf45771));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "32");
}

void Test::testTdf97941()
{
    //Check tspan fontsize when using relative units
    Primitive2DSequence aSequenceTdf97941 = parseSvg("/svgio/qa/cppunit/data/tdf97941.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf97941.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf97941));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", "48");
}

void Test::testTdf85770()
{
    Primitive2DSequence aSequenceTdf85770 = parseSvg("/svgio/qa/cppunit/data/tdf85770.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf85770.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf85770));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", "Start Middle End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", "Start ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", "End");
}

void Test::testTdf79163()
{
    //Check Opacity
    Primitive2DSequence aSequenceTdf79163 = parseSvg("/svgio/qa/cppunit/data/tdf79163.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf79163.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf79163));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", "0");
}

void Test::testTdf97542_1()
{
    Primitive2DSequence aSequenceTdf97542_1 = parseSvg("/svgio/qa/cppunit/data/tdf97542_1.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf97542_1.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf97542_1));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "fontcolor", "#ffff00");
}

void Test::testTdf97542_2()
{
    Primitive2DSequence aSequenceTdf97542_2 = parseSvg("/svgio/qa/cppunit/data/tdf97542_2.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf97542_2.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf97542_2));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "x", "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "y", "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient[1]", "radius", "3");
}

void Test::testTdf97543()
{
    // check visibility="inherit"
    Primitive2DSequence aSequenceTdf97543 = parseSvg("/svgio/qa/cppunit/data/tdf97543.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf97543.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf97543));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxy", "110");
}

void Test::testRGBColor()
{
    Primitive2DSequence aSequenceRGBColor = parseSvg("/svgio/qa/cppunit/data/RGBColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceRGBColor.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceRGBColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#646464");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "miny", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxx", "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxy", "110");
}

void Test::testRGBAColor()
{
    Primitive2DSequence aSequenceRGBAColor = parseSvg("/svgio/qa/cppunit/data/RGBAColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceRGBAColor.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceRGBAColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", "0");
}

void Test::testTdf97936()
{
    // check that both rectangles are rendered in the viewBox
    Primitive2DSequence aSequenceTdf97936 = parseSvg("/svgio/qa/cppunit/data/tdf97936.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceTdf97936.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceTdf97936));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "width", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "minx", "70");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "miny", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "maxx", "120");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "maxy", "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "height", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "width", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "minx", "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "miny", "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "maxx", "60");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "maxy", "100");
}

void Test::testClipPathAndParentStyle()
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndParentStyle = parseSvg("/svgio/qa/cppunit/data/ClipPathAndParentStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceClipPathAndParentStyle.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceClipPathAndParentStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "5");

}

void Test::testClipPathAndStyle()
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg("/svgio/qa/cppunit/data/ClipPathAndStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceClipPathAndStyle.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#0000cc");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "2");

}

void Test::testi125329()
{
    //Check style inherit from * css element
    Primitive2DSequence aSequencei125329 = parseSvg("/svgio/qa/cppunit/data/i125329.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequencei125329.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequencei125329));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", "#c0c0c0"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "height", "30"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "width", "50"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "minx", "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "miny", "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "maxx", "65");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "maxy", "45");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "color", "#008000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "width", "1"); // rect stroke width
}

void Test::testMaskingPath07b()
{
    //For the time being, check that masking-path-07-b.svg can be imported and it doesn't hang on loading
    //it used to hang after d5649ae7b76278cb3155f951d6327157c7c92b65
    Primitive2DSequence aSequenceMaskingPath07b = parseSvg("/svgio/qa/cppunit/data/masking-path-07-b.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceMaskingPath07b.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceMaskingPath07b));

    CPPUNIT_ASSERT (pDocument);

}

void Test::test47446()
{
    //Check that marker's fill attribute is black is not set
    Primitive2DSequence aSequence47446 = parseSvg("/svgio/qa/cppunit/data/47446.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequence47446.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence47446));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", "#000000");

}

void Test::test47446b()
{
    //Check that marker's fill attribute is inherit from def
    Primitive2DSequence aSequence47446b = parseSvg("/svgio/qa/cppunit/data/47446b.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequence47446b.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence47446b));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", "#ffff00");

}

void Test::testMaskText()
{
    //Check that mask is applied on text
    Primitive2DSequence aSequenceMaskText = parseSvg("/svgio/qa/cppunit/data/maskText.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceMaskText.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceMaskText));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion", "text", "Black White");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
