/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: jb $ $Date: 2002-07-03 15:54:37 $
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

#include "bootstrap.hxx"

#ifndef CONFIGMGR_SESSIONFACTORY_HXX_
#include "sessionfactory.hxx"
#endif
#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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

// ---------------------------------------------------------------------------------------
#define BOOTSTRAP_CONFIGMGR_DATA            SAL_CONFIGFILE("configmgr")
// ---------------------------------------------------------------------------------------
#define SETTING_UNOSERVICE                  "BackendService"
#define SETTING_UNOWRAPPER                  "BackendWrapper"
#define SETTING_OFFLINE                     "Offline"
// ---------------------------------------------------------------------------------------
// configuration bootstrap items
#define BOOTSTRAP_ITEM_PREFIX_              "CFG_"

#define BOOTSTRAP_ITEM_SERVERTYPE           "CFG_ServerType"
#define BOOTSTRAP_ITEM_UNOSERVICE           BOOTSTRAP_ITEM_PREFIX_ SETTING_UNOSERVICE
#define BOOTSTRAP_ITEM_UNOWRAPPER           BOOTSTRAP_ITEM_PREFIX_ SETTING_UNOWRAPPER
#define BOOTSTRAP_ITEM_LOCALE               BOOTSTRAP_ITEM_PREFIX_ "Locale"
#define BOOTSTRAP_ITEM_ASYNCENABLE          BOOTSTRAP_ITEM_PREFIX_ "EnableAsync"

#define BOOTSTRAP_ITEM_SOURCE_PATH          "CFG_BaseDataURL"
#define BOOTSTRAP_ITEM_UPDATE_PATH          "CFG_UserDataURL"

#define BOOTSTRAP_ITEM_SERVER               "CFG_Server"
#define BOOTSTRAP_ITEM_TIMEOUT              "CFG_Timeout_ms"

#define BOOTSTRAP_ITEM_USER                 "CFG_User"
#define BOOTSTRAP_ITEM_PASSWORD             "CFG_Password"

#define BOOTSTRAP_ITEM_PROFILE_NAME         "CFG_INIFILE"

// ---------------------------------------------------------------------------------------
// default bootstrap values from an INI ('sregistryrc')
#define BOOTSTRAP_FROM_PROFILE(sect,key) "${$" BOOTSTRAP_ITEM_PROFILE_NAME ":" sect ":" key "}"

#define BOOTSTRAP_SERVERTYPE_FROM_PROFILE   BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_CONFIGURATION,SREGISTRY_KEY_SERVERTYPE)
#define BOOTSTRAP_UNOSERVICE_FROM_PROFILE   BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_CONFIGURATION,BOOTSTRAP_ITEM_UNOSERVICE)
#define BOOTSTRAP_UNOWRAPPER_FROM_PROFILE   BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_CONFIGURATION,BOOTSTRAP_ITEM_UNOWRAPPER)

#define BOOTSTRAP_LOCALE_FROM_PROFILE       BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_CONFIGURATION,SREGISTRY_KEY_LOCALE)
#define BOOTSTRAP_ASYNCENABLE_FROM_PROFILE  BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_CONFIGURATION,SREGISTRY_KEY_ASYNC)

#define BOOTSTRAP_SERVER_FROM_PROFILE       BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_REMOTE,SREGISTRY_KEY_SERVER)
#define BOOTSTRAP_TIMEOUT_FROM_PROFILE      BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_REMOTE,SREGISTRY_KEY_TIMEOUT)

#define BOOTSTRAP_BASEPATH_FROM_PROFILE     BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_LOCAL,SREGISTRY_KEY_SOURCEPATH)
#define BOOTSTRAP_DATAPATH_FROM_PROFILE     BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_LOCAL,SREGISTRY_KEY_UPDATEPATH)

#define BOOTSTRAP_USER_FROM_PROFILE         BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_AUTHENTICATION,SREGISTRY_KEY_USER)
#define BOOTSTRAP_PASSWORD_FROM_PROFILE     BOOTSTRAP_FROM_PROFILE(SREGISTRY_SECTION_AUTHENTICATION,SREGISTRY_KEY_PASSWORD)

// ---------------------------------------------------------------------------------------
// sections and entries of srgeistryrc
#define SREGISTRY_SECTION_CONFIGURATION     "configuration"
#define SREGISTRY_KEY_SERVERTYPE            "servertype"
#define SREGISTRY_KEY_SESSIONCLASS          "service"
#define SREGISTRY_KEY_LOCALE                "locale"
#define SREGISTRY_KEY_ASYNC                 "enable_async"

#define SREGISTRY_SECTION_REMOTE            "RemoteRegistry"
#define SREGISTRY_KEY_SERVER                "Server"
#define SREGISTRY_KEY_TIMEOUT               "Timeout"

#define SREGISTRY_SECTION_LOCAL             "LocalRegistry"
#define SREGISTRY_KEY_SOURCEPATH            "sourcepath"
#define SREGISTRY_KEY_UPDATEPATH            "updatepath"

#define SREGISTRY_SECTION_AUTHENTICATION    "Authentication"
#define SREGISTRY_KEY_USER                  "User"
#define SREGISTRY_KEY_PASSWORD              "Password"
// ---------------------------------------------------------------------------------------

// legacy settings
#define SETTING_SERVERTYPE                  "servertype"
#define SETTING_SESSIONCLASS               "_session_class_"
#define SETTING_LOCALE                      "locale"
#define SETTING_ASYNC                       "lazywrite"

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
// authentication settings
#define SETTING_USER                        "user"
#define SETTING_PASSWORD                    "password"
// 'option' settings

// ---------------------------------------------------------------------------------------
#define SERVICE_USERSESSION                 "configuration"
#define SERVICE_ADMINSESSION                "adminconfiguration"
// ---------------------------------------------------------------------------------------
#define NAME( N ) OUString(RTL_CONSTASCII_USTRINGPARAM(N))
#define ITEM( N ) OUString(RTL_CONSTASCII_USTRINGPARAM(N))
// ---------------------------------------------------------------------------------------
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
        SETTING_SOURCEPATH, SETTING_UPDATEPATH, SETTING_REINITIALIZE,
        SETTING_UNOSERVICE, SETTING_UNOWRAPPER, SETTING_OFFLINE
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
        { UNO_SESSION_IDENTIFIER,           createNoSession },
        { REMOTE_SESSION_IDENTIFIER,        createRemoteSession },
        { LOCAL_SESSION_IDENTIFIER,         createLocalSession  },
        { PORTAL_SESSION_IDENTIFIER,        createPortalSession },
        { SETUP_SESSION_IDENTIFIER,         createSetupSession }
    };
    int const nSessionClasses = sizeof(aSessionClasses)/sizeof(aSessionClasses[0]);

// ---------------------------------------------------------------------------------------
    OUString Settings::Setting::toString() const
    {
        OUString sReturn;

        sal_Bool bSuccess = this->m_aValue >>= sReturn;

        OSL_ENSURE(bSuccess || !this->m_aValue.hasValue(), "Settings::Setting::toString: setting is not a string!");

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
                bSuccess = (nReturn != 0 );
            }
        }
        OSL_ENSURE(bSuccess || !this->m_aValue.hasValue(), "Settings::getIntSetting: setting is not an integer!");
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
        case uno::TypeClass_VOID: // don't assert, if no value
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
        implAddOverrides(_rOverrides,_eOrigin);
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
    void Settings::implAddOverrides(const Sequence< Any >& _rOverrides, Origin _eOrigin)
    {
        // transfer the runtime overrides
        const sal_Int32 nCount = _rOverrides.getLength();
        OSL_ENSURE(0 <= nCount && nCount <= 0x7FFF, "Unexpected number of arguments");

        Any const * pOverrides = _rOverrides.getConstArray();
        for (sal_Int32 nArg = 0; nArg < nCount; ++nArg)
        {
            OUString sName;
            Any aValue;

            if ( implExtractOverride(pOverrides[nArg],sName,aValue) )
            {
                putSetting(sName, Setting(aValue, _eOrigin));
                CFG_TRACE_INFO("provider bootstrapping: runtime parameter: %s", OUSTRING2ASCII(sName));
            }
            else
            {
                CFG_TRACE_ERROR("provider bootstrapping: illegal parameter of type %s", OUSTRING2ASCII(pOverrides[nArg].getValueType().getTypeName()));
                throw IllegalArgumentException(OUString::createFromAscii("Configuration: Provider Creation Argument is not a com.sun.star.beans.PropertyValue or NameValue."), NULL, sal_Int16(nArg));
            }
        }
    }

// ---------------------------------------------------------------------------------------
    bool Settings::implExtractOverride(const Any & _rOverride, Name& _rName, Any& _rValue)
    {
        {
            PropertyValue aCurrentArgPV;
            // it must be a PropertyValue
            if (_rOverride >>= aCurrentArgPV)
            {
                _rName  = aCurrentArgPV.Name;
                _rValue = aCurrentArgPV.Value;

                return true;
            }
        }
        {
            NamedValue aCurrentArgNV;
            // or a NamedValue
            if (_rOverride >>= aCurrentArgNV)
            {
                _rName  = aCurrentArgNV.Name;
                _rValue = aCurrentArgNV.Value;

                return true;
            }
        }

        return false;
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
        // translate compatibility and special settings
        implNormalizeSettings();
    }

// ---------------------------------------------------------------------------------------

    uno::Sequence< beans::NamedValue > ConnectionSettings::getUnoSettings() const
    {
        uno::Sequence< beans::NamedValue > aResult( m_aSettings.size() );

        beans::NamedValue * p = aResult.getArray();

        for ( Settings::Iterator it = m_aSettings.begin(); it != m_aSettings.end(); ++it )
        {
            p->Name  = it->first;
            p->Value = it->second.value();
            ++p;
        }

        OSL_ASSERT(p - aResult.getConstArray() == aResult.getLength());

        return aResult;
    }
// ---------------------------------------------------------------------------------------

    void ConnectionSettings::setSessionType(const OUString& _rSessionIdentifier, Settings::Origin _eOrigin)
    {
        putSetting(NAME(SETTING_SERVERTYPE), Settings::Setting(_rSessionIdentifier, _eOrigin));
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setService(const OUString& _rService, Settings::Origin _eOrigin)
    {
        putSetting(NAME(SETTING_SERVICE), Settings::Setting(_rService, _eOrigin));
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isSessionTypeKnown() const { return haveSetting(NAME(SETTING_SERVERTYPE)); }
    sal_Bool ConnectionSettings::hasUnoBackendService() const { return haveSetting(NAME(SETTING_UNOSERVICE)); }
    sal_Bool ConnectionSettings::hasUnoBackendWrapper() const { return haveSetting(NAME(SETTING_UNOWRAPPER)); }
    sal_Bool ConnectionSettings::hasUser() const            { return haveSetting(NAME(SETTING_USER)); }
    sal_Bool ConnectionSettings::hasPassword() const        { return haveSetting(NAME(SETTING_PASSWORD)); }
    sal_Bool ConnectionSettings::hasLocale() const          { return haveSetting(NAME(SETTING_LOCALE)); }
    sal_Bool ConnectionSettings::hasServer() const          { return haveSetting(NAME(SETTING_SERVER)); }
    sal_Bool ConnectionSettings::hasService() const         { return haveSetting(NAME(SETTING_SERVICE)); }
    sal_Bool ConnectionSettings::hasPort() const            { return haveSetting(NAME(SETTING_PORT)); }
    sal_Bool ConnectionSettings::hasTimeout() const         { return haveSetting(NAME(SETTING_TIMEOUT)); }
    sal_Bool ConnectionSettings::hasAsyncSetting() const    { return haveSetting(NAME(SETTING_ASYNC)); }
    sal_Bool ConnectionSettings::hasOfflineSetting() const  { return haveSetting(NAME(SETTING_OFFLINE)); }
    sal_Bool ConnectionSettings::hasReinitializeFlag() const { return haveSetting(NAME(SETTING_REINITIALIZE)); }

// ---------------------------------------------------------------------------------------
    OUString    ConnectionSettings::getSessionType() const  { return m_aSettings.getStringSetting(NAME(SETTING_SERVERTYPE)); }
    OUString    ConnectionSettings::getUnoBackendService() const { return m_aSettings.getStringSetting(NAME(SETTING_UNOSERVICE)); }
    OUString    ConnectionSettings::getUnoBackendWrapper() const { return m_aSettings.getStringSetting(NAME(SETTING_UNOWRAPPER)); }
    OUString    ConnectionSettings::getUser() const         { return m_aSettings.getStringSetting(NAME(SETTING_USER)); }
    OUString    ConnectionSettings::getPassword() const     { return m_aSettings.getStringSetting(NAME(SETTING_PASSWORD)); }
    OUString    ConnectionSettings::getLocale() const       { return m_aSettings.getStringSetting(NAME(SETTING_LOCALE)); }
    OUString    ConnectionSettings::getSourcePath() const   { return m_aSettings.getStringSetting(NAME(SETTING_SOURCEPATH)); }
    OUString    ConnectionSettings::getUpdatePath() const   { return m_aSettings.getStringSetting(NAME(SETTING_UPDATEPATH)); }
    OUString    ConnectionSettings::getServer() const       { return m_aSettings.getStringSetting(NAME(SETTING_SERVER)); }
    OUString    ConnectionSettings::getService() const      { return m_aSettings.getStringSetting(NAME(SETTING_SERVICE)); }
    sal_Int32   ConnectionSettings::getPort() const         { return m_aSettings.getIntSetting(NAME(SETTING_PORT)); }
    sal_Int32   ConnectionSettings::getTimeout() const      { return m_aSettings.getIntSetting(NAME(SETTING_TIMEOUT)); }
    sal_Bool ConnectionSettings::getAsyncSetting() const    { return m_aSettings.getBoolSetting(NAME(SETTING_ASYNC)); }
    sal_Bool ConnectionSettings::getOfflineSetting() const  { return m_aSettings.getBoolSetting(NAME(SETTING_OFFLINE)); }
    sal_Bool ConnectionSettings::getReinitializeFlag() const { return m_aSettings.getBoolSetting(NAME(SETTING_REINITIALIZE)); }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isSourcePathValid() const
    {
        return haveSetting(NAME(SETTING_SOURCEPATH));
    }

// ---------------------------------------------------------------------------------------
    sal_Bool ConnectionSettings::isUpdatePathValid() const
    {
        return haveSetting(NAME(SETTING_UPDATEPATH));
    }

// ---------------------------------------------------------------------------------------
    static inline bool isValidFileURL(OUString const& _sFileURL)
    {
        using osl::File;

        OUString sSystemPath;
        return _sFileURL.getLength() && (File::E_None == File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
    }
// ---------------------------------------------------------------------------------------
    static
    bool implEnsureAbsoluteURL(OUString & _rsURL) // also strips embedded dots etc.
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
            OSL_ENSURE(false, "Could not get absolute file URL for valid URL");
            return false;
        }
    }
// ---------------------------------------------------------------------------------------

    static
    bool implNormalizeURL(OUString const & _sPathOrURL, OUString& _rsURL)
    {
        using osl::File;

        OUString sOther;

        bool bURL = false;

        // if empty, clear _rsURL, but return false
        if (_sPathOrURL.getLength() == 0)
        {
            _rsURL = _sPathOrURL;
            bURL = false;
        }

        // check if it already was normalized
        else if ( File::E_None == File::getSystemPathFromFileURL(_sPathOrURL, sOther) )
        {
            _rsURL = _sPathOrURL;
            bURL = true;
        }

        // allow for system pathes
        else if ( File::E_None == File::getFileURLFromSystemPath(_sPathOrURL, sOther) )
        {
            CFG_TRACE_WARNING_NI("provider bootstrapping: URL was specified as system path '%s'.", OUSTRING2ASCII(_sPathOrURL));
            _rsURL = sOther;
            bURL = true;
        }

        else
            bURL = false;

        return bURL && implEnsureAbsoluteURL(_rsURL);
    }
// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::implNormalizePathSetting(Settings::Name const& _pSetting)
    {
        using osl::File;

        if (!haveSetting(_pSetting))
            return false;

        Settings::Setting aSetting = getSetting(_pSetting);

        OUString const sValue = aSetting.toString();

        OUString sNormalized;

        if ( implNormalizeURL(sValue,sNormalized) )
        {
            putSetting(_pSetting, Settings::Setting(sNormalized,aSetting.origin()) );
            return true;
        }
        else
        {
            CFG_TRACE_ERROR_NI("provider bootstrapping: could not normalize URL (setting: %s, value: %s).", OUSTRING2ASCII(_pSetting), OUSTRING2ASCII(sValue));
            clearSetting(_pSetting);
            return false;
        }
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
                if (aCheck->first.equalsAscii(pKnownSettings[i]))
                {
                    bRecognized = true;
                    break;
                }
            }

            if (!bRecognized)
            {
                OSL_ENSURE(bRecognized, "Configuration: unknown provider parameter\n");
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

        return implDetermineSessionType() && isComplete( getSessionType() );
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

        else if(0 == sSessionType.compareToAscii(UNO_SESSION_IDENTIFIER) )
        {
            // cannot check backend-specific arguments here
            return hasUnoBackendService() ||
                    hasUnoBackendWrapper() && hasOfflineSetting() && getOfflineSetting();
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
    void ConnectionSettings::implMergeOverrides(const Settings& _rOverrides)
    {
        // update path validity depends on data in its base source path
        if (_rOverrides.haveSetting(NAME(SETTING_SOURCEPATH)))
        {
            this->clearSetting(NAME(SETTING_SOURCEPATH));
            this->clearSetting(NAME(SETTING_UPDATEPATH));
        }

        // port is only valid relative to server (and may even be part of the server setting)
        if (_rOverrides.haveSetting(NAME(SETTING_SERVER)))
        {
            this->clearSetting(NAME(SETTING_SERVER));
            this->clearSetting(NAME(SETTING_PORT));
        }

        // if changing the user, password must be changed as well
        if (_rOverrides.haveSetting(NAME(SETTING_USER)))
        {
            this->clearSetting(NAME(SETTING_USER));
            this->clearSetting(NAME(SETTING_PASSWORD));
        }

        m_aSettings.mergeOverrides(_rOverrides);
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implNormalizeRemoteServer()
    {
        Settings::Name const sServerSetting = NAME(SETTING_SERVER);
        Settings::Name const sPortSetting = NAME(SETTING_PORT);

        if (haveSetting(sServerSetting) && !haveSetting(sPortSetting))
        {
            Settings::Setting aServerData = getSetting(sServerSetting);

            OUString sServerAndPort = aServerData.toString();

            // there was such an entry
            const sal_Int32 nColon = sServerAndPort.lastIndexOf(':');
            if (0 <= nColon)
            {
                if (sal_Int32 nPort = sServerAndPort.copy(nColon+1).toInt32())
                {
                    OUString sServer = sServerAndPort.copy(0, nColon);

                    Settings::Origin eOrigin = aServerData.origin();
                    putSetting(sServerSetting,  Settings::Setting(sServer, eOrigin));
                    putSetting(sPortSetting,    Settings::Setting(nPort, eOrigin));
                }
            }
        }

    }
// ---------------------------------------------------------------------------------------
    void ConnectionSettings::implNormalizeSettings()
    {
        implNormalizePathSetting(NAME(SETTING_SOURCEPATH));
        implNormalizePathSetting(NAME(SETTING_UPDATEPATH));

        implNormalizeRemoteServer();
    }

// ---------------------------------------------------------------------------------------
    bool ConnectionSettings::implDetermineSessionType()
    {
        Settings::Name const sSettingServertype = NAME(SETTING_SERVERTYPE);
        if (!haveSetting(sSettingServertype))
        {   // we already have the setting
            CFG_TRACE_INFO("provider bootstrapping: no session type. looking for fallback");

            char const * psSessionType = NULL;
            if (haveSetting(NAME(SETTING_UNOSERVICE)))
            {
                psSessionType = UNO_SESSION_IDENTIFIER;
            }
            else if (haveSetting(NAME(SETTING_SOURCEPATH)) && haveSetting(NAME(SETTING_UPDATEPATH)))
            {
                psSessionType = LOCAL_SESSION_IDENTIFIER;
            }
            else if (haveSetting(NAME(SETTING_SERVER)))
            {
                psSessionType = REMOTE_SESSION_IDENTIFIER;
            }
            else if (haveSetting(NAME(SETTING_SERVICE)))
            {
                psSessionType = PORTAL_SESSION_IDENTIFIER;
            }
            else
            {
                CFG_TRACE_WARNING_NI("provider bootstrapping: cannot determine session type"
                                     "- using fallback to '"UNO_SESSION_IDENTIFIER"'");
                psSessionType = PORTAL_SESSION_IDENTIFIER;
            }

            if (!psSessionType)
            {
                CFG_TRACE_WARNING_NI("provider bootstrapping: cannot determine session type");
                return false;
            }

            OUString sSessionType = OUString::createFromAscii(psSessionType);
            putSetting(sSettingServertype, Settings::Setting(sSessionType, Settings::SO_FALLBACK));
        }
        CFG_TRACE_INFO_NI("provider bootstrapping: using session type: %s",
                        OUSTRING2ASCII(m_aSettings.getStringSetting(sSettingServertype)));

        return true;
    }

// ---------------------------------------------------------------------------------------

    bool ConnectionSettings::isUnoBackend() const
    {
        if (!isSessionTypeKnown()) return false;

        OUString const sSessionType = getSessionType();
        return  (0 == sSessionType.compareToAscii(UNO_SESSION_IDENTIFIER));
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

    sal_Bool ConnectionSettings::isAdminSession() const
    {
        OSL_ENSURE(haveSetting(NAME(SETTING_SESSIONCLASS)),"Cannot determine session class");

        return getSetting(NAME(SETTING_SESSIONCLASS)).toString().equalsAscii(SERVICE_ADMINSESSION);

    }
// ---------------------------------------------------------------------------------------

    void ConnectionSettings::setUserSession()
    {
        OSL_ENSURE(isUnoBackend() || isLocalSession() || isRemoteSession(),"Invalid/No session type for user session");
        OSL_ENSURE(getSessionType().compareToAscii(SETUP_SESSION_IDENTIFIER) != 0, "WARNING: Explicit creation of 'setup' sessions is obsolete. Create 'AdministrationProvider' service instead");

        OUString const sService( RTL_CONSTASCII_USTRINGPARAM(SERVICE_USERSESSION) );

        if( !hasService() && isServiceRequired() )
        {
            CFG_TRACE_INFO("No service set for user session. Using default service '%s'",OUSTRING2ASCII(sService));

            this->setService(sService, Settings::SO_DEFAULT);
        }

        if ( !hasAsyncSetting() )
        {
            Any aDefaultAsync = makeAny(sal_Bool(true));
            putSetting( NAME(SETTING_ASYNC), Settings::Setting(aDefaultAsync, Settings::SO_DEFAULT) );
            OSL_ASSERT( hasAsyncSetting() && getAsyncSetting() );
        }

        putSetting(NAME(SETTING_SESSIONCLASS),Settings::Setting(sService, Settings::SO_DEFAULT));
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setAdminSession()
    {
        OSL_ENSURE(isUnoBackend() || isLocalSession() || isRemoteSession(),"Invalid/No session type for admin session");
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

        OUString const sService( RTL_CONSTASCII_USTRINGPARAM(SERVICE_ADMINSESSION) );

        if( !hasService() && isServiceRequired() )
        {
            CFG_TRACE_INFO("No service set for admin session. Using default service '%s'",OUSTRING2ASCII(sService));

            this->setService(sService, Settings::SO_DEFAULT);
        }

        if ( !hasAsyncSetting() )
        {
            Any aDefaultNoAsync = makeAny(sal_Bool(false));
            putSetting( NAME(SETTING_ASYNC), Settings::Setting(aDefaultNoAsync, Settings::SO_DEFAULT) );
            OSL_ASSERT( hasAsyncSetting() && !getAsyncSetting() );
        }

        if (!hasLocale())
        {
            CFG_TRACE_INFO("Settings for Admin session: Using 'all locales' by default");
            this->setAnyLocale(Settings::SO_DEFAULT);
        }

        putSetting(NAME(SETTING_SESSIONCLASS),Settings::Setting(sService, Settings::SO_DEFAULT));
    }

// ---------------------------------------------------------------------------------------
    void ConnectionSettings::setAnyLocale(Settings::Origin _eOrigin)
    {
        rtl::OUString sAnyLocale = localehelper::getAnyLocale();

        this->putSetting( NAME(SETTING_LOCALE), Settings::Setting(sAnyLocale, _eOrigin));
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
                return aSessionClasses[i].create(_rxServiceMgr, *this);
            }
        }

        OSL_ENSURE(false, "unable to bootstrap the configuration - no match for session type!");
        return 0;
    }

// ---------------------------------------------------------------------------------------
// caching helpers
//---------------------------------------------------------------------------------------
    OUString buildConnectString(const ConnectionSettings& _rSettings)
    {
        OSL_ENSURE(_rSettings.isComplete(),"WARNING: creating connect string for incomplete settings");

        rtl::OUStringBuffer sConnect = _rSettings.getSessionType();

#if 0 // disabled: service may not be set
        if (_rSettings.isServiceRequired() && _rSettings.hasService())
        {
            sConnect.append(sal_Unicode(':'));
            sConnect.append(_rSettings.getService());
        }
#endif
        if (_rSettings.isUnoBackend())
        {
            // no support for connection strings
            return OUString();
        }
        else if (_rSettings.isLocalSession())
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
// - bootstrapping helper
// ---------------------------------------------------------------------------------------
namespace {
// ---------------------------------------------------------------------------------------
// error handling
// ---------------------------------------------------------------------------------------
    enum BootstrapResult
    {
        BOOTSTRAP_DATA_OK,
        INCOMPLETE_BOOTSTRAP_DATA,
        INCOMPLETE_BOOTSTRAP_FILE,
        MISSING_BOOTSTRAP_FILE,
        BOOTSTRAP_FAILURE
    };
// ---------------------------------------------------------------------------------------
    static
    OUString getFallbackErrorMessage( BootstrapResult _rc )
    {
        OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("The program cannot start. "));

        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is missing"));
            break;

        case INCOMPLETE_BOOTSTRAP_FILE:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is invalid"));
            break;

        case INCOMPLETE_BOOTSTRAP_DATA:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Required bootstrap data is not available"));
            break;

        default:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Unexpected bootstrap failure"));
            break;

        case BOOTSTRAP_DATA_OK:
            break;
        }
        sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(" (No detailed error message available.)"));

        return sMessage;
    }
// ---------------------------------------------------------------------------------------

    static
    void impl_raiseBootstrapException( BootstrapResult _rc, OUString const& _sMessage, OUString const& _sURL, Reference< XInterface > _xContext )
    {
        OUString sMessage(_sMessage);
        // ensure a message
        if (sMessage.getLength()== 0)
        {
            OSL_ENSURE(false, "Bootstrap error message missing");

            sMessage = getFallbackErrorMessage(_rc);
        }

        using namespace com::sun::star::configuration;

        // raise the error
        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            throw MissingBootstrapFileException(sMessage, _xContext, _sURL);

        case INCOMPLETE_BOOTSTRAP_FILE:
            throw InvalidBootstrapFileException(sMessage, _xContext, _sURL);

        default: OSL_ENSURE(false, "Undefined BootstrapResult code");
        case INCOMPLETE_BOOTSTRAP_DATA:
        case BOOTSTRAP_FAILURE:
            throw CannotLoadConfigurationException(sMessage, _xContext);

        case BOOTSTRAP_DATA_OK:
            break;
        }
    }
// ---------------------------------------------------------------------------------------
} // anonymous namespace

// ---------------------------------------------------------------------------------------
// bootstrapping
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// class BootstrapSettings::Impl
// ---------------------------------------------------------------------------------------
    struct BootstrapSettings::Impl
    {
        Impl();

        bool hasInifile() const;
        bool getInifile(OUString& _rInifile) const;
        bool hasExistingInifile() const;

        void collectSettings(Settings& _rSettings);
        void collectDefaultsFromINI(Settings& _rSettings);
        void adjustToInstallation(Settings& _rSettings);

        void addSetting(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem);
        void addWithDefault(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem, OUString const& _sDefault);
        void maybeAddWithDefault(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem, OUString const& _sDefault);

        BootstrapResult getBootstrapErrorMessage(ConnectionSettings const& _rSettings, OUString& _rMessage, OUString& _rIniFile ) const;

        static bool urlExists(OUString const& _sURL);

        rtl::Bootstrap m_data;
    };

// ---------------------------------------------------------------------------------------

    void BootstrapSettings::bootstrap()
    {
        CFG_TRACE_INFO("provider bootstrapping: collecting bootstrap setting");

        Impl aBootstrapper;

        aBootstrapper.collectSettings(this->settings.m_aSettings);

        if ( aBootstrapper.hasExistingInifile() )
            aBootstrapper.collectDefaultsFromINI(this->settings.m_aSettings);

        this->settings.implNormalizeSettings();

        this->valid = this->settings.implDetermineSessionType();

        aBootstrapper.adjustToInstallation(this->settings.m_aSettings);
        this->settings.implNormalizeSettings();

        if (this->valid)
        {
            this->valid = this->settings.isComplete();

            if (!this->valid)
                CFG_TRACE_WARNING_NI("provider bootstrapping: bootstrap data is incomplete");
        }
        else
        {
            CFG_TRACE_WARNING_NI("provider bootstrapping: could not collect bootstrap data");

            if (!this->settings.implDetermineSessionType())
                CFG_TRACE_ERROR_NI("provider bootstrapping: no default session available");
        }
    }
// ---------------------------------------------------------------------------------------

    void BootstrapSettings::raiseBootstrapException(Reference< XInterface > const & _xContext ) const
    {
        if (!this->valid)
        {
            OUString sMessage,sURL;

            BootstrapResult rc = Impl().getBootstrapErrorMessage(this->settings,sMessage,sURL);

            impl_raiseBootstrapException(rc,sMessage,sURL,_xContext);

            OSL_ASSERT(rc == BOOTSTRAP_DATA_OK);

            // could not discover what went wrong => no exception here
            OSL_ENSURE(false, "cannot detect bootstrap error");
        }
    }
// ---------------------------------------------------------------------------------------
    // need an exported symbol of this shared object
    extern "C" sal_Bool SAL_CALL component_writeInfo(void* , void* );

    static OUString getCurrentModuleDirectory()
    {
        OUString aFileURL;
        if ( !osl::Module::getUrlFromAddress((void*)&component_writeInfo,aFileURL) )
        {
            OSL_TRACE(false, "Cannot locate current module - using executable instead");

            OSL_VERIFY(osl_Process_E_None == osl_getExecutableFile(&aFileURL.pData));
        }

        OSL_ENSURE(0 < aFileURL.lastIndexOf('/'), "Cannot find directory for module URL");

        return aFileURL.copy(0, aFileURL.lastIndexOf('/'));
    }
// ---------------------------------------------------------------------------------------
    OUString BootstrapSettings::getURL()
    {
        return getCurrentModuleDirectory() + OUString(RTL_CONSTASCII_USTRINGPARAM("/"BOOTSTRAP_CONFIGMGR_DATA));
    }
// ---------------------------------------------------------------------------------------
    BootstrapSettings::Impl::Impl()
    : m_data(getURL())
    {
    }
// ---------------------------------------------------------------------------------------
    inline
    bool BootstrapSettings::Impl::urlExists(OUString const& _sURL)
    {
        osl::DirectoryItem aCheck;
        return (osl::DirectoryItem::get(_sURL,aCheck) == osl::DirectoryItem::E_None);
    }
// ---------------------------------------------------------------------------------------
    inline
    bool BootstrapSettings::Impl::getInifile(OUString& _rInifile) const
    {
        if (m_data.getFrom(ITEM(BOOTSTRAP_ITEM_PROFILE_NAME),_rInifile))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
// ---------------------------------------------------------------------------------------

    bool BootstrapSettings::Impl::hasInifile() const
    {
        OUString sInifile;
        bool bResult = getInifile(sInifile);
        return bResult;
    }
// ---------------------------------------------------------------------------------------

    bool BootstrapSettings::Impl::hasExistingInifile() const
    {
        OUString sInifile;
        if ( getInifile(sInifile) )
        {
            if (urlExists(sInifile))
            {
                CFG_TRACE_INFO_NI("provider bootstrapping: using configuration INI file '%'", OUSTRING2ASCII(sInifile));
                return true;
            }
            else
                CFG_TRACE_WARNING_NI("provider bootstrapping: specified configuration INI file '%s' does not exist", OUSTRING2ASCII(sInifile));
        }
        else
            CFG_TRACE_INFO_NI("provider bootstrapping: no configuration INI file specified");

        return false;
    }
// ---------------------------------------------------------------------------------------

    void BootstrapSettings::Impl::addWithDefault(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem, OUString const& _sDefault)
    {
        OUString sValue;
        m_data.getFrom(_sBootstrapItem, sValue, _sDefault);
        if (sValue.getLength())
            _rSettings.putSetting( _sSetting, Settings::Setting(sValue, Settings::SO_INIFILE) );
    }
// ---------------------------------------------------------------------------------------

    void BootstrapSettings::Impl::addSetting(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem)
    {
        OUString sValue;
        if (m_data.getFrom(_sBootstrapItem,sValue))
        {
            _rSettings.putSetting(_sSetting,Settings::Setting(sValue, Settings::SO_BOOTSTRAP) );
        }
    }
// ---------------------------------------------------------------------------------------

    inline
    void BootstrapSettings::Impl::maybeAddWithDefault(Settings& _rSettings,Settings::Name const & _sSetting, OUString const& _sBootstrapItem, OUString const& _sDefault)
    {
        if (!_rSettings.haveSetting(_sSetting))
        {
            addWithDefault(_rSettings,_sSetting,_sBootstrapItem,_sDefault);
        }
    }
// ---------------------------------------------------------------------------------------

    void BootstrapSettings::Impl::collectDefaultsFromINI(Settings& _rSettings)
    {
        OUString sDummy;
        maybeAddWithDefault(_rSettings, NAME(SETTING_SERVERTYPE), ITEM(BOOTSTRAP_ITEM_SERVERTYPE),  ITEM(BOOTSTRAP_SERVERTYPE_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_UNOSERVICE), ITEM(BOOTSTRAP_ITEM_UNOSERVICE),  ITEM(BOOTSTRAP_UNOSERVICE_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_UNOWRAPPER), ITEM(BOOTSTRAP_ITEM_UNOWRAPPER),  ITEM(BOOTSTRAP_UNOWRAPPER_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_LOCALE),     ITEM(BOOTSTRAP_ITEM_LOCALE),      ITEM(BOOTSTRAP_LOCALE_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_ASYNC),      ITEM(BOOTSTRAP_ITEM_ASYNCENABLE), ITEM(BOOTSTRAP_ASYNCENABLE_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_SOURCEPATH), ITEM(BOOTSTRAP_ITEM_SOURCE_PATH), ITEM(BOOTSTRAP_BASEPATH_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_UPDATEPATH), ITEM(BOOTSTRAP_ITEM_UPDATE_PATH), ITEM(BOOTSTRAP_DATAPATH_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_SERVER),     ITEM(BOOTSTRAP_ITEM_SERVER),      ITEM(BOOTSTRAP_SERVER_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_TIMEOUT),    ITEM(BOOTSTRAP_ITEM_TIMEOUT),     ITEM(BOOTSTRAP_TIMEOUT_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_USER),       ITEM(BOOTSTRAP_ITEM_USER),        ITEM(BOOTSTRAP_USER_FROM_PROFILE) );
        maybeAddWithDefault(_rSettings, NAME(SETTING_PASSWORD),   ITEM(BOOTSTRAP_ITEM_PASSWORD),    ITEM(BOOTSTRAP_PASSWORD_FROM_PROFILE) );

    }
// ---------------------------------------------------------------------------------------

    void BootstrapSettings::Impl::collectSettings(Settings& _rSettings)
    {
        addSetting(_rSettings, NAME(SETTING_SERVERTYPE), ITEM(BOOTSTRAP_ITEM_SERVERTYPE) );
        addSetting(_rSettings, NAME(SETTING_UNOSERVICE), ITEM(BOOTSTRAP_ITEM_UNOSERVICE) );
        addSetting(_rSettings, NAME(SETTING_UNOWRAPPER), ITEM(BOOTSTRAP_ITEM_UNOWRAPPER) );
        addSetting(_rSettings, NAME(SETTING_LOCALE),     ITEM(BOOTSTRAP_ITEM_LOCALE) );
        addSetting(_rSettings, NAME(SETTING_ASYNC),      ITEM(BOOTSTRAP_ITEM_ASYNCENABLE) );
        addSetting(_rSettings, NAME(SETTING_SOURCEPATH), ITEM(BOOTSTRAP_ITEM_SOURCE_PATH) );
        addSetting(_rSettings, NAME(SETTING_UPDATEPATH), ITEM(BOOTSTRAP_ITEM_UPDATE_PATH) );
        addSetting(_rSettings, NAME(SETTING_SERVER),     ITEM(BOOTSTRAP_ITEM_SERVER) );
        addSetting(_rSettings, NAME(SETTING_TIMEOUT),    ITEM(BOOTSTRAP_ITEM_TIMEOUT) );
        addSetting(_rSettings, NAME(SETTING_USER),       ITEM(BOOTSTRAP_ITEM_USER) );
        addSetting(_rSettings, NAME(SETTING_PASSWORD),   ITEM(BOOTSTRAP_ITEM_PASSWORD) );
    }
// ---------------------------------------------------------------------------------------
    static inline void getDirWithDefault(
                    rtl::Bootstrap& _rData,
                    OUString const& _sItem,
                    OUString& _sResult,
                    OUString const& _sDefault
                )
    {
        _rData.getFrom(_sItem,_sResult,_sDefault);
    }
    //----------------------------------------
    #define BOOTSTRAP_ITEM_SHAREDIR        "$BaseInstallation/share"
    #define BOOTSTRAP_ITEM_USERDIR         "$UserInstallation/user"
    #define CONFIGURATION_STANDARDPATH     "config/registry"
    #define BOOTSTRAP_BASEPATH_DEFAULT     ITEM(BOOTSTRAP_ITEM_SHAREDIR"/"CONFIGURATION_STANDARDPATH)
    #define BOOTSTRAP_DATAPATH_DEFAULT     ITEM(BOOTSTRAP_ITEM_USERDIR "/"CONFIGURATION_STANDARDPATH)

// ---------------------------------------------------------------------------------------

    static inline OUString getDefaultSourcePath(rtl::Bootstrap& _rData)
    {
        OUString sResult;

        getDirWithDefault(_rData,ITEM(BOOTSTRAP_ITEM_SOURCE_PATH),sResult,BOOTSTRAP_BASEPATH_DEFAULT);

        return sResult;
    }
// ---------------------------------------------------------------------------------------

    static inline OUString getDefaultUpdatePath(rtl::Bootstrap& _rData)
    {
        OUString sResult;

        getDirWithDefault(_rData,ITEM(BOOTSTRAP_ITEM_UPDATE_PATH),sResult,BOOTSTRAP_DATAPATH_DEFAULT);

        return sResult;
    }
// ---------------------------------------------------------------------------------------
    // if we do not already have path settings, try to use the defaults (of an office install)
    void BootstrapSettings::Impl::adjustToInstallation(Settings& _rSettings)
    {
        // if we do not already have a source path setting, create one from the base install path
        Settings::Name const sSourceSetting = NAME(SETTING_SOURCEPATH);
        if (!_rSettings.haveSetting(sSourceSetting))
        {
            OUString aSourcePath = getDefaultSourcePath(m_data);

            if (isValidFileURL(aSourcePath) && urlExists(aSourcePath))
            {
                _rSettings.putSetting(sSourceSetting,Settings::Setting(aSourcePath,Settings::SO_DEFAULT));

                // and then  also try to update the Update-path
                Settings::Name const sUpdateSetting = NAME(SETTING_UPDATEPATH);
                if (!_rSettings.haveSetting(sUpdateSetting))
                {
                    OUString aUpdatePath = getDefaultUpdatePath(m_data);

                    if (isValidFileURL(aUpdatePath))
                        _rSettings.putSetting(sUpdateSetting,Settings::Setting(aUpdatePath,Settings::SO_DEFAULT));
                }
            }
        }
    }
// ---------------------------------------------------------------------------------------

    static
    OUString buildBootstrapError( sal_Char const* _sWhat, OUString const& _sName, sal_Char const* _sHow)
    {
        rtl::OUStringBuffer sMessage;

        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM("The program cannot start. "));
        sMessage.appendAscii(_sWhat);
        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM(" '")).append(_sName).appendAscii(RTL_CONSTASCII_STRINGPARAM("' "));
        sMessage.appendAscii(_sHow).appendAscii(". ");

        return sMessage.makeStringAndClear();
    }
// ---------------------------------------------------------------------------------------

    BootstrapResult BootstrapSettings::Impl::getBootstrapErrorMessage(ConnectionSettings const & _rSettings, OUString& _rMessage, OUString& _rIniFile ) const
    {
        BootstrapResult eResult = BOOTSTRAP_DATA_OK;

        if ( this->getInifile(_rIniFile) )
        {
            if ( urlExists(_rIniFile) )
            {
                _rMessage = buildBootstrapError("The configuration file ",_rIniFile.copy(1+_rIniFile.lastIndexOf('/')),"is invalid");
                eResult = INCOMPLETE_BOOTSTRAP_FILE;
            }
            else
            {
                _rMessage = buildBootstrapError("The configuration file ",_rIniFile.copy(1+_rIniFile.lastIndexOf('/')),"is missing");
                eResult = MISSING_BOOTSTRAP_FILE;
            }
        }
        else if (!_rSettings.isSessionTypeKnown())
        {
            _rMessage = buildBootstrapError("Needed information to access",OUString::createFromAscii("application"),"configuration data is missing");
            eResult = BOOTSTRAP_FAILURE;
        }
        else if (!_rSettings.isComplete() )
        {
             _rMessage = buildBootstrapError("Needed information to access",_rSettings.getSessionType(), "configuration data is missing");
            eResult = INCOMPLETE_BOOTSTRAP_DATA;
        }

        return eResult;
    }
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
} // namespace configmgr


