/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-07-06 07:17:45 $
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

#ifndef _UTL_BOOTSTRAP_HXX
#include "unotools/bootstrap.hxx"
#endif

#ifndef _OSL_PROFILE_HXX_
#include <osl/profile.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _OSL_SECURITY_H_
#include <osl/security.h>
#endif
#undef INI_FILE_NAME

#ifdef UNX
#define INI_FILE_NAME( name ) #name "rc"
#else
#define INI_FILE_NAME( name ) #name ".ini"
#endif

#define LOCAL_SESSION_IDENTIFIER            "local"

#define BOOTSTRAP_FILE                  INI_FILE_NAME( bootstrap )
#define CONFIGURATION_PROFILE_NAME      INI_FILE_NAME( sregistry )

#define BOOTSTRAP_SECTION                   "Bootstrap"
#define BOOTSTRAP_ITEM_PRODUCT_KEY          "ProductKey"
#define BOOTSTRAP_ITEM_LOCATION             "Location"
#define BOOTSTRAP_ITEM_SECTION              "Section"
#define BOOTSTRAP_ITEM_LOGO                 "Logo"

#define BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT "StarOffice 6.0"
#define BOOTSTRAP_VALUE_SECTION_DEFAULT     "Versions"
#define BOOTSTRAP_VALUE_LOGO_DEFAULT        "1"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
namespace uno = ::com::sun::star::uno;

// ---------------------------------------------------------------------------------------
typedef char const * AsciiString;

namespace utl{

// ---------------------------------------------------------------------------------------

BootstrapRetVal locateBootstrapFiles(OUString& _rOfficeInstall, OUString& _rUserInstallPath, OUString& _rProfilePath);
BootstrapRetVal getProductKeyAndLogo( OUString& _rProductKey, OUString& _rLogo, OUString& _sIniPath );

// ---------------------------------------------------------------------------------------

inline bool oslOK(::osl::FileBase::RC code) { return code == ::osl::FileBase::E_None; }
inline bool oslOK(oslFileError code) { return code == osl_File_E_None; }

// ---------------------------------------------------------------------------------------
BootstrapResult const mapBootstrapResult[/*RetVal*/] =
{
    BOOTSTRAP_DATA_OK,      //  BOOTSTRAP_OK,
    MISSING_BOOTSTRAP_FILE, //  BOOTSTRAP_INI_NOT_FOUND,
    INVALID_BOOTSTRAP_DATA, //  BOOTSTRAP_INI_INVALID,
    INVALID_INSTALLATION,   //  SVERSION_INI_NOT_FOUND,
    INVALID_INSTALLATION,   //  SVERSION_INI_NO_ENTRY,
    INVALID_BOOTSTRAP_DATA, //  SVERSION_INI_INVALID,
    MISSING_BOOTSTRAP_FILE, //  SREGISTRY_INI_NOT_FOUND
    INVALID_BOOTSTRAP_DATA, //  SREGISTRY_INI_INVALID
};

// ---------------------------------------------------------------------------------------
OUString getBootstrapErrorMessage( BootstrapResult rc )
{
    sal_Char const * const mapErrMsg[/*BootstrapResult*/] =
    {
        "Bootstrap OK",                     // BOOTSTRAP_DATA_OK,
        "A bootstrap file is missing",      // MISSING_BOOTSTRAP_FILE,
        "Invalid bootstrap data",           // INVALID_BOOTSTRAP_DATA,
        "Invalid user installation",        // INVALID_INSTALLATION
        "An unexpected failure occurred while bootstrapping" // BOOTSTRAP_FAILURE
    };
    OSL_ENSURE(0 <= rc && rc < (sizeof mapErrMsg/sizeof 0[mapErrMsg]), "FATAL ERROR: Result code out of range in getBootstrapErrorMessage()");

    return OUString::createFromAscii( mapErrMsg[rc] );
}

// ---------------------------------------------------------------------------------------
static bool isEmptySettingValue(uno::Any const& aAny)
{
    if (!aAny.hasValue())
    {
        return true;
    }


    // string check
    OUString sStringCheck;
    if (aAny >>= sStringCheck)
    {
        // it's a string - check if empty
        return (0 == sStringCheck.getLength());
    }

    // boolean check - 'false' must be accepted
    if (aAny.getValueType() == ::getBooleanCppuType())
    {
        return false;
    }

    // integer check
    sal_Int32 nIntCheck = 0;
    if (aAny >>= nIntCheck)
    {
        // it's an int - check for zero
        return(0 == nIntCheck);
    }

    OSL_ENSURE(false, "Unknown settings type");
    return false; // nevertheless accept
}

// ---------------------------------------------------------------------------------------
// - helper
// ---------------------------------------------------------------------------------------

// normalizeAndSubstitutePathVariables
// ---------------------------------------------------------------------------------------

bool normalizeAndSubstitutePathVariables(OUString& _rPath)
{
    OSL_PRECOND(_rPath.getLength() != 0 , "Invalid parameter: Empty path in normalizeAndSubstitutePathVariables" );

    typedef sal_Bool (SAL_CALL * getSystemDirectoryFunction)(oslSecurity, rtl_uString **);
    // recognized variables and methods to retrieve their substitute pathes
    // Note: variables are expected to be a system path and thus must be at the beginning of _rPath
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
            OSL_ENSURE( !bIsNormalized, "Found duplicate path variable");

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

#ifdef TF_FILEURL
    if ( !bIsNormalized)
    {
        OUString sOther;

        // make a normalized path
        if ( oslOK(osl::File::getFileURLFromSystemPath(_rPath, sOther)))
        {
            _rPath = sOther;

        }
        // check if it already was normalized
        else if ( !oslOK(osl::File::getSystemPathFromFileURL(_rPath, sOther)) )
            return false;
    }

#else // ! TF_FILEURL
    OUString sNormalized;
    if ( oslOK(osl_normalizePath(_rPath.pData, &sNormalized.pData)) )
    {
        _rPath = sNormalized;
    }
    else
        return false;

#endif // TF_FILEURL

    #ifdef _DEBUG
    OUString sSystemPathCheck; // check that this has become a file url for a local path
    #endif // _DEBUG
    OSL_POSTCOND( oslOK( osl::File::getSystemPathFromFileURL(_rPath, sSystemPathCheck) ),
                  "Unexpected: System path variable substitution did not result in valid file URL");

    return true;
}


// ----------------------------------------------------------------------------------
sal_Unicode const cURLSeparator = '/';

// ----------------------------------------------------------------------------------
static inline void moveUpOneDirectory(OUString& _rsPath, bool leaveSlash = false)
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

static bool locateBootstrapProfile(OUString& _rOfficeInstallPath, OUString& _rProfileFile)
{
    // get the bootstrap.ini file, where we find basic informations about the version of the product
    // we're running in
    // the bootstrap.ini resides in the same dir as our executable
    OUString sBootstrap;
    osl_getExecutableFile(&sBootstrap.pData);

    // take care for the office install path
    _rOfficeInstallPath = sBootstrap;
    // for this, cut two levels from the executable (the executable name and the program directory)
    moveUpOneDirectory(_rOfficeInstallPath);
    moveUpOneDirectory(_rOfficeInstallPath);

    // cut the executable file name and append the bootstrap ini file name
    moveUpOneDirectory(sBootstrap,true);
    sBootstrap += OUString::createFromAscii(BOOTSTRAP_FILE);

    _rProfileFile = sBootstrap;

    osl::DirectoryItem aCheckPath;
    return oslOK(aCheckPath.get(sBootstrap, aCheckPath));
}

static inline bool isValidFileURL(OUString const& _sFileURL)
{
    OUString sSystemPath;
    return _sFileURL.getLength() && oslOK(osl::File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
}

// ----------------------------------------------------------------------------------
// locate the user path

BootstrapRetVal bootstrap_locateUserInstallationPath(OUString& _rOfficeInstallPath,OUString& _rUserInstallPath, OUString& _sIniPath)
{
    rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

    BootstrapRetVal nStatus = BOOTSTRAP_INI_NOT_FOUND;

    try
    {
        if ( !locateBootstrapProfile(_rOfficeInstallPath,_sIniPath) )
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
        if (!oslOK(aCheckPath.get(sProductVersionFile, aCheckPath)))
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
#ifdef TF_FILEURL
            if (!oslOK(osl::File::getFileURLFromSystemPath(sUserPath, sNormalized)))
#else
            if (!oslOK(osl_normalizePath(sUserPath.pData, &sNormalized.pData)))
#endif
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

// locateBootstrapFiles
// ---------------------------------------------------------------------------------------

static bool locateConfigProfile(OUString const& _sBasePath, OUString& _rProfileFile)
{
    if ( !isValidFileURL(_sBasePath) ) // do we have a location ?
        return false;

    // the name of our very personal ini file
    const OUString sIniName(RTL_CONSTASCII_USTRINGPARAM(CONFIGURATION_PROFILE_NAME));

     // the composed name of the user dir and the ini file name
    OUString sProfileFile = _sBasePath;

    sProfileFile += OUString(RTL_CONSTASCII_USTRINGPARAM("/user/"));
    sProfileFile += sIniName;

    // does our ini file exist in the user directory ?
    ::osl::DirectoryItem aItem;
    bool bExists = oslOK( aItem.get(sProfileFile, aItem) );

    if (bExists) _rProfileFile = sProfileFile;

    return bExists;
}

// ----------------------------------------------------------------------------------
BootstrapRetVal locateBootstrapFiles(OUString& _rOfficeInstall, OUString& _rUserInstallPath, OUString& _rProfileFile)
{
    // get the office/user install directories (the latter may be empty resp. unknown)
    BootstrapRetVal nLocateError = bootstrap_locateUserInstallationPath(_rOfficeInstall, _rUserInstallPath, _rProfileFile);

    switch (nLocateError)
    {
        case BOOTSTRAP_OK: // check if we have a sregistry.ini
            OSL_ASSERT( isValidFileURL(_rUserInstallPath) );
            OSL_ASSERT( _rProfileFile.getLength() == 0 );

            if ( !locateConfigProfile(_rUserInstallPath,_rProfileFile) )
            {
                nLocateError = SREGISTRY_INI_NOT_FOUND;
            }
            break;

        case BOOTSTRAP_INI_NOT_FOUND: // support installations without bootstrap.ini and sversion.ini
            OSL_ASSERT( _rUserInstallPath.getLength() == 0 );
            {
                OUString sProfile;
                if ( locateConfigProfile(_rOfficeInstall,sProfile) )
                {
                    nLocateError = BOOTSTRAP_OK;
                    _rUserInstallPath = _rOfficeInstall;
                    _rProfileFile = sProfile;
                }
            }
            break;

        default:
            OSL_ASSERT( _rUserInstallPath.getLength() == 0 );
            break;
    }
    return nLocateError;
}


// ----------------------------------------------------------------------------------
// retrieve product key and logo information

BootstrapRetVal bootstrap_getProductKeyAndLogo( OUString& _rProductKey, OUString& _rLogo, OUString& _sIniPath )
{
    OUString aOfficeInstallPath;

    rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

    BootstrapRetVal nStatus = BOOTSTRAP_INI_NOT_FOUND;

    try
    {
        if ( !locateBootstrapProfile( aOfficeInstallPath, _sIniPath) )
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
        if (!oslOK(aCheckPath.get(sProductVersionFile, aCheckPath)))
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

} // namespace utl

