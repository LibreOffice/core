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

#include <com/sun/star/graphic/EmfTools.hpp>
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
    void checkRectPrimitive(Primitive2DSequence const & rPrimitive);

    void testWorking();
    void TestLinesAndDrawString();

    Primitive2DSequence parseEmf(const char* aSource);

public:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testWorking);
    CPPUNIT_TEST(TestLinesAndDrawString);
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseEmf(const char* aSource)
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
    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    // emfio: add examples (later)
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100"); // rect background height
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100"); // rect background width
}

void Test::testWorking()
{
    Primitive2DSequence aSequenceRect = parseEmf("/emfio/qa/cppunit/emf/data/fdo79679-2.emf");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRect.getLength());
    checkRectPrimitive(aSequenceRect);
}

void Test::TestLinesAndDrawString()
{
    // This unit test checks correct line width and DrawString attributes
    // in a given image of a molecule

    // first, get the sequence of primitives
    Primitive2DSequence aSequence = parseEmf("/emfio/qa/cppunit/emf/data/image1.emf");

    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(aSequence),"temp.dat");

    CPPUNIT_ASSERT (pDocument);

    // test the lines color and width (15 lines)
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[1]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[2]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[3]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[4]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[5]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[6]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[7]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[8]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[9]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[10]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[11]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[12]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[13]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[14]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[15]/line", "color", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[1]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[2]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[3]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[4]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[5]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[6]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[7]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[8]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[9]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[10]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[11]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[12]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[13]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[14]/line", "width", "35");
    assertXPath(pDocument, "/primitive2D/metafile/transform/polypolygonstroke[15]/line", "width", "35");

    // test string position, height, text, color and font (3 strings)
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "x", "1330");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "x", "2809");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "x", "599");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "y", "1905");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "y", "2745");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "y", "645");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "height", "583");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "height", "583");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "height", "583");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "text", "O");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "text", "COOH");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "text", "O");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "fontcolor", "#000000");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[1]", "familyname", "ARIAL");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[2]", "familyname", "ARIAL");
    assertXPath(pDocument, "/primitive2D/metafile/transform/textsimpleportion[3]", "familyname", "ARIAL");

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
