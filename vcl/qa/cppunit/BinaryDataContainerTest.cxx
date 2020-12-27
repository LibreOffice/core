/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/BinaryDataContainer.hxx>

using namespace css;

namespace
{
class BinaryDataContainerTest : public CppUnit::TestFixture
{
    void test();

    CPPUNIT_TEST_SUITE(BinaryDataContainerTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void BinaryDataContainerTest::test()
{
    {
        BinaryDataContainer aContainer;
        CPPUNIT_ASSERT_EQUAL(true, bool(aContainer.isEmpty()));
        CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSize());
    }
    {
        std::vector<sal_uInt8> aTestByteArray = { 1, 2, 3, 4 };
        BinaryDataContainer aContainer(aTestByteArray.data(), aTestByteArray.size());
        CPPUNIT_ASSERT_EQUAL(false, bool(aContainer.isEmpty()));
        CPPUNIT_ASSERT_EQUAL(size_t(4), aContainer.getSize());

        // Test Copy
        BinaryDataContainer aCopyOfContainer = aContainer;
        CPPUNIT_ASSERT_EQUAL(false, bool(aCopyOfContainer.isEmpty()));
        CPPUNIT_ASSERT_EQUAL(size_t(4), aCopyOfContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aCopyOfContainer.getData(), aContainer.getData());

        // Test Move
        BinaryDataContainer aMovedInContainer = std::move(aCopyOfContainer);
        CPPUNIT_ASSERT_EQUAL(false, bool(aMovedInContainer.isEmpty()));
        CPPUNIT_ASSERT_EQUAL(size_t(4), aMovedInContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aMovedInContainer.getData(), aContainer.getData());

        CPPUNIT_ASSERT_EQUAL(true, bool(aCopyOfContainer.isEmpty()));
        CPPUNIT_ASSERT_EQUAL(size_t(0), aCopyOfContainer.getSize());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryDataContainerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
