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
        CPPUNIT_ASSERT_MESSAGE("Couldn't recieve content buffer form zipfile.", isPassed);
}

//#####################################
// register test suites

CPPUNIT_PLUGIN_IMPLEMENT();

