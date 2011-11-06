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



#ifndef __FRAMEWORK_OMUTEXMEMBER_HXX_
#define __FRAMEWORK_OMUTEXMEMBER_HXX_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          definition of a public mutex member
    @descr          You can use this struct as baseclass to get a public mutex member for right initialization.
                    Don't use it as member. You can't guarantee the right order of initialization of baseclasses then!
                    And some other helper classes share the mutex with an implementation and must have a valid one.

    @seealso        See implementation of constructors in derived classes for further informations!

    @devstatus      ready
*//*-*************************************************************************************************************/

struct OMutexMember
{
    ::osl::Mutex    m_aMutex;
};

#endif  //  #ifndef __FRAMEWORK_OMUTEXMEMBER_HXX_
