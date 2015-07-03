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

class Test : public test::BootstrapFixture, public XmlTestTools
{
    void checkRectPrimitive(css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > >& rPrimitive);

    void testStyles();

    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > parseSvg(const char* aSource);

public:
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStyles);
    CPPUNIT_TEST_SUITE_END();
};

css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > Test::parseSvg(const char* aSource)
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

void Test::checkRectPrimitive(css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > >& rPrimitive)
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
    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aSequenceRect = parseSvg("/svgio/qa/cppunit/data/Rect.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRect.getLength());
    checkRectPrimitive(aSequenceRect);

    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aSequenceRectWithStyle = parseSvg("/svgio/qa/cppunit/data/RectWithStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithStyle.getLength());
    checkRectPrimitive(aSequenceRectWithStyle);

    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aSequenceRectWithParentStyle = parseSvg("/svgio/qa/cppunit/data/RectWithParentStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithParentStyle.getLength());
    checkRectPrimitive(aSequenceRectWithParentStyle);

    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aSequenceRectWithStylesByGroup = parseSvg("/svgio/qa/cppunit/data/RectWithStylesByGroup.svg");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRectWithStylesByGroup.getLength());
    checkRectPrimitive(aSequenceRectWithStylesByGroup);

    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithParentStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStylesByGroup));

}


CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
