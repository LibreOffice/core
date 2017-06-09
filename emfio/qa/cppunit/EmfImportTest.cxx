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
    void checkRectPrimitive(Primitive2DSequence& rPrimitive);

    void testWorking();

    Primitive2DSequence parseEmf(const char* aSource);

public:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testWorking);
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

    return xEmfParser->getDecomposition(aInputStream, aPath);
}

void Test::checkRectPrimitive(Primitive2DSequence& rPrimitive)
{
    Primitive2dXmlDump dumper;
    xmlDocPtr pDocument = dumper.dumpAndParse(comphelper::sequenceToContainer<Primitive2DContainer>(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    // emfio: add examples
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", "#00cc00"); // rect background color
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "height", "100"); // rect background height
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "width", "100"); // rect background width
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "minx", "10");
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "miny", "10");
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxx", "110");
    // assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "maxy", "110");
    // assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", "#ff0000"); // rect stroke color
    // assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", "3"); // rect stroke width
}

void Test::testWorking()
{
    Primitive2DSequence aSequenceRect = parseEmf("/emfio/qa/cppunit/data/fdo79679-2.emf");
    CPPUNIT_ASSERT_EQUAL(1, (int) aSequenceRect.getLength());
    checkRectPrimitive(aSequenceRect);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
