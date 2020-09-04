/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
private:
    void testOString()
    {
        struct S
        {
            char a[4];
        };
        S s{ "x\0y" };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OString(s.a).getLength());
        // Ideally, the below would work the same as the above.  However, the const reference makes
        // the ConstCharArrayDetector instead of the NonConstCharArrayDetector kick in, so that the
        // call to OString(r.a) would fire the ConstCharArrayDetector<T>::isValid assert (and in
        // NDEBUG builds the CPPUNIT_ASSERT_EQUAL would fail with 3 != 1):
        if ((false))
        {
            S const& r = s;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OString(r.a).getLength());
        }
    }

    void testOUStringChar()
    {
        struct S
        {
            char a[4];
        };
        S s{ "x\0y" };
        // This would fail to compile, as there is no OUString ctor taking a
        // NonConstCharArrayDetector char array:
#if 0
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OUString(s.a).getLength());
#endif
        // Ideally, the below would fail to compile the same as the above.  However, the const
        // reference makes the ConstCharArrayDetector instead of the NonConstCharArrayDetector kick
        // in, so that the call to OUString(r.a) would fire the ConstCharArrayDetector<T>::isValid
        // assert (and in NDEBUG builds the CPPUNIT_ASSERT_EQUAL would fail with 3 != 1):
        if ((false))
        {
            S const& r = s;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OUString(r.a).getLength());
        }
    }

    void testOUStringChar16t()
    {
        struct S
        {
            char16_t a[4];
        };
        S s{ u"x\0y" };
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OUString(s.a).getLength());
        // Ideally, the below would work the same as the above.  However, the const reference makes
        // the ConstCharArrayDetector instead of the NonConstCharArrayDetector kick in, so that the
        // call to OUString(r.a) would fire the ConstCharArrayDetector<T>::isValid assert (and in
        // NDEBUG builds the CPPUNIT_ASSERT_EQUAL would fail with 3 != 1)::
        if ((false))
        {
            S const& r = s;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), OUString(r.a).getLength());
        }
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testOString);
    CPPUNIT_TEST(testOUStringChar);
    CPPUNIT_TEST(testOUStringChar16t);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
