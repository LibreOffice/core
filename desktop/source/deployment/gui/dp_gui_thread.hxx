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



#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX

#include "sal/config.h"

#include <cstddef>
#include <new>
#include "osl/thread.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

/// @HTML

namespace dp_gui {

/**
   A safe encapsulation of <code>osl::Thread</code>.
*/
class Thread: public salhelper::SimpleReferenceObject, private osl::Thread {
public:
    Thread();

    /**
       Launch the thread.

       <p>This function must be called at most once.</p>
    */
    void launch();

    using osl::Thread::join;

    static void * operator new(std::size_t size) throw (std::bad_alloc);

    static void operator delete(void * p) throw ();

protected:
    virtual ~Thread();

    /**
       The main function executed by the thread.

       <p>Any exceptions terminate the thread and are effectively ignored.</p>
    */
    virtual void execute() = 0;

private:
    Thread(Thread &); // not defined
    void operator =(Thread &); // not defined

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();
};

}

#endif
