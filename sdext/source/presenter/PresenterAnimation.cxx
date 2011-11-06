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
#include "precompiled_sdext.hxx"

#include "PresenterAnimation.hxx"

#include <osl/time.h>

namespace sdext { namespace presenter {

sal_uInt64 GetCurrentTime (void)
{
    TimeValue aTimeValue;
    if (osl_getSystemTime(&aTimeValue))
        return sal_uInt64(aTimeValue.Seconds * 1000.0 + aTimeValue.Nanosec / 1000000.0);
    else
        return 0;
}




PresenterAnimation::PresenterAnimation (
    const sal_uInt64 nStartDelay,
    const sal_uInt64 nTotalDuration,
    const sal_uInt64 nStepDuration)
    : mnStartTime(GetCurrentTime()+nStartDelay),
      mnTotalDuration(nTotalDuration),
      mnStepDuration(nStepDuration),
      mpStartCallbacks(),
      mpEndCallbacks()
{
}




PresenterAnimation::~PresenterAnimation (void)
{
}




sal_uInt64 PresenterAnimation::GetStartTime (void)
{
    return mnStartTime;
}




sal_uInt64 PresenterAnimation::GetEndTime (void)
{
    return mnStartTime + mnTotalDuration;
}




sal_uInt64 PresenterAnimation::GetStepDuration (void)
{
    return mnStepDuration;
}




void PresenterAnimation::AddStartCallback (const Callback& rCallback)
{
    if (mpStartCallbacks.get() == NULL)
        mpStartCallbacks.reset(new ::std::vector<Callback>());
    mpStartCallbacks->push_back(rCallback);
}




void PresenterAnimation::AddEndCallback (const Callback& rCallback)
{
    if (mpEndCallbacks.get() == NULL)
        mpEndCallbacks.reset(new ::std::vector<Callback>());
    mpEndCallbacks->push_back(rCallback);
}



void PresenterAnimation::RunStartCallbacks (void)
{
    if (mpStartCallbacks.get() != NULL)
    {
        ::std::vector<Callback>::const_iterator iCallback;
        for (iCallback=mpStartCallbacks->begin(); iCallback!=mpStartCallbacks->end(); ++iCallback)
            (*iCallback)();
    }
}




void PresenterAnimation::RunEndCallbacks (void)
{
    if (mpEndCallbacks.get() != NULL)
    {
        ::std::vector<Callback>::const_iterator iCallback;
        for (iCallback=mpEndCallbacks->begin(); iCallback!=mpEndCallbacks->end(); ++iCallback)
            (*iCallback)();
    }
}




} } // end of namespace ::sdext::presenter
