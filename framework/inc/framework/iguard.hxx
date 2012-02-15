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



#ifndef __FRAMEWORK_THREADHELP_IGUARD_H_
#define __FRAMEWORK_THREADHELP_IGUARD_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <sal/types.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          interface for guarding a lock
*//*-*************************************************************************************************************/
class SAL_NO_VTABLE IGuard
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /** clears the lock. If the guard does not currently hold the lock, nothing happens.
        */
        virtual void clear() = 0;

        /** attempts to re-establishes the lock, blocking until the attempt is successful.
        */
        virtual void reset() = 0;

};      //  class IGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IGUARD_H_
