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
#include <exception>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XTextInputStream2.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
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
    Input(): open_(true), index_(0) {}

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
        assert(index_ >= 0 && index_ <= SIZE);
        sal_Int32 n = std::min<sal_Int32>(
            std::min<sal_Int32>(nMaxBytesToRead, 2), SIZE - index_);
        assert(n >= 0 && n <= SIZE - index_);
        aData.realloc(n);
        std::memcpy(aData.getArray(), data + index_, n);
        index_ += n;
        assert(index_ >= 0 && index_ <= SIZE);
        return n;
    }

    void SAL_CALL skipBytes(sal_Int32 nBytesToSkip) override
    {
        assert(nBytesToSkip >= 0);
        osl::MutexGuard g(mutex_);
        checkClosed();
        assert(index_ >= 0 && index_ <= SIZE);
        index_ += std::min<sal_Int32>(nBytesToSkip, SIZE - index_);
        assert(index_ >= 0 && index_ <= SIZE);
    }

    sal_Int32 SAL_CALL available() override
    {
        osl::MutexGuard g(mutex_);
        checkClosed();
        assert(index_ >= 0 && index_ <= SIZE);
        return SIZE - index_;
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
                "test input stream already closed");
        }
    }

    static sal_Int32 const SIZE = 9;
    static char const data[SIZE];

    osl::Mutex mutex_;
    bool open_;
    sal_Int32 index_;
};

char const Input::data[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

class Test: public test::BootstrapFixtureBase {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testReadLine);
    CPPUNIT_TEST_SUITE_END();

    void testReadLine();
};

void Test::testReadLine() {
    css::uno::Reference<css::io::XTextInputStream2> s(
        css::io::TextInputStream::create(getComponentContext()));
    s->setInputStream(new Input);
    OUString l(s->readLine());
    CPPUNIT_ASSERT_EQUAL(OUString("123456789"), l);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
