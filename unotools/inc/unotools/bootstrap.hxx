/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "unotools/unotoolsdllapi.h"

#ifndef _UTL_BOOTSTRAP_HXX
#define _UTL_BOOTSTRAP_HXX

namespace rtl
{
    class OUString;
}

namespace utl
{
//-----------------------------------------------------------------------------
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
        static rtl::OUString getProductKey();

        /// retrieve the product key; uses the given default, if not found
        static rtl::OUString getProductKey(rtl::OUString const& _sDefault);

        /// retrieve the product source (MWS name)
        static ::rtl::OUString getProductSource(rtl::OUString const& _sDefault);

        /// retrieve the BUILDID information item; uses the given default, if not found
        static rtl::OUString getBuildIdData(rtl::OUString const& _sDefault);

        /// retrieve the ALLUSERS information item from setup.ini file; uses the given default, if not found
        static rtl::OUString getAllUsersValue(rtl::OUString const& _sDefault);

        /// reload cached data
        static void reloadData();

    public: // retrieve path information about the installatíon location
        enum PathStatus
        {
            PATH_EXISTS,  // Success: Found a path to an existing file or directory
            PATH_VALID,   // Found a valid path, but the file or directory does not exist
            DATA_INVALID, // Retrieved a string for this path, that is not a valid file url or system path
            DATA_MISSING, // Could not retrieve any data for this path
            DATA_UNKNOWN  // No attempt to retrieve data for this path was made
        };

        /// get a file URL to the common base installation [${insturl}]
        static PathStatus locateBaseInstallation(rtl::OUString& _rURL);

        /// get a file URL to the user installation [${userurl}]
        static PathStatus locateUserInstallation(rtl::OUString& _rURL);

        /// get a file URL to the shared data directory [default is ${insturl}/share]
        static PathStatus locateSharedData(rtl::OUString& _rURL);

        /// get a file URL to the user data directory [default is ${userurl}/user]
        static PathStatus locateUserData(rtl::OUString& _rURL);

    // the next two items are mainly supported for diagnostic purposes. both items may be unused
        /// get a file URL to the bootstrap INI file used [e.g. ${insturl}/program/bootraprc]
        static PathStatus locateBootstrapFile(rtl::OUString& _rURL);
        /// get a file URL to the version locator INI file used [e.g. ${SYSUSERCONFIG}/sversion.ini]
        static PathStatus locateVersionFile(rtl::OUString& _rURL);

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

        /// Evaluates the status of the installation and returns a diagnostic message corresponding to this status
        static Status checkBootstrapStatus(rtl::OUString& _rDiagnosticMessage);

        /** Evaluates the status of the installation and returns a diagnostic
            message and error code corresponding to this status
        */
        static Status checkBootstrapStatus(rtl::OUString& _rDiagnosticMessage, FailureCode& _rErrCode);

    public:
        // singleton impl-class
        class Impl;
        static Impl const& data(); // the data related to the bootstrap.ini file
    };
//-----------------------------------------------------------------------------
} // namespace utl

#endif // _UTL_BOOTSTRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
