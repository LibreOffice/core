/*************************************************************************
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 14:42:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    class Bootstrap
    {
    // the static interface
    public: // some common information items

        /// retrieve the product key; defaults to executable name (without extension)
        static rtl::OUString getProductKey();
        /// retrieve the product key; uses the given default, if not found
        static rtl::OUString getProductKey(rtl::OUString const& _sDefault);

        /// retrieve the LOGO information item; uses the given default, if not found
        static rtl::OUString getLogoData(rtl::OUString const& _sDefault);     //

        /// retrieve the BUILDID information item; uses the given default, if not found
        static rtl::OUString getBuildIdData(rtl::OUString const& _sDefault);

        /// retrieve the installation mode information item; uses the given default, if not found
        static rtl::OUString getInstallMode(rtl::OUString const& _sDefault);

        /// retrieve the product patch level; uses the given default, if not found
        static rtl::OUString getProductPatchLevel(rtl::OUString const& _sDefault);

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
        static Impl const& data();
    };
//-----------------------------------------------------------------------------
} // namespace utl

#endif // _UTL_BOOTSTRAP_HXX
