/*************************************************************************
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-05 14:31:45 $
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
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace osl {
    class Profile;
}

namespace configmgr
{
    class IConfigSession;

    // ===================================================================================

    #define PORTAL_SESSION_IDENTIFIER           "portal"
    #define REMOTE_SESSION_IDENTIFIER           "remote"
    #define LOCAL_SESSION_IDENTIFIER            "local"
    #define SETUP_SESSION_IDENTIFIER            "setup"
    #define PLUGIN_SESSION_IDENTIFIER           "plugin"

    // ===================================================================================
    // = ConnectionSettings
    // ===================================================================================
    class ConnectionSettings
    {
    public:
        enum SETTING_ORIGIN
        {
            SO_SREGISTRY,
            SO_OVERRIDE,
            SO_FALLBACK,
            SO_UNKNOWN
        };

    protected:
        // ine single setting
        struct Setting
        {
            ::com::sun::star::uno::Any      aValue;
            SETTING_ORIGIN                  eOrigin;

            Setting() : eOrigin(SO_UNKNOWN) { }
            Setting(const ::rtl::OUString& _rValue, SETTING_ORIGIN _eOrigin) : aValue(::com::sun::star::uno::makeAny(_rValue)), eOrigin(_eOrigin) { }
            Setting(const sal_Int32 _nValue, SETTING_ORIGIN _eOrigin) : aValue(::com::sun::star::uno::makeAny(_nValue)), eOrigin(_eOrigin) { }
            Setting(const ::com::sun::star::uno::Any& _rValue, SETTING_ORIGIN _eOrigin) : aValue(_rValue), eOrigin(_eOrigin) { }
        };
        DECLARE_STL_USTRINGACCESS_MAP( Setting, SettingsImpl );
        SettingsImpl        m_aImpl;

        ::osl::Profile*     m_pSRegistry;
        sal_Bool            m_bFoundRegistry;

    public:
        /// default ctor
        ConnectionSettings();

        /// dtor
        ~ConnectionSettings();

        /// copy ctor
        ConnectionSettings(const ConnectionSettings& _rSource);

        /** construct a settings object.

            <p>The runtime overrides given will be merged with the settings found in a sversion.ini (sversionrc),
            if any. The former overrule the latter</p>
        */
        ConnectionSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rRuntimeOverrides );

        /// merge the given overrides into a new ConnectionSettings object
        ConnectionSettings& mergeOverrides(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rOverrides);

        /// merge the given overrides into the object itself
        ConnectionSettings createMergedSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rOverrides) const
        {
            return ConnectionSettings(*this).mergeOverrides(_rOverrides);
        }

        // check setting existence
        sal_Bool            hasRegistry() const { return m_bFoundRegistry; }
        sal_Bool            hasUser() const;
        sal_Bool            hasPassword() const;
        sal_Bool            hasLocale() const;
        sal_Bool            hasAsyncSetting() const;
        sal_Bool            hasServer() const;
        sal_Bool            hasPort() const;
        sal_Bool            hasTimeout() const;
        sal_Bool            hasService() const;

        sal_Bool            isLocalSession() const;
        sal_Bool            isRemoteSession() const;

        sal_Bool            isValidSourcePath() const;
        sal_Bool            isValidUpdatePath() const;

        // get a special setting
        ::rtl::OUString     getSessionType() const;
        ::rtl::OUString     getUser() const;
        ::rtl::OUString     getPassword() const;
        ::rtl::OUString     getLocale() const;
        ::rtl::OUString     getSourcePath() const;
        ::rtl::OUString     getUpdatePath() const;
        ::rtl::OUString     getServer() const;
        ::rtl::OUString     getService() const;
        sal_Int32           getPort() const;
        sal_Int32           getTimeout() const;
        sal_Bool            getAsyncSetting() const;

        // make sure this behaves as a user session
        void setUserSession();
        void setUserSession(const ::rtl::OUString& _rRemoteServiceName);

        // make sure this behaves as an administrative session
        void setAdminSession();
        void setAdminSession(const ::rtl::OUString& _rRemoteServiceName);

        // set a new session type. Must be one of the *_SESSION_IDENTIFIER defines
        void                setSessionType(const ::rtl::OUString& _rSessionIdentifier);
        // set a desired service, only necessary in remote environments
        sal_Bool            isServiceRequired() const;
        void                setService(const ::rtl::OUString& _rService);

        // set this to a wildcard locale
        void                setAnyLocale();

        IConfigSession* ConnectionSettings::createConnection(
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const& _rxServiceMgr) const;

    protected:
        void construct(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rOverrides);

        // transfer runtime overwrites into m_aImpl. Existent settings will be overwritten.
        void implTranslate(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rOverrides)
            throw (::com::sun::star::lang::IllegalArgumentException);

        /** normalize a path setting, delete it if the value could not be normalized
            @return <TRUE/> if the setting exists and is a valid path
        */
        sal_Bool implNormalizePathSetting(const sal_Char* _pSetting);

        // translate old settings, which exist for compatiblity only, into new ones
        void implTranslateCompatibilitySettings();

        // if we do not already have the given config path setting, ensure that it exists (calculated relative to a given path)
        void ensureConfigPath(const sal_Char* _pSetting, const ::rtl::OUString& _rBasePath, const sal_Char* _pRelative);

        sal_Bool        haveSetting(const sal_Char* _pName) const;
        void            putSetting(const sal_Char* _pName, const Setting& _rSetting);
        void            clearSetting(const sal_Char* _pName);

        ::rtl::OUString getStringSetting(const sal_Char* _pName) const;
        sal_Int32       getIntSetting(const sal_Char* _pName) const;
        sal_Bool        getBoolSetting(const sal_Char* _pName) const;
        Setting         getSetting(const sal_Char* _pName) const;
        Setting         getMaybeSetting(const sal_Char* _pName) const;

        ::rtl::OUString getProfileStringItem(const sal_Char* _pSection, const sal_Char* _pKey);
        sal_Int32       getProfileIntItem(const sal_Char* _pSection, const sal_Char* _pKey);

    private:
        // ensures that m_aImpl contains a session type
        // to be called from within construct only
        void implDetermineSessionType();

        // collect settings from the sregistry, put them into m_aImpl, if not overruled by runtime settings
        void implCollectSRegistrySetting();

        // clear items which are not relevant because of the session type origin
        void clearIrrelevantItems();
    };
}

#endif // CONFIGMGR_BOOTSTRAP_HXX_


