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
using drawinglayer::primitive2d::arePrimitive2DSequencesEqual;
using drawinglayer::primitive2d::Primitive2DSequence;

class Test : public test::BootstrapFixture, public XmlTestTools
{
    void checkRectPrimitive(Primitive2DSequence& rPrimitive);

    void testStyles();
    void testTdf87309();

    Primitive2DSequence parseSvg(const char* aSource);

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStyles);
    CPPUNIT_TEST(testTdf87309);
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
    xmlDocPtr pDocument = dumper.dumpAndParse(rPrimitive);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "3"); // rect stroke width

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
    xmlDocPtr pDocument = dumper.dumpAndParse(aSequenceTdf87309);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#000000");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
