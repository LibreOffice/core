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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <string>
#include "testimpl/testzipimpl.hxx"
using namespace std;

class Test : public CppUnit::TestFixture
{
private:
    string documentName;
public:
    Test();
    void setUp() {}
    void tearDown() {}
    void test_directory();
    void test_hasContentCaseInSensitive();
    void test_getContent();
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_directory);
    CPPUNIT_TEST(test_hasContentCaseInSensitive);
    CPPUNIT_TEST(test_getContent);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

Test::Test() :
		documentName("simpledocument.odt")
{
}

void Test::test_directory()
{
    TestZipImpl testImpl(documentName.c_str());
    bool isPassed = testImpl.test_directory();
    CPPUNIT_ASSERT_MESSAGE("Content does not match with expected directory names.", isPassed);
}

void Test::test_hasContentCaseInSensitive()
{
    TestZipImpl testImpl(documentName.c_str());
    bool isPassed = testImpl.test_hasContentCaseInSensitive();
    CPPUNIT_ASSERT_MESSAGE("Content in zip file was not found.", isPassed);
}

void Test::test_getContent()
{
		TestZipImpl testImpl(documentName.c_str());
		bool isPassed = testImpl.test_getContent();
		CPPUNIT_ASSERT_MESSAGE("Couldn't recieve content buffer form zipfile.", isPassed);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
