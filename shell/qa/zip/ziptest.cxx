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

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <ole2.h>
#include <stdio.h>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <string>
#include "stream_helper.hxx"
#include "testzipimpl.hxx"
using namespace std;

class Test : public CppUnit::TestFixture
{
private:
    wstring documentName;
    LPSTREAM pStream;
public:
    Test();
    void test_file_directory();
    void test_file_hasContentCaseInSensitive();
    void test_file_getContent();
    void test_stream_directory();
    void test_stream_hasContentCaseInSensitive();
    void test_stream_getContent();
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_file_directory);
    CPPUNIT_TEST(test_file_hasContentCaseInSensitive);
    CPPUNIT_TEST(test_file_getContent);
    CPPUNIT_TEST(test_stream_directory);
    CPPUNIT_TEST(test_stream_hasContentCaseInSensitive);
    CPPUNIT_TEST(test_stream_getContent);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

Test::Test() : documentName(), pStream(nullptr)
{
    const wchar_t* pSrcRoot = _wgetenv(L"SRC_ROOT");
    if (pSrcRoot)
    {
        documentName.append(pSrcRoot);
        documentName.append(L"/");
    }
    documentName.append(L"shell/qa/zip/simpledocument.odt");

    // Create an IStream pointer from the file
    HANDLE hFile = CreateFileW(documentName.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    DWORD dwFileSize = GetFileSize(hFile, nullptr);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

    LPVOID pvData = GlobalLock(hGlobal);
    DWORD dwBytesRead = 0;
    BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, nullptr);
    CPPUNIT_ASSERT_MESSAGE("FileStream: ReadFile error.", bRead);
    GlobalUnlock(hGlobal);
    CloseHandle(hFile);

    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("FileStream: CreateStreamOnHGlobal failure.", S_OK, hr);
}

void Test::test_file_directory()
{
    FileStream stream(documentName.c_str());
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_directory();
    CPPUNIT_ASSERT_MESSAGE("FileStream: Content does not match with expected directory names.", isPassed);
}

void Test::test_file_hasContentCaseInSensitive()
{
    FileStream stream(documentName.c_str());
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_hasContentCaseInSensitive();
    CPPUNIT_ASSERT_MESSAGE("FileStream: Content in zip file was not found.", isPassed);
}

void Test::test_file_getContent()
{
    FileStream stream(documentName.c_str());
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_getContent();
    CPPUNIT_ASSERT_MESSAGE("FileStream: Couldn't receive content buffer form zipfile.", isPassed);
}

void Test::test_stream_directory()
{
    BufferStream stream(pStream);
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_directory();
    CPPUNIT_ASSERT_MESSAGE("BufferStream: Content does not match with expected directory names.", isPassed);
}

void Test::test_stream_hasContentCaseInSensitive()
{
    BufferStream stream(pStream);
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_hasContentCaseInSensitive();
    CPPUNIT_ASSERT_MESSAGE("BufferStream: Content in zip file was not found.", isPassed);
}

void Test::test_stream_getContent()
{
    BufferStream stream(pStream);
    TestZipImpl testImpl(&stream);
    bool isPassed = testImpl.test_getContent();
    CPPUNIT_ASSERT_MESSAGE("BufferStream: Couldn't receive content buffer form zipfile.", isPassed);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
