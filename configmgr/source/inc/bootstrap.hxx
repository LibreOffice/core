/*************************************************************************
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: jb $ $Date: 2001-08-06 16:06:35 $
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

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#define CONFIGMGR_BOOTSTRAP_HXX_

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
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <map>

namespace osl {
    class Profile;
}

namespace configmgr
{
    class IConfigSession;

    // ===================================================================================

    namespace uno = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;
    using ::rtl::OUString;
    using ::rtl::OString;

    // ===================================================================================
    #define PORTAL_SESSION_IDENTIFIER           "portal"
    #define REMOTE_SESSION_IDENTIFIER           "remote"
    #define LOCAL_SESSION_IDENTIFIER            "local"
    #define SETUP_SESSION_IDENTIFIER            "setup"
    #define PLUGIN_SESSION_IDENTIFIER           "plugin"


    // ===================================================================================
    // = Settings
    // ===================================================================================
    class Settings
    {
    public:
        enum Origin
        {
            SO_NOT_SET,
            SO_UNKNOWN,
            SO_FALLBACK,
            SO_INIFILE,
            SO_OVERRIDE,
            SO_DEFAULT,
            SO_ADJUSTMENT,
            SO_MANUAL
        };
        typedef OString Name;

        // a single setting
        class Setting
        {
            uno::Any        m_aValue;
            Origin          m_eOrigin;
        public:
            Setting() : m_aValue(), m_eOrigin(SO_NOT_SET) { }
            Setting(const OUString& _rValue, Origin _eOrigin) : m_aValue(uno::makeAny(_rValue)), m_eOrigin(_eOrigin) { }
            Setting(const sal_Int32 _nValue, Origin _eOrigin) : m_aValue(uno::makeAny(_nValue)), m_eOrigin(_eOrigin) { }
            Setting(const uno::Any& _rValue, Origin _eOrigin) : m_aValue(_rValue), m_eOrigin(_eOrigin) { }


            uno::Any    value()  const { return m_aValue; }
            Origin      origin() const { return m_eOrigin; };

            OUString        toString() const;
            sal_Int32       toInt32() const;
            sal_Bool        toBool() const;
        };
    protected:
        typedef std::map< Name, Setting > SettingsImpl;
        SettingsImpl        m_aImpl;

    public:
        typedef SettingsImpl::const_iterator Iterator;
    public:
        /// default ctor
        Settings();

        /** construct a settings object
           containing the given overrides

        */
        Settings(const uno::Sequence< uno::Any >& _rOverrides, Origin _eOrigin = SO_OVERRIDE);

        /// merge the given overrides into the object itself
        void mergeOverrides(const Settings& _rOverrides);

        // add settings
        void override(const uno::Sequence< uno::Any >& _rOverrides, Origin _eOrigin = SO_OVERRIDE);

        // check setting existence
        sal_Bool        haveSetting(Name const& _pName) const;
        Origin          getOrigin(Name const& _pName) const;

        void            putSetting(Name const&  _pName, const Setting& _rSetting);
        void            clearSetting(Name const& _pName);

        OUString        getStringSetting(Name const& _pName) const;
        sal_Int32       getIntSetting(Name const& _pName) const;
        sal_Bool        getBoolSetting(Name const& _pName) const;
        Setting         getSetting(Name const& _pName) const;
        Setting         getMaybeSetting(Name const& _pName) const;

        Iterator begin()    const { return m_aImpl.begin(); }
        Iterator end()      const { return m_aImpl.end(); }

        void swap(Settings& _rOther) { m_aImpl.swap(_rOther.m_aImpl); }
    };

    class ConnectionSettings
    {
        friend class BootstrapSettings;

        Settings m_aSettings;

        ConnectionSettings() : m_aSettings() {};
    public:
        ConnectionSettings(const uno::Sequence< uno::Any >& _rOverrides,
                            Settings::Origin _eOrigin = Settings::SO_OVERRIDE);

        /// merge the given overrides into the object itself
        void mergeOverrides(const Settings& _rOverrides);
        /// merge the given overrides into the object itself
        void mergeOverrides(const ConnectionSettings& _rOverrides)
        { mergeOverrides(_rOverrides.m_aSettings); }

        void loadFromInifile(osl::Profile & rProfile,
                             Settings::Origin _eOrigin = Settings::SO_INIFILE);

        void fillFromInifile(osl::Profile & rProfile,
                             Settings::Origin _eOrigin = Settings::SO_INIFILE);

        bool validate();

        bool isComplete() const;
        bool isComplete(OUString const& aSessionType) const;

        sal_Bool    isSessionTypeKnown() const;

        sal_Bool    hasUser() const;
        sal_Bool    hasPassword() const;

        sal_Bool    hasLocale() const;
        sal_Bool    hasAsyncSetting() const;

        sal_Bool    hasServer() const;
        sal_Bool    hasPort() const;
        sal_Bool    hasTimeout() const;
        sal_Bool    hasService() const;

        sal_Bool    isPlugin() const;
        sal_Bool    isLocalSession() const;
        sal_Bool    isRemoteSession() const;

        sal_Bool    isSourcePathValid() const;
        sal_Bool    isUpdatePathValid() const;
        sal_Bool    hasReinitializeFlag() const;

        // get a special setting
        OUString    getSessionType() const;

        OUString    getUser() const;
        OUString    getPassword() const;

        OUString    getLocale() const;
        sal_Bool    getAsyncSetting() const;

        OUString    getSourcePath() const;
        OUString    getUpdatePath() const;
        sal_Bool    getReinitializeFlag() const;

        OUString    getServer() const;
        OUString    getService() const;
        sal_Int32   getPort() const;
        sal_Int32   getTimeout() const;

        // make sure this behaves as a user session
        void setUserSession();
        void setUserSession(const OUString& _rRemoteServiceName);

        // make sure this behaves as an administrative session
        void setAdminSession();
        void setAdminSession(const OUString& _rRemoteServiceName);

        // set a new session type. Must be one of the *_SESSION_IDENTIFIER defines
        void setSessionType(const OUString& _rSessionIdentifier, Settings::Origin _eOrigin /*= SO_MANUAL*/);

        // set a desired service, only necessary in remote environments
        sal_Bool    isServiceRequired() const;
        void        setService(const OUString& _rService, Settings::Origin _eOrigin /*= SO_MANUAL*/);

        // set this to a wildcard locale
        void        setAnyLocale(Settings::Origin _eOrigin /*= SO_MANUAL*/);

        IConfigSession* createConnection(
            uno::Reference< lang::XMultiServiceFactory > const& _rxServiceMgr) const;

        void swap(ConnectionSettings& _rOther) { m_aSettings.swap(_rOther.m_aSettings); }
    protected:

        bool checkSettings() const;

        /** @return <TRUE/> if the setting exists and is a valid path
        */
        sal_Bool isValidPathSetting(Settings::Name const& _pSetting) const;
        sal_Bool implPutSystemPathSetting(Settings::Name const& _pSetting, OUString const& _sSystemPath, Settings::Origin _eOrigin);
        sal_Bool implNormalizePathSetting(Settings::Name const& _pSetting);

        // translate old settings, which exist for compatiblity only, into new ones
        void implTranslateCompatibilitySettings();

        // if we do not already have the given config path setting, ensure that it exists (calculated relative to a given path)
        sal_Bool ensureConfigPath(Settings::Name const& _pSetting, const OUString& _rBasePath);

        // if we do not already have path settings, ensure that they exists (in an office install)
        sal_Bool implAdjustToInstallation(const OUString& _rShareDataPath, const OUString& _rUserDataPath);

    private:

        // collect settings from the sregistry, put them into m_aSettings
        void implCollectSRegistrySetting(osl::Profile & rProfile, Settings::Origin _eOrigin);

        static OUString     getProfileStringItem(osl::Profile & rProfile, OString const& _pSection, OString const& _pKey, rtl_TextEncoding _nEncoding);
        static sal_Int32    getProfileIntItem(osl::Profile & rProfile, OString const& _pSection, OString const& _pKey);

        // ensures that m_aImpl contains a session type
        void implDetermineSessionType();
        // clear items which are not relevant because of the session type origin
        void implClearIrrelevantItems();

    // convenience wrappers for Settings members
    public:
        sal_Bool    haveSetting(Settings::Name const& _pName) const
        { return m_aSettings.haveSetting(_pName); }

        Settings::Setting   getSetting(Settings::Name const& _pName) const
        { return m_aSettings.getSetting(_pName); }

    // private convenience wrappers for Settings members
    private:
        void        putSetting(Settings::Name const&  _pName, const Settings::Setting& _rSetting)
        { m_aSettings.putSetting(_pName,_rSetting); }

        void        clearSetting(Settings::Name const& _pName)
        { m_aSettings.clearSetting(_pName); }
    };

// ===================================================================================

    class BootstrapSettings
    {
    public:
        ConnectionSettings  settings; /// the settings collected from bootstrapping (may work even if !valid)
        bool                valid;    /// indicates whether the whole bootstrap process was executed successfully

        BootstrapSettings()
        : settings()
        , valid(false)
        {
            bootstrap();
        }

        void bootstrap();
    };

    extern void raiseBootstrapException( class BootstrapSettings const& rBootstrapData, uno::Reference< uno::XInterface > xContext );
// ===================================================================================
}

#endif // CONFIGMGR_BOOTSTRAP_HXX_


