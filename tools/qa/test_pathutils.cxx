/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_tools.hxx"
#include "sal/config.h"

#include <cwchar>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
