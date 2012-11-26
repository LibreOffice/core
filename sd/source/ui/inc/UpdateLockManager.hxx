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



#ifndef SD_UPDATE_LOCK_MANAGER_HXX
#define SD_UPDATE_LOCK_MANAGER_HXX

#include <memory>

namespace sd {

class ViewShellBase;

/** Manage update locks of ViewShellBase objects.
    A ViewShellBase object is locked while views are switched in order to
    avoid unnecessary repaints of views and object bars.
    Locking a ViewShellBase locks the frame::XLayoutManager and prevents
    Activate() and Deactivate() calls at ViewShell objects being processed.

    The main responsibility of this class is find the right moment to unlock
    the managed ViewShellBase object: Only Lock() has to be called from the
    outside (usually from PaneManager).  Unlock() is called by this class
    itself.  When all else fails it has a timer that calls Unlock()
    eventually.
*/
class UpdateLockManager
{
public:
    /** The newly created instance supports locking for the specified
        ViewShellBase object by default.  Call Disable() for Lock() and
        Unlock() calls being ignored.
    */
    UpdateLockManager (ViewShellBase& rBase);
    ~UpdateLockManager (void);

    /** For e.g. the PresentationViewShellBase locking is not necessary and
        does lead to problems.  This method lets Lock() and Unlock() calls
        be ignored and thus turns locking essentially off.
    */
    void Disable (void);

    /** Lock some UI updates.  For every call to this method a call to
        Unlock() is required to really unlock.
    */
    void Lock (void);

    /** When called as many times as Lock() has been called before then the
        ViewShellBase object is unlocked.
    */
    void Unlock (void);

    /** Return whether the ViewShellBase object is locked.  When locking is
        disabled, i.e. Disable() has been called before, then this method
        always returns <false/>.
    */
    bool IsLocked (void) const;

private:
    class Implementation;
    Implementation* mpImpl;

    UpdateLockManager (const UpdateLockManager&); // Not supported.
    UpdateLockManager& operator= (const UpdateLockManager&); // Not supported.
};

} // end of namespace sd

#endif

