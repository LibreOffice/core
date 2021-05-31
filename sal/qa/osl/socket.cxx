/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <osl/socket.h>
#include <rtl/ustring.hxx>

namespace
{
class SocketTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SocketTest);
    CPPUNIT_TEST(test_getDottedInetAddrOfSocketAddr);
    CPPUNIT_TEST_SUITE_END();

    void test_getDottedInetAddrOfSocketAddr()
    {
        OUString const in("123.4.56.78");
        auto const addr = osl_createInetSocketAddr(in.pData, 0);
        CPPUNIT_ASSERT(addr != nullptr);
        OUString out;
        auto const res = osl_getDottedInetAddrOfSocketAddr(addr, &out.pData);
        CPPUNIT_ASSERT_EQUAL(osl_Socket_Ok, res);
        CPPUNIT_ASSERT_EQUAL(in, out);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SocketTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
