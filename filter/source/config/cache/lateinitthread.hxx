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



#ifndef __FILTER_CONFIG_LATEINITTHREAD_HXX_
#define __FILTER_CONFIG_LATEINITTHREAD_HXX_

//_______________________________________________
// includes

#include "filtercache.hxx"
#include <salhelper/singletonref.hxx>
#include <osl/thread.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements a thread, which will update the
                global filter cache of an office, after its
                startup was finished.

    @descr      Its started by a LateInitListener instance ...

    @see        LateInitListener

    @attention  The filter cache will be blocked during this thrad runs!
 */
class LateInitThread : public ::osl::Thread
{
    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  initialize new instance of this class.
         */
        LateInitThread();

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~LateInitThread();

        //---------------------------------------

        /** @short  thread function.
         */
        virtual void SAL_CALL run();

        virtual void SAL_CALL onTerminated();
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_LATEINITTHREAD_HXX_
