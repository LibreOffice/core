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

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------
#include <stdio.h>
#include <osl/profile.h>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

//==================================================================================================
// -----------------------------------------------------------------------------
namespace osl_Profile
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_profile();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_profile );
        CPPUNIT_TEST_SUITE_END( );
    };

void oldtests::test_profile(void)
{
    oslProfile hProfile;
    rtl_uString* ustrProfileName=0;
    rtl_uString* ustrProfileName2=0;

    rtl_uString_newFromAscii(&ustrProfileName,"//./tmp/soffice.ini");
    rtl_uString_newFromAscii(&ustrProfileName2,"//./tmp/not_existing_path/soffice.ini");


    // successful write
    hProfile = osl_openProfile( ustrProfileName, 0 );
    if (hProfile != 0)
    {
        if (! osl_writeProfileBool( hProfile, "testsection", "testbool", 1 ))
            printf( "### cannot write into init file!\n" );

        osl_closeProfile( hProfile );
    }

    // unsuccessful write
    hProfile = osl_openProfile( ustrProfileName2, 0 );
    if (hProfile != 0)
    {
        if (osl_writeProfileBool( hProfile, "testsection", "testbool", 1 ))
            printf( "### unexpected success writing into test2.ini!\n" );

        osl_closeProfile( hProfile );
    }

    rtl_uString_release(ustrProfileName);
    rtl_uString_release(ustrProfileName2);
}

} // namespace osl_Profile

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( osl_Profile::oldtests );

// -----------------------------------------------------------------------------
CPPUNIT_PLUGIN_IMPLEMENT();
