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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <osl_Socket_Const.h>

#include "gtest/gtest.h"
#include <osl/socket.hxx>
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// tests cases begins here
//------------------------------------------------------------------------

namespace osl_Socket
{

    class tests : public ::testing::Test
    {
    public:
    };

    TEST_F(tests, test_001)
        {
            // _osl_getFullQualifiedDomainName( );
            oslSocketResult aResult;
            rtl::OUString suHostname = osl::SocketAddr::getLocalHostname(&aResult);
            ASSERT_TRUE(aResult == osl_Socket_Ok) << "getLocalHostname failed";
        }
}


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.

/*#if (defined LINUX)

void RegisterAdditionalFunctions( FktRegFuncPtr _pFunc )
{
    // for cover lines in _osl_getFullQualifiedDomainName( )
    // STAR_OVERRIDE_DOMAINNAME is more an internal HACK for 5.2, which should remove from sal
    setenv( "STAR_OVERRIDE_DOMAINNAME", "PRC.Sun.COM", 0 );
}

#else*/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

//#endif
