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



#ifndef _RTL_TRES_HXX_
#define _RTL_TRES_HXX_


#include <rtl/tres.h>

//#include <stdarg.h>

// <namespace_rtl>
namespace rtl
{
/*==========================================================================*/

// <class_TestResult>
class TestResult
{
    // pointer to testresult structure
    rtl_TestResult* pData;

    // <private_ctors>
    TestResult();
    TestResult( const TestResult& oRes );
    // </private_ctors>

public:


    // <public_ctors>
    TestResult( const sal_Char* meth, sal_uInt32 flags = 0 )
    {
        pData = rtl_tres_create( meth, flags );
    } // </public_ctors>

    // <dtor>
    ~TestResult()
    {
        rtl_tres_destroy( pData );
    }
    // </dtor>

    // <public_methods>
    rtl_TestResult* getData()
    {
        return pData;
    }
    // <method_state>
    sal_Bool state(
                    sal_Bool tst_state,
                    const sal_Char* msg = 0,
                    const sal_Char* sub = 0,
                    sal_Bool verbose = sal_False
                    )
    {
        return pData->pFuncs->state_( pData, tst_state, msg, sub, verbose );
    } // </method_state>

    void end( sal_Char* msg = 0 )
    {
        pData->pFuncs->end_( pData, msg );
    } // </method_state>
}; // </class_TestResult>

} // </namespace_rtl>
#endif





















