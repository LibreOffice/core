/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 14:42:49 $
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

#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h> // for osl_getExecutableFile
#endif

// ---------------------------------------------------------------------------------------
// #define this to a non-zero value, if remembering defaults is not supported properly
#define RTL_BOOTSTRAP_DEFAULTS_BROKEN 1

// ---------------------------------------------------------------------------------------
#define BOOTSTRAP_DATA_NAME                 SAL_CONFIGFILE("bootstrap")

#define BOOTSTRAP_ITEM_PRODUCT_KEY          "ProductKey"
#define BOOTSTRAP_ITEM_VERSIONFILE          "Location"
#define BOOTSTRAP_ITEM_LOGO                 "Logo"
#define BOOTSTRAP_ITEM_BUILDID              "buildid"
#define BOOTSTRAP_ITEM_INSTALLMODE          "InstallMode"

#define BOOTSTRAP_ITEM_BASEINSTALLATION     "BaseInstallation"
#define BOOTSTRAP_ITEM_USERINSTALLATION     "UserInstallation"

#define BOOTSTRAP_ITEM_SHAREDIR             "SharedDataDir"
#define BOOTSTRAP_ITEM_USERDIR              "UserDataDir"

#define BOOTSTRAP_ITEM_PRODUCT_PATCH_LEVEL  "ProductPatch"

#define BOOTSTRAP_DEFAULT_BASEINSTALL       "$SYSBINDIR/.."

#define BOOTSTRAP_DIRNAME_SHAREDIR          "share"
#define BOOTSTRAP_DIRNAME_USERDIR           "user"

#define VERSIONFILE_SECTION                 "Versions"
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
        OUString const m_aImplName;
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
        PathData aBaseInstall_;

        // user install data
        PathData aUserInstall_;

        // INI files
        PathData aBootstrapINI_;
        PathData aVersionINI_;

        // overall status
        Status status_;

    public: // construction and initialization
        explicit
        Impl(OUString const& _aImplName)
        : m_aImplName(_aImplName)
        {
            status_ = initialize();
        }

        Status initialize();

        // access helper
        OUString getBootstrapValue(OUString const& _sName, OUString const& _sDefault) const;

        OUString getImplName() const { return m_aImplName; }

    private: // implementation
        bool initBaseInstallationData(rtl::Bootstrap& _rData);
        bool initUserInstallationData(rtl::Bootstrap& _rData);
    };
// ---------------------------------------------------------------------------------------
    static OUString getExecutableDirectory();
// ---------------------------------------------------------------------------------------

    static Bootstrap::Impl* s_pData = NULL;

    Bootstrap::Impl const& Bootstrap::data()
    {

        if (!s_pData)
        {
            using namespace osl;
            MutexGuard aGuard( Mutex::getGlobalMutex() );

            // static Impl s_theData(getExecutableDirectory() + OUString(RTL_CONSTASCII_USTRINGPARAM("/"BOOTSTRAP_DATA_NAME)));
            // s_pData = &s_theData;
            s_pData = new Impl(getExecutableDirectory() + OUString(RTL_CONSTASCII_USTRINGPARAM("/"BOOTSTRAP_DATA_NAME)));
        }
        return *s_pData;
    }

    void Bootstrap::reloadData()
    {
        if (s_pData != NULL) {
            delete s_pData;
            s_pData = NULL;
        }
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
PathStatus implCheckStatusOfURL(OUString const& _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    PathStatus eStatus = Bootstrap::DATA_UNKNOWN;

    if (_sURL.getLength() != 0)
    {
        switch( DirectoryItem::get(_sURL, aDirItem) )
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
// ---------------------------------------------------------------------------------------

static
bool implNormalizeURL(OUString & _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    OSL_PRECOND(aDirItem.is(), "Opened DirItem required");

    static const sal_uInt32 cFileStatusMask = FileStatusMask_FileURL;

    FileStatus aFileStatus(cFileStatusMask);

    if (aDirItem.getFileStatus(aFileStatus) != DirectoryItem::E_None)
        return false;

    OUString aNormalizedURL = aFileStatus.getFileURL();

    if (aNormalizedURL.getLength() == 0)
        return false;

    // #109863# sal/osl returns final slash for file URLs contradicting
    // the URL/URI RFCs.
    if ( aNormalizedURL.getStr()[aNormalizedURL.getLength()-1] != cURLSeparator )
        _sURL = aNormalizedURL;
    else
        _sURL = aNormalizedURL.copy( 0, aNormalizedURL.getLength()-1 );

    return true;
}
// ---------------------------------------------------------------------------------------
static
bool implEnsureAbsolute(OUString & _rsURL) // also strips embedded dots !!
{
    using osl::File;

    OUString sBasePath = _rsURL;
    OSL_VERIFY(osl_Process_E_None == osl_getProcessWorkingDir(&sBasePath.pData));

    OUString sAbsolute;
    if ( File::E_None == File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
    {
        _rsURL = sAbsolute;
        return true;
    }
    else
    {
        OSL_ENSURE(false, "Could not get absolute file URL for URL");
        return false;
    }
}
/*  old code to strip embedded dots
    static OUString const sDots(RTL_CONSTASCII_USTRINGPARAM("/.."));

    sal_Int32 nDotsIndex = _rsURL.indexOf(sDots);
    while (nDotsIndex >= 0)
    {
        OSL_ASSERT(_rsURL.indexOf(sDots) == nDotsIndex);

        sal_Int32 nStripIndex = _rsURL.lastIndexOf(cURLSeparator,nDotsIndex);
        if (nStripIndex < 0 || nStripIndex+1 == nDotsIndex)
        {
            OSL_TRACE("Invalid use of dots in bootstrap URL");
            return false;
        }
        _rsURL = _rsURL.copy(0,nStripIndex) + _rsURL.copy(nDotsIndex + sDots.getLength());

        nDotsIndex = _rsURL.indexOf(sDots,nStripIndex);
    }
    return true;
}

*/
// ---------------------------------------------------------------------------------------

static
bool implMakeAbsoluteURL(OUString & _rsPathOrURL)
{
    using namespace osl;

    bool bURL;

    OUString sOther;
    // check if it already was normalized
    if ( File::E_None == File::getSystemPathFromFileURL(_rsPathOrURL, sOther) )
    {
        bURL = true;
    }

    else if ( File::E_None == File::getFileURLFromSystemPath(_rsPathOrURL, sOther) )
    {
        _rsPathOrURL = sOther;
        bURL = true;
    }
    else
        bURL = false;

    return bURL && implEnsureAbsolute(_rsPathOrURL);
}
// ---------------------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
static
PathStatus dbgCheckStatusOfURL(OUString const& _sURL)
{
    using namespace osl;

    DirectoryItem aDirItem;

    return implCheckStatusOfURL(_sURL,aDirItem);
}
// ---------------------------------------------------------------------------------------
#endif

static
bool normalizeURL(OUString & _sURL)
{
    using namespace osl;

    DirectoryItem aDirItem;

    bool bResult = DirectoryItem::get(_sURL, aDirItem) == DirectoryItem::E_None;

    if ( bResult )
        bResult = implNormalizeURL(_sURL,aDirItem);

    return bResult;
}
// ---------------------------------------------------------------------------------------

static
PathStatus checkStatusAndNormalizeURL(OUString & _sURL)
{
    using namespace osl;

    PathStatus eStatus = Bootstrap::DATA_UNKNOWN;

    if (_sURL.getLength() == 0)
        eStatus = Bootstrap::DATA_MISSING;

    else if ( !implMakeAbsoluteURL(_sURL) )
        eStatus = Bootstrap::DATA_INVALID;

    else
    {
        DirectoryItem aDirItem;

        eStatus = implCheckStatusOfURL(_sURL,aDirItem);

        if (eStatus == Bootstrap::PATH_EXISTS)
        {
            if (!implNormalizeURL(_sURL,aDirItem))
                OSL_ENSURE(false,"Unexpected failure getting actual URL for existing object");
        }
    }
    return eStatus;
}


// ----------------------------------------------------------------------------------
// helpers to build and check a nested URL
static
PathStatus getDerivedPath(
              OUString& _rURL,
              OUString const& _aBaseURL, PathStatus _aBaseStatus,
              OUString const& _sRelativeURL,
              rtl::Bootstrap& _rData, OUString const& _sBootstrapParameter
          )
{
    OUString sDerivedURL;

    OSL_PRECOND(!_rData.getFrom(_sBootstrapParameter,sDerivedURL),"Setting for derived path is already defined");
    OSL_PRECOND(_sRelativeURL.getLength() != 0 && _sRelativeURL[0] != cURLSeparator,"Invalid Relative URL");

    PathStatus aStatus = _aBaseStatus;

    // do we have a base path ?
    if (_aBaseURL.getLength())
    {
        OSL_PRECOND(_aBaseURL[_aBaseURL.getLength()-1] != cURLSeparator,"Unexpected: base URL ends in slash");

        sDerivedURL = _aBaseURL + getURLSeparator() + _sRelativeURL;

        // a derived (nested) URL can only exist or have a lesser status, if the parent exists
        if (aStatus == Bootstrap::PATH_EXISTS)
            aStatus = checkStatusAndNormalizeURL(sDerivedURL);

        else // the relative appendix must be valid
            OSL_ASSERT(aStatus != Bootstrap::PATH_VALID || dbgCheckStatusOfURL(sDerivedURL) == Bootstrap::PATH_VALID);

        _rData.getFrom(_sBootstrapParameter, _rURL, sDerivedURL);

        OSL_ENSURE(sDerivedURL == _rURL,"Could not set derived URL via Bootstrap default parameter");
        OSL_POSTCOND(RTL_BOOTSTRAP_DEFAULTS_BROKEN ||
                    _rData.getFrom(_sBootstrapParameter,sDerivedURL) && sDerivedURL==_rURL,"Use of default did not affect bootstrap value");
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
PathStatus getDerivedPath(
              OUString& _rURL,
              Bootstrap::Impl::PathData const& _aBaseData,
              OUString const& _sRelativeURL,
              rtl::Bootstrap& _rData, OUString const& _sBootstrapParameter
          )
{
    return getDerivedPath(_rURL,_aBaseData.path,_aBaseData.status,_sRelativeURL,_rData,_sBootstrapParameter);
}

// ---------------------------------------------------------------------------------------

static
OUString getExecutableBaseName()
{
    OUString sExecutable;

    if (osl_Process_E_None == osl_getExecutableFile(&sExecutable.pData))
    {
        // split the executable name
        sal_Int32 nSepIndex = sExecutable.lastIndexOf(cURLSeparator);

        sExecutable = sExecutable.copy(nSepIndex + 1);

        // ... and get the basename (strip the extension)
        sal_Unicode const cExtensionSep = '.';

        sal_Int32 const nExtIndex =     sExecutable.lastIndexOf(cExtensionSep);
        sal_Int32 const nExtLength =    sExecutable.getLength() - nExtIndex - 1;
        if (0 < nExtIndex && nExtLength < 4)
           sExecutable  = sExecutable.copy(0,nExtIndex);
    }
    else
        OSL_TRACE("Cannot get executable name: osl_getExecutableFile failed\n");

    return sExecutable;
}

// ---------------------------------------------------------------------------------------
static
OUString getExecutableDirectory()
{
    OUString sFileName;
    OSL_VERIFY(osl_Process_E_None == osl_getExecutableFile(&sFileName.pData));

    sal_Int32 nDirEnd = sFileName.lastIndexOf(cURLSeparator);

    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    return sFileName.copy(0,nDirEnd);
}

// ----------------------------------------------------------------------------------

static
inline
Bootstrap::PathStatus updateStatus(Bootstrap::Impl::PathData & _rResult)
{
    return _rResult.status = checkStatusAndNormalizeURL(_rResult.path);
}
// ---------------------------------------------------------------------------------------

static
Bootstrap::PathStatus implGetBootstrapFile(rtl::Bootstrap& _rData, Bootstrap::Impl::PathData & _rBootstrapFile)
{
    _rData.getIniName(_rBootstrapFile.path);

    return updateStatus(_rBootstrapFile);
}
// ---------------------------------------------------------------------------------------

static
Bootstrap::PathStatus implGetVersionFile(rtl::Bootstrap& _rData, Bootstrap::Impl::PathData & _rVersionFile)
{
    OUString const csVersionFileItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_VERSIONFILE));

    _rData.getFrom(csVersionFileItem,_rVersionFile.path);

    return updateStatus(_rVersionFile);
}
// ---------------------------------------------------------------------------------------
// Error reporting

static char const IS_MISSING[] = "is missing";
static char const IS_INVALID[] = "is corrupt";
static char const PERIOD[] = ". ";

// ---------------------------------------------------------------------------------------
static void addFileError(OUStringBuffer& _rBuf, OUString const& _aPath, AsciiString _sWhat)
{
    OUString sSimpleFileName = _aPath.copy(1 +_aPath.lastIndexOf(cURLSeparator));

    _rBuf.appendAscii("The configuration file");
    _rBuf.appendAscii(" '").append(sSimpleFileName).appendAscii("' ");
    _rBuf.appendAscii(_sWhat).appendAscii(PERIOD);
}
// ---------------------------------------------------------------------------------------

static void addMissingDirectoryError(OUStringBuffer& _rBuf, OUString const& _aPath)
{
    _rBuf.appendAscii("The configuration directory");
    _rBuf.appendAscii(" '").append(_aPath).appendAscii("' ");
    _rBuf.appendAscii(IS_MISSING).appendAscii(PERIOD);
}
// ---------------------------------------------------------------------------------------

static void addUnexpectedError(OUStringBuffer& _rBuf, AsciiString _sExtraInfo = NULL)
{
    if (NULL == _sExtraInfo)
        _sExtraInfo = "An internal failure occurred";

    _rBuf.appendAscii(_sExtraInfo).appendAscii(PERIOD);
}
// ---------------------------------------------------------------------------------------

static Bootstrap::FailureCode describeError(OUStringBuffer& _rBuf, Bootstrap::Impl const& _rData)
{
    Bootstrap::FailureCode eErrCode = Bootstrap::INVALID_BOOTSTRAP_DATA;

    _rBuf.appendAscii("The program cannot be started. ");

    switch (_rData.aUserInstall_.status)
    {
    case Bootstrap::PATH_EXISTS:
        switch (_rData.aBaseInstall_.status)
        {
        case Bootstrap::PATH_VALID:
            addMissingDirectoryError(_rBuf, _rData.aBaseInstall_.path);
            eErrCode = Bootstrap::MISSING_INSTALL_DIRECTORY;
            break;

        case Bootstrap::DATA_INVALID:
            addUnexpectedError(_rBuf,"The installation path is invalid");
            break;

        case Bootstrap::DATA_MISSING:
            addUnexpectedError(_rBuf,"The installation path is not available");
            break;

        case Bootstrap::PATH_EXISTS: // seems to be all fine (?)
            addUnexpectedError(_rBuf,"");
            break;

        default: OSL_ASSERT(false);
            addUnexpectedError(_rBuf);
            break;
        }
        break;

    case Bootstrap::PATH_VALID:
        addMissingDirectoryError(_rBuf, _rData.aUserInstall_.path);
        eErrCode = Bootstrap::MISSING_USER_DIRECTORY;
        break;

        // else fall through
    case Bootstrap::DATA_INVALID:
        if (_rData.aVersionINI_.status == Bootstrap::PATH_EXISTS)
        {
            addFileError(_rBuf, _rData.aVersionINI_.path, IS_INVALID);
            eErrCode = Bootstrap::INVALID_VERSION_FILE_ENTRY;
            break;
        }
        // else fall through

    case Bootstrap::DATA_MISSING:
        switch (_rData.aVersionINI_.status)
        {
        case Bootstrap::PATH_EXISTS:
            addFileError(_rBuf, _rData.aVersionINI_.path, "does not support the current version");
            eErrCode = Bootstrap::MISSING_VERSION_FILE_ENTRY;
            break;

        case Bootstrap::PATH_VALID:
            addFileError(_rBuf, _rData.aVersionINI_.path, IS_MISSING);
            eErrCode = Bootstrap::MISSING_VERSION_FILE;
            break;

        default:
            switch (_rData.aBootstrapINI_.status)
            {
            case Bootstrap::PATH_EXISTS:
                addFileError(_rBuf, _rData.aBootstrapINI_.path, IS_INVALID);

                if (_rData.aVersionINI_.status == Bootstrap::DATA_MISSING)
                    eErrCode = Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY;
                else
                    eErrCode = Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY;
                break;

            case Bootstrap::DATA_INVALID: OSL_ASSERT(false);
            case Bootstrap::PATH_VALID:
                addFileError(_rBuf, _rData.aBootstrapINI_.path, IS_MISSING);
                eErrCode = Bootstrap::MISSING_BOOTSTRAP_FILE;
                break;

            default:
                addUnexpectedError(_rBuf);
                break;
            }
            break;
        }
        break;

    default: OSL_ASSERT(false);
        addUnexpectedError(_rBuf);
        break;
    }

    return eErrCode;
}
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// class Bootstrap
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getProductKey()
{
    OUString const csProductKeyItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_PRODUCT_KEY));

    OUString const sDefaultProductKey = getExecutableBaseName();

    return data().getBootstrapValue( csProductKeyItem, sDefaultProductKey );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getProductKey(OUString const& _sDefault)
{
    OUString const csProductKeyItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_PRODUCT_KEY));

    return data().getBootstrapValue( csProductKeyItem, _sDefault );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getLogoData(OUString const& _sDefault)
{
    OUString const csLogoItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_LOGO));

    return data().getBootstrapValue( csLogoItem, _sDefault );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getBuildIdData(OUString const& _sDefault)
{
    OUString const csBuildIdItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_BUILDID));

    return data().getBootstrapValue( csBuildIdItem, _sDefault );
}
// ---------------------------------------------------------------------------------------

/// retrieve the installation mode information item; uses the given default, if not found
OUString Bootstrap::getInstallMode(rtl::OUString const& _sDefault)
{
    OUString const csInstallModeItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_INSTALLMODE));

    return data().getBootstrapValue( csInstallModeItem, _sDefault );
}
// ---------------------------------------------------------------------------------------

OUString Bootstrap::getProductPatchLevel(OUString const& _sDefault)
{
    OUString const csBuildIdItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_PRODUCT_PATCH_LEVEL));

    return data().getBootstrapValue( csBuildIdItem, _sDefault );
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
    OUString const csShareDirItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_SHAREDIR));

    rtl::Bootstrap aData( data().getImplName() );

    if ( aData.getFrom(csShareDirItem, _rURL) )
    {
        return checkStatusAndNormalizeURL(_rURL);
    }
    else
    {
        OUString const csShareDir(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_DIRNAME_SHAREDIR));
        return getDerivedPath(_rURL, data().aBaseInstall_, csShareDir, aData, csShareDirItem);
    }
}
// ---------------------------------------------------------------------------------------

PathStatus Bootstrap::locateUserData(OUString& _rURL)
{
    OUString const csUserDirItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_USERDIR));

    rtl::Bootstrap aData( data().getImplName() );

    if ( aData.getFrom(csUserDirItem, _rURL) )
    {
        return checkStatusAndNormalizeURL(_rURL);
    }
    else
    {
        OUString const csUserDir(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_DIRNAME_USERDIR));
        return getDerivedPath(_rURL, data().aUserInstall_ ,csUserDir, aData, csUserDirItem);
    }
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
    FailureCode eDummyCode(NO_FAILURE);

    return checkBootstrapStatus(_rDiagnosticMessage,eDummyCode);
}
// ---------------------------------------------------------------------------------------

Bootstrap::Status Bootstrap::checkBootstrapStatus(rtl::OUString& _rDiagnosticMessage, FailureCode& _rErrCode)
{
    Impl const& aData = data();

    Status result = aData.status_;

    // maybe do further checks here

    OUStringBuffer sErrorBuffer;
    if (result != DATA_OK)
        _rErrCode = describeError(sErrorBuffer,aData);

    else
        _rErrCode = NO_FAILURE;

    _rDiagnosticMessage = sErrorBuffer.makeStringAndClear();

    return result;
}

// ---------------------------------------------------------------------------------------
// class Bootstrap::Impl
// ---------------------------------------------------------------------------------------

bool Bootstrap::Impl::initBaseInstallationData(rtl::Bootstrap& _rData)
{
    OUString const csBaseInstallItem( RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_BASEINSTALLATION) );
    OUString const csBaseInstallDefault( RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_DEFAULT_BASEINSTALL) );

    _rData.getFrom(csBaseInstallItem, aBaseInstall_.path, csBaseInstallDefault);

    bool bResult = (PATH_EXISTS == updateStatus(aBaseInstall_));

    implGetBootstrapFile(_rData, aBootstrapINI_);

    return bResult;
}
// ---------------------------------------------------------------------------------------

bool Bootstrap::Impl::initUserInstallationData(rtl::Bootstrap& _rData)
{
    OUString const csUserInstallItem( RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_USERINSTALLATION) );

    if (_rData.getFrom(csUserInstallItem, aUserInstall_.path))
    {
        updateStatus(aUserInstall_);
    }
    else
    {
        // should we do just this
        aUserInstall_.status = DATA_MISSING;

        // .. or this - look for a single-user user directory ?
        OUString const csUserDirItem(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_ITEM_USERDIR));
        OUString sDummy;
        // look for $BASEINSTALLATION/user only if default UserDir setting is used
        if (! _rData.getFrom(csUserDirItem, sDummy))
        {
            OUString const csUserDir(RTL_CONSTASCII_USTRINGPARAM(BOOTSTRAP_DIRNAME_USERDIR));

            if ( PATH_EXISTS == getDerivedPath(sDummy, aBaseInstall_, csUserDir, _rData, csUserDirItem) )
                aUserInstall_ = aBaseInstall_;
        }
    }

    bool bResult = (PATH_EXISTS == aUserInstall_.status);

    implGetVersionFile(_rData, aVersionINI_);

    return bResult;
}
// ---------------------------------------------------------------------------------------

Bootstrap::Status Bootstrap::Impl::initialize()
{
    Bootstrap::Status result;

    rtl::Bootstrap aData( m_aImplName );

    if (!initBaseInstallationData(aData))
    {
        result = INVALID_BASE_INSTALL;
    }
    else if (!initUserInstallationData(aData))
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

OUString Bootstrap::Impl::getBootstrapValue(OUString const& _sName, OUString const& _sDefault) const
{
    rtl::Bootstrap aData( m_aImplName );

    OUString sResult;
    aData.getFrom(_sName,sResult,_sDefault);
    return sResult;
}
// ---------------------------------------------------------------------------------------

} // namespace utl

