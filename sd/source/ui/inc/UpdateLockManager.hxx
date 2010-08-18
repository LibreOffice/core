/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        always returns <FALSE/>.
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

