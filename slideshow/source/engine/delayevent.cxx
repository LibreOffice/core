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
#include "precompiled_slideshow.hxx"

#include <osl/diagnose.h>
#include "delayevent.hxx"

namespace slideshow {
namespace internal {

bool Delay::fire()
{
    OSL_ASSERT( isCharged() );
    if (isCharged()) {
        mbWasFired = true;
        maFunc();
        maFunc.clear(); // early release of payload
    }
    return true;
}

bool Delay::isCharged() const
{
    return !mbWasFired;
}

double Delay::getActivationTime( double nCurrentTime ) const
{
    return nCurrentTime + mnTimeout;
}

void Delay::dispose()
{
    // don't clear unconditionally, because it may currently be executed:
    if (isCharged()) {
        mbWasFired = true;
        maFunc.clear(); // release of payload
    }
}

} // namespace internal
} // namespace presentation

