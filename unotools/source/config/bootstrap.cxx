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

#include <config_folders.h>

#include <unotools/bootstrap.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <osl/diagnose.h>

#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>
#include <osl/process.h>

// #define this to true, if remembering defaults is not supported properly
#define RTL_BOOTSTRAP_DEFAULTS_BROKEN true

#define BOOTSTRAP_ITEM_PRODUCT_KEY          "ProductKey"
#define BOOTSTRAP_ITEM_VERSIONFILE          "Location"
#define BOOTSTRAP_ITEM_BUILDID              "buildid"

#define BOOTSTRAP_ITEM_BASEINSTALLATION     "BRAND_BASE_DIR"
#define BOOTSTRAP_ITEM_USERINSTALLATION     "UserInstallation"

#define BOOTSTRAP_ITEM_USERDIR              "UserDataDir"

#define BOOTSTRAP_DEFAULT_BASEINSTALL       "$SYSBINDIR/.."

#define BOOTSTRAP_DIRNAME_USERDIR           "user"

typedef char const * AsciiString;

namespace utl
{

// Implementation class: Bootstrap::Impl

static OUString makeImplName()
{
    OUString uri;
    rtl::Bootstrap::get( "BRAND_BASE_DIR", uri);
    return uri + "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap");
}

class Bootstrap::Impl
{
    const OUString m_aImplName;
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
    Impl() : m_aImplName(makeImplName())
    {
        initialize();
    }

    void initialize();

    // access helper
    OUString getBootstrapValue(OUString const& _sName, OUString const& _sDefault) const;

    const OUString& getImplName() const { return m_aImplName; }

private: // implementation
    bool initBaseInstallationData(rtl::Bootstrap const & _rData);
    bool initUserInstallationData(rtl::Bootstrap const & _rData);
};

namespace
{
    class theImpl : public rtl::Static<Bootstrap::Impl, theImpl> {};
}

const Bootstrap::Impl& Bootstrap::data()
{
    return theImpl::get();
}

bool Bootstrap::getProcessWorkingDir(OUString &rUrl)
{
    rUrl.clear();
    OUString s("$OOO_CWD");
    rtl::Bootstrap::expandMacros(s);
    if (s.isEmpty())
    {
        if (osl_getProcessWorkingDir(&rUrl.pData) == osl_Process_E_None)
            return true;
    }
    else if (s[0] == '1')
    {
        rUrl = s.copy(1);
        return true;
    }
    else if (s[0] == '2' &&
               (osl::FileBase::getFileURLFromSystemPath(s.copy(1), rUrl) ==
                osl::FileBase::E_None))
    {
        return true;
    }
    return false;
}

void Bootstrap::reloadData()
{
    theImpl::get().initialize();
}

// helper

typedef Bootstrap::PathStatus PathStatus;

sal_Unicode const cURLSeparator = '/';

// path status utility function
static PathStatus implCheckStatusOfURL(OUString const& _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    PathStatus eStatus = Bootstrap::DATA_UNKNOWN;

    if (!_sURL.isEmpty())
    {
        switch( DirectoryItem::get(_sURL, aDirItem) )
        {
        case DirectoryItem::E_None:         // Success
            eStatus = Bootstrap::PATH_EXISTS;
            break;

        case DirectoryItem::E_NOENT:        // No such file or directory
            eStatus = Bootstrap::PATH_VALID;
            break;

        case DirectoryItem::E_INVAL:        // the format of the parameters was not valid
        case DirectoryItem::E_NAMETOOLONG:  // File name too long
        case DirectoryItem::E_NOTDIR:       // A component of the path prefix of path is not a directory
            eStatus = Bootstrap::DATA_INVALID;
            break;

        // how to handle these ?
        case DirectoryItem::E_LOOP:         // Too many symbolic links encountered
        case DirectoryItem::E_ACCES:        // permission denied
        // any other error - what to do ?
        default:
            eStatus = Bootstrap::DATA_UNKNOWN;
            break;
        }
    }
    else
    {
        eStatus = Bootstrap::DATA_MISSING;
    }

    return eStatus;
}

static bool implNormalizeURL(OUString & _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    OSL_PRECOND(aDirItem.is(), "Opened DirItem required");

    static const sal_uInt32 cosl_FileStatus_Mask = osl_FileStatus_Mask_FileURL;

    FileStatus aFileStatus(cosl_FileStatus_Mask);

    if (aDirItem.getFileStatus(aFileStatus) != DirectoryItem::E_None)
        return false;

    OUString aNormalizedURL = aFileStatus.getFileURL();

    if (aNormalizedURL.isEmpty())
        return false;

    // #109863# sal/osl returns final slash for file URLs contradicting
    // the URL/URI RFCs.
    if ( !aNormalizedURL.endsWith(OUStringChar(cURLSeparator)) )
        _sURL = aNormalizedURL;
    else
        _sURL = aNormalizedURL.copy( 0, aNormalizedURL.getLength()-1 );

    return true;
}

static bool implEnsureAbsolute(OUString & _rsURL) // also strips embedded dots !!
{
    using osl::File;

    OUString sBasePath;
    OSL_VERIFY(Bootstrap::getProcessWorkingDir(sBasePath));

    OUString sAbsolute;
    if ( File::E_None == File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
    {
        _rsURL = sAbsolute;
        return true;
    }
    else
    {
        OSL_FAIL("Could not get absolute file URL for URL");
        return false;
    }
}

static bool implMakeAbsoluteURL(OUString & _rsPathOrURL)
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

static PathStatus dbgCheckStatusOfURL(OUString const& _sURL)
{
    using namespace osl;

    DirectoryItem aDirItem;

    return implCheckStatusOfURL(_sURL,aDirItem);
}

static PathStatus checkStatusAndNormalizeURL(OUString & _sURL)
{
    using namespace osl;

    PathStatus eStatus = Bootstrap::DATA_UNKNOWN;

    if (_sURL.isEmpty())
        eStatus = Bootstrap::DATA_MISSING;

    else if ( !implMakeAbsoluteURL(_sURL) )
        eStatus = Bootstrap::DATA_INVALID;

    else
    {
        DirectoryItem aDirItem;

        eStatus = implCheckStatusOfURL(_sURL,aDirItem);

        if (eStatus == Bootstrap::PATH_EXISTS && !implNormalizeURL(_sURL,aDirItem))
            OSL_FAIL("Unexpected failure getting actual URL for existing object");
    }
    return eStatus;
}

// helpers to build and check a nested URL
static PathStatus getDerivedPath(
              OUString& _rURL,
              OUString const& _aBaseURL, PathStatus _aBaseStatus,
              OUString const& _sRelativeURL,
              rtl::Bootstrap const & _rData, OUString const& _sBootstrapParameter
          )
{
    OUString sDerivedURL;
    OSL_PRECOND(!_rData.getFrom(_sBootstrapParameter,sDerivedURL),"Setting for derived path is already defined");
    OSL_PRECOND(!_sRelativeURL.isEmpty() && _sRelativeURL[0] != cURLSeparator,"Invalid Relative URL");

    PathStatus aStatus = _aBaseStatus;

    // do we have a base path ?
    if (!_aBaseURL.isEmpty())
    {
        OSL_PRECOND(!_aBaseURL.endsWith(OUStringChar(cURLSeparator)), "Unexpected: base URL ends in slash");

        sDerivedURL = _aBaseURL + OUStringChar(cURLSeparator) + _sRelativeURL;

        // a derived (nested) URL can only exist or have a lesser status, if the parent exists
        if (aStatus == Bootstrap::PATH_EXISTS)
            aStatus = checkStatusAndNormalizeURL(sDerivedURL);

        else // the relative appendix must be valid
            OSL_ASSERT(aStatus != Bootstrap::PATH_VALID || dbgCheckStatusOfURL(sDerivedURL) == Bootstrap::PATH_VALID);

        _rData.getFrom(_sBootstrapParameter, _rURL, sDerivedURL);

        OSL_ENSURE(sDerivedURL == _rURL,"Could not set derived URL via Bootstrap default parameter");
        SAL_WARN_IF( !(RTL_BOOTSTRAP_DEFAULTS_BROKEN || (_rData.getFrom(_sBootstrapParameter,sDerivedURL) && sDerivedURL==_rURL)),
            "unotools.config",
            "Use of default did not affect bootstrap value");
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

static PathStatus getDerivedPath(
                      OUString& _rURL,
                      Bootstrap::Impl::PathData const& _aBaseData,
                      OUString const& _sRelativeURL,
                      rtl::Bootstrap const & _rData, OUString const& _sBootstrapParameter
              )
{
    return getDerivedPath(_rURL,_aBaseData.path,_aBaseData.status,_sRelativeURL,_rData,_sBootstrapParameter);
}

static OUString getExecutableBaseName()
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
        SAL_WARN("unotools.config", "Cannot get executable name: osl_getExecutableFile failed");

    return sExecutable;
}

static Bootstrap::PathStatus updateStatus(Bootstrap::Impl::PathData & _rResult)
{
    _rResult.status = checkStatusAndNormalizeURL(_rResult.path);
    return _rResult.status;
}

static Bootstrap::PathStatus implGetBootstrapFile(rtl::Bootstrap const & _rData, Bootstrap::Impl::PathData & _rBootstrapFile)
{
    _rData.getIniName(_rBootstrapFile.path);

    return updateStatus(_rBootstrapFile);
}

static Bootstrap::PathStatus implGetVersionFile(rtl::Bootstrap const & _rData, Bootstrap::Impl::PathData & _rVersionFile)
{
    _rData.getFrom(BOOTSTRAP_ITEM_VERSIONFILE, _rVersionFile.path);

    return updateStatus(_rVersionFile);
}

// Error reporting

char const IS_MISSING[] = "is missing";
char const IS_INVALID[] = "is corrupt";
char const PERIOD[] = ". ";

static void addFileError(OUStringBuffer& _rBuf, OUString const& _aPath, AsciiString _sWhat)
{
    OUString sSimpleFileName = _aPath.copy(1 +_aPath.lastIndexOf(cURLSeparator));

    _rBuf.append("The configuration file");
    _rBuf.append(" '").append(sSimpleFileName).append("' ");
    _rBuf.appendAscii(_sWhat).append(PERIOD);
}

static void addMissingDirectoryError(OUStringBuffer& _rBuf, OUString const& _aPath)
{
    _rBuf.append("The configuration directory");
    _rBuf.append(" '").append(_aPath).append("' ");
    _rBuf.append(IS_MISSING).append(PERIOD);
}

static void addUnexpectedError(OUStringBuffer& _rBuf, AsciiString _sExtraInfo = nullptr)
{
    if (nullptr == _sExtraInfo)
        _sExtraInfo = "An internal failure occurred";

    _rBuf.appendAscii(_sExtraInfo).append(PERIOD);
}

static Bootstrap::FailureCode describeError(OUStringBuffer& _rBuf, Bootstrap::Impl const& _rData)
{
    Bootstrap::FailureCode eErrCode = Bootstrap::INVALID_BOOTSTRAP_DATA;

    _rBuf.append("The program cannot be started. ");

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
        [[fallthrough]];

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

            case Bootstrap::DATA_INVALID: OSL_ASSERT(false); [[fallthrough]];
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


OUString Bootstrap::getProductKey()
{
    OUString const sDefaultProductKey = getExecutableBaseName();

    return data().getBootstrapValue( BOOTSTRAP_ITEM_PRODUCT_KEY, sDefaultProductKey );
}

OUString Bootstrap::getProductKey(OUString const& _sDefault)
{
    return data().getBootstrapValue( BOOTSTRAP_ITEM_PRODUCT_KEY, _sDefault );
}

OUString Bootstrap::getBuildIdData(OUString const& _sDefault)
{
    // try to open version.ini (versionrc)
    OUString uri;
    rtl::Bootstrap::get( "BRAND_BASE_DIR", uri);
    rtl::Bootstrap aData( uri + "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") );
    if ( aData.getHandle() == nullptr )
        // version.ini (versionrc) doesn't exist
        return _sDefault;

    // read value
    OUString sBuildId;
    aData.getFrom(BOOTSTRAP_ITEM_BUILDID,sBuildId,_sDefault);
    return sBuildId;
}

Bootstrap::PathStatus Bootstrap::locateBaseInstallation(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aBaseInstall_;

    _rURL = aPathData.path;
    return aPathData.status;
}

Bootstrap::PathStatus Bootstrap::locateUserInstallation(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aUserInstall_;

    _rURL = aPathData.path;
    return aPathData.status;
}

Bootstrap::PathStatus Bootstrap::locateUserData(OUString& _rURL)
{
    OUString const csUserDirItem(BOOTSTRAP_ITEM_USERDIR);

    rtl::Bootstrap aData( data().getImplName() );

    if ( aData.getFrom(csUserDirItem, _rURL) )
    {
        return checkStatusAndNormalizeURL(_rURL);
    }
    else
    {
        return getDerivedPath(_rURL, data().aUserInstall_ ,BOOTSTRAP_DIRNAME_USERDIR, aData, csUserDirItem);
    }
}

Bootstrap::PathStatus Bootstrap::locateBootstrapFile(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aBootstrapINI_;

    _rURL = aPathData.path;
    return aPathData.status;
}

Bootstrap::PathStatus Bootstrap::locateVersionFile(OUString& _rURL)
{
    Impl::PathData const& aPathData = data().aVersionINI_;

    _rURL = aPathData.path;
    return aPathData.status;
}

Bootstrap::Status Bootstrap::checkBootstrapStatus(OUString& _rDiagnosticMessage, FailureCode& _rErrCode)
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


bool Bootstrap::Impl::initBaseInstallationData(rtl::Bootstrap const & _rData)
{
    _rData.getFrom(BOOTSTRAP_ITEM_BASEINSTALLATION, aBaseInstall_.path, BOOTSTRAP_DEFAULT_BASEINSTALL);

    bool bResult = (PATH_EXISTS == updateStatus(aBaseInstall_));

    implGetBootstrapFile(_rData, aBootstrapINI_);

    return bResult;
}

bool Bootstrap::Impl::initUserInstallationData(rtl::Bootstrap const & _rData)
{
    if (_rData.getFrom(BOOTSTRAP_ITEM_USERINSTALLATION, aUserInstall_.path))
    {
        updateStatus(aUserInstall_);
    }
    else
    {
        // should we do just this
        aUserInstall_.status = DATA_MISSING;

        // ... or this - look for a single-user user directory ?
        OUString const csUserDirItem(BOOTSTRAP_ITEM_USERDIR);
        OUString sDummy;
        // look for $BASEINSTALLATION/user only if default UserDir setting is used
        if (! _rData.getFrom(csUserDirItem, sDummy))
        {
            if ( PATH_EXISTS == getDerivedPath(sDummy, aBaseInstall_, BOOTSTRAP_DIRNAME_USERDIR, _rData, csUserDirItem) )
                aUserInstall_ = aBaseInstall_;
        }
    }

    bool bResult = (PATH_EXISTS == aUserInstall_.status);

    implGetVersionFile(_rData, aVersionINI_);

    return bResult;
}

void Bootstrap::Impl::initialize()
{
    rtl::Bootstrap aData( m_aImplName );

    if (!initBaseInstallationData(aData))
    {
        status_ = INVALID_BASE_INSTALL;
    }
    else if (!initUserInstallationData(aData))
    {
        status_ = INVALID_USER_INSTALL;

        if (aUserInstall_.status >= DATA_MISSING)
        {
            switch (aVersionINI_.status)
            {
            case PATH_EXISTS:
            case PATH_VALID:
                status_ = MISSING_USER_INSTALL;
                break;

            case DATA_INVALID:
            case DATA_MISSING:
                status_ = INVALID_BASE_INSTALL;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        status_ = DATA_OK;
    }
}

OUString Bootstrap::Impl::getBootstrapValue(OUString const& _sName, OUString const& _sDefault) const
{
    rtl::Bootstrap aData( m_aImplName );

    OUString sResult;
    aData.getFrom(_sName,sResult,_sDefault);
    return sResult;
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
