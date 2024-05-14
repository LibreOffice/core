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

#include <memory>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/XSvgParser.hpp>

#include <basegfx/DrawCommands.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace
{
using namespace css;

class TestParsing : public test::BootstrapFixture
{
    void testSimpleRectangle();
    void testPath();
    uno::Reference<io::XInputStream> parseSvg(const OUString& aSource);

public:
    CPPUNIT_TEST_SUITE(TestParsing);
    CPPUNIT_TEST(testSimpleRectangle);
    CPPUNIT_TEST(testPath);
    CPPUNIT_TEST_SUITE_END();
};

uno::Reference<io::XInputStream> TestParsing::parseSvg(const OUString& aSource)
{
    SvFileStream aFileStream(aSource, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    uno::Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    uno::Reference<io::XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    return aInputStream;
}

void TestParsing::testSimpleRectangle()
{
    OUString aSvgFile = u"/svgio/qa/cppunit/data/VisiotorTest-Rect.svg"_ustr;
    OUString aUrl = m_directories.getURLFromSrc(aSvgFile);
    OUString aPath = m_directories.getPathFromSrc(aSvgFile);

    uno::Reference<io::XInputStream> xStream = parseSvg(aUrl);
    CPPUNIT_ASSERT(xStream.is());

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    uno::Any aAny = xSvgParser->getDrawCommands(xStream, aPath);
    CPPUNIT_ASSERT(aAny.has<sal_uInt64>());
    auto* pDrawRoot = reinterpret_cast<gfx::DrawRoot*>(aAny.get<sal_uInt64>());

    basegfx::B2DRange aSurfaceRectangle(0, 0, 120, 120);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDrawRoot->maChildren.size());
    CPPUNIT_ASSERT_EQUAL(aSurfaceRectangle, pDrawRoot->maRectangle);

    auto* pDrawBase = pDrawRoot->maChildren[0].get();
    CPPUNIT_ASSERT_EQUAL(gfx::DrawCommandType::Rectangle, pDrawRoot->maChildren[0]->getType());
    auto* pDrawRect = static_cast<gfx::DrawRectangle*>(pDrawBase);
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(10, 10, 110, 110), pDrawRect->maRectangle);
    CPPUNIT_ASSERT_EQUAL(3.0, pDrawRect->mnStrokeWidth);
    CPPUNIT_ASSERT(bool(pDrawRect->mpStrokeColor));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(*pDrawRect->mpStrokeColor));
    CPPUNIT_ASSERT(bool(pDrawRect->mpFillColor));
    CPPUNIT_ASSERT_EQUAL(Color(0x00cc00), Color(*pDrawRect->mpFillColor));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, pDrawRect->mnOpacity, 1E-12);
}

void TestParsing::testPath()
{
    OUString aSvgFile = u"/svgio/qa/cppunit/data/path.svg"_ustr;
    OUString aUrl = m_directories.getURLFromSrc(aSvgFile);
    OUString aPath = m_directories.getPathFromSrc(aSvgFile);

    uno::Reference<io::XInputStream> xStream = parseSvg(aUrl);
    CPPUNIT_ASSERT(xStream.is());

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    uno::Any aAny = xSvgParser->getDrawCommands(xStream, aPath);
    CPPUNIT_ASSERT(aAny.has<sal_uInt64>());
    auto* pDrawRoot = reinterpret_cast<gfx::DrawRoot*>(aAny.get<sal_uInt64>());

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDrawRoot->maChildren.size());

    auto* pDrawBase = pDrawRoot->maChildren[0].get();
    CPPUNIT_ASSERT_EQUAL(gfx::DrawCommandType::Path, pDrawBase->getType());
    auto* pDrawPath = static_cast<gfx::DrawPath*>(pDrawBase);

    CPPUNIT_ASSERT_EQUAL(u"m1 1h42v24h-42v-24z"_ustr,
                         basegfx::utils::exportToSvgD(pDrawPath->maPolyPolygon, true, true, false));
    CPPUNIT_ASSERT_EQUAL(0.0, pDrawPath->mnStrokeWidth);
    CPPUNIT_ASSERT(bool(pDrawPath->mpStrokeColor));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(*pDrawPath->mpStrokeColor));
    CPPUNIT_ASSERT(bool(pDrawPath->mpFillColor));
    CPPUNIT_ASSERT_EQUAL(Color(0x007aff), Color(*pDrawPath->mpFillColor));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, pDrawPath->mnOpacity, 1E-12);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestParsing);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
