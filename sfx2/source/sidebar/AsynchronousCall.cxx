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

#include "precompiled_sfx2.hxx"

#include "AsynchronousCall.hxx"

#include <vcl/svapp.hxx>


namespace sfx2 { namespace sidebar {

AsynchronousCall::AsynchronousCall (const Action& rAction)
    : maAction(rAction),
      mnCallId(0)
{
}




AsynchronousCall::~AsynchronousCall (void)
{
    CancelRequest();
}




void AsynchronousCall::RequestCall (const Action& rAction)
{
    CancelRequest();
    maAction = rAction;
    RequestCall();
}




void AsynchronousCall::RequestCall (void)
{
    if (mnCallId == 0)
    {
        Link aLink (LINK(this, AsynchronousCall, HandleUserCall));
        mnCallId = Application::PostUserEvent(aLink);
    }
}




void AsynchronousCall::CancelRequest (void)
{
    if (mnCallId != 0)
    {
        Application::RemoveUserEvent(mnCallId);
        mnCallId = -1;
    }
}




IMPL_LINK(AsynchronousCall, HandleUserCall, void*, EMPTYARG )
{
    mnCallId = 0;
    if (maAction)
        maAction();

    return sal_True;
}


} } // end of namespace sfx2::sidebar

