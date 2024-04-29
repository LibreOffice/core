/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/types.hxx>
#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace
{
class TypesTest : public CppUnit::TestFixture
{
public:
    void testGetINT64();
    void testGetINT32();
    void testGetINT16();
    void testGetDouble();
    void testGetFloat();
    void testGetString();

    CPPUNIT_TEST_SUITE(TypesTest);

    CPPUNIT_TEST(testGetINT64);
    CPPUNIT_TEST(testGetINT32);
    CPPUNIT_TEST(testGetINT16);
    CPPUNIT_TEST(testGetDouble);
    CPPUNIT_TEST(testGetFloat);
    CPPUNIT_TEST(testGetString);

    CPPUNIT_TEST_SUITE_END();
};

void TypesTest::testGetINT64()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1337), ::comphelper::getINT64(uno::Any(sal_Int64(1337))));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), ::comphelper::getINT64(aValue));
}

void TypesTest::testGetINT32()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1337), ::comphelper::getINT32(uno::Any(sal_Int32(1337))));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ::comphelper::getINT32(aValue));
}

void TypesTest::testGetINT16()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1337), ::comphelper::getINT16(uno::Any(sal_Int16(1337))));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), ::comphelper::getINT16(aValue));
}

void TypesTest::testGetDouble()
{
    CPPUNIT_ASSERT_EQUAL(1337.1337, ::comphelper::getDouble(uno::Any(1337.1337)));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(0.0, ::comphelper::getDouble(aValue));
}

void TypesTest::testGetFloat()
{
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(1337.0),
                         ::comphelper::getFloat(uno::Any(static_cast<float>(1337.0))));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(0.0), ::comphelper::getFloat(aValue));
}

void TypesTest::testGetString()
{
    CPPUNIT_ASSERT_EQUAL(u"1337"_ustr, ::comphelper::getString(uno::Any(u"1337"_ustr)));

    uno::Any aValue;
    CPPUNIT_ASSERT_EQUAL(u""_ustr, ::comphelper::getString(aValue));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TypesTest);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
