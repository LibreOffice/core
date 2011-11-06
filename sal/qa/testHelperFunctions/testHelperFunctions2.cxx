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
#include "precompiled_sal.hxx"
#include <testshl/simpleheader.hxx>
#include "stringhelper.hxx"

namespace testOfHelperFunctions
{
    class test_valueequal : public CppUnit::TestFixture
    {
    public:
        void valueequal_001();

        CPPUNIT_TEST_SUITE( test_valueequal );
        CPPUNIT_TEST( valueequal_001 );
        CPPUNIT_TEST_SUITE_END( );
    };

    void test_valueequal::valueequal_001( )
    {
        rtl::OString sValue;
        rtl::OUString suValue(rtl::OUString::createFromAscii("This is only a test of some helper functions"));
        sValue <<= suValue;
        t_print("'%s'\n", sValue.getStr());
    }

} // namespace testOfHelperFunctions

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( testOfHelperFunctions::test_valueequal, "helperFunctions" );

// -----------------------------------------------------------------------------
// This is only allowed to be in one file!
// NOADDITIONAL;
