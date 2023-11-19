/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/Guid.hxx>

namespace tools
{
class GuidTest : public CppUnit::TestFixture
{
public:
    void testGetString()
    {
        sal_uInt8 pArray[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        Guid aGuid(pArray);
        CPPUNIT_ASSERT_EQUAL("{01020304-0506-0708-090A-0B0C0D0E0F10}"_ostr, aGuid.getString());
    }

    void testCreate()
    {
        // data is generated when Guid is created
        Guid aGuid1(Guid::Generate);

        // check it's not initialized to 0
        CPPUNIT_ASSERT(*std::max_element(aGuid1.begin(), aGuid1.end()) > 0u);

        // data is generated when Guid is created
        Guid aGuid2(Guid::Generate);

        CPPUNIT_ASSERT_EQUAL(aGuid1, aGuid1);
        CPPUNIT_ASSERT_EQUAL(aGuid2, aGuid2);

        CPPUNIT_ASSERT(aGuid1 != aGuid2);
    }

    void testParse()
    {
        sal_uInt8 pArray1[16] = { 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 5 };
        Guid aGuid1(pArray1);

        Guid aGuid2("{01010101-0202-0303-0404-050505050505}");
        CPPUNIT_ASSERT_EQUAL(aGuid1, aGuid2);

        sal_uInt8 pArray2[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        Guid aGuid3(pArray2);

        Guid aGuid4("{FFffFFff-FFff-FFff-FFff-FFffFFffFFff}");
        CPPUNIT_ASSERT_EQUAL(aGuid3, aGuid4);

        Guid aGuid5("{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}");
        CPPUNIT_ASSERT_EQUAL(aGuid5, aGuid4);

        Guid aGuid6("01010101-0202-0303-0404-0505050505005");
        CPPUNIT_ASSERT(aGuid6.isEmpty());

        Guid aGuid7("Random");
        CPPUNIT_ASSERT(aGuid7.isEmpty());

        Guid aGuid8("{0G010101-0202-0303-0404-050505050505}");
        CPPUNIT_ASSERT(aGuid8.isEmpty());

        Guid aGuid9("{FFAAFFAA-FFAA-FFAA-FFAA-FF00FF11FF22}");
        CPPUNIT_ASSERT(!aGuid9.isEmpty());

        Guid aGuid10("{FFAAFFAA?FFAA-FFAA-FFAA-FF00FF11FF22}");
        CPPUNIT_ASSERT(aGuid10.isEmpty());
    }

    void testEmpty()
    {
        sal_uInt8 pArray1[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        Guid aGuid1(pArray1);
        CPPUNIT_ASSERT(aGuid1.isEmpty());

        sal_uInt8 pArray2[16] = { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        Guid aGuid2(pArray2);
        CPPUNIT_ASSERT(!aGuid2.isEmpty());

        Guid aGuid3;
        CPPUNIT_ASSERT(aGuid3.isEmpty());
    }

    void testCopyAndAssign()
    {
        sal_uInt8 pArray1[16] = { 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 5 };
        Guid aGuid1(pArray1);

        // test copy constructor
        Guid aGuid2(aGuid1);
        CPPUNIT_ASSERT_EQUAL(aGuid1, aGuid2);
        CPPUNIT_ASSERT(std::equal(aGuid1.cbegin(), aGuid1.cend(), aGuid2.cbegin(), aGuid2.cend()));

        // test assign
        Guid aGuid3 = aGuid1;
        CPPUNIT_ASSERT_EQUAL(aGuid3, aGuid1);
        CPPUNIT_ASSERT(std::equal(aGuid3.cbegin(), aGuid3.cend(), aGuid1.cbegin(), aGuid1.cend()));
        CPPUNIT_ASSERT_EQUAL(aGuid3, aGuid2);
        CPPUNIT_ASSERT(std::equal(aGuid3.cbegin(), aGuid3.cend(), aGuid2.cbegin(), aGuid2.cend()));
    }

    CPPUNIT_TEST_SUITE(GuidTest);
    CPPUNIT_TEST(testGetString);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testCopyAndAssign);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GuidTest);

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
