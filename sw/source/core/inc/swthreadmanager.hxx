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


#ifndef _SWTHREADMANAGER_HXX
#define _SWTHREADMANAGER_HXX

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>

#include <boost/utility.hpp>
#include <memory>


class ObservableThread;
class ThreadManager;

/** class to manage threads in Writer using a <ThreadManager> instance

    OD 2007-04-13 #i73788#
    Conforms the singleton pattern

    @author OD
*/
class SwThreadManager
    : private ::boost::noncopyable
{
    public:

        static SwThreadManager& GetThreadManager();

        static bool ExistsThreadManager();

        // private: don't call!
        SwThreadManager();
        // private: don't call!
        ~SwThreadManager();

        oslInterlockedCount AddThread( const rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.

            @author OD
        */
        void SuspendStartingOfThreads();

        /** continues the starting of threads after it has been suspended

            @author OD
        */
        void ResumeStartingOfThreads();

        bool StartingOfThreadsSuspended();

    private:
        static bool mbThreadManagerInstantiated;

        ::std::auto_ptr<ThreadManager> mpThreadManagerImpl;

};
#endif
