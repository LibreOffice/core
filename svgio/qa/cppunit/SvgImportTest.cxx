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
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>

#include <boost/scoped_array.hpp>

namespace
{

using namespace css::uno;
using namespace css::io;
using namespace css::graphic;

class Test : public test::BootstrapFixture
{
    Reference<XComponentContext> mxContext;

    void testStyles();

    Primitive2DSequence parseSvg(const char* aSource);

public:
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStyles);
    CPPUNIT_TEST_SUITE_END();
};

Primitive2DSequence Test::parseSvg(const char* aSource)
{
    const Reference<XSvgParser> xSvgParser = SvgTools::create(mxContext);

    OUString aUrl  = getURLFromSrc(aSource);
    OUString aPath = getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, STREAM_READ);
    sal_Size nSize = aFileStream.remainingSize();
    boost::scoped_array<sal_Int8> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.Read(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    return xSvgParser->getDecomposition(aInputStream, aPath);
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    mxContext = Reference<XComponentContext>(comphelper::getProcessComponentContext());
}

void Test::tearDown()
{
    BootstrapFixture::tearDown();
}

void Test::testStyles()
{
    Primitive2DSequence maSequenceRect = parseSvg("/svgio/qa/cppunit/data/Rect.svg");
    CPPUNIT_ASSERT_EQUAL(1, maSequenceRect.getLength());

    Primitive2DSequence maSequenceRectWithStyle = parseSvg("/svgio/qa/cppunit/data/RectWithStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, maSequenceRectWithStyle.getLength());

    Primitive2DSequence maSequenceRectWithParentStyle = parseSvg("/svgio/qa/cppunit/data/RectWithParentStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, maSequenceRectWithParentStyle.getLength());

    // TODO: Test if the 3 sequences are equal..
    //const Primitive2DReference xReference(maSequenceRect[0]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
