/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <vcl/BinaryDataContainer.hxx>

using namespace css;

namespace
{
class BinaryDataContainerTest : public CppUnit::TestFixture
{
    void testConstruct();
    void testSizeHolderCount();

    CPPUNIT_TEST_SUITE(BinaryDataContainerTest);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testSizeHolderCount);
    CPPUNIT_TEST_SUITE_END();
};

void BinaryDataContainerTest::testConstruct()
{
    {
        BinaryDataContainer aContainer;
        CPPUNIT_ASSERT(aContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSize());
    }
    {
        // construct a data array
        sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
        SvMemoryStream stream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);

        BinaryDataContainer aContainer(stream, std::size(aTestByteArray));

        CPPUNIT_ASSERT(!aContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aContainer.getSize());

        // Test Copy
        BinaryDataContainer aCopyOfContainer = aContainer;
        CPPUNIT_ASSERT(!aCopyOfContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aCopyOfContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aCopyOfContainer.getData(), aContainer.getData());

        // Test Move
        BinaryDataContainer aMovedInContainer = std::move(aCopyOfContainer);
        CPPUNIT_ASSERT(!aMovedInContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aMovedInContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aMovedInContainer.getData(), aContainer.getData());

        CPPUNIT_ASSERT(aCopyOfContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aCopyOfContainer.getSize());
    }
}

// the size holder count follows the registered holders, not the number of container copies
void BinaryDataContainerTest::testSizeHolderCount()
{
    {
        BinaryDataContainer aEmpty;
        CPPUNIT_ASSERT_EQUAL(size_t(0), aEmpty.getSizeHolderCount());
        aEmpty.addSizeHolder();
        CPPUNIT_ASSERT_EQUAL(size_t(0), aEmpty.getSizeHolderCount());
    }

    sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
    SvMemoryStream aStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);
    BinaryDataContainer aContainer(aStream, std::size(aTestByteArray));
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSizeHolderCount());

    // copies alone do not count as holders
    BinaryDataContainer aCopy = aContainer;
    CPPUNIT_ASSERT_EQUAL(size_t(0), aCopy.getSizeHolderCount());

    // a holder registered through one copy is visible through every copy of the same bytes
    aContainer.addSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aContainer.getSizeHolderCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aCopy.getSizeHolderCount());

    aCopy.addSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aContainer.getSizeHolderCount());

    aContainer.removeSizeHolder();
    aCopy.removeSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSizeHolderCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), aCopy.getSizeHolderCount());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryDataContainerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
