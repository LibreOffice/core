/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_BACKUPFILEHELPER_HXX
#define INCLUDED_COMPHELPER_BACKUPFILEHELPER_HXX

#include <sal/config.h>

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>

namespace comphelper
{
    /** Helper class to backup/restore a single file
     *
     *  You need to hand over the URL of the file to look at and
     *  a maximum number of allowed copies. That number is internally
     *  limited to a max of 10 (see implementation). The number of
     *  allowed copies is limited to [1..max].
     *
     *  Calling tryPush() will check if there is no backup yet or if
     *  there is one that the file has changed. If yes, a new copy is
     *  created on a kind of 'stack' of copies. Tre return value can
     *  be used to see if a backup was indeed created.
     *
     *  Calling tryPop() will do the opposite: If a backup is available,
     *  delete the orig file and re-instantiate the backup. The backup
     *  is taken off the 'stack' of copies. The return value can be
     *  used to check if this was done.
     *
     *  isPopPossible can be called to see if there is a backup available
     *  before calling tryPop().
     *
     *  The 'stack' of copies works by using the same path, filename
     *  and extension, but adding a '_1' -> '_(num_of_copy)' to it.
     */
    class COMPHELPER_DLLPUBLIC BackupFileHelper
    {
    private:
        // internal data
        const OUString&     mrBaseURL;
        sal_uInt16          mnNumBackups;
        OUString            maBase;
        OUString            maExt;
        osl::File           maBaseFile;
        bool                mbBaseFileIsOpen;

        // internal flag if _exit() was called already - a hint to evtl.
        // not create copies of potentially not well-defined data. This
        // may be used in destructors of static instances - which unfortunately
        // get called on WNT but not on linux. Thus I thought about encapsulating
        // in some '#ifdefs', but it's just more safe to always do it and
        // allows to add a SAL_WARN when one of these destructors is called
        // after _exit()
        static bool         mbExitWasCalled;

        // internal upper limit (max) of allowed backups
        static sal_uInt16   mnMaxAllowedBackups;

    public:
        /** Constructor to handle Backups of the given file
         *
         *  @param  rBaseURL
         *          URL to an existing file that needs to be backed up
         *
         *  @param  nNumBackups
         *          Specifies the maximum number of backups to allow for
         *          the file. This value gets truncated to [1..max] where
         *          max currently is 10 and defined in the implementation.
         *          It is used in tryPush() and tryPop() calls to cleanup/
         *          reduce the number of existing backups
         */
        BackupFileHelper(const OUString& rBaseURL, sal_uInt16 nNumBackups = 5);

        // allow to set flag when app had to call _exit()
        static void setExitWasCalled();
        static bool getExitWasCalled();

        /** tries to create a new backup, if there is none yet, or if the
         *  last differs from the base file. It will then put a new verion
         *  on the 'stack' of copies and evtl. delete the oldest backup.
         *  Also may cleanup older backups when NumBackups given in the
         *  constructor has changed.
         *
         *  @return bool
         *          returns true if a new backup was actually created
         */
        bool tryPush();

        /** finds out if a restore is possible
         *
         *  @return bool
         *          returns true if a restore to an older backup is possible
         */
        bool isPopPossible();

        /** tries to execute a restore. Will overwrite the base file
         *  in that case and take one version off the 'stack' of copies.
         *  Also may cleanup older backups when NumBackups given in the
         *  constructor has changed.
         *
         *  @return bool
         *          returns true if a restore was actually created
         */
        bool tryPop();

    private:
        // internal helper methods
        rtl::OUString getName(sal_uInt16 n);
        bool firstExists();
        void pop();
        void push();
        bool isDifferentOrNew();
        bool equalsBase(osl::File& rLastFile);
        bool splitBaseURL();
        bool baseFileOpen();
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
