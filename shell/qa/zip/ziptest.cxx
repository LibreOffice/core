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
#include "precompiled_shell.hxx"
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

//------------------------------------------------
void Test::test_directory()
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_directory();
        CPPUNIT_ASSERT_MESSAGE("Content does not match with expected directory names.", isPassed);
}

//------------------------------------------------
void Test::test_hasContentCaseInSensitive()
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_hasContentCaseInSensitive();
        CPPUNIT_ASSERT_MESSAGE("Content in zip file was not found.", isPassed);
}

//------------------------------------------------
void Test::test_getContent()
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_getContent();
        CPPUNIT_ASSERT_MESSAGE("Couldn't receive content buffer form zipfile.", isPassed);
}

//#####################################
// register test suites

CPPUNIT_PLUGIN_IMPLEMENT();

