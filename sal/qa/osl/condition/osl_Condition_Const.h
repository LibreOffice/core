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



//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_CONDITION_CONST_H_
#define _OSL_CONDITION_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#include <sal/types.h>
#include <rtl/ustring.hxx>

#ifndef _OSL_THREAD_HXX_
#include <osl/thread.hxx>
#endif
#include <osl/mutex.hxx>
#include <osl/pipe.hxx>

#ifndef _OSL_SEMAPHOR_HXX_
#include <osl/semaphor.hxx>
#endif

#ifndef _OSL_CONDITION_HXX_
#include <osl/conditn.hxx>
#endif
#include <osl/time.h>

#ifdef UNX
#include <unistd.h>
#endif

//#include <testshl/simpleheader.hxx>

#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// condition names
//------------------------------------------------------------------------
OSLTEST_DECLARE_USTRING( TestCon,  "testcondition" );

const char pTestString[27] = "Apache Software Foundation";


#endif /* _OSL_CONDITION_CONST_H_ */
