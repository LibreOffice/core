/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>

#include <boost/shared_ptr.hpp>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include "comphelper/seqstream.hxx"

#include "rtl/ref.hxx"

#include "test/bootstrapfixture.hxx"

#include "WPXSvStream.hxx"

namespace io = com::sun::star::io;
namespace uno = com::sun::star::uno;

using boost::shared_ptr;

using std::equal;

namespace
{

class WPXSvStreamTest : public test::BootstrapFixture
{
public:
    CPPUNIT_TEST_SUITE(WPXSvStreamTest);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST(testSeekSet);
    CPPUNIT_TEST(testSeekCur);
    CPPUNIT_TEST(testSeekEnd);
    CPPUNIT_TEST_SUITE_END();

private:
    void testRead();
    void testSeekSet();
    void testSeekCur();
    void testSeekEnd();
};

static const char aText[] = "hello world";

shared_ptr<WPXInputStream> lcl_createStream()
{
    using comphelper::SequenceInputStream;

    const comphelper::ByteSequence aData(reinterpret_cast<const sal_Int8*>(aText), sizeof aText);
    const uno::Reference<io::XInputStream> xInputStream(new SequenceInputStream(aData));

    shared_ptr<WPXInputStream> pInputStream;
    if (xInputStream.is())
        pInputStream.reset(new WPXSvInputStream(xInputStream));

    return pInputStream;
}

void WPXSvStreamTest::testRead()
{
    const shared_ptr<WPXInputStream> pInput(lcl_createStream());
    const unsigned long nLen = sizeof aText;

    unsigned long nReadBytes = 0;
    const unsigned char* pData = 0;
    const unsigned char* const pTextOrig = reinterpret_cast<const unsigned char*>(aText);
    const unsigned char* pText = pTextOrig;

    // reading by small pieces
    pData = pInput->read(1UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(1UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->atEOS());
    pText += nReadBytes;

    pData = pInput->read(2UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(2UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->atEOS());
    pText += nReadBytes;

    pData = pInput->read(3UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(3UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->atEOS());
    pText += nReadBytes;

    assert(nLen > 6);
    pData = pInput->read(nLen - 6, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen - 6, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->atEOS());

    // reading everything at once
    pInput->seek(0, WPX_SEEK_SET);
    pText = pTextOrig;

    pData = pInput->read(nLen, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->atEOS());

    // trying to read too much
    pInput->seek(0, WPX_SEEK_SET);
    pText = pTextOrig;

    pData = pInput->read(nLen + 1, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->atEOS());

    // trying to read nothing
    pInput->seek(0, WPX_SEEK_SET);
    pText = pTextOrig;

    pData = pInput->read(0UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(0UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());
}

void WPXSvStreamTest::testSeekSet()
{
    const shared_ptr<WPXInputStream> pInput(lcl_createStream());
    const size_t nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    // valid seeks
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(1, WPX_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(1L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(nLen, WPX_SEEK_SET));
    CPPUNIT_ASSERT(nLen == pInput->tell());
    CPPUNIT_ASSERT(pInput->atEOS());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(-1, WPX_SEEK_SET));
    // Okay, this is not defined. But it is what the WPXSvInputStream
    // does ATM and it is reasonable.
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT(0 != pInput->seek(nLen + 1, WPX_SEEK_SET));
    CPPUNIT_ASSERT(nLen == pInput->tell());
    CPPUNIT_ASSERT(pInput->atEOS());
}

void WPXSvStreamTest::testSeekCur()
{
    const shared_ptr<WPXInputStream> pInput(lcl_createStream());
    const size_t nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT(!pInput->atEOS());
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // valid seeks

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(1, WPX_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(1L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-1, WPX_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(-1, WPX_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT(0 != pInput->seek(nLen + 1, WPX_SEEK_CUR));
    CPPUNIT_ASSERT(nLen == pInput->tell());
    CPPUNIT_ASSERT(pInput->atEOS());
}

void WPXSvStreamTest::testSeekEnd()
{
    const shared_ptr<WPXInputStream> pInput(lcl_createStream());
    const size_t nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT(!pInput->atEOS());
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // valid seeks
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_END));
    CPPUNIT_ASSERT(nLen == pInput->tell());
    CPPUNIT_ASSERT(pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-1, WPX_SEEK_END));
    CPPUNIT_ASSERT((nLen - 1) == pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-nLen, WPX_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, WPX_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(1, WPX_SEEK_END));
    CPPUNIT_ASSERT(nLen == pInput->tell());
    CPPUNIT_ASSERT(pInput->atEOS());

    CPPUNIT_ASSERT(0 != pInput->seek(-nLen - 1, WPX_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->atEOS());
}

CPPUNIT_TEST_SUITE_REGISTRATION(WPXSvStreamTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
