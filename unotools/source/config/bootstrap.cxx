/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2001-08-06 16:00:48 $
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

#include <stdio.h>

#include "unotools/bootstrap.hxx"

// ---------------------------------------------------------------------------------------
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
// ---------------------------------------------------------------------------------------
// needed until
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _OSL_PROFILE_HXX_
#include <osl/profile.hxx>
#endif
#ifndef _OSL_SECURITY_H_
#include <osl/security.h> // for osl_getConfigDir
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h> // for osl_getExecutableFile
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h> // for osl_getThreadTextEncoding
#endif
// ---------------------------------------------------------------------------------------
#define BOOTSTRAP_FILE                  SAL_CONFIGFILE( "bootstrap" )

#define BOOTSTRAP_SECTION                   "Bootstrap"
#define BOOTSTRAP_ITEM_PRODUCT_KEY          "ProductKey"
#define BOOTSTRAP_ITEM_LOCATION             "Location"
#define BOOTSTRAP_ITEM_SECTION              "Section"
#define BOOTSTRAP_ITEM_LOGO                 "Logo"

#define BOOTSTRAP_VALUE_SECTION_DEFAULT     "Versions"
// ---------------------------------------------------------------------------------------
typedef char const * AsciiString;
// ---------------------------------------------------------------------------------------

namespace utl
{
// ---------------------------------------------------------------------------------------
    using ::rtl::OUString;
    using ::rtl::OUStringBuffer;
    using ::rtl::OString;

// ---------------------------------------------------------------------------------------
// Implementation class: Bootstrap::Impl
// ---------------------------------------------------------------------------------------

    class Bootstrap::Impl
    {
    public: // struct to cache the result of a path lookup
        struct PathData
        {
            OUString     path;
            PathStatus   status;

            PathData()
            : path()
            , status(DATA_UNKNOWN)
            {}
        };
    public: // data members
        // base install data
        PathData aBootstrapINI_;
        PathData aBaseInstall_;
        OUString sExename_;

        // user install data
        PathData aVersionINI_;
        PathData aUserInstall_;

        // overall status
        Status status_;

    public: // construction and initialization
        Impl()
        {
            status_ = initialize();
        }

        Status initialize();

        // access helper - only use for pure ASCII data
        OUString getBootstrapValue(AsciiString _sName, OUString const& _sDefault) const;

    private: // implementation
        bool initBaseInstallationData();
        bool initUserInstallationData();
    };
// ---------------------------------------------------------------------------------------
    Bootstrap::Impl const& Bootstrap::data()
    {
        static Impl* s_pData = NULL;

        if (!s_pData)
        {
            using namespace osl;
            MutexGuard aGuard( Mutex::getGlobalMutex() );

            static Impl s_theData;

            s_pData = &s_theData;
        }
        return *s_pData;
    }

// ---------------------------------------------------------------------------------------
// helper
// ---------------------------------------------------------------------------------------

typedef Bootstrap::PathStatus PathStatus;

sal_Unicode const cURLSeparator = '/';

// ---------------------------------------------------------------------------------------
static
inline
OUString getURLSeparator()
{
    static OUString theSep(&cURLSeparator,1);
    return theSep;
}

// ---------------------------------------------------------------------------------------
// path status utility function
static
PathStatus checkStatusOfURL(OUString const& _sURL)
{
    using namespace osl;

    PathStatus eStatus = Bootstrap::DATA_UNKNOWN;

    if (_sURL.getLength() != 0)
    {
        DirectoryItem aItem;
        switch( DirectoryItem::get(_sURL, aItem) )
        {
        case DirectoryItem::E_None:         // Success
            eStatus = Bootstrap::PATH_EXISTS;
            break;

        case DirectoryItem::E_NOENT:        // No such file or directory<br>
            eStatus = Bootstrap::PATH_VALID;
            break;

        case DirectoryItem::E_INVAL:        // the format of the parameters was not valid<br>
        case DirectoryItem::E_NAMETOOLONG:  // File name too long<br>
        case DirectoryItem::E_NOTDIR:       // A component of the path prefix of path is not a directory<p>
            eStatus = Bootstrap::DATA_INVALID;
            break;

        // how to handle these ?
        case DirectoryItem::E_LOOP:         // Too many symbolic links encountered<br>
        case DirectoryItem::E_ACCES:        // permission denied<br>
        // any other error - what to do ?
        default:
            eStatus = Bootstrap::DATA_UNKNOWN;
            break;
        }
    }
    else
        eStatus = Bootstrap::DATA_MISSING;

    return eStatus;
}

// ----------------------------------------------------------------------------------
// helpers to build and check a nested URL
static
PathStatus getDerivedPath(OUString& _rURL, OUString const& _sRelativeURL, OUString const& _aBaseURL, PathStatus _aBaseStatus)
{
    OSL_ASSERT(_sRelativeURL.getLength() != 0 && _sRelativeURL[0] != cURLSeparator);

    PathStatus aStatus = _aBaseStatus;

    // do we have a base path ?
    if (_aBaseURL.getLength())
    {
        OSL_PRECOND(_aBaseURL[_aBaseURL.getLength()-1] != cURLSeparator,"Unexpected: base URL ends in slash");

        _rURL = _aBaseURL + getURLSeparator() + _sRelativeURL;

        // a derived (nested) URL can only exist or have a lesser status, if the parent exists
        if (aStatus == Bootstrap::PATH_EXISTS)
            aStatus = checkStatusOfURL(_rURL);

        else // the relative appendix must be valid
            OSL_ASSERT(aStatus != Bootstrap::PATH_VALID || checkStatusOfURL(_rURL) == Bootstrap::PATH_VALID);
    }
    else
    {
        // clear the result
        _rURL = _aBaseURL;

        // if we have no data it can't be a valid path
        OSL_ASSERT( aStatus > Bootstrap::PATH_VALID );
    }

    return aStatus;
}

// ----------------------------------------------------------------------------------
static
inline
PathStatus getDerivedPath(OUString& _rURL, OUString const& _sRelativeURL, Bootstrap::Impl::PathData const& _aBaseData)
{
    return getDerivedPath(_rURL,_sRelativeURL,_aBaseData.path,_aBaseData.status);
}

// ---------------------------------------------------------------------------------------
static bool normalizeAndSubstitutePathVariables(OUString& _rPath)
{
    OSL_PRECOND(_rPath.getLength() != 0 , "Invalid parameter: Empty path in normalizeAndSubstitutePathVariables" );

    typedef sal_Bool (SAL_CALL * getSystemDirectoryFunction)(oslSecurity, rtl_uString **);
    // recognized variables and methods to retrieve their substitute pathes
    // Note: variables are expected to be an absolute URL and thus must be at the beginning of _rPath
    struct
    {
        sal_Char const *            pattern;
        getSystemDirectoryFunction  getPath;
    }
    aPathVariables[] =
    {
        { "$(SYSUSERHOME)",   ::osl_getHomeDir    },
        { "$(SYSUSERCONFIG)", ::osl_getConfigDir  }
    };

    bool bIsNormalized = false;

    // check for all variables
    for (sal_Int32 i=0; i < sizeof(aPathVariables)/sizeof(aPathVariables[0]); ++i)
    {
        OUString sVariable = OUString::createFromAscii(aPathVariables[i].pattern);
        sal_Int32 nVariablePos = _rPath.indexOf(sVariable);

        OSL_ENSURE( 0 >= nVariablePos, "Unexpected: System directory variable in the middle of a path");
        if (0 == nVariablePos)
        {
            OSL_ENSURE( !bIsNormalized, "Found multiple path variables");

            oslSecurity aCurrentUserSec = osl_getCurrentSecurity();

            OUString sSubstitute;
            aPathVariables[i].getPath(aCurrentUserSec, &sSubstitute.pData);
            _rPath = _rPath.replaceAt(nVariablePos, sVariable.getLength(), sSubstitute);

            bIsNormalized = true;

        #ifndef DBG_UTIL
            break; // in debug builds continue for additional checks
        #endif // DBG_UTIL
        }
    }

    if ( !bIsNormalized)
    {
        OUString sOther;

        // make a normalized path
        if ( osl::File::E_None == osl::File::getFileURLFromSystemPath(_rPath, sOther) )
        {
            _rPath = sOther;
            bIsNormalized = true;

        }
        // check if it already was normalized
        else
            bIsNormalized = ( osl::File::E_None == osl::File::getSystemPathFromFileURL(_rPath, sOther) );
    }

    return bIsNormalized;
}

// ----------------------------------------------------------------------------------
static
void readPathFromINI(Bootstrap::Impl::PathData & _rResult, osl::Profile & _rIniFile, OString const& _sSection, OString const& _sEntry)
{
    // encoding to use for converting pathes read from an INI
    rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

    OString sConvertable = _rIniFile.readString(_sSection, _sEntry, OString());

    _rResult.path   = rtl::OStringToOUString(sConvertable, nCvtEncoding);

    if ( _rResult.path.getLength() == 0)
        _rResult.status = Bootstrap::DATA_MISSING;

    else if ( normalizeAndSubstitutePathVariables(_rResult.path) )
        _rResult.status = checkStatusOfURL(_rResult.path);

    else
        _rResult.status = Bootstrap::DATA_INVALID;
}
// ---------------------------------------------------------------------------------------

static void addFileError(OUStringBuffer& _rBuf, OUString const& _aPath, AsciiString _sWhat)
{
    OUString sSimpleFileName = _aPath.copy(1 +_aPath.lastIndexOf(cURLSeparator));

    _rBuf.appendAscii("A main configuration file ").appendAscii(_sWhat);
    _rBuf.appendAscii(". (").append(sSimpleFileName).appendAscii(") ");
}
// ---------------------------------------------------------------------------------------

static void addDirectoryError(OUStringBuffer& _rBuf, OUString const& _aPath, AsciiString _sWhat)
{
    _rBuf.appendAscii("A configuration directory ").appendAscii(_sWhat);
    _rBuf.appendAscii(". (").append(_aPath).appendAscii(") ");
}
// ---------------------------------------------------------------------------------------

static void addUnexpectedError(OUStringBuffer& _rBuf, AsciiString _sExtraInfo)
{
    _rBuf.appendAscii("An unexpected bootstrap problem occurred");
    _rBuf.appendAscii(". (").appendAscii(_sExtraInfo).appendAscii(") ");
}
// ---------------------------------------------------------------------------------------

static void describeError(OUStringBuffer& _rBuf, Bootstrap::Impl const& _rData)
{
    switch (_rData.aUserInstall_.status)
    {
    case Bootstrap::PATH_EXISTS:
        switch (_rData.aBaseInstall_.status)
        {
        case Bootstrap::PATH_VALID:
            addDirectoryError(_rBuf, _rData.aBaseInstall_.path, "cannot be found");
            break;

        case Bootstrap::DATA_INVALID:
            addUnexpectedError(_rBuf,"Installation path invalid");
            break;

        case Bootstrap::DATA_MISSING:
            addUnexpectedError(_rBuf,"Installation path not available");
            break;

        case Bootstrap::PATH_EXISTS: // seems to be all fine (?)
            addUnexpectedError(_rBuf,"Data OK");
            break;

        default: OSL_ASSERT(false);
            addUnexpectedError(_rBuf,"Internal failure");
            break;
        }
        break;

    case Bootstrap::PATH_VALID:
        addDirectoryError(_rBuf, _rData.aUserInstall_.path, "is missing");
        break;

        // else fall through
    case Bootstrap::DATA_INVALID:
        if (_rData.aVersionINI_.status == Bootstrap::PATH_EXISTS)
        {
            addFileError(_rBuf, _rData.aVersionINI_.path, "is corrupt");
            break;
        }
        // else fall through

    case Bootstrap::DATA_MISSING:
        switch (_rData.aVersionINI_.status)
        {
        case Bootstrap::PATH_EXISTS:
            addFileError(_rBuf, _rData.aVersionINI_.path, "has no data for this version");
            break;

        case Bootstrap::PATH_VALID:
            addFileError(_rBuf, _rData.aVersionINI_.path, "is missing");
            break;

        default:
            switch (_rData.aBootstrapINI_.status)
            {
            case Bootstrap::PATH_EXISTS:
                addFileError(_rBuf, _rData.aBootstrapINI_.path, "is corrupt");
                break;

            case Bootstrap::DATA_INVALID: OSL_ASSERT(false);
            case Bootstrap::PATH_VALID:
                addFileError(_rBuf, _rData.aBootstrapINI_.path, "is missing");
                break;

            default:
                addUnexpectedError(_rBuf,"No data");
                break;
            }
            break;
        }
        break;

    default: OSL_ASSERT(false);
        addUnexpectedError(_rBuf,"Internal Failure");
        break;
    }
}
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// class Bootstrap
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getProductKey()
{
    Impl const& aData = data();

    OUString const& sDefaultProductKey = aData.sExename_;
    //OUString const sDefaultProductKey(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT));

    return aData.getBootstrapValue( BOOTSTRAP_ITEM_PRODUCT_KEY, sDefaultProductKey );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getProductKey(OUString const& _sDefault)
{
    Impl const& aData = data();

    return aData.getBootstrapValue( BOOTSTRAP_ITEM_PRODUCT_KEY, _sDefault );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getLogoData(OUString const& _sDefault)
{
    Impl const& aData = data();

    return aData.getBootstrapValue( BOOTSTRAP_ITEM_LOGO, _sDefault );
}
// ---------------------------------------------------------------------------------------

Bootstrap::PathStatus Bootstrap::locateBaseInstallation(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aBaseInstall_;

    _rURL = aPathData.path;
    return aPathData.status;
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateUserInstallation(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aUserInstall_;

    _rURL = aPathData.path;
    return aPathData.status;
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateSharedData(OUString& _rURL)
{
    OUString const aShareDir(RTL_CONSTASCII_USTRINGPARAM("share"));

    return getDerivedPath(_rURL,aShareDir,data().aBaseInstall_);
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateUserData(OUString& _rURL)
{
    OUString const aUserDir(RTL_CONSTASCII_USTRINGPARAM("user"));

    return getDerivedPath(_rURL,aUserDir,data().aUserInstall_);
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateBootstrapFile(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aBootstrapINI_;

    _rURL = aPathData.path;
    return aPathData.status;
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateVersionFile(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aVersionINI_;

    _rURL = aPathData.path;
    return aPathData.status;
}
// ---------------------------------------------------------------------------------------

Bootstrap::Status Bootstrap::checkBootstrapStatus(OUString& _rDiagnosticMessage)
{
    Impl const& aData = data();

    Status result = aData.status_;

    // maybe do further checks here

    OUStringBuffer sErrorBuffer;
    if (result != DATA_OK)
    {
        describeError(sErrorBuffer,aData);
    }
    _rDiagnosticMessage = sErrorBuffer.makeStringAndClear();

    return result;
}
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// class Bootstrap::Impl
// ---------------------------------------------------------------------------------------

bool Bootstrap::Impl::initBaseInstallationData()
{
    OUString sExecutable;

    if (osl_Process_E_None != osl_getExecutableFile(&sExecutable.pData))
    {
        aBaseInstall_.status = DATA_MISSING;
        return false;
    }

    // split the executable name
    sal_Int32 nSepIndex = sExecutable.lastIndexOf(cURLSeparator);

    sExename_ = sExecutable.copy(nSepIndex + 1);

    OSL_ENSURE(nSepIndex > 0, "osl_getExecutableFile returns local path only");
    if (nSepIndex <= 0)
    {
        aBaseInstall_.status = DATA_INVALID;
        return false;
    }

    // reduce to executable directory
    sExecutable = sExecutable.copy(0,nSepIndex);

    // ... and split to find installation directory
    nSepIndex = sExecutable.lastIndexOf(cURLSeparator);
    OSL_ENSURE(nSepIndex > 0, "osl_getExecutableFile returns relative path only");
    if (nSepIndex <= 0)
    {
        aBaseInstall_.status = DATA_INVALID;
        return false;
    }

    aBaseInstall_.path      = sExecutable.copy(0,nSepIndex);
    aBaseInstall_.status    = PATH_EXISTS;

    OSL_ASSERT( checkStatusOfURL(sExecutable) == PATH_EXISTS );
    OSL_ASSERT( checkStatusOfURL(aBaseInstall_.path) == PATH_EXISTS );

    // now find the bootstrap ini
    OUString const sBootstrapName(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_FILE));
    aBootstrapINI_.status = getDerivedPath(aBootstrapINI_.path,sBootstrapName,sExecutable,PATH_EXISTS);

    return true;
}
// ---------------------------------------------------------------------------------------

bool Bootstrap::Impl::initUserInstallationData()
{
    OString sVersionFileSection(BOOTSTRAP_VALUE_SECTION_DEFAULT);
    //OString sVersionFileProductKey(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT);
    OString sVersionFileProductKey = rtl::OUStringToOString(
                        this->getBootstrapValue(BOOTSTRAP_ITEM_PRODUCT_KEY,sExename_),
                        RTL_TEXTENCODING_ASCII_US);

    // try to find a sversion ini
    if (aBootstrapINI_.status == PATH_EXISTS)
    {
        // read the bootstrap file
        osl::Profile aBootstrapFile(aBootstrapINI_.path);

        OString const sSection(BOOTSTRAP_SECTION);
        // get the section/key name of the entry in the product version file
        sVersionFileSection = aBootstrapFile.readString(sSection,BOOTSTRAP_ITEM_SECTION,sVersionFileSection);

        sVersionFileProductKey = aBootstrapFile.readString(sSection,BOOTSTRAP_ITEM_PRODUCT_KEY,sVersionFileProductKey);

        readPathFromINI(aVersionINI_, aBootstrapFile, sSection,BOOTSTRAP_ITEM_LOCATION);
    }

    // try to find a user installation
    switch (aVersionINI_.status)
    {
    case PATH_EXISTS: // sversion ini found
        {
        // open the product version profile
            osl::Profile aProductVersionFile(aVersionINI_.path);

            readPathFromINI(aUserInstall_, aProductVersionFile, sVersionFileSection, sVersionFileProductKey);
        }
        break;

    case PATH_VALID:   // sversion ini location invalid or file missing
    case DATA_INVALID:
        aUserInstall_.status = DATA_MISSING;
        break;

    default: OSL_ASSERT(!"Unreachable code");
    case DATA_UNKNOWN: // no bootstrap ini
    case DATA_MISSING: // no sversion ini location
        {
            // should we do this - look for a single-user user directory ?
            OUString const aUserDir(RTL_CONSTASCII_USTRINGPARAM("user"));
            OUString sTestUserDir;
            if ( getDerivedPath(sTestUserDir,aUserDir,aBaseInstall_) )
                aUserInstall_ = aBaseInstall_;
            else

            // or just this - distinguishing this from
                aUserInstall_.status = DATA_MISSING;
        }
        break;
    }

    return aUserInstall_.status == PATH_EXISTS;
}
// ---------------------------------------------------------------------------------------

Bootstrap::Status Bootstrap::Impl::initialize()
{
    Bootstrap::Status result;

    if (!initBaseInstallationData())
    {
        result = INVALID_BASE_INSTALL;
    }
    else if (!initUserInstallationData())
    {
        result = INVALID_USER_INSTALL;

        if (aUserInstall_.status >= DATA_MISSING)
        {
            switch (aVersionINI_.status)
            {
            case PATH_EXISTS:
            case PATH_VALID:
                result = MISSING_USER_INSTALL;
                break;

            case DATA_INVALID:
            case DATA_MISSING:
                result = INVALID_BASE_INSTALL;
                break;
            }
        }
    }
    else
    {
        result = DATA_OK;
    }
    return result;
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::Impl::getBootstrapValue(AsciiString _sName, OUString const& _sDefault) const
{
    if (aBootstrapINI_.status != PATH_EXISTS) return _sDefault;

    osl::Profile aBootstrapFile(aBootstrapINI_.path);

    // get the path to the sversion.ini
    OString sConvertable = aBootstrapFile.readString( BOOTSTRAP_SECTION, _sName, OString());

    return rtl::OStringToOUString(sConvertable, RTL_TEXTENCODING_ASCII_US);
}
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// The old (deprecated) interface
// ----------------------------------------------------------------------------------
// the old hard-coded defaults
#define BOOTSTRAP_VALUE_LOGO_DEFAULT        "1"
#define BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT "StarOffice 6.0"

#ifdef DBG_UTIL
// old implementations used for cross-checking
BootstrapRetVal legacy_bootstrap_getProductKeyAndLogo( OUString& _rProductKey, OUString& _rLogo, OUString& _sIniPath );
BootstrapRetVal legacy_bootstrap_locateUserInstallationPath(OUString& _rOfficeInstallPath,OUString& _rUserInstallPath, OUString& _sIniPath);
#endif

// locate the user path
BootstrapRetVal bootstrap_locateUserInstallationPath(OUString& _rOfficeInstallPath,OUString& _rUserInstallPath, OUString& _sIniPath)
{
    Bootstrap::locateBaseInstallation(_rOfficeInstallPath);

     BootstrapRetVal nStatus = SREGISTRY_INI_INVALID; // tag unexcpected control flow

    switch ( Bootstrap::locateUserInstallation(_rUserInstallPath) )
    {
    case Bootstrap::PATH_EXISTS:
        nStatus = BOOTSTRAP_OK;
        break;

    case Bootstrap::PATH_VALID:
        nStatus = SREGISTRY_INI_NOT_FOUND;
        break;

    case Bootstrap::DATA_INVALID:
        nStatus = SVERSION_INI_INVALID;
        Bootstrap::locateVersionFile(_sIniPath);
        break;

    case Bootstrap::DATA_MISSING:
        switch (Bootstrap::locateVersionFile(_sIniPath))
        {
        case Bootstrap::PATH_EXISTS:
            nStatus = SVERSION_INI_NO_ENTRY;
            break;

        case Bootstrap::PATH_VALID:
            nStatus = SVERSION_INI_NOT_FOUND;
            break;

        default:
            if (Bootstrap::locateBootstrapFile(_sIniPath) == Bootstrap::PATH_EXISTS)
                nStatus = BOOTSTRAP_INI_INVALID;
            else
                nStatus = BOOTSTRAP_INI_NOT_FOUND;
            break;
        }
        break;
    }
    #ifdef DBG_UTIL
    OUString chkOI,chkUI, chkIP;
    BootstrapRetVal chkRet = legacy_bootstrap_locateUserInstallationPath(chkOI,chkUI, chkIP);
    OSL_ENSURE(chkOI == _rOfficeInstallPath || (nStatus && !chkOI.getLength()), "Bootstrap: semantics changed for Base Install Path");
    OSL_ENSURE(chkUI == _rUserInstallPath   || (nStatus && !chkUI.getLength()), "Bootstrap: semantics changed for User Install Path");
    OSL_ENSURE(chkIP == _sIniPath           || (nStatus && !chkIP.getLength()), "Bootstrap: semantics changed for INI path");
    OSL_ENSURE(chkRet == nStatus, "Bootstrap: semantics changed for status returned");
    #endif


    return nStatus;
}
// ----------------------------------------------------------------------------------

// retrieve product key and logo information
BootstrapRetVal bootstrap_getProductKeyAndLogo( OUString& _rProductKey, OUString& _rLogo, OUString& _sIniPath )
{
    OString sAsciiProductKey(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT);
    OString sAsciiLogo      (BOOTSTRAP_VALUE_LOGO_DEFAULT);

    BootstrapRetVal nStatus = BOOTSTRAP_INI_INVALID;

    if ( Bootstrap::locateBootstrapFile(_sIniPath) == Bootstrap::PATH_EXISTS )
    {
        osl::Profile aBootstrapFile(_sIniPath);

        OString const sSection(BOOTSTRAP_SECTION);

        sAsciiProductKey = aBootstrapFile.readString(sSection, BOOTSTRAP_ITEM_PRODUCT_KEY, sAsciiProductKey);
        sAsciiLogo       = aBootstrapFile.readString(sSection, BOOTSTRAP_ITEM_LOGO, sAsciiLogo);

        OUString sVersionINI;
        switch( Bootstrap::locateVersionFile(sVersionINI) )
        {
        case Bootstrap::PATH_EXISTS:
            _sIniPath = sVersionINI;
            nStatus = BOOTSTRAP_OK;
            break;

        case Bootstrap::PATH_VALID:
            _sIniPath = sVersionINI;
            nStatus = SVERSION_INI_NOT_FOUND;
            break;

        default:
            nStatus = BOOTSTRAP_INI_INVALID;
            break;
        }
    }
    else
        nStatus = BOOTSTRAP_INI_NOT_FOUND;

    _rProductKey    = rtl::OStringToOUString(sAsciiProductKey,RTL_TEXTENCODING_ASCII_US);
    _rLogo          = rtl::OStringToOUString(sAsciiLogo,RTL_TEXTENCODING_ASCII_US);

    #ifdef DBG_UTIL
    OUString chkPK, chkLG, chkIP;
    BootstrapRetVal chkRet = legacy_bootstrap_getProductKeyAndLogo(chkPK, chkLG, chkIP);
    OSL_ENSURE(chkPK == _rProductKey || (nStatus && !chkPK.getLength()), "Bootstrap: semantics changed for PRODUCT_KEY");
    OSL_ENSURE(chkLG == _rLogo       || (nStatus && !chkLG.getLength()), "Bootstrap: semantics changed for LOGO");
    OSL_ENSURE(chkIP == _sIniPath    || (nStatus && !chkIP.getLength()), "Bootstrap: semantics changed for INI path");
    OSL_ENSURE(chkRet == nStatus     || chkRet>=SREGISTRY_INI_NOT_FOUND, "Bootstrap: semantics changed for status returned");
    #endif

    return nStatus;
}
// ----------------------------------------------------------------------------------
#ifdef DBG_UTIL
static inline void legacy_moveUpOneDirectory(OUString& _rsPath, bool leaveSlash = false)
{
    sal_Int32 nSepIndex = _rsPath.lastIndexOf(cURLSeparator);

    OSL_ENSURE(nSepIndex > 0, "Cannot move up one directory - path has at most one component");
    if (nSepIndex > 0)
    {
        OSL_ENSURE(nSepIndex+1 != _rsPath.getLength(), "Cannot move up one directory: Unexpected path format - path must not be slash-terminated");
        if (leaveSlash) ++nSepIndex;
        _rsPath = _rsPath.copy(0, nSepIndex);
    }
    else
        _rsPath = OUString();
}

// ----------------------------------------------------------------------------------
// locate the office install and bootstrap ini

static bool legacy_locateBootstrapProfile(OUString& _rOfficeInstallPath, OUString& _rProfileFile)
{
    // get the bootstrap.ini file, where we find basic informations about the version of the product
    // we're running in
    // the bootstrap.ini resides in the same dir as our executable
    OUString sBootstrap;
    osl_getExecutableFile(&sBootstrap.pData);

    // take care for the office install path
    _rOfficeInstallPath = sBootstrap;
    // for this, cut two levels from the executable (the executable name and the program directory)
    legacy_moveUpOneDirectory(_rOfficeInstallPath);
    legacy_moveUpOneDirectory(_rOfficeInstallPath);

    // cut the executable file name and append the bootstrap ini file name
    legacy_moveUpOneDirectory(sBootstrap,true);
    sBootstrap += OUString::createFromAscii(BOOTSTRAP_FILE);

    _rProfileFile = sBootstrap;

    osl::DirectoryItem aCheckPath;
    return osl::File::E_None==aCheckPath.get(sBootstrap, aCheckPath);
}

static inline bool isValidFileURL(OUString const& _sFileURL)
{
    OUString sSystemPath;
    return _sFileURL.getLength() && osl::File::E_None==osl::File::getSystemPathFromFileURL(_sFileURL, sSystemPath);
}

// ----------------------------------------------------------------------------------
// locate the user path

BootstrapRetVal legacy_bootstrap_locateUserInstallationPath(OUString& _rOfficeInstallPath,OUString& _rUserInstallPath, OUString& _sIniPath)
{
    rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

    BootstrapRetVal nStatus = BOOTSTRAP_INI_NOT_FOUND;

    try
    {
        if ( !legacy_locateBootstrapProfile(_rOfficeInstallPath,_sIniPath) )
        {
            return BOOTSTRAP_INI_NOT_FOUND;
        }

        // get the path to the sversion.ini file from the bootstrap file
        ::osl::Profile aBootstrapFile(_sIniPath);

        // get the path to the sversion.ini
        OString sConvertable = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_LOCATION),
                OString());

        OUString sProductVersionFile = rtl::OStringToOUString(sConvertable, nCvtEncoding);

        if (sProductVersionFile.getLength() == 0)
        {
            return BOOTSTRAP_INI_INVALID;
        }

        nStatus = SVERSION_INI_NOT_FOUND;

        // in the path to the product, replace the reference to the user system directory, make it a file URL
        if ( ! normalizeAndSubstitutePathVariables(sProductVersionFile) )
        {
            return BOOTSTRAP_INI_INVALID;
        }

        _sIniPath = sProductVersionFile;

        osl::DirectoryItem aCheckPath;
        if (osl::File::E_None!=aCheckPath.get(sProductVersionFile, aCheckPath) )
        {
            return SVERSION_INI_NOT_FOUND;
        }

        // open the product version profile
        ::osl::Profile aProductVersionFile(sProductVersionFile);

        // get the section/key name of the entry in the product version file
        OString sVersionFileSection = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_SECTION),
                OString(BOOTSTRAP_VALUE_SECTION_DEFAULT));
        OString sVersionFileProductKey = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_PRODUCT_KEY),
                OString(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT));

        // get the user path from the product version file
        sConvertable = aProductVersionFile.readString(
            sVersionFileSection,
            sVersionFileProductKey,
            OString());


        OUString sUserPath = OUString(sConvertable.getStr(), sConvertable.getLength(), nCvtEncoding);
        if (sUserPath.getLength() == 0)
        {
            return SVERSION_INI_NO_ENTRY;
        }

        nStatus = BOOTSTRAP_OK; //every ini read ok
        {
            // normalize the path
            OUString sNormalized;
            // normalize the user directory path
            if ( osl::File::E_None!= osl::File::getFileURLFromSystemPath(sUserPath, sNormalized) )
            {
                return SVERSION_INI_INVALID;
            }
            sUserPath = sNormalized;
        }

        _rUserInstallPath = sUserPath;
        _sIniPath = OUString();
        OSL_ASSERT(nStatus == BOOTSTRAP_OK);
    }
    catch (::std::exception& )
    {   // this exception may be thrown by the Profile ctor - though we checked the arguments we passed to
        // this ctor (thus preventing exceptions), this is just to be sure ...
        OSL_ASSERT(nStatus != BOOTSTRAP_OK);
    }

    return nStatus;
}


// ----------------------------------------------------------------------------------
// retrieve product key and logo information

BootstrapRetVal legacy_bootstrap_getProductKeyAndLogo( OUString& _rProductKey, OUString& _rLogo, OUString& _sIniPath )
{
    OUString aOfficeInstallPath;

    rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

    BootstrapRetVal nStatus = BOOTSTRAP_INI_NOT_FOUND;

    try
    {
        if ( !legacy_locateBootstrapProfile( aOfficeInstallPath, _sIniPath) )
        {
            return BOOTSTRAP_INI_NOT_FOUND;
        }

        // get the path to the sversion.ini file from the bootstrap file
        ::osl::Profile aBootstrapFile(_sIniPath);

        // get the path to the sversion.ini
        OString sConvertable = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_LOCATION),
                OString());

        OUString sProductVersionFile = rtl::OStringToOUString(sConvertable, nCvtEncoding);

        if (sProductVersionFile.getLength() == 0)
        {
            return BOOTSTRAP_INI_INVALID;
        }

        nStatus = SVERSION_INI_NOT_FOUND;

        // in the path to the product, replace the reference to the user system directory, make it a file URL
        if ( ! normalizeAndSubstitutePathVariables(sProductVersionFile) )
        {
            return BOOTSTRAP_INI_INVALID;
        }

        _sIniPath = sProductVersionFile;

        osl::DirectoryItem aCheckPath;
        if (osl::File::E_None!=aCheckPath.get(sProductVersionFile, aCheckPath) )
        {
            return SVERSION_INI_NOT_FOUND;
        }

        // open the product version profile
        ::osl::Profile aProductVersionFile(sProductVersionFile);

        // get the section/key name of the entry in the product version file
        OString sVersionFileSection = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_SECTION),
                OString(BOOTSTRAP_VALUE_SECTION_DEFAULT));
        OString sVersionFileProductKey = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_PRODUCT_KEY),
                OString(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT));
        OString sLogo = aBootstrapFile.readString(
                OString(BOOTSTRAP_SECTION),
                OString(BOOTSTRAP_ITEM_LOGO),
                OString(BOOTSTRAP_VALUE_LOGO_DEFAULT));

        _rProductKey = rtl::OStringToOUString(sVersionFileProductKey, nCvtEncoding);
        _rLogo = rtl::OStringToOUString( sLogo, nCvtEncoding );
        nStatus = BOOTSTRAP_OK;
    }
    catch (::std::exception& )
    {   // this exception may be thrown by the Profile ctor - though we checked the arguments we passed to
        // this ctor (thus preventing exceptions), this is just to be sure ...
        OSL_ASSERT(nStatus != BOOTSTRAP_OK);
    }

    return nStatus;
}
#endif
// ----------------------------------------------------------------------------------

} // namespace utl

