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
#include <sstream>

#include <memory>

#include <cppunit/extensions/HelperMacros.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>

#include <test/bootstrapfixture.hxx>

#include <WPXSvInputStream.hxx>

namespace io = com::sun::star::io;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

using std::shared_ptr;
using std::unique_ptr;

using librevenge::RVNGInputStream;
using librevenge::RVNG_SEEK_CUR;
using librevenge::RVNG_SEEK_END;
using librevenge::RVNG_SEEK_SET;

using std::equal;

using writerperfect::WPXSvInputStream;

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
    CPPUNIT_TEST(testStructured);
    CPPUNIT_TEST_SUITE_END();

private:
    void testRead();
    void testSeekSet();
    void testSeekCur();
    void testSeekEnd();
    void testStructured();
};

const char aText[] = "hello world";
constexpr OUStringLiteral aOLEFile = u"/writerperfect/qa/unit/data/stream/fdo40686-1.doc";
constexpr OUStringLiteral aZipFile = u"/writerperfect/qa/unit/data/stream/test.odt";

shared_ptr<RVNGInputStream> lcl_createStream()
{
    using comphelper::SequenceInputStream;

    const css::uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(aText),
                                             sizeof aText);
    const uno::Reference<io::XInputStream> xInputStream(new SequenceInputStream(aData));

    shared_ptr<RVNGInputStream> pInputStream;
    if (xInputStream.is())
        pInputStream = std::make_shared<WPXSvInputStream>(xInputStream);

    return pInputStream;
}

shared_ptr<RVNGInputStream> lcl_createStreamForURL(const OUString& rURL)
{
    using uno::Reference;
    using uno::UNO_QUERY_THROW;

    const Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                     css::uno::UNO_SET_THROW);
    const Reference<ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.ucb.SimpleFileAccess", xContext),
        UNO_QUERY_THROW);
    const Reference<io::XInputStream> xInputStream(xFileAccess->openFileRead(rURL),
                                                   css::uno::UNO_SET_THROW);

    return std::make_shared<WPXSvInputStream>(xInputStream);
}

void lcl_testSubStreams(const shared_ptr<RVNGInputStream>& pInput)
{
    shared_ptr<RVNGInputStream> pSubStream;

    // all valid substreams can be read
    for (std::size_t i = 0; i != pInput->subStreamCount(); ++i)
    {
        std::ostringstream msg("opening substream ");
        msg << i;
        pSubStream.reset(pInput->getSubStreamById(i));
        CPPUNIT_ASSERT_MESSAGE(msg.str(), bool(pSubStream));
    }

    // invalid substreams cannot be read
    pSubStream.reset(pInput->getSubStreamById(pInput->subStreamCount()));
    CPPUNIT_ASSERT(!pSubStream);
}

void WPXSvStreamTest::testRead()
{
    const shared_ptr<RVNGInputStream> pInput(lcl_createStream());
    const unsigned long nLen = sizeof aText;

    unsigned long nReadBytes = 0;
    const unsigned char* pData = nullptr;
    const unsigned char* const pTextOrig = reinterpret_cast<const unsigned char*>(aText);
    const unsigned char* pText = pTextOrig;

    // reading by small pieces
    pData = pInput->read(1UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(1UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->isEnd());
    pText += nReadBytes;

    pData = pInput->read(2UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(2UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->isEnd());
    pText += nReadBytes;

    pData = pInput->read(3UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(3UL, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(!pInput->isEnd());
    pText += nReadBytes;

    assert(nLen > 6);
    pData = pInput->read(nLen - 6, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen - 6, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->isEnd());

    // reading everything at once
    pInput->seek(0, RVNG_SEEK_SET);
    pText = pTextOrig;

    pData = pInput->read(nLen, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->isEnd());

    // trying to read too much
    pInput->seek(0, RVNG_SEEK_SET);
    pText = pTextOrig;

    pData = pInput->read(nLen + 1, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(nLen, nReadBytes);
    CPPUNIT_ASSERT(equal(pText, pText + nReadBytes, pData));
    CPPUNIT_ASSERT(pInput->isEnd());

    // trying to read nothing
    pInput->seek(0, RVNG_SEEK_SET);
    pData = pInput->read(0UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(0UL, nReadBytes);
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT_EQUAL(pData, static_cast<const unsigned char*>(nullptr));
    CPPUNIT_ASSERT(!pInput->isEnd());
}

void WPXSvStreamTest::testSeekSet()
{
    const shared_ptr<RVNGInputStream> pInput(lcl_createStream());
    const long nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    // valid seeks
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(1, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(1L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(nLen, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(nLen, pInput->tell());
    CPPUNIT_ASSERT(pInput->isEnd());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(-1, RVNG_SEEK_SET));
    // Okay, this is not defined. But it is what the WPXSvInputStream
    // does ATM and it is reasonable.
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT(0 != pInput->seek(nLen + 1, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(nLen, pInput->tell());
    CPPUNIT_ASSERT(pInput->isEnd());
}

void WPXSvStreamTest::testSeekCur()
{
    const shared_ptr<RVNGInputStream> pInput(lcl_createStream());
    const long nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT(!pInput->isEnd());
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // valid seeks

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(1, RVNG_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(1L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-1, RVNG_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(-1, RVNG_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT(0 != pInput->seek(nLen + 1, RVNG_SEEK_CUR));
    CPPUNIT_ASSERT_EQUAL(nLen, pInput->tell());
    CPPUNIT_ASSERT(pInput->isEnd());
}

void WPXSvStreamTest::testSeekEnd()
{
    const shared_ptr<RVNGInputStream> pInput(lcl_createStream());
    const long nLen = sizeof aText;

    // check initial state
    CPPUNIT_ASSERT(!pInput->isEnd());
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // valid seeks
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(nLen, pInput->tell());
    CPPUNIT_ASSERT(pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-1, RVNG_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(nLen - 1, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(-nLen, RVNG_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());

    // go back to the beginning
    CPPUNIT_ASSERT_EQUAL(0, pInput->seek(0, RVNG_SEEK_SET));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());

    // invalid seeks
    CPPUNIT_ASSERT(0 != pInput->seek(1, RVNG_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(nLen, pInput->tell());
    CPPUNIT_ASSERT(pInput->isEnd());

    CPPUNIT_ASSERT(0 != pInput->seek(-nLen - 1, RVNG_SEEK_END));
    CPPUNIT_ASSERT_EQUAL(0L, pInput->tell());
    CPPUNIT_ASSERT(!pInput->isEnd());
}

void WPXSvStreamTest::testStructured()
{
    // OLE2
    {
        const shared_ptr<RVNGInputStream> pInput(
            lcl_createStreamForURL(m_directories.getURLFromSrc(aOLEFile)));
        assert(bool(pInput));

        CPPUNIT_ASSERT(pInput->isStructured());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned>(2), pInput->subStreamCount());
        lcl_testSubStreams(pInput);

        // check for existing substream
        CPPUNIT_ASSERT(pInput->existsSubStream("WordDocument"));
        unique_ptr<RVNGInputStream> pSubStream(pInput->getSubStreamByName("WordDocument"));
        CPPUNIT_ASSERT(bool(pSubStream));
        CPPUNIT_ASSERT(!pSubStream->isEnd());

        // check for not existing substream
        CPPUNIT_ASSERT(!pInput->existsSubStream("foo"));
        pSubStream.reset(pInput->getSubStreamByName("foo"));
        CPPUNIT_ASSERT(!pSubStream);
    }

    // Zip
    {
        const shared_ptr<RVNGInputStream> pInput(
            lcl_createStreamForURL(m_directories.getURLFromSrc(aZipFile)));
        assert(bool(pInput));

        CPPUNIT_ASSERT(pInput->isStructured());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned>(9), pInput->subStreamCount());
        lcl_testSubStreams(pInput);

        // check for existing substream
        CPPUNIT_ASSERT(pInput->existsSubStream("content.xml"));
        unique_ptr<RVNGInputStream> pSubStream(pInput->getSubStreamByName("content.xml"));
        CPPUNIT_ASSERT(bool(pSubStream));
        CPPUNIT_ASSERT(!pSubStream->isEnd());

        // check for not existing substream
        CPPUNIT_ASSERT(!pInput->existsSubStream("foo"));
        pSubStream.reset(pInput->getSubStreamByName("foo"));
        CPPUNIT_ASSERT(!pSubStream);
    }

    // not structured
    {
        const shared_ptr<RVNGInputStream> pInput(lcl_createStream());

        CPPUNIT_ASSERT(!pInput->isStructured());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned>(0), pInput->subStreamCount());
        CPPUNIT_ASSERT(!pInput->existsSubStream("foo"));
        CPPUNIT_ASSERT(!pInput->getSubStreamByName("foo"));
        CPPUNIT_ASSERT(!pInput->getSubStreamById(42));
        CPPUNIT_ASSERT(!pInput->subStreamName(42));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(WPXSvStreamTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
