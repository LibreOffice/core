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


#include "precompiled_sw.hxx"
#include <swthreadjoiner.hxx>
#include <finalthreadmanager.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

/** Testing

    @author OD
*/
using namespace ::com::sun::star;

uno::Reference< util::XJobManager > SwThreadJoiner::mpThreadJoiner;
osl::Mutex* SwThreadJoiner::mpGetJoinerMutex = new osl::Mutex();

uno::Reference< util::XJobManager >& SwThreadJoiner::GetThreadJoiner()
{
    osl::MutexGuard aGuard(*mpGetJoinerMutex);

    if ( !mpThreadJoiner.is() )
    {
        mpThreadJoiner =
            uno::Reference< util::XJobManager >(
                ::comphelper::getProcessServiceFactory()->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.util.JobManager" ) ),
                uno::UNO_QUERY );
    }

    return mpThreadJoiner;
}

void SwThreadJoiner::ReleaseThreadJoiner()
{
    mpThreadJoiner.clear();
}
