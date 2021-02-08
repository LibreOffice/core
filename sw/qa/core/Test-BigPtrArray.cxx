/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

//#define TIMELOG for measuring performance

#include <bparr.hxx>

using namespace std;

namespace /* private */
{
    const sal_uLong NUM_ENTRIES = 10;

    class BigPtrEntryMock : public BigPtrEntry
    {
    public:
        explicit BigPtrEntryMock(sal_uLong count) : count_(count)
        {
        }

        sal_uLong getCount() const
        {
            return count_;
        }

        sal_uLong Position() const
        {
            return GetPos();
        }

    private:
        sal_uLong count_;
    };

    void fillBigPtrArray(BigPtrArray& bparr, sal_uLong numEntries)
    {
        for (sal_uLong i = 0; i < numEntries; i++)
            bparr.Insert(new BigPtrEntryMock(i), bparr.Count());
    }

    bool checkElementPositions(const BigPtrArray& bparr)
    {
        for (sal_uLong i = 0; i < bparr.Count(); i++)
        {
            if (static_cast<BigPtrEntryMock*>(bparr[i])->Position() != i)
                return false;
        }
        return true;
    }

    void releaseBigPtrArrayContent(BigPtrArray const & bparr)
    {
        for (sal_uLong i = 0; i < bparr.Count(); i++)
            delete bparr[i];
    }
}

class BigPtrArrayUnittest : public CppUnit::TestFixture
{
public:

    BigPtrArrayUnittest()
    {
    }

    /** Test constructor/destructor
        The size of the BigPtrArray
        aka the 'Count' should be 0
        initially.
    */
    void test_ctor()
    {
        BigPtrArray bparr;

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "BigPtrArray ctor failed",
            static_cast<sal_uLong>(0), bparr.Count()
        );
    }

    void test_insert_entries_at_front()
    {
        BigPtrArray bparr;

        for (sal_uLong i = 0; i < NUM_ENTRIES; i++)
        {
            sal_uLong oldCount = bparr.Count();
            bparr.Insert(new BigPtrEntryMock(i), 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_insert_entries_at_front failed",
                oldCount + 1, bparr.Count()
            );
        }

        for (sal_uLong i = 0, j = NUM_ENTRIES - 1; i < NUM_ENTRIES; i++, j--)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_insert_entries_at_front failed",
                j, static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_insert_entries_at_front failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_insert_entries_in_the_middle()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        sal_uLong oldCount = bparr.Count();

        bparr.Insert(new BigPtrEntryMock(NUM_ENTRIES), bparr.Count() / 2);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_insert_entries_in_the_middle failed",
            oldCount + 1, bparr.Count()
        );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_insert_entries_in_the_middle failed",
            NUM_ENTRIES, static_cast<BigPtrEntryMock*>(bparr[bparr.Count() / 2])->getCount()
        );

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_insert_entries_in_the_middle failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_insert_at_already_used_index()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        const sal_uLong oldCount = bparr.Count();

        // insert 5 elements
        for (sal_uLong i = 0, j = 30; i < 5; i++, j++)
            bparr.Insert(new BigPtrEntryMock(j), i);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_insert_at_already_used_index failed",
            oldCount + 5, bparr.Count()
        );

        // now, first 5 elements have counts: 30,31,..34
        // next 10 elements have counts: 0,1,..9
        for (sal_uLong i = 0, j = 30; i < bparr.Count(); i++, j++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_insert_at_already_used_index failed",
                (i < 5 ? j : i - 5), static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_insert_at_already_used_index failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_insert_at_end()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        sal_uLong oldCount = bparr.Count();
        bparr.Insert(new BigPtrEntryMock(NUM_ENTRIES), bparr.Count());

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_insert_at_end failed",
            oldCount + 1, bparr.Count()
        );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_insert_at_end failed",
            NUM_ENTRIES, static_cast<BigPtrEntryMock*>(bparr[bparr.Count()-1])->getCount()
        );

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_insert_at_end failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_remove_at_front()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (sal_uLong i = 0; i < NUM_ENTRIES; i++)
        {
            sal_uLong oldCount = bparr.Count();

            delete bparr[0]; // release content
            bparr.Remove(0); // remove item from container

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_remove_at_front failed (wrong count)",
                oldCount - 1, bparr.Count()
            );

            for (sal_uLong j = 0, k = i + 1; j < bparr.Count(); j++, k++)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE
                (
                    "test_remove_at_front failed",
                    k, static_cast<BigPtrEntryMock*>(bparr[j])->getCount()
                );
            }

            CPPUNIT_ASSERT_MESSAGE
            (
                "test_remove_at_front failed",
                checkElementPositions(bparr)
            );
        }
    }

    void test_remove_at_back()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (int i = NUM_ENTRIES - 1; i >= 0; i--)
        {
            sal_uLong oldCount = bparr.Count();
            delete bparr[i];
            bparr.Remove(i);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_remove_at_back failed (wrong count)",
                (oldCount - 1), bparr.Count()
            );

            for (sal_uLong j = 0; j < bparr.Count(); j++)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE
                (
                    "test_remove_at_back failed",
                    j, static_cast<BigPtrEntryMock*>(bparr[j])->getCount()
                );
            }

            CPPUNIT_ASSERT_MESSAGE
            (
                "test_remove_at_back failed",
                checkElementPositions(bparr)
            );
        }
    }

    void test_remove_in_the_middle()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        while (bparr.Count())
        {
            sal_uLong oldCount = bparr.Count();
            sal_uLong oldElement = static_cast<BigPtrEntryMock*>(bparr[bparr.Count() / 2])->getCount();

            delete bparr[bparr.Count() / 2];
            bparr.Remove(bparr.Count() / 2);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_remove_in_the_middle failed (wrong count)",
                oldCount - 1, bparr.Count()
            );

            for (sal_uLong i = 0; i < bparr.Count(); i++)
            {
                CPPUNIT_ASSERT_MESSAGE
                (
                    "test_remove_in_the_middle failed",
                    static_cast<BigPtrEntryMock*>(bparr[i])->getCount() != oldElement
                );
            }

            CPPUNIT_ASSERT_MESSAGE
            (
                "test_remove_in_the_middle failed",
                checkElementPositions(bparr)
            );
        }
    }

    void test_remove_multiple_elements_at_once()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        while(bparr.Count())
        {
            sal_uLong nRemove = std::min<sal_uLong>(bparr.Count(), 3);
            sal_uLong oldCount = bparr.Count();

            for (sal_uLong i = 0; i < nRemove; i++)
                delete bparr[i];

            bparr.Remove(0, nRemove);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_remove_multiple_elements_at_once failed",
                oldCount - nRemove, bparr.Count()
            );

            CPPUNIT_ASSERT_MESSAGE
            (
                "test_remove_multiple_elements_at_once failed",
                checkElementPositions(bparr)
            );
        }
    }

    void test_remove_all_elements_at_once()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        releaseBigPtrArrayContent(bparr);
        bparr.Remove(0, bparr.Count());

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_remove_all_elements_at_once failed",
            static_cast<sal_uLong>(0), bparr.Count()
        );
    }

    void test_move_elements_from_lower_to_higher_pos()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (sal_uLong i = 0; i < NUM_ENTRIES - 1; i++)
        {
            bparr.Move(i, i + 2);
        }

        for (sal_uLong i = 0; i < (NUM_ENTRIES - 1); i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_move_elements_from_lower_to_higher_pos failed",
                (i + 1), static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_move_elements_from_lower_to_higher_pos failed",
            static_cast<sal_uLong>(0), static_cast<BigPtrEntryMock*>(bparr[NUM_ENTRIES -1])->getCount()
        );

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_move_elements_from_lower_to_higher_pos failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_move_elements_from_higher_to_lower_pos()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (int i = NUM_ENTRIES - 1; i >= 1; i--)
        {
            bparr.Move(i, i - 1);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "test_move_elements_from_higher_to_lower_pos failed",
            (NUM_ENTRIES - 1), static_cast<BigPtrEntryMock*>(bparr[0])->getCount()
        );

        for (sal_uLong i = 1; i < NUM_ENTRIES; i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_move_elements_from_higher_to_lower_pos failed",
                (i - 1), static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_move_elements_from_higher_to_lower_pos failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_move_to_same_position()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (sal_uLong i = 0; i < NUM_ENTRIES; i++)
        {
            bparr.Move(i, i);
        }

        for (sal_uLong i = 0; i < NUM_ENTRIES; i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_move_to_same_position failed",
                i, static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_move_to_same_position failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    void test_replace_elements()
    {
        BigPtrArray bparr;

        fillBigPtrArray(bparr, NUM_ENTRIES);

        for (sal_uLong i = 0, j = NUM_ENTRIES - 1; i < NUM_ENTRIES; i++, j--)
        {
            delete bparr[i];
            bparr.Replace(i, new BigPtrEntryMock(j));
        }

        for (sal_uLong i = 0; i < NUM_ENTRIES; i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test_replace_elements failed",
                (NUM_ENTRIES - i - 1), static_cast<BigPtrEntryMock*>(bparr[i])->getCount()
            );
        }

        CPPUNIT_ASSERT_MESSAGE
        (
            "test_replace_elements failed",
            checkElementPositions(bparr)
        );

        releaseBigPtrArrayContent(bparr);
    }

    CPPUNIT_TEST_SUITE(BigPtrArrayUnittest);
    CPPUNIT_TEST(test_ctor);
    CPPUNIT_TEST(test_insert_entries_at_front);
    CPPUNIT_TEST(test_insert_entries_in_the_middle);
    CPPUNIT_TEST(test_insert_at_already_used_index);
    CPPUNIT_TEST(test_insert_at_end);
    CPPUNIT_TEST(test_remove_at_front);
    CPPUNIT_TEST(test_remove_at_back);
    CPPUNIT_TEST(test_remove_in_the_middle);
    CPPUNIT_TEST(test_remove_multiple_elements_at_once);
    CPPUNIT_TEST(test_remove_all_elements_at_once);
    CPPUNIT_TEST(test_move_elements_from_lower_to_higher_pos);
    CPPUNIT_TEST(test_move_elements_from_higher_to_lower_pos);
    CPPUNIT_TEST(test_move_to_same_position);
    CPPUNIT_TEST(test_replace_elements);
    CPPUNIT_TEST_SUITE_END();
};

#if defined TIMELOG

const char* const START = "START: ";
const char* const END = "END: ";

class PerformanceTracer
{
public:

public:
    explicit PerformanceTracer(const string& methodName) :
        startString_(START),
        endString_(END)
    {
        startString_ += methodName;
        endString_ += methodName;
    }

    ~PerformanceTracer()
    {
    }

private:
    string startString_;
    string endString_;
};

class BigPtrArrayPerformanceTest : public CppUnit::TestFixture
{
public:
    BigPtrArrayPerformanceTest()
    {
    }

    void test_insert_at_end_1000()
    { test_insert_at_end("1000"); }

    void test_insert_at_end_10000()
    { test_insert_at_end("10000"); }

    void test_insert_at_end_100000()
    { test_insert_at_end("100000"); }

    void test_insert_at_end_1000000()
    { test_insert_at_end("1000000"); }

    void test_insert_at_front_1000()
    { test_insert_at_front("1000"); }

    void test_insert_at_front_10000()
    { test_insert_at_front("10000"); }

    void test_insert_at_front_100000()
    { test_insert_at_front("100000"); }

    void test_insert_at_front_1000000()
    { test_insert_at_front("1000000"); }

    CPPUNIT_TEST_SUITE(BigPtrArrayPerformanceTest);
    CPPUNIT_TEST(test_insert_at_end_1000);
    CPPUNIT_TEST(test_insert_at_end_10000);
    CPPUNIT_TEST(test_insert_at_end_100000);
    CPPUNIT_TEST(test_insert_at_end_1000000);
    CPPUNIT_TEST(test_insert_at_front_1000);
    CPPUNIT_TEST(test_insert_at_front_10000);
    CPPUNIT_TEST(test_insert_at_front_100000);
    CPPUNIT_TEST(test_insert_at_front_1000000);
    CPPUNIT_TEST_SUITE_END();

private:
    void test_insert_at_end(const char* numElements)
    {
        OStringBuffer buff("test_insert_at_end ");
        buff.append(numElements);
        int n = atoi(numElements);
        PerformanceTracer tracer(buff.getStr());
        BigPtrArray bparr;
        for (int i = 0; i < n; i++)
            bparr.Insert(new BigPtrEntryMock(i), bparr.Count());

        releaseBigPtrArrayContent(bparr);
    }

    void test_insert_at_front(const char* numElements)
    {
        OStringBuffer buff("test_insert_at_front ");
        buff.append(numElements);
        int n = atoi(numElements);
        PerformanceTracer tracer(buff.getStr());
        BigPtrArray bparr;
        for (int i = 0; i < n; i++)
            bparr.Insert(new BigPtrEntryMock(i), 0);

        releaseBigPtrArrayContent(bparr);
    }
};

#endif

// register test suites
CPPUNIT_TEST_SUITE_REGISTRATION(BigPtrArrayUnittest);
#ifdef TIMELOG
CPPUNIT_TEST_SUITE_REGISTRATION(BigPtrArrayPerformanceTest);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
