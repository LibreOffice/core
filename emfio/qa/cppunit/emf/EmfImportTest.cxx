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

#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/graphic/EmfTools.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>

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
    void checkRectPrimitive(Primitive2DSequence const & rPrimitive);

    void testWorking();
    void TestDrawString();
    void TestDrawStringTransparent();
    void TestDrawLine();
    void TestLinearGradient();

    Primitive2DSequence parseEmf(const OUString& aSource);

public:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testWorking);
    CPPUNIT_TEST(TestDrawString);
    CPPUNIT_TEST(TestDrawStringTransparent);
    CPPUNIT_TEST(TestDrawLine);
    CPPUNIT_TEST(TestLinearGradient);
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseEmf(const OUString& aSource)
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

void Test::checkRectPrimitive(Primitive2DSequence const & rPrimitive)
{
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    // emfio: add examples (later)
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100"); // rect background height
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100"); // rect background width
}

void Test::testWorking()
{
    Primitive2DSequence aSequenceRect = parseEmf("/emfio/qa/cppunit/emf/data/fdo79679-2.emf");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRect.getLength()));
    checkRectPrimitive(aSequenceRect);
}

void Test::TestDrawString()
{
#if HAVE_MORE_FONTS
    // This unit checks for a correct import of an EMF+ file with only one DrawString Record
    // Since the text is undecorated the optimal choice is a simpletextportion primitive

    // first, get the sequence of primitives and dump it
    Primitive2DSequence aSequence = parseEmf("/emfio/qa/cppunit/emf/data/TestDrawString.emf");
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
    Primitive2DSequence aSequence = parseEmf("/emfio/qa/cppunit/emf/data/TestDrawStringTransparent.emf");
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
    Primitive2DSequence aSequence = parseEmf("/emfio/qa/cppunit/emf/data/TestDrawLine.emf");
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
    Primitive2DSequence aSequence = parseEmf("/emfio/qa/cppunit/emf/data/TestLinearGradient.emf");
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
