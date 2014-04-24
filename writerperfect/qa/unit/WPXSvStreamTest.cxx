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
#include "com/sun/star/ucb/XSimpleFileAccess.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include "comphelper/processfactory.hxx"
#include "comphelper/seqstream.hxx"

#include "rtl/ref.hxx"

#include "test/bootstrapfixture.hxx"

#include <writerperfect/WPXSvInputStream.hxx>

namespace io = com::sun::star::io;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

using boost::shared_ptr;

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

static const char aText[] = "hello world";
static const char aOLEFile[] = "/writerperfect/qa/unit/data/fdo40686-1.doc";
static const char aZipFile[] = "/writerperfect/qa/unit/data/test.odt";

shared_ptr<WPXInputStream> lcl_createStream()
{
    using comphelper::SequenceInputStream;

    const css::uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(aText), sizeof aText);
    const uno::Reference<io::XInputStream> xInputStream(new SequenceInputStream(aData));

    shared_ptr<WPXInputStream> pInputStream;
    if (xInputStream.is())
        pInputStream.reset(new WPXSvInputStream(xInputStream));

    return pInputStream;
}

const shared_ptr<WPXInputStream> lcl_createStreamForURL(const rtl::OUString &rURL)
{
    using uno::Reference;
    using uno::UNO_QUERY_THROW;

    const Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(), UNO_QUERY_THROW);
    const Reference<ucb::XSimpleFileAccess> xFileAccess(
            xContext->getServiceManager()->createInstanceWithContext("com.sun.star.ucb.SimpleFileAccess", xContext),
            UNO_QUERY_THROW);
    const Reference<io::XInputStream> xInputStream(xFileAccess->openFileRead(rURL), UNO_QUERY_THROW);

    const shared_ptr<WPXInputStream> pInput(new WPXSvInputStream(xInputStream));
    return pInput;
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
    CPPUNIT_ASSERT_EQUAL(pData, static_cast<const unsigned char*>(0));
    CPPUNIT_ASSERT(!pInput->atEOS());
}

void WPXSvStreamTest::testSeekSet()
{
    const shared_ptr<WPXInputStream> pInput(lcl_createStream());
    const long nLen = sizeof aText;

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
    const long nLen = sizeof aText;

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
    const long nLen = sizeof aText;

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

void WPXSvStreamTest::testStructured()
{
    // OLE2
    {
        const shared_ptr<WPXInputStream> pInput(lcl_createStreamForURL(getURLFromSrc(aOLEFile)));
        assert(bool(pInput));

        CPPUNIT_ASSERT(pInput->isOLEStream());
        shared_ptr<WPXInputStream> pSubStream(pInput->getDocumentOLEStream("WordDocument"));
        CPPUNIT_ASSERT(bool(pSubStream));
        pSubStream.reset(pInput->getDocumentOLEStream("foo"));
        CPPUNIT_ASSERT(!pSubStream);
    }

    // Zip
    {
        const shared_ptr<WPXInputStream> pInput(lcl_createStreamForURL(getURLFromSrc(aZipFile)));
        assert(bool(pInput));

        CPPUNIT_ASSERT(pInput->isOLEStream());
        shared_ptr<WPXInputStream> pSubStream(pInput->getDocumentOLEStream("content.xml"));
        CPPUNIT_ASSERT(bool(pSubStream));
        pSubStream.reset(pInput->getDocumentOLEStream("foo"));
        CPPUNIT_ASSERT(!pSubStream);
    }

    // not structured
    {
        const shared_ptr<WPXInputStream> pInput(lcl_createStream());

        CPPUNIT_ASSERT(!pInput->isOLEStream());
        CPPUNIT_ASSERT(0 == pInput->getDocumentOLEStream("foo"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(WPXSvStreamTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
