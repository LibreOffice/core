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



/* Information:
 * This class implements a mechanism to lock a users installation directory,
 * which is necessesary because instances of staroffice could be running on
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

#include "sal/types.h"
#include "rtl/ustring.hxx"

class ByteString;

namespace desktop {

    class Lockfile;
    bool Lockfile_execWarning( Lockfile * that );

    class Lockfile
    {
    public:

        // contructs a new lockfile onject
        Lockfile( bool bIPCserver = true );

        // separating GUI code:
        typedef bool (* fpExecWarning)( Lockfile * that );

        // checks the lockfile, asks user when lockfile is
        // found (iff gui) and returns false when we may not continue
        sal_Bool check( fpExecWarning execWarning );

        // removes the lockfile. should only be called in exceptional situations
        void clean(void);

        // removes the lockfile
        ~Lockfile(void);

    private:
        // data in lockfile
        static const ByteString Group();
        static const ByteString Userkey();
        static const ByteString Hostkey();
        static const ByteString Stampkey();
        static const ByteString Timekey();
        static const ByteString IPCkey();
        // lockfilename
        static const rtl::OUString Suffix();
        bool m_bIPCserver;
        // full qualified name (file://-url) of the lockfile
        rtl::OUString m_aLockname;
        // flag whether the d'tor should delete the lock
        sal_Bool m_bRemove;
        sal_Bool m_bIsLocked;
        // ID
        rtl::OUString m_aId;
        rtl::OUString m_aDate;
        // access to data in file
        void syncToFile(void) const;
        sal_Bool isStale(void) const;
        friend bool Lockfile_execWarning( Lockfile * that );

    };

}
