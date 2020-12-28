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
#include <set>
#include <string_view>
#include <vector>

namespace comphelper
{
    /** Helper class to backup/restore a single file
     *
     *  This is a general class to manage backups/restore of the file
     *  given by the URL. The container holding the backups is created
     *  aside the original file, e.g for 'test.txt' a container
     *  called '.test.pack' will be used. If it was not yet backed-up
     *  this container file will be created at the 1st backup and deleted
     *  when the last gets removed. The container holds a stack with a
     *  maximum given number (in the constructor) of copies, these are by
     *  default compressed, but don't have to be (see tryPush).
     *
     *  Due to being on a low system level here, no UNO API and not much
     *  other tooling can be used, as a consequence for the container a
     *  own simple format is used and e.g. the zip lib directly.
     *
     *  You need to hand over the URL of the file to look at and
     *  a maximum number of allowed copies. That number is internally
     *  limited to an absolute max of 10 (see implementation). The number
     *  of allowed copies is limited to [1..max].
     *
     *  Calling tryPush() will check if there is no backup yet or if
     *  there is one that the file has changed. If yes, a new copy is
     *  created on a kind of 'stack' of copies. The return value can
     *  be used to see if a backup was indeed created.
     *
     *  Calling tryPop() will do the opposite: if a backup is available,
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
        std::set< OUString >    maDirs;
        std::set< std::pair< OUString, OUString > > maFiles;

        sal_uInt16              mnNumBackups;
        sal_uInt16              mnMode;

        bool                    mbActive;
        bool                    mbExtensions;
        bool                    mbCompress;

        // internal flag if _exit() was called already - a hint to evtl.
        // not create copies of potentially not well-defined data. This
        // may be used in destructors of static instances - which unfortunately
        // get called on WNT but not on linux. Thus I thought about encapsulating
        // in some '#ifdefs', but it's just more safe to always do it and
        // allows to add a SAL_WARN when one of these destructors is called
        // after _exit()
        static bool         mbExitWasCalled;

        // internal detector if SafeModeName dir exists
        static bool         mbSafeModeDirExists;

        // internal upper limit (max) of allowed backups
        static sal_uInt16   mnMaxAllowedBackups;

        // path to User's configuration directory and derived strings
        static OUString     maInitialBaseURL;
        static OUString     maUserConfigBaseURL;
        static OUString     maUserConfigWorkURL;
        static OUString     maRegModName;
        static OUString     maExt;

        // get path to User's configuration directory (created on-demand)
        static const OUString& getInitialBaseURL();

        // the name of the SafeMode directory for temporary processing
        static const OUString& getSafeModeName();

    public:
        /** Constructor to handle Backups of the given file, will internally
         *  detect configuration values and URL to initial registrymodifications
         *  and thus the User configuration directory
         */
        BackupFileHelper();

        // allow to set static global flag when app had to call _exit()
        static void setExitWasCalled();
        static bool getExitWasCalled();

        // This call initializes the state of the UserDirectory as needed, it may
        // initialize to SafeMode configuration or return from it by moving files
        // in that directory
        static void reactOnSafeMode(bool bSafeMode);

        /** tries to create a new backup, if there is none yet, or if the
         *  last differs from the base file. It will then put a new version
         *  on the 'stack' of copies and evtl. delete the oldest backup.
         *  Also may cleanup older backups when NumBackups given in the
         *  constructor has changed.
         *
         * tryPushExtensionInfo is the specialized version for ExtensionInfo
         */
        void tryPush();
        void tryPushExtensionInfo();

        /** finds out if a restore is possible
         *
         *  @return bool
         *          returns true if a restore to an older backup is possible
         *
         * isPopPossibleExtensionInfo is the specialized version for ExtensionInfo
         */
        bool isPopPossible();
        bool isPopPossibleExtensionInfo() const;

        /** tries to execute a restore. Will overwrite the base file
         *  in that case and take one version off the 'stack' of copies.
         *  Also may cleanup older backups when NumBackups given in the
         *  constructor has changed.
         *
         * tryPopExtensionInfo is the specialized version for ExtensionInfo
         */
        void tryPop();
        void tryPopExtensionInfo();

        /** tries to iterate the extensions and to disable all of them
        */
        static bool isTryDisableAllExtensionsPossible();
        static void tryDisableAllExtensions();

        /** Deinstall all User Extensions (installed for User only)
        */
        static bool isTryDeinstallUserExtensionsPossible();
        static void tryDeinstallUserExtensions();

        /** Reset shared Extensions
        */
        static bool isTryResetSharedExtensionsPossible();
        static void tryResetSharedExtensions();

        /** Reset bundled Extensions
        */
        static bool isTryResetBundledExtensionsPossible();
        static void tryResetBundledExtensions();

        /// Disables OpenGL and OpenCL
        static void tryDisableHWAcceleration();

        /** resets User-Customizations like Settings and UserInterface modifications
        */
        static bool isTryResetCustomizationsPossible();
        static void tryResetCustomizations();

        /** resets the whole UserProfile
        */
        static void tryResetUserProfile();

        /** Return the profile url */
        static const OUString& getUserProfileURL();

        /** Return the url of the backed up profile (when in safe mode) */
        static const OUString& getUserProfileWorkURL();

    private:
        // internal helper methods
        static OUString getPackURL();
        static const std::vector< OUString >& getCustomizationDirNames();
        static const std::vector< OUString >& getCustomizationFileNames();

        // file push helpers
        bool tryPush_Files(const std::set< OUString >& rDirs, const std::set< std::pair< OUString, OUString > >& rFiles, std::u16string_view rSourceURL, const OUString& rTargetURL);
        bool tryPush_file(std::u16string_view rSourceURL, std::u16string_view rTargetURL, std::u16string_view rName, std::u16string_view rExt);

        // file pop possibilities helper
        bool isPopPossible_files(const std::set< OUString >& rDirs, const std::set< std::pair< OUString, OUString > >& rFiles, std::u16string_view rSourceURL, std::u16string_view rTargetURL);
        static bool isPopPossible_file(std::u16string_view rSourceURL, std::u16string_view rTargetURL, std::u16string_view rName, std::u16string_view rExt);

        // file pop helpers
        bool tryPop_files(const std::set< OUString >& rDirs, const std::set< std::pair< OUString, OUString > >& rFiles, std::u16string_view rSourceURL, const OUString& rTargetURL);
        bool tryPop_file(std::u16string_view rSourceURL, std::u16string_view rTargetURL, std::u16string_view rName, std::u16string_view rExt);

        // ExtensionInfo helpers
        bool tryPush_extensionInfo(std::u16string_view rTargetURL);
        static bool isPopPossible_extensionInfo(std::u16string_view rTargetURL);
        bool tryPop_extensionInfo(std::u16string_view rTargetURL);

        // FileDirInfo helpers
        void fillDirFileInfo();
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
