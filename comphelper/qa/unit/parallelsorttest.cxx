/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/parallelsort.hxx>
#include <comphelper/threadpool.hxx>
#include <rtl/string.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>

class ParallelSortTest : public CppUnit::TestFixture
{
public:
    void testSortTiny();
    void testSortMedium();
    void testSortBig();

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ParallelSortTest);
    CPPUNIT_TEST(testSortTiny);
    CPPUNIT_TEST(testSortMedium);
    CPPUNIT_TEST(testSortBig);
    CPPUNIT_TEST_SUITE_END();

private:
    void sortTest(size_t nLen);
    void fillRandomUptoN(std::vector<size_t>& rVector, size_t N);

    comphelper::ThreadPool* pThreadPool;
    size_t mnThreads;
};

void ParallelSortTest::setUp()
{
    pThreadPool = &comphelper::ThreadPool::getSharedOptimalPool();
    mnThreads = pThreadPool->getWorkerCount();
}

void ParallelSortTest::tearDown()
{
    if (pThreadPool)
        pThreadPool->joinThreadsIfIdle();
}

void ParallelSortTest::fillRandomUptoN(std::vector<size_t>& rVector, size_t N)
{
    rVector.resize(N);
    for (size_t nIdx = 0; nIdx < N; ++nIdx)
        rVector[nIdx] = nIdx;
    std::shuffle(rVector.begin(), rVector.end(), std::default_random_engine(42));
}

void ParallelSortTest::sortTest(size_t nLen)
{
    std::vector<size_t> aVector(nLen);
    fillRandomUptoN(aVector, nLen);
    comphelper::parallelSort(aVector.begin(), aVector.end());
    for (size_t nIdx = 0; nIdx < nLen; ++nIdx)
    {
        OString aMsg = "Wrong aVector[" + OString::number(nIdx) + "]";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nIdx, aVector[nIdx]);
    }
}

void ParallelSortTest::testSortTiny()
{
    sortTest(5);
    sortTest(15);
    sortTest(16);
    sortTest(17);
}

void ParallelSortTest::testSortMedium()
{
    sortTest(1025);
    sortTest(1029);
    sortTest(1024 * 2 + 1);
    sortTest(1024 * 2 + 9);
}

void ParallelSortTest::testSortBig() { sortTest(1024 * 16 + 3); }

CPPUNIT_TEST_SUITE_REGISTRATION(ParallelSortTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
