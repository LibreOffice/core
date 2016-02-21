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
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseSvg(const char* aSource)
{
    const Reference<XSvgParser> xSvgParser = SvgTools::create(m_xContext);

    OUString aUrl  = getURLFromSrc(aSource);
    OUString aPath = getPathFromSrc(aSource);

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
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "3"); // rect stroke width

}


static bool arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB)
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
}

void Test::testRGBColor()
{
    Primitive2DSequence aSequenceRGBColor = parseSvg("/svgio/qa/cppunit/data/RGBColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int)aSequenceRGBColor.getLength());

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequenceRGBColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#646464");
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
