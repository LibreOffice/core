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


#ifndef _SOLTOOLS_TESTSHL_TSTMGR_HXX__
#define _SOLTOOLS_TESTSHL_TSTMGR_HXX__

#include    <sal/types.h>

#include <vector>

using namespace std;

// <namespace_tstutl>
namespace tstutl {

// <class_tstMgr>
class tstMgr {

    // <private_members>
    struct tstMgr_Impl;
    tstMgr_Impl* pImpl;
    // </private_members>

    // <private_methods>
    void cleanup();
    // </private_methods>

public:

    // <dtor>
    ~tstMgr(){
        cleanup();
    } // </dtor>


    // <public_methods>
    sal_Bool initialize( sal_Char* moduleName, sal_Bool boom = sal_False );
    sal_Bool test_Entry( sal_Char* entry, sal_Char* logName = 0 );
    sal_Bool test_Entries( vector< sal_Char* > entries, sal_Char* logName = 0 );
    sal_Bool test_EntriesFromFile( sal_Char* fName, sal_Char* logName = 0 );
    // </public_methods>

}; // </class_tstMgr>

} // </namespace_tstutl>

#endif



