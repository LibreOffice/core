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
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>
#include <new>

#include "osl/thread.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "dp_gui_thread.hxx"

using dp_gui::Thread;

Thread::Thread() {}

void Thread::launch() {
    // Assumption is that osl::Thread::create returns normally iff it causes
    // osl::Thread::run to start executing:
    acquire();
    try {
        create();
    } catch (...) {
        release();
        throw;
    }
}

void * Thread::operator new(std::size_t size)
    throw (std::bad_alloc)
{
    return SimpleReferenceObject::operator new(size);
}

void Thread::operator delete(void * p) throw () {
    SimpleReferenceObject::operator delete(p);
}

Thread::~Thread() {}

void Thread::run() {
    try {
        execute();
    } catch (...) {
        // Work around the problem that onTerminated is not called if run throws
        // an exception:
        onTerminated();
        throw;
    }
}

void Thread::onTerminated() {
    release();
}
