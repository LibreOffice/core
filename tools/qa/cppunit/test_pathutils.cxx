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

#include <cwchar>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/pathutils.hxx>

namespace {

void buildPath(
    wchar_t const * front, wchar_t const * back, wchar_t const * path)
{
#if defined(_WIN32)
    wchar_t p[MAX_PATH];
    wchar_t * e = tools::buildPath(
        p, front, front + std::wcslen(front), back, std::wcslen(back));
    CPPUNIT_ASSERT_EQUAL(static_cast<void *>(p + std::wcslen(path)), static_cast<void *>(e));
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
