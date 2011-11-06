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
#include "precompiled_sd.hxx"

#include "tools/AsynchronousCall.hxx"

#include <boost/bind.hpp>
#include "DrawViewShell.hxx"

namespace sd { namespace tools {

AsynchronousCall::AsynchronousCall (void)
    : maTimer(),
      mpFunction()
{
    Link aCallback (LINK(this,AsynchronousCall,TimerCallback));
    maTimer.SetTimeoutHdl(aCallback);
}




AsynchronousCall::~AsynchronousCall (void)
{
    mpFunction.reset();
    maTimer.Stop();
}




void AsynchronousCall::Post (
    const AsynchronousFunction& rFunction,
    sal_uInt32 nTimeoutInMilliseconds)
{
    mpFunction.reset(new AsynchronousFunction(rFunction));
    maTimer.SetTimeout(nTimeoutInMilliseconds);
    maTimer.Start();
}




IMPL_LINK(AsynchronousCall,TimerCallback,Timer*,pTimer)
{
    if (pTimer == &maTimer)
    {
        ::std::auto_ptr<AsynchronousFunction> pFunction (mpFunction);
        mpFunction.reset();
        (*pFunction)();
    }
    return 0;
}


} } // end of namespace ::sd::tools
