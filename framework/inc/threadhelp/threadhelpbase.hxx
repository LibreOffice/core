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



#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#define __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/lockhelper.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          "baseclass" to make own classes threadsafe
    @descr          Sometimes you must share your lock- or mutex member with any other baseclasses.
                    And baseclasses are initialized erlier then members! That's why you should use
                    this struct as first of your baseclasses!!!
                    Then you will get a public member "m_aLock" which can be used by special guard implementations
                    to make your code threadsafe.

    @seealso        class LockHelper

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct ThreadHelpBase
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        ThreadHelpBase( ::vos::IMutex* pSolarMutex = NULL )
                :   m_aLock( pSolarMutex )
        {
        }

    //-------------------------------------------------------------------------------------------------------------
    //  public member
    //  Make it mutable for using in const functions!
    //-------------------------------------------------------------------------------------------------------------
    public:

        mutable LockHelper m_aLock;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
