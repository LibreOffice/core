/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-30 10:12:48 $
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
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif

#ifndef _OSL_PROFILE_HXX_
#include <osl/profile.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_MISSINGBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INSTALLATIONINCOMPLETEEXCEPTION_HPP_
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef CONFIGMGR_SESSIONFACTORY_HXX_
#include "sessionfactory.hxx"
#endif
#include "matchlocale.hxx"

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

#define BOOTSTRAP_FILE                  INI_FILE_NAME( bootstrap )
#define CONFIGURATION_PROFILE_NAME      INI_FILE_NAME( sregistry )

#define BOOSTRAP_SECTION            "Bootstrap"
#define BOOSTRAP_ITEM_PRODUCT_KEY   "ProductKey"
#define BOOSTRAP_ITEM_LOCATION      "Location"
#define BOOSTRAP_ITEM_SECTION       "Section"

#define BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT     "StarOffice 6.0"
#define BOOTSTRAP_VALUE_SECTION_DEFAULT         "Versions"

#define SREGISTRY_SECTION_CONFIGURATION     "configuration"
#define SREGISTRY_KEY_SERVERTYPE            "servertype"
#define SREGISTRY_KEY_LOCALE                "DefaultLocale"
#define SREGISTRY_KEY_ASYNC                 "EnableAsyncUpdates"
#define SREGISTRY_SECTION_REMOTE            "RemoteRegistry"
#define SREGISTRY_KEY_SERVER                "Server"
#define SREGISTRY_KEY_TIMEOUT               "Timeout"
#define SREGISTRY_SECTION_LOCAL             "LocalRegistry"
#define SREGISTRY_KEY_SOURCEPATH            "sourcepath"
#define SREGISTRY_KEY_UPDATEPATH            "updatepath"
#define SREGISTRY_SECTION_AUTHENTICATION    "Authentication"
#define SREGISTRY_KEY_USER                  "User"
#define SREGISTRY_KEY_PASSWORD              "Password"

// general settings
#define SETTING_SERVERTYPE                  "servertype"
// portal settings
#define SETTING_SERVICE                     "service"
// remote settings
#define SETTING_SERVER                      "server"
#define SETTING_PORT                        "port"
#define SETTING_TIMEOUT                     "timeout"
// local settings
#define SETTING_SOURCEPATH                  "sourcepath"
#define SETTING_UPDATEPATH                  "updatepath"
#define SETTING_REINITIALIZE                "reinitialize"
// 'option' settings
#define SETTING_USER                        "user"
#define SETTING_PASSWORD                    "password"
#define SETTING_LOCALE                      "locale"
#define SETTING_ASYNC                       "lazywrite"
// deprecated and obsolete
#define SETTING_ROOTPATH                    "rootpath"

namespace configmgr
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    namespace uno = ::com::sun::star::uno;

// ---------------------------------------------------------------------------------------
    static const sal_Char* pKnownSettings[] =
    {
        SETTING_SERVERTYPE,
        SETTING_USER, SETTING_PASSWORD,
        SETTING_SERVER, SETTING_SERVICE, SETTING_PORT,
        SETTING_TIMEOUT,
        SETTING_LOCALE, SETTING_ASYNC,
        SETTING_SOURCEPATH, SETTING_UPDATEPATH, SETTING_REINITIALIZE
    };

// ---------------------------------------------------------------------------------------
    typedef char const * AsciiString;

    struct SessionClass
    {
        AsciiString name;
        SessionFactoryFunc create;
    };

// ---------------------------------------------------------------------------------------
    SessionClass const aSessionClasses[] =
    {
        { REMOTE_SESSION_IDENTIFIER,        createRemoteSession },
        { LOCAL_SESSION_IDENTIFIER,         createLocalSession  },
        { PORTAL_SESSION_IDENTIFIER,        createPortalSession },
        { SETUP_SESSION_IDENTIFIER,         createSetupSession },
//      { PLUGIN_SESSION_IDENTIFIER,        createPluginSession }
    };
    int const nSessionClasses = sizeof(aSessionClasses)/sizeof(aSessionClasses[0]);

// ---------------------------------------------------------------------------------------
    namespace
    {
    // ---------------------------------------------------------------------------------------
        enum RetVal
        {
            BOOTSTRAP_OK = 0,
            BOOTSTRAP_INI_NOT_FOUND,
            BOOTSTRAP_INI_INVALID,
            SVERSION_INI_NOT_FOUND,
            SVERSION_INI_NO_ENTRY,
            SVERSION_INI_INVALID,
            SREGISTRY_INI_NOT_FOUND,
            SREGISTRY_INI_INVALID
        };

    // ---------------------------------------------------------------------------------------

        RetVal locateBootstrapFiles(OUString& _rOfficeInstall, OUString& _rUserInstallPath, OUString& _rProfilePath);

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
    }

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
    void raiseBootstrapException( BootstrapResult rc, OUString const& sURL, Reference< XInterface > xContext )
    {
        using namespace ::com::sun::star::configuration;

        sal_Char const sBaseMessage[] = "Cannot locate Configuration: ";

        OUString sMessage = OUString::createFromAscii(sBaseMessage);
        sMessage += getBootstrapErrorMessage( rc );

        switch (rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            throw MissingBootstrapFileException(sMessage, xContext, sURL);

        case INVALID_BOOTSTRAP_DATA:
            throw InvalidBootstrapFileException(sMessage, xContext, sURL);

        case INVALID_INSTALLATION:
            throw InstallationIncompleteException(sMessage, xContext);

        default: OSL_ENSURE(false, "Undefined BootstrapResult code");
        case BOOTSTRAP_FAILURE:
            throw CannotLoadConfigurationException(sMessage, xContext);

        case BOOTSTRAP_DATA_OK:     break;
        }
    }

// ---------------------------------------------------------------------------------------
    void raiseBootstrapException( BootstrapSettings const& rBootstrapData, Reference< XInterface > xContext )
    {
        raiseBootstrapException(rBootstrapData.status,rBootstrapData.url,xContext);
    }

// ---------------------------------------------------------------------------------------
    OUString Settings::Setting::toString() const
    {
        OUString sReturn;

        sal_Bool bSuccess = this->m_aValue >>= sReturn;

        OSL_ENSURE(bSuccess, "Settings::Setting::toString: setting is not a string!");

        return sReturn;
    }

// ---------------------------------------------------------------------------------------
    sal_Int32 Settings::Setting::toInt32() const
    {
        sal_Int32 nReturn = 0;

        sal_Bool bSuccess = this->m_aValue >>= nReturn;
        if (!bSuccess)
        {
            OUString sValue;
            if (this->m_aValue >>= sValue)
            {
                nReturn = sValue.toInt32();
                bSuccess = (nReturn != 0);
            }
        }
        OSL_ENSURE(bSuccess, "Settings::getIntSetting: setting is not an integer!");
        return nReturn;
    }

// ---------------------------------------------------------------------------------------
    sal_Bool Settings::Setting::toBool() const
    {
        sal_Bool bReturn = false;

        switch (this->m_aValue.getValueTypeClass())
        {
        case uno::TypeClass_BOOLEAN:
            bReturn = *static_cast<sal_Bool const*>(this->m_aValue.getValue());
            break;

        case uno::TypeClass_LONG:
            bReturn = (0 != *static_cast<sal_Int32 const*>(this->m_aValue.getValue()));
            break;

        case uno::TypeClass_STRING:
            {

                OUString sValue; OSL_VERIFY(this->m_aValue >>= sValue);
                if (sValue.getLength() == 0)
                {
                    bReturn = false;
                }
                else if (sValue.equalsIgnoreAsciiCase(OUString::createFromAscii("true")) ||
                         sValue.equalsIgnoreAsciiCase(OUString::createFromAscii("yes")) )
                {
                    bReturn = true;
                }
                else if (sValue.equalsIgnoreAsciiCase(OUString::createFromAscii("false")) ||
                         sValue.equalsIgnoreAsciiCase(OUString::createFromAscii("no")) )
                {
                    bReturn = false;
                }
                else
                    OSL_ENSURE(false, "Settings::Setting::toBool: string setting is no known bool value name!");

            } break;

        default:
            OSL_ENSURE(false, "Settings::Setting::toBool: setting is not a boolean!");
            break;
        }

        return bReturn;
    }

// ---------------------------------------------------------------------------------------
    Settings::Settings()
    {
    }

// ---------------------------------------------------------------------------------------
    Settings::Settings(const Sequence< Any >& _rOverrides, Origin _eOrigin)
    {
        override(_rOverrides,_eOrigin);
    }

// ---------------------------------------------------------------------------------------
    void Settings::mergeOverrides(const Settings& _rOverrides)
    {
        for (Iterator it = _rOverrides.begin(); it != _rOverrides.end(); ++it)
        {
            m_aImpl[it->first] = it->second;
        }
    }

// ---------------------------------------------------------------------------------------
    void Settings::override(const Sequence< Any >& _rOverrides, Origin _eOrigin)
    {
        // transfer the runtime overrides
        const sal_Int32 nCount = _rOverrides.getLength();
        OSL_ENSURE(0 <= nCount && nCount <= 0x7FFF, "Unexpected number of arguments");

        PropertyValue aCurrentArg;
        for (sal_Int32 nArg = 0; nArg < nCount; ++nArg)
        {
            // it must be a PropertyValue (with ascii name)
            if (_rOverrides[nArg] >>= aCurrentArg)
            {
                OString aAsciiName = rtl::OUStringToOString(aCurrentArg.Name, RTL_TEXTENCODING_ASCII_US );
                putSetting(aAsciiName, Setting(aCurrentArg.Value, _eOrigin));

                CFG_TRACE_INFO("provider bootstrapping: runtime parameter: %s", aAsciiName.getStr());
            }
            else
            {
                CFG_TRACE_ERROR("provider bootstrapping: illegal parameter of type %s", OUSTRING2ASCII(_rOverrides[nArg].getValueType().getTypeName()));
                throw IllegalArgumentException(OUString::createFromAscii("Configuration: Provider Creation Argument is not a com.sun.star.beans.PropertyValue."), NULL, sal_Int16(nArg));
            }
        }
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
    void Settings::putSetting(Name const& _pName, const Setting& _rSetting)
    {
        // catch invalid settings
        if ( ! isEmptySettingValue(_rSetting.value()) )
        {
            m_aImpl[_pName] = _rSetting;
        }

        else
        {
            CFG_TRACE_WARNING("bootstrap: Putting empty setting for '%s'. Will be cleared instead");
            this->clearSetting(_pName);
        }
    }

// ---------------------------------------------------------------------------------------
    void Settings::clearSetting(Name const& _pName)
    {
        m_aImpl.erase(_pName);
    }

// ---------------------------------------------------------------------------------------
    sal_Bool Settings::haveSetting(Name const& _pName) const
    {
        Iterator aPos = m_aImpl.find(_pName);

        // have a setting at all ?
        return (aPos != m_aImpl.end());
    }

// ---------------------------------------------------------------------------------------
    Settings::Origin Settings::getOrigin(Name const& _pName) const
    {
        Iterator aPos = m_aImpl.find(_pName);

        // have a setting at all ?
        return (aPos != m_aImpl.end()) ? aPos->second.origin() : SO_NOT_SET;
    }

// ---------------------------------------------------------------------------------------
    Settings::Setting Settings::getMaybeSetting(Name const& _pName) const
    {
        Iterator aPos = m_aImpl.find(_pName);

        if (aPos != m_aImpl.end())
            return aPos->second;

        else
            return Setting();
    }

// ---------------------------------------------------------------------------------------
    Settings::Setting Settings::getSetting(Name const& _pName) const
    {
        OSL_ENSURE(haveSetting(_pName), "Settings::getSetting: don't have the requested setting!");
        return getMaybeSetting(_pName);
    }

// ---------------------------------------------------------------------------------------
    OUString Settings::getStringSetting(Name const& _pName) const
    {
        Setting aSetting = this->getSetting(_pName);

        return aSetting.toString();
    }

// ---------------------------------------------------------------------------------------
    sal_Int32 Settings::getIntSetting(Name const& _pName) const
    {
        Setting aSetting = this->getSetting(_pName);

        return aSetting.toInt32();
    }

// ---------------------------------------------------------------------------------------
    sal_Bool Settings::getBoolSetting(Name const& _pName) const
    {
        Setting aSetting = this->getSetting(_pName);

        return aSetting.toBool();
    }

// ---------------------------------------------------------------------------------------
    ConnectionSettings::ConnectionSettings(const uno::Sequence< uno::Any >& _rOverrides,
                                            Settings::Origin _eOrigin)
    : m_aSettings(_rOverrides, _eOrigin)
    {

        // translate old compatibility settings
        implTranslateCompatibilitySettings();

        implNormalizePathSetting(SETTING_SOURCEPATH);
        implNormalizePathSetting(SETTING_UPDATEPATH);
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setSessionType(const OUString& _rSessionIdentifier, Settings::Origin _eOrigin)
    {
        putSetting(SETTING_SERVERTYPE, Settings::Setting(_rSessionIdentifier, _eOrigin));
        OSL_ENSURE(haveSetting(SETTING_SERVERTYPE), "Settings::setSessionType: could not set session type!");
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setService(const OUString& _rService, Settings::Origin _eOrigin)
    {
        putSetting(SETTING_SERVICE, Settings::Setting(_rService, _eOrigin));
        OSL_ENSURE(haveSetting(SETTING_SERVICE), "Settings::setService: could not set service type!");
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isSessionTypeKnown() const { return haveSetting(SETTING_SERVERTYPE); }
    sal_Bool ConnectionSettings::hasUser() const            { return haveSetting(SETTING_USER); }
    sal_Bool ConnectionSettings::hasPassword() const        { return haveSetting(SETTING_PASSWORD); }
    sal_Bool ConnectionSettings::hasLocale() const          { return haveSetting(SETTING_LOCALE); }
    sal_Bool ConnectionSettings::hasServer() const          { return haveSetting(SETTING_SERVER); }
    sal_Bool ConnectionSettings::hasService() const         { return haveSetting(SETTING_SERVICE); }
    sal_Bool ConnectionSettings::hasPort() const            { return haveSetting(SETTING_PORT); }
    sal_Bool ConnectionSettings::hasTimeout() const         { return haveSetting(SETTING_TIMEOUT); }
    sal_Bool ConnectionSettings::hasAsyncSetting() const    { return haveSetting(SETTING_ASYNC); }
    sal_Bool ConnectionSettings::hasReinitializeFlag() const { return haveSetting(SETTING_REINITIALIZE); }

// ---------------------------------------------------------------------------------------
    OUString    ConnectionSettings::getSessionType() const  { return m_aSettings.getStringSetting(SETTING_SERVERTYPE); }
    OUString    ConnectionSettings::getUser() const         { return m_aSettings.getStringSetting(SETTING_USER); }
    OUString    ConnectionSettings::getPassword() const     { return m_aSettings.getStringSetting(SETTING_PASSWORD); }
    OUString    ConnectionSettings::getLocale() const       { return m_aSettings.getStringSetting(SETTING_LOCALE); }
    OUString    ConnectionSettings::getSourcePath() const   { return m_aSettings.getStringSetting(SETTING_SOURCEPATH); }
    OUString    ConnectionSettings::getUpdatePath() const   { return m_aSettings.getStringSetting(SETTING_UPDATEPATH); }
    OUString    ConnectionSettings::getServer() const       { return m_aSettings.getStringSetting(SETTING_SERVER); }
    OUString    ConnectionSettings::getService() const      { return m_aSettings.getStringSetting(SETTING_SERVICE); }
    sal_Int32   ConnectionSettings::getPort() const         { return m_aSettings.getIntSetting(SETTING_PORT); }
    sal_Int32   ConnectionSettings::getTimeout() const      { return m_aSettings.getIntSetting(SETTING_TIMEOUT); }
    sal_Bool ConnectionSettings::getAsyncSetting() const    { return m_aSettings.getBoolSetting(SETTING_ASYNC); }
    sal_Bool ConnectionSettings::getReinitializeFlag() const { return m_aSettings.getBoolSetting(SETTING_REINITIALIZE); }

// ---------------------------------------------------------------------------------------
    static inline bool isValidFileURL(OUString const& _sFileURL)
    {
        OUString sSystemPath;
        return _sFileURL.getLength() && oslOK(osl::File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
   }
// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isValidPathSetting(Settings::Name const& _sSetting) const
    {
        return haveSetting(_sSetting) &&
                isValidFileURL(m_aSettings.getStringSetting(_sSetting));
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isSourcePathValid() const
    {
        return isValidPathSetting(SETTING_SOURCEPATH);
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isUpdatePathValid() const
    {
        return isValidPathSetting(SETTING_UPDATEPATH);
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::implPutSystemPathSetting(Settings::Name const& _pSetting, OUString const& _sSystemPath, Settings::Origin _eOrigin)
    {
        OUString sFileURL;

        bool bOK = _sSystemPath.getLength() &&  oslOK(osl::File::getFileURLFromSystemPath(_sSystemPath, sFileURL));

        if (!bOK)
        {
            bOK = isValidFileURL(_sSystemPath);
            if (bOK)
                sFileURL = _sSystemPath;
        }
        if (bOK)
        {
            putSetting(_pSetting, Settings::Setting(sFileURL, _eOrigin));
        }
        else
        {
            if (_sSystemPath.getLength() != 0)
                CFG_TRACE_ERROR_NI("provider bootstrapping: could not normalize a given path (setting: %s, value: %s)", _pSetting.getStr(), OUSTRING2ASCII(_sSystemPath));
            clearSetting(_pSetting);
        }

        return bOK;
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::implNormalizePathSetting(Settings::Name const& _pSetting)
    {
        sal_Bool bOK = haveSetting(_pSetting);
        if (bOK)
        {
            Settings::Setting aSetting = getSetting(_pSetting);

            OUString sRawPath = aSetting.toString();
            OUString sNormalized;

            bOK = oslOK(osl::File::getFileURLFromSystemPath(sRawPath, sNormalized));
            if (bOK)
            {
                putSetting(_pSetting, Settings::Setting(sNormalized,aSetting.origin()) );
            }
            else if (!isValidFileURL(sRawPath))
            {
                CFG_TRACE_WARNING_NI("provider bootstrapping: could not normalize a given path (setting: %s, value: %s)", _pSetting.getStr(), OUSTRING2ASCII(sRawPath));
                //clearSetting(_pSetting);
            }
        }
        return bOK;
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::loadFromInifile( osl::Profile & rProfile, Settings::Origin _eOrigin )
    {
        implCollectSRegistrySetting(rProfile,_eOrigin);

        // translate old compatibility settings
        implTranslateCompatibilitySettings();
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::fillFromInifile( osl::Profile & rProfile, Settings::Origin _eOrigin )
    {
        // use existing settings as overrides
        Settings aOldValues;
        m_aSettings.swap(aOldValues);

        loadFromInifile(rProfile,_eOrigin);

        mergeOverrides(aOldValues);
    }

// ---------------------------------------------------------------------------------------
    BootstrapResult ConnectionSettings::findInifile(OUString& _rsInifile)
    {
        OUString sOfficeInstall, sUserInstall;

        RetVal rv = locateBootstrapFiles( sOfficeInstall, sUserInstall, _rsInifile );

        if (rv == BOOTSTRAP_OK)
        {
            CFG_TRACE_INFO("provider bootstrapping: located an ini file: %s", OUSTRING2ASCII(_rsInifile));
        }

        else
        {
            CFG_TRACE_WARNING("provider bootstrapping: could not locate ini file. File %s was missing or invalid.", OUSTRING2ASCII(_rsInifile));
        }

        return mapBootstrapResult[rv];
    }

// ---------------------------------------------------------------------------------------
    ConnectionSettings ConnectionSettings::bootstrap(BootstrapResult& rc, OUString& _rsInifile)
    {
        // try to locate the sregistry.ini
        OUString sOfficeInstall, sUserInstall, sIniFile;

        CFG_TRACE_INFO("provider bootstrapping: trying to locate the installation and the ini file");
        RetVal rv = locateBootstrapFiles( sOfficeInstall, sUserInstall, _rsInifile );

        ConnectionSettings aBootstrapSettings;

        if (BOOTSTRAP_OK == rv)
        {
            CFG_TRACE_INFO_NI("provider bootstrapping: located an ini file: %s", OUSTRING2ASCII( _rsInifile));
            try
            {
                osl::Profile aSRegistryIni( _rsInifile );

                aBootstrapSettings.loadFromInifile( aSRegistryIni );
            }
            catch (std::exception& e)
            {
                CFG_TRACE_ERROR_NI("provider bootstrapping: Opening %s failed unexpectedly due to ERROR: \"%s\"", OUSTRING2ASCII(_rsInifile), e.what());
                rv = SREGISTRY_INI_NOT_FOUND;
            }

            // try to locate the local stuff anyways
            aBootstrapSettings.implAdjustToInstallation(sOfficeInstall,sUserInstall);
        }
        else
        {
            CFG_TRACE_INFO_NI("provider bootstrapping: could not locate ini file - using fallback bootstrap [File %s was missing or invalid].", OUSTRING2ASCII( _rsInifile));

            if (rv >= SREGISTRY_INI_NOT_FOUND) // but found all up to user install
            {
                CFG_TRACE_INFO_NI("provider bootstrapping: found an office installation");
                // .. so try to locate the local stuff anyways
                aBootstrapSettings.implAdjustToInstallation(sOfficeInstall,sUserInstall);
            }
        }

        if (BOOTSTRAP_OK == rv)
        {
            if (!aBootstrapSettings.isComplete())
            {
                CFG_TRACE_WARNING_NI("provider bootstrapping: data from ini file is incomplete or corrupt");
                rv = SREGISTRY_INI_INVALID;
            }
        }

        rc = mapBootstrapResult[rv];

        return aBootstrapSettings;
    }

// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::checkSettings() const
    {
        bool bCheckResult = true;

    #if defined(_DEBUG) || defined(CFG_ENABLE_TRACING)
        const int cKnownSettingsCount = sizeof(pKnownSettings)/sizeof(pKnownSettings[0]);

        // check if we know all the settings
        for (   Settings::Iterator aCheck = m_aSettings.begin();
                aCheck != m_aSettings.end();
                ++aCheck
            )
        {
            bool bRecognized = false;
            for (sal_Int32 i=0; i<cKnownSettingsCount; ++i)
            {
                if (aCheck->first.equals(pKnownSettings[i]))
                {
                    bRecognized = true;
                    break;
                }
            }

            if (!bRecognized)
            {
                OSL_ENSURE(bRecognized, OString("ConnectionSettings: unrecognized parameter: ").concat(aCheck->first).getStr() );
                CFG_TRACE_WARNING_NI("provider bootstrapping: unrecognized parameter found: %s", aCheck->first.getStr() );
                bCheckResult = false;
            }
        }
    #endif

        return bCheckResult;
    }

// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::validate()
    {
        checkSettings();

        // determine the session type
        implDetermineSessionType();

        // remove unneccessary items
        implClearIrrelevantItems();

        return isComplete( getSessionType() );
    }

// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::isComplete() const
    {
        return isSessionTypeKnown() && ! isPlugin()  && isComplete(getSessionType());
    }

// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::isComplete(OUString const& sSessionType) const
    {
        if( (0 == sSessionType.compareToAscii(LOCAL_SESSION_IDENTIFIER)) ||
            (0 == sSessionType.compareToAscii(SETUP_SESSION_IDENTIFIER)) )
        {
            // local sessions needs a source path
            return !! isSourcePathValid();
        }

        else if(0 == sSessionType.compareToAscii(PORTAL_SESSION_IDENTIFIER) )
        {
            return true;
        }

        else if(0 == sSessionType.compareToAscii(REMOTE_SESSION_IDENTIFIER) )
        {
            // remote sessions needs a server (and a user ?)
            return hasServer() && hasUser();
        }
        else
        {
            OSL_ENSURE(0 == sSessionType.compareToAscii(PLUGIN_SESSION_IDENTIFIER),
                        "Unknown session type, cannot determine validity of settings");
            return false; // assume false
        }

    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::mergeOverrides(const Settings& _rOverrides)
    {
        // update path validity depends on data in its base source path
        if (_rOverrides.haveSetting(SETTING_SOURCEPATH))
        {
            this->clearSetting(SETTING_SOURCEPATH);
            this->clearSetting(SETTING_UPDATEPATH);
        }

        // if changing the user, password must be changed as well
        if (_rOverrides.haveSetting(SETTING_USER))
        {
            this->clearSetting(SETTING_USER);
            this->clearSetting(SETTING_PASSWORD);
        }

        // do it
        m_aSettings.mergeOverrides(_rOverrides);

        // translate old compatibility settings
        implTranslateCompatibilitySettings();
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::ensureConfigPath(Settings::Name const& _pSetting, const OUString& _rBasePath, const sal_Char* _pRelative)
    {
        // ensure that the existing setting value is a valid path
        if (isValidPathSetting(_pSetting))
            // nothing to do
            return true;

        if (_rBasePath.getLength() == 0)
        {
            OSL_ENSURE(false,OString("No base path found for setting: ").concat( _pSetting ). getStr());
            return false;
        }

        // append the name given, plus config/registry
        OUString sDirectory(_rBasePath);

        if (_pRelative != NULL)
            sDirectory  += OUString::createFromAscii("/").concat( OUString::createFromAscii(_pRelative) );

        sDirectory += OUString::createFromAscii("/config/registry");

        putSetting(_pSetting, Settings::Setting(sDirectory, Settings::SO_FALLBACK));

        CFG_TRACE_INFO("provider bootstrapping: set fallback path for %s to %s", _pSetting.getStr(), OUSTRING2ASCII(sDirectory));
        return isValidPathSetting(_pSetting);
    }

// ---------------------------------------------------------------------------------------
    // if we do not already have path settings, ensure that they exists (in an office install)
    void ConnectionSettings::implAdjustToInstallation(const OUString& _sOfficeInstallPath, const OUString& _sUserInstallPath)
    {
        // if we do not already have a share path setting, create one from the user install path
        if (ensureConfigPath(SETTING_SOURCEPATH, _sOfficeInstallPath, "share"))
        {
            // set update path only if source path could be set

            // if we do not already have a update path setting, create one from the user install path
            ensureConfigPath(SETTING_UPDATEPATH, _sUserInstallPath, "user");
        }
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implCollectSRegistrySetting(osl::Profile & rProfile, Settings::Origin eOrigin)
    {
        rtl_TextEncoding const nExtEncoding = osl_getThreadTextEncoding();
        rtl_TextEncoding const nIntEncoding = RTL_TEXTENCODING_ASCII_US;

        typedef Settings::Setting Setting;

        // session type
        OUString sSessionType = getProfileStringItem(rProfile, SREGISTRY_SECTION_CONFIGURATION, SREGISTRY_KEY_SERVERTYPE, nIntEncoding);
        if (sSessionType.getLength())
        {
            CFG_TRACE_INFO("provider bootstrapping: session type as found in the sregistry: %s", OUSTRING2ASCII(sSessionType));
            putSetting(SETTING_SERVERTYPE, Setting(sSessionType, eOrigin));
        }
        else
        {
            CFG_TRACE_WARNING("provider bootstrapping: no session type found in the sregistry");
        }

        // common provider settings
        OUString sLocale = getProfileStringItem(rProfile, SREGISTRY_SECTION_CONFIGURATION, SREGISTRY_KEY_LOCALE, nIntEncoding);
        OUString sAsync  = getProfileStringItem(rProfile, SREGISTRY_SECTION_CONFIGURATION, SREGISTRY_KEY_ASYNC, nIntEncoding);

        if (sLocale.getLength())    putSetting(SETTING_LOCALE, Setting(sLocale, eOrigin));
        if (sAsync.getLength())     putSetting(SETTING_ASYNC, Setting(sAsync, eOrigin));

        // authentication data
        OUString sUser      = getProfileStringItem(rProfile, SREGISTRY_SECTION_AUTHENTICATION, SREGISTRY_KEY_USER, nExtEncoding);
        OUString sPassword  = getProfileStringItem(rProfile, SREGISTRY_SECTION_AUTHENTICATION, SREGISTRY_KEY_PASSWORD, nExtEncoding);

        if (sUser.getLength())      putSetting(SETTING_USER,        Setting(sUser, eOrigin));
        if (sPassword.getLength())  putSetting(SETTING_PASSWORD,    Setting(sPassword, eOrigin));

        // local session settings
        OUString sSourcePath    = getProfileStringItem(rProfile, SREGISTRY_SECTION_LOCAL, SREGISTRY_KEY_SOURCEPATH, nExtEncoding);
        OUString sUpdatePath    = getProfileStringItem(rProfile, SREGISTRY_SECTION_LOCAL, SREGISTRY_KEY_UPDATEPATH, nExtEncoding);

        implPutSystemPathSetting(SETTING_SOURCEPATH, sSourcePath, eOrigin);
        implPutSystemPathSetting(SETTING_UPDATEPATH, sUpdatePath, eOrigin);

        // remote session settings
        OUString sServerAndPort = getProfileStringItem(rProfile, SREGISTRY_SECTION_REMOTE, SREGISTRY_KEY_SERVER, nExtEncoding);
        sal_Int32 nTimeout      = getProfileIntItem(rProfile, SREGISTRY_SECTION_REMOTE, SREGISTRY_KEY_TIMEOUT);

        if (sServerAndPort.getLength())
        {   // there was such an entry
            const int nColon = sServerAndPort.lastIndexOf(':');
            if (0 <= nColon)
            {
                putSetting(SETTING_SERVER,  Setting(sServerAndPort.copy(0, nColon), eOrigin));
                putSetting(SETTING_PORT,    Setting(sServerAndPort.copy(nColon+1).toInt32(), eOrigin));
            }
            else
                putSetting(SETTING_SERVER, Setting(sServerAndPort, eOrigin));
        }

        if (nTimeout) putSetting(SETTING_TIMEOUT, Setting(nTimeout, eOrigin));

    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implTranslateCompatibilitySettings()
    {
        // the former "rootpath" is the "updatepath" now
        if (isValidPathSetting(SETTING_ROOTPATH))
        {
            OSL_ENSURE(!isValidPathSetting(SETTING_UPDATEPATH), "Error: Settings have both 'updatepath' and 'rootpath'");
            OSL_ENSURE(false, "Error: Setting 'rootpath' is obsolete");
            // it survived the normalizing
            putSetting(SETTING_UPDATEPATH, getSetting(SETTING_ROOTPATH));
            clearSetting(SETTING_ROOTPATH);
        }
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implDetermineSessionType()
    {
        if (haveSetting(SETTING_SERVERTYPE))
        {   // we already have the setting
            CFG_TRACE_INFO("provider bootstrapping: using session type: %s",
                            OUSTRING2ASCII(m_aSettings.getStringSetting(SETTING_SERVERTYPE)));

        }
        else
        {
            CFG_TRACE_INFO("provider bootstrapping: no session type. trying portal session as fallback");

            OUString sSessionType = OUString::createFromAscii(PORTAL_SESSION_IDENTIFIER);
            putSetting(SETTING_SERVERTYPE, Settings::Setting(sSessionType, Settings::SO_FALLBACK));
        }
    }

// ---------------------------------------------------------------------------------------
    OUString ConnectionSettings::getProfileStringItem(osl::Profile & rProfile, OString const& _pSection, OString const& _pKey, rtl_TextEncoding _nEncoding)
    {
        OString sEntry = rProfile.readString( _pSection, _pKey, rtl::OString() );

        return rtl::OStringToOUString(sEntry, _nEncoding);
    }

// ---------------------------------------------------------------------------------------
    sal_Int32 ConnectionSettings::getProfileIntItem(osl::Profile & rProfile, OString const& _pSection, OString const& _pKey)
    {
        OString sValue = rProfile.readString( _pSection, _pKey, rtl::OString() );

        return sValue.getLength() ? sValue.toInt32() : 0;
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implClearIrrelevantItems()
    {
        if (m_aSettings.getOrigin(SETTING_SERVERTYPE) == Settings::SO_OVERRIDE)
        {   // the session type is a runtime override
            // clear the user if it's origin is SREGISTRY
            if (m_aSettings.getOrigin(SETTING_USER) == Settings::SO_INIFILE)
                clearSetting(SETTING_USER);
            // same for the password
            if (m_aSettings.getOrigin(SETTING_PASSWORD) == Settings::SO_INIFILE)
                clearSetting(SETTING_PASSWORD);
        }
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isPlugin() const
    {
        if (!isSessionTypeKnown()) return false;

        OUString const sSessionType = getSessionType();
        return  (0 == sSessionType.compareToAscii(PLUGIN_SESSION_IDENTIFIER));

    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isLocalSession() const
    {
        if (!isSessionTypeKnown()) return false;

        OUString const sSessionType = getSessionType();
        return  (0 == sSessionType.compareToAscii(LOCAL_SESSION_IDENTIFIER)) ||
                (0 == sSessionType.compareToAscii(SETUP_SESSION_IDENTIFIER));

    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isRemoteSession() const
    {
        if (!isSessionTypeKnown()) return false;

        OUString const sSessionType = getSessionType();
        return  (0 == sSessionType.compareToAscii(PORTAL_SESSION_IDENTIFIER)) ||
                (0 == sSessionType.compareToAscii(REMOTE_SESSION_IDENTIFIER));

    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isServiceRequired() const
    {
        OUString const sSessionType = getSessionType();
        return  (0 == sSessionType.compareToAscii(PORTAL_SESSION_IDENTIFIER));

    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setUserSession()
    {
        OSL_ENSURE(isLocalSession() || isRemoteSession(),"Invalid/No session type for user session");
        OSL_ENSURE(getSessionType().compareToAscii(SETUP_SESSION_IDENTIFIER) != 0, "WARNING: Explicit creation of 'setup' sessions is obsolete. Create 'AdministrationProvider' service instead");

        if( !hasService() && isServiceRequired() )
        {
            char const c_sDefaultService[] = "configuration";

            CFG_TRACE_INFO("No service set for user session. Using default service '%s'",c_sDefaultService);

            OUString const sService( RTL_CONSTASCII_USTRINGPARAM(c_sDefaultService) );
            this->setService(sService, Settings::SO_DEFAULT);
        }

        if ( !hasAsyncSetting() )
        {
            Any aDefaultAsync = makeAny(sal_Bool(true));
            putSetting( SETTING_ASYNC, Settings::Setting(aDefaultAsync, Settings::SO_DEFAULT) );
            OSL_ASSERT( hasAsyncSetting() && getAsyncSetting() );
        }
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setAdminSession()
    {
        OSL_ENSURE(isLocalSession() || isRemoteSession(),"Invalid/No session type for admin session");
        if (isLocalSession())
        {
            OSL_ENSURE(!hasUser(), "Local Admin Session has 'user' parameter - ignoring (admin data will be used)");

            if (getSessionType().compareToAscii(LOCAL_SESSION_IDENTIFIER) == 0)
            {
                CFG_TRACE_INFO("Local Admin session: Changing session type to 'setup'");

                OUString const sSetup( RTL_CONSTASCII_USTRINGPARAM(SETUP_SESSION_IDENTIFIER) );
                this->setSessionType(sSetup,Settings::SO_DEFAULT);
            }
            else
                OSL_ENSURE(getSessionType().compareToAscii(SETUP_SESSION_IDENTIFIER) != 0, "WARNING: Explicit creation of 'setup' sessions is obsolete. ");
        }

        if( !hasService() && isServiceRequired() )
        {
            char const c_sDefaultService[] = "adminconfiguration";

            CFG_TRACE_INFO("No service set for admin session. Using default service '%s'",c_sDefaultService);

            OUString const sService( RTL_CONSTASCII_USTRINGPARAM(c_sDefaultService) );
            this->setService(sService, Settings::SO_DEFAULT);
        }

        if ( !hasAsyncSetting() )
        {
            Any aDefaultNoAsync = makeAny(sal_Bool(false));
            putSetting( SETTING_ASYNC, Settings::Setting(aDefaultNoAsync, Settings::SO_DEFAULT) );
            OSL_ASSERT( hasAsyncSetting() && !getAsyncSetting() );
        }

        if (!hasLocale())
        {
            CFG_TRACE_INFO("Settings for Admin session: Using 'all locales' by default");
            this->setAnyLocale(Settings::SO_DEFAULT);
        }
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setAnyLocale(Settings::Origin _eOrigin)
    {
        rtl::OUString sAnyLocale;
        localehelper::getAnyLocale( sAnyLocale );

        this->putSetting(SETTING_LOCALE, Settings::Setting(sAnyLocale, _eOrigin));
    }
// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setUserSession(const OUString& _rRemoteServiceName)
    {
        this->setService(_rRemoteServiceName, Settings::SO_MANUAL);
        this->setUserSession();
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setAdminSession(const OUString& _rRemoteServiceName)
    {
        this->setService(_rRemoteServiceName, Settings::SO_MANUAL);
        this->setAdminSession();
    }

// ---------------------------------------------------------------------------------------
    IConfigSession* ConnectionSettings::createConnection(Reference<XMultiServiceFactory> const& _rxServiceMgr) const
    {
        OUString sSessionType = getSessionType();
        OSL_ENSURE(!isPlugin(), "Settings::createConnection: can't create a plugin session!");

        for(int i= 0; i<nSessionClasses; ++i)
        {
            if (0 == sSessionType.compareToAscii(aSessionClasses[i].name))
            {
                if (IConfigSession* pSession = aSessionClasses[i].create(_rxServiceMgr, *this))
                {
                    OSL_ASSERT(pSession);
                    return pSession;
                }
            }
        }

        OSL_ENSURE(false, "unable to bootstrap the configuration - no match for session type!");
        return 0;
    }

// ---------------------------------------------------------------------------------------
    BootstrapSettings::BootstrapSettings()
    : url()
    , status(BOOTSTRAP_FAILURE)
    , settings(ConnectionSettings::bootstrap(status,url))
    {}

// ---------------------------------------------------------------------------------------
// caching helpers
//---------------------------------------------------------------------------------------
    OUString buildConnectString(const ConnectionSettings& _rSettings)
    {
        OSL_ENSURE(_rSettings.isComplete(),"WARNING: creating connect string for incomplete settings");

        rtl::OUStringBuffer sConnect = _rSettings.getSessionType();

        if (_rSettings.isServiceRequired() && _rSettings.hasService())
        {
            sConnect.append(sal_Unicode(':'));
            sConnect.append(_rSettings.getService());
        }

        if (_rSettings.isLocalSession())
        {
            if (_rSettings.isSourcePathValid())
            {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(":share@"));
                sConnect.append(_rSettings.getSourcePath());
            }
            if (_rSettings.isUpdatePathValid())
            {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(":user@"));
                sConnect.append(_rSettings.getUpdatePath());
            }
            // maybe consider user here as well ?
        }
        else if (_rSettings.isRemoteSession())
        {
            if (_rSettings.hasServer() || _rSettings.hasPort())
            {
                sConnect.append(sal_Unicode('@'));

                if ( _rSettings.hasServer())
                {
                    sConnect.append(_rSettings.getServer());
                }

                if ( _rSettings.hasPort())
                {
                    sConnect.append(sal_Unicode(':'));
                    sConnect.append(_rSettings.getPort());
                }
            }

            if (_rSettings.hasUser())
            {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";user="));
                sConnect.append(_rSettings.getUser());
            }
            if (_rSettings.hasPassword())
            {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";pwd="));
                sConnect.append(_rSettings.getPassword());
            }
            if (_rSettings.hasTimeout())
            {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";timeout="));
                sConnect.append( _rSettings.getTimeout());
            }
        }
        else
        {
            OSL_ENSURE(false, "Unknown session type");
        }

        if (_rSettings.hasLocale())
        {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";locale="));
                sConnect.append(_rSettings.getLocale());
        }
        if (_rSettings.hasAsyncSetting())
        {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";async="));
                sConnect.append(_rSettings.getAsyncSetting());
        }
        if (_rSettings.hasReinitializeFlag() && _rSettings.getReinitializeFlag())
        {
                sConnect.appendAscii(RTL_CONSTASCII_STRINGPARAM(";reinitialize="));
                sConnect.append(_rSettings.getReinitializeFlag());
        }

        return sConnect.makeStringAndClear();
    }
// ---------------------------------------------------------------------------------------
// - helper
// ---------------------------------------------------------------------------------------
namespace {

// normalizeAndSubstitutePathVariables
// ---------------------------------------------------------------------------------------
    typedef sal_Bool (SAL_CALL * getSystemDirectoryFunction)(oslSecurity, rtl_uString **);

    bool normalizeAndSubstitutePathVariables(OUString& _rPath)
    {
        OSL_PRECOND(_rPath.getLength() != 0 , "Invalid parameter: Empty path in normalizeAndSubstitutePathVariables" );

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
        CFG_TRACE_INFO("provider bootstrapping: calculated office install path: %s", OUSTRING2ASCII(_rOfficeInstallPath));

        // cut the executable file name and append the bootstrap ini file name
        moveUpOneDirectory(sBootstrap,true);
        sBootstrap += OUString::createFromAscii(BOOTSTRAP_FILE);

        CFG_TRACE_INFO_NI("provider bootstrapping: bootstrap file to find: %s", OUSTRING2ASCII(sBootstrap));
        _rProfileFile = sBootstrap;

        osl::DirectoryItem aCheckPath;
        return oslOK(aCheckPath.get(sBootstrap, aCheckPath));
    }

// ----------------------------------------------------------------------------------
// locate the user path

    RetVal locateUserInstallationPath(OUString& _rOfficeInstallPath,OUString& _rUserInstallPath, OUString& _sIniPath)
    {
        rtl_TextEncoding const nCvtEncoding = osl_getThreadTextEncoding();

        RetVal nStatus = BOOTSTRAP_INI_NOT_FOUND;

        try
        {
            if ( !locateBootstrapProfile(_rOfficeInstallPath,_sIniPath) )
            {
                CFG_TRACE_WARNING_NI("provider bootstrapping: bootstrap file not found");

                return BOOTSTRAP_INI_NOT_FOUND;
            }

            // get the path to the sversion.ini file from the bootstrap file
            ::osl::Profile aBootstrapFile(_sIniPath);

            // get the path to the sversion.ini
            OString sConvertable = aBootstrapFile.readString(
                    OString(BOOSTRAP_SECTION),
                    OString(BOOSTRAP_ITEM_LOCATION),
                    OString());

            OUString sProductVersionFile = rtl::OStringToOUString(sConvertable, nCvtEncoding);

            if (sProductVersionFile.getLength() == 0)
            {
                CFG_TRACE_ERROR_NI("provider bootstrapping: bootstrap file does not have expected entry");
                return BOOTSTRAP_INI_INVALID;
            }
            CFG_TRACE_INFO_NI("provider bootstrapping: product version file to find: %s", OUSTRING2ASCII(sProductVersionFile));

            nStatus = SVERSION_INI_NOT_FOUND;

            // in the path to the product, replace the reference to the user system directory, make it a file URL
            if ( ! normalizeAndSubstitutePathVariables(sProductVersionFile) )
            {
                CFG_TRACE_ERROR_NI("provider bootstrapping: could not normalize the product version file name from the bootstrap file");
                return BOOTSTRAP_INI_INVALID;
            }
            CFG_TRACE_INFO_NI("provider bootstrapping: product version file to find (after substituting and normalizing): %s", OUSTRING2ASCII(sProductVersionFile));

            _sIniPath = sProductVersionFile;

            osl::DirectoryItem aCheckPath;
            if (!oslOK(aCheckPath.get(sProductVersionFile, aCheckPath)))
            {
                CFG_TRACE_ERROR_NI("provider bootstrapping: product version file not found");

                return SVERSION_INI_NOT_FOUND;
            }

            // open the product version profile
            ::osl::Profile aProductVersionFile(sProductVersionFile);

            // get the section/key name of the entry in the product version file
            OString sVersionFileSection = aBootstrapFile.readString(
                    OString(BOOSTRAP_SECTION),
                    OString(BOOSTRAP_ITEM_SECTION),
                    OString(BOOTSTRAP_VALUE_SECTION_DEFAULT));
            OString sVersionFileProductKey = aBootstrapFile.readString(
                    OString(BOOSTRAP_SECTION),
                    OString(BOOSTRAP_ITEM_PRODUCT_KEY),
                    OString(BOOTSTRAP_VALUE_PRODUCT_KEY_DEFAULT));
            CFG_TRACE_INFO_NI("provider bootstrapping: section to lookup the product version: %s", sVersionFileSection.getStr());
            CFG_TRACE_INFO_NI("provider bootstrapping: key to lookup the product version: %s", sVersionFileProductKey.getStr());

            // get the user path from the product version file
            sConvertable = aProductVersionFile.readString(
                sVersionFileSection,
                sVersionFileProductKey,
                OString());


            OUString sUserPath = OUString(sConvertable.getStr(), sConvertable.getLength(), nCvtEncoding);
            if (sUserPath.getLength() == 0)
            {
                CFG_TRACE_ERROR_NI("provider bootstrapping: product version file does not have expected entry");

                return SVERSION_INI_NO_ENTRY;
            }
            CFG_TRACE_INFO_NI("provider bootstrapping: raw user installation directory: %s", OUSTRING2ASCII(sUserPath));

            nStatus = BOOTSTRAP_OK; //every ini read ok
            {
                // normalize the path
                OUString sNormalized;
                // normalize the user directory path
                if (!oslOK(osl::File::getFileURLFromSystemPath(sUserPath, sNormalized)))
                {
                    CFG_TRACE_ERROR_NI("provider bootstrapping: could not normalize the user path from the product version file");

                    return SVERSION_INI_INVALID;
                }
                sUserPath = sNormalized;
            }

            CFG_TRACE_INFO_NI("provider bootstrapping: user install directory (normalized): %s", OUSTRING2ASCII(sUserPath));
            _rUserInstallPath = sUserPath;
            _sIniPath = OUString();
            OSL_ASSERT(nStatus == BOOTSTRAP_OK);
        }
        catch (::std::exception& e)
        {   // this exception may be thrown by the Profile ctor - though we checked the arguments we passed to
            // this ctor (thus preventing exceptions), this is just to be sure ...
            CFG_TRACE_ERROR("provider bootstrapping: exception craeting ini file. Error message: %s", e.what());

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

        CFG_TRACE_INFO("provider bootstrapping: composed name (ini file to lookup): %s", OUSTRING2ASCII(sProfileFile));

        // does our ini file exist in the user directory ?
        ::osl::DirectoryItem aItem;
        bool bExists = oslOK( aItem.get(sProfileFile, aItem) );

        if (bExists) _rProfileFile = sProfileFile;

        return bExists;
    }

    // ----------------------------------------------------------------------------------
    RetVal locateBootstrapFiles(OUString& _rOfficeInstall, OUString& _rUserInstallPath, OUString& _rProfileFile)
    {
        // get the office/user install directories (the latter may be empty resp. unknown)
        RetVal nLocateError = locateUserInstallationPath(_rOfficeInstall, _rUserInstallPath, _rProfileFile);

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

    // -----------------------------------------------------------------------------

} // anon namespace

// ---------------------------------------------------------------------------------------
} // namespace configmgr


