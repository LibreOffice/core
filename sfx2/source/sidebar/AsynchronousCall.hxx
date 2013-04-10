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

#ifndef SFX_SIDEBAR_ASYNCHRONOUS_CALL_HXX
#define SFX_SIDEBAR_ASYNCHRONOUS_CALL_HXX

#include <boost/function.hpp>
#include <tools/solar.h>
#include <tools/link.hxx>

namespace sfx2 { namespace sidebar {

/** A simple asynchronous call via Application::PostUserCall.
*/
class AsynchronousCall
{
public:
    typedef ::boost::function<void(void)> Action;

    AsynchronousCall (const Action& rAction);
    ~AsynchronousCall (void);

    void RequestCall (const Action& rAction);
    void RequestCall (void);
    void CancelRequest (void);

private:
    Action maAction;
    sal_uLong mnCallId;

    DECL_LINK(HandleUserCall, void*);
};


} } // end of namespace sfx2::sidebar

#endif
