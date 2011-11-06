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



#include "precompiled_tools.hxx"
#include "sal/config.h"

#include <cwchar>

#include "testshl/simpleheader.hxx"
#include "tools/pathutils.hxx"

namespace {

void buildPath(
    wchar_t const * front, wchar_t const * back, wchar_t const * path)
{
#if defined WNT
    wchar_t p[MAX_PATH];
    wchar_t * e = tools::buildPath(
        p, front, front + std::wcslen(front), back, std::wcslen(back));
    CPPUNIT_ASSERT_EQUAL(p + std::wcslen(path), e);
    CPPUNIT_ASSERT_EQUAL(0, std::wcscmp(path, p));
#else
    (void) front;
    (void) back;
    (void) path;
#endif
}

class Test: public CppUnit::TestFixture {
public:
    void testBuildPath();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testBuildPath);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testBuildPath() {
    buildPath(L"a:\\b\\", L"..", L"a:\\");
    buildPath(L"a:\\b\\", L"..\\", L"a:\\");
    buildPath(L"a:\\b\\c\\", L"..\\..\\..\\d", L"a:\\..\\d");
    buildPath(L"\\\\a\\b\\", L"..\\..\\..\\c", L"\\\\..\\c");
    buildPath(L"\\", L"..\\a", L"\\..\\a");
    buildPath(L"", L"..\\a", L"..\\a");
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
