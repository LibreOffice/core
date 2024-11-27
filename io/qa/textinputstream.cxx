/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <cstring>

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XTextInputStream2.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotest/bootstrapfixturebase.hxx>

namespace {

class Input: public cppu::WeakImplHelper<css::io::XInputStream> {
public:
    Input(char* inputData, sal_Int32 inputSize):
        open_(true),
        index_(0),
        size(inputSize),
        data(inputData) {}

private:
    virtual ~Input() override {}

    sal_Int32 SAL_CALL readBytes(css::uno::Sequence<sal_Int8> &, sal_Int32)
        override
    { CPPUNIT_FAIL("readLine is supposed to call readSomeBytes instead"); }

    sal_Int32 SAL_CALL readSomeBytes(
        css::uno::Sequence<sal_Int8 > & aData, sal_Int32 nMaxBytesToRead) override
    {
        assert(nMaxBytesToRead >= 0);
        osl::MutexGuard g(mutex_);
        checkClosed();
        assert(index_ >= 0 && index_ <= size);
        sal_Int32 n = std::min<sal_Int32>(
            std::min<sal_Int32>(nMaxBytesToRead, 2), size - index_);
        assert(n >= 0 && n <= size - index_);
        aData.realloc(n);
        std::memcpy(aData.getArray(), data + index_, n);
        index_ += n;
        assert(index_ >= 0 && index_ <= size);
        return n;
    }

    void SAL_CALL skipBytes(sal_Int32 nBytesToSkip) override
    {
        assert(nBytesToSkip >= 0);
        osl::MutexGuard g(mutex_);
        checkClosed();
        assert(index_ >= 0 && index_ <= size);
        index_ += std::min<sal_Int32>(nBytesToSkip, size - index_);
        assert(index_ >= 0 && index_ <= size);
    }

    sal_Int32 SAL_CALL available() override
    {
        osl::MutexGuard g(mutex_);
        checkClosed();
        assert(index_ >= 0 && index_ <= size);
        return size - index_;
    }

    void SAL_CALL closeInput() override
    {
        osl::MutexGuard g(mutex_);
        checkClosed();
        open_ = true;
    }

    void checkClosed() {
        if (!open_) {
            throw css::io::NotConnectedException(
                u"test input stream already closed"_ustr);
        }
    }

    osl::Mutex mutex_;
    bool open_;
    sal_Int32 index_;

    sal_Int32 size;
    char* data;

};

class Test: public test::BootstrapFixtureBase {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testReadLine);
    CPPUNIT_TEST(testReadLineEndChars);
    CPPUNIT_TEST_SUITE_END();

    void testReadLine();
    void testReadLineEndChars();

    OUString readFirstLine(char data1[], int size);
};

OUString Test::readFirstLine(char *inputData, int inputSize) {
    css::uno::Reference<css::io::XTextInputStream2> s(
        css::io::TextInputStream::create(getComponentContext()));
    s->setInputStream(new Input(inputData, inputSize));
    return s->readLine();
}

void Test::testReadLine() {
    char inputData[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    OUString l(readFirstLine(inputData, sizeof(inputData)));
    CPPUNIT_ASSERT_EQUAL(u"123456789"_ustr, l);
}

void Test::testReadLineEndChars() {
    std::vector<char> inputData = {'a', 'b', 'c', '\r'};
    OUString l(readFirstLine(inputData.data(), inputData.size()));
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);

    inputData = {'a', 'b', 'c', '\n'};
    l = readFirstLine(inputData.data(), inputData.size());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);

    inputData = {'a', 'b', 'c', '\r', '\n'};
    l = readFirstLine(inputData.data(), inputData.size());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);

    inputData = {'a', 'b', 'c', '\r', 'd', 'e', 'f'};
    l = readFirstLine(inputData.data(), inputData.size());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);

    inputData = {'a', 'b', 'c', '\n', 'd', 'e', 'f'};
    l = readFirstLine(inputData.data(), inputData.size());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);

    css::uno::Reference<css::io::XTextInputStream2> s(
        css::io::TextInputStream::create(getComponentContext()));
    inputData = {'a', 'b', 'c', '\r', '\n', 'd', 'e', 'f'};
    s->setInputStream(new Input(inputData.data(), inputData.size()));
    l = s->readLine();
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, l);
    l = s->readLine();
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, l);
}


CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
