/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/BigEndianTypes.hxx>
#include <osl/endian.h>
#include <array>

namespace
{
// Struct using endian of the system with 16-bit variables
struct SystemEndian16
{
    sal_uInt16 x;
    sal_uInt16 y;
    sal_uInt16 z;
    sal_uInt16 w;
};

// Struct using big endian with 16-bit variables
struct BigEndian16
{
    o3tl::sal_uInt16_BE x;
    o3tl::sal_uInt16_BE y;
    o3tl::sal_uInt16_BE z;
    o3tl::sal_uInt16_BE w;
};

// Struct using endian of the system with 32-bit variables
struct SystemEndian32
{
    sal_uInt32 x;
    sal_uInt32 y;
};

// Struct using big endian with 32-bit variables
struct BigEndian32
{
    o3tl::sal_uInt32_BE x;
    o3tl::sal_uInt32_BE y;
};

// Test data
constexpr std::array<sal_uInt8, 8> aDataArray = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

class BigEndianTypesTest : public CppUnit::TestFixture
{
public:
    void testCast()
    {
        auto* pSystem16 = reinterpret_cast<const SystemEndian16*>(aDataArray.data());

        // We expect different results depending on the system endian
#ifdef OSL_LITENDIAN
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0201), pSystem16->x);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0403), pSystem16->y);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0605), pSystem16->z);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0807), pSystem16->w);
#else
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0102), pSystem16->x);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0304), pSystem16->y);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0506), pSystem16->z);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0708), pSystem16->w);
#endif

        // Reading in big endian should read the same independent of system endian
        auto* pBig16 = reinterpret_cast<const BigEndian16*>(aDataArray.data());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0102), sal_uInt16(pBig16->x));
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0304), sal_uInt16(pBig16->y));
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0506), sal_uInt16(pBig16->z));
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x0708), sal_uInt16(pBig16->w));

        // We expect different results depending on the system endian
        auto* pSystem32 = reinterpret_cast<const SystemEndian32*>(aDataArray.data());
#ifdef OSL_LITENDIAN
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x04030201), pSystem32->x);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x08070605), pSystem32->y);
#else
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x01020304), pSystem32->x);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x05060708), pSystem32->y);
#endif

        // Reading in big endian should read the same independent of system endian
        auto* pBig32 = reinterpret_cast<const BigEndian32*>(aDataArray.data());
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x01020304), sal_uInt32(pBig32->x));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0x05060708), sal_uInt32(pBig32->y));
    }

    CPPUNIT_TEST_SUITE(BigEndianTypesTest);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BigEndianTypesTest);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
