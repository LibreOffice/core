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
#include "precompiled_filter.hxx"

#include "lateinitthread.hxx"

//_______________________________________________
// includes

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    14.08.2003 09:31
-----------------------------------------------*/
LateInitThread::LateInitThread()
{
}

/*-----------------------------------------------
    14.08.2003 08:42
-----------------------------------------------*/
LateInitThread::~LateInitThread()
{
}

/*-----------------------------------------------
    28.10.2003 09:30
-----------------------------------------------*/
void SAL_CALL LateInitThread::run()
{
    // sal_True => It indicates using of this method by this thread
    // The filter cache use this information to show an assertion
    // for "optimization failure" in case the first calli of loadAll()
    // was not this thread ...

    // Further please dont catch any exception here.
    // May be they show the problem of a corrupted filter
    // configuration, which is handled inside our event loop or desktop.main()!

    ::salhelper::SingletonRef< FilterCache > rCache;
    rCache->load(FilterCache::E_CONTAINS_ALL, sal_True);
}

void SAL_CALL LateInitThread::onTerminated()
{
    delete this;
}

    } // namespace config
} // namespace filter
