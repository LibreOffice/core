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
#include "gtest/gtest.h"
#include <string>
#include "testimpl/testzipimpl.hxx"
using namespace std;

class Test : public ::testing::Test
{
        protected:
                string documentName;
        public:
                Test();
                void SetUp() {}
                void TearDown() {}
};

Test::Test() :
        documentName("simpledocument.odt")
{
}

//------------------------------------------------
TEST_F(Test, test_directory)
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_directory();
        ASSERT_TRUE(isPassed) << "Content does not match with expected directory names.";
}

//------------------------------------------------
TEST_F(Test, test_hasContentCaseInSensitive)
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_hasContentCaseInSensitive();
        ASSERT_TRUE(isPassed) << "Content in zip file was not found.";
}

//------------------------------------------------
TEST_F(Test, test_getContent)
{
        TestZipImpl testImpl(documentName.c_str());
        bool isPassed = testImpl.test_getContent();
        ASSERT_TRUE(isPassed) << "Couldn't receive content buffer form zipfile.";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
