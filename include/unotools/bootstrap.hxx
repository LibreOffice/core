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
#include "unotools/unotoolsdllapi.h"

#ifndef _UTL_BOOTSTRAP_HXX
#define _UTL_BOOTSTRAP_HXX

#include <rtl/ustring.hxx>

namespace utl
{
    /** provides configuration information needed for application startup.
        <p>This class handles the startup information for the office application.
           It encapsulates knowledge of how to retriev such information and how
           to diagnose failures to retriev required data.
        </p>
    */
    class UNOTOOLS_DLLPUBLIC Bootstrap
    {
    // the static interface
    public: // some common information items

        /// retrieve the product key; defaults to executable name (without extension)
        static OUString getProductKey();

        /// retrieve the product key; uses the given default, if not found
        static OUString getProductKey(OUString const& _sDefault);

        /// retrieve the BUILDID information item; uses the given default, if not found
        static OUString getBuildIdData(OUString const& _sDefault);

        /// retrieve the BuildVersion information item; uses the given default, if not found
        static OUString getBuildVersion(OUString const& _sDefault);

        /// reload cached data
        static void reloadData();

    public: // retrieve path information about the installation location
        enum PathStatus
        {
            PATH_EXISTS,  // Success: Found a path to an existing file or directory
            PATH_VALID,   // Found a valid path, but the file or directory does not exist
            DATA_INVALID, // Retrieved a string for this path, that is not a valid file url or system path
            DATA_MISSING, // Could not retrieve any data for this path
            DATA_UNKNOWN  // No attempt to retrieve data for this path was made
        };

        /// get a file URL to the common base installation [${insturl}]
        static PathStatus locateBaseInstallation(OUString& _rURL);

        /// get a file URL to the user installation [${userurl}]
        static PathStatus locateUserInstallation(OUString& _rURL);

        /// get a file URL to the user data directory [default is ${userurl}/user]
        static PathStatus locateUserData(OUString& _rURL);

    // the next two items are mainly supported for diagnostic purposes. both items may be unused
        /// get a file URL to the bootstrap INI file used [e.g. ${insturl}/program/bootraprc]
        static PathStatus locateBootstrapFile(OUString& _rURL);
        /// get a file URL to the version locator INI file used [e.g. ${SYSUSERCONFIG}/sversion.ini]
        static PathStatus locateVersionFile(OUString& _rURL);

    public: // evaluate the validity of the installation
        /// high-level status of bootstrap success
        enum Status
        {
            DATA_OK,              /// user-dir and share-dir do exist, product key found or can be defaulted to exe-name
            MISSING_USER_INSTALL, /// ${userurl} does not exist; or version-file cannot be found or is invalid
            INVALID_USER_INSTALL, /// can locate ${userurl}, but user-dir is missing
            INVALID_BASE_INSTALL  /// other failure: e.g. cannot locate share-dir; bootstraprc missing or invalid; no product key
        };

        /// error code for detailed diagnostics of bootstrap failures
        enum FailureCode
        {
            NO_FAILURE,                   /// bootstrap was successful
            MISSING_INSTALL_DIRECTORY,    /// the shared installation directory could not be located
            MISSING_BOOTSTRAP_FILE,       /// the bootstrap INI file could not be found or read
            MISSING_BOOTSTRAP_FILE_ENTRY, /// the bootstrap INI is missing a required entry
            INVALID_BOOTSTRAP_FILE_ENTRY, /// the bootstrap INI contains invalid data
            MISSING_VERSION_FILE,         /// the version locator INI file could not be found or read
            MISSING_VERSION_FILE_ENTRY,   /// the version locator INI has no entry for this version
            INVALID_VERSION_FILE_ENTRY,   /// the version locator INI entry is not a valid directory URL
            MISSING_USER_DIRECTORY,       /// the user installation directory does not exist
            INVALID_BOOTSTRAP_DATA        /// some bootstrap data was invalid in unexpected ways
        };

        /** Evaluates the status of the installation and returns a diagnostic
            message and error code corresponding to this status
        */
        static Status checkBootstrapStatus(OUString& _rDiagnosticMessage, FailureCode& _rErrCode);

    public:
        // singleton impl-class
        class Impl;
        static const Impl& data(); // the data related to the bootstrap.ini file
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
