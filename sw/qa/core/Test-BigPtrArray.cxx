/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#define TIMELOG

#include "gtest/gtest.h"
#include <bparr.hxx>
#include <string>
#include <stdlib.h>
#include <rtl/logfile.hxx>

#undef ENABLE_DEBUG_OUTPUT

// FIXME:
#define RUN_OLD_FAILING_TESTS 0

using namespace std;


namespace /* private */
{
    const int NUM_ENTRIES = 1000;

    class BigPtrEntryMock : public BigPtrEntry
    {
    public:
        BigPtrEntryMock(int count) : count_(count)
        {
        }

        ~BigPtrEntryMock()
        {
        #ifdef ENABLE_DEBUG_OUTPUT
            printf("Destructor called (%i)\n", count_);
        #endif
        }

        int getCount() const
        {
            return count_;
        }

        void setCount(int newCount)
        {
            count_ = newCount;
        }

        sal_uLong Position() const
        {
            return GetPos();
        }

    private:
        int count_;
    };

    sal_Bool AddToCount(const ElementPtr& rElem, void* pArgs)
    {
        BigPtrEntryMock* const pbem = static_cast<BigPtrEntryMock* const>(rElem);
        pbem->setCount(pbem->getCount() + *((int*)pArgs));
        return true;
    }

    void dumpBigPtrArray(const BigPtrArray& bparr)
    {
    #ifdef ENABLE_DEBUG_OUTPUT
        for (int i = 0; i < bparr.Count(); i++)
            printf("bparr[%i,%i]: %i\n", i, static_cast<BigPtrEntryMock*>(bparr[i])->Position(), static_cast<BigPtrEntryMock*>(bparr[i])->getCount());

        printf("\n");
    #endif
    }

    void fillBigPtrArray(BigPtrArray& bparr, sal_uLong numEntries)
    {
        for (int i = 0; i < numEntries; i++)
            bparr.Insert(new BigPtrEntryMock(i), bparr.Count());
    }

    void printMethodName(const char* name)
    {
    #ifdef ENABLE_DEBUG_OUTPUT
        printf(name);
    #endif
    }

    bool checkElementPositions(const BigPtrArray& bparr)
    {
        for (int i = 0; i < bparr.Count(); i++)
        {
            if (static_cast<BigPtrEntryMock*>(bparr[i])->Position() != i)
                return false;
        }
        return true;
    }

    void releaseBigPtrArrayContent(BigPtrArray& bparr)
    {
        for (int i = 0; i < bparr.Count(); i++)
            delete bparr[i];
    }

    RTL_LOGFILE_CONTEXT(logFile, "BigPtrArray performance measures" );
}

class BigPtrArrayUnittest : public ::testing::Test
{
public:

    BigPtrArrayUnittest()
    {
    }
};

/** Test constructor/destructor
    The size of the BigPtrArray
    aka the 'Count' should be 0
    initially.
*/
TEST_F(BigPtrArrayUnittest, test_ctor)
{
    printMethodName("test_ctor\n");

    BigPtrArray bparr;

    ASSERT_TRUE(bparr.Count() == 0) << "BigPtrArray ctor failed";;
}

TEST_F(BigPtrArrayUnittest, test_insert_entries_at_front)
{
    printMethodName("test_insert_entries_at_front\n");

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "START: test_insert_entries_at_front");

    BigPtrArray bparr;

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        sal_uLong oldCount = bparr.Count();
        bparr.Insert(new BigPtrEntryMock(i), 0);
        ASSERT_TRUE((bparr.Count() == oldCount + 1)) << "test_insert_entries_at_front failed";
    }

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "END: test_insert_entries_at_front");

    for (int i = 0, j = NUM_ENTRIES - 1; i < NUM_ENTRIES; i++, j--)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == j) << "test_insert_entries_at_front failed";
    }

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_insert_entries_at_front failed";

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_insert_entries_in_the_middle)
{
    printMethodName("test_insert_entries_in_the_middle\n");

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "START: test_insert_entries_in_the_middle");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    sal_uLong oldCount = bparr.Count();

    bparr.Insert(new BigPtrEntryMock(NUM_ENTRIES), bparr.Count() / 2);

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "END: test_insert_entries_in_the_middle");

    ASSERT_TRUE((oldCount + 1 == bparr.Count() && static_cast<BigPtrEntryMock*>(bparr[bparr.Count() / 2])->getCount() == NUM_ENTRIES)) << "test_insert_entries_in_the_middle failed";

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_insert_entries_in_the_middle failed";

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_insert_at_already_used_index)
{
    printMethodName("test_insert_at_already_used_index\n");

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "START: test_insert_at_already_used_index");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    int oldCount = bparr.Count();

    for (int i = 0, j = -5; i < 5; i++, j++)
        bparr.Insert(new BigPtrEntryMock(j), i);

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "END: test_insert_at_already_used_index");

    ASSERT_TRUE((oldCount + 5 == bparr.Count())) << "test_insert_at_already_used_index failed";

    for (int i = 0, j = -5; i < bparr.Count(); i++, j++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == j) << "test_insert_at_already_used_index failed";
    }

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_insert_at_already_used_index failed";

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_insert_at_end)
{
    printMethodName("test_insert_at_end\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    sal_uLong oldCount = bparr.Count();
    bparr.Insert(new BigPtrEntryMock(NUM_ENTRIES), bparr.Count());

    ASSERT_TRUE((oldCount + 1 == bparr.Count() && static_cast<BigPtrEntryMock*>(bparr[bparr.Count()-1])->getCount() == NUM_ENTRIES)) << "test_insert_at_end failed";

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_insert_at_end failed";

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_remove_at_front)
{
    printMethodName("test_remove_at_front\n");

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "START: test_remove_at_front");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        sal_uLong oldCount = bparr.Count();

        delete bparr[0]; // release content
        bparr.Remove(0); // remove item from container

        ASSERT_TRUE((oldCount - 1 == bparr.Count())) << "test_remove_at_front failed (wrong count)";

        for (int j = 0, k = i + 1; j < bparr.Count(); j++, k++)
        {
            ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[j])->getCount() == k) << "test_remove_at_front failed";
        }

        ASSERT_TRUE(checkElementPositions(bparr)) << "test_remove_at_front failed";

        dumpBigPtrArray(bparr);
    }

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "END: test_remove_at_front");
}

TEST_F(BigPtrArrayUnittest, test_remove_at_back)
{
    printMethodName("test_remove_at_back\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = NUM_ENTRIES - 1; i >= 0; i--)
    {
        sal_uLong oldCount = bparr.Count();
        delete bparr[i];
        bparr.Remove(i);

        ASSERT_TRUE((oldCount - 1 == bparr.Count())) << "test_remove_at_back failed (wrong count)";

        for (int j = 0; j < bparr.Count(); j++)
        {
            ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[j])->getCount() == j) << "test_remove_at_back failed";
        }

        ASSERT_TRUE(checkElementPositions(bparr)) << "test_remove_at_back failed";

        dumpBigPtrArray(bparr);
    }
}

TEST_F(BigPtrArrayUnittest, test_remove_in_the_middle)
{
    printMethodName("test_remove_in_the_middle\n");

    RTL_LOGFILE_CONTEXT_TRACE(logFile, "START: test_remove_in_the_middle");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    while (bparr.Count())
    {
        sal_uLong oldCount = bparr.Count();
        int oldElement = static_cast<BigPtrEntryMock*>(bparr[bparr.Count() / 2])->getCount();

        delete bparr[bparr.Count() / 2];
        bparr.Remove(bparr.Count() / 2);

        ASSERT_TRUE((oldCount - 1 == bparr.Count())) << "test_remove_in_the_middle failed (wrong count)";

        for (int i = 0; i < bparr.Count(); i++)
        {
            ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() != oldElement) << "test_remove_in_the_middle failed";
        }

        ASSERT_TRUE(checkElementPositions(bparr)) << "test_remove_in_the_middle failed";

        dumpBigPtrArray(bparr);
    }
    RTL_LOGFILE_CONTEXT_TRACE(logFile, "END: test_remove_in_the_middle");
}

TEST_F(BigPtrArrayUnittest, test_remove_multiple_elements_at_once)
{
    printMethodName("test_remove_multiple_elements_at_once\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    while(bparr.Count())
    {
        sal_uLong nRemove = (bparr.Count() > 3) ? 3 : bparr.Count();
        sal_uLong oldCount = bparr.Count();

        for (int i = 0; i < nRemove; i++)
            delete bparr[i];

        bparr.Remove(0, nRemove);

        ASSERT_TRUE((oldCount - nRemove == bparr.Count())) << "test_remove_multiple_elements_at_once failed";

        ASSERT_TRUE(checkElementPositions(bparr)) << "test_remove_multiple_elements_at_once failed";

        dumpBigPtrArray(bparr);
    }
}

TEST_F(BigPtrArrayUnittest, test_remove_all_elements_at_once)
{
    printMethodName("test_remove_all_elements_at_once\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    releaseBigPtrArrayContent(bparr);
    bparr.Remove(0, bparr.Count());

    ASSERT_TRUE(bparr.Count() == 0) << "test_remove_all_elements_at_once failed";

    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_move_elements_from_lower_to_higher_pos)
{
    printMethodName("test_move_elements_from_lower_to_higher_pos\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = 0; i < NUM_ENTRIES - 1; i++)
    {
        bparr.Move(i, i + 2);
        dumpBigPtrArray(bparr);
    }

    for (int i = 0; i < (NUM_ENTRIES - 1); i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i + 1)) << "test_move_elements_from_lower_to_higher_pos failed";
    }

    ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[NUM_ENTRIES -1])->getCount() == 0) << "test_move_elements_from_lower_to_higher_pos failed";

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_move_elements_from_lower_to_higher_pos failed";

    releaseBigPtrArrayContent(bparr);
}

TEST_F(BigPtrArrayUnittest, test_move_elements_from_higher_to_lower_pos)
{
    printMethodName("test_move_elements_from_higher_to_lower_pos\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = NUM_ENTRIES - 1; i >= 1; i--)
    {
        bparr.Move(i, i - 1);
        dumpBigPtrArray(bparr);
    }

    ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[0])->getCount() == (NUM_ENTRIES - 1)) << "test_move_elements_from_higher_to_lower_pos failed";

    for (int i = 1; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i - 1)) << "test_move_elements_from_higher_to_lower_pos failed";
    }

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_move_elements_from_higher_to_lower_pos failed";

    releaseBigPtrArrayContent(bparr);
}

TEST_F(BigPtrArrayUnittest, test_move_to_same_position)
{
    printMethodName("test_move_to_same_position\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        bparr.Move(i, i);
    }

#if RUN_OLD_FAILING_TESTS
    ASSERT_TRUE(checkElementPositions(bparr)) << "test_move_to_same_position failed";
#endif

    dumpBigPtrArray(bparr);

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == i) << "test_move_to_same_position failed";
    }

#if RUN_OLD_FAILING_TESTS
    ASSERT_TRUE(checkElementPositions(bparr)) << "test_move_to_same_position failed";
#endif

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_replace_elements)
{
    printMethodName("test_replace_elements\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    for (int i = 0, j = NUM_ENTRIES - 1; i < NUM_ENTRIES; i++, j--)
    {
        delete bparr[i];
        bparr.Replace(i, new BigPtrEntryMock(j));
        dumpBigPtrArray(bparr);
    }

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (NUM_ENTRIES - i - 1)) << "test_replace_elements failed";
    }

    ASSERT_TRUE(checkElementPositions(bparr)) << "test_replace_elements failed";

    releaseBigPtrArrayContent(bparr);
}

TEST_F(BigPtrArrayUnittest, test_for_each)
{
    printMethodName("test_for_each\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    int addCount = 1;
    bparr.ForEach(AddToCount, &addCount);

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i+1)) << "test_for_each failed";
    }

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_for_some1)
{
     printMethodName("test_for_some1\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    int addCount = 1;
    bparr.ForEach(0, NUM_ENTRIES / 2, AddToCount, &addCount);

    int i = 0;
    for (/* */; i < NUM_ENTRIES / 2; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i+1)) << "test_for_some1 failed";
    }

    for (/* */; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i)) << "test_for_some1 failed";
    }

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_for_some2)
{
    printMethodName("test_for_some2\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    int addCount = 1;
    bparr.ForEach(NUM_ENTRIES / 2, NUM_ENTRIES, AddToCount, &addCount);

    int i = 0;
    for (/* */; i < NUM_ENTRIES / 2; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i)) << "test_for_some2 failed";
    }

    for (/* */; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == (i+1)) << "test_for_some2 failed";
    }

    releaseBigPtrArrayContent(bparr);
    dumpBigPtrArray(bparr);
}

TEST_F(BigPtrArrayUnittest, test_for_some3)
{
    printMethodName("test_for_some3\n");

    BigPtrArray bparr;

    fillBigPtrArray(bparr, NUM_ENTRIES);
    dumpBigPtrArray(bparr);

    int addCount = 1;
    bparr.ForEach(0, 0, AddToCount, &addCount);

    for (int i = 0; i < NUM_ENTRIES; i++)
    {
        ASSERT_TRUE(static_cast<BigPtrEntryMock*>(bparr[i])->getCount() == i) << "test_for_some3 failed";
    }
    releaseBigPtrArrayContent(bparr);
}


const char* START = "START: ";
const char* END = "END: ";

class PerformanceTracer
{
public:

public:
    PerformanceTracer(const string& methodName) :
        startString_(START),
        endString_(END)
    {
        startString_ += methodName;
        endString_ += methodName;
        RTL_LOGFILE_CONTEXT_TRACE(logFile, startString_.c_str());
    }

    ~PerformanceTracer()
    {
        RTL_LOGFILE_CONTEXT_TRACE(logFile, endString_.c_str());
    }

private:
    string startString_;
    string endString_;
};

class BigPtrArrayPerformanceTest : public ::testing::Test
{
public:
    BigPtrArrayPerformanceTest()
    {
    }

protected:
    void test_insert_at_end(const char* numElements)
    {
        char buff[100] = { 0 };
        strcat(buff, "test_insert_at_end ");
        strcat(buff, numElements);
        int n = atoi(numElements);
        PerformanceTracer tracer(buff);
        BigPtrArray bparr;
        for (int i = 0; i < n; i++)
            bparr.Insert(new BigPtrEntryMock(i), bparr.Count());

        releaseBigPtrArrayContent(bparr);
    }

    void test_insert_at_front(const char* numElements)
    {
        char buff[100] = { 0 };
        strcat(buff, "test_insert_at_front ");
        strcat(buff, numElements);
        int n = atoi(numElements);
        PerformanceTracer tracer(buff);
        BigPtrArray bparr;
        for (int i = 0; i < n; i++)
            bparr.Insert(new BigPtrEntryMock(i), 0);

        releaseBigPtrArrayContent(bparr);
    }
};

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_end_1000)
{ test_insert_at_end("1000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_end_10000)
{ test_insert_at_end("10000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_end_100000)
{ test_insert_at_end("100000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_end_1000000)
{ test_insert_at_end("1000000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_front_1000)
{ test_insert_at_front("1000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_front_10000)
{ test_insert_at_front("10000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_front_100000)
{ test_insert_at_front("100000"); }

TEST_F(BigPtrArrayPerformanceTest, test_insert_at_front_1000000)
{ test_insert_at_front("1000000"); }


//#####################################

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
