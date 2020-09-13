/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/* Information:
 * This class implements a mechanism to lock a users installation directory,
 * which is necessary because instances of staroffice could be running on
 * different hosts while using the same directory thus causing data
 * inconsistency.
 * When an existing lock is detected, the user will be asked whether he wants
 * to continue anyway, thus removing the lock and replacing it with a new one
 *
 * ideas:
 * - store information about user and host and time in the lockfile and display
 * these when asking whether to remove the lockfile.
 * - periodically check the lockfile and warn the user when it gets replaced
 *
 */

#pragma once

#include <rtl/ustring.hxx>

#include "dp_misc_api.hxx"

#define LOCKFILE_GROUP    "Lockdata"
#define LOCKFILE_USERKEY  "User"
#define LOCKFILE_HOSTKEY  "Host"
#define LOCKFILE_STAMPKEY "Stamp"
#define LOCKFILE_TIMEKEY  "Time"
#define LOCKFILE_IPCKEY   "IPCServer"

namespace desktop {

    class Lockfile;
    bool Lockfile_execWarning( Lockfile const * that );

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Lockfile
    {
    public:

        // constructs a new lockfile object
        Lockfile( bool bIPCserver = true );

        // separating GUI code:
        typedef bool (* fpExecWarning)( Lockfile const * that );

        // checks the lockfile, asks user when lockfile is
        // found (iff gui) and returns false when we may not continue
        bool check( fpExecWarning execWarning );

        // removes the lockfile
        ~Lockfile();

    private:
        bool m_bIPCserver;
        // full qualified name (file://-url) of the lockfile
        OUString m_aLockname;
        // flag whether the d'tor should delete the lock
        bool m_bRemove;
        bool m_bIsLocked;
        // ID
        OUString m_aId;
        OUString m_aDate;
        // access to data in file
        void syncToFile() const;
        bool isStale() const;
        friend bool Lockfile_execWarning( Lockfile const * that );

    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
