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


#ifndef _SWWAIT_HXX
#define _SWWAIT_HXX

#include <tools/solar.h>
#include "swdllapi.h"

#include <hash_set>

class SwDocShell;
class SfxDispatcher;

class SW_DLLPUBLIC SwWait
{
public:
    // Activate wait cursor for all windows of given document <rDocShell>
    // Optional all dispatcher could be Locked
    SwWait(
        SwDocShell &rDocShell,
        const bool bLockUnlockDispatcher );
    ~SwWait();

private:
    void EnterWaitAndLockDispatcher();
    void LeaveWaitAndUnlockDispatcher();

    SwDocShell& mrDoc;
    const bool mbLockUnlockDispatcher;
    std::unordered_set< SfxDispatcher* > mpLockedDispatchers;
};

#endif
