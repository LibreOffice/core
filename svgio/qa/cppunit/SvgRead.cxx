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
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <com/sun/star/graphic/XSvgParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

#include <svgdocumenthandler.hxx>
#include <svgrectnode.hxx>
#include <svgsvgnode.hxx>
#include <svggnode.hxx>

#include <basegfx/DrawCommands.hxx>

namespace
{
using namespace css;

class Test : public test::BootstrapFixture
{
    void test();
    uno::Reference<io::XInputStream> parseSvg(const OUString& aSource);

public:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

uno::Reference<io::XInputStream> Test::parseSvg(const OUString& aSource)
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

void Test::test()
{
    OUString aSvgFile = "/svgio/qa/cppunit/data/Rect.svg";
    OUString aUrl = m_directories.getURLFromSrc(aSvgFile);
    OUString aPath = m_directories.getPathFromSrc(aSvgFile);

    uno::Reference<io::XInputStream> xStream = parseSvg(aUrl);
    CPPUNIT_ASSERT(xStream.is());

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    uno::Any aAny = xSvgParser->getDrawCommands(xStream, aPath);
    CPPUNIT_ASSERT(aAny.has<sal_uInt64>());
    gfx::DrawRoot* pDrawRoot = reinterpret_cast<gfx::DrawRoot*>(aAny.get<sal_uInt64>());

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDrawRoot->maChildren.size());
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DRange(0, 0, 120, 120), pDrawRoot->maRectangle);

    CPPUNIT_ASSERT_EQUAL(gfx::DrawCommandType::Rectangle, pDrawRoot->maChildren[0]->getType());
    CPPUNIT_ASSERT_EQUAL(
        basegfx::B2DRange(10, 10, 110, 110),
        static_cast<gfx::DrawRectangle*>(pDrawRoot->maChildren[0].get())->maRectangle);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
